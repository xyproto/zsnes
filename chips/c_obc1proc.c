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

/*
 * Pure-C port of chips/obc1proc.asm.
 *
 * Routing logic (RouteAccess macro in obc1proc.asm):
 *   addr & 0x8000  -> forward to memaccessbank* (normal banked RAM/ROM)
 *   addr < 0x6000  -> forward to regaccessbank* (SNES I/O registers)
 *   0x6000-0x7FFF  -> OBC1 chip registers via GetOBC1/SetOBC1
 */

#include "../types.h"
#include "c_obc1proc.h"
#include "../cpu/memory.h"

extern u2   obc1_address;
extern u1   obc1_byte;
extern void GetOBC1(void);
extern void SetOBC1(void);

u1 __attribute__((fastcall)) OBC1Read8b(u2 addr)
{
    if (addr & 0x8000)
        return memaccessbankr8(addr);
    if (addr < 0x6000)
        return regaccessbankr8(addr);
    obc1_address = addr;
    GetOBC1();
    return obc1_byte;
}

void __attribute__((fastcall)) OBC1Write8b(u2 addr, u1 val)
{
    if (addr & 0x8000) { memaccessbankw8(addr, val); return; }
    if (addr < 0x6000) { regaccessbankw8(addr, val); return; }
    obc1_address = addr;
    obc1_byte    = val;
    SetOBC1();
}

u2 __attribute__((fastcall)) OBC1Read16b(u2 addr)
{
    if (addr & 0x8000)
        return memaccessbankr16(addr);
    if (addr < 0x6000)
        return regaccessbankr16(addr);
    obc1_address = addr;
    GetOBC1();
    u1 lo = obc1_byte;
    obc1_address++;
    GetOBC1();
    return (u2)lo | ((u2)obc1_byte << 8);
}

void __attribute__((fastcall)) OBC1Write16b(u2 addr, u2 val)
{
    if (addr & 0x8000) { memaccessbankw16(addr, val); return; }
    if (addr < 0x6000) { regaccessbankw16(addr, val); return; }
    obc1_address = addr;
    obc1_byte    = (u1)val;
    SetOBC1();
    obc1_address++;
    obc1_byte = (u1)(val >> 8);
    SetOBC1();
}
