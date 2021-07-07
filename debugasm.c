/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * http://www.zsnes.com
 * http://sourceforge.net/projects/zsnes
 * https://zsnes.bountysource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include "cpu/c_65816d.h"
#include "cpu/c_memory.h"
#include "cpu/execute.h"
#include "cpu/memory.h"
#include "cpu/memtable.h"
#include "cpu/regs.h"
#include "cpu/spc700.h"
#include "debugasm.h"
#include "debugger.h"
#include "endmem.h"
#include "init.h"
#include "initc.h"
#include "types.h"

void breakops(void)
{
    u4 const page = PrevBreakPt_page;
    u4 const offset = PrevBreakPt_offset;
    u1 const* const map = offset & 0x8000 ? snesmmap[page] : snesmap2[page];
    u1 const* const breakarea = map + offset; // add program counter to address

    u4 const pc = xpc;
    u4 const pb = xpb;
    u1* const addr = pc & 0x8000 ? snesmmap[pb] : pc < 0x4300 || memtabler8[pb] != regaccessbankr8 ? snesmap2[pb]
                                                                                                   : (u1*)dmadata - 0x4300; // XXX ugly cast
    initaddrl = addr;

    u4 ecx = 0;
    u4 edx = curcyc /* cycles */ << 8 | xp /* flags */;
    u1* ebp = spcPCRam;
    u1* esi = addr + pc; // add program counter to address
    eop** edi = Curtableaddr;
    UpdateDPage();
    // execute
    do {
        splitflags(edx);
        u4 ebx;
        // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
        asm volatile("push %%ebp;  mov %0, %%ebp;  call %P6;  mov %%ebp, %0;  pop %%ebp"
                     : "+a"(ebp), "+c"(ecx), "+d"(edx), "=b"(ebx), "+S"(esi), "+D"(edi)
                     : "X"(execsingle)
                     : "cc", "memory");
        edx = joinflags(edx);
        edx = edx & 0xFFFF00FF | pdh << 8;
        if ((++numinst & 0xFF) == 0 && getch() == 27)
            break;
    } while (esi != breakarea);
    // copy back data
    spcPCRam = ebp;
    Curtableaddr = edi;
    xp = edx;
    curcyc = edx >> 8;
    xpc = esi - initaddrl; // subtract program counter by address
}

void execnextop(void)
{
    u4 const pc = xpc;
    u4 const pb = xpb;
    u1* const addr = pc & 0x8000 ? snesmmap[pb] : pc < 0x4300 || memtabler8[pb] != regaccessbankr8 ? snesmap2[pb]
                                                                                                   : (u1*)dmadata - 0x4300; // XXX ugly cast
    initaddrl = addr;

    u4 ecx = 0;
    u4 edx = curcyc /* cycles */ << 8 | xp /* flags */;
    u1* ebp = spcPCRam;
    u1* esi = addr + pc; // add program counter to address
    eop** edi = Curtableaddr;

    // execute
    splitflags(edx);
    u4 ebx;
    // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
    asm volatile("push %%ebp;  mov %0, %%ebp;  call %P6;  mov %%ebp, %0;  pop %%ebp"
                 : "+a"(ebp), "+c"(ecx), "+d"(edx), "=b"(ebx), "+S"(esi), "+D"(edi)
                 : "X"(execsingle)
                 : "cc", "memory");
    edx = joinflags(edx);
    UpdateDPage();

    // copy back data
    spcPCRam = ebp;
    Curtableaddr = edi;
    xp = edx;
    curcyc = pdh;
    xpc = esi - initaddrl; // subtract program counter by address
    ++numinst;
}
