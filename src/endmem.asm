;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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



SECTION .bss
NEWSYM wramdataa, resb 65536
NEWSYM ram7fa,    resb 65536
NEWSYM Inbetweendat, resd 4
NEWSYM opcjmptab, resd 256

NEWSYM Bank0datr8 ,  resd 256
NEWSYM Bank0datr16,  resd 256
NEWSYM Bank0datw8 ,  resd 256
NEWSYM Bank0datw16,  resd 256

NEWSYM tableA,  resd 256
NEWSYM tableB,  resd 256
NEWSYM tableC,  resd 256
NEWSYM tableD,  resd 256
NEWSYM tableE,  resd 256
NEWSYM tableF,  resd 256
NEWSYM tableG,  resd 256
NEWSYM tableH,  resd 256

NEWSYM tableAb,  resd 256
NEWSYM tableBb,  resd 256
NEWSYM tableCb,  resd 256
NEWSYM tableDb,  resd 256
NEWSYM tableEb,  resd 256
NEWSYM tableFb,  resd 256
NEWSYM tableGb,  resd 256
NEWSYM tableHb,  resd 256

NEWSYM tableAc,  resd 256
NEWSYM tableBc,  resd 256
NEWSYM tableCc,  resd 256
NEWSYM tableDc,  resd 256
NEWSYM tableEc,  resd 256
NEWSYM tableFc,  resd 256
NEWSYM tableGc,  resd 256
NEWSYM tableHc,  resd 256

NEWSYM SA1tableA,  resd 256
NEWSYM SA1tableB,  resd 256
NEWSYM SA1tableC,  resd 256
NEWSYM SA1tableD,  resd 256
NEWSYM SA1tableE,  resd 256
NEWSYM SA1tableF,  resd 256
NEWSYM SA1tableG,  resd 256
NEWSYM SA1tableH,  resd 256

NEWSYM tablead, resd 256
NEWSYM tableadb, resd 256
NEWSYM tableadc, resd 256
NEWSYM SA1tablead, resd 256

NEWSYM memtabler8, resd 256
NEWSYM memtablew8, resd 256
NEWSYM memtabler16, resd 256
NEWSYM memtablew16, resd 256
NEWSYM vidmemch2, resb 4096
NEWSYM vidmemch4, resb 4096
NEWSYM vidmemch8, resb 4096
NEWSYM snesmmap, resd 256
NEWSYM snesmap2, resd 256
NEWSYM cachebg1,    resb 64
NEWSYM cachebg2,    resb 64
NEWSYM cachebg3,    resb 64
NEWSYM cachebg4,    resb 64
NEWSYM sprlefttot,  resb 256
NEWSYM sprleftpr,   resb 256
NEWSYM sprleftpr1,  resb 256
NEWSYM sprleftpr2,  resb 256
NEWSYM sprleftpr3,  resb 256
NEWSYM sprpriodata, resb 288
NEWSYM sprprtabc,   resb 64
NEWSYM sprprtabu,   resb 64
NEWSYM prevpal,   resw 256          ; previous palette buffer
NEWSYM winbgdata, resb 288          ; window buffer for backgrounds
NEWSYM winspdata, resb 288          ; window buffer for sprites
NEWSYM FxTable, resd 256
NEWSYM FxTableA1, resd 256
NEWSYM FxTableA2, resd 256
NEWSYM FxTableA3, resd 256
NEWSYM FxTableb, resd 256
NEWSYM FxTablebA1, resd 256
NEWSYM FxTablebA2, resd 256
NEWSYM FxTablebA3, resd 256
NEWSYM FxTablec, resd 256
NEWSYM FxTablecA1, resd 256
NEWSYM FxTablecA2, resd 256
NEWSYM FxTablecA3, resd 256
NEWSYM FxTabled, resd 256
NEWSYM FxTabledA1, resd 256
NEWSYM FxTabledA2, resd 256
NEWSYM FxTabledA3, resd 256
NEWSYM SfxMemTable, resd 256
NEWSYM fxxand,  resd 256
NEWSYM fxbit01, resd 256
NEWSYM fxbit23, resd 256
NEWSYM fxbit45, resd 256
NEWSYM fxbit67, resd 256
NEWSYM PLOTJmpa, resd 64
NEWSYM PLOTJmpb, resd 64

