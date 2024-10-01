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

//Fastforward
bool NetFastforward = false;

//This gets the lowest tic, aka currently active tick.
int GetLowTic() {
	int LOWEST_TIC = INT_MAX;
	for(int i = 0; i <= ZPlayers; i++) {
		if(ZSNodes[i].NetTic < LOWEST_TIC) {
			LOWEST_TIC = ZSNodes[i].NetTic;
		}
	}
	return LOWEST_TIC;
}

int GetHighTic() {
	int HIGHEST_TIC = 0;
	for(int i = 0; i <= ZPlayers; i++) {
		if(ZSNodes[i].NetTic > HIGHEST_TIC) {
			HIGHEST_TIC = ZSNodes[i].NetTic;
		}
	}
	return HIGHEST_TIC;
}

// Get Packets
void GetPackets() {
	//Player input sending
	while(true) {
		int pck = PacketGet(false);
		if(pck >= 0) {
			int AmountOfPlayersPackage = ZTransmitBuffer[HEADER_SIZE_NET] & 0xF;
			int pos = HEADER_SIZE_NET + 1;
			//loop
			for(int player = 0; player < AmountOfPlayersPackage; player++) {
				//4 bytes for the tick
				int StartTickFill = getU4TransmitBuffer(pos); pos += 4;

				//1 byte for the node
				int node = ZTransmitBuffer[pos++];
				if(node <= ZPlayers) {
					//1 byte for the amount of tics
					int AmountOfTics = ZTransmitBuffer[pos++];

					//tic data
					//printf("NODE %d IS TIC: %d / %d (From packet sized: %d). They sent us %d tics.\n", node, StartTickFill, NetworkTick, TransmitBufferSize, AmountOfTics);

					//check if packet is in order
					for(int i = 0; i < AmountOfTics; i++) {
						ZSNodes[node].JoyDataFrames[(StartTickFill + i) & BACKUPTICS_ANDV] = getU4TransmitBuffer(pos); pos += 4;
					}

					//increment tics!
					int NewTics = StartTickFill + AmountOfTics;
					if(NewTics > ZSNodes[node].NetTic) {
						ZSNodes[node].NetTic = NewTics;
					}
				}
			}
		} else {
			break;
		}
	}
}

// Reads from Keyboard, etc.
void NetplayHandleInputsBlank(void) {
	int ticks8 = 0;
	while(true) {
		//Check for timeout.
		if (ticks8 > 600) {
			HandleDisconnection();
			Msgptr = "TIMED OUT.";
			return;
		}

		//Fill local player tics.
		if((LocalNetworkTick - NetworkTick) < (BACKUPTICS / 2 - 1) && !NetFastforward) {
			ZSNodes[MyNetworkNode].JoyDataFrames[LocalNetworkTick & BACKUPTICS_ANDV] = JoyAOrig;
			LocalNetworkTick++; ZSNodes[MyNetworkNode].NetTic = LocalNetworkTick;
		} //else too far ahead

		//Send our own packets.
		for(int i = 0; i < 1; i++) {
			ZTransmitBuffer[HEADER_SIZE_NET] = 1;
			TransmitBufferSize = HEADER_SIZE_NET + 1;

			//So here's what happening.
			int AmountOfTicsToTransfer = LocalNetworkTick - NetworkTick;
			if(AmountOfTicsToTransfer >= BACKUPTICS) {
				HandleDisconnection();
				Msgptr = "TOO MUCH PING.";
				return;
			}
			copyVarTransmitBuffer(TransmitBufferSize, NetworkTick); TransmitBufferSize += 4;

			//Put tick
			ZTransmitBuffer[TransmitBufferSize++] = MyNetworkNode;
			ZTransmitBuffer[TransmitBufferSize++] = AmountOfTicsToTransfer;

			//Package all of the inputs
			for(int tick = NetworkTick; tick < LocalNetworkTick; tick++) {
				copyVarTransmitBuffer(TransmitBufferSize, ZSNodes[MyNetworkNode].JoyDataFrames[tick & BACKUPTICS_ANDV]); TransmitBufferSize += 4;
			}

			//And send!
			PacketSend(i);

			//Diagnostics
			//printf("PK (SIZE=%d) with %d tics from %d.\n", TransmitBufferSize, AmountOfTicsToTransfer, NetworkTick);
		}

		//Get incoming packets.
		GetPackets();

		//Check if we can continue towards a next tic.
		int LowTic = GetLowTic();
		if(NetworkTick < LowTic) {
			int ExpectedLatency = GetHighTic() - GetLowTic();
			int Disp = GetHighTic() - ZSNodes[MyNetworkNode].NetTic;

			snprintf(ZNetplayMessage, 48, "%d%c, ping: %dms - %d>,%d< (%d/%d)", MyNetworkNode, NetIsClient ? 'C' : 'S', ExpectedLatency * 16, LowTic - NetworkTick, Disp, ZSNodes[0].NetTic, ZSNodes[1].NetTic);

			//These buffers should be filled.
			JoyAOrig = ZSNodes[0].JoyDataFrames[NetworkTick & BACKUPTICS_ANDV];
			JoyBOrig = ZSNodes[1].JoyDataFrames[NetworkTick & BACKUPTICS_ANDV];
			JoyCOrig = ZSNodes[2].JoyDataFrames[NetworkTick & BACKUPTICS_ANDV];
			JoyDOrig = ZSNodes[3].JoyDataFrames[NetworkTick & BACKUPTICS_ANDV];
			JoyEOrig = ZSNodes[4].JoyDataFrames[NetworkTick & BACKUPTICS_ANDV];

			//Advance.
			NetworkTick++;
			NetFastforward = NetworkTick < LowTic || Disp > 2;

			//Should we FF?
			break;
		} else {
			//printf("Not enough tics. Local = %d (%d / %d)\n", LocalNetworkTick, ZSNodes[0].NetTic, ZSNodes[1].NetTic);
			NetFastforward = false;
			ticks8++;
			usleep(8000);
		}
	}
}