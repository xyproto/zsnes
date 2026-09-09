/* Netplay: the session state machine.
 *
 * This lived in gui/c_guiwindp.c, next to the code that draws the video panel,
 * which is why it could never be built or tested without a windowing system.
 * There is no platform code left in it: everything it needs from the machine
 * it runs on comes from net/transport.h, and unix/net_transport.c is the BSD
 * sockets implementation of that. The `__UNIXSDL__` guards below say only that
 * no other transport has been written yet, not that this file cares.
 *
 * The wire format is net/packet.c, which the tests link directly. */

#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <stdlib.h>

#include "../c_init.h"
#include "../cpu/execute.h"
#include "../gblhdr.h"
#include "../types.h"
#include "../ui.h"
#include "netplay.h"
#include "packet.h"
#include "transport.h"

enum {
    NETPLAY_IDLE = 0,
    NETPLAY_WAITING = 1,
    NETPLAY_CONNECTING = 2,
    NETPLAY_CONNECTED = 3,
    NETPLAY_JOINING = 4,
    NETPLAY_HANDSHAKING = 5
};

char NetplayStatusLine[64] = "IDLE";
static char NetplayLastEvent[64] = "";
#ifdef __UNIXSDL__
static int NetplayClientSocket = -1;
static int NetplayServerSocket = -1;
#endif
static u1 NetplaySessionState = NETPLAY_IDLE;
#ifdef __UNIXSDL__
static uint16_t const NetplayDefaultPort = 7845;
#endif
u1 NetplayHostRole = 0;
static u4 NetplayLocalSeq = 0;
static u4 NetplayRemoteSeq = 0;
static u1 NetplayRemoteSeqValid = 0;
static u4 NetplayRemoteJoy = 0x00008000;
static u4 NetplaySessionToken = 0;
u1 NetplayUDPConfig = 1;
char NetplayHostName[32] = "127.0.0.1";
#ifdef __UNIXSDL__
static u1 NetplayPendingRemoteValid = 0;
static u1 NetplayHandshakePending = 0;
static NetplayPacket NetplayPendingRemote;
#endif

#define NETPLAY_INPUT_DELAY 3
#define NETPLAY_FRAME_MS 17

#ifdef __UNIXSDL__
static u4 NetplayInputQueue[NETPLAY_INPUT_DELAY];
/* The peer's input, filed under the frame it is for rather than applied on
   whichever frame it happened to arrive. `seq` carries that frame number, so
   a packet that turns up early is held until its frame comes round and one
   that turns up twice is ignored the second time. Applying the peer's pad on
   arrival is what let the two machines drift apart while both believed they
   were in lockstep. */
enum { NETPLAY_RING = 64 };
static u4 NetplayRemoteRing[NETPLAY_RING];
static u4 NetplayRemoteRingCrc[NETPLAY_RING];
static u4 NetplayRemoteRingFrame[NETPLAY_RING];
static u1 NetplayRemoteRingValid[NETPLAY_RING];
static u4 NetplayFrame = 0;
static int NetplayInputQueuePos = 0;
static int NetplayInputQueueFilled = 0;
static int NetplayJoinIsUDP = 0;
static char NetplayJoinHostCopy[32] = "";
static uint64_t NetplayHandshakeDeadline = 0;

static int NetplaySendPacket(int const fd, NetplayPacket const* const packet, int timeout_ms)
{
    uint8_t wire[NETPLAY_PACKET_BYTES];

    netplay_packet_encode(wire, packet);
    if (NetplayUDPConfig == 0)
        return net_send_all(fd, wire, sizeof(wire), timeout_ms);
    if (net_wait(fd, 1, timeout_ms) <= 0)
        return 0;
    return net_send(fd, wire, sizeof(wire)) == (long)sizeof(wire);
}

static int NetplaySendPacketTo(NetSocket const fd, NetplayPacket const* const packet, NetAddr const* const to)
{
    uint8_t wire[NETPLAY_PACKET_BYTES];

    netplay_packet_encode(wire, packet);
    return net_send_to(fd, wire, sizeof(wire), to) == (long)sizeof(wire);
}

