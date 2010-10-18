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

EXTSYM dssel
EXTSYM c_SBHandler


;****************************************************
; Sound Blaster Interrupt Stuff
;****************************************************

section .bss

NEWSYM oldhandSBs, resw 1
NEWSYM oldhandSBo, resd 1

section .text

NEWSYM SBHandler
    cli
    push ds
    push es
    push eax

    mov ax,[cs:dssel]
    mov ds,ax
    mov es,ax

    ccall c_SBHandler

    pop eax
    pop es
    pop ds
    iretd

section .bss
NEWSYM sbselec,   resw 1        ; Selector of Memory location
NEWSYM sbpmofs,   resd 1        ; offset of Memory location

SECTION .data
NEWSYM PICMaskP,   db 21h
