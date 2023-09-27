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
#define TRANSMIT_SIZE 32
#define HEADER_SIZE_NET 5
#define MAXNETNODES 2

// Netplay variables
extern int ZMaxPlayers;
extern bool NetIsNetplay;
extern bool NetIsClient;
extern u4 MyJoyData;
extern u4 CurrentInputFetch;
extern char *ZTransmitBuffer;
extern char *ZReadBuffer;
extern int ZPlayers;

// Netplay functions
extern void StartServer();
extern int PacketSend();
extern int PacketGet(bool isForConnection);
extern void NetplayHandleInputsBlank();
extern void HandleDisconnection();

// Packet info (Client)
struct PacketInfoClient {
	u4 CurrentInputTimer;
	u4 JoyDataClient;
};

// Packet info (Server)
struct PacketInfoServer {
	u1 SendNetdup;
	u4 CurrentInputTimer;
	u4 JoyDataA;
	u4 JoyDataB;
	u4 JoyDataC;
};

// Info state
extern struct PacketInfoServer DataServer;
extern struct PacketInfoClient ClientData;