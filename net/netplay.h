/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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

/* What the panel shows. */
extern char NetplayStatusLine[64];
extern u1 NetplayHostRole;

/* Settings the panel edits. */
extern char NetplayHostName[32];
extern u1 NetplayUDPConfig;

#endif