static int NetplayRecvPacket(int const fd, NetplayPacket* const packet, int timeout_ms)
{
    uint8_t wire[NETPLAY_PACKET_BYTES];

    if (NetplayUDPConfig == 0) {
        if (!net_recv_all(fd, wire, sizeof(wire), timeout_ms))
            return 0;
        netplay_packet_decode(packet, wire);
        return 1;
    }
    uint64_t const deadline = net_now_ms() + (uint64_t)(timeout_ms > 0 ? timeout_ms : 0);
    for (;;) {
        if (net_wait_until(fd, 0, deadline) <= 0)
            return 0;
        NetplayPacket tmp;
        long n = net_recv(fd, wire, sizeof(wire));
        if (n == (long)sizeof(wire)) {
            if (netplay_packet_decode(&tmp, wire)) {
                *packet = tmp;
                return 1;
            }
            continue;
        }
        if (n == 0)
            return 0;
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        return 0;
    }
}

static u4 NetplayStateHash(void)
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

#endif

#ifdef __UNIXSDL__
/* Everything that counts frames, cleared together whenever a session starts
   or ends. Missing one of these leaves a new session reading a stale frame's
   input out of the ring. */
static void NetplaySessionResetTiming(void)
{
    NetplayInputQueuePos = 0;
    NetplayInputQueueFilled = 0;
    NetplayFrame = 0;
    NetplayLocalSeq = 0;
    memset(NetplayRemoteRingValid, 0, sizeof(NetplayRemoteRingValid));
}
#endif

#ifdef __UNIXSDL__
/* Say hello, take the session token from the challenge, say hello again with
   it. Exactly what the connecting thread used to do, moved here because it is
   protocol rather than transport. */
static int NetplayJoinHandshake(NetSocket const fd)
{
    NetplayPacket hello;
    NetplayPacket challenge;

    int attempt;

    memset(&hello, 0, sizeof(hello));
    hello.magic = NETPLAY_MAGIC;
    hello.joy = NETPLAY_JOY_NEUTRAL;

    /* Say hello until answered. Over UDP the opening packet is the one most
       likely to go missing - the peer may not have finished binding - and
       sending it once meant a lost datagram ended the join outright, which
       showed up as an intermittent "HANDSHAKE FAILED" on loopback of all
       places. */
    for (attempt = 0; attempt < 5; attempt++) {
        if (NetplayJoinIsUDP != 0 && !NetplaySendPacket(fd, &hello, 1000)) {
            return 0;
        }
        if (NetplayRecvPacket(fd, &challenge, 400)
            && challenge.magic == NETPLAY_MAGIC && challenge.session != 0
            && challenge.seq == 0 && challenge.joy == NETPLAY_JOY_NEUTRAL
            && challenge.crc == 0) {
            break;
        }
        if (NetplayJoinIsUDP == 0) {
            return 0; /* a stream would have delivered it */
        }
    }
    if (attempt == 5) {
        return 0;
    }
    NetplaySessionToken = challenge.session;
    hello.session = NetplaySessionToken;
    return NetplaySendPacket(fd, &hello, 1000);
}
#endif

void NetplayDisconnectSession(void)
{
#ifdef __UNIXSDL__
    if (NetplayClientSocket >= 0) {
        net_close(NetplayClientSocket);
        NetplayClientSocket = -1;
    }
    if (NetplayServerSocket >= 0) {
        net_close(NetplayServerSocket);
        NetplayServerSocket = -1;
    }
    NetplaySessionResetTiming();
    NetplayLastEvent[0] = '\0';
    net_connect_cancel();
#endif
    NetplaySessionState = NETPLAY_IDLE;
    NetplayHostRole = 0;
    NetplayLocalSeq = 0;
    NetplayRemoteSeq = 0;
    NetplayRemoteSeqValid = 0;
    NetplayRemoteJoy = 0x00008000;
    NetplaySessionToken = 0;
#ifdef __UNIXSDL__
    NetplayPendingRemoteValid = 0;
    NetplayHandshakePending = 0;
#endif
}

