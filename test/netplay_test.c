// Looks good
/*
 * Netplay unit tests
 *
 * Covers: packet byte-swap roundtrip, magic constant, FNV1a hash,
 * UDP loopback send/recv, TCP loopback send/recv, desync detection logic.
 *
 * The packet codec is the emulator's own (net/packet.c), which is why that
 * file exists: this test used to carry its own copy of the struct and the
 * byte-swapping, and had drifted a field behind the real one.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "zstest.h"

/* usleep is gone from POSIX.1-2008; nanosleep replaced it. */
static void zsleep_us(unsigned int const usec)
{
    struct timespec ts;

    ts.tv_sec = (time_t)(usec / 1000000u);
    ts.tv_nsec = (long)(usec % 1000000u) * 1000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
    }
}

#include "../net/packet.h"

typedef NetplayPacket Packet;

static const uint32_t NETP_MAGIC = NETPLAY_MAGIC;
static const uint16_t TEST_PORT_UDP = 17845;
static const uint16_t TEST_PORT_TCP = 17846;

/* The tests below send a decoded packet and read one back; on the wire it is
   always NETPLAY_PACKET_BYTES big-endian bytes. */
static void pkt_encode(uint8_t out[NETPLAY_PACKET_BYTES], Packet const* p)
{
    netplay_packet_encode(out, p);
}

static int pkt_decode(Packet* out, uint8_t const in[NETPLAY_PACKET_BYTES])
{
    return netplay_packet_decode(out, in);
}

static uint32_t fnv1a(uint8_t const* data, int len)
{
    return netplay_fnv1a(data, (size_t)len);
}

/* Helpers */

static int make_udp_sock(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;
    if (port != 0) {
        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        struct sockaddr_in a;
        memset(&a, 0, sizeof(a));
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons(port);
        if (bind(fd, (struct sockaddr*)&a, sizeof(a)) != 0) {
            close(fd);
            return -1;
        }
    }
    return fd;
}

static int make_tcp_listener(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&a, sizeof(a)) != 0 || listen(fd, 1) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

/* Send all bytes (blocking). Returns 1 on success. */
static int send_exact(int fd, void const* buf, size_t n)
{
    char const* p = buf;
    size_t sent = 0;
    while (sent < n) {
        ssize_t r = send(fd, p + sent, n - sent, MSG_NOSIGNAL);
        if (r <= 0)
            return 0;
        sent += (size_t)r;
    }
    return 1;
}

/* Recv all bytes (blocking). Returns 1 on success. */
static int recv_exact(int fd, void* buf, size_t n)
{
    char* p = buf;
    size_t got = 0;
    while (got < n) {
        ssize_t r = recv(fd, p + got, n - got, 0);
        if (r <= 0)
            return 0;
        got += (size_t)r;
    }
    return 1;
}

/* Tests */

static void test_packet_roundtrip(void)
{
    ZT_SECTION("packet survives a trip through the wire format");

    Packet const orig = { NETP_MAGIC, 0x11223344u, 42, 0xDEADBEEFu, 0xCAFEBABEu };
    uint8_t wire[NETPLAY_PACKET_BYTES];
    Packet back;

    pkt_encode(wire, &orig);
    ZT_CHECK(pkt_decode(&back, wire));
    ZT_CHECK(back.magic == orig.magic);
    ZT_CHECK(back.session == orig.session);
    ZT_CHECK(back.seq == orig.seq);
    ZT_CHECK(back.joy == orig.joy);
    ZT_CHECK(back.crc == orig.crc);
}

static void test_packet_wire_layout(void)
{
    ZT_SECTION("wire layout is five big-endian words");

    Packet const p = { NETP_MAGIC, 0x01020304u, 0x05060708u, 0x090A0B0Cu,
        0x0D0E0F10u };
    uint8_t wire[NETPLAY_PACKET_BYTES];
    uint8_t const expect[NETPLAY_PACKET_BYTES] = { 'N', 'E', 'T', 'P',
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };

    pkt_encode(wire, &p);
    ZT_CHECK(memcmp(wire, expect, sizeof(expect)) == 0);
}

