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

/* One frame of netplay input, and the rules for putting it on a wire.
 *
 * Nothing here touches a socket, a window or emulator state, so it builds and
 * runs anywhere and the tests link the same code the emulator does. The bytes
 * are read and written one at a time rather than by casting a struct over the
 * buffer: the layout is then the format, whatever a compiler chooses to do
 * with padding or alignment. */

#ifndef ZSNES_NET_PACKET_H
#define ZSNES_NET_PACKET_H

#include <stddef.h>
#include <stdint.h>

enum { NETPLAY_MAGIC = 0x4E455450u, /* "NETP" */
    NETPLAY_PACKET_BYTES = 20,
    /* A pad reading neutral: no buttons, and the bit the SNES always sets. */
    NETPLAY_JOY_NEUTRAL = 0x00008000u };

typedef struct {
    uint32_t magic;
    uint32_t session;
    uint32_t seq;
    uint32_t joy;
    uint32_t crc;
} NetplayPacket;

/* Big-endian, five words, in the order declared above. */
void netplay_packet_encode(uint8_t out[NETPLAY_PACKET_BYTES], NetplayPacket const* p);

/* Decodes whatever arrived. Returns zero when the bytes are not a packet,
   which is what a stray datagram on the port looks like, so the caller can
   drop it and keep waiting. */
int netplay_packet_decode(NetplayPacket* out, uint8_t const in[NETPLAY_PACKET_BYTES]);

/* The opening packet of a session: right magic, right session, and a neutral
   pad on frame zero. */
int netplay_packet_is_handshake(NetplayPacket const* p, uint32_t session);

uint32_t netplay_fnv1a(void const* data, size_t len);

#endif
