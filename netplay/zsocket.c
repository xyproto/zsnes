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

#include "znet.h"

// Get more portable
#ifdef _WIN32
typedef int socklen_t;
const char *neterror(void) {
	int code = WSAGetLastError();
	switch (code) {
		case WSAEACCES: return "EACCES";
		case WSAEADDRINUSE: return "EADDRINUSE";
		case WSAEADDRNOTAVAIL: return "EADDRNOTAVAIL";
		case WSAEAFNOSUPPORT: return "EAFNOSUPPORT";
		case WSAEALREADY: return "EALREADY";
		case WSAECONNABORTED: return "ECONNABORTED";
		case WSAECONNREFUSED: return "ECONNREFUSED";
		case WSAECONNRESET: return "ECONNRESET";
		case WSAEDESTADDRREQ: return "EDESTADDRREQ";
		case WSAEFAULT: return "EFAULT";
		case WSAEHOSTDOWN: return "EHOSTDOWN";
		case WSAEHOSTUNREACH: return "EHOSTUNREACH";
		case WSAEINPROGRESS: return "EINPROGRESS";
		case WSAEINTR: return "EINTR";
		case WSAEINVAL: return "EINVAL";
		case WSAEISCONN: return "EISCONN";
		case WSAEMFILE: return "EMFILE";
		case WSAEMSGSIZE: return "EMSGSIZE";
		case WSAENETDOWN: return "ENETDOWN";
		case WSAENETRESET: return "ENETRESET";
		case WSAENETUNREACH: return "ENETUNREACH";
		case WSAENOBUFS: return "ENOBUFS";
		case WSAENOPROTOOPT: return "ENOPROTOOPT";
		case WSAENOTCONN: return "ENOTCONN";
		case WSAENOTSOCK: return "ENOTSOCK";
		case WSAEOPNOTSUPP: return "EOPNOTSUPP";
		case WSAEPFNOSUPPORT: return "EPFNOSUPPORT";
		case WSAEPROCLIM: return "EPROCLIM";
		case WSAEPROTONOSUPPORT: return "EPROTONOSUPPORT";
		case WSAEPROTOTYPE: return "EPROTOTYPE";
		case WSAESHUTDOWN: return "ESHUTDOWN";
		case WSAESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
		case WSAETIMEDOUT: return "ETIMEDOUT";
		case WSAEWOULDBLOCK: return "EWOULDBLOCK";
		case WSAHOST_NOT_FOUND: return "HOST_NOT_FOUND";
		case WSANOTINITIALISED: return "NOTINITIALISED";
		case WSANO_DATA: return "NO_DATA";
		case WSANO_RECOVERY: return "NO_RECOVERY";
		case WSASYSNOTREADY: return "SYSNOTREADY";
		case WSATRY_AGAIN: return "TRY_AGAIN";
		case WSAVERNOTSUPPORTED: return "VERNOTSUPPORTED";
		case WSAEDISCON: return "EDISCON";
		default: return "UNKNOWN";
	}
}
#else
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket close
#define ioctlsocket ioctl
#define Sleep(x) usleep(x * 1000)
#define neterror() strerror(errno)
#endif

static unsigned short ZSNESPORT = 25500;
static SOCKET mysocket = INVALID_SOCKET;
static struct sockaddr_in sendaddress[MAXNETNODES];
int ZPlayers = 0;
int ZMaxPlayers = 0;
char *ZTransmitBuffer = NULL;
char *ZReadBuffer = NULL;

// Net
bool NetIsNetplay = false;
bool NetIsClient = false;

// Handle disconnection
void HandleDisconnection() {
	Msgptr = "SOCKET DISCONNECTED.";
	MessageOn = MsgCount;
	NetIsNetplay = false;
	InitSound();
}

// PacketSend
int PacketSend(int node) {
	int size = NetIsClient ? sizeof(struct PacketInfoClient) : sizeof(struct PacketInfoServer);
	int bytes = sendto(mysocket, ZTransmitBuffer, size + 5, 0, (struct sockaddr *)&sendaddress[node], sizeof(sendaddress[node]));
	if (bytes == -1) {
		printf("PacketSend(%d) with %d size data failed: %s\n", node, size, neterror());
		return 0;
	}
	return 1;
}