static void test_packet_rejects_rubbish(void)
{
    ZT_SECTION("a packet that is not ours is refused");

    uint8_t wire[NETPLAY_PACKET_BYTES];
    Packet out;
    unsigned i;

    /* Anything at all on the port: the decoder has to say no rather than hand
       back a packet built from someone else's datagram. */
    memset(wire, 0, sizeof(wire));
    ZT_CHECK(!pkt_decode(&out, wire));

    for (i = 0; i < sizeof(wire); i++) {
        wire[i] = (uint8_t)(i * 37u + 11u);
    }
    ZT_CHECK(!pkt_decode(&out, wire));

    /* One bit off in the magic is still not ours. */
    {
        Packet const good = { NETP_MAGIC, 1, 2, 3, 4 };

        pkt_encode(wire, &good);
        ZT_CHECK(pkt_decode(&out, wire));
        wire[3] ^= 0x01u;
        ZT_CHECK(!pkt_decode(&out, wire));
    }
}

static void test_handshake_recognised(void)
{
    ZT_SECTION("handshake is magic, session, neutral pad on frame zero");

    Packet p = { NETP_MAGIC, 0xABCDu, 0, NETPLAY_JOY_NEUTRAL, 0 };

    ZT_CHECK(netplay_packet_is_handshake(&p, 0xABCDu));
    ZT_CHECK(!netplay_packet_is_handshake(&p, 0xABCEu));

    p.seq = 1;
    ZT_CHECK(!netplay_packet_is_handshake(&p, 0xABCDu));
    p.seq = 0;
    p.joy = 0;
    ZT_CHECK(!netplay_packet_is_handshake(&p, 0xABCDu));
}

static void test_magic_bytes(void)
{
    ZT_SECTION("magic constant spells NETP");

    uint8_t bytes[4];
    uint32_t wire = htonl(NETP_MAGIC);
    memcpy(bytes, &wire, 4);

    ZT_CHECK(bytes[0] == 'N');
    ZT_CHECK(bytes[1] == 'E');
    ZT_CHECK(bytes[2] == 'T');
    ZT_CHECK(bytes[3] == 'P');
}

static void test_fnv1a(void)
{
    ZT_SECTION("FNV1a hash");

    /* empty input → FNV offset basis */
    ZT_CHECK(fnv1a(NULL, 0) == 2166136261u);

    /* single zero byte — precomputed: 2166136261 XOR 0 = 2166136261, * FNV_prime */
    uint8_t zero = 0;
    uint32_t expected_zero = (2166136261u ^ 0u) * 16777619u;
    ZT_CHECK(fnv1a(&zero, 1) == expected_zero);

    /* two identical buffers produce the same hash */
    uint8_t buf1[256], buf2[256];
    memset(buf1, 0xAB, sizeof(buf1));
    memset(buf2, 0xAB, sizeof(buf2));
    ZT_CHECK(fnv1a(buf1, 256) == fnv1a(buf2, 256));

    /* different buffers produce different hashes (collision is theoretically
     * possible but astronomically unlikely for these inputs) */
    uint8_t buf3[256];
    memset(buf3, 0xCD, sizeof(buf3));
    ZT_CHECK(fnv1a(buf1, 256) != fnv1a(buf3, 256));

    /* known value for "abc" → 0x1A47E90B (FNV1a-32) */
    uint8_t const abc[] = { 'a', 'b', 'c' };
    ZT_CHECK(fnv1a(abc, 3) == 0x1A47E90Bu);
}

static void test_desync_detection(void)
{
    ZT_SECTION("desync detection (hash mismatch)");

    uint8_t wram_a[256], wram_b[256];
    memset(wram_a, 0x11, sizeof(wram_a));
    memset(wram_b, 0x11, sizeof(wram_b));

    /* identical WRAM → no desync */
    ZT_CHECK(fnv1a(wram_a, 256) == fnv1a(wram_b, 256));

    /* one byte differs → desync */
    wram_b[128] ^= 1;
    ZT_CHECK(fnv1a(wram_a, 256) != fnv1a(wram_b, 256));

    /* zero CRC on either side → skip check (wramdata not yet loaded) */
    uint32_t local_crc = 0, remote_crc = fnv1a(wram_b, 256);
    int desync = (local_crc != 0 && remote_crc != 0 && local_crc != remote_crc);
    ZT_CHECK(!desync);

    local_crc = fnv1a(wram_a, 256);
    remote_crc = 0;
    desync = (local_crc != 0 && remote_crc != 0 && local_crc != remote_crc);
    ZT_CHECK(!desync);

    /* both non-zero and mismatched → desync */
    local_crc = fnv1a(wram_a, 256);
    remote_crc = fnv1a(wram_b, 256);
    desync = (local_crc != 0 && remote_crc != 0 && local_crc != remote_crc);
    ZT_CHECK(desync);
}

typedef struct {
    Packet received;
    int ok;
} UDPServerResult;

