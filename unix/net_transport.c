/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* BSD sockets behind net/transport.h. This is the only file in the netplay
   path that knows what a sockaddr is. */

#include "../net/transport.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* Sends on a closed connection should fail, not raise SIGPIPE and take the
   emulator down with them. */
#ifdef MSG_NOSIGNAL
enum { NET_SEND_FLAGS = MSG_NOSIGNAL };
#else
enum { NET_SEND_FLAGS = 0 };
#endif

static void net_set_nonblocking(int const fd)
{
    int const flags = fcntl(fd, F_GETFL, 0);

    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

/* Nagle off so a pad's worth of bytes leaves immediately, and keepalives so a
   peer that vanishes is noticed rather than waited on for ever. */
static void net_set_stream_options(int const fd)
{
    int const one = 1;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
#ifdef TCP_KEEPIDLE
    {
        int const idle = 5;

        setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    }
#endif
#ifdef TCP_KEEPINTVL
    {
        int const intvl = 2;

        setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    }
#endif
#ifdef TCP_KEEPCNT
    {
        int const cnt = 3;

        setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    }
#endif
}

NetSocket net_open(int const udp)
{
    int const fd = socket(AF_INET, udp ? SOCK_DGRAM : SOCK_STREAM, 0);
    int const reuse = 1;

    if (fd < 0) {
        return NET_SOCKET_NONE;
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (!udp) {
        net_set_stream_options(fd);
    }
    net_set_nonblocking(fd);
    return fd;
}

void net_close(NetSocket const s)
{
    if (s != NET_SOCKET_NONE) {
        close(s);
    }
}

void net_adopt(NetSocket const s, int const udp)
{
    if (s == NET_SOCKET_NONE) {
        return;
    }
    if (!udp) {
        net_set_stream_options(s);
    }
    net_set_nonblocking(s);
}

int net_bind_any(NetSocket const s, uint16_t const port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    return bind(s, (struct sockaddr*)&addr, sizeof(addr)) == 0;
}

int net_listen(NetSocket const s, int const backlog)
{
    return listen(s, backlog) == 0;
}

NetSocket net_accept(NetSocket const s)
{
    int const fd = accept(s, NULL, NULL);

    if (fd < 0) {
        return NET_SOCKET_NONE;
    }
    net_set_stream_options(fd);
    net_set_nonblocking(fd);
    return fd;
}

int net_wait(NetSocket const s, int const want_write, int timeout_ms)
{
    struct timeval timeout;
    fd_set fds;

    if (timeout_ms < 0) {
        timeout_ms = 0;
    }
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    if (want_write != 0) {
        return select(s + 1, NULL, &fds, NULL, &timeout);
    }
    return select(s + 1, &fds, NULL, NULL, &timeout);
}

int net_wait_until(NetSocket const s, int const want_write, uint64_t const deadline_ms)
{
    uint64_t const now = net_now_ms();
    uint64_t remaining;

    if (now >= deadline_ms) {
        return 0;
    }
    remaining = deadline_ms - now;
    if (remaining > (uint64_t)INT_MAX) {
        remaining = (uint64_t)INT_MAX;
    }
    return net_wait(s, want_write, (int)remaining);
}

int net_connect_pending_ok(NetSocket const s)
{
    socklen_t len = sizeof(int);
    int err = 0;

    if (getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len) != 0) {
        return 0;
    }
    return err == 0;
}

long net_send(NetSocket const s, void const* const buf, size_t const len)
{
    return (long)send(s, buf, len, NET_SEND_FLAGS);
}

long net_recv(NetSocket const s, void* const buf, size_t const len)
{
    return (long)recv(s, buf, len, 0);
}

long net_recv_from(NetSocket const s, void* const buf, size_t const len, NetAddr* const from)
{
    struct sockaddr_storage peer;
    socklen_t peer_len = sizeof(peer);
    ssize_t n;

    memset(&peer, 0, sizeof(peer));
    n = recvfrom(s, buf, len, 0, (struct sockaddr*)&peer, &peer_len);
    if (n >= 0 && from) {
        if (peer_len > sizeof(from->opaque)) {
            peer_len = sizeof(from->opaque);
        }
        memcpy(from->opaque, &peer, peer_len);
        from->len = (unsigned)peer_len;
    }
    return (long)n;
}

long net_send_to(NetSocket const s, void const* const buf, size_t const len, NetAddr const* const to)
{
    return (long)sendto(s, buf, len, NET_SEND_FLAGS,
        (struct sockaddr const*)to->opaque, (socklen_t)to->len);
}

int net_connect_addr(NetSocket const s, NetAddr const* const a)
{
    return connect(s, (struct sockaddr const*)a->opaque, (socklen_t)a->len) == 0;
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
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
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
        if (n == 0) {
            return 0;
        }
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }
        return 0;
    }
    return 1;
}

