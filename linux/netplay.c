/*
 * ZSNES netplay client.
 *
 * Connects to the Go relay server (server/main.go), authenticates with a room
 * code + optional password, and from there exchanges 16-byte input frames
 * with the paired peer in lockstep. Transport is TLS-on-TCP (OpenSSL) when
 * built with WITH_TLS, otherwise plain TCP — see Makefile.
 *
 * See linux/netplay.h for the public API and configuration env vars.
 */

#include "netplay.h"

#ifdef __UNIXSDL__

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifdef WITH_TLS
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

#include "../ui.h"

/* Public globals — referenced by the GUI. */
char NetplayHostName[32] = "127.0.0.1";
char* GUINetplayTextPtr[1] = { NetplayHostName };

/* 1 = use TLS, 0 = plaintext. Default secure. */
u1 NetplayTLSConfig = 1;

/* ------- internal state ------- */

#define NETPLAY_INPUT_DELAY 3
#define NETPLAY_FRAME_MS 17

enum {
    NS_IDLE = 0,
    NS_CONNECTING = 1,
    NS_WAITING = 2,
    NS_READY = 3,
    NS_ERROR = 4
};

typedef struct {
    int sock;
#ifdef WITH_TLS
    SSL* ssl;
    SSL_CTX* ssl_ctx;
#endif
} NetIO;

static NetIO g_io = { -1 };

static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_state = NS_IDLE;
static volatile int g_worker_running = 0;
static volatile int g_cancel = 0;
static pthread_t g_worker;

static char g_status[96] = "IDLE";
static char g_room[ZNP_ROOM_CODE_LEN + 1] = "";
static u1 g_role = 0;
static u4 g_local_seq = 0;
static u4 g_input_queue[NETPLAY_INPUT_DELAY];
static int g_input_queue_pos = 0;
static int g_input_queue_filled = 0;

/* RTT tracking — set by NetplayServiceTick when paired. */
static volatile int g_rtt_ms = -1; /* last measured client↔server RTT */
static u8 g_ping_sent_us = 0; /* monotonic-us when in-flight PING was sent */
static u8 g_ping_next_us = 0; /* monotonic-us at which to send the next PING */
static int g_ping_in_flight = 0;

static u8 monotonic_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u8)ts.tv_sec * 1000000ull + (u8)(ts.tv_nsec / 1000);
}

static void set_status(char const* s)
{
    pthread_mutex_lock(&g_mu);
    snprintf(g_status, sizeof(g_status), "%s", s);
    pthread_mutex_unlock(&g_mu);
}

static void set_statusf(char const* fmt, ...)
    __attribute__((format(printf, 1, 2)));

static void set_statusf(char const* fmt, ...)
{
    char buf[96];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    set_status(buf);
}

char const* NetplayStatusText(void)
{
    static __thread char copy[96];
    pthread_mutex_lock(&g_mu);
    memcpy(copy, g_status, sizeof(copy));
    pthread_mutex_unlock(&g_mu);
    return copy;
}

/* ------- config parsing ------- */

/*
 * NetplayHostName is parsed as "host[:port][/room]". A trailing "/room" is
 * peeled off and used as the room code; if absent, the room code is
 * "default". Likewise port defaults to ZNP_DEFAULT_PORT.
 */
static void parse_config(char* host, size_t host_sz,
    char* port, size_t port_sz,
    char* room, size_t room_sz)
{
    char src[sizeof(NetplayHostName)];
    snprintf(src, sizeof(src), "%s", NetplayHostName);

    char* slash = strchr(src, '/');
    if (slash != NULL) {
        *slash = '\0';
        snprintf(room, room_sz, "%s", slash + 1);
    } else {
        snprintf(room, room_sz, "default");
    }

    char* colon = strrchr(src, ':');
    /* Don't mis-parse "::1" — only treat as port separator if the remainder
     * is all digits. */
    int is_port = 0;
    if (colon != NULL && colon[1] != '\0') {
        is_port = 1;
        for (char* p = colon + 1; *p; p++) {
            if (!isdigit((unsigned char)*p)) {
                is_port = 0;
                break;
            }
        }
    }
    if (is_port) {
        *colon = '\0';
        snprintf(port, port_sz, "%s", colon + 1);
    } else {
        snprintf(port, port_sz, "%u", (unsigned)ZNP_DEFAULT_PORT);
    }
    snprintf(host, host_sz, "%s", src[0] != '\0' ? src : "127.0.0.1");
}

