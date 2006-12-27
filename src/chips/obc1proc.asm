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

EXTSYM obc1_address,obc1_byte,SetOBC1,GetOBC1
EXTSYM regaccessbankr16,regaccessbankr8,regaccessbankw16,regaccessbankw8
EXTSYM memaccessbankr16,memaccessbankr8,memaccessbankw16,memaccessbankw8

SECTION .text

%macro RouteAccess 1
    test ecx,8000h
    jnz memaccessbank%1
    cmp ecx,6000h
    jb regaccessbank%1
%endmacro

NEWSYM OBC1Read8b
    RouteAccess r8
    mov [obc1_address],cx
    pushad
    call GetOBC1
    popad
    mov al,[obc1_byte]
    ret

NEWSYM OBC1Write8b
    RouteAccess w8
    mov [obc1_address],cx
    mov [obc1_byte],al
    pushad
    call SetOBC1
    popad
    ret

NEWSYM OBC1Read16b
    RouteAccess r16
    mov [obc1_address],cx
    pushad
    call GetOBC1
    mov al,[obc1_byte]
    mov [obc1temp],al
    inc word[obc1_address]
    call GetOBC1
    popad
    mov al,[obc1temp]
    mov ah,[obc1_byte]
    ret

NEWSYM OBC1Write16b
    RouteAccess w16
    mov [obc1_address],cx
    mov [obc1_byte],al
    mov [obc1temp],ah
    pushad
    call SetOBC1
    mov ah,[obc1temp]
    mov [obc1_byte],ah
    inc word[obc1_address]
    call SetOBC1
    popad
    ret

SECTION .bss
NEWSYM obc1temp, resb 1
