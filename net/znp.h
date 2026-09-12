/* ZNP1: the framing the relay in server/ speaks, for when neither player is
 * reachable from the internet. Both connect out and name the same room.
 *
 * Frames are `u16 type, u16 length, payload`, big-endian, after a one-time
 * "ZNP1" prefix each way. An input frame carries net/packet.c's encoding, so
 * the emulator has one wire format either way. Protocol only; net/netplay.c
 * runs the session. */

#ifndef ZSNES_NET_ZNP_H
#define ZSNES_NET_ZNP_H

#include <stddef.h>
#include <stdint.h>

#include "transport.h"

enum { ZNP_PREFIX_BYTES = 4,
    ZNP_VERSION = 1,
    ZNP_MAX_PAYLOAD = 4096,
    ZNP_ROOM_BYTES = 16,
    ZNP_PASSWORD_BYTES = 32,
    ZNP_NICK_BYTES = 16,
    ZNP_HELLO_BYTES = 4 + 1 + ZNP_ROOM_BYTES + ZNP_PASSWORD_BYTES + ZNP_NICK_BYTES,
    ZNP_SERVER_HELLO_BYTES = 4 + 1 + ZNP_ROOM_BYTES,
    ZNP_DEFAULT_PORT = 7845 };

enum { ZNP_CLIENT_HELLO = 0x0001,
    ZNP_SERVER_HELLO = 0x0002,
    ZNP_PEER_READY = 0x0003,
    ZNP_SERVER_ERROR = 0x0004,
    ZNP_INPUT = 0x0010,
    ZNP_PING = 0x0020,
    ZNP_PONG = 0x0021,
    ZNP_BYE = 0x00FF };

/* CREATE also joins a room that already exists. */
enum { ZNP_MODE_CREATE = 1,
    ZNP_MODE_JOIN = 2 };

/* Which pad the session's own input ends up on. */
enum { ZNP_ROLE_HOST = 1,
    ZNP_ROLE_CLIENT = 2 };

extern char const znp_prefix[ZNP_PREFIX_BYTES]; /* "ZNP1", no terminator */

/* "host[:port][/room]", each part defaulting. An IPv6 literal needs brackets
   to carry a port, since "::1" is all address. */
void znp_parse_target(char const* spec, char* host, size_t host_sz,
    uint16_t* port, char* room, size_t room_sz);

/* Fixed-width NUL-padded fields; over-long strings are cut. */
void znp_hello_encode(uint8_t out[ZNP_HELLO_BYTES], unsigned mode,
    char const* room, char const* password, char const* nick);

/* Zero when the bytes are not a hello this client understands. */
int znp_server_hello_decode(uint8_t const* in, size_t len, unsigned* role,
    char* room, size_t room_sz);

/* One frame, within the deadline. Non-zero on success. A payload too big for
   the buffer fails: truncating would leave its tail to be read as a header. */
int znp_frame_send(NetSocket s, unsigned type, void const* payload, size_t len,
    int timeout_ms);
int znp_frame_recv(NetSocket s, unsigned* type, void* payload, size_t cap,
    size_t* len, int timeout_ms);

int znp_prefix_exchange(NetSocket s, int timeout_ms);

#endif