void NetplayHostSession(void)
{
#ifndef __UNIXSDL__
    strcpy(NetplayStatusLine, "UNSUPPORTED ON THIS PORT");
    NetplaySessionState = NETPLAY_IDLE;
#else
    int const udp = NetplayUDPConfig != 0;
    NetSocket const fd = net_open(udp);

    if (fd == NET_SOCKET_NONE) {
        strcpy(NetplayLastEvent, "SERVER SOCKET FAILED");
        return;
    }
    if (!net_bind_any(fd, NetplayDefaultPort)) {
        net_close(fd);
        strcpy(NetplayLastEvent, "BIND FAILED");
        return;
    }

    NetplayDisconnectSession();
    NetplayHostRole = 1;
    NetplaySessionToken = net_random_u32();
    if (!udp) {
        if (!net_listen(fd, 1)) {
            net_close(fd);
            strcpy(NetplayLastEvent, "LISTEN FAILED");
            return;
        }
        NetplayServerSocket = fd;
    } else {
        NetplayClientSocket = fd;
    }
    NetplaySessionState = NETPLAY_WAITING;
    net_extip_start();
#endif
}

void NetplayJoinSession(void)
{
#ifndef __UNIXSDL__
    strcpy(NetplayLastEvent, "UNSUPPORTED ON THIS PORT");
    NetplaySessionState = NETPLAY_IDLE;
#else
    NetplayDisconnectSession();
    NetplayHostRole = 0;
    NetplayJoinIsUDP = NetplayUDPConfig != 0 ? 1 : 0;
    memcpy(NetplayJoinHostCopy, NetplayHostName, sizeof(NetplayJoinHostCopy));
    if (net_connect_start(NetplayJoinHostCopy, NetplayDefaultPort, NetplayJoinIsUDP)) {
        NetplaySessionState = NETPLAY_JOINING;
    } else {
        strcpy(NetplayLastEvent, "CONNECT FAILED");
    }
#endif
}

#ifdef __UNIXSDL__
/* Waiting for a peer, or reaching for one. */
static int NetplaySessionPending(void)
{
    return NetplaySessionState == NETPLAY_WAITING
        || NetplaySessionState == NETPLAY_CONNECTING
        || NetplaySessionState == NETPLAY_JOINING
        || NetplaySessionState == NETPLAY_HANDSHAKING;
}

/* Hold the emulator still until the session is up.
 *
 * Without this the host runs at full speed while it waits for someone to
 * join, so by the time a peer arrives the two machines are thousands of
 * frames apart with nothing to reconcile them - they exchange input happily
 * and show different games. The pause is released as soon as the session is
 * either connected or over, and a pause the player set themselves is left
 * alone: only a pause netplay applied is one netplay may lift.
 *
 * Emulation can be held here because the input path runs before the pause
 * check in c_cpuover, so this function keeps being called and the handshake
 * still makes progress. */
static u1 NetplayHeldEmulation;

static void NetplayHoldEmulation(int const hold)
{
    if (hold) {
        if (EMUPause == 0) {
            EMUPause = 1;
            NetplayHeldEmulation = 1;
        }
    } else if (NetplayHeldEmulation != 0) {
        EMUPause = 0;
        NetplayHeldEmulation = 0;
    }
}
#endif

