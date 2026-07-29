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
EXTSYM romdata,sramb4save,curromspace,SA1Overflow
EXTSYM MemSeamB,MemSeamC,MemSeamA,MemSeamD
EXTSYM BWUsed2,BWUsed,LatestBank
EXTSYM c_membank0r8ram,c_membank0r8inv,c_membank0r8rom,c_membank0r8romram
EXTSYM c_membank0r8reg,c_membank0r16reg,c_membank0w8reg,c_membank0w16reg
EXTSYM c_memaccessbankr8,c_memaccessbankr16,c_memaccessbankw8,c_memaccessbankw16
EXTSYM c_wramaccessbankr8,c_wramaccessbankr16,c_wramaccessbankw8
EXTSYM c_wramaccessbankw16,c_eramaccessbankr8,c_eramaccessbankr16
EXTSYM c_eramaccessbankw8,c_eramaccessbankw16
EXTSYM c_sramaccessbankr8b,c_sramaccessbankr16b,c_sramaccessbankw8b
EXTSYM c_sramaccessbankw16b,c_sramaccessbankr8s,c_sramaccessbankr16s
EXTSYM c_sramaccessbankw8s,c_sramaccessbankw16s
EXTSYM c_stsramr8,c_stsramr16,c_stsramw8,c_stsramw16
EXTSYM c_stsramr8b,c_stsramr16b,c_stsramw8b,c_stsramw16b
EXTSYM c_regaccessbankr8,c_regaccessbankw8,c_regaccessbankr16,c_regaccessbankw16
EXTSYM c_membank0r8,c_membank0r16,c_membank0w8,c_membank0w16
EXTSYM c_membank0r8ramSA1,c_membank0r16ramSA1,c_membank0w8ramSA1,c_membank0w16ramSA1
EXTSYM c_SA1RAMaccessbankr8,c_SA1RAMaccessbankr16,c_SA1RAMaccessbankw8,c_SA1RAMaccessbankw16
EXTSYM c_regaccessbankr8SA1,c_regaccessbankw8SA1,c_regaccessbankr16SA1,c_regaccessbankw16SA1
EXTSYM c_membank0r8SA1,c_membank0r16SA1,c_membank0w8SA1,c_membank0w16SA1
EXTSYM c_membank0r16ram,c_membank0r16ramh,c_membank0r16rom,c_membank0r16romram
EXTSYM c_membank0w8ram,c_membank0w8inv,c_membank0w8rom,c_membank0w8romram
EXTSYM c_membank0w16ram,c_membank0w16ramh,c_membank0w16inv,c_membank0w16romram
EXTSYM SFXEnable,regptra,sfxramdata,snesmmap,wramdataa
EXTSYM cpu_mdr
EXTSYM DSP1Write8b,regptwa,writeon,DSP1Read16b
EXTSYM DSP1Read8b,DSP1Type,SA1Enable,DSP1Write16b
EXTSYM ramsize,ramsizeand,sram,sram2,ram7fa
EXTSYM SA1Status,IRAM,CurBWPtr,SA1RAMArea
EXTSYM Sdd1Mode,Sdd1Bank,Sdd1Addr,Sdd1NewAddr,memtabler8,AddrNoIncr,SDD1BankA
EXTSYM SDD1_init,SDD1_get_byte,BWShift,SA1BWPtr
EXTSYM SA1_in_cc1_dma,SA1_DMA_ADDR,SA1_DMA_VALUE,SA1_DMA_CC1

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

%macro BWCheck 0
    cmp byte[BWShift],0
    jne near .shift
.nosa1
%endmacro

; BWUsed2/BWUsed and LatestBank moved to cpu/c_memops.c.
section .text

%macro BWCheck2r8 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    ; value of 8Fh
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2r16 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    ; value of 8Fh
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    push ecx
    push ebx
    sub ecx,6000h
    inc ecx
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov ah,0Fh
    shl ah,cl
    add ebx,[SA1BWPtr]
    and ah,[ebx]
    shr ah,cl
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    push ecx
    push ebx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov ah,03h
    shl ah,cl
    add ebx,[SA1BWPtr]
    and ah,[ebx]
    shr ah,cl
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2w8 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2w16 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push eax
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    pop eax
    push ecx
    push ebx
    push edx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and ah,0Fh
    shl ah,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],ah
    pop edx
    pop ebx
    pop ecx
    ret
.2bit
    push eax
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    pop eax
    push ecx
    push ebx
    push edx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and ah,03h
    shl ah,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],ah
    pop edx
    pop ebx
    pop ecx
    ret
%endmacro

%macro writetobank0table 2
    mov ebx,%1
    mov ecx,%2
%%loop
    mov [eax],ebx
    add eax,4
    dec ecx
    jnz %%loop
%endmacro

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
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    xor al,al
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read8b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov al,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

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
    add ecx,ebx
    mov al,ch
    mov ah,ch
    mov ax,8080h
    ret
