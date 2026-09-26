/* Winsock behind net/transport.h. A NetSocket is an index into socks[], since
   a SOCKET does not fit the int the rest of netplay passes around. */

#define _CRT_RAND_S
#include <mstcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../net/transport.h"

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

enum { NET_MAX_SOCKETS = 32 };

static SOCKET socks[NET_MAX_SOCKETS];
static int socks_ready;
static int wsa_ready;

static void net_startup(void)
{
    WSADATA wsa;
    int i;

    if (!socks_ready) {
        for (i = 0; i < NET_MAX_SOCKETS; i++) {
            socks[i] = INVALID_SOCKET;
        }
        socks_ready = 1;
    }
    if (!wsa_ready && WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
        wsa_ready = 1;
    }
}

static NetSocket net_register(SOCKET const s)
{
    int i;

    for (i = 0; i < NET_MAX_SOCKETS; i++) {
        if (socks[i] == INVALID_SOCKET) {
            socks[i] = s;
            return i;
        }
    }
    closesocket(s);
    return NET_SOCKET_NONE;
}

static SOCKET net_sock(NetSocket const s)
{
    return s >= 0 && s < NET_MAX_SOCKETS ? socks[s] : INVALID_SOCKET;
}

static int net_again(void)
{
    int const e = WSAGetLastError();

    return e == WSAEWOULDBLOCK || e == WSAEINTR || e == WSAEINPROGRESS;
}

int net_retry(void)
{
    return net_again();
}

static void net_set_nonblocking(SOCKET const s)
{
    u_long on = 1;

    ioctlsocket(s, FIONBIO, &on);
}

/* An ICMP "port unreachable" would otherwise fail the next recvfrom. */
static void net_set_datagram_options(SOCKET const s)
{
    BOOL off = FALSE;
    DWORD got = 0;

    WSAIoctl(s, SIO_UDP_CONNRESET, &off, sizeof(off), NULL, 0, &got, NULL, NULL);
}

static void net_set_stream_options(SOCKET const s)
{
    BOOL const one = TRUE;
    struct tcp_keepalive ka;
    DWORD got = 0;

    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char const*)&one, sizeof(one));
    ka.onoff = 1;
    ka.keepalivetime = 5000;
    ka.keepaliveinterval = 2000;
    WSAIoctl(s, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), NULL, 0, &got, NULL, NULL);
}

static void net_set_options(SOCKET const s, int const udp)
{
    if (udp) {
        net_set_datagram_options(s);
    } else {
        net_set_stream_options(s);
    }
    net_set_nonblocking(s);
}

NetSocket net_open(int const udp)
{
    int const type = udp ? SOCK_DGRAM : SOCK_STREAM;
    DWORD const v6only = 0;
    SOCKET s;

    net_startup();
    s = socket(AF_INET6, type, 0);
    if (s != INVALID_SOCKET) {
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char const*)&v6only, sizeof(v6only));
    } else {
        s = socket(AF_INET, type, 0);
    }
    if (s == INVALID_SOCKET) {
        return NET_SOCKET_NONE;
    }
    net_set_options(s, udp);
    return net_register(s);
}

void net_close(NetSocket const s)
{
    SOCKET const h = net_sock(s);

    if (h != INVALID_SOCKET) {
        closesocket(h);
        socks[s] = INVALID_SOCKET;
    }
}

void net_adopt(NetSocket const s, int const udp)
{
    SOCKET const h = net_sock(s);

    if (h != INVALID_SOCKET) {
        net_set_options(h, udp);
    }
}

int net_bind_any(NetSocket const s, uint16_t const port)
{
    SOCKET const h = net_sock(s);
    struct sockaddr_storage self;
    int self_len = sizeof(self);

    memset(&self, 0, sizeof(self));
    if (getsockname(h, (struct sockaddr*)&self, &self_len) != 0) {
        /* Unbound sockets fail getsockname here; ask for the family instead. */
        WSAPROTOCOL_INFOW info;
        int len = sizeof(info);

        if (getsockopt(h, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&info, &len) != 0) {
            return 0;
        }
        self.ss_family = (ADDRESS_FAMILY)info.iAddressFamily;
    }
    if (self.ss_family == AF_INET6) {
        struct sockaddr_in6 addr;

        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(port);
        return bind(h, (struct sockaddr*)&addr, sizeof(addr)) == 0;
    }
    {
        struct sockaddr_in addr;

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        return bind(h, (struct sockaddr*)&addr, sizeof(addr)) == 0;
    }
}

