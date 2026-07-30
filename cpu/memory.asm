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

%include "cpu/regs.mac"
%include "cpu/regsw.mac"
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1+
	push eax
	ccall %1
	pop eax
%endmacro
EXTSYM MemSeamB,MemSeamC,MemSeamA,MemSeamD
EXTSYM c_membank0r8ram,c_membank0r8inv,c_membank0r8rom,c_membank0r8romram
EXTSYM c_membank0r8reg,c_membank0r16reg,c_membank0w8reg,c_membank0w16reg
EXTSYM c_memaccessbankr8,c_memaccessbankr16,c_memaccessbankw8,c_memaccessbankw16
EXTSYM c_wramaccessbankr8,c_wramaccessbankr16,c_wramaccessbankw8
EXTSYM c_wramaccessbankw16,c_eramaccessbankr8,c_eramaccessbankr16
EXTSYM c_eramaccessbankw8,c_eramaccessbankw16
EXTSYM c_sramaccessbankr8,c_sramaccessbankr16,c_sramaccessbankw8,c_sramaccessbankw16
EXTSYM c_sramaccessbankr8b,c_sramaccessbankr16b,c_sramaccessbankw8b
EXTSYM c_sramaccessbankw16b,c_sramaccessbankr8s,c_sramaccessbankr16s
EXTSYM c_sramaccessbankw8s,c_sramaccessbankw16s
EXTSYM c_stsramr8,c_stsramr16,c_stsramw8,c_stsramw16
EXTSYM c_stsramr8b,c_stsramr16b,c_stsramw8b,c_stsramw16b
EXTSYM c_regaccessbankr8,c_regaccessbankw8,c_regaccessbankr16,c_regaccessbankw16
EXTSYM c_membank0r8,c_membank0r16,c_membank0w8,c_membank0w16
EXTSYM c_membank0r8ramSA1,c_membank0r16ramSA1,c_membank0w8ramSA1,c_membank0w16ramSA1
EXTSYM c_SA1RAMaccessbankr8,c_SA1RAMaccessbankr16,c_SA1RAMaccessbankw8,c_SA1RAMaccessbankw16
EXTSYM c_SA1RAMaccessbankr8b,c_SA1RAMaccessbankr16b,c_SA1RAMaccessbankw8b,c_SA1RAMaccessbankw16b
EXTSYM c_regaccessbankr8SA1,c_regaccessbankw8SA1,c_regaccessbankr16SA1,c_regaccessbankw16SA1
EXTSYM c_membank0r8SA1,c_membank0r16SA1,c_membank0w8SA1,c_membank0w16SA1
EXTSYM c_membank0r16ram,c_membank0r16ramh,c_membank0r16rom,c_membank0r16romram
EXTSYM c_membank0w8ram,c_membank0w8inv,c_membank0w8rom,c_membank0w8romram
EXTSYM c_membank0w16ram,c_membank0w16ramh,c_membank0w16inv,c_membank0w16romram
EXTSYM c_membank0r8chip,c_membank0r16inv,c_membank0r16chip
EXTSYM c_membank0w8chip,c_membank0w16chip,c_membank0w16rom
EXTSYM c_memaccessbankr8sdd1

;*******************************************************
; Register & Memory Access Banks (0 - 3F) / (80 - BF)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read


; Body of a direct-page handler that has been ported to C (cpu/mem_ops.h).
; ebx is the direct-page offset, ecx the direct page register and al/ax the
; value; all three are outputs as well, so spill and reload every one.
%macro memcop 1
    mov [MemSeamB], ebx
    mov [MemSeamC], ecx
    mov [MemSeamA], eax
    mov [MemSeamD], edx
    ccall %1
    mov ebx, [MemSeamB]
    mov ecx, [MemSeamC]
    mov eax, [MemSeamA]
    mov edx, [MemSeamD]
    ret
%endmacro

SECTION .text

NEWSYM regaccessbankr8
    memcop c_regaccessbankr8
NEWSYM regaccessbankr16
    memcop c_regaccessbankr16
NEWSYM regaccessbankw8
    memcop c_regaccessbankw8
NEWSYM regaccessbankw16
    memcop c_regaccessbankw16