uint64_t net_now_ms(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000u + (uint64_t)now.tv_nsec / 1000000u;
}

uint32_t net_random_u32(void)
{
    uint32_t token = 0;
    FILE* const fp = fopen("/dev/urandom", "rb");

    if (fp) {
        if (fread(&token, sizeof(token), 1, fp) != 1) {
            token = 0;
        }
        fclose(fp);
    }
    if (token == 0) {
        token = (uint32_t)net_now_ms() ^ (uint32_t)(uintptr_t)&token
            ^ (uint32_t)getpid();
    }
    return token != 0 ? token : 1u;
}

/* ---- asking a STUN server what address the world sees ---- */

static char net_extip_buf[24] = "";
static volatile int net_extip_active = 0;

static void net_extip_lookup(void)
{
    // RFC 5389 STUN Binding Request — no attributes, fixed transaction ID
    static uint8_t const req[20] = {
        0x00,
        0x01, // Binding Request
        0x00,
        0x00, // Attributes length: 0
        0x21,
        0x12,
        0xA4,
        0x42, // Magic cookie
        0x6E,
        0x65,
        0x74,
        0x70, // Transaction ID
        0x6C,
        0x61,
        0x79,
        0x5A,
        0x53,
        0x4E,
        0x45,
        0x53,
    };

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo* res = NULL;
    if (getaddrinfo("stun.l.google.com", "19302", &hints, &res) != 0 || res == NULL)
        return;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        freeaddrinfo(res);
        return;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    sendto(fd, req, sizeof(req), 0, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    uint8_t resp[256];
    ssize_t n = recv(fd, resp, sizeof(resp), 0);
    close(fd);
    if (n < 20)
        return;

    uint16_t msg_len;
    memcpy(&msg_len, resp + 2, 2);
    msg_len = ntohs(msg_len);

    int pos = 20;
    while (pos + 4 <= (int)n && pos < 20 + (int)msg_len) {
        uint16_t attr_type, attr_len;
        memcpy(&attr_type, resp + pos, 2);
        memcpy(&attr_len, resp + pos + 2, 2);
        attr_type = ntohs(attr_type);
        attr_len = ntohs(attr_len);

        if (attr_type == 0x0020 && attr_len >= 8) { // XOR-MAPPED-ADDRESS
            uint32_t xaddr;
            memcpy(&xaddr, resp + pos + 8, 4);
            uint32_t const ip = ntohl(xaddr) ^ 0x2112A442u;
            snprintf(net_extip_buf, sizeof(net_extip_buf), "%u.%u.%u.%u",
                (ip >> 24) & 0xFFu, (ip >> 16) & 0xFFu,
                (ip >> 8) & 0xFFu, ip & 0xFFu);
            return;
        }
        if (attr_type == 0x0001 && attr_len >= 8) { // MAPPED-ADDRESS fallback
            uint32_t addr;
            memcpy(&addr, resp + pos + 8, 4);
            uint32_t const ip = ntohl(addr);
            snprintf(net_extip_buf, sizeof(net_extip_buf), "%u.%u.%u.%u",
                (ip >> 24) & 0xFFu, (ip >> 16) & 0xFFu,
                (ip >> 8) & 0xFFu, ip & 0xFFu);
            return;
        }
        pos += 4 + (int)((attr_len + 3u) & ~3u);
    }
}

static void* net_extip_thread(void* arg)
{
    (void)arg;
    net_extip_lookup();
    net_extip_active = 0;
    return NULL;
}

void net_extip_start(void)
{
    net_extip_buf[0] = '\0';
    net_extip_active = 1;
    pthread_t t;
    if (pthread_create(&t, NULL, net_extip_thread, NULL) == 0)
        pthread_detach(t);
    else
        net_extip_active = 0;
}

int net_extip_busy(void)
{
    return net_extip_active;
}

char const* net_extip(void)
{
    return net_extip_buf;
}

/* ---- resolving and connecting, off the emulator's thread ---- */

static volatile int net_conn_active = 0;
static volatile int net_conn_cancel = 0;
static int net_conn_result = NET_CONNECT_PENDING;
static NetSocket net_conn_sock = NET_SOCKET_NONE;
static char net_conn_host[64] = "";
static uint16_t net_conn_port = 0;
static int net_conn_udp = 0;

static void* net_connect_thread(void* arg)
{
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    char portstr[8];
    NetSocket fd;

    (void)arg;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = net_conn_udp ? SOCK_DGRAM : SOCK_STREAM;
    snprintf(portstr, sizeof(portstr), "%u", (unsigned)net_conn_port);

    if (getaddrinfo(net_conn_host, portstr, &hints, &res) != 0 || res == NULL
        || net_conn_cancel) {
        if (res) {
            freeaddrinfo(res);
        }
        net_conn_result = NET_CONNECT_FAILED;
        net_conn_active = 0;
        return NULL;
    }

    /* Blocking on purpose: this is a thread of its own, and a connect that
       has to wait for a distant peer is exactly what it is here for. */
    fd = socket(hints.ai_family, hints.ai_socktype, 0);
    if (fd < 0 || connect(fd, res->ai_addr, (socklen_t)res->ai_addrlen) != 0
        || net_conn_cancel) {
        if (fd >= 0) {
            close(fd);
        }
        freeaddrinfo(res);
        net_conn_result = NET_CONNECT_FAILED;
        net_conn_active = 0;
        return NULL;
    }
    freeaddrinfo(res);

    net_adopt(fd, net_conn_udp);
    net_conn_sock = fd;
    net_conn_result = NET_CONNECT_READY;
    net_conn_active = 0;
    return NULL;
}

int net_connect_start(char const* const host, uint16_t const port, int const udp)
{
    pthread_t t;

    if (net_conn_active) {
        return 0;
    }
    snprintf(net_conn_host, sizeof(net_conn_host), "%s", host);
    net_conn_port = port;
    net_conn_udp = udp;
    net_conn_sock = NET_SOCKET_NONE;
    net_conn_result = NET_CONNECT_PENDING;
    net_conn_cancel = 0;
    net_conn_active = 1;
    if (pthread_create(&t, NULL, net_connect_thread, NULL) != 0) {
        net_conn_active = 0;
        return 0;
    }
    pthread_detach(t);
    return 1;
}

int net_connect_poll(NetSocket* const out)
{
    if (net_conn_active) {
        return NET_CONNECT_PENDING;
    }
    if (net_conn_result == NET_CONNECT_READY) {
        *out = net_conn_sock;
        net_conn_sock = NET_SOCKET_NONE;
        net_conn_result = NET_CONNECT_PENDING;
        return NET_CONNECT_READY;
    }
    if (net_conn_result == NET_CONNECT_FAILED) {
        net_conn_result = NET_CONNECT_PENDING;
        return NET_CONNECT_FAILED;
    }
    return NET_CONNECT_PENDING;
}

void net_connect_cancel(void)
{
    net_conn_cancel = 1;
    if (!net_conn_active && net_conn_sock != NET_SOCKET_NONE) {
        net_close(net_conn_sock);
        net_conn_sock = NET_SOCKET_NONE;
    }
    net_conn_result = NET_CONNECT_PENDING;
}
