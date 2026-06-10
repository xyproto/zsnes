/*
 * Netplay unit tests
 *
 * Covers: packet byte-swap roundtrip, magic constant, FNV1a hash,
 * UDP loopback send/recv, TCP loopback send/recv, desync detection logic.
 *
 * All logic is self-contained — no ZSNES object files needed.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "zstest.h"

/* Types mirroring c_guiwindp.c */

typedef struct {
    uint32_t magic;
    uint32_t seq;
    uint32_t joy;
    uint32_t crc;
} Packet;

static const uint32_t NETP_MAGIC = 0x4E455450u; /* "NETP" */
static const uint16_t TEST_PORT_UDP = 17845;
static const uint16_t TEST_PORT_TCP = 17846;

static void pkt_hton(Packet* p)
{
    p->magic = htonl(p->magic);
    p->seq = htonl(p->seq);
    p->joy = htonl(p->joy);
    p->crc = htonl(p->crc);
}

static void pkt_ntoh(Packet* p)
{
    p->magic = ntohl(p->magic);
    p->seq = ntohl(p->seq);
    p->joy = ntohl(p->joy);
    p->crc = ntohl(p->crc);
}

static uint32_t fnv1a(uint8_t const* data, int len)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < len; i++) {
        h ^= (uint32_t)data[i];
        h *= 16777619u;
    }
    return h;
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

static void test_packet_byteswap(void)
{
    ZT_SECTION("packet byte-swap roundtrip");

    Packet p = { NETP_MAGIC, 42, 0xDEADBEEFu, 0xCAFEBABEu };
    Packet orig = p;

    pkt_hton(&p);
    pkt_ntoh(&p);

    ZT_CHECK(p.magic == orig.magic);
    ZT_CHECK(p.seq == orig.seq);
    ZT_CHECK(p.joy == orig.joy);
    ZT_CHECK(p.crc == orig.crc);
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

    Packet wire;
    ssize_t n = recv(fd, &wire, sizeof(wire), 0);
    close(fd);
    if (n != (ssize_t)sizeof(wire)) {
        r->ok = 0;
        return NULL;
    }

    pkt_ntoh(&wire);
    r->received = wire;
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

    usleep(20000); /* 20 ms – give server time to bind */

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

    Packet pkt = { NETP_MAGIC, 7, 0x00008000u, 0xDEADu };
    Packet wire = pkt;
    pkt_hton(&wire);
    sendto(fd, &wire, sizeof(wire), 0, (struct sockaddr*)&dst, sizeof(dst));
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

    Packet wire;
    if (!recv_exact(cfd, &wire, sizeof(wire))) {
        close(cfd);
        r->ok = 0;
        return NULL;
    }
    close(cfd);
    pkt_ntoh(&wire);
    r->received = wire;
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

    usleep(20000);

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

    Packet pkt = { NETP_MAGIC, 99, 0xFFFF0000u, 0xBEEFu };
    Packet wire = pkt;
    pkt_hton(&wire);
    send_exact(fd, &wire, sizeof(wire));
    close(fd);

    pthread_join(srv, NULL);

    ZT_CHECK(result.ok == 1);
    ZT_CHECK(result.received.magic == NETP_MAGIC);
    ZT_CHECK(result.received.seq == 99);
    ZT_CHECK(result.received.joy == 0xFFFF0000u);
    ZT_CHECK(result.received.crc == 0xBEEFu);
}

static void test_packet_layout(void)
{
    ZT_SECTION("packet struct layout");

    /* 4 x uint32_t, no padding */
    ZT_CHECK_INT((int)sizeof(Packet), 16);

    /* each field is 4 bytes at the expected offset */
    Packet p;
    ZT_CHECK_INT((int)((char*)&p.magic - (char*)&p), 0);
    ZT_CHECK_INT((int)((char*)&p.seq - (char*)&p), 4);
    ZT_CHECK_INT((int)((char*)&p.joy - (char*)&p), 8);
    ZT_CHECK_INT((int)((char*)&p.crc - (char*)&p), 12);
}

/* Relay-protocol framing tests — these mirror the on-wire layout the C
 * client (linux/netplay.c) and the Go server (server/main.go) both use. */

#define ZNP_FRAME_MAX_PAYLOAD 4096
#define ZNP_ROOM_CODE_LEN 16
#define ZNP_PASSWORD_LEN 32
#define ZNP_NICKNAME_LEN 16
#define ZNP_PROTOCOL_VERSION 1u

#define ZNP_FRAME_CLIENT_HELLO 0x0001u
#define ZNP_FRAME_SERVER_HELLO 0x0002u
#define ZNP_FRAME_INPUT 0x0010u
#define ZNP_FRAME_PING 0x0020u
#define ZNP_FRAME_BYE 0x00FFu

#define ZNP_HELLO_MODE_CREATE 1u

