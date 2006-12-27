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

; by kode54, stainless

%macro SetVGAMode 1
    mov byte[cbitmode],0

    mov ax,0013h
    int 10h

    mov dx,03d4h
    mov ax,0011h
    out dx,ax
    inc edx
    in al,dx
    and al,07fh
    mov ah,al
    mov al,11h
    dec edx
    out dx,ax

    xor ecx,ecx
    jmp %%start

%%loop
    add ecx,4

%%start
    mov eax,[%1+ecx]
    or eax,eax
    jz %%end

    cmp ax,03c0h
    je %%3c0
    cmp ax,03c2h
    je %%3c2
    cmp ax,03c3h
    je %%3c2
    jmp %%default

%%3c0
    mov dx,03dah
    in al,dx
    mov dl,0c0h
    shr eax,16
    or al,20h
    out dx,al
    shr eax,8
    out dx,al
    jmp %%loop

%%3c2
    mov dx,ax
    rol eax,8
    out dx,al
    jmp %%loop

%%default
    mov dx,ax
    shr eax,16
    out dx,ax
    jmp %%loop

%%end
%endmacro


SECTION .data

.Mode256x224c:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,03f01h,03d4h,04002h,03d4h,08203h
    dw 03d4h,04a04h,03d4h,09a05h,03d4h,00b06h,03d4h,03e07h,03d4h,00008h
    dw 03d4h,06109h,03d4h,0da10h,03d4h,0ac11h,03d4h,0bf12h,03d4h,02013h
    dw 03d4h,04014h,03d4h,00715h,03d4h,01a16h,03d4h,0a317h,03c4h,00101h
    dw 03c4h,00e04h,03cEh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode256x240c:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,03f01h,03d4h,04002h,03d4h,08203h
    dw 03d4h,04a04h,03d4h,09a05h,03d4h,00d06h,03d4h,03e07h,03d4h,00008h
    dw 03d4h,04109h,03d4h,0ea10h,03d4h,0ac11h,03d4h,0df12h,03d4h,02013h
    dw 03d4h,04014h,03d4h,0e715h,03d4h,00616h,03d4h,0a317h,03c4h,00101h
    dw 03c4h,00e04h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode256x256c:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,03f01h,03d4h,04002h,03d4h,08203h
    dw 03d4h,04a04h,03d4h,09a05h,03d4h,02306h,03d4h,0b207h,03d4h,00008h
    dw 03d4h,06109h,03d4h,00a10h,03d4h,0ac11h,03d4h,0FF12h,03d4h,02013h
    dw 03d4h,04014h,03d4h,00715h,03d4h,01a16h,03d4h,0a317h,03c4h,00101h
    dw 03c4h,00e04h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode256x256cs:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,03f01h,03d4h,04002h,03d4h,08203h
    dw 03d4h,04a04h,03d4h,09a05h,03d4h,02306h,03d4h,01d07h,03d4h,00008h
    dw 03d4h,06009h,03d4h,00a10h,03d4h,0ac11h,03d4h,0ff12h,03d4h,02013h
    dw 03d4h,04014h,03d4h,00715h,03d4h,01a16h,03d4h,0a317h,03c4h,00101h
    dw 03c4h,00e04h,03ceh,04005h,03cEh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode320x224:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,04f01h,03d4h,05002h,03d4h,08203h
    dw 03d4h,05404h,03d4h,08005h,03d4h,00b06h,03d4h,03e07h,03d4h,00008h
    dw 03d4h,04109h,03d4h,0da10h,03d4h,09c11h,03d4h,0c012h,03d4h,02813h
    dw 03d4h,00014h,03d4h,0c715h,03d4h,00416h,03d4h,0e317h,03c4h,00101h
    dw 03c4h,00604h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode320x240:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,04f01h,03d4h,05002h,03d4h,08203h
    dw 03d4h,05404h,03d4h,08005h,03d4h,00d06h,03d4h,03e07h,03d4h,00008h
    dw 03d4h,04109h,03d4h,0ea10h,03d4h,0ac11h,03d4h,0df12h,03d4h,02813h
    dw 03d4h,00014h,03d4h,0e715h,03d4h,00616h,03d4h,0e317h,03c4h,00101h
    dw 03c4h,00604h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode320x256:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,04f01h,03d4h,05002h,03d4h,08203h
    dw 03d4h,05404h,03d4h,08005h,03d4h,02306h,03d4h,0b207h,03d4h,00008h
    dw 03d4h,06109h,03d4h,00a10h,03d4h,0ac11h,03d4h,0ff12h,03d4h,02813h
    dw 03d4h,00014h,03d4h,00715h,03d4h,01a16h,03d4h,0e317h,03c4h,00101h
    dw 03c4h,00604h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0

.Mode320x256s:
    dw 03c2h,0e300h,03d4h,05f00h,03d4h,04f01h,03d4h,05002h,03d4h,08203h
    dw 03d4h,05404h,03d4h,08005h,03d4h,02306h,03d4h,01d07h,03d4h,00008h
    dw 03d4h,06009h,03d4h,00a10h,03d4h,0ac11h,03d4h,0ff12h,03d4h,02813h
    dw 03d4h,00014h,03d4h,00715h,03d4h,01a16h,03d4h,0e317h,03c4h,00101h
    dw 03c4h,00604h,03ceh,04005h,03ceh,00506h,03c0h,04110h,03c0h,00013h
    dd 0


SECTION .text