static void env_or(char const* name, char const* fallback, char* out, size_t out_sz)
{
    char const* v = getenv(name);
    snprintf(out, out_sz, "%s", (v != NULL && v[0] != '\0') ? v : fallback);
}

/* ------- IO helpers ------- */

static int io_send_all(NetIO* io, void const* data, size_t n)
{
    char const* p = (char const*)data;
    size_t sent = 0;
    while (sent < n) {
        if (g_cancel)
            return 0;
#ifdef WITH_TLS
        if (io->ssl != NULL) {
            int r = SSL_write(io->ssl, p + sent, (int)(n - sent));
            if (r > 0) {
                sent += (size_t)r;
                continue;
            }
            int err = SSL_get_error(io->ssl, r);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                struct timeval tv = { 1, 0 };
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(io->sock, &fds);
                select(io->sock + 1, NULL, &fds, NULL, &tv);
                continue;
            }
            return 0;
        }
#endif
        ssize_t r = send(io->sock, p + sent, n - sent, MSG_NOSIGNAL);
        if (r > 0) {
            sent += (size_t)r;
            continue;
        }
        if (r < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        return 0;
    }
    return 1;
}

static int io_recv_all(NetIO* io, void* data, size_t n, int timeout_ms)
{
    char* p = (char*)data;
    size_t got = 0;
    while (got < n) {
        if (g_cancel)
            return 0;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(io->sock, &fds);
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        int sel = select(io->sock + 1, &fds, NULL, NULL, &tv);
        if (sel < 0 && errno == EINTR)
            continue;
        if (sel <= 0)
            return 0;
#ifdef WITH_TLS
        if (io->ssl != NULL) {
            int r = SSL_read(io->ssl, p + got, (int)(n - got));
            if (r > 0) {
                got += (size_t)r;
                continue;
            }
            int err = SSL_get_error(io->ssl, r);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
                continue;
            return 0;
        }
#endif
        ssize_t r = recv(io->sock, p + got, n - got, 0);
        if (r > 0) {
            got += (size_t)r;
            continue;
        }
        if (r == 0)
            return 0;
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
        return 0;
    }
    return 1;
}

static int io_recv_peek(NetIO* io, int timeout_ms)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(io->sock, &fds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    int sel = select(io->sock + 1, &fds, NULL, NULL, &tv);
    if (sel <= 0)
        return 0;
#ifdef WITH_TLS
    if (io->ssl != NULL) {
        char b;
        int n = SSL_peek(io->ssl, &b, 1);
        return n > 0 ? 1 : 0;
    }
#endif
    char b;
    ssize_t n = recv(io->sock, &b, 1, MSG_PEEK);
    return n > 0 ? 1 : 0;
}

/* Big-endian 16/32-bit helpers (avoid byteswap.h portability headaches). */
static inline u2 be16(u2 v) { return (u2)((v >> 8) | (v << 8)); }
static inline u4 be32(u4 v)
{
    return ((v & 0xFF000000u) >> 24) | ((v & 0x00FF0000u) >> 8) | ((v & 0x0000FF00u) << 8) | ((v & 0x000000FFu) << 24);
}
static int is_big_endian(void)
{
    u4 const probe = 1;
    return ((u1 const*)&probe)[0] == 0;
}
static u2 hto16(u2 v) { return is_big_endian() ? v : be16(v); }
static u4 hto32(u4 v) { return is_big_endian() ? v : be32(v); }
static u2 toh16(u2 v) { return hto16(v); }
static u4 toh32(u4 v) { return hto32(v); }

/* ------- framing ------- */

static int frame_send(NetIO* io, u2 type, void const* payload, u2 length)
{
    if (length > ZNP_FRAME_MAX_PAYLOAD)
        return 0;
    u1 hdr[4];
    u2 t = hto16(type);
    u2 l = hto16(length);
    memcpy(hdr, &t, 2);
    memcpy(hdr + 2, &l, 2);
    if (!io_send_all(io, hdr, 4))
        return 0;
    if (length != 0 && !io_send_all(io, payload, length))
        return 0;
    return 1;
}