int net_listen(NetSocket const s, int const backlog)
{
    return listen(net_sock(s), backlog) == 0;
}

NetSocket net_accept(NetSocket const s)
{
    SOCKET const h = accept(net_sock(s), NULL, NULL);

    if (h == INVALID_SOCKET) {
        return NET_SOCKET_NONE;
    }
    net_set_options(h, 0);
    return net_register(h);
}

int net_wait(NetSocket const s, int const want_write, int timeout_ms)
{
    SOCKET const h = net_sock(s);
    struct timeval timeout;
    fd_set fds;

    if (h == INVALID_SOCKET) {
        return -1;
    }
    if (timeout_ms < 0) {
        timeout_ms = 0;
    }
    FD_ZERO(&fds);
    FD_SET(h, &fds);
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    if (want_write != 0) {
        /* A failed non-blocking connect shows up in the except set only. */
        fd_set errs;

        FD_ZERO(&errs);
        FD_SET(h, &errs);
        return select(0, NULL, &fds, &errs, &timeout);
    }
    return select(0, &fds, NULL, NULL, &timeout);
}

int net_wait_until(NetSocket const s, int const want_write, uint64_t const deadline_ms)
{
    uint64_t const now = net_now_ms();
    uint64_t remaining;

    if (now >= deadline_ms) {
        return 0;
    }
    remaining = deadline_ms - now;
    if (remaining > 0x7FFFFFFFu) {
        remaining = 0x7FFFFFFFu;
    }
    return net_wait(s, want_write, (int)remaining);
}

int net_connect_pending_ok(NetSocket const s)
{
    int err = 0;
    int len = sizeof(err);

    if (getsockopt(net_sock(s), SOL_SOCKET, SO_ERROR, (char*)&err, &len) != 0) {
        return 0;
    }
    return err == 0;
}

long net_send(NetSocket const s, void const* const buf, size_t const len)
{
    return (long)send(net_sock(s), (char const*)buf, (int)len, 0);
}

long net_recv(NetSocket const s, void* const buf, size_t const len)
{
    return (long)recv(net_sock(s), (char*)buf, (int)len, 0);
}

long net_recv_from(NetSocket const s, void* const buf, size_t const len, NetAddr* const from)
{
    struct sockaddr_storage peer;
    int peer_len = sizeof(peer);
    int n;

    memset(&peer, 0, sizeof(peer));
    n = recvfrom(net_sock(s), (char*)buf, (int)len, 0, (struct sockaddr*)&peer, &peer_len);
    if (n >= 0 && from) {
        if (peer_len > (int)sizeof(from->opaque)) {
            peer_len = (int)sizeof(from->opaque);
        }
        memcpy(from->opaque, &peer, (size_t)peer_len);
        from->len = (unsigned)peer_len;
    }
    return (long)n;
}

long net_send_to(NetSocket const s, void const* const buf, size_t const len, NetAddr const* const to)
{
    return (long)sendto(net_sock(s), (char const*)buf, (int)len, 0,
        (struct sockaddr const*)to->opaque, (int)to->len);
}

int net_connect_addr(NetSocket const s, NetAddr const* const a)
{
    return connect(net_sock(s), (struct sockaddr const*)a->opaque, (int)a->len) == 0;
}

int net_send_all(NetSocket const s, void const* const buf, size_t const len, int const timeout_ms)
{
    uint64_t const deadline = net_now_ms() + (uint64_t)(timeout_ms > 0 ? timeout_ms : 0);
    char const* const ptr = (char const*)buf;
    size_t sent = 0;

    while (sent < len) {
        long n;

        if (net_wait_until(s, 1, deadline) <= 0) {
            return 0;
        }
        n = net_send(s, ptr + sent, len - sent);
        if (n > 0) {
            sent += (size_t)n;
            continue;
        }
        if (n < 0 && net_again()) {
            continue;
        }
        return 0;
    }
    return 1;
}