;*******************************************************
; Register & Memory Bank (Bank 0)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read

; BWUsed2/BWUsed and LatestBank moved to cpu/c_memops.c.
section .text

; SA1 Stuff
NEWSYM membank0r8ramSA1             ; 0000-1FFF
    memcop c_membank0r8ramSA1
NEWSYM membank0r16ramSA1             ; 0000-1FFF
    memcop c_membank0r16ramSA1
NEWSYM membank0w8ramSA1             ; 0000-1FFF
    memcop c_membank0w8ramSA1
NEWSYM membank0w16ramSA1             ; 0000-1FFF
    memcop c_membank0w16ramSA1
; --- 8 BIT READ STUFF ---
NEWSYM membank0r8ram             ; 0000-1FFF
    memcop c_membank0r8ram
NEWSYM membank0r8reg             ; 2000-48FF
    memcop c_membank0r8reg
NEWSYM membank0r8inv             ; 4800-5FFF
    memcop c_membank0r8inv
NEWSYM membank0r8chip            ; 6000-7FFF
    memcop c_membank0r8chip
NEWSYM membank0r8rom             ; 8000-FFFF
    memcop c_membank0r8rom
NEWSYM membank0r8romram             ; 0000-1FFF
    memcop c_membank0r8romram
; --- 16 BIT READ STUFF ---
NEWSYM membank0r16ram             ; 0000-1EFF
    memcop c_membank0r16ram
NEWSYM membank0r16ramh            ; 1F00-1FFF
    memcop c_membank0r16ramh
NEWSYM membank0r16reg             ; 2000-48FF
    memcop c_membank0r16reg
NEWSYM membank0r16inv             ; 4800-5FFF
    memcop c_membank0r16inv
NEWSYM membank0r16chip            ; 6000-FFFF
    memcop c_membank0r16chip
NEWSYM membank0r16rom             ; 8000-FFFF
    memcop c_membank0r16rom
NEWSYM membank0r16romram             ; 0000-1FFF
    memcop c_membank0r16romram
; --- 8 BIT WRITE STUFF ---
NEWSYM membank0w8ram             ; 0000-1FFF
    memcop c_membank0w8ram
NEWSYM membank0w8reg             ; 2000-48FF
    memcop c_membank0w8reg
NEWSYM membank0w8inv             ; 4800-5FFF
    memcop c_membank0w8inv
NEWSYM membank0w8chip            ; 6000-FFFF
    memcop c_membank0w8chip
NEWSYM membank0w8rom             ; 8000-FFFF
    memcop c_membank0w8rom
NEWSYM membank0w8romram             ; 0000-1FFF
    memcop c_membank0w8romram
; --- 16 BIT WRITE STUFF ---
NEWSYM membank0w16ram             ; 0000-1EFF
    memcop c_membank0w16ram
NEWSYM membank0w16ramh            ; 1F00-1FFF
    memcop c_membank0w16ramh
NEWSYM membank0w16reg             ; 2000-48FF
    memcop c_membank0w16reg
NEWSYM membank0w16inv             ; 4800-5FFF
    memcop c_membank0w16inv
NEWSYM membank0w16chip            ; 6000-FFFF
    memcop c_membank0w16chip
NEWSYM membank0w16rom             ; 8000-FFFF
    memcop c_membank0w16rom
NEWSYM membank0w16romram             ; 0000-1FFF
    memcop c_membank0w16romram
NEWSYM membank0r8
    memcop c_membank0r8
NEWSYM membank0r16
    memcop c_membank0r16
NEWSYM membank0w8
    memcop c_membank0w8
NEWSYM membank0w16
    memcop c_membank0w16
NEWSYM membank0r8SA1
    memcop c_membank0r8SA1
NEWSYM membank0r16SA1
    memcop c_membank0r16SA1
NEWSYM membank0w8SA1
    memcop c_membank0w8SA1
NEWSYM membank0w16SA1
    memcop c_membank0w16SA1
;*******************************************************
; ROM Only Access Banks (40 - 6F) / (C0 - FF)
;*******************************************************

NEWSYM memaccessbankr8
    memcop c_memaccessbankr8
