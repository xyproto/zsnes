/* ZNP1: the framing the relay in server/ speaks.
 *
 * Peer-to-peer netplay needs one side to be reachable from the internet. The
 * relay exists for when neither is: both players connect out to it, name the
 * same room, and it forwards their input frames. This file is the client half
 * of that conversation - the framing and the handshake, nothing above it, so
 * net/netplay.c decides what to do with a session and this decides what the
 * bytes look like.
 *
 * Frames are `u16 type, u16 length, payload`, big-endian, after a one-time
 * "ZNP1" prefix in each direction. The input frame's payload is the same
 * net/packet.c encoding used peer-to-peer, so the emulator has one wire
 * format either way and the relay only checks its magic. */

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

/* The relay creates a room that does not exist and joins one that does, so a
   client that does not care which it is asks for CREATE. */
enum { ZNP_MODE_CREATE = 1,
    ZNP_MODE_JOIN = 2 };

/* Which pad the session's own input ends up on. */
enum { ZNP_ROLE_HOST = 1,
    ZNP_ROLE_CLIENT = 2 };

extern char const znp_prefix[ZNP_PREFIX_BYTES]; /* "ZNP1", no terminator */

/* "host[:port][/room]" split into its parts, each with a default: this
   machine, ZNP_DEFAULT_PORT, room "default". A colon is only a port when
   digits follow it, so "::1" stays an address. */
void znp_parse_target(char const* spec, char* host, size_t host_sz,
    uint16_t* port, char* room, size_t room_sz);

/* Fixed-width NUL-padded fields, which is why the room and nick are copied
   rather than pointed at. Over-long strings are truncated. */
void znp_hello_encode(uint8_t out[ZNP_HELLO_BYTES], unsigned mode,
    char const* room, char const* password, char const* nick);

/* Returns zero when the bytes are not a server hello, including a version the
   relay answered with that this client does not know. */
int znp_server_hello_decode(uint8_t const* in, size_t len, unsigned* role,
    char* room, size_t room_sz);

/* One frame on or off a stream socket, within the deadline. Non-zero on
   success; a payload longer than the caller's buffer fails rather than
   truncating, since the rest of the frame would then be read as a header. */
int znp_frame_send(NetSocket s, unsigned type, void const* payload, size_t len,
    int timeout_ms);
int znp_frame_recv(NetSocket s, unsigned* type, void* payload, size_t cap,
    size_t* len, int timeout_ms);

/* Prefix exchange, both directions. */
int znp_prefix_exchange(NetSocket s, int timeout_ms);

#endif