static void* udp_server_thread(void* arg)
{
    UDPServerResult* r = arg;
    int fd = make_udp_sock(TEST_PORT_UDP);
    if (fd < 0) {
        r->ok = 0;
        return NULL;
    }

    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t wire[NETPLAY_PACKET_BYTES];
    ssize_t n = recv(fd, wire, sizeof(wire), 0);
    close(fd);
    if (n != (ssize_t)sizeof(wire) || !pkt_decode(&r->received, wire)) {
        r->ok = 0;
        return NULL;
    }

    r->ok = 1;
    return NULL;
}

static void test_udp_loopback(void)
{
    ZT_SECTION("UDP loopback send/recv");

    UDPServerResult result = { .ok = -1 };
    pthread_t srv;
    if (pthread_create(&srv, NULL, udp_server_thread, &result) != 0) {
        fprintf(stderr, "    SKIP: pthread_create failed\n");
        return;
    }

    zsleep_us(20000); /* 20 ms – give server time to bind */

    int fd = make_udp_sock(0); /* ephemeral source port */
    if (fd < 0) {
        pthread_join(srv, NULL);
        fprintf(stderr, "    SKIP: client socket failed\n");
        return;
    }

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dst.sin_port = htons(TEST_PORT_UDP);

    Packet pkt = { NETP_MAGIC, 3, 7, NETPLAY_JOY_NEUTRAL, 0xDEADu };
    uint8_t wire[NETPLAY_PACKET_BYTES];

    pkt_encode(wire, &pkt);
    sendto(fd, wire, sizeof(wire), 0, (struct sockaddr*)&dst, sizeof(dst));
    close(fd);

    pthread_join(srv, NULL);

    ZT_CHECK(result.ok == 1);
    ZT_CHECK(result.received.magic == NETP_MAGIC);
    ZT_CHECK(result.received.seq == 7);
    ZT_CHECK(result.received.joy == 0x00008000u);
    ZT_CHECK(result.received.crc == 0xDEADu);
}

typedef struct {
    Packet received;
    int ok;
} TCPServerResult;

static void* tcp_server_thread(void* arg)
{
    TCPServerResult* r = arg;
    int lfd = make_tcp_listener(TEST_PORT_TCP);
    if (lfd < 0) {
        r->ok = 0;
        return NULL;
    }

    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(lfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int cfd = accept(lfd, NULL, NULL);
    close(lfd);
    if (cfd < 0) {
        r->ok = 0;
        return NULL;
    }

    setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t wire[NETPLAY_PACKET_BYTES];
    if (!recv_exact(cfd, wire, sizeof(wire))) {
        close(cfd);
        r->ok = 0;
        return NULL;
    }
    close(cfd);
    if (!pkt_decode(&r->received, wire)) {
        r->ok = 0;
        return NULL;
    }
    r->ok = 1;
    return NULL;
}

static void test_tcp_loopback(void)
{
    ZT_SECTION("TCP loopback send/recv");

    TCPServerResult result = { .ok = -1 };
    pthread_t srv;
    if (pthread_create(&srv, NULL, tcp_server_thread, &result) != 0) {
        fprintf(stderr, "    SKIP: pthread_create failed\n");
        return;
    }

    zsleep_us(20000);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        pthread_join(srv, NULL);
        fprintf(stderr, "    SKIP: client socket failed\n");
        return;
    }

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dst.sin_port = htons(TEST_PORT_TCP);

    if (connect(fd, (struct sockaddr*)&dst, sizeof(dst)) != 0) {
        close(fd);
        pthread_join(srv, NULL);
        fprintf(stderr, "    SKIP: connect failed (errno %d)\n", errno);
        return;
    }

    Packet pkt = { NETP_MAGIC, 5, 99, 0xFFFF0000u, 0xBEEFu };
    uint8_t wire[NETPLAY_PACKET_BYTES];

    pkt_encode(wire, &pkt);
    send_exact(fd, wire, sizeof(wire));
    close(fd);

    pthread_join(srv, NULL);

    ZT_CHECK(result.ok == 1);
    ZT_CHECK(result.received.magic == NETP_MAGIC);
    ZT_CHECK(result.received.seq == 99);
    ZT_CHECK(result.received.joy == 0xFFFF0000u);
    ZT_CHECK(result.received.crc == 0xBEEFu);
}

/* Entry point */

int main(void)
{
    printf("ZSNES2 netplay tests\n");

    test_packet_wire_layout();
    test_magic_bytes();
    test_packet_roundtrip();
    test_packet_rejects_rubbish();
    test_handshake_recognised();
    test_fnv1a();
    test_desync_detection();
    test_udp_loopback();
    test_tcp_loopback();

    ZT_RESULTS();
}
