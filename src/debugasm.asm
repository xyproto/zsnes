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



; Stuff that looked like too much trouble to port

%include "macros.mac"

EXTSYM snesmmap, snesmap2, memtabler8, memtablew8, regaccessbankr8, dmadata
EXTSYM initaddrl, spcPCRam, UpdateDPage, pdh, numinst, writeon
EXTSYM xp, xpb, xpc, curcyc, Curtableaddr, splitflags, execsingle, joinflags

;;; from debugger.c
EXTSYM PrevBreakPt_page, PrevBreakPt_offset, my_getch_ret, my_getch

SECTION .text

;; Wrapper for calls to routines in memtabler8

NEWSYM memtabler8_wrapper
        push    ebp
        mov     ebp, esp

        push    ebx
        push    edi
        push    esi

        movzx   ebx, BYTE [ebp+8]
        movzx   ecx, WORD [ebp+12]
        xor     eax, eax
        mov     al, bl
        call    DWORD [memtabler8+eax*4]
        and     eax, 255

        pop     esi
        pop     edi
        pop     ebx

        pop     ebp
        ret

NEWSYM memtablew8_wrapper
        push    ebp
        mov     ebp, esp

        push    ebx
        push    edi
        push    esi

        movzx   ebx, BYTE [ebp+8]
        movzx   ecx, WORD [ebp+12]
        movzx   eax, BYTE [ebp+16]
        mov     byte[writeon],1
        call    DWORD [memtablew8+ebx*4]
        mov     byte[writeon],0
        and     eax, 255

        pop     esi
        pop     edi
        pop     ebx

        pop     ebp
        ret



NEWSYM breakops_wrapper
        push    ebp
        mov     ebp, esp
        pushad
        movzx   ebx, BYTE [PrevBreakPt_page]
        movzx   ecx, WORD [PrevBreakPt_offset]
        call    breakops
        popad
        pop     ebp
        ret


;*******************************************************
; BreakOps                          Breaks at Breakpoint
;*******************************************************

NEWSYM breakops
    ; set cursor to (12,60)
    mov [PrevBreakPt_offset],cx
    mov [PrevBreakPt_page],bl

;     push ebx
;     mov ah,02h
;     mov bl,0
;     mov dh,12
;     mov dl,60
;     int 10h
;     pop ebx

    test cx,8000h
    jz .loweraddr2
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower2
.loweraddr2
    mov esi,[snesmap2+ebx*4]
.skiplower2
    add esi,ecx                 ; add program counter to address
    mov [breakarea],esi

        ;; factored out
;     mov byte[wx],14
;     mov byte[wx2],65
;     mov byte[wy],11
;     mov byte[wy2],13
;     call drawwindow
;     mov ax,[selcB800]
;     mov es,ax
;     mov edi,12*160+18*2
;     mov esi,.message1
;     mov ecx,42
;     mov ah,31
; .loopb
;     lodsb
;     stosw
;     dec ecx
;     jnz .loopb
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov esi,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov esi,dmadata-4300h
.skiplower
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    mov ebp,[spcPCRam]
    mov dl,[xp]                 ; set flags
    mov dh,[curcyc]             ; set cycles
    mov edi,[Curtableaddr]
    call UpdateDPage
    ; execute
.loopa
    call splitflags
    call execsingle
    call joinflags
    mov dh,[pdh]
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa

        ;; not DOS anymore
;     mov ah,0bh
;     int 21h
;     test al,0FFh
;     jz .skipa
;     mov ah,07h
;     int 21h

    pushad
    call my_getch
    popad
    mov eax, [my_getch_ret]

    cmp eax,27
    je .skipc
.skipa
    cmp esi,[breakarea]
    jne .loopa
.skipc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si

    ret

SECTION .data
NEWSYM breakarea, dd 0
SECTION .text


;*******************************************************
; Execute Next Opcode
;*******************************************************

NEWSYM execnextop
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov esi,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov esi,dmadata-4300h
.skiplower
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    mov ebp,[spcPCRam]
    mov dl,[xp]                 ; set flags
    mov dh,[curcyc]             ; set cycles
    mov edi,[Curtableaddr]
    call splitflags
    call execsingle
    call joinflags
    call UpdateDPage
    ; execute
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov dh,[pdh]
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    inc dword[numinst]
    ret