/* Encode a frame header into 4 bytes, big-endian. Returns 1 on success. */
static int frame_hdr_encode(uint8_t out[4], uint16_t typ, uint16_t length)
{
    if (length > ZNP_FRAME_MAX_PAYLOAD)
        return 0;
    out[0] = (uint8_t)(typ >> 8);
    out[1] = (uint8_t)(typ & 0xFF);
    out[2] = (uint8_t)(length >> 8);
    out[3] = (uint8_t)(length & 0xFF);
    return 1;
}

static int frame_hdr_decode(uint8_t const in[4], uint16_t* typ, uint16_t* length)
{
    *typ = ((uint16_t)in[0] << 8) | in[1];
    *length = ((uint16_t)in[2] << 8) | in[3];
    if (*length > ZNP_FRAME_MAX_PAYLOAD)
        return 0;
    return 1;
}

static void test_frame_header_layout(void)
{
    ZT_SECTION("frame header layout (big-endian, 4 bytes)");

    uint8_t hdr[4];
    ZT_CHECK(frame_hdr_encode(hdr, ZNP_FRAME_INPUT, 16) == 1);
    ZT_CHECK_INT(hdr[0], 0x00);
    ZT_CHECK_INT(hdr[1], 0x10);
    ZT_CHECK_INT(hdr[2], 0x00);
    ZT_CHECK_INT(hdr[3], 0x10);

    uint16_t typ, length;
    ZT_CHECK(frame_hdr_decode(hdr, &typ, &length) == 1);
    ZT_CHECK_INT(typ, ZNP_FRAME_INPUT);
    ZT_CHECK_INT(length, 16);

    /* BYE: type=0x00FF, length=0 */
    ZT_CHECK(frame_hdr_encode(hdr, ZNP_FRAME_BYE, 0) == 1);
    ZT_CHECK_INT(hdr[0], 0x00);
    ZT_CHECK_INT(hdr[1], 0xFF);
    ZT_CHECK_INT(hdr[2], 0x00);
    ZT_CHECK_INT(hdr[3], 0x00);
}

static void test_frame_oversize_rejected(void)
{
    ZT_SECTION("frame header rejects oversize payload");

    uint8_t hdr[4];
    ZT_CHECK(frame_hdr_encode(hdr, ZNP_FRAME_INPUT, ZNP_FRAME_MAX_PAYLOAD) == 1);
    ZT_CHECK(frame_hdr_encode(hdr, ZNP_FRAME_INPUT, ZNP_FRAME_MAX_PAYLOAD + 1) == 0);

    /* A bogus length on the read side must also fail. */
    hdr[2] = 0xFF;
    hdr[3] = 0xFF;
    uint16_t typ, length;
    ZT_CHECK(frame_hdr_decode(hdr, &typ, &length) == 0);
}

/* CLIENT_HELLO payload layout: u32 proto, u8 mode, char room[16],
 * char password[32], char nickname[16]. Total = 4+1+16+32+16 = 69 bytes. */
static void encode_hello(uint8_t* out, uint8_t mode,
    char const* room, char const* password, char const* nick)
{
    uint32_t proto = htonl(ZNP_PROTOCOL_VERSION);
    memcpy(out, &proto, 4);
    out[4] = mode;
    memset(out + 5, 0, ZNP_ROOM_CODE_LEN);
    memset(out + 5 + ZNP_ROOM_CODE_LEN, 0, ZNP_PASSWORD_LEN);
    memset(out + 5 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN, 0, ZNP_NICKNAME_LEN);
    size_t n = strlen(room);
    if (n > ZNP_ROOM_CODE_LEN)
        n = ZNP_ROOM_CODE_LEN;
    memcpy(out + 5, room, n);
    n = strlen(password);
    if (n > ZNP_PASSWORD_LEN)
        n = ZNP_PASSWORD_LEN;
    memcpy(out + 5 + ZNP_ROOM_CODE_LEN, password, n);
    n = strlen(nick);
    if (n > ZNP_NICKNAME_LEN)
        n = ZNP_NICKNAME_LEN;
    memcpy(out + 5 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN, nick, n);
}

static void test_hello_payload_layout(void)
{
    ZT_SECTION("CLIENT_HELLO payload layout");

    enum { HELLO_SZ = 4 + 1 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN + ZNP_NICKNAME_LEN };
    ZT_CHECK_INT(HELLO_SZ, 69);

    uint8_t buf[HELLO_SZ];
    encode_hello(buf, ZNP_HELLO_MODE_CREATE, "lobby", "secret", "alice");

    /* protocol version is BE-1 */
    uint32_t proto;
    memcpy(&proto, buf, 4);
    ZT_CHECK_INT((int)ntohl(proto), (int)ZNP_PROTOCOL_VERSION);

    /* mode byte */
    ZT_CHECK_INT(buf[4], ZNP_HELLO_MODE_CREATE);

    /* room is at offset 5, NUL-padded */
    ZT_CHECK(strncmp((char const*)buf + 5, "lobby", 5) == 0);
    ZT_CHECK(buf[5 + 5] == 0);

    /* password at offset 5+16 = 21 */
    ZT_CHECK(strncmp((char const*)buf + 21, "secret", 6) == 0);
    ZT_CHECK(buf[21 + 6] == 0);

    /* nickname at offset 5+16+32 = 53 */
    ZT_CHECK(strncmp((char const*)buf + 53, "alice", 5) == 0);
    ZT_CHECK(buf[53 + 5] == 0);
}