void NetplayAdvanceState(int timeout_ms)
{
#if defined(__UNIXSDL__) && defined(ZSNES_DEBUG_HOOKS)
    /* Report every state change once. The status line only updates while the
       panel is on screen, so a headless run would otherwise leave no trace of
       whether the two sides ever paired. */
    {
        static int last = -1;
        static char last_event[64];

        if ((int)NetplaySessionState != last
            || strcmp(last_event, NetplayLastEvent) != 0) {
            last = (int)NetplaySessionState;
            snprintf(last_event, sizeof(last_event), "%s", NetplayLastEvent);
            fprintf(stderr, "NETPLAY state=%d role=%u seq=%u event=%s\n", last,
                (unsigned)NetplayHostRole, (unsigned)NetplayLocalSeq,
                NetplayLastEvent[0] ? NetplayLastEvent : "-");
        }
    }
#endif

#ifdef __UNIXSDL__
    NetplayHoldEmulation(NetplaySessionPending());
#endif
#ifdef __UNIXSDL__
    if (NetplaySessionState == NETPLAY_WAITING && NetplayUDPConfig == 0 && NetplayServerSocket >= 0) {
        if (net_wait(NetplayServerSocket, 0, timeout_ms) > 0) {
            NetSocket const fd = net_accept(NetplayServerSocket);
            if (fd != NET_SOCKET_NONE) {
                NetplayPacket challenge;
                memset(&challenge, 0, sizeof(challenge));
                challenge.magic = NETPLAY_MAGIC;
                challenge.session = NetplaySessionToken;
                challenge.joy = 0x00008000u;
                if (NetplaySendPacket(fd, &challenge, 1000)) {
                    NetplayClientSocket = fd;
                    NetplayHandshakeDeadline = net_now_ms() + 1000;
                    NetplaySessionState = NETPLAY_HANDSHAKING;
                    strcpy(NetplayLastEvent, "TCP CLIENT HANDSHAKING");
                } else {
                    close(fd);
                }
            }
        }
    }

    if (NetplaySessionState == NETPLAY_HANDSHAKING && NetplayClientSocket >= 0) {
        if (net_now_ms() >= NetplayHandshakeDeadline) {
            close(NetplayClientSocket);
            NetplayClientSocket = -1;
            NetplaySessionState = NETPLAY_WAITING;
            strcpy(NetplayLastEvent, "TCP HANDSHAKE TIMEOUT");
        } else if (net_wait(NetplayClientSocket, 0, timeout_ms) > 0) {
            NetplayPacket handshake;
            if (NetplayRecvPacket(NetplayClientSocket, &handshake, 1000) && netplay_packet_is_handshake(&handshake, NetplaySessionToken)) {
                NetplayPendingRemote = handshake;
                NetplayPendingRemoteValid = 1;
                NetplayHandshakePending = 1;
                NetplaySessionResetTiming();
                close(NetplayServerSocket);
                NetplayServerSocket = -1;
                NetplaySessionState = NETPLAY_CONNECTED;
                strcpy(NetplayLastEvent, "TCP CLIENT CONNECTED");
            } else {
                close(NetplayClientSocket);
                NetplayClientSocket = -1;
                NetplaySessionState = NETPLAY_WAITING;
                strcpy(NetplayLastEvent, "TCP HANDSHAKE FAILED");
            }
        }
    }

    if (NetplaySessionState == NETPLAY_WAITING && NetplayUDPConfig != 0 && NetplayClientSocket >= 0 && NetplayHostRole != 0) {
        if (net_wait(NetplayClientSocket, 0, timeout_ms) > 0) {
            NetAddr peer;
            NetplayPacket packet;
            uint8_t wire[NETPLAY_PACKET_BYTES];
            long n = net_recv_from(NetplayClientSocket, wire, sizeof(wire), &peer);
            if (n == (long)sizeof(wire)) {
                netplay_packet_decode(&packet, wire);
                if (netplay_packet_is_handshake(&packet, 0)) {
                    NetplayPacket challenge;
                    memset(&challenge, 0, sizeof(challenge));
                    challenge.magic = NETPLAY_MAGIC;
                    challenge.session = NetplaySessionToken;
                    challenge.joy = 0x00008000u;
                    NetplaySendPacketTo(NetplayClientSocket, &challenge, &peer);
                } else if (netplay_packet_is_handshake(&packet, NetplaySessionToken) && net_connect_addr(NetplayClientSocket, &peer)) {
                    NetplayPendingRemote = packet;
                    NetplayPendingRemoteValid = 1;
                    NetplayHandshakePending = 1;
                    NetplaySessionResetTiming();
                    NetplaySessionState = NETPLAY_CONNECTED;
                    strcpy(NetplayLastEvent, "UDP PEER CONNECTED");
                }
            }
        }
    }

    if (NetplaySessionState == NETPLAY_CONNECTING && NetplayClientSocket >= 0) {
        if (net_wait(NetplayClientSocket, 1, timeout_ms) > 0) {
            if (net_connect_pending_ok(NetplayClientSocket)) {
                NetplaySessionResetTiming();
                NetplaySessionState = NETPLAY_CONNECTED;
                strcpy(NetplayLastEvent, "TCP CONNECTED");
            } else {
                NetplayDisconnectSession();
                strcpy(NetplayLastEvent, "CONNECT FAILED");
            }
        }
    }

    if (NetplaySessionState == NETPLAY_JOINING) {
        NetSocket fd = NET_SOCKET_NONE;
        int const got = net_connect_poll(&fd);

        if (got == NET_CONNECT_READY) {
            /* The handshake runs here rather than on the connecting thread:
               it is protocol, not transport. Blocking for it costs nothing
               visible because emulation is held until the session is up. */
            if (NetplayJoinHandshake(fd)) {
                NetplayClientSocket = fd;
                NetplaySessionResetTiming();
                NetplaySessionState = NETPLAY_CONNECTED;
                strcpy(NetplayLastEvent,
                    NetplayJoinIsUDP != 0 ? "UDP CONNECTED" : "TCP CONNECTED");
            } else {
                net_close(fd);
                NetplaySessionState = NETPLAY_IDLE;
                strcpy(NetplayLastEvent, "HANDSHAKE FAILED");
            }
        } else if (got == NET_CONNECT_FAILED) {
            NetplaySessionState = NETPLAY_IDLE;
            strcpy(NetplayLastEvent, "CONNECT FAILED");
        }
    }
#else
    (void)timeout_ms;
#endif
}

