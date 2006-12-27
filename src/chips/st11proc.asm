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

EXTSYM seta11_address,seta11_byte,setaramdata,ST011_DR
EXTSYM ST011_MapR_60,ST011_MapW_60,ST011_MapW_68;ST011_MapR_68

SECTION .text


NEWSYM Seta11Read8_68
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov al,[ebx+ecx]
    mov [ST011_DR],al
    xor ebx,ebx
    ret

NEWSYM Seta11Write8_68
    test ecx,8000h
    jnz .nosetenablew8 ; ignore ROM writes
    mov [seta11_address],cx
    mov [seta11_byte],al
    pushad
    call ST011_MapW_68
    popad
.nosetenablew8
    ret

NEWSYM Seta11Read16_68
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov ax,[ebx+ecx]
    mov [ST011_DR],ah
    xor ebx,ebx
    ret

NEWSYM Seta11Write16_68
    test ecx,8000h
    jnz .nosetenablew16 ; ignore ROM writes
    mov [seta11_address],cx
    mov [seta11_byte],al
    mov [seta11temp],ah
    pushad
    call ST011_MapW_68
    mov ah,[seta11temp]
    mov [seta11_byte],ah
    inc word[seta11_address]
    call ST011_MapW_68
    popad
.nosetenablew16
    ret


NEWSYM Seta11Read8_60
    xor al,al
    cmp ecx,4000h
    jae .nosetenabler8
    and ecx,3
    mov [seta11_address],cx
    pushad
    call ST011_MapR_60
    popad
    mov al,[seta11_byte]
 .nosetenabler8
    ret

NEWSYM Seta11Write8_60
    cmp ecx,4000h
    jae .nosetenablew8
    and ecx,3
    mov [seta11_address],cx
    mov [seta11_byte],al
    pushad
    call ST011_MapW_60
    popad
.nosetenablew8
    ret

NEWSYM Seta11Read16_60
    xor ax,ax
    cmp ecx,4000h
    jae .nosetenabler16
    and ecx,3
    mov [seta11_address],cx
    pushad
    call ST011_MapR_60
    mov al,[seta11_byte]
    mov [seta11temp],al
    inc word[seta11_address]
    and word[seta11_address],3
    call ST011_MapR_60
    popad
    mov al,[seta11temp]
    mov ah,[seta11_byte]
.nosetenabler16
    ret

NEWSYM Seta11Write16_60
    cmp ecx,4000h
    jae .nosetenablew16
    and ecx,3
    mov [seta11_address],cx
    mov [seta11_byte],al
    mov [seta11temp],ah
    pushad
    call ST011_MapW_60
    mov ah,[seta11temp]
    mov [seta11_byte],ah
    inc word[seta11_address]
    and word[seta11_address],3
    call ST011_MapW_60
    popad
.nosetenablew16
    ret


SECTION .bss
NEWSYM seta11temp, resb 1
