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

void ipx_init(){};
void sendpacket(){};
void checkpacket(){};
void read_packet(){};
void ipx_deinit(){};

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