static int frame_recv(NetIO* io, u2* type, void* payload, u2 payload_cap, u2* out_len, int timeout_ms)
{
    u1 hdr[4];
    if (!io_recv_all(io, hdr, 4, timeout_ms))
        return 0;
    u2 t, l;
    memcpy(&t, hdr, 2);
    memcpy(&l, hdr + 2, 2);
    *type = toh16(t);
    *out_len = toh16(l);
    if (*out_len > ZNP_FRAME_MAX_PAYLOAD || *out_len > payload_cap)
        return 0;
    if (*out_len != 0 && !io_recv_all(io, payload, *out_len, timeout_ms))
        return 0;
    return 1;
}

/* ------- TLS pinning ------- */

#ifdef WITH_TLS
static int hex_decode(char const* hex, u1* out, size_t out_len)
{
    size_t n = strlen(hex);
    if (n != out_len * 2)
        return 0;
    for (size_t i = 0; i < out_len; i++) {
        char c1 = hex[i * 2], c2 = hex[i * 2 + 1];
        int h = -1, l = -1;
        if (c1 >= '0' && c1 <= '9')
            h = c1 - '0';
        else if (c1 >= 'a' && c1 <= 'f')
            h = c1 - 'a' + 10;
        else if (c1 >= 'A' && c1 <= 'F')
            h = c1 - 'A' + 10;
        if (c2 >= '0' && c2 <= '9')
            l = c2 - '0';
        else if (c2 >= 'a' && c2 <= 'f')
            l = c2 - 'a' + 10;
        else if (c2 >= 'A' && c2 <= 'F')
            l = c2 - 'A' + 10;
        if (h < 0 || l < 0)
            return 0;
        out[i] = (u1)((h << 4) | l);
    }
    return 1;
}

static int verify_pin(SSL* ssl, char const* pin_hex)
{
    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL)
        return 0;
    u1* der = NULL;
    int der_len = i2d_X509_PUBKEY(X509_get_X509_PUBKEY(cert), &der);
    X509_free(cert);
    if (der_len <= 0 || der == NULL)
        return 0;
    u1 digest[SHA256_DIGEST_LENGTH];
    SHA256(der, der_len, digest);
    OPENSSL_free(der);
    u1 expected[SHA256_DIGEST_LENGTH];
    if (!hex_decode(pin_hex, expected, SHA256_DIGEST_LENGTH))
        return 0;
    /* constant-time compare */
    u1 diff = 0;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        diff |= (u1)(digest[i] ^ expected[i]);
    return diff == 0;
}
#endif /* WITH_TLS */

/* ------- connect / handshake ------- */

static void io_close(NetIO* io)
{
#ifdef WITH_TLS
    if (io->ssl != NULL) {
        SSL_shutdown(io->ssl);
        SSL_free(io->ssl);
        io->ssl = NULL;
    }
    if (io->ssl_ctx != NULL) {
        SSL_CTX_free(io->ssl_ctx);
        io->ssl_ctx = NULL;
    }
#endif
    if (io->sock >= 0) {
        close(io->sock);
        io->sock = -1;
    }
}

static int tcp_dial(char const* host, char const* port, int timeout_ms)
{
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0 || res == NULL)
        return -1;

    int fd = -1;
    for (struct addrinfo* ai = res; ai != NULL; ai = ai->ai_next) {
        if (g_cancel)
            break;
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0)
            continue;
        /* non-blocking connect with deadline */
        int fl = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, fl | O_NONBLOCK);
        int rc = connect(fd, ai->ai_addr, ai->ai_addrlen);
        if (rc == 0)
            goto ok;
        if (errno != EINPROGRESS) {
            close(fd);
            fd = -1;
            continue;
        }
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(fd, &wfds);
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        int sel = select(fd + 1, NULL, &wfds, NULL, &tv);
        if (sel <= 0) {
            close(fd);
            fd = -1;
            continue;
        }
        int so_err = 0;
        socklen_t l = sizeof(so_err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_err, &l) != 0 || so_err != 0) {
            close(fd);
            fd = -1;
            continue;
        }
    ok:
        fcntl(fd, F_SETFL, fl);
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
        break;
    }
    freeaddrinfo(res);
    return fd;
}