#ifdef ZSNES_DEBUG_HOOKS
/* ZSNES_NETPLAY=host, or =join:<address>, opens a session on the first frame
   without anyone clicking the panel. That is what lets two headless instances
   be paired in a test, which is the only way this code has ever been run end
   to end. */
static void NetplayDebugAutoStart(void)
{
    static int done;
    char const* spec;

    if (done) {
        return;
    }
    done = 1;
    spec = getenv("ZSNES_NETPLAY");
    if (!spec) {
        return;
    }
    if (!strcmp(spec, "host")) {
        NetplayHostSession();
    } else if (!strncmp(spec, "join:", 5)) {
        snprintf(NetplayHostName, sizeof(NetplayHostName), "%s", spec + 5);
        NetplayJoinSession();
    }
}
#endif

#ifdef __UNIXSDL__
/* Read packets until the peer's input for `frame` is in hand, filing away
   anything that arrives early. Returns zero only when the peer has gone
   quiet for the whole of `patience` timeouts. */
static int NetplayAwaitFrame(u4 const frame, int const timeout, int patience,
    u4* const joy, u4* const crc)
{
    unsigned const want = (unsigned)(frame % NETPLAY_RING);

    for (;;) {
        NetplayPacket p;

        if (NetplayRemoteRingValid[want] != 0
            && NetplayRemoteRingFrame[want] == frame) {
            *joy = NetplayRemoteRing[want];
            *crc = NetplayRemoteRingCrc[want];
            NetplayRemoteRingValid[want] = 0;
            return 1;
        }
        if (!NetplayRecvPacket(NetplayClientSocket, &p, timeout)) {
            if (--patience <= 0) {
                return 0;
            }
            continue;
        }
        if (p.magic != NETPLAY_MAGIC || p.session != NetplaySessionToken) {
            continue; /* not ours; keep listening */
        }
        {
            unsigned const slot = (unsigned)(p.seq % NETPLAY_RING);

            NetplayRemoteRing[slot] = p.joy;
            NetplayRemoteRingCrc[slot] = p.crc;
            NetplayRemoteRingFrame[slot] = p.seq;
            NetplayRemoteRingValid[slot] = 1;
        }
    }
}
#endif