// PacketGet
int PacketGet(bool isForConnection) {
	socklen_t fromlen = sizeof(struct sockaddr_in);
	struct sockaddr_in fromaddress;
	int c = recvfrom(mysocket, ZReadBuffer, TRANSMIT_SIZE, 0, (struct sockaddr *)&fromaddress, &fromlen);
	if (c < (HEADER_SIZE_NET + 2)) {
		return -1;
	}
	if (c == SOCKET_ERROR) {
		HandleDisconnection();
		return -1;
	}

	// check if packet is valid (has ZSNES header)
	if (
		ZReadBuffer[0] == 'Z' &&
		ZReadBuffer[1] == 'S' &&
		ZReadBuffer[2] == 'N' &&
		ZReadBuffer[3] == 'E' &&
		ZReadBuffer[4] == 'S') {
		if (!NetIsClient) {
			// find remote node number
			int nodeNumber = -1;
			for (int i = 0; i < ZMaxPlayers; i++) {
				if (fromaddress.sin_addr.s_addr == sendaddress[i].sin_addr.s_addr && fromaddress.sin_port == sendaddress[i].sin_port) {
					nodeNumber = i;
					break;
				}
			}
			if (isForConnection && ZReadBuffer[5] == 'C') {
				memcpy(&sendaddress[ZPlayers], &fromaddress, sizeof(fromaddress));
				PacketSend(ZPlayers);
				printf("Got connect from player %d from %s:%d\n", ZPlayers + 1, inet_ntoa(fromaddress.sin_addr), fromaddress.sin_port);
				ZPlayers++;
			}
			return nodeNumber;
		} else {
			if (isForConnection) {
				if (ZReadBuffer[5] == 'C') {
					netdupValue = ZReadBuffer[6];
					currentNetdup = (int)netdupValue;
					printf("Got join info from server. Running on netdup value %d.\n", netdupValue);
					if(ZReadBuffer[7] > 0) {
						printf("Ready.\n");
						return 0;
					}
				}
			} else {
				return 0;
			}
		}
	}
	// printf("We got a packet from %s:%d, but it isn't a ZSNES packet.\n", inet_ntoa(fromaddress.sin_addr), fromaddress.sin_port);
	return -1;
}

// BindToLocalPort
void BindToLocalPort(SOCKET s, unsigned short port) {
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int v = bind(s, (struct sockaddr *)&address, sizeof(address));
	if (v == SOCKET_ERROR) {
		printf("BindToPort: %s\n", neterror());
	}
	printf("Binded to local port %d.\n", htons(address.sin_port));
}

// Get connections
bool Host_CheckForConnects() {
	PacketGet(true);
	return ZPlayers >= ZMaxPlayers;
}

// Start network
void StartNetwork(bool autoPort) {
// Windows init
#ifdef _WIN32
	WSADATA wsad;
	if (WSAStartup(0x0101, &wsad)) {
		printf("Could not initialize Windows Sockets\n");
	}
#endif

	// Ensure no garbage data
	memset(&DataServer, 0, sizeof(struct PacketInfoServer));
	memset(&ClientData, 0, sizeof(struct PacketInfoClient));

	// Mark this as a netplay game
	NetIsNetplay = true;

	// Create communication socket
	mysocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mysocket == INVALID_SOCKET) {
		printf("Can't create socket: %s\n", neterror());
	}
	if (!autoPort) {
		BindToLocalPort(mysocket, autoPort ? 0 : ZSNESPORT);
	}

	u_long trueval = 1;
	ioctlsocket(mysocket, FIONBIO, &trueval);

	// Ready
	printf("Network ready! Initializing buffers.\n");

	// Initialize transit buffer and read buffer
	ZTransmitBuffer = malloc(TRANSMIT_SIZE);
	ZReadBuffer = malloc(TRANSMIT_SIZE);

	// Write packet header (this should never be overwritten by anything)
	ZTransmitBuffer[0] = 'Z';
	ZTransmitBuffer[1] = 'S';
	ZTransmitBuffer[2] = 'N';
	ZTransmitBuffer[3] = 'E';
	ZTransmitBuffer[4] = 'S';
	ZTransmitBuffer[5] = 'C';
	ZTransmitBuffer[6] = netdupValue;
	ZTransmitBuffer[7] = ZMaxPlayers == 1;
}

// Start server loop
void StartServer(char *Players) {
	printf("Starting ZSNES server..\n");
	ZMaxPlayers = Players[0] - 0x30;
	if (ZMaxPlayers > 0 && ZMaxPlayers <= MAXNETNODES) {
		StartNetwork(false);
		NetIsClient = false;
		printf("Waiting for client connections.\n");
		while (!Host_CheckForConnects()) {
			;
			;
		}
		ZTransmitBuffer[7] = 1;
		printf("All here!\n");
		for (int i = 0; i < ZPlayers; i++) { PacketSend(i); }
		Sleep(1);
	} else {
		printf("Invalid player count. Just going to game.\n");
	}
}

// Start client loop
void StartClient(char *IPAddress) {
	printf("Starting ZSNES client..\n");
	StartNetwork(true);
	NetIsClient = true;

	// Allocate the server sock address
	printf("Preparing client, connecting to %s.\n", IPAddress);
	sendaddress[0].sin_family = AF_INET;
	sendaddress[0].sin_port = htons(ZSNESPORT);
	sendaddress[0].sin_addr.s_addr = inet_addr(IPAddress);

	// Packet sending
	PacketSend(0);
	printf("Packet sent. Waiting for server response.\n");
	while (PacketGet(true) < 0) {
		Sleep(1);
	}
	printf("Got hit by server. We are online!\n");
}