#include "packet.h"

static void put32(uint8_t* const p, uint32_t const v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static uint32_t get32(uint8_t const* const p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8
        | (uint32_t)p[3];
}

void netplay_packet_encode(uint8_t out[NETPLAY_PACKET_BYTES], NetplayPacket const* const p)
{
    put32(out + 0, p->magic);
    put32(out + 4, p->session);
    put32(out + 8, p->seq);
    put32(out + 12, p->joy);
    put32(out + 16, p->crc);
}

int netplay_packet_decode(NetplayPacket* const out, uint8_t const in[NETPLAY_PACKET_BYTES])
{
    out->magic = get32(in + 0);
    out->session = get32(in + 4);
    out->seq = get32(in + 8);
    out->joy = get32(in + 12);
    out->crc = get32(in + 16);
    return out->magic == NETPLAY_MAGIC;
}

int netplay_packet_is_handshake(NetplayPacket const* const p, uint32_t const session)
{
    return p->magic == NETPLAY_MAGIC && p->session == session && p->seq == 0;
}

NetplayPacket netplay_hello(uint32_t const session, uint32_t const game)
{
    NetplayPacket const p = { NETPLAY_MAGIC, session, 0, NETPLAY_PROTOCOL, game };

    return p;
}

int netplay_hello_verdict(NetplayPacket const* const p, uint32_t const game)
{
    if (p->joy != NETPLAY_PROTOCOL) {
        return NETPLAY_HELLO_VERSION;
    }
    return p->crc == game ? NETPLAY_HELLO_OK : NETPLAY_HELLO_GAME;
}

uint32_t netplay_fnv1a(void const* const data, size_t const len)
{
    uint8_t const* const p = (uint8_t const*)data;
    uint32_t h = 2166136261u;
    size_t i;

    for (i = 0; i < len; i++) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h;
}
