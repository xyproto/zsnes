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

EXTSYM romdata,sramb4save,curromspace,SA1Overflow
EXTSYM SFXEnable,regptra,sfxramdata,snesmmap,wramdataa
EXTSYM DSP1Write8b,regptwa,writeon,DSP1Read16b
EXTSYM Bank0datr8,Bank0datw8,Bank0datr16,Bank0datw16,xd,SA1xd
EXTSYM DSP1Read8b,DSP1Type,SA1Enable,DSP1Write16b
EXTSYM ramsize,ramsizeand,sram,sram2,ram7fa
EXTSYM SA1Status,IRAM,CurBWPtr,SA1RAMArea
EXTSYM Sdd1Mode,Sdd1Bank,Sdd1Addr,Sdd1NewAddr,memtabler8,AddrNoIncr,SDD1BankA
EXTSYM SDD1_init,SDD1_get_byte,BWShift,SA1BWPtr

;*******************************************************
; Register & Memory Access Banks (0 - 3F) / (80 - BF)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read

SECTION .text

NEWSYM regaccessbankr8
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    mov al,[wramdataa+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .hiromsram
    mov al,ch
    ret
.hiromsram
    cmp byte[SFXEnable],1
    je .sfxram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor al,al
    xor ebx,ebx
    ret
.dsp1
    xor al,al
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read8b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr8b
    pop ecx
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

NEWSYM regaccessbankr16
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    mov ax,[wramdataa+ecx]
    cmp ecx,1FFFh
    jne .notopenbus
    mov ah,al
.notopenbus
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    mov al,ch
    mov ah,ch
    ret
.hiromsram
    cmp byte[SFXEnable],1
    je .sfxram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor ax,ax
    xor ebx,ebx
    ret
.dsp1
    xor ax,ax
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read16b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr16b
    pop ecx
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

NEWSYM regaccessbankw8
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    ret
.hiromsram
    cmp byte[SFXEnable],1
    je .sfxram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor ebx,ebx
    ret
.dsp1
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write8b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw8b
    pop ecx
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],al
    xor ebx,ebx
    pop ecx
    ret

NEWSYM regaccessbankw16
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp ecx,1FFFh
    je .endwram
    mov [wramdataa+ecx],ax
    ret
.endwram
    mov [wramdataa+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    ret
.hiromsram
    cmp byte[SFXEnable],1
    je .sfxram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor al,al
    xor ebx,ebx
    ret
.dsp1
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write16b
.nodsp1
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw16b
    pop ecx
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

NEWSYM regaccessbankr8mp
    ret

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

section .bss
NEWSYM BWUsed2, resb 1
NEWSYM BWUsed, resb 1
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

section .bss
NEWSYM DPageR8, resd 1
NEWSYM DPageR16, resd 1
NEWSYM DPageW8, resd 1
NEWSYM DPageW16, resd 1
NEWSYM SA1DPageR8, resd 1
NEWSYM SA1DPageR16, resd 1
NEWSYM SA1DPageW8, resd 1
NEWSYM SA1DPageW16, resd 1
section .text

NEWSYM UpdateDPage
    push eax
    movzx eax,byte[xd+1]
    push ecx
    mov ecx,[Bank0datr8+eax*4]
    mov [DPageR8],ecx
    mov ecx,[Bank0datr16+eax*4]
    mov [DPageR16],ecx
    mov ecx,[Bank0datw8+eax*4]
    mov [DPageW8],ecx
    mov ecx,[Bank0datw16+eax*4]
    mov [DPageW16],ecx
    pop ecx
    pop eax
    ret

NEWSYM SA1UpdateDPage
    push eax
    movzx eax,byte[SA1xd+1]
    push ecx
    mov ecx,[Bank0datr8+eax*4]
    mov [SA1DPageR8],ecx
    mov ecx,[Bank0datr16+eax*4]
    mov [SA1DPageR16],ecx
    mov ecx,[Bank0datw8+eax*4]
    mov [SA1DPageW8],ecx
    mov ecx,[Bank0datw16+eax*4]
    mov [SA1DPageW16],ecx
    pop ecx
    pop eax
    ret

; SA1 Stuff
NEWSYM membank0r8ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx+ebx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx+ebx]
    ret
.invaccess
    xor al,al
    ret
NEWSYM membank0r16ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx+ebx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx+ebx]
    ret
.invaccess
    xor ax,ax
    ret