int net_recv_all(NetSocket const s, void* const buf, size_t const len, int const timeout_ms)
{
    uint64_t const deadline = net_now_ms() + (uint64_t)(timeout_ms > 0 ? timeout_ms : 0);
    char* const ptr = (char*)buf;
    size_t got = 0;

    while (got < len) {
        long n;

        if (net_wait_until(s, 0, deadline) <= 0) {
            return 0;
        }
        n = net_recv(s, ptr + got, len - got);
        if (n > 0) {
            got += (size_t)n;
            continue;
        }
        if (n < 0 && net_again()) {
            continue;
        }
        return 0;
    }
    return 1;
}

uint64_t net_now_ms(void)
{
    return (uint64_t)GetTickCount64();
}

uint32_t net_random_u32(void)
{
    unsigned int token = 0;

    if (rand_s(&token) != 0 || token == 0) {
        token = (unsigned int)GetTickCount64() ^ (unsigned int)GetCurrentProcessId();
    }
    return token != 0 ? (uint32_t)token : 1u;
}

/* ---- asking a STUN server what address the world sees ---- */

static char net_extip_buf[24] = "";
static volatile LONG net_extip_active = 0;

static void net_extip_lookup(void)
{
    /* RFC 5389 Binding Request, no attributes, fixed transaction ID. */
    static uint8_t const req[20] = { 0x00, 0x01, 0x00, 0x00, 0x21, 0x12, 0xA4, 0x42,
        0x6E, 0x65, 0x74, 0x70, 0x6C, 0x61, 0x79, 0x5A, 0x53, 0x4E, 0x45, 0x53 };
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    uint8_t resp[256];
    DWORD const tv = 1000;
    SOCKET s;
    int n;
    int pos;
    unsigned msg_len;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo("stun.l.google.com", "19302", &hints, &res) != 0 || res == NULL) {
        return;
    }
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        freeaddrinfo(res);
        return;
    }
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char const*)&tv, sizeof(tv));
    sendto(s, (char const*)req, sizeof(req), 0, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);
    n = recv(s, (char*)resp, sizeof(resp), 0);
    closesocket(s);
    if (n < 20) {
        return;
    }
    msg_len = ((unsigned)resp[2] << 8) | resp[3];
    pos = 20;
    while (pos + 4 <= n && pos < 20 + (int)msg_len) {
        unsigned const type = ((unsigned)resp[pos] << 8) | resp[pos + 1];
        unsigned const alen = ((unsigned)resp[pos + 2] << 8) | resp[pos + 3];
        uint32_t ip;

        if (pos + 4 + (int)alen > n) {
            break;
        }
        if ((type == 0x0020 || type == 0x0001) && alen >= 8) {
            ip = ((uint32_t)resp[pos + 8] << 24) | ((uint32_t)resp[pos + 9] << 16)
                | ((uint32_t)resp[pos + 10] << 8) | resp[pos + 11];
            if (type == 0x0020) {
                ip ^= 0x2112A442u;
            }
            snprintf(net_extip_buf, sizeof(net_extip_buf), "%u.%u.%u.%u",
                (unsigned)(ip >> 24) & 0xFFu, (unsigned)(ip >> 16) & 0xFFu,
                (unsigned)(ip >> 8) & 0xFFu, (unsigned)ip & 0xFFu);
            return;
        }
        pos += 4 + (int)((alen + 3u) & ~3u);
    }
}

static DWORD WINAPI net_extip_thread(LPVOID arg)
{
    (void)arg;
    net_extip_lookup();
    InterlockedExchange(&net_extip_active, 0);
    return 0;
}

void net_extip_start(void)
{
    HANDLE t;

    net_startup();
    net_extip_buf[0] = '\0';
    InterlockedExchange(&net_extip_active, 1);
    t = CreateThread(NULL, 0, net_extip_thread, NULL, 0, NULL);
    if (t != NULL) {
        CloseHandle(t);
    } else {
        InterlockedExchange(&net_extip_active, 0);
    }
}

