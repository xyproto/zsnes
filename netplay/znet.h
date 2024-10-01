/*
Copyright (C) 2023 Sneed, ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#include "../gblhdr.h"
#include "../c_init.h"
#include "../video/procvid.h"

// Use Winsock for Win32:
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif

// As per http://support.microsoft.com/kb/q192599/ the standard size for network buffers is 8k.
#define TRANSMIT_SIZE 4000
#define MAXNETNODES 4
#define TOTALNETNODES 5
#define BACKUPTICS 64
#define BACKUPTICS_ANDV 63

// Netplay variables
extern int ZMaxPlayers;
extern int NetworkTick;
extern int LocalNetworkTick;
extern int MyNetworkNode;
extern unsigned char NetIsNetplay;
extern bool NetIsClient;
extern bool NetFastforward;
extern int ZPlayers;

// Player
struct NetworkNode {
	int NetTic;
	u4 JoyDataFrames[BACKUPTICS];
};

extern struct NetworkNode ZSNodes[TOTALNETNODES];

// Packet data
// Headers:
// 2 bytes for "ZS"
// 1 byte for header. 4 bits for command 4 bits for player count
// foreach player:
//    4 bytes for the current tic
//    1 byte for the node
//    1 byte for amount of tics packaged in this packet
//    foreach input tick:
//        4 bytes for each tic

#define HEADER_SIZE_NET 2
extern char ZNetplayMessage[50];
extern char ZTransmitBuffer[TRANSMIT_SIZE];
extern int TransmitBufferSize;

// Getting
#define copyVarTransmitBuffer(point, val) memcpy(ZTransmitBuffer + point, &val, sizeof(val))
extern u4 getU4TransmitBuffer(int point);

// Netplay functions
extern void StartServer();
extern int PacketSend();
extern int PacketGet(bool isForConnection);
extern void NetplayHandleInputsBlank();
extern void HandleDisconnection();