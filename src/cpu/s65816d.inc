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



EXTSYM Sflagnz,Sflago,Sflagc,dmadata,IRAM
%ifndef NO_DEBUGGER
EXTSYM debuggeron
%endif

;*******************************************************
; Flag Setting Macros
;*******************************************************

%macro endloop 0
;    call sa1debug
    mov bl,[esi]
    inc esi
    sub dh,[cpucycle+ebx]
    jc %%cpuover
    jmp dword near [edi+ebx*4]
%%cpuover
    ret
%endmacro

%macro restoredl 0
   mov dword[Sflagnz],0
   test dl,80h
   jz %%noneg
   or dword[Sflagnz],10000h
%%noneg
   test dl,2h
   jnz %%zero
   or dword[Sflagnz],1
%%zero
   mov dword[Sflagc],0
   test dl,01h
   jz %%nocarry
   mov dword[Sflagc],0FFh
%%nocarry
   mov dword[Sflago],0
   test dl,40h
   jz %%nov
   mov dword[Sflago],0FFh
%%nov
%endmacro

%macro makedl 0
   and dl,00111100b
   test dword[Sflagnz],18000h
   jz %%noneg
   or dl,80h
%%noneg
   test dword[Sflagnz],0FFFFh
   jnz %%nozero
   or dl,02h
%%nozero
   test dword[Sflagc],0FFh
   jz %%nocarry
   or dl,01h
%%nocarry
   test dword[Sflago],0FFh
   jz %%nov
   or dl,40h
%%nov
%endmacro

SECTION .text

NEWSYM Ssplitflags
   restoredl
   ret

NEWSYM Sjoinflags
   makedl
   ret

; Sets flags n and z according to al
%macro flagsetnz8b 0
    mov dword[Sflagnz],0
    mov [Sflagnz+1],al
    endloop
%endmacro

; Sets flags n and z according to ax
%macro flagsetnz16b 0
    mov cx,ax
    mov [Sflagnz],ecx
    endloop
%endmacro

; Sets flags n and z according to al
%macro flagsetnz8bnel 0
    mov dword[Sflagnz],0
    mov [Sflagnz+1],al
%endmacro

; Sets flags n and z according to ax
%macro flagsetnz16bnel 0
    mov cx,ax
    mov [Sflagnz],ecx
%endmacro

; Sets flags N V . . . . Z C according to flags
%macro flagsetnvzc8b 0
    mov dword[Sflagnz],0
    mov al,[SA1xa]
    seto byte[Sflago]
    mov [Sflagnz+1],al
    jc .carry
    mov dword[Sflagc],0h
    endloop
.carry
    mov dword[Sflagc],0FFh
    endloop
%endmacro

%macro flagsetnvzc16b 0
    mov cx,[SA1xa]
    seto byte[Sflago]
    mov [Sflagnz],ecx
    jc .carry
    mov dword[Sflagc],0h
    endloop
.carry
    mov dword[Sflagc],0FFh
    endloop
%endmacro

; Sets flags N V . . . . Z C according to flags
%macro flagsetnvzcs8b 0
    mov dword[Sflagnz],0
    mov al,[SA1xa]
    seto byte[Sflago]
    mov [Sflagnz+1],al
    jc .carry
    mov dword[Sflagc],0FFh
    endloop
.carry
    mov dword[Sflagc],0h
    endloop
%endmacro

%macro flagsetnvzcs16b 0
    mov cx,[SA1xa]
    seto byte[Sflago]
    mov [Sflagnz],ecx
    jc .carry
    mov dword[Sflagc],0FFh
    endloop
.carry
    mov dword[Sflagc],0h
    endloop
%endmacro

; Sets flags N V . . . . Z C for 16-bit decimal mode only
%macro flagsetnvzcd 0
    mov cx,[SA1xa]
    seto byte[Sflago]
    mov [Sflagnz],ecx
    jc .carry
    mov dword[Sflagc],0h
    endloop
.carry
    mov dword[Sflagc],0FFh
    endloop
%endmacro

; Sets flags N . . . . . Z C according to flags and don't jump to execloop
%macro flagsetnzc8b 0
    mov dword[Sflagnz],0
    mov dword[Sflagc],0h
    mov [Sflagnz+1],al
    jnc .carry
    mov dword[Sflagc],0FFh
.carry
%endmacro

; Sets flags N . . . . . Z C according to flags and don't jump to execloop
%macro flagsetnzc 0
    mov cx,ax
    mov dword[Sflagc],0h
    mov [Sflagnz],ecx
    jnc .carry
    mov dword[Sflagc],0FFh
.carry
%endmacro

; Sets flags N . . . . . Z C according to flags and jump to execloop
%macro flagsetnzcel8b 0
    mov dword[Sflagnz],0h
    jc .carry
    mov [Sflagnz+1],cl
    mov dword[Sflagc],0FFh
    endloop
.carry
    mov [Sflagnz+1],cl
    mov dword[Sflagc],0h
    endloop
%endmacro

%macro flagsetnzcel16b 0
    jc .carry
    mov [Sflagnz],ecx
    mov dword[Sflagc],0FFh
    endloop
.carry
    mov [Sflagnz],ecx
    mov dword[Sflagc],0h
    endloop
%endmacro

;*******************************************************
; Opcode Instructions
;*******************************************************

%macro ADCMacro8bnd 0
    mov cl,[Sflagc]
    add cl,cl
    adc [SA1xa],al
    flagsetnvzc8b
%endmacro

%macro ADCMacro16bnd 0
    mov cl,[Sflagc]
    add cl,cl
    adc [SA1xa],ax
    flagsetnvzc16b
%endmacro

%macro ADCMacro8bd 0
    mov cl,[Sflagc]
    add cl,cl
    mov cl,al
    mov al,[SA1xa]
    adc al,cl
    daa
    mov [SA1xa],al
    flagsetnvzc8b
    endloop
%endmacro

%macro ADCMacro16bd 0
    mov cl,[Sflagc]
    add cl,cl
    mov cx,ax
    mov al,[SA1xa]
    adc al,cl
    daa
    mov [SA1xa],al
    mov al,[SA1xa+1]
    adc al,ch
    daa
    mov [SA1xa+1],al
    flagsetnvzcd
%endmacro

%macro ANDMacro8b 0
    and al,[SA1xa]
    mov dword[Sflagnz],0
    mov [SA1xa],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro ANDMacro16b 0
    and eax,[SA1xa]
    mov [SA1xa],ax
    flagsetnz16b
%endmacro

%macro ASLMacro8b 0
    add al,al
    flagsetnzc8b
%endmacro

%macro ASLMacro16b 0
    add ax,ax
    flagsetnzc
%endmacro

%macro JumpMacro 0
    movsx eax,byte[esi]
    add esi,eax
.skip
    inc esi
    endloop
%endmacro

%macro BITMacroim8b 0
    mov al,[esi]
    test dword[Sflagnz],18000h
    jz .notSflagnz
    or dword[Sflagnz],10000h
.notSflagnz
    inc esi
    test [SA1xa],al
    jz .zero
    mov word[Sflagnz],1
    endloop
.zero
    mov word[Sflagnz],0
    endloop
%endmacro

%macro BITMacroim16b 0
    mov ax,[esi]
    test dword[Sflagnz],18000h
    jz .notSflagnz
    or dword[Sflagnz],10000h
.notSflagnz
    add esi,2
    test [SA1xa],ax
    jz .zero
    mov word[Sflagnz],1
    endloop
.zero
    mov word[Sflagnz],0
    endloop
%endmacro

%macro BITMacro8b 0
    mov dword[Sflagnz],10000h
    test al,80h
    jnz .flagn
    mov dword[Sflagnz],0
.flagn
    mov dword[Sflago],1
    test al,40h
    jnz .Sflago
    mov dword[Sflago],0
.Sflago
    test [SA1xa],al
    jz .zero
    mov word[Sflagnz],1
    endloop
.zero
    mov word[Sflagnz],0
    endloop
%endmacro

%macro BITMacro16b 0
    mov dword[Sflagnz],10000h
    test ax,8000h
    jnz .flagn
    mov dword[Sflagnz],0
.flagn
    mov dword[Sflago],1
    test ax,4000h
    jnz .Sflago
    mov dword[Sflago],0
.Sflago
    test [SA1xa],ax
    jz .zero
    mov word[Sflagnz],1
    endloop
.zero
    mov word[Sflagnz],0
    endloop
%endmacro