NEWSYM membank0w8ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx+ebx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx+ebx],al
.invaccess
    ret
NEWSYM membank0w16ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx+ebx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx+ebx],ax
.invaccess
    ret

; --- 8 BIT READ STUFF ---
NEWSYM membank0r8ram             ; 0000-1FFF
    mov al,[wramdataa+ebx+ecx]
    ret
NEWSYM membank0r8reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
NEWSYM membank0r8inv             ; 4800-5FFF
    add ecx,ebx
    mov al,ch
    ret
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
    add ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
NEWSYM membank0r8romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov al,[wramdataa+ecx]
    ret
.rom
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

; --- 16 BIT READ STUFF ---
NEWSYM membank0r16ram             ; 0000-1EFF
    mov ax,[wramdataa+ebx+ecx]
    ret
NEWSYM membank0r16ramh            ; 1F00-1FFF
    add ecx,ebx
    cmp ecx,1FFFh
    je .over
    mov ax,[wramdataa+ecx]
    ret
.over
    mov al,[wramdataa+ecx]
    mov ah,al ;open bus
    ret
NEWSYM membank0r16reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
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
    add ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
NEWSYM membank0r16romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov ax,[wramdataa+ecx]
    ret
.rom
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

; --- 8 BIT WRITE STUFF ---
NEWSYM membank0w8ram             ; 0000-1FFF
    mov [wramdataa+ebx+ecx],al
    ret
NEWSYM membank0w8reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
NEWSYM membank0w8inv             ; 4800-5FFF
    ret
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
    ret
NEWSYM membank0w8romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov [wramdataa+ecx],al
    ret
.rom
    ret

; --- 16 BIT WRITE STUFF ---
NEWSYM membank0w16ram             ; 0000-1EFF
    mov [wramdataa+ebx+ecx],ax
    ret
NEWSYM membank0w16ramh            ; 1F00-1FFF
    add ecx,ebx
    cmp ecx,1FFFh
    je .over
    mov [wramdataa+ecx],ax
    ret
.over
    mov [wramdataa+ecx],al
    ret
NEWSYM membank0w16reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
NEWSYM membank0w16inv             ; 4800-5FFF
    ret
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
    add cx,bx
    test cx,8000h
    jnz .rom
    mov [wramdataa+ecx],ax
    ret
.rom
    ret

NEWSYM membank0r8
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0r8SA1
    cmp ecx,2000h
    jae .regs
    mov al,[wramdataa+ecx]
    ret
.regs
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    mov al,ch
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
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

NEWSYM membank0r16
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0r16SA1
    cmp ecx,2000h
    jae .regs
    mov ax,[wramdataa+ecx]
    ret
.regs
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    xor ax,ax
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
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

NEWSYM membank0w8
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0w8SA1
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    test ecx,8000h
    jnz .romacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
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
NEWSYM membank0w16
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0w16SA1
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],ax
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    test ecx,8000h
    jnz .romacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
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

NEWSYM membank0r8SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor al,al
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

NEWSYM membank0r16SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor ax,ax
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16

NEWSYM membank0w8SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],al
    ret
.romacc
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8

NEWSYM membank0w16SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],ax
    ret
.romacc
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16

;*******************************************************
; ROM Only Access Banks (40 - 6F) / (C0 - FF)
;*******************************************************

NEWSYM memaccessbankr8
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM memaccessbankr16
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM memaccessbankw8
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM memaccessbankw16
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret

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
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr8b
    pop ecx
    ret
NEWSYM sramaccessbankr16s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr16b
    pop ecx
    ret
NEWSYM sramaccessbankw8s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw8b
    pop ecx
    ret
NEWSYM sramaccessbankw16s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw16b
    pop ecx
    ret

NEWSYM sramaccessbankr8b
    cmp dword[ramsize],0
    je .noaccess
    push ecx
    and ecx,[ramsizeand]
    mov ebx,[sram]
    mov al,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret
.noaccess
    xor al,al
    xor ebx,ebx
    ret

NEWSYM sramaccessbankr16b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    inc ecx
    and ecx,[ramsizeand]
    mov ah,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret
.noaccess
    xor ax,ax
    xor ebx,ebx
    ret

NEWSYM sramaccessbankw8b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    pop ecx
    mov dword[sramb4save],5*60
.noaccess
    xor ebx,ebx
    ret

