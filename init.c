// Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
//
// http://www.zsnes.com
// http://sourceforge.net/projects/zsnes
// https://zsnes.bountysource.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// version 2 as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdint.h>
#include <stddef.h>

#ifdef __GNUC__
#define ALIGN32 __attribute__((aligned(32)))
#else
#define ALIGN32
#endif

/* Initiation */
uint8_t regsbackup[3019] = {0};
uint8_t forceromtype = 0;
/* FIX STATMAT */
uint8_t autoloadstate = 0;
uint8_t autoloadmovie = 0;
uint8_t ZMVRawDump = 0;

/* global variables */
uint8_t romtype = 0;
uint16_t resetv = 0;
uint16_t abortv = 0;    /* abort vector */
uint16_t nmiv2 = 0;      /* nmi vector */
uint16_t nmiv = 0;       /* nmi vector */
uint16_t irqv = 0;
uint16_t irqv2 = 0;
uint16_t brkv = 0;       /* brk vector */
uint16_t copv = 0;       /* cop vector */
uint16_t abortv8 = 0;    /* abort vector emulation mode */
uint16_t nmiv8 = 0;      /* nmi vector emulation mode */
uint16_t irqv8 = 0;
uint16_t brkv8 = 0;      /* brk vector emulation mode */
uint16_t copv8 = 0;      /* cop vector emulation mode */
uint8_t cycpb268 = 109;  /* 110 */
uint8_t cycpb358 = 149;  /* 155 */
uint8_t cycpbl2 = 109;   /* percentage left of CPU/SPC to run  (3.58 = 175) */
uint8_t cycpblt2 = 149;  /* percentage of CPU/SPC to run */
uint8_t writeon = 0;
uint16_t totlines = 263; /* total # of lines */

/* This is saved in states */
uint8_t curcyc = 0;
uint8_t cacheud = 1;
uint8_t ccud = 0;
uint8_t spcon = 0;

/* 65816 registers */
uint16_t xat = 0;
uint8_t xdbt = 0;
uint8_t xpbt = 0;
uint16_t xst = 0;
uint16_t xdt = 0;
uint16_t xxt = 0;
uint16_t xyt = 0;
uint16_t xpc = 0;
uint8_t debugger = 0;
uint8_t curnmi = 0;      /* if in NMI(1) or not(0) */

ALIGN32 uint32_t cycpbl = 110;  /* percentage left of CPU/SPC to run  (3.58 = 175) */
ALIGN32 uint32_t cycpblt = 110;

ALIGN32 uint32_t xa = 0;
ALIGN32 uint32_t xdb = 0;
ALIGN32 uint32_t xpb = 0;
ALIGN32 uint32_t xs = 0;
ALIGN32 uint32_t xd = 0;
ALIGN32 uint32_t xx = 0;
ALIGN32 uint32_t xy = 0;
ALIGN32 uint32_t flagnz = 0;
ALIGN32 uint32_t flago = 0;
ALIGN32 uint32_t flagc = 0;
ALIGN32 uint32_t bankkp = 0;
ALIGN32 uint32_t Sflagnz = 0;
ALIGN32 uint32_t Sflago = 0;
ALIGN32 uint32_t Sflagc = 0;

/* Init 65816                   Initializes the Registers */
uint8_t disablespcclr = 0;
uint8_t ENVDisable = 0;

uint8_t IPSPatched = 0;
uint8_t SramExists = 0;
uint32_t NumofBanks = 0;
uint32_t NumofBytes = 0;

/* Show Information */
/* Maker Code = FFB0-FFB1
   Game Code = FFB2-FFB5
   Expansion RAM Size = FFBD (0=none, 1=16kbit, 3=64kbit, 5=256kbit,etc.
   Map Mode = FFD5 2.68-20h=map20h,21h=map21h,22h=reserved,23h=SA-1,25h=map25h
               3.58-30h=map20h,31h=map21h,35h=map25h,highspeed
   Rom Mask Version = FFDB
   FFD6 (ROM Type) : 0*=DSP,1*=SFX,2*=OBC1,3*=SA-1,E*-F*=other
                     *3=ROM,*4=ROM+RAM,*5=ROM+RAM+BATTERY,*6=ROM+BATTERY
                     F3=C4 */
uint8_t DSP1Type = 0;

uint8_t yesoutofmemory = 0;