%macro BRKMacro 0
    inc esi
    test byte[xe],1
    jne near BRKemulmode
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr2
    mov eax,[snesmmap+ebx*4]
    jmp .nextaddr
.loweraddr2
    mov eax,[snesmap2+ebx*4]
.nextaddr
    mov ebx,esi
    sub ebx,eax
    mov [xpc],bx
    mov eax,[wramdata]
    mov bx,[SA1xs]
    mov cl,[SA1xpb]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov cl,[xpc+1]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov cl,[xpc]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    makedl
    mov [eax+ebx],dl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov [SA1xs],bx
    xor bh,bh
    mov bl,[xirqb]
    mov [SA1xpb],bl
    xor eax,eax
    mov ax,[brkv]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
BRKemulmode
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr2
    mov eax,[snesmmap+ebx*4]
    jmp .nextaddr
.loweraddr2
    mov eax,[snesmap2+ebx*4]
.nextaddr
    mov ebx,esi
    sub ebx,eax
    mov [xpc],bx
    mov eax,[wramdata]
    mov bx,[SA1xs]
    mov cl,[xpc+1]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov cl,[xpc]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    makedl
    mov [eax+ebx],dl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov [SA1xs],bx
    xor bh,bh
    mov bl,[SA1xpb]
    xor eax,eax
    mov ax,[brkv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro BRLMacro 0
    mov ebx,esi
    sub ebx,[initaddrl]
    add bx,2
    xor eax,eax
    add bx,[esi]
    mov ax,bx
    xor ebx,ebx
    mov [xpc],ax
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro CMPMacro8b 0
    mov cl,[SA1xa]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CMPMacro16b 0
    mov cx,[SA1xa]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro COPMacro 0
    inc esi
    test byte[xe],1
    jne near COPemulmode
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr2
    mov eax,[snesmmap+ebx*4]
    jmp .nextaddr
.loweraddr2
    mov eax,[snesmap2+ebx*4]
.nextaddr
    mov ebx,esi
    sub ebx,eax
    mov [xpc],bx

    mov cx,[SA1xs]
    mov al,[SA1xpb]
    call membank0w8
    dec cx
    mov al,[xpc+1]
    call membank0w8
    dec cx
    or cx,word[stackor]

    mov al,[xpc]
    call membank0w8
    dec cx
    or cx,word[stackor]

    makedl
    mov al,dl
    call membank0w8
    dec cx
    or cx,word[stackor]

    mov [SA1xs],cx

    xor bh,bh
    mov bl,[xirqb]
    mov [SA1xpb],bl
    xor eax,eax
    mov ax,[copv]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
COPemulmode
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr2
    mov eax,[snesmmap+ebx*4]
    jmp .nextaddr
.loweraddr2
    mov eax,[snesmap2+ebx*4]
.nextaddr
    mov ebx,esi
    sub ebx,eax
    mov [xpc],bx
    mov eax,[wramdata]
    mov bx,[SA1xs]
    mov cl,[xpc+1]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov cl,[xpc]
    mov [eax+ebx],cl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    makedl
    mov [eax+ebx],dl
    dec bx
    and bx,word[stackand]
    or bx,word[stackor]
    mov [SA1xs],bx
    xor bh,bh
    mov bl,[SA1xpb]
    xor eax,eax
    mov ax,[copv8]
    mov [xpc],ax
    and dl,11110011b
    or dl,00000100b
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro CPXMacro8b 0
    mov cl,[SA1xx]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CPXMacro16b 0
    mov cx,[SA1xx]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro CPYMacro8b 0
    mov cl,[SA1xy]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CPYMacro16b 0
    mov cx,[SA1xy]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro EORMacro8b 0
    xor al,[SA1xa]
    mov dword[Sflagnz],0
    mov [SA1xa],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro EORMacro16b 0
    xor eax,[SA1xa]
    mov [SA1xa],ax
    flagsetnz16b
%endmacro

%macro JMLMacro 0
    mov cx,[esi]
    xor eax,eax
    call membank0r16
    add cx,2
    push eax
    call membank0r8
    mov bl,al
    pop eax
    mov [xpc],ax
    mov [SA1xpb],bl
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JMPMacro4C 0
    xor eax,eax
    mov ax,[esi]
    mov bl,[SA1xpb]
    mov [xpc],ax
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
;    mov esi,[snesmap2+ebx*4]
;    mov [initaddrl],esi
;    add esi,eax
    mov esi,[snesmap2+ebx*4]
    cmp eax,4300h
    jae .upperlower
    mov [initaddrl],esi
    add esi,eax
    endloop
.upperlower
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JMPMacro6C 0
    mov cx,[esi]
    xor eax,eax
    call membank0r16
    mov [xpc],ax
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JMPMacro7C 0
    mov cx,[esi]
    xor eax,eax
    add cx,[SA1xx]
    mov bl,[SA1xpb]
    call dword near [memtabler16+ebx*4]
    mov [xpc],ax
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JMPMacro5C 0
    xor eax,eax
    mov bl,[esi+2]
    mov ax,[esi]
    mov [SA1xpb],bl
    mov [xpc],ax
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JSLMacro 0
    mov ebx,esi
    sub ebx,[initaddrl]
    add bx,2
    mov [xpc],bx

    mov cx,[SA1xs]
    mov al,[SA1xpb]
    call membank0w8
    dec cx
    mov al,[xpc+1]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx

    xor eax,eax
    xor bh,bh
    mov ax,[esi]
    mov bl,[esi+2]
    mov [xpc],ax
    mov [SA1xpb],bl
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    cmp ebx,0
    jne .notupper
    cmp eax,2000h
    ja .upperbank
.notupper
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.upperbank
    mov esi,IRAM
    sub esi,3000h
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JSRMacro20 0
    mov ebx,esi
    sub ebx,[initaddrl]
    inc bx
    mov [xpc],bx

    mov cx,[SA1xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx

    xor eax,eax
    mov ax,[esi]
    xor bh,bh
    mov [xpc],ax
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    cmp eax,4300h
    jae .upperlower
    mov [initaddrl],esi
    add esi,eax
    endloop
.upperlower
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JSRMacroFC 0
    mov ebx,esi
    sub ebx,[initaddrl]
    inc bx
    mov [xpc],bx

    mov cx,[SA1xs]
    mov al,[xpc+1]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx

    xor eax,eax
    xor bh,bh
    mov cx,[esi]
    mov bl,[SA1xpb]
    add cx,[SA1xx]
    call dword near [memtabler16+ebx*4]
    mov [xpc],ax
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro LDAMacro8b 0
    mov dword[Sflagnz],0
    mov [SA1xa],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro LDAMacro16b 0
    mov [SA1xa],ax
    flagsetnz16b
    endloop
%endmacro

%macro LDXMacro8b 0
    mov dword[Sflagnz],0
    mov [SA1xx],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro LDXMacro16b 0
    mov [SA1xx],ax
    flagsetnz16b
    endloop
%endmacro

%macro LDYMacro8b 0
    mov dword[Sflagnz],0
    mov [SA1xy],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro LDYMacro16b 0
    mov [SA1xy],ax
    flagsetnz16b
    endloop
%endmacro

%macro LSRMacro8b 0
    shr al,1
    flagsetnzc8b
%endmacro

%macro LSRMacro16b 0
    shr ax,1
    flagsetnzc
%endmacro

%macro MVNMacro 0
    mov ax,[esi]
    mov [SA1xdb],al
    mov bl,ah
    mov cx,[SA1xx]
    call dword near [memtabler8+ebx*4]
    mov bl,[SA1xdb]
    mov cx,[SA1xy]
    call dword near [memtablew8+ebx*4]
    test dl,10h
    jnz .pagezero
    inc word[SA1xx]
    inc word[SA1xy]
    dec word[SA1xa]
    cmp word[SA1xa],0FFFFh
    je .endmove
    dec esi
    endloop
.endmove
    add esi,2
    endloop
.pagezero
    inc byte[SA1xx]
    inc byte[SA1xy]
    dec word[SA1xa]
    cmp word[SA1xa],0FFFFh
    je .endmove
    dec esi
    endloop
%endmacro

%macro MVPMacro 0
    mov ax,[esi]
    mov [SA1xdb],al
    mov bl,ah
    mov cx,[SA1xx]
    call dword near [memtabler8+ebx*4]
    mov bl,[SA1xdb]
    mov cx,[SA1xy]
    call dword near [memtablew8+ebx*4]
    test dl,10h
    jnz .pagezero
    dec word[SA1xx]
    dec word[SA1xy]
    dec word[SA1xa]
    cmp word[SA1xa],0FFFFh
    je .endmove
    dec esi
    endloop
.endmove
    add esi,2
    endloop
.pagezero
    dec byte[SA1xx]
    dec byte[SA1xy]
    dec word[SA1xa]
    cmp word[SA1xa],0FFFFh
    je .endmove
    dec esi
    endloop
%endmacro

%macro ORAMacro8b 0
    or al,[SA1xa]
    mov dword[Sflagnz],0
    mov [SA1xa],al
    mov [Sflagnz+1],al
    endloop
%endmacro

%macro ORAMacro16b 0
    or ax,[SA1xa]
    mov [SA1xa],ax
    flagsetnz16b
%endmacro

%macro PUSHMacro8b 1
    mov eax,[wramdata]
    mov cx,[SA1xs]
    mov al,%1
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    endloop
%endmacro

%macro PUSHMacro8bp 0
    mov eax,[wramdata]
    makedl
    mov cx,[SA1xs]
    mov al,dl
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    endloop
%endmacro

%macro PUSHMacro16b 2
    mov cx,[SA1xs]
    mov al,%2
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,%1
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    endloop
%endmacro

%macro PEAMacro 0
    mov cx,[SA1xs]
    mov al,[esi+1]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,[esi]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    add esi,2
    endloop
%endmacro

%macro PEIMacro 0
    xor ah,ah
    mov al,[esi]
    mov cx,[SA1xd]
    inc esi
    add cx,ax
    call membank0r16
    mov cx,[SA1xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    endloop
%endmacro

%macro PERMacro 0
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr
    mov eax,[snesmmap+ebx*4]
    mov ebx,esi
    sub ebx,eax
    add bx,[esi]
    mov ax,bx
    add esi,2
    add ax,2
    mov cx,[SA1xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    xor ebx,ebx
    endloop
.loweraddr
    mov eax,[snesmap2+ebx*4]
    mov ebx,esi
    sub ebx,eax
    add bx,[esi]
    mov ax,bx
    add esi,2
    add ax,2
    mov cx,[SA1xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [SA1xs],cx
    xor ebx,ebx
    endloop
%endmacro

%macro POPMacro8b 1
    mov eax,[wramdata]
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [SA1xs],cx
    mov %1,al
    flagsetnz8b
    endloop
%endmacro

%macro POPMacro16b 2
    mov eax,[wramdata]
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %1,al
    mov [SA1xs],cx
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %2,al
    mov [SA1xs],cx
    mov ah,al
    mov al,%1
    flagsetnz16b
%endmacro

%macro POPMacro16bd 2
    mov eax,[wramdata]
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %1,al
    mov [SA1xs],cx
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %2,al
    call SA1UpdateDPage
    mov [SA1xs],cx
    mov ah,al
    mov al,%1
    flagsetnz16b
%endmacro

%macro POPMacroP 0
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [SA1xs],cx
    mov dl,al
    restoredl
    test byte[xe],01h
    jnz .emul
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[SA1xx+1],0
    mov byte[SA1xy+1],0
    endloop
.emul
    or dl,00110000b
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
%endmacro

%macro REPMacro 0
    mov al,[esi]
    inc esi
    test al,11000011b
    jnz .extraflags
    not al
    and dl,al
    test byte[xe],01h
    jnz .emul
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
.emul
    or dl,00110000b
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop

.extraflags
    not al
    makedl
    and dl,al
    restoredl
    test byte[xe],01h
    jnz .emul2
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
.emul2
    or dl,00110000b
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
%endmacro

%macro ROLMacro8b 0
    mov cl,[Sflagc]
    add cl,cl
    rcl al,1
    flagsetnzc8b
%endmacro

%macro ROLMacro16b 0
    mov cl,[Sflagc]
    add cl,cl
    rcl ax,1
    flagsetnzc
%endmacro

%macro RORMacro8b 0
    mov cl,[Sflagc]
    add cl,cl
    rcr al,1
    flagsetnzc8b
%endmacro

%macro RORMacro16b 0
    mov cl,[Sflagc]
    add cl,cl
    rcr ax,1
    flagsetnzc
%endmacro

%macro RTIMacro 0
    cmp byte[curexecstate],0
    jne .nn
    xor dh,dh
.nn
.nodis658162
    mov byte[curnmi],0

    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov dl,al
    restoredl

    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc],al
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc+1],al
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [SA1xpb],al
    mov [SA1xs],cx

    xor bh,bh
    xor eax,eax
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    test dl,00010000b
    jnz .setx
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    cmp eax,4300h
    jae .upperlower
    mov [initaddrl],esi
    add esi,eax
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[SA1xx+1],0
    mov byte[SA1xy+1],0
    endloop
.upperlower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
    mov [initaddrl],esi
    add esi,eax
    test dl,00010000b
    jnz .setx
    endloop
.dma
    mov esi,dmadata-4300h
    mov [initaddrl],esi
    add esi,eax
    test dl,00010000b
    jnz .setx
    endloop
%endmacro

%macro RTLMacro 0
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc],al
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc+1],al
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [SA1xpb],al
    mov [SA1xs],cx

    xor bh,bh
    xor eax,eax
    inc word[xpc]
    mov bl,[SA1xpb]
    mov ax,[xpc]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro RTSMacro 0
    mov cx,[SA1xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc],al
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc+1],al
    mov [SA1xs],cx

    xor bh,bh
    inc word[xpc]
    xor eax,eax
    mov ax,[xpc]
    mov bl,[SA1xpb]
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro SBCMacro8bnd 0
    mov cl,[Sflagc]
    sub cl,1
    sbb [SA1xa],al
    flagsetnvzcs8b
    endloop
