;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
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







NEWSYM romdatahere

; Much of this used to be in uninitialized space, until I discovered that
;   ZSNES was unstable because of them...  Odd.

Stuff db 'ZSNES v?.??? / Freeware, programmed by zsKnight, _Demo_, and Pharos',13,10,0

ALIGN32

NEWSYM wramdataa, times 65536 db 0
NEWSYM ram7fa,    times 65536 db 0
NEWSYM Inbetweendat, dd 0,0,0,0
NEWSYM opcjmptab, times 256 dd 0

NEWSYM Bank0datr8 ,times 256 dd 0
NEWSYM Bank0datr16,times 256 dd 0
NEWSYM Bank0datw8 ,times 256 dd 0
NEWSYM Bank0datw16,times 256 dd 0

NEWSYM tableA,  times 256 dd 0
NEWSYM tableB,  times 256 dd 0
NEWSYM tableC,  times 256 dd 0
NEWSYM tableD,  times 256 dd 0
NEWSYM tableE,  times 256 dd 0
NEWSYM tableF,  times 256 dd 0
NEWSYM tableG,  times 256 dd 0
NEWSYM tableH,  times 256 dd 0

NEWSYM tableAb,  times 256 dd 0
NEWSYM tableBb,  times 256 dd 0
NEWSYM tableCb,  times 256 dd 0
NEWSYM tableDb,  times 256 dd 0
NEWSYM tableEb,  times 256 dd 0
NEWSYM tableFb,  times 256 dd 0
NEWSYM tableGb,  times 256 dd 0
NEWSYM tableHb,  times 256 dd 0

NEWSYM tableAc,  times 256 dd 0
NEWSYM tableBc,  times 256 dd 0
NEWSYM tableCc,  times 256 dd 0
NEWSYM tableDc,  times 256 dd 0
NEWSYM tableEc,  times 256 dd 0
NEWSYM tableFc,  times 256 dd 0
NEWSYM tableGc,  times 256 dd 0
NEWSYM tableHc,  times 256 dd 0

NEWSYM SA1tableA,  times 256 dd 0
NEWSYM SA1tableB,  times 256 dd 0
NEWSYM SA1tableC,  times 256 dd 0
NEWSYM SA1tableD,  times 256 dd 0
NEWSYM SA1tableE,  times 256 dd 0
NEWSYM SA1tableF,  times 256 dd 0
NEWSYM SA1tableG,  times 256 dd 0
NEWSYM SA1tableH,  times 256 dd 0

NEWSYM tablead, times 256 dd 0
NEWSYM tableadb, times 256 dd 0
NEWSYM tableadc, times 256 dd 0
NEWSYM SA1tablead, times 256 dd 0

NEWSYM memtabler8, times 256 dd 0
NEWSYM memtablew8, times 256 dd 0
NEWSYM memtabler16, times 256 dd 0
NEWSYM memtablew16, times 256 dd 0
NEWSYM vidmemch2, times 4096 db 0
NEWSYM vidmemch4, times 4096 db 0
NEWSYM vidmemch8, times 4096 db 0
NEWSYM snesmmap, times 256 dd 0
NEWSYM snesmap2, times 256 dd 0
NEWSYM cachebg1,    times 64 db 0
NEWSYM cachebg2,    times 64 db 0
NEWSYM cachebg3,    times 64 db 0
NEWSYM cachebg4,    times 64 db 0
NEWSYM sprlefttot,  times 256 db 0
NEWSYM sprleftpr,   times 256 db 0
NEWSYM sprleftpr1,  times 256 db 0
NEWSYM sprleftpr2,  times 256 db 0
NEWSYM sprleftpr3,  times 256 db 0
NEWSYM sprpriodata, times 288 db 0
NEWSYM sprprtabc,   times 64 db 0
NEWSYM sprprtabu,   times 64 db 0
NEWSYM prevpal, times 256 dw 0          ; previous palette buffer
NEWSYM winbgdata, times 288 db 0          ; window buffer for backgrounds
NEWSYM winspdata, times 288 db 0          ; window buffer for sprites
NEWSYM FxTable, times 256 dd 0
NEWSYM FxTableA1, times 256 dd 0
NEWSYM FxTableA2, times 256 dd 0
NEWSYM FxTableA3, times 256 dd 0
NEWSYM FxTableb, times 256 dd 0
NEWSYM FxTablebA1, times 256 dd 0
NEWSYM FxTablebA2, times 256 dd 0
NEWSYM FxTablebA3, times 256 dd 0
NEWSYM FxTablec, times 256 dd 0
NEWSYM FxTablecA1, times 256 dd 0
NEWSYM FxTablecA2, times 256 dd 0
NEWSYM FxTablecA3, times 256 dd 0
NEWSYM FxTabled, times 256 dd 0
NEWSYM FxTabledA1, times 256 dd 0
NEWSYM FxTabledA2, times 256 dd 0
NEWSYM FxTabledA3, times 256 dd 0
NEWSYM SfxMemTable, times 256 dd 0
NEWSYM fxxand,  times 256 dd 0
NEWSYM fxbit01, times 256 dd 0
NEWSYM fxbit23, times 256 dd 0
NEWSYM fxbit45, times 256 dd 0
NEWSYM fxbit67, times 256 dd 0
NEWSYM PLOTJmpa, times 64 dd 0
NEWSYM PLOTJmpb, times 64 dd 0