#ifdef WITH_TLS
static int tls_wrap(NetIO* io, char const* host, char const* pin_hex, int insecure)
{
    static int initialized = 0;
    if (!initialized) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        initialized = 1;
    }
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == NULL)
        return 0;
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION | SSL_OP_NO_RENEGOTIATION);
    SSL_CTX_set_default_verify_paths(ctx);

    /* If a pin is provided OR the user has opted into insecure, skip the CA
     * chain check; the pin (or absence-of-check) decides trust below. */
    if (pin_hex != NULL || insecure)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    else
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    SSL* ssl = SSL_new(ctx);
    if (ssl == NULL) {
        SSL_CTX_free(ctx);
        return 0;
    }
    SSL_set_tlsext_host_name(ssl, host);
    SSL_set_fd(ssl, io->sock);

    /* Blocking handshake — io->sock is currently blocking after tcp_dial. */
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return 0;
    }

    if (pin_hex != NULL && !verify_pin(ssl, pin_hex)) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        set_status("CERTIFICATE PIN MISMATCH");
        return 0;
    }

    io->ssl = ssl;
    io->ssl_ctx = ctx;
    return 1;
}
#endif

static void copy_padded(char* dst, size_t dst_sz, char const* src)
{
    memset(dst, 0, dst_sz);
    size_t n = strlen(src);
    if (n > dst_sz)
        n = dst_sz;
    memcpy(dst, src, n);
}

static int send_client_hello(NetIO* io, char const* room, char const* password, char const* nick, u1 mode)
{
    u1 buf[4 + 1 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN + ZNP_NICKNAME_LEN];
    u4 proto = hto32(ZNP_PROTOCOL_VERSION);
    memcpy(buf, &proto, 4);
    buf[4] = mode;
    copy_padded((char*)(buf + 5), ZNP_ROOM_CODE_LEN, room);
    copy_padded((char*)(buf + 5 + ZNP_ROOM_CODE_LEN), ZNP_PASSWORD_LEN, password);
    copy_padded((char*)(buf + 5 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN), ZNP_NICKNAME_LEN, nick);
    return frame_send(io, ZNP_FRAME_CLIENT_HELLO, buf, sizeof(buf));
}