NEWSYM sramaccessbankw16b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    inc ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],ah
    pop ecx
    mov dword[sramb4save],5*60
.noaccess
    xor ebx,ebx
    ret

%macro STsramaccess 1
    test ecx,8000h
    jz %1
%endmacro

NEWSYM stsramr8
    STsramaccess memaccessbankr8
    push ecx
    sub bl,60h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram]
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret

NEWSYM stsramr16
    STsramaccess memaccessbankr16
    push ecx
    sub bl,60h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram]
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    inc ecx
    and ecx,[ramsizeand]
    mov ah,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret

NEWSYM stsramw8
    STsramaccess memaccessbankw8
    push ecx
    sub bl,60h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram]
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    pop ecx
    mov dword[sramb4save],5*60
    xor ebx,ebx
    ret

NEWSYM stsramw16
    STsramaccess memaccessbankw16
    push ecx
    sub bl,60h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram]
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    inc ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],ah
    pop ecx
    mov dword[sramb4save],5*60
    xor ebx,ebx
    ret

NEWSYM stsramr8b
    STsramaccess memaccessbankr8
    push ecx
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram2]
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret

NEWSYM stsramr16b
    STsramaccess memaccessbankr16
    push ecx
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram2]
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    inc ecx
    and ecx,[ramsizeand]
    mov ah,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret

NEWSYM stsramw8b
    STsramaccess memaccessbankw8
    push ecx
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram2]
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    pop ecx
    mov dword[sramb4save],5*60
    xor ebx,ebx
    ret

NEWSYM stsramw16b
    STsramaccess memaccessbankw16
    push ecx
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    mov ebx,[sram2]
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    inc ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],ah
    pop ecx
    mov dword[sramb4save],5*60
    xor ebx,ebx
    ret

;*******************************************************
; WorkRAM/ExpandRAM Access Bank (7Eh)
;*******************************************************

NEWSYM wramaccessbankr8
;    mov ebx,[wramdata]
;    mov al,[ebx+ecx]
;    xor ebx,ebx
    mov al,[wramdataa+ecx]
    ret

NEWSYM wramaccessbankr16
;    mov ebx,[wramdata]
;    mov ax,[ebx+ecx]
;    xor ebx,ebx
    mov ax,[wramdataa+ecx]
    ret

NEWSYM wramaccessbankw8
;    mov ebx,[wramdata]
;    mov [ebx+ecx],al
;    xor ebx,ebx
    mov [wramdataa+ecx],al
    ret

NEWSYM wramaccessbankw16
;    mov ebx,[wramdata]
;    mov [ebx+ecx],ax
;    xor ebx,ebx
    mov [wramdataa+ecx],ax
    ret

;*******************************************************
; ExpandRAM Access Bank (7Fh)
;*******************************************************
NEWSYM eramaccessbankr8
;    mov ebx,[ram7f]
;    mov al,[ebx+ecx]
;    xor ebx,ebx
    mov al,[ram7fa+ecx]
    ret

NEWSYM eramaccessbankr16
;    mov ebx,[ram7f]
;    mov ax,[ebx+ecx]
;    xor ebx,ebx
    mov ax,[ram7fa+ecx]
    ret

NEWSYM eramaccessbankw8
;    mov ebx,[ram7f]
;    mov [ebx+ecx],al
;    xor ebx,ebx
    mov [ram7fa+ecx],al
    ret

NEWSYM eramaccessbankw16
;    mov ebx,[ram7f]
;    mov [ebx+ecx],ax
;    xor ebx,ebx
    mov [ram7fa+ecx],ax
    ret

;*******************************************************
; SA-1 Bank Accesses
;*******************************************************

NEWSYM regaccessbankr8SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor al,al
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

NEWSYM regaccessbankr16SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor ax,ax
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16

NEWSYM regaccessbankw8SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8

NEWSYM regaccessbankw16SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],ax
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16

NEWSYM SA1RAMaccessbankr8
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankr16
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw8
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw16
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret


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

SECTION .data
NEWSYM LatestBank, dd 0FFFFh
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

    pushad
    push eax
    call SDD1_init
    pop eax
    popad

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
    pushad
    call SDD1_get_byte
    mov [.tmpbyte], al
    popad
    mov al, [.tmpbyte]
    ret

.failed
    push ebx
    call .nomoredec
    pop ebx
    jmp memaccessbankr8
SECTION .bss
.tmpbyte resb 1


