/*
Copyright (C) 2004-2008 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )

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

#ifndef PORTABLE_H
#define PORTABLE_H

#include <stdint.h>
#include <string.h>

typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;

typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

typedef unsigned UINT_PTR;

#define HRESULT int
#define S_OK 0
#define E_INVALIDARG (-1)
#define E_OUTOFMEMORY (-2)
#define E_FAIL (-3)
#define E_INTERNAL_ERROR (-4)
#define E_INVALIDDATA (-5)

#define JMA_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JMA_MAX(a, b) ((a) > (b) ? (a) : (b))

#define RETURN_IF_NOT_S_OK(x) \
    do {                      \
        HRESULT _r = (x);     \
        if (_r != S_OK)       \
            return _r;        \
    } while (0)

#define UINT_SIZE 4
#define USHORT_SIZE 2

static inline unsigned int charp_to_uint(const unsigned char buf[UINT_SIZE])
{
    return ((unsigned int)buf[0] << 24) | ((unsigned int)buf[1] << 16)
        | ((unsigned int)buf[2] << 8) | (unsigned int)buf[3];
}

static inline unsigned short charp_to_ushort(const unsigned char buf[USHORT_SIZE])
{
    return (unsigned short)(((unsigned short)buf[0] << 8) | buf[1]);
}

#endif /* PORTABLE_H */