static void* worker_main(void* arg)
{
    (void)arg;

    char host[64], port[16], room[ZNP_ROOM_CODE_LEN + 1];
    char password[ZNP_PASSWORD_LEN + 1];
    char nick[ZNP_NICKNAME_LEN + 1];
    parse_config(host, sizeof(host), port, sizeof(port), room, sizeof(room));
    env_or("ZSNES_NETPLAY_PASSWORD", "", password, sizeof(password));
    env_or("ZSNES_NETPLAY_NICK", "p1", nick, sizeof(nick));

    set_statusf("CONNECTING %s:%s", host, port);
    int fd = tcp_dial(host, port, 5000);
    if (fd < 0) {
        set_status("CONNECT FAILED");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }

    NetIO io;
    memset(&io, 0, sizeof(io));
    io.sock = fd;

    if (NetplayTLSConfig != 0) {
#ifdef WITH_TLS
        char const* pin = getenv("ZSNES_NETPLAY_PIN");
        char const* insec_env = getenv("ZSNES_NETPLAY_INSECURE");
        int insec = (insec_env != NULL && insec_env[0] == '1') ? 1 : 0;
        if (pin == NULL && !insec) {
            io_close(&io);
            set_status("NO PIN - SET ZSNES_NETPLAY_PIN");
            g_state = NS_ERROR;
            g_worker_running = 0;
            return NULL;
        }
        if (!tls_wrap(&io, host, pin, insec)) {
            io_close(&io);
            if (g_status[0] == 'C' && strncmp(g_status, "CERTIFICATE", 11) == 0) {
                /* keep pin-mismatch message */
            } else {
                set_status("TLS HANDSHAKE FAILED");
            }
            g_state = NS_ERROR;
            g_worker_running = 0;
            return NULL;
        }
#else
        io_close(&io);
        set_status("TLS NOT COMPILED - REBUILD WITH OPENSSL");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
#endif
    }

    /* Stream prefix exchange. */
    if (!io_send_all(&io, ZNP_STREAM_PREFIX, ZNP_STREAM_PREFIX_LEN)) {
        io_close(&io);
        set_status("PREFIX SEND FAILED");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }
    char prefix[ZNP_STREAM_PREFIX_LEN];
    if (!io_recv_all(&io, prefix, ZNP_STREAM_PREFIX_LEN, 5000) || memcmp(prefix, ZNP_STREAM_PREFIX, ZNP_STREAM_PREFIX_LEN) != 0) {
        io_close(&io);
        set_status("BAD SERVER PREFIX");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }

    /* Always use CREATE mode: the server treats it as "create the room if it
     * doesn't exist, otherwise join it". The first arriving client is the
     * host (P1), the second is the client (P2). */
    if (!send_client_hello(&io, room, password, nick, ZNP_HELLO_MODE_CREATE)) {
        io_close(&io);
        set_status("HELLO SEND FAILED");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }

    u2 type = 0, len = 0;
    u1 buf[ZNP_FRAME_MAX_PAYLOAD];
    if (!frame_recv(&io, &type, buf, sizeof(buf), &len, 5000)) {
        io_close(&io);
        set_status("HELLO RECV FAILED");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }
    if (type == ZNP_FRAME_SERVER_ERROR) {
        char msg[64] = "";
        size_t take = len < sizeof(msg) - 1 ? len : sizeof(msg) - 1;
        memcpy(msg, buf, take);
        msg[take] = '\0';
        io_close(&io);
        set_statusf("SERVER: %s", msg);
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }
    if (type != ZNP_FRAME_SERVER_HELLO || len < 5 + ZNP_ROOM_CODE_LEN) {
        io_close(&io);
        set_status("BAD SERVER HELLO");
        g_state = NS_ERROR;
        g_worker_running = 0;
        return NULL;
    }
    u4 srv_proto;
    memcpy(&srv_proto, buf, 4);
    srv_proto = toh32(srv_proto);
    (void)srv_proto; /* protocol field reserved for future negotiation */
    pthread_mutex_lock(&g_mu);
    g_role = buf[4];
    memcpy(g_room, buf + 5, ZNP_ROOM_CODE_LEN);
    g_room[ZNP_ROOM_CODE_LEN] = '\0';
    g_io = io;
    g_state = NS_WAITING;
    pthread_mutex_unlock(&g_mu);

    set_statusf("ROOM %s - WAITING FOR PEER (%s)", g_room, g_role == ZNP_ROLE_HOST ? "P1" : "P2");

    g_worker_running = 0;
    return NULL;
}

/* ------- public API ------- */

void NetplayJoinSession(void)
{
    if (g_worker_running)
        return;
    NetplayDisconnectSession();
    g_cancel = 0;
    g_rtt_ms = -1;
    g_ping_in_flight = 0;
    g_ping_next_us = 0;
    g_state = NS_CONNECTING;
    g_worker_running = 1;
    if (pthread_create(&g_worker, NULL, worker_main, NULL) != 0) {
        g_worker_running = 0;
        g_state = NS_ERROR;
        set_status("THREAD FAILED");
        return;
    }
    pthread_detach(g_worker);
}

void NetplayDisconnectSession(void)
{
    g_cancel = 1;
    /* Snapshot under lock, then release before doing slow IO. */
    pthread_mutex_lock(&g_mu);
    NetIO io = g_io;
    g_io.sock = -1;
#ifdef WITH_TLS
    g_io.ssl = NULL;
    g_io.ssl_ctx = NULL;
#endif
    g_state = NS_IDLE;
    g_role = 0;
    g_local_seq = 0;
    g_input_queue_pos = 0;
    g_input_queue_filled = 0;
    g_room[0] = '\0';
    pthread_mutex_unlock(&g_mu);

    if (io.sock >= 0) {
        /* Best-effort BYE; ignore errors. */
        frame_send(&io, ZNP_FRAME_BYE, NULL, 0);
    }
    io_close(&io);
    set_status("IDLE");
}

static char g_peer_nick[ZNP_NICKNAME_LEN + 1] = "";