void NetplaySyncInputs(unsigned int* joy_a, unsigned int* joy_b)
{
#ifdef __UNIXSDL__
#ifdef ZSNES_DEBUG_HOOKS
    NetplayDebugAutoStart();
#endif
    NetplayAdvanceState(0);
    if (NetplaySessionState != NETPLAY_CONNECTED || NetplayClientSocket < 0)
        return;

    u4 const raw = NetplayHostRole != 0 ? (u4)*joy_a : (u4)*joy_b;

    // Delay local input by NETPLAY_INPUT_DELAY frames so the peer has time to
    // receive it before the frame executes — absorbs up to ~50ms of jitter.
    u4 delayed;
    if (NetplayInputQueueFilled < NETPLAY_INPUT_DELAY) {
        delayed = 0x00008000u; // neutral during warmup
        NetplayInputQueueFilled++;
    } else {
        delayed = NetplayInputQueue[NetplayInputQueuePos];
    }
    NetplayInputQueue[NetplayInputQueuePos] = raw;
    NetplayInputQueuePos = (NetplayInputQueuePos + 1) % NETPLAY_INPUT_DELAY;

    if (NetplayHostRole != 0)
        *joy_a = delayed;
    else
        *joy_b = delayed;

    u4 const local_crc = NetplayStateHash();

    NetplayPacket local;
    local.magic = NETPLAY_MAGIC;
    local.session = NetplaySessionToken;
    local.seq = NetplayFrame;
    NetplayLocalSeq++;
    local.joy = delayed;
    local.crc = local_crc;

    int const timeout = NETPLAY_INPUT_DELAY * NETPLAY_FRAME_MS;
    /* How long to keep waiting for a peer that has gone quiet before calling
       the session dead. One missed deadline used to end it outright, which a
       loopback test reached after 279 frames - on a real link a pause that
       long happens constantly. Lockstep cannot invent the peer's input, so
       the honest response to a late packet is to keep waiting. */
    int const patience = 20;

    u4 remote_joy = NETPLAY_JOY_NEUTRAL;
    u4 remote_crc = 0;
    int ok = 0;
    if (NetplayHostRole != 0) {
        if (NetplayHandshakePending != 0) {
            NetplayPacket handshake;
            if (NetplayPendingRemoteValid != 0) {
                handshake = NetplayPendingRemote;
                NetplayPendingRemoteValid = 0;
                ok = 1;
            } else {
                ok = NetplayRecvPacket(NetplayClientSocket, &handshake, timeout);
            }
            if (ok == 0 || !netplay_packet_is_handshake(&handshake, NetplaySessionToken)) {
                NetplayDisconnectSession();
                strcpy(NetplayLastEvent, "INVALID HANDSHAKE");
                return;
            }
            NetplayHandshakePending = 0;
        }
        ok = NetplayAwaitFrame(NetplayFrame, timeout, patience, &remote_joy, &remote_crc);
        if (ok != 0)
            ok = NetplaySendPacket(NetplayClientSocket, &local, timeout);
    } else {
        ok = NetplaySendPacket(NetplayClientSocket, &local, timeout);
#ifdef ZSNES_DEBUG_HOOKS
        if (ok == 0) {
            fprintf(stderr, "NETPLAY lost: send failed, errno %d, seq %u\n",
                errno, (unsigned)NetplayLocalSeq);
        }
#endif
        if (ok != 0) {
            ok = NetplayAwaitFrame(NetplayFrame, timeout, patience, &remote_joy, &remote_crc);
#ifdef ZSNES_DEBUG_HOOKS
            if (ok == 0) {
                fprintf(stderr, "NETPLAY lost: no packet in %d ms, frame %u\n",
                    timeout * patience, (unsigned)NetplayFrame);
            }
#endif
        }
    }

    if (ok == 0) {
        NetplayDisconnectSession();
        strcpy(NetplayLastEvent, "CONNECTION LOST");
        return;
    }

    /* Both sides now compare the checksum for the *same* frame, because the
       packet says which frame it is for. */
    if (local_crc != 0 && remote_crc != 0 && remote_crc != local_crc
        && NetplayFrame > NETPLAY_INPUT_DELAY + 2)
        strcpy(NetplayLastEvent, "DESYNC DETECTED");

    NetplayRemoteJoy = remote_joy;
    if (NetplayHostRole != 0)
        *joy_b = NetplayRemoteJoy;
    else
        *joy_a = NetplayRemoteJoy;

    NetplayFrame++;
#else
    (void)joy_a;
    (void)joy_b;
#endif
}

void NetplayUpdateStatus(void)
{
#ifdef __UNIXSDL__

    switch ((int)NetplaySessionState) {
    case NETPLAY_IDLE:
        snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "%s",
            NetplayLastEvent[0] != '\0' ? NetplayLastEvent : "IDLE");
        return;
    case NETPLAY_WAITING:
        if (net_extip_busy() == 0 && net_extip()[0] != '\0')
            snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "%s - EXT: %s",
                NetplayUDPConfig != 0 ? "WAITING UDP" : "WAITING TCP",
                net_extip());
        else
            snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "%s",
                NetplayUDPConfig != 0 ? "WAITING FOR UDP PEER" : "WAITING FOR TCP CLIENT");
        return;
    case NETPLAY_CONNECTING:
        snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "CONNECTING (TCP)");
        return;
    case NETPLAY_HANDSHAKING:
        snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "HANDSHAKING (TCP)");
        return;
    case NETPLAY_JOINING:
        snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "RESOLVING %s...", NetplayJoinHostCopy);
        return;
    case NETPLAY_CONNECTED:
        snprintf(NetplayStatusLine, sizeof(NetplayStatusLine), "%s",
            NetplayLastEvent[0] != '\0' ? NetplayLastEvent : "CONNECTED");
        return;
    }
#endif
}