NEWSYM membank0r16chip            ; 6000-FFFF
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    xor ax,ax
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16
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
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write8b
.nodsp1
    ret
.sfxram
    push ecx
    sub cx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],al
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8
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
    add ecx,ebx
NEWSYM membank0w16rom             ; 8000-FFFF
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],ax
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16
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

%macro SRAMAccess 1
    cmp dword[curromspace],0x200000
    ja .large
    cmp  dword[ramsize],0x8000
    ja .large
    jmp .notlarge
.large
    test ecx,8000h
    jnz %1
.notlarge
%endmacro

NEWSYM sramaccessbankr8
    SRAMAccess memaccessbankr8
    push ecx
    and bl,7Fh
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr8b
    pop ecx
    ret

NEWSYM sramaccessbankr16
    SRAMAccess memaccessbankr16
    push ecx
    and bl,7Fh
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr16b
    pop ecx
    ret

NEWSYM sramaccessbankw8
    SRAMAccess memaccessbankw8
    push ecx
    and bl,7Fh
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw8b
    pop ecx
    ret

NEWSYM sramaccessbankw16
    SRAMAccess memaccessbankw16
    push ecx
    and bl,7Fh
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw16b
    pop ecx
    ret


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
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    and al,0Fh
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    shr al,4
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    mov al,[ebx+ecx]
    and al,3
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,2
    and al,3
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,4
    and al,3
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,6
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankr16b
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    and al,0Fh
    mov ah,[ebx+ecx]
    shr ah,4
    xor ebx,ebx
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov ah,[ebx+ecx+1]
    and ah,0Fh
    mov al,[ebx+ecx]
    shr al,4
    xor ebx,ebx
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    mov al,[ebx+ecx]
    and al,3
    mov ah,[ebx+ecx]
    shr ah,2
    and ah,3
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,2
    and al,2
    mov ah,[ebx+ecx]
    shr ah,4
    and ah,3
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,4
    and al,3
    mov ah,[ebx+ecx]
    shr ah,6
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,6
    mov ah,[ebx+ecx+1]
    and ah,3
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw8b
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    and al,0Fh
    and byte[ebx+ecx],0F0h
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    and al,0Fh
    shl al,4
    and byte[ebx+ecx],0Fh
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    and byte[ebx+ecx],0FCh
    and al,3
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    and byte[ebx+ecx],0F3h
    and al,3
    shl al,2
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    and byte[ebx+ecx],0CFh
    and al,3
    shl al,4
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    and byte[ebx+ecx],03Fh
    and al,3
    shl al,6
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw16b
    push ecx
    push ebx
    call SA1RAMaccessbankw8b
    pop ebx
    pop ecx
    inc ecx
    mov al,ah
    call SA1RAMaccessbankw8b
    ret

SECTION .text

%macro GetBankLog 1
    cmp bl,0C0h
    jb %%illegal
    cmp bl,0D0h
    jb %%firstbank
    cmp bl,0E0h
    jb %%secondbank
    cmp bl,0F0h
    jb %%thirdbank
    mov %1,[SDD1BankA+3]
    jmp %%done
%%firstbank
    mov %1,[SDD1BankA]
    jmp %%done
%%secondbank
    mov %1,[SDD1BankA+1]
    jmp %%done
%%thirdbank
    mov %1,[SDD1BankA+2]
    jmp %%done
%%illegal
    mov %1,0Fh
%%done
%endmacro

SECTION .text

; Software decompression version
NEWSYM memaccessbankr8sdd1
    cmp byte[AddrNoIncr],0
    je near .failed

    cmp dword[Sdd1Mode],2
    je near .decompress

    mov [Sdd1Bank],ebx
    mov [Sdd1Addr],ecx
    mov [Sdd1NewAddr],ecx

    mov dword[Sdd1Mode],2
    push edx
    push eax
    push ecx

    and ecx,0FFFFh
    xor eax,eax
    GetBankLog al
    shl eax, 20
    mov edx, [Sdd1Bank]
    and edx, 0Fh
    shl edx, 16
    add eax, edx
    add eax, [romdata]
    add eax, ecx

    ccallv SDD1_init, eax

    pop ecx
    pop eax
    pop edx

.decompress
    cmp [Sdd1Bank],ebx
    jne .nomoredec
    cmp [Sdd1Addr],ecx
    je .yesdec
.nomoredec
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    push eax
    mov eax,memtabler8+0C0h*4
    mov ebx,40h
.loopb
    mov dword[eax],memaccessbankr8
    add eax,4
    dec ebx
    jnz .loopb
    pop eax
    xor ebx,ebx
    ret
.yesdec
    push eax
    ccall SDD1_get_byte
    mov [esp], al
    pop eax
    ret

.failed
    push ebx
    call .nomoredec
    pop ebx
    jmp memaccessbankr8
