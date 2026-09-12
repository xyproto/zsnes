/* ZNP1 framing. See net/znp.h. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "znp.h"

char const znp_prefix[ZNP_PREFIX_BYTES] = { 'Z', 'N', 'P', '1' };

static void znp_put16(uint8_t* const out, unsigned const v)
{
    out[0] = (uint8_t)(v >> 8);
    out[1] = (uint8_t)v;
}

static unsigned znp_get16(uint8_t const* const in)
{
    return ((unsigned)in[0] << 8) | (unsigned)in[1];
}

static void znp_put32(uint8_t* const out, uint32_t const v)
{
    out[0] = (uint8_t)(v >> 24);
    out[1] = (uint8_t)(v >> 16);
    out[2] = (uint8_t)(v >> 8);
    out[3] = (uint8_t)v;
}

static uint32_t znp_get32(uint8_t const* const in)
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16)
        | ((uint32_t)in[2] << 8) | (uint32_t)in[3];
}

/* A fixed-width field: as much of `src` as fits, zeroes after it. */
static void znp_put_field(uint8_t* const out, size_t const width, char const* const src)
{
    size_t const n = src != NULL ? strlen(src) : 0;

    memset(out, 0, width);
    memcpy(out, src, n < width ? n : width);
}

/* And back out, stopping at the first zero. */
static void znp_get_field(char* const out, size_t const out_sz,
    uint8_t const* const in, size_t const width)
{
    size_t const room = out_sz > 0 ? out_sz - 1 : 0;
    size_t n = 0;

    while (n < width && n < room && in[n] != 0) {
        out[n] = (char)in[n];
        n++;
    }
    if (out_sz > 0) {
        out[n] = '\0';
    }
}

static int znp_all_digits(char const* p)
{
    if (*p == '\0') {
        return 0;
    }
    for (; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }
    return 1;
}

void znp_parse_target(char const* const spec, char* const host, size_t const host_sz,
    uint16_t* const port, char* const room, size_t const room_sz)
{
    char copy[64];
    char* slash;
    char* colon;

    snprintf(copy, sizeof(copy), "%s", spec != NULL ? spec : "");
    *port = (uint16_t)ZNP_DEFAULT_PORT;

    slash = strchr(copy, '/');
    if (slash != NULL) {
        *slash = '\0';
        snprintf(room, room_sz, "%s", slash + 1);
    }
    if (slash == NULL || room[0] == '\0') {
        snprintf(room, room_sz, "default");
    }

    /* An IPv6 literal is full of colons, so a port only follows one when the
       address is bracketed, or when there is a single colon to split on. */
    if (copy[0] == '[') {
        char* const close = strchr(copy, ']');

        colon = close != NULL ? close + 1 : NULL;
        if (close != NULL) {
            /* Drop the brackets. Only the address moves, so whatever follows
               the bracket - a port, or nothing - stays where it was. */
            *close = '\0';
            memmove(copy, copy + 1, strlen(copy + 1) + 1);
        }
    } else {
        colon = strchr(copy, ':');
        if (colon != NULL && strchr(colon + 1, ':') != NULL) {
            colon = NULL;
        }
    }
    if (colon != NULL && *colon == ':' && znp_all_digits(colon + 1)) {
        long const v = strtol(colon + 1, NULL, 10);

        *colon = '\0';
        if (v > 0 && v <= 65535) {
            *port = (uint16_t)v;
        }
    }
    snprintf(host, host_sz, "%s", copy[0] != '\0' ? copy : "127.0.0.1");
}

void znp_hello_encode(uint8_t out[ZNP_HELLO_BYTES], unsigned const mode,
    char const* const room, char const* const password, char const* const nick)
{
    znp_put32(out, (uint32_t)ZNP_VERSION);
    out[4] = (uint8_t)mode;
    znp_put_field(out + 5, ZNP_ROOM_BYTES, room);
    znp_put_field(out + 5 + ZNP_ROOM_BYTES, ZNP_PASSWORD_BYTES, password);
    znp_put_field(out + 5 + ZNP_ROOM_BYTES + ZNP_PASSWORD_BYTES, ZNP_NICK_BYTES, nick);
}

int znp_server_hello_decode(uint8_t const* const in, size_t const len,
    unsigned* const role, char* const room, size_t const room_sz)
{
    if (len < (size_t)ZNP_SERVER_HELLO_BYTES || znp_get32(in) != (uint32_t)ZNP_VERSION) {
        return 0;
    }
    if (in[4] != ZNP_ROLE_HOST && in[4] != ZNP_ROLE_CLIENT) {
        return 0;
    }
    *role = in[4];
    znp_get_field(room, room_sz, in + 5, ZNP_ROOM_BYTES);
    return 1;
}

int znp_frame_send(NetSocket const s, unsigned const type, void const* const payload,
    size_t const len, int const timeout_ms)
{
    uint8_t hdr[4];

    if (len > (size_t)ZNP_MAX_PAYLOAD) {
        return 0;
    }
    znp_put16(hdr, type);
    znp_put16(hdr + 2, (unsigned)len);
    if (!net_send_all(s, hdr, sizeof(hdr), timeout_ms)) {
        return 0;
    }
    return len == 0 || net_send_all(s, payload, len, timeout_ms);
}

int znp_frame_recv(NetSocket const s, unsigned* const type, void* const payload,
    size_t const cap, size_t* const len, int const timeout_ms)
{
    uint8_t hdr[4];
    size_t n;

    if (!net_recv_all(s, hdr, sizeof(hdr), timeout_ms)) {
        return 0;
    }
    n = (size_t)znp_get16(hdr + 2);
    if (n > (size_t)ZNP_MAX_PAYLOAD || n > cap) {
        return 0;
    }
    if (n != 0 && !net_recv_all(s, payload, n, timeout_ms)) {
        return 0;
    }
    *type = znp_get16(hdr);
    *len = n;
    return 1;
}

int znp_prefix_exchange(NetSocket const s, int const timeout_ms)
{
    char theirs[ZNP_PREFIX_BYTES];

    if (!net_send_all(s, znp_prefix, sizeof(znp_prefix), timeout_ms)) {
        return 0;
    }
    if (!net_recv_all(s, theirs, sizeof(theirs), timeout_ms)) {
        return 0;
    }
    return memcmp(theirs, znp_prefix, sizeof(theirs)) == 0;
}