NEWSYM pal16b,   times 256 dd 0
NEWSYM pal16bcl, times 256 dd 0
NEWSYM pal16bclha, times 256 dd 0
NEWSYM pal16bxcl, times 256 dd 0
NEWSYM xtravbuf, times 576 db 0
NEWSYM BG1SXl, times 256 dw 0
NEWSYM BG2SXl, times 256 dw 0
NEWSYM BG3SXl, times 256 dw 0
NEWSYM BG4SXl, times 256 dw 0
NEWSYM BG1SYl, times 256 dw 0
NEWSYM BG2SYl, times 256 dw 0
NEWSYM BG3SYl, times 256 dw 0
NEWSYM BG4SYl, times 256 dw 0
NEWSYM BGMA,   times 256 db 0
NEWSYM BGFB,   times 256 db 0
NEWSYM BG3PRI, times 256 db 0
NEWSYM BGOPT1, times 256 dw 0
NEWSYM BGOPT2, times 256 dw 0
NEWSYM BGOPT3, times 256 dw 0
NEWSYM BGOPT4, times 256 dw 0
NEWSYM BGPT1,  times 256 dw 0
NEWSYM BGPT2,  times 256 dw 0
NEWSYM BGPT3,  times 256 dw 0
NEWSYM BGPT4,  times 256 dw 0
NEWSYM BGPT1X, times 256 dw 0
NEWSYM BGPT2X, times 256 dw 0
NEWSYM BGPT3X, times 256 dw 0
NEWSYM BGPT4X, times 256 dw 0
NEWSYM BGPT1Y, times 256 dw 0
NEWSYM BGPT2Y, times 256 dw 0
NEWSYM BGPT3Y, times 256 dw 0
NEWSYM BGPT4Y, times 256 dw 0
NEWSYM BGMS1,  times 1024 dw 0
NEWSYM prdata, times 256 db 0
NEWSYM prdatb, times 256 db 0
NEWSYM prdatc, times 256 db 0
NEWSYM ngpalcon2b, times 20h dd 0
NEWSYM ngpalcon4b, times 20h dd 0
NEWSYM ngpalcon8b, times 20h dd 0
NEWSYM tltype2b, times 4096 db 0
NEWSYM tltype4b, times 2048 db 0
NEWSYM tltype8b, times 1024 db 0

NEWSYM ngptrdat, times 1024 dd 0
NEWSYM ngceax,   times 1024 dd 0
NEWSYM ngcedi,   times 1024 dd 0
NEWSYM bgtxad,   times 1024 dw 0
NEWSYM sprtbng,  times 256 dd 0
NEWSYM sprtlng,  times 256 db 0
NEWSYM mosszng,  times 256 db 0
NEWSYM mosenng,  times 256 db 0

NEWSYM vidmemch2s, times 4096 db 0FFh
NEWSYM vidmemch4s, times 2048 db 0FFh
NEWSYM vidmemch8s, times 1024 db 0FFh

NEWSYM mode7ab,  times 256 dd 0
NEWSYM mode7cd,  times 256 dd 0
NEWSYM mode7xy,  times 256 dd 0
NEWSYM mode7st,  times 256 db 0

NEWSYM t16x161,  times 256 db 0
NEWSYM t16x162,  times 256 db 0
NEWSYM t16x163,  times 256 db 0
NEWSYM t16x164,  times 256 db 0

NEWSYM intrlng,  times 256 db 0
NEWSYM mode7hr,  times 256 db 0

NEWSYM scadsng,  times 256 db 0
NEWSYM scadtng,  times 256 db 0

NEWSYM scbcong,  times 256 dw 0

NEWSYM cpalval,  times 256 dd 0
NEWSYM cgfxmod,  times 256 db 0

NEWSYM winboundary, times 256 dd 0
NEWSYM winbg1enval, times 256 db 0
NEWSYM winbg2enval, times 256 db 0
NEWSYM winbg3enval, times 256 db 0
NEWSYM winbg4enval, times 256 db 0
NEWSYM winbgobjenval, times 256 db 0
NEWSYM winbgbackenval, times 256 db 0
NEWSYM winlogicaval, times 256 dw 0

NEWSYM winbg1envals, times 256 db 0
NEWSYM winbg2envals, times 256 db 0
NEWSYM winbg3envals, times 256 db 0
NEWSYM winbg4envals, times 256 db 0
NEWSYM winbgobjenvals, times 256 db 0
NEWSYM winbgbackenvals, times 256 db 0
NEWSYM winbg1envalm, times 256 db 0
NEWSYM winbg2envalm, times 256 db 0
NEWSYM winbg3envalm, times 256 db 0
NEWSYM winbg4envalm, times 256 db 0
NEWSYM winbgobjenvalm, times 256 db 0
NEWSYM winbgbackenvalm, times 256 db 0

NEWSYM FillSubScr, times 256 db 0

NEWSYM objclineptr, times 256 dd 0    ; l1,r1,l2,r2,en,log,ptr
NEWSYM objwlrpos  , times 256 dd 0FFFFFFFFh
NEWSYM objwen     , times 256 dw 0FFFFh    ; en,log

NEWSYM SpecialLine, times 256 db 0

NEWSYM bgallchange, times 256 db 0
NEWSYM bg1change, times 256 db 0
NEWSYM bg2change, times 256 db 0
NEWSYM bg3change, times 256 db 0
NEWSYM bg4change, times 256 db 0
NEWSYM bgwinchange, times 256 db 0

NEWSYM PrevPicture, times 64*56*2 db 0
