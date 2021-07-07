;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;https://zsnes.bountysource.com
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;version 2 as published by the Free Software Foundation.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



%include "macros.mac"

; Initiation

SECTION .data
NEWSYM regsbackup, times 3019 db 0
NEWSYM forceromtype, db 0
; FIX STATMAT
NEWSYM autoloadstate, db 0
NEWSYM autoloadmovie, db 0
NEWSYM ZMVRawDump, db 0

; global variables

SECTION .data

NEWSYM romtype, db 0
NEWSYM resetv,  dw 0
NEWSYM abortv,  dw 0    ; abort vector
NEWSYM nmiv2,   dw 0    ; nmi vector
NEWSYM nmiv,    dw 0    ; nmi vector
NEWSYM irqv,    dw 0
NEWSYM irqv2,   dw 0
NEWSYM brkv,    dw 0    ; brk vector
NEWSYM copv,    dw 0    ; cop vector
NEWSYM abortv8, dw 0    ; abort vector emulation mode
NEWSYM nmiv8,   dw 0    ; nmi vector emulation mode
NEWSYM irqv8,   dw 0
NEWSYM brkv8,   dw 0    ; brk vector emulation mode
NEWSYM copv8,   dw 0    ; cop vector emulation mode
NEWSYM cycpb268, db 109  ; 110
NEWSYM cycpb358, db 149  ; 155
NEWSYM cycpbl2,  db 109  ; percentage left of CPU/SPC to run  (3.58 = 175)
NEWSYM cycpblt2, db 149  ; percentage of CPU/SPC to run
NEWSYM writeon, db 0
NEWSYM totlines, dw 263 ; total # of lines

;This is saved in states
NEWSYM curcyc,  db 0
NEWSYM cacheud, db 1
NEWSYM ccud,    db 0
NEWSYM spcon,   db 0

; 65816 registers
NEWSYM xat,      dw 0
NEWSYM xdbt,     db 0
NEWSYM xpbt,     db 0
NEWSYM xst,      dw 0
NEWSYM xdt,      dw 0
NEWSYM xxt,      dw 0
NEWSYM xyt,      dw 0
NEWSYM xpc,      dw 0
NEWSYM debugger, db 0
NEWSYM curnmi,   db 0           ; if in NMI(1) or not(0)

ALIGN32
NEWSYM cycpbl,  dd 110  ; percentage left of CPU/SPC to run  (3.58 = 175)
NEWSYM cycpblt, dd 110

ALIGN32
NEWSYM xa,       dd 0
NEWSYM xdb,      dd 0
NEWSYM xpb,      dd 0
NEWSYM xs,       dd 0
NEWSYM xd,       dd 0
NEWSYM xx,       dd 0
NEWSYM xy,       dd 0
NEWSYM flagnz,   dd 0
NEWSYM flago,    dd 0
NEWSYM flagc,    dd 0
NEWSYM bankkp,   dd 0
NEWSYM Sflagnz,  dd 0
NEWSYM Sflago,   dd 0
NEWSYM Sflagc,   dd 0

;*******************************************************
; Init 65816                   Initializes the Registers
;*******************************************************

SECTION .data

NEWSYM disablespcclr,  db 0
NEWSYM ENVDisable, db 0

SECTION .bss
NEWSYM IPSPatched, resb 1
NEWSYM SramExists,    resb 1
NEWSYM NumofBanks,    resd 1
NEWSYM NumofBytes,    resd 1

;*******************************************************
; Show Information
;*******************************************************
;
; Maker Code = FFB0-FFB1
; Game Code = FFB2-FFB5
; Expansion RAM Size = FFBD (0=none, 1=16kbit, 3=64kbit, 5=256kbit,etc.
; Map Mode = FFD5 2.68-20h=map20h,21h=map21h,22h=reserved,23h=SA-1,25h=map25h
;                 3.58-30h=map20h,31h=map21h,35h=map25h,highspeed
; Rom Mask Version = FFDB
; FFD6 (ROM Type) : 0*=DSP,1*=SFX,2*=OBC1,3*=SA-1,E*-F*=other
;                   *3=ROM,*4=ROM+RAM,*5=ROM+RAM+BATTERY,*6=ROM+BATTERY
;                   F3=C4


NEWSYM DSP1Type, resb 1

NEWSYM yesoutofmemory, resb 1