NEWSYM pal16b,   resd 256
NEWSYM pal16bcl, resd 256
NEWSYM pal16bclha, resd 256
NEWSYM pal16bxcl, resd 256
NEWSYM xtravbuf, resb 576
NEWSYM BG1SXl, resw 256
NEWSYM BG2SXl, resw 256
NEWSYM BG3SXl, resw 256
NEWSYM BG4SXl, resw 256
NEWSYM BG1SYl, resw 256
NEWSYM BG2SYl, resw 256
NEWSYM BG3SYl, resw 256
NEWSYM BG4SYl, resw 256
NEWSYM BGMA,   resb 256
NEWSYM BGFB,   resb 256
NEWSYM BG3PRI, resb 256
NEWSYM BGOPT1, resw 256
NEWSYM BGOPT2, resw 256
NEWSYM BGOPT3, resw 256
NEWSYM BGOPT4, resw 256
NEWSYM BGPT1,  resw 256
NEWSYM BGPT2,  resw 256
NEWSYM BGPT3,  resw 256
NEWSYM BGPT4,  resw 256
NEWSYM BGPT1X, resw 256
NEWSYM BGPT2X, resw 256
NEWSYM BGPT3X, resw 256
NEWSYM BGPT4X, resw 256
NEWSYM BGPT1Y, resw 256
NEWSYM BGPT2Y, resw 256
NEWSYM BGPT3Y, resw 256
NEWSYM BGPT4Y, resw 256
NEWSYM BGMS1,  resw 1024
NEWSYM prdata, resb 256
NEWSYM prdatb, resb 256
NEWSYM prdatc, resb 256
NEWSYM ngpalcon2b, resd 20h
NEWSYM ngpalcon4b, resd 20h
NEWSYM ngpalcon8b, resd 20h
NEWSYM tltype2b, resb 4096
NEWSYM tltype4b, resb 2048
NEWSYM tltype8b, resb 1024

NEWSYM ngptrdat, resd 1024
NEWSYM ngceax,   resd 1024
NEWSYM ngcedi,   resd 1024
NEWSYM bgtxad,   resw 1024
NEWSYM sprtbng,  resd 256
NEWSYM sprtlng,  resb 256
NEWSYM mosszng,  resb 256
NEWSYM mosenng,  resb 256

SECTION .data
ALIGN32

NEWSYM vidmemch2s, times 4096 db 0FFh
NEWSYM vidmemch4s, times 2048 db 0FFh
NEWSYM vidmemch8s, times 1024 db 0FFh

SECTION .bss

NEWSYM mode7ab,  resd 256
NEWSYM mode7cd,  resd 256
NEWSYM mode7xy,  resd 256
NEWSYM mode7st,  resb 256

NEWSYM t16x161,  resb 256
NEWSYM t16x162,  resb 256
NEWSYM t16x163,  resb 256
NEWSYM t16x164,  resb 256

NEWSYM intrlng,  resb 256
NEWSYM mode7hr,  resb 256

NEWSYM scadsng,  resb 256
NEWSYM scadtng,  resb 256

NEWSYM scbcong,  resw 256

NEWSYM cpalval,  resd 256
NEWSYM cgfxmod,  resb 256

NEWSYM winboundary, resd 256
NEWSYM winbg1enval, resb 256
NEWSYM winbg2enval, resb 256
NEWSYM winbg3enval, resb 256
NEWSYM winbg4enval, resb 256
NEWSYM winbgobjenval, resb 256
NEWSYM winbgbackenval, resb 256
NEWSYM winlogicaval, resw 256

NEWSYM winbg1envals, resb 256
NEWSYM winbg2envals, resb 256
NEWSYM winbg3envals, resb 256
NEWSYM winbg4envals, resb 256
NEWSYM winbgobjenvals, resb 256
NEWSYM winbgbackenvals, resb 256
NEWSYM winbg1envalm, resb 256
NEWSYM winbg2envalm, resb 256
NEWSYM winbg3envalm, resb 256
NEWSYM winbg4envalm, resb 256
NEWSYM winbgobjenvalm, resb 256
NEWSYM winbgbackenvalm, resb 256

NEWSYM FillSubScr, resb 256

NEWSYM objclineptr, resd 256    ; l1,r1,l2,r2,en,log,ptr

SECTION .data
ALIGN32

NEWSYM objwlrpos  , times 256 dd 0FFFFFFFFh
NEWSYM objwen     , times 256 dw 0FFFFh    ; en,log

SECTION .bss

NEWSYM SpecialLine, resb 256

NEWSYM bgallchange, resb 256
NEWSYM bg1change, resb 256
NEWSYM bg2change, resb 256
NEWSYM bg3change, resb 256
NEWSYM bg4change, resb 256
NEWSYM bgwinchange, resb 256

NEWSYM PrevPicture, resb 64*56*2
