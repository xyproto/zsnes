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

EXTSYM SA1tableA,SA1tableB,SA1tableC,SA1tableD,SA1tableE
EXTSYM SA1tableF,SA1tableG,SA1tableH,SA1tablead
EXTSYM cpucycle,SA1UpdateDPage,intrset

%include "cpu/s65816d.inc"
%include "cpu/saddress.inc"
%include "cpu/saddrni.inc"
%include "cpu/se65816.inc"

; global variables
;tableA  times 256             ; Table addresses (M:0,X:0,D:0)
;tableB  times 256             ; Table addresses (M:1,X:0,D:0)
;tableC  times 256             ; Table addresses (M:0,X:1,D:0)
;tableD  times 256             ; Table addresses (M:1,X:1,D:0)
;tableE  times 256             ; Table addresses (M:0,X:0,D:1)
;tableF  times 256             ; Table addresses (M:1,X:0,D:1)
;tableG  times 256             ; Table addresses (M:0,X:1,D:1)
;tableH  times 256             ; Table addresses (M:1,X:1,D:1)
;tablead times 256             ; Table address location according to P
;memtabler8 times 256          ; Memory Bank Locations for reading 8-bit
;memtablew8 times 256          ; Memory Bank Locations for writing 8-bit
;memtabler16 times 256          ; Memory Bank Locations for reading 16-bit
;memtablew16 times 256          ; Memory Bank Locations for reading 16-bit

; global variables

SECTION .text

eopINVALID
    ret
