;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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

SECTION .text

%macro RouteAccess 1
    cmp ecx,06000h
    jb %1
    cmp ecx,08000h
    jae %1
%endmacro

NEWSYM OBC1Read8b
    RouteAccess regaccessbankr8
    mov [obc1_address],cx
    pushad
    call GetOBC1
    popad
    mov al,[obc1_byte]
    ret

NEWSYM OBC1Write8b
    RouteAccess regaccessbankw8
    mov [obc1_address],cx
    mov [obc1_byte],al
    pushad
    call SetOBC1
    popad
    ret

NEWSYM OBC1Read16b
    RouteAccess regaccessbankr16
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
    RouteAccess regaccessbankw16
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
