/* What the rest of the emulator may ask of netplay. The GUI draws the panel
   and calls in here; nothing outside this file needs to know that there is a
   socket behind it. */

#ifndef ZSNES_NET_NETPLAY_H
#define ZSNES_NET_NETPLAY_H

#include "../types.h"

/* Session control, driven by the buttons on the netplay panel. */
void NetplayHostSession(void);
void NetplayJoinSession(void);
void NetplayDisconnectSession(void);

/* One frame of input, exchanged with the peer. Called from the input path
   after local device mapping, so each side sees the same two pads. */
void NetplaySyncInputs(unsigned int* joy_a, unsigned int* joy_b);

/* Pump the state machine and refresh the status line. The panel calls both
   every time it is drawn; `timeout_ms` is how long the pump may block. */
void NetplayAdvanceState(int timeout_ms);
void NetplayUpdateStatus(void);

/* Non-zero once a session has connected and the two consoles have not yet
   been put into the same state. Exchanging input frame by frame only keeps
   two machines together if they agreed to begin with, so the emulator answers
   this by power-cycling at a safe point and then calls NetplayStartDone. */
int NetplayStartPending(void);
void NetplayStartDone(void);

/* What the panel shows. */
extern char NetplayStatusLine[64];
extern u1 NetplayHostRole;

/* Settings the panel edits. */
extern char NetplayHostName[32];
extern u1 NetplayUDPConfig;
/* Reach the peer through the relay in server/ rather than directly, for when
   neither player can accept a connection. The host field then reads
   "relay[:port][/room]". */
extern u1 NetplayRelayConfig;

#endif
