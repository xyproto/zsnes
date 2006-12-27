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

EXTSYM dsp3_address,dsp3_byte,DSP3GetByte,DSP3SetByte
EXTSYM regaccessbankr16,regaccessbankr8,regaccessbankw16,regaccessbankw8

SECTION .text

%macro RouteAccess 1
    test ecx,8000h
    jz %1
%endmacro

NEWSYM DSP3Read8b
    RouteAccess regaccessbankr8
    mov [dsp3_address],cx
    pushad
    call DSP3GetByte
    popad
    mov al,[dsp3_byte]
    ret

NEWSYM DSP3Write8b
    RouteAccess regaccessbankw8
    mov [dsp3_address],cx
    mov [dsp3_byte],al
    pushad
    call DSP3SetByte
    popad
    ret

NEWSYM DSP3Read16b
    RouteAccess regaccessbankr16
    mov [dsp3_address],cx
    pushad
    call DSP3GetByte
    mov al,[dsp3_byte]
    mov [dsp3temp],al
    inc word[dsp3_address]
    call DSP3GetByte
    popad
    mov al,[dsp3temp]
    mov ah,[dsp3_byte]
    ret

NEWSYM DSP3Write16b
    RouteAccess regaccessbankw16
    mov [dsp3_address],cx
    mov [dsp3_byte],al
    mov [dsp3temp],ah
    pushad
    call DSP3SetByte
    mov ah,[dsp3temp]
    mov [dsp3_byte],ah
    inc word[dsp3_address]
    call DSP3SetByte
    popad
    ret

SECTION .bss
NEWSYM dsp3temp, resb 1