static void update_ready_status(void)
{
    char const* who = g_role == ZNP_ROLE_HOST ? "P1" : "P2";
    char const* peer = g_peer_nick[0] != '\0' ? g_peer_nick : "p?";
    if (g_rtt_ms >= 0)
        set_statusf("READY %s - PEER %s - RTT %dms", who, peer, g_rtt_ms);
    else
        set_statusf("READY %s - PEER %s - PINGING...", who, peer);
}

/*
 * Handle one frame that's already known to be available. Used by both the
 * GUI-only WAITING poll and the READY ping/keepalive loop.
 */
static int dispatch_one_frame(int timeout_ms)
{
    u2 type = 0, len = 0;
    u1 buf[ZNP_FRAME_MAX_PAYLOAD];
    if (!frame_recv(&g_io, &type, buf, sizeof(buf), &len, timeout_ms)) {
        NetplayDisconnectSession();
        set_status("CONNECTION LOST");
        return 0;
    }
    switch (type) {
    case ZNP_FRAME_PEER_READY: {
        size_t take = len < ZNP_NICKNAME_LEN ? len : ZNP_NICKNAME_LEN;
        memcpy(g_peer_nick, buf, take);
        g_peer_nick[take] = '\0';
        pthread_mutex_lock(&g_mu);
        g_state = NS_READY;
        pthread_mutex_unlock(&g_mu);
        update_ready_status();
        break;
    }
    case ZNP_FRAME_PONG: {
        if (len == 8 && g_ping_in_flight) {
            u8 echoed = 0;
            for (int i = 0; i < 8; i++)
                echoed = (echoed << 8) | (u8)buf[i];
            u8 now = monotonic_us();
            if (now >= echoed) {
                int rtt = (int)((now - echoed) / 1000ull);
                if (rtt > 9999)
                    rtt = 9999;
                g_rtt_ms = rtt;
            }
            g_ping_in_flight = 0;
            if (g_state == NS_READY)
                update_ready_status();
        }
        break;
    }
    case ZNP_FRAME_SERVER_ERROR: {
        char msg[64] = "";
        size_t take = len < sizeof(msg) - 1 ? len : sizeof(msg) - 1;
        memcpy(msg, buf, take);
        set_statusf("SERVER: %s", msg);
        NetplayDisconnectSession();
        break;
    }
    case ZNP_FRAME_BYE:
        NetplayDisconnectSession();
        set_status("PEER LEFT");
        break;
    default:
        /* ignore */
        break;
    }
    return 1;
}

void NetplayServiceTick(void)
{
    if (g_state != NS_WAITING && g_state != NS_READY)
        return;

    /* Drain any pending frames (PEER_READY, PONG, BYE, ...) non-blockingly. */
    while (io_recv_peek(&g_io, 0)) {
        if (!dispatch_one_frame(200))
            return;
    }

    /* Once paired, emit a heartbeat PING about once per second so the user can
     * see live RTT in the netplay screen and the server's idle-timeout never
     * fires while the GUI is open without a ROM loaded. */
    if (g_state == NS_READY) {
        u8 now = monotonic_us();
        if (!g_ping_in_flight && now >= g_ping_next_us) {
            u1 ts[8];
            for (int i = 7; i >= 0; i--) {
                ts[i] = (u1)(now & 0xFFu);
                now >>= 8;
            }
            if (frame_send(&g_io, ZNP_FRAME_PING, ts, 8)) {
                g_ping_sent_us = monotonic_us();
                g_ping_in_flight = 1;
                g_ping_next_us = g_ping_sent_us + 1000000ull;
            }
        }
        /* If a PING has been in flight for too long, mark RTT as unknown. */
        if (g_ping_in_flight && monotonic_us() - g_ping_sent_us > 3000000ull) {
            g_ping_in_flight = 0;
            g_rtt_ms = -1;
            update_ready_status();
        }
    }
}

/* ------- per-frame lockstep ------- */

static u4 wram_hash(void)
{
    if (wramdata == NULL)
        return 0;
    u4 h = 2166136261u;
    for (int i = 0; i < 256; i++) {
        h ^= (u4)wramdata[i];
        h *= 16777619u;
    }
    return h;
}