int net_extip_busy(void)
{
    return net_extip_active != 0;
}

char const* net_extip(void)
{
    return net_extip_buf;
}

/* ---- resolving and connecting, off the emulator's thread ---- */

static volatile LONG net_conn_active = 0;
static volatile LONG net_conn_cancel = 0;
static int net_conn_result = NET_CONNECT_PENDING;
/* A raw SOCKET: only the main thread touches socks[]. */
static SOCKET net_conn_sock = INVALID_SOCKET;
static char net_conn_host[64] = "";
static uint16_t net_conn_port = 0;
static int net_conn_udp = 0;

static DWORD WINAPI net_connect_thread(LPVOID arg)
{
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    struct addrinfo* ai;
    char portstr[8];
    SOCKET s = INVALID_SOCKET;

    (void)arg;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = net_conn_udp ? SOCK_DGRAM : SOCK_STREAM;
    snprintf(portstr, sizeof(portstr), "%u", (unsigned)net_conn_port);

    if (getaddrinfo(net_conn_host, portstr, &hints, &res) != 0 || res == NULL
        || net_conn_cancel) {
        if (res) {
            freeaddrinfo(res);
        }
        net_conn_result = NET_CONNECT_FAILED;
        InterlockedExchange(&net_conn_active, 0);
        return 0;
    }
    /* Blocking on purpose, as on Unix: this thread exists to wait. */
    for (ai = res; ai != NULL && !net_conn_cancel; ai = ai->ai_next) {
        s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s != INVALID_SOCKET && connect(s, ai->ai_addr, (int)ai->ai_addrlen) == 0) {
            break;
        }
        if (s != INVALID_SOCKET) {
            closesocket(s);
        }
        s = INVALID_SOCKET;
    }
    freeaddrinfo(res);
    if (s == INVALID_SOCKET || net_conn_cancel) {
        if (s != INVALID_SOCKET) {
            closesocket(s);
        }
        net_conn_result = NET_CONNECT_FAILED;
        InterlockedExchange(&net_conn_active, 0);
        return 0;
    }
    net_set_options(s, net_conn_udp);
    net_conn_sock = s;
    net_conn_result = NET_CONNECT_READY;
    InterlockedExchange(&net_conn_active, 0);
    return 0;
}

int net_connect_start(char const* const host, uint16_t const port, int const udp)
{
    HANDLE t;

    net_startup();
    if (net_conn_active) {
        return 0;
    }
    snprintf(net_conn_host, sizeof(net_conn_host), "%s", host);
    net_conn_port = port;
    net_conn_udp = udp;
    net_conn_sock = INVALID_SOCKET;
    net_conn_result = NET_CONNECT_PENDING;
    InterlockedExchange(&net_conn_cancel, 0);
    InterlockedExchange(&net_conn_active, 1);
    t = CreateThread(NULL, 0, net_connect_thread, NULL, 0, NULL);
    if (t == NULL) {
        InterlockedExchange(&net_conn_active, 0);
        return 0;
    }
    CloseHandle(t);
    return 1;
}

int net_connect_poll(NetSocket* const out)
{
    if (net_conn_active) {
        return NET_CONNECT_PENDING;
    }
    if (net_conn_result == NET_CONNECT_READY) {
        *out = net_register(net_conn_sock);
        net_conn_sock = INVALID_SOCKET;
        net_conn_result = NET_CONNECT_PENDING;
        return *out != NET_SOCKET_NONE ? NET_CONNECT_READY : NET_CONNECT_FAILED;
    }
    if (net_conn_result == NET_CONNECT_FAILED) {
        net_conn_result = NET_CONNECT_PENDING;
        return NET_CONNECT_FAILED;
    }
    return NET_CONNECT_PENDING;
}

void net_connect_cancel(void)
{
    InterlockedExchange(&net_conn_cancel, 1);
    if (!net_conn_active && net_conn_sock != INVALID_SOCKET) {
        closesocket(net_conn_sock);
        net_conn_sock = INVALID_SOCKET;
    }
    net_conn_result = NET_CONNECT_PENDING;
}
