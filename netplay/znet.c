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

// Messages
u4 MyJoyData;
u4 MyJoyDataBlacklistCurrent;
u4 MyJoyDataBlacklistNext;
u4 CurrentInputFetch;
bool allMatching[MAXNETNODES];
bool joyDataPressed[32];
int netdupValue = 0;
int currentNetdup = 0;
int playersCountedInput = 0;
int retries = 0;
int triedToHitBetterNetdup = 0;

// For current frame (if netdup)
u4 LastJoyDataA;
u4 LastJoyDataB;
u4 LastJoyDataC;

// For next frame
u4 JoyDataNextA;
u4 JoyDataNextB;
u4 JoyDataNextC;

struct PacketInfoServer DataServer;
struct PacketInfoClient ClientData;

// Check for new state (Server)
void CheckForNewStateServer(bool repeat) {
	// let's wait until we know everyone is in the same state.
	bool badNetdup = false;
	clock_t start = clock();
	do {
		// check
		int pck = PacketGet(false);
		if (pck >= 0) {
			memcpy((void *)&ClientData, ZReadBuffer + HEADER_SIZE_NET, sizeof(struct PacketInfoClient));
			// printf("[Server] %d = Client %d	Server %d\n", pck, ClientData.CurrentInputTimer, CurrentInputFetch);
			if (ClientData.CurrentInputTimer == CurrentInputFetch) {
				// where to put the input
				if (pck == 0) { JoyDataNextB = ClientData.JoyDataClient; }
				if (pck == 1) { JoyDataNextC = ClientData.JoyDataClient; }
				if (!allMatching[pck]) {
					allMatching[pck] = true;
					playersCountedInput++;
				}
			}
		}

		// resend previous packet so we can get a response
		if (repeat) {
			float seconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			if (seconds > 1.f) {
				printf("[Server] No sync in a second! Retrying.\n");
				memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &DataServer, sizeof(struct PacketInfoServer));
				for (int i = 0; i < ZPlayers; i++) {
					PacketSend(i);
				}
				start = clock();

				// check for failure
				retries++;
				if (retries >= 10) {
					HandleDisconnection();
					return;
				}
			}
			if(seconds > (1.f/30.f) && !badNetdup) {
				//we didn't receive everything in time, netdup should probably be raised to avoid lagging
				netdupValue++;
				if(netdupValue > 20) { netdupValue = 20; }
				triedToHitBetterNetdup = 0;
				//printf("Raised netdup due to repeat to: %d\n", netdupValue);
				badNetdup = true;
			}
		}
	} while (repeat && playersCountedInput < ZPlayers);

	//we got everything earlier than expected, maybe it's safe to try a less little netdup?
	if(repeat && !badNetdup) {
		triedToHitBetterNetdup++;
		if(triedToHitBetterNetdup >= 10) { //it must really be better if we have tried it 10 times
			netdupValue--;
			if(netdupValue < 0) { netdupValue = 0; }
			triedToHitBetterNetdup = 0;
		}
		//printf("Lowered netdup (good): %d\n", netdupValue);
	}
	retries = 0;
}

// Check for new state (Client)
void CheckForNewStateClient(bool repeat) {
	// Get Joy Data
	clock_t start = clock();
	do {
		int pck = PacketGet(false);
		if (pck == 0) {
			memcpy((void *)&DataServer, ZReadBuffer + HEADER_SIZE_NET, sizeof(struct PacketInfoServer));
			// printf("[Client] Server %d	Client %d\n", pck, DataServer.CurrentInputTimer, CurrentInputFetch);
			if (DataServer.CurrentInputTimer == (CurrentInputFetch + 1)) {
				JoyDataNextA = DataServer.JoyDataA;
				JoyDataNextB = DataServer.JoyDataB;
				JoyDataNextC = DataServer.JoyDataC;
				playersCountedInput++;
				break;
			}
		}

		// resend previous packet so we can get a response
		if (repeat) {
			float seconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			if (seconds > 1.f) {
				printf("[Client] No sync in a second! Retrying.\n");
				memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &ClientData, sizeof(struct PacketInfoClient));
				PacketSend(0);
				start = clock();

				// check for failure
				retries++;
				if (retries >= 10) {
					HandleDisconnection();
					return;
				}
			}
		}
	} while (repeat && playersCountedInput <= 0);
	retries = 0;
}

// Reads from Keyboard, etc.
void NetplayHandleInputsBlank(void) {
	currentNetdup++;

	// To handle better button presses
	if (netdupValue > 0) {
		for (int i = 0; i < 32; i++) {
			if (
				i == 31 || i == 30 ||
				i == 29 || i == 28 ||
				i == 23 || i == 22 ||
				i == 21 || i == 20) { // only handle this for ABXYLR
				bool bit = (JoyAOrig >> i) & 1;
				// printf("%i = %i\n", i, (int)bit);
				if (bit != joyDataPressed[i]) {
					// printf("Detected %d press\n", i);
					joyDataPressed[i] = bit;
					if (!joyDataPressed[i] && !(MyJoyDataBlacklistCurrent & (1 << i))) {
						MyJoyDataBlacklistNext |= 1 << i;
					}
				}
			}
		}
		MyJoyData |= JoyAOrig;
		MyJoyData &= ~MyJoyDataBlacklistCurrent;
	} else {
		MyJoyData = JoyAOrig;
	}

	// Let's check for new inputs
	if (currentNetdup > netdupValue) {
		currentNetdup = 0;
		if (!NetIsClient) { // server input handling, receive client data, send over full input data
			CheckForNewStateServer(true);

			// Get JoyAOrig
			JoyDataNextA = MyJoyData;

			// We've received all of them by now, let's just throw the results in.
			CurrentInputFetch++;
			playersCountedInput = 0;
			memset(allMatching, 0, sizeof(allMatching));

			// Let's send the clients our data
			DataServer.CurrentInputTimer = CurrentInputFetch;
			DataServer.JoyDataA = JoyDataNextA;
			DataServer.JoyDataB = JoyDataNextB;
			DataServer.JoyDataC = JoyDataNextC;
			DataServer.SendNetdup = netdupValue;
			memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &DataServer, sizeof(struct PacketInfoServer));
			for (int i = 0; i < ZPlayers; i++) { PacketSend(i); }
		} else { // client input handling, receive server data for handling input
			CheckForNewStateClient(true);
			netdupValue = DataServer.SendNetdup;

			// Update timer.
			CurrentInputFetch++;
			playersCountedInput = 0;

			// Let's send the server our data
			ClientData.CurrentInputTimer = CurrentInputFetch;
			ClientData.JoyDataClient = MyJoyData;
			memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &ClientData, sizeof(struct PacketInfoClient));
			PacketSend(0);
		}
		MyJoyData = 0;
		MyJoyDataBlacklistCurrent = MyJoyDataBlacklistNext;
		MyJoyDataBlacklistNext = 0;
		LastJoyDataA = JoyDataNextA;
		LastJoyDataB = JoyDataNextB;
		LastJoyDataC = JoyDataNextC;
	} else {
		if (!NetIsClient) {
			CheckForNewStateServer(false);
			memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &DataServer, sizeof(struct PacketInfoServer));
			for (int i = 0; i < ZPlayers; i++) {
				PacketSend(i);
			}
		} else {
			CheckForNewStateClient(false);
			memcpy(ZTransmitBuffer + HEADER_SIZE_NET, &ClientData, sizeof(struct PacketInfoClient));
			PacketSend(0);
		}
	}
	JoyAOrig = LastJoyDataA;
	JoyBOrig = LastJoyDataB;
	JoyCOrig = LastJoyDataC;
}