NEWSYM memaccessbankr16
    memcop c_memaccessbankr16
NEWSYM memaccessbankw8
    memcop c_memaccessbankw8
NEWSYM memaccessbankw16
    memcop c_memaccessbankw16
;*******************************************************
; SRAM Access Bank (70h)
;*******************************************************

NEWSYM sramaccessbankr8
    memcop c_sramaccessbankr8
NEWSYM sramaccessbankr16
    memcop c_sramaccessbankr16
NEWSYM sramaccessbankw8
    memcop c_sramaccessbankw8
NEWSYM sramaccessbankw16
    memcop c_sramaccessbankw16
NEWSYM sramaccessbankr8s
    memcop c_sramaccessbankr8s
NEWSYM sramaccessbankr16s
    memcop c_sramaccessbankr16s
NEWSYM sramaccessbankw8s
    memcop c_sramaccessbankw8s
NEWSYM sramaccessbankw16s
    memcop c_sramaccessbankw16s
NEWSYM sramaccessbankr8b
    memcop c_sramaccessbankr8b
NEWSYM sramaccessbankr16b
    memcop c_sramaccessbankr16b
NEWSYM sramaccessbankw8b
    memcop c_sramaccessbankw8b
NEWSYM sramaccessbankw16b
    memcop c_sramaccessbankw16b
%macro STsramaccess 1
    test ecx,8000h
    jz %1
%endmacro

NEWSYM stsramr8
    memcop c_stsramr8
NEWSYM stsramr16
    memcop c_stsramr16
NEWSYM stsramw8
    memcop c_stsramw8
NEWSYM stsramw16
    memcop c_stsramw16
NEWSYM stsramr8b
    memcop c_stsramr8b
NEWSYM stsramr16b
    memcop c_stsramr16b
NEWSYM stsramw8b
    memcop c_stsramw8b
NEWSYM stsramw16b
    memcop c_stsramw16b
;*******************************************************
; WorkRAM/ExpandRAM Access Bank (7Eh)
;*******************************************************

NEWSYM wramaccessbankr8
    memcop c_wramaccessbankr8
NEWSYM wramaccessbankr16
    memcop c_wramaccessbankr16
NEWSYM wramaccessbankw8
    memcop c_wramaccessbankw8
NEWSYM wramaccessbankw16
    memcop c_wramaccessbankw16
;*******************************************************
; ExpandRAM Access Bank (7Fh)
;*******************************************************
NEWSYM eramaccessbankr8
    memcop c_eramaccessbankr8
NEWSYM eramaccessbankr16
    memcop c_eramaccessbankr16
NEWSYM eramaccessbankw8
    memcop c_eramaccessbankw8
NEWSYM eramaccessbankw16
    memcop c_eramaccessbankw16
;*******************************************************
; SA-1 Bank Accesses
;*******************************************************

NEWSYM regaccessbankr8SA1
    memcop c_regaccessbankr8SA1
NEWSYM regaccessbankr16SA1
    memcop c_regaccessbankr16SA1
NEWSYM regaccessbankw8SA1
    memcop c_regaccessbankw8SA1
NEWSYM regaccessbankw16SA1
    memcop c_regaccessbankw16SA1
NEWSYM SA1RAMaccessbankr8
    memcop c_SA1RAMaccessbankr8
NEWSYM SA1RAMaccessbankr16
    memcop c_SA1RAMaccessbankr16
NEWSYM SA1RAMaccessbankw8
    memcop c_SA1RAMaccessbankw8
NEWSYM SA1RAMaccessbankw16
    memcop c_SA1RAMaccessbankw16
NEWSYM SA1RAMaccessbankr8b
    memcop c_SA1RAMaccessbankr8b
NEWSYM SA1RAMaccessbankr16b
    memcop c_SA1RAMaccessbankr16b
NEWSYM SA1RAMaccessbankw8b
    memcop c_SA1RAMaccessbankw8b
NEWSYM SA1RAMaccessbankw16b
    memcop c_SA1RAMaccessbankw16b
SECTION .text

; Software decompression version
NEWSYM memaccessbankr8sdd1
    memcop c_memaccessbankr8sdd1