void NetplaySyncInputs(unsigned int* joy_a, unsigned int* joy_b)
{
    NetplayServiceTick();
    if (g_state != NS_READY)
        return;

    /* Both peers read their LOCAL pad from joy_a — ZSNES routes the active
     * keyboard/joypad to controller 1 by default, and forcing the client to
     * read from joy_b would silently drop the joiner's inputs (since most
     * setups don't have controller 2 mapped). The role only decides which
     * slot the local input ends up in for the engine, and which slot the
     * remote input fills. */
    u4 const raw = (u4)*joy_a;

    /* Input delay: hand the local pad's input to the engine NETPLAY_INPUT_DELAY
     * frames late, so the peer's packet for the same frame has time to land. */
    u4 delayed;
    if (g_input_queue_filled < NETPLAY_INPUT_DELAY) {
        delayed = 0x00008000u; /* neutral */
        g_input_queue_filled++;
    } else {
        delayed = g_input_queue[g_input_queue_pos];
    }
    g_input_queue[g_input_queue_pos] = raw;
    g_input_queue_pos = (g_input_queue_pos + 1) % NETPLAY_INPUT_DELAY;

    if (g_role == ZNP_ROLE_HOST)
        *joy_a = delayed;
    else
        *joy_b = delayed;

    u4 const local_crc = wram_hash();

    NetplayInputPacket local;
    local.magic = hto32(ZNP_INPUT_MAGIC);
    local.seq = hto32(g_local_seq);
    local.joy = hto32(delayed);
    local.crc = hto32(local_crc);
    g_local_seq++;

    int const timeout = NETPLAY_INPUT_DELAY * NETPLAY_FRAME_MS;
    if (!frame_send(&g_io, ZNP_FRAME_INPUT, &local, sizeof(local))) {
        NetplayDisconnectSession();
        set_status("CONNECTION LOST");
        return;
    }

    /* Receive remote frames until we get one INPUT — silently absorb any
     * BYE/error/peer-ready frames so the loop is robust to interleaving. */
    for (;;) {
        u2 type = 0, len = 0;
        u1 buf[ZNP_FRAME_MAX_PAYLOAD];
        if (!frame_recv(&g_io, &type, buf, sizeof(buf), &len, timeout)) {
            NetplayDisconnectSession();
            set_status("CONNECTION LOST");
            return;
        }
        if (type == ZNP_FRAME_INPUT) {
            if (len != sizeof(NetplayInputPacket))
                continue;
            NetplayInputPacket remote;
            memcpy(&remote, buf, sizeof(remote));
            remote.magic = toh32(remote.magic);
            remote.seq = toh32(remote.seq);
            remote.joy = toh32(remote.joy);
            remote.crc = toh32(remote.crc);
            if (remote.magic != ZNP_INPUT_MAGIC)
                continue;
            if (local_crc != 0 && remote.crc != 0 && remote.crc != local_crc && g_local_seq > NETPLAY_INPUT_DELAY + 2) {
                set_status("DESYNC DETECTED");
            }
            if (g_role == ZNP_ROLE_HOST)
                *joy_b = remote.joy;
            else
                *joy_a = remote.joy;
            return;
        }
        if (type == ZNP_FRAME_BYE) {
            NetplayDisconnectSession();
            set_status("PEER LEFT");
            return;
        }
        if (type == ZNP_FRAME_SERVER_ERROR) {
            char msg[64] = "";
            size_t take = len < sizeof(msg) - 1 ? len : sizeof(msg) - 1;
            memcpy(msg, buf, take);
            set_statusf("SERVER: %s", msg);
            NetplayDisconnectSession();
            return;
        }
        /* Ignore PEER_READY echoes / PONG / unknown — keep waiting for INPUT. */
    }
}

#else /* !__UNIXSDL__ — non-Linux stubs */

char NetplayHostName[32] = "127.0.0.1";
char* GUINetplayTextPtr[1] = { NetplayHostName };
u1 NetplayTLSConfig = 1;

static char const* const g_status_stub = "UNSUPPORTED ON THIS PORT";

void NetplayJoinSession(void) { }
void NetplayDisconnectSession(void) { }
void NetplaySyncInputs(unsigned int* joy_a, unsigned int* joy_b)
{
    (void)joy_a;
    (void)joy_b;
}
void NetplayServiceTick(void) { }
char const* NetplayStatusText(void) { return g_status_stub; }

#endif