static void test_hello_truncates_overlong_fields(void)
{
    ZT_SECTION("CLIENT_HELLO truncates overlong strings");

    enum { HELLO_SZ = 4 + 1 + ZNP_ROOM_CODE_LEN + ZNP_PASSWORD_LEN + ZNP_NICKNAME_LEN };
    uint8_t buf[HELLO_SZ];
    /* 20-char room name (> ZNP_ROOM_CODE_LEN=16) */
    encode_hello(buf, ZNP_HELLO_MODE_CREATE,
        "abcdefghijklmnopqrst", "supersecretpasswordsupersecretpasswordmore",
        "a-very-long-nickname-indeed");

    /* room field holds exactly 16 chars, no terminator inside it */
    ZT_CHECK(memcmp(buf + 5, "abcdefghijklmnop", 16) == 0);
    /* the byte immediately after the room field is the start of the password
     * field, so it should hold "supe..." rather than any leaked nul */
    ZT_CHECK(buf[5 + 16] == 's');

    /* password field is exactly 32 chars */
    ZT_CHECK(memcmp(buf + 21, "supersecretpasswordsupersecretpa", 32) == 0);

    /* nickname field is exactly 16 chars */
    ZT_CHECK(memcmp(buf + 53, "a-very-long-nick", 16) == 0);
}

/* Stream prefix must be exactly the 4 ASCII bytes "ZNP1". */
static void test_stream_prefix(void)
{
    ZT_SECTION("stream prefix is ASCII 'ZNP1'");
    char const* p = "ZNP1";
    ZT_CHECK(p[0] == 'Z');
    ZT_CHECK(p[1] == 'N');
    ZT_CHECK(p[2] == 'P');
    ZT_CHECK(p[3] == '1');
}

/* Send a length-prefixed frame on a TCP loopback, read it back, and verify
 * the full structure decodes the same way the relay would see it. */
static void test_framed_tcp_loopback(void)
{
    ZT_SECTION("framed TCP loopback (write/read of one INPUT frame)");

    int lfd = make_tcp_listener(17847);
    if (lfd < 0) {
        fprintf(stderr, "    SKIP: listener failed\n");
        return;
    }

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0) {
        close(lfd);
        fprintf(stderr, "    SKIP: client socket failed\n");
        return;
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(17847);
    if (connect(cfd, (struct sockaddr*)&a, sizeof(a)) != 0) {
        close(cfd);
        close(lfd);
        fprintf(stderr, "    SKIP: connect failed\n");
        return;
    }

    int afd = accept(lfd, NULL, NULL);
    close(lfd);
    if (afd < 0) {
        close(cfd);
        fprintf(stderr, "    SKIP: accept failed\n");
        return;
    }

    /* Write: 4-byte header + 16-byte INPUT payload. */
    uint8_t wire[4 + 16];
    frame_hdr_encode(wire, ZNP_FRAME_INPUT, 16);
    Packet payload = { NETP_MAGIC, 7, 0xAA55AA55u, 0x12345678u };
    Packet wirep = payload;
    pkt_hton(&wirep);
    memcpy(wire + 4, &wirep, 16);
    ZT_CHECK(send_exact(cfd, wire, sizeof(wire)));

    /* Read back at the other end. */
    uint8_t read_hdr[4];
    ZT_CHECK(recv_exact(afd, read_hdr, 4));
    uint16_t typ, length;
    ZT_CHECK(frame_hdr_decode(read_hdr, &typ, &length) == 1);
    ZT_CHECK_INT(typ, ZNP_FRAME_INPUT);
    ZT_CHECK_INT(length, 16);

    Packet read_pkt;
    ZT_CHECK(recv_exact(afd, &read_pkt, sizeof(read_pkt)));
    pkt_ntoh(&read_pkt);
    ZT_CHECK_INT((int)read_pkt.magic, (int)NETP_MAGIC);
    ZT_CHECK_INT((int)read_pkt.seq, 7);
    ZT_CHECK((unsigned)read_pkt.joy == 0xAA55AA55u);
    ZT_CHECK((unsigned)read_pkt.crc == 0x12345678u);

    close(afd);
    close(cfd);
}

/* Entry point */

int main(void)
{
    printf("ZSNES2 netplay tests\n");

    test_packet_layout();
    test_magic_bytes();
    test_packet_byteswap();
    test_fnv1a();
    test_desync_detection();
    test_udp_loopback();
    test_tcp_loopback();

    test_stream_prefix();
    test_frame_header_layout();
    test_frame_oversize_rejected();
    test_hello_payload_layout();
    test_hello_truncates_overlong_fields();
    test_framed_tcp_loopback();

    ZT_RESULTS();
}
