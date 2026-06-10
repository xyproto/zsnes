/*
 * ZSNES netplay client — header.
 *
 * The client connects to a Go relay server (see server/main.go), exchanges a
 * room-code + password handshake, and from there forwards/receives lockstep
 * input packets through the relay. Transport is TLS-on-TCP when the build has
 * OpenSSL (WITH_TLS=yes); otherwise plain TCP is used (development only).
 *
 * Public API:
 *   - NetplayHostSession()         — create a room and wait for a peer.
 *   - NetplayJoinSession()         — join an existing room.
 *   - NetplayDisconnectSession()   — tear down the session.
 *   - NetplaySyncInputs(a, b)      — exchange one frame of input. Called by
 *                                    the emulator's input loop.
 *
 * Configuration is read from globals (set by the GUI) and from environment:
 *   NetplayHostName        — "host[:port][/room]"  (e.g. "relay.example.com:7845/mygame")
 *   NetplayTLSConfig       — 1 = TLS, 0 = plaintext (default 1)
 *   ZSNES_NETPLAY_PIN      — hex SHA-256 of server cert SPKI; required for TLS
 *                            verification unless ZSNES_NETPLAY_INSECURE=1
 *   ZSNES_NETPLAY_INSECURE — "1" disables certificate verification (testing)
 *   ZSNES_NETPLAY_PASSWORD — optional shared/room password
 *   ZSNES_NETPLAY_NICK     — optional nickname (max 15 chars)
 */

#ifndef ZSNES_LINUX_NETPLAY_H
#define ZSNES_LINUX_NETPLAY_H

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Wire-protocol constants — must stay in sync with server/main.go. */
#define ZNP_STREAM_PREFIX "ZNP1"
#define ZNP_STREAM_PREFIX_LEN 4
#define ZNP_PROTOCOL_VERSION 1u
#define ZNP_FRAME_MAX_PAYLOAD 4096
#define ZNP_ROOM_CODE_LEN 16
#define ZNP_PASSWORD_LEN 32
#define ZNP_NICKNAME_LEN 16
#define ZNP_INPUT_MAGIC 0x4E455450u /* "NETP" */

#define ZNP_FRAME_CLIENT_HELLO 0x0001u
#define ZNP_FRAME_SERVER_HELLO 0x0002u
#define ZNP_FRAME_PEER_READY 0x0003u
#define ZNP_FRAME_SERVER_ERROR 0x0004u
#define ZNP_FRAME_INPUT 0x0010u
#define ZNP_FRAME_PING 0x0020u
#define ZNP_FRAME_PONG 0x0021u
#define ZNP_FRAME_BYE 0x00FFu

#define ZNP_HELLO_MODE_CREATE 1u
#define ZNP_HELLO_MODE_JOIN 2u

#define ZNP_ROLE_HOST 1u
#define ZNP_ROLE_CLIENT 2u

#define ZNP_DEFAULT_PORT 7845

/* Same struct used pre- and post-server-refactor; preserved so the existing
 * netplay tests in test/netplay_test.c still cover the on-wire layout. */
typedef struct {
    u4 magic;
    u4 seq;
    u4 joy;
    u4 crc;
} NetplayInputPacket;

void NetplayJoinSession(void);
void NetplayDisconnectSession(void);
void NetplaySyncInputs(unsigned int* joy_a, unsigned int* joy_b);

/* Status accessors used by the GUI. */
char const* NetplayStatusText(void);
void NetplayServiceTick(void);

#ifdef __cplusplus
}
#endif

#endif /* ZSNES_LINUX_NETPLAY_H */
