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


EXTSYM SA1DPageR8,SA1DPageR16,SA1DPageW8,SA1DPageW16


;*******************************************************
; Address Modes
;*******************************************************

; 1. Immediate Addressing -- #    - DONE IN PROGRAM

%macro addr_I_8br 0
    mov al,[esi]
    inc esi
%endmacro

%macro addr_I_16br 0
    mov eax,[esi]
    add esi,2
%endmacro

; 2. Absolute -- a (TESTED)

%macro addr_a_8br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_a_16br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_a_8bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_a_16bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    call dword near [memtablew16+ebx*4]
%endmacro

; 3. Absolute Long -- al

%macro addr_al_8br 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_al_16br 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_al_8bw 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_al_16bw 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    call dword near [memtablew16+ebx*4]
%endmacro

; 4. Direct -- d (TESTED)

%macro addr_d_8br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR8]
%endmacro

%macro addr_d_16br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
%endmacro

%macro addr_d_8bw 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageW8]
%endmacro

%macro addr_d_16bw 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageW16]
%endmacro

; 5. Accumulator -- A

%macro addr_A_8br 0
    mov al,[SA1xa]
%endmacro

%macro addr_A_16br 0
    mov eax,[SA1xa]
%endmacro

%macro addr_A_8bw 0
    mov [SA1xa],al
%endmacro

%macro addr_A_16bw 0
    mov [SA1xa],ax
%endmacro

; 7. Direct Indirect Indexed -- (d),y

%macro addr_BdBCy_8br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_BdBCy_16br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_BdBCy_8bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    pop ax
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_BdBCy_16bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    pop ax
    call dword near [memtablew16+ebx*4]
%endmacro

; 8. Direct Indirect Indexed Long -- [d],y

%macro addr_LdLCy_8br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_LdLCy_16br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_LdLCy_8bw 0
    push ax
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    add cx,[SA1xy]
    pop ax
    jnc .np
    inc bl
.np
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_LdLCy_16bw 0
    push ax
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    push cx
    call membank0r16
    pop cx
    add cx,2
    xor bl,bl
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    add cx,[SA1xy]
    pop ax
    jnc .np
    inc bl
.np
    call dword near [memtablew16+ebx*4]
%endmacro

; 9. Direct Indexed Indirect -- (d,x)

%macro addr_BdCxB_8br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_BdCxB_16br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_BdCxB_8bw 0
    push ax
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_BdCxB_16bw 0
    push ax
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    call dword near [memtablew16+ebx*4]
%endmacro

; 10. Direct Indexed With X -- d,x

%macro addr_dCx_8br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r8
%endmacro

%macro addr_dCx_16br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0r16
%endmacro

%macro addr_dCx_8bw 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0w8
%endmacro

%macro addr_dCx_16bw 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xx]
    call membank0w16
%endmacro

; 11. Direct Indexed With Y -- d,y

%macro addr_dCy_8br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xy]
    call membank0r8
%endmacro

%macro addr_dCy_16br 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xy]
    call membank0r16
%endmacro

%macro addr_dCy_8bw 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xy]
    call membank0w8
%endmacro

%macro addr_dCy_16bw 0
    mov ecx,[SA1xd]
    mov bl,[esi]
    add cx,bx
    inc esi
    add cx,[SA1xy]
    call membank0w16
%endmacro

; 12. Absolute Indexed With X -- a,x

%macro addr_aCx_8br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_aCx_16br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_aCx_8bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_aCx_16bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtablew16+ebx*4]
%endmacro

; 13. Absolute Indexed With Y -- a,y

%macro addr_aCy_8br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_aCy_16br 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_aCy_8bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_aCy_16bw 0
    mov cx,[esi]
    mov bl,[SA1xdb]
    add esi,2
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtablew16+ebx*4]
%endmacro

; 14. Absolute Long Indexed With X -- al,x

%macro addr_alCx_8br 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_alCx_16br 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_alCx_8bw 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_alCx_16bw 0
    mov cx,[esi]
    mov bl,[esi+2]
    add esi,3
    add cx,[SA1xx]
    jnc .np
    inc bl
.np
    call dword near [memtablew16+ebx*4]
%endmacro

; 18. Direct Indirect -- (d)
;                 ___________________
;    Instruction: | opcode | offset |
;                 ~~~~~~~~~~~~~~~~~~~
;                          | Direct Register   |
;                         +         |  offset  |
;                          ---------------------
;                 |  00    |  direct address   |
;    then:
;                 |  00    | (direct address)  |
;               + |  DB    |
;                -------------------------------
;    Address:     |     effective address      |

%macro addr_BdB_8br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_BdB_16br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_BdB_8bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_BdB_16bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    call dword near [SA1DPageR16]
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    call dword near [memtablew16+ebx*4]
%endmacro

; 19. Direct Indirect Long -- [d]
;                 ___________________
;    Instruction: | opcode | offset |
;                 ~~~~~~~~~~~~~~~~~~~
;                          | Direct Register   |
;                         +         |  offset  |
;                          ---------------------
;                 |  00    |  direct address   |
;    then:
;                -------------------------------
;    Address:     |       (direct address)     |

%macro addr_LdL_8br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    add cx,bx
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_LdL_16br 0
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    add cx,bx
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_LdL_8bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    add cx,bx
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    pop ax
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_LdL_16bw 0
    push ax
    mov bl,[esi]
    mov ecx,[SA1xd]
    inc esi
    add cx,bx
    push cx
    call membank0r16
    pop cx
    add cx,2
    push ax
    call membank0r8
    mov bl,al
    pop ax
    mov cx,ax
    pop ax
    call dword near [memtablew16+ebx*4]
%endmacro

; 22. Stack Relative -- d,s

%macro addr_dCs_8br 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r8
%endmacro

%macro addr_dCs_16br 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r16
%endmacro

%macro addr_dCs_8bw 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0w8
%endmacro

%macro addr_dCs_16bw 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0w16
%endmacro

; 23. Stack Relative Indirect Indexed -- (d,s),y (TESTED)

%macro addr_BdCsBCy_8br 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler8+ebx*4]
%endmacro

%macro addr_BdCsBCy_16br 0
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtabler16+ebx*4]
%endmacro

%macro addr_BdCsBCy_8bw 0
    push ax
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtablew8+ebx*4]
%endmacro

%macro addr_BdCsBCy_16bw 0
    push ax
    mov bl,[esi]
    mov cx,[SA1xs]
    inc esi
    add cx,bx
    call membank0r16
    mov cx,ax
    mov bl,[SA1xdb]
    pop ax
    add cx,[SA1xy]
    jnc .np
    inc bl
.np
    call dword near [memtablew16+ebx*4]
%endmacro


