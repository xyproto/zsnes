//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <windows.h>
#include <wsipx.h>
#include <wsnwlink.h>

void ipx_init() {
int          iAdapters,iOpt=sizeof(iAdapters),iSize=sizeof(SOCKADDR_IPX);
SOCKET       skNum;
SOCKADDR_IPX Addr;
WSADATA      Wsa;

// Init Winsock and get IPX address

if (WSAStartup(0x0101,&Wsa)) return;
if ((skNum=socket(AF_IPX,SOCK_DGRAM,NSPROTO_IPX)) != INVALID_SOCKET)
   {
   memset(&Addr,0,sizeof(Addr));
   Addr.sa_family=AF_IPX;
   if (bind(skNum,(SOCKADDR *)&Addr,iSize)!=SOCKET_ERROR)
      {
      if (getsockopt(skNum,NSPROTO_IPX,IPX_MAX_ADAPTER_NUM,
                     (char *)&iAdapters,&iOpt)!=SOCKET_ERROR)
         {
         while(iAdapters)
            {
            IPX_ADDRESS_DATA Data;
            memset(&Data,0,sizeof(Data));
            Data.adapternum=iAdapters-1;
            iOpt=sizeof(Data);
            if (getsockopt(skNum,NSPROTO_IPX,IPX_ADDRESS, (char *) &Data, 
                &iOpt)!=SOCKET_ERROR)
               {
//               sprintf("Addr: %02X%02X%02X%02X:%02X%02X%02X%02X%02X%02X\n",
//               (int)Data.netnum[0],(int)Data.netnum[1],(int)Data.netnum[2],
//               (int)Data.netnum[3],(int)Data.netnum[4],(int)Data.netnum[5],
//               (int)Data.netnum[6],(int)Data.netnum[7],(int)Data.netnum[8],
//               (int)Data.netnum[9]);
               }
            iAdapters--;
            }
         }
      }
   closesocket(skNum);
   }
WSACleanup();

}

void sendpacket() {
}

void checkpacket() {
}

void read_packet() {
}

void ipx_deinit() {
}

unsigned short ipx_initcode;
unsigned int ipx_packet;
unsigned int ipx_packet_size;
unsigned char ipx_packet_ready;
unsigned int ipx_read_packet;

/*
EXTSYM _ipx_init               ; To init ipx
EXTSYM _ipx_initcode           ; return 0 if everything is ok (int)

EXTSYM _ipx_packet             ; 80 bytes buffer to send
EXTSYM _ipx_packet_size        ; size to send (max 80 bytes) (dword)
EXTSYM _sendpacket             ; to send a packet

EXTSYM _checkpacket            ; check if a packet is ready to receive
EXTSYM _ipx_packet_ready       ; return 1 if there is a packet ready (byte)

EXTSYM _read_packet            ; to read an incoming packet
EXTSYM _ipx_read_packet        ; 80 bytes buffer of received packet

EXTSYM _ipx_deinit             ; to deinit the ipx
*/
