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

EXTSYM flagnz,flago,flagc,SfxSCMR,curnmi,execloop,initaddrl,nmiv,snesmap2
EXTSYM snesmmap,stackand,stackor,xe,xirqb,xpb,xpc,xs,irqon,irqv,irqv8
EXTSYM execloopdeb,nmiv8,membank0w8

;        NMI     Hardware        00FFFA,B    00FFEA,B     3  -> 000108
;        RES     Hardware        00FFFC.D    00FFFC,D     1
;        BRK     Software        00FFFE,F    00FFE6,7    N/A
;        IRQ     Hardware        00FFFE,F    00FFEE,F     4  -> 00010C


%macro makedl 0
   and dl,00111100b
   test dword[flagnz],18000h
   jz %%noneg
   or dl,80h
%%noneg
   test dword[flagnz],0FFFFh
   jnz %%nozero
   or dl,02h
%%nozero
   test dword[flagc],0FFh
   jz %%nocarry
   or dl,01h
%%nocarry
   test dword[flago],0FFh
   jz %%nov
   or dl,40h
%%nov
%endmacro

SECTION .text

;*******************************************************
; SwitchToNMI/VIRQ                        Calls NMI/VIRQ
;*******************************************************
NEWSYM switchtonmi
    mov byte[curnmi],1
    sub dh,100
    test byte[xe],1
    jne near NMIemulmode
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    xor ebx,ebx

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
    xor eax,eax
    mov ax,[nmiv]
    test byte[SfxSCMR],10h
    jz .nosfxnmi
;    mov ax,0108h
.nosfxnmi
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop

NEWSYM NMIemulmode
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov [xpb],bh
    mov bl,[xpb]
    xor eax,eax
    mov ax,[nmiv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop

NEWSYM switchtovirq
    mov byte[irqon],80h
    sub dh,3
    test byte[xe],1
    jne near IRQemulmode

    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
    xor eax,eax
    mov ax,[irqv]
    test byte[SfxSCMR],10h
    jz .nosfxnmi
    mov ax,010Ch
.nosfxnmi
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop

NEWSYM switchtovirqret
    mov byte[irqon],80h
    test byte[xe],1
    jne near IRQemulmode

    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
    xor eax,eax
    mov ax,[irqv]
    test byte[SfxSCMR],10h
    jz .nosfxnmi
    mov ax,010Ch
.nosfxnmi
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    ret
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    ret

NEWSYM IRQemulmode
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov [xpb],bh
    mov bl,[xpb]
    xor eax,eax
    mov ax,[irqv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloop


NEWSYM switchtovirqdeb
    mov byte[irqon],80h
    test byte[xe],1
    jne near IRQemulmodedeb

    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
    xor eax,eax
    mov ax,[irqv]
    test byte[SfxSCMR],10h
    jz .nosfxnmi
    mov ax,010Ch
.nosfxnmi
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb

NEWSYM IRQemulmodedeb
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov [xpb],bh
    mov bl,[xpb]
    xor eax,eax
    mov ax,[irqv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb

NEWSYM switchtonmideb
    mov byte[curnmi],1
    test byte[xe],1
    jne near NMIemulmodedeb
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
    xor eax,eax
    mov ax,[nmiv]
    test byte[SfxSCMR],10h
    jz .nosfxnmi
    mov ax,0108h
.nosfxnmi
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb

NEWSYM NMIemulmodedeb
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    and cx,word[stackand]
    or cx,word[stackor]

    mov [xs],cx

    xor bh,bh
    mov [xpb],bh
    mov bl,[xpb]
    xor eax,eax
    mov ax,[nmiv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    jmp execloopdeb
