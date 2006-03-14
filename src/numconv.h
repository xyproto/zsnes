/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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

#ifndef NUMCONV_H
#define NUMCONV_H

#include <stdio.h>

//Get correct mask for particular bit
#define BIT(X) (1 << (X))

/*
Functions that the compiler should inline that will convert
uint32, uint24, uint16 to 4 byte, 3 byte, 2 byte arrays
and back. -Nach
*/

static unsigned char *uint32_to_bytes(unsigned int num)
{
  static unsigned char buffer[4];
  buffer[3] = (num >> 24) & 0xFF;
  buffer[2] = (num >> 16) & 0xFF;
  buffer[1] = (num >> 8) & 0xFF;
  buffer[0] = num & 0xFF;
  return(buffer);
}

static unsigned int bytes_to_uint32(const unsigned char buffer[4])
{
  unsigned int num = (unsigned int)buffer[0];
  num |= ((unsigned int)buffer[1]) << 8;
  num |= ((unsigned int)buffer[2]) << 16;
  num |= ((unsigned int)buffer[3]) << 24;
  return(num);
}

static unsigned char *uint24_to_bytes(unsigned int num)
{
  static unsigned char buffer[3];
  buffer[2] = (num >> 16) & 0xFF;
  buffer[1] = (num >> 8) & 0xFF;
  buffer[0] = num & 0xFF;
  return(buffer);
}

static unsigned int bytes_to_uint24(const unsigned char buffer[3])
{
  unsigned int num = (unsigned int)buffer[0];
  num |= ((unsigned int)buffer[1]) << 8;
  num |= ((unsigned int)buffer[2]) << 16;
  return(num);
}

static unsigned char *uint16_to_bytes(unsigned short num)
{
  static unsigned char buffer[2];
  buffer[1] = (num >> 8) & 0xFF;
  buffer[0] = num & 0xFF;
  return(buffer);
}

static unsigned short bytes_to_uint16(const unsigned char buffer[2])
{
  unsigned short num = (unsigned short)buffer[0];
  num |= ((unsigned short)buffer[1]) << 8;
  return(num);
}

//Functions to read 2, 3, 4 bytes and convert to uint16, uint24, uint32
static unsigned short fread2(FILE *fp)
{
  unsigned char uint16buf[2];
  fread(uint16buf, 2, 1, fp);
  return(bytes_to_uint16(uint16buf));
}

static unsigned int fread3(FILE *fp)
{
  unsigned char uint24buf[3];
  fread(uint24buf, 3, 1, fp);
  return(bytes_to_uint24(uint24buf));
}

static unsigned int fread4(FILE *fp)
{
  unsigned char uint32buf[4];
  fread(uint32buf, 4, 1, fp);
  return(bytes_to_uint32(uint32buf));
}

//Functions to write uint16, uint24, uint32 as 2, 3, 4 bytes
static void fwrite2(unsigned short var, FILE *fp)
{
  fwrite(uint16_to_bytes(var), 2, 1, fp);
}

static void fwrite3(unsigned int var, FILE *fp)
{
  fwrite(uint24_to_bytes(var), 3, 1, fp);
}

static void fwrite4(unsigned int var, FILE *fp)
{
  fwrite(uint32_to_bytes(var), 4, 1, fp);
}

#endif