%endmacro

%macro SBCMacro16bnd 0
    mov cl,[Sflagc]
    sub cl,1
    sbb [SA1xa],ax
    flagsetnvzcs16b
    endloop
%endmacro

%macro SBCMacro8bd 0
    mov cl,[Sflagc]
    sub cl,1
    mov cl,al
    mov al,[SA1xa]
    sbb al,cl
    das
    mov [SA1xa],al
    flagsetnvzcs8b
    endloop
%endmacro

%macro SBCMacro16bd 0
    mov cl,[Sflagc]
    sub cl,1
    mov cx,ax
    mov al,[SA1xa]
    sbb al,cl
    das
    mov [SA1xa],al
    mov al,[SA1xa+1]
    sbb al,ch
    das
    mov [SA1xa+1],al
    cmc
    flagsetnvzcd
    endloop
%endmacro

%macro SEPMacro 0
    mov al,[esi]
    inc esi
    test al,11000011b
    jnz .extraflags
    or dl,al
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[SA1xx+1],0
    mov byte[SA1xy+1],0
    endloop
.extraflags
    makedl
    or dl,al
    restoredl
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    test dl,00010000b
    jnz .setx2
    endloop
.setx2
    mov byte[SA1xx+1],0
    mov byte[SA1xy+1],0
    endloop
