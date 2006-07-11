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

EXTSYM MMXSupport,MMXextSupport,init18_2hz,PrintChar

%ifndef __MSDOS__
EXTSYM exit
%endif

SECTION .text

ALIGN32

;*******************************************************
; Variable section
;*******************************************************

SECTION .bss

;ALIGN32

; vcache.asm

; table.asm

; vesa2.asm

NEWSYM fulladdtab, resw 65536

; dspproc.asm

NEWSYM spcRamcmp, resb 65536
NEWSYM VolumeConvTable, resw 32768
NEWSYM dspWptr,  resd 256
NEWSYM dspRptr,  resd 256

SECTION .text

;*******************************************************
; Print Numbers                Prints # in EAX to screen
;*******************************************************
NEWSYM printnum
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
.loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
.loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    call PrintChar
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    ret

NEWSYM convertnum
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
.loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
.loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    mov [esi],dl
    inc esi
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    mov byte[esi],0
    ret

; eax = value, ecx = # of bytes
NEWSYM converthex
    mov ebx,ecx
    mov ecx,4
    sub ecx,ebx
    shl ecx,3
    shl eax,cl
    mov ecx,ebx
    xor ebx,ebx
    add ecx,ecx
.loopb
    mov ebx,eax
    and ebx,0F0000000h
    shr ebx,28
    mov dl,[.hexdat+ebx]
    mov [esi],dl
    inc esi
    shl eax,4
    dec ecx
    jnz .loopb
    mov byte[esi],0
    ret

section .data
.hexdat db '0123456789ABCDEF'

;*******************************************************
; Check Parameter          This Processes the Parameters
;*******************************************************

SECTION .text

NEWSYM DosExit ; Terminate Program
%ifdef __MSDOS__
	call init18_2hz
	mov    ax,4c00h            ;terminate
	int    21h
%else
  call exit
%endif

NEWSYM MMXCheck
    ; Check for cpu that doesn't support CPUID

    cmp byte[MMXSupport],0
    je .nommx

    ; Real way to check for presence of CPUID instruction  -kode54
    pushfd
    pop eax
    mov edx,eax
    xor eax,1 << 21
    push eax
    popfd
    pushfd
    pop eax
    xor eax,edx
    jz .nommx

    ; MMX support
    mov byte[MMXSupport],0
    mov byte[MMXextSupport],0
    mov eax,1
    CPUID

    test edx,1 << 23
    jz .nommx
    mov byte[MMXSupport],1

    ; Check if CPU has SSE (also support mmxext)
    test edx,1 << 25
    jz .tryextmmx
    mov byte[MMXextSupport],1
    jmp .nommx

.tryextmmx
    ; Test extended CPU flag
    mov eax,80000001h
    CPUID
    test edx,1 << 22
    jz .nommx
    mov byte[MMXextSupport],1
.nommx
    ret
