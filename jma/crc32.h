/*
Copyright (C) 2004-2008 NSRT Team ( http://nsrt.edgeemu.com )

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

#ifndef CRC32_H
#define CRC32_H

#include <stddef.h>

/* PKZip CRC32.  Pass initial crc=0xFFFFFFFF; returns the final CRC value. */
unsigned int CRC32_calc(const unsigned char* data, size_t size, unsigned int crc);

#endif /* CRC32_H */