%endmacro

%macro STAMacro8b 0
    mov al,[SA1xa]
%endmacro

%macro STAMacro16b 0
    mov eax,[SA1xa]
%endmacro

%macro STXMacro8b 0
    mov al,[SA1xx]
%endmacro

%macro STXMacro16b 0
    mov eax,[SA1xx]
%endmacro

%macro STYMacro8b 0
    mov al,[SA1xy]
%endmacro

%macro STYMacro16b 0
    mov eax,[SA1xy]
%endmacro

%macro WAIMacro 0
    cmp byte[intrset],1
    jne .notws
    dec esi
    endloop
.notws
    test byte[intrset],0FFh
    jz .waitstate
    cmp byte[intrset],2
    je .stopint
    dec esi
    endloop
.waitstate
    mov byte[intrset],1
    dec esi
    endloop
.stopint
    mov byte[intrset],0
    endloop
%endmacro

%macro XCEMacro 0
    mov al,[Sflagc]
    mov dword[Sflagc],0
    and al,00000001b
    test byte[xe],0FFh
    jz .noemul
    mov dword[Sflagc],0FFh
.noemul
    mov [xe],al
    test byte[xe],01h
    jnz .emul
    or dl,00100000b
    mov word[stackand],0FFFFh
    mov word[stackor],0000h
    endloop
.emul
    or dl,00110000b
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    mov byte[SA1xx+1],0
    mov byte[SA1xy+1],0
    mov byte[SA1xs+1],1
    mov word[stackand],01FFh
    mov word[stackor],0100h
    endloop
%endmacro
