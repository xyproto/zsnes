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



EXTSYM flagnz,flago,flagc,cpucycle,cycpbl,cycpblt,opcjmptab,doirqnext,dmadata

;*******************************************************
; Flag Setting Macros
;*******************************************************


%macro endloop 0
    sub dword[cycpbl],55
    jnc %%skipallspc
    mov eax,[cycpblt]
    mov bl,[ebp]
    add dword[cycpbl],eax
    ; 1260, 10000/12625
    inc ebp
    call dword [opcjmptab+ebx*4]
    xor ebx,ebx
%%skipallspc
    mov bl,[esi]
    inc esi
    sub dh,[cpucycle+ebx]
    jc %%cpuover
    jmp dword [edi+ebx*4]
%%cpuover
    ret
%endmacro

%macro restoredl 0
   mov dword[flagnz],0
   test dl,80h
   jz %%noneg
   or dword[flagnz],10000h
%%noneg
   test dl,2h
   jnz %%zero
   or dword[flagnz],1
%%zero
   mov dword[flagc],0
   test dl,01h
   jz %%nocarry
   mov dword[flagc],0FFh
%%nocarry
   mov dword[flago],0
   test dl,40h
   jz %%nov
   mov dword[flago],0FFh
%%nov
%endmacro

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

; Sets flags n and z according to al
%macro flagsetnz8b 0
    mov dword[flagnz],0
    mov [flagnz+1],al
    endloop
%endmacro

; Sets flags n and z according to ax
%macro flagsetnz16b 0
    mov cx,ax
    mov [flagnz],ecx
    endloop
%endmacro

; Sets flags n and z according to al
%macro flagsetnz8bnel 0
    mov dword[flagnz],0
    mov [flagnz+1],al
%endmacro

; Sets flags n and z according to ax
%macro flagsetnz16bnel 0
    mov cx,ax
    mov [flagnz],ecx
%endmacro

; Sets flags N V . . . . Z C according to flags
%macro flagsetnvzc8b 0
    mov dword[flagnz],0
    mov al,[xa]
    seto byte[flago]
    mov [flagnz+1],al
    jc .carry
    mov dword[flagc],0h
    endloop
.carry
    mov dword[flagc],0FFh
    endloop
%endmacro

%macro flagsetnvzc16b 0
    mov cx,[xa]
    seto byte[flago]
    mov [flagnz],ecx
    jc .carry
    mov dword[flagc],0h
    endloop
.carry
    mov dword[flagc],0FFh
    endloop
%endmacro

; Sets flags N V . . . . Z C according to flags
%macro flagsetnvzcs8b 0
    mov dword[flagnz],0
    mov al,[xa]
    seto byte[flago]
    mov [flagnz+1],al
    jc .carry
    mov dword[flagc],0FFh
    endloop
.carry
    mov dword[flagc],0h
    endloop
%endmacro

%macro flagsetnvzcs16b 0
    mov cx,[xa]
    seto byte[flago]
    mov [flagnz],ecx
    jc .carry
    mov dword[flagc],0FFh
    endloop
.carry
    mov dword[flagc],0h
    endloop
%endmacro

; Sets flags N V . . . . Z C for 16-bit decimal mode only
%macro flagsetnvzcd 0
    mov cx,[xa]
    seto byte[flago]
    mov [flagnz],ecx
    jc .carry
    mov dword[flagc],0h
    endloop
.carry
    mov dword[flagc],0FFh
    endloop
%endmacro

; Sets flags N . . . . . Z C according to flags and don't jump to execloop
%macro flagsetnzc8b 0
    mov dword[flagnz],0
    mov dword[flagc],0h
    mov [flagnz+1],al
    jnc .carry
    mov dword[flagc],0FFh
.carry
%endmacro

; Sets flags N . . . . . Z C according to flags and don't jump to execloop
%macro flagsetnzc 0
    mov cx,ax
    mov dword[flagc],0h
    mov [flagnz],ecx
    jnc .carry
    mov dword[flagc],0FFh
.carry
%endmacro

; Sets flags N . . . . . Z C according to flags and jump to execloop
%macro flagsetnzcel8b 0
    mov dword[flagnz],0h
    jc .carry
    mov [flagnz+1],cl
    mov dword[flagc],0FFh
    endloop
.carry
    mov [flagnz+1],cl
    mov dword[flagc],0h
    endloop
%endmacro

%macro flagsetnzcel16b 0
    jc .carry
    mov [flagnz],ecx
    mov dword[flagc],0FFh
    endloop
.carry
    mov [flagnz],ecx
    mov dword[flagc],0h
    endloop
%endmacro

;*******************************************************
; Opcode Instructions
;*******************************************************

%macro ADCMacro8bnd 0
    mov cl,[flagc]
    add cl,cl
    adc [xa],al
    flagsetnvzc8b
%endmacro

%macro ADCMacro16bnd 0
    mov cl,[flagc]
    add cl,cl
    adc [xa],ax
    flagsetnvzc16b
%endmacro

%macro ADCMacro8bd 0
    mov cl,[flagc]
    add cl,cl
    mov cl,al
    mov al,[xa]
    adc al,cl
    daa
    mov [xa],al
    flagsetnvzc8b
    endloop
%endmacro

%macro ADCMacro16bd 0
    mov cl,[flagc]
    add cl,cl
    mov cx,ax
    mov al,[xa]
    adc al,cl
    daa
    mov [xa],al
    mov al,[xa+1]
    adc al,ch
    daa
    mov [xa+1],al
    flagsetnvzcd
%endmacro

%macro ANDMacro8b 0
    and al,[xa]
    mov dword[flagnz],0
    mov [xa],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro ANDMacro16b 0
    and eax,[xa]
    mov [xa],ax
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
    test dword[flagnz],18000h
    jz .notflagnz
    or dword[flagnz],10000h
.notflagnz
    inc esi
    test [xa],al
    jz .zero
    mov word[flagnz],1
    endloop
.zero
    mov word[flagnz],0
    endloop
%endmacro

%macro BITMacroim16b 0
    mov ax,[esi]
    test dword[flagnz],18000h
    jz .notflagnz
    or dword[flagnz],10000h
.notflagnz
    add esi,2
    test [xa],ax
    jz .zero
    mov word[flagnz],1
    endloop
.zero
    mov word[flagnz],0
    endloop
%endmacro

%macro BITMacro8b 0
    mov dword[flagnz],10000h
    test al,80h
    jnz .flagn
    mov dword[flagnz],0
.flagn
    mov dword[flago],1
    test al,40h
    jnz .flago
    mov dword[flago],0
.flago
    test [xa],al
    jz .zero
    mov word[flagnz],1
    endloop
.zero
    mov word[flagnz],0
    endloop
%endmacro

%macro BITMacro16b 0
    mov dword[flagnz],10000h
    test ax,8000h
    jnz .flagn
    mov dword[flagnz],0
.flagn
    mov dword[flago],1
    test ax,4000h
    jnz .flago
    mov dword[flago],0
.flago
    test [xa],ax
    jz .zero
    mov word[flagnz],1
    endloop
.zero
    mov word[flagnz],0
    endloop
%endmacro

%macro BRKMacro 0
    inc esi
    test byte[xe],1
    jne near BRKemulmode
    mov bl,[xpb]
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
    xor ebx,ebx
    mov bx,[xs]
    mov cl,[xpb]
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
    mov [xs],bx
    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
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
    mov bl,[xpb]
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
    xor ebx,ebx
    mov bx,[xs]
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
    mov [xs],bx
    xor bh,bh
    mov bl,[xpb]
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
    mov ax,[xpc]
    mov bl,[xpb]
    test eax,8000h
    jz .loweraddr2
    mov eax,[snesmmap+ebx*4]
    jmp .nextaddr
.loweraddr2
    mov eax,[snesmap2+ebx*4]
.nextaddr
    mov ebx,esi
    sub ebx,eax
    add bx,2
    xor eax,eax
    add bx,[esi]
    mov ax,bx
    xor ebx,ebx
    mov [xpc],ax
    mov bl,[xpb]
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
    mov cl,[xa]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CMPMacro16b 0
    mov cx,[xa]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro COPMacro 0
    inc esi
    test byte[xe],1
    jne near COPemulmode
    mov bl,[xpb]
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
    xor ebx,ebx
    mov bx,[xs]
    mov cl,[xpb]
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
    mov [xs],bx
    xor bh,bh
    mov bl,[xirqb]
    mov [xpb],bl
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
    mov bl,[xpb]
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
    xor ebx,ebx
    mov bx,[xs]
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
    mov [xs],bx
    xor bh,bh
    mov bl,[xpb]
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
    mov cl,[xx]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CPXMacro16b 0
    mov cx,[xx]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro CPYMacro8b 0
    mov cl,[xy]
    sub cl,al
    flagsetnzcel8b
%endmacro

%macro CPYMacro16b 0
    mov cx,[xy]
    sub cx,ax
    flagsetnzcel16b
%endmacro

%macro EORMacro8b 0
    xor al,[xa]
    mov dword[flagnz],0
    mov [xa],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro EORMacro16b 0
    xor eax,[xa]
    mov [xa],ax
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
    mov [xpb],bl
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
    mov bl,[xpb]
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
;    endloop
    mov esi,[snesmap2+ebx*4]
    cmp eax,4300h
    jae .upperlower
    mov [initaddrl],esi
    add esi,eax
    endloop
.upperlower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
    mov [initaddrl],esi
    add esi,eax
    endloop
.dma
    mov esi,dmadata-4300h
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JMPMacro6C 0
    mov cx,[esi]
    xor eax,eax
    call membank0r16
    mov [xpc],ax
    mov bl,[xpb]
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
    add cx,[xx]
    mov bl,[xpb]
    call dword [memtabler16+ebx*4]
    mov [xpc],ax
    mov bl,[xpb]
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
    mov [xpb],bl
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

    mov cx,[xs]
    mov al,[xpb]
    call membank0w8

    dec cx
    or cx,word[stackor]
    mov al,[xpc+1]
    call membank0w8

    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8

    dec cx
    or cx,word[stackor]

    mov [xs],cx
    xor eax,eax
    xor bh,bh
    mov ax,[esi]
    mov bl,[esi+2]
    mov [xpc],ax
    mov [xpb],bl
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

%macro JSRMacro20 0
    mov ebx,esi
    sub ebx,[initaddrl]
    inc bx
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8

    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8

    dec cx
    or cx,word[stackor]

    xor eax,eax
    mov [xs],cx

    mov ax,[esi]
    xor bh,bh

    mov [xpc],ax
    mov bl,[xpb]
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
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
    mov [initaddrl],esi
    add esi,eax
    endloop
.dma
    mov esi,dmadata-4300h
    mov [initaddrl],esi
    add esi,eax
    endloop
%endmacro

%macro JSRMacroFC 0
    mov ebx,esi
    sub ebx,[initaddrl]
    inc bx
    mov [xpc],bx

    mov cx,[xs]
    mov al,[xpc+1]
    call membank0w8

    dec cx
    or cx,word[stackor]
    mov al,[xpc]
    call membank0w8

    dec cx
    or cx,word[stackor]

    xor eax,eax
    mov [xs],cx


    xor eax,eax
    xor bh,bh
    mov cx,[esi]
    mov bl,[xpb]
    add cx,[xx]
    call dword [memtabler16+ebx*4]
    mov [xpc],ax
    mov bl,[xpb]
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
    mov dword[flagnz],0
    mov [xa],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro LDAMacro16b 0
    mov [xa],ax
    flagsetnz16b
    endloop
%endmacro

%macro LDXMacro8b 0
    mov dword[flagnz],0
    mov [xx],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro LDXMacro16b 0
    mov [xx],ax
    flagsetnz16b
    endloop
%endmacro

%macro LDYMacro8b 0
    mov dword[flagnz],0
    mov [xy],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro LDYMacro16b 0
    mov [xy],ax
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
    mov [xdb],al
    mov bl,ah
    mov cx,[xx]
    call dword [memtabler8+ebx*4]
    mov bl,[xdb]
    mov cx,[xy]
    call dword [memtablew8+ebx*4]
    test dl,10h
    jnz near .pagezero
    inc word[xx]
    inc word[xy]
    dec word[xa]
    cmp word[xa],0FFFFh
    je .endmove
    dec esi
    endloop
.endmove
    add esi,2
    endloop
.pagezero
    inc byte[xx]
    inc byte[xy]
    dec word[xa]
    cmp word[xa],0FFFFh
    je .endmove
    dec esi
    endloop
%endmacro

%macro MVPMacro 0
    mov ax,[esi]
    mov [xdb],al
    mov bl,ah
    mov cx,[xx]
    call dword [memtabler8+ebx*4]
    mov bl,[xdb]
    mov cx,[xy]
    call dword [memtablew8+ebx*4]
    test dl,10h
    jnz near .pagezero
    dec word[xx]
    dec word[xy]
    dec word[xa]
    cmp word[xa],0FFFFh
    je .endmove
    dec esi
    endloop
.endmove
    add esi,2
    endloop
.pagezero
    dec byte[xx]
    dec byte[xy]
    dec word[xa]
    cmp word[xa],0FFFFh
    je .endmove
    dec esi
    endloop
%endmacro

%macro ORAMacro8b 0
    or al,[xa]
    mov dword[flagnz],0
    mov [xa],al
    mov [flagnz+1],al
    endloop
%endmacro

%macro ORAMacro16b 0
    or ax,[xa]
    mov [xa],ax
    flagsetnz16b
%endmacro

%macro PUSHMacro8b 1
    mov eax,[wramdata]
    mov cx,[xs]
    mov al,%1
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    endloop
%endmacro

%macro PUSHMacro8bp 0
    mov eax,[wramdata]
    makedl
    mov cx,[xs]
    mov al,dl
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    endloop
%endmacro

%macro PUSHMacro16b 2
    mov cx,[xs]
    mov al,%2
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,%1
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    endloop
%endmacro

%macro PEAMacro 0
    mov cx,[xs]
    mov al,[esi+1]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov al,[esi]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    add esi,2
    endloop
%endmacro

%macro PEIMacro 0
    xor ah,ah
    mov al,[esi]
    mov cx,[xd]
    inc esi
    add cx,ax
    call membank0r16
    mov cx,[xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    endloop
%endmacro

%macro PERMacro 0
    mov bl,[xpb]
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
    mov cx,[xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
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
    mov cx,[xs]
    push eax
    mov al,ah
    call membank0w8
    pop eax
    dec cx
    or cx,word[stackor]
    call membank0w8
    dec cx
    or cx,word[stackor]
    mov [xs],cx
    xor ebx,ebx
    endloop
%endmacro

%macro POPMacro8b 1
    mov eax,[wramdata]
    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xs],cx
    mov %1,al
    flagsetnz8b
    endloop
%endmacro

%macro POPMacro16b 2
    mov eax,[wramdata]
    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %1,al
    mov [xs],cx
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %2,al
    mov [xs],cx
    mov ah,al
    mov al,%1
    flagsetnz16b
%endmacro

%macro POPMacro16bd 2
    mov eax,[wramdata]
    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %1,al
    mov [xs],cx
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov %2,al
    call UpdateDPage
    mov [xs],cx
    mov ah,al
    mov al,%1
    flagsetnz16b
%endmacro

%macro POPMacroP 0
    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xs],cx

    xor bh,bh
    mov dl,al
    restoredl
    test byte[xe],01h
    jnz near .emul
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[xx+1],0
    mov byte[xy+1],0
    endloop
.emul
    or dl,00110000b
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
%endmacro

%macro REPMacro 0
    mov al,[esi]
    inc esi
    test al,11000011b
    jnz near .extraflags
    not al
    and dl,al
    test byte[xe],01h
    jnz .emul
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
.emul
    or dl,00110000b
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop

.extraflags
    not al
    makedl
    and dl,al
    restoredl
    test byte[xe],01h
    jnz .emul2
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
.emul2
    or dl,00110000b
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
%endmacro

%macro ROLMacro8b 0
    mov cl,[flagc]
    add cl,cl
    rcl al,1
    flagsetnzc8b
%endmacro

%macro ROLMacro16b 0
    mov cl,[flagc]
    add cl,cl
    rcl ax,1
    flagsetnzc
%endmacro

%macro RORMacro8b 0
    mov cl,[flagc]
    add cl,cl
    rcr al,1
    flagsetnzc8b
%endmacro

%macro RORMacro16b 0
    mov cl,[flagc]
    add cl,cl
    rcr ax,1
    flagsetnzc
%endmacro

%macro RTIMacro 0
    cmp byte[nmistatus],3
    jne .nodis658162
    test byte[curexecstate],01h
    jz .nodis65816
    and byte[curexecstate],0FEh
.nodis65816
    cmp byte[curexecstate],0
    jne .nn
    xor dh,dh
.nn
.nodis658162
    mov byte[curnmi],0
    test byte[xe],1
    jne near emulRTI

    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xs],cx

    mov dl,al
    restoredl

    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc],al

    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc+1],al

    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpb],al
    mov [xs],cx

    xor bh,bh
    xor eax,eax
    mov ax,[xpc]
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    mov bl,[xpb]
    mov [xpc],ax
    test eax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    mov [initaddrl],esi
    add esi,eax
    cmp byte[esi],0CBh
    jne .notwai
    mov byte[intrset],2
.notwai
    test dl,00010000b
    jnz near .setx
    endloop
.loweraddr
    mov esi,[snesmap2+ebx*4]
    cmp eax,4300h
    jae near .upperlower
    mov [initaddrl],esi
    add esi,eax
    cmp byte[esi],0CBh
    jne .notwai2
    mov byte[intrset],2
.notwai2
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[xx+1],0
    mov byte[xy+1],0
    endloop
.upperlower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
    mov byte[doirqnext],0
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

emulRTI
    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xs],cx

    mov dl,al
    or dl,00110000b
    restoredl

    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc],al

    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc+1],al
    mov [xs],cx


    xor bh,bh
    xor eax,eax
    mov ax,[xpc]
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    xor bl,bl
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

%macro RTLMacro 0

    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc],al

    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpc+1],al

    inc cx
    and cx,word[stackand]
    xor eax,eax
    call membank0r8
    mov [xpb],al

;    inc bx
;    and bx,word[stackand]
;    mov cl,[eax+ebx]

;    inc bx
;    and bx,word[stackand]
;    mov ch,[eax+ebx]

;    inc bx
;    and bx,word[stackand]
;    mov al,[eax+ebx]

;    mov [xpb],al

    mov [xs],cx
    xor bh,bh
    xor eax,eax
    mov ax,[xpc]
    inc ax
    mov bl,[xpb]
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

%macro RTSMacro 0

    mov cx,[xs]
    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc],al

    inc cx
    and cx,word[stackand]
    call membank0r8
    mov [xpc+1],al

    mov [xs],cx
    xor bh,bh
    xor eax,eax
    mov ax,[xpc]
    inc ax
    mov [xpc],ax
    mov bl,[xpb]
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
    mov cl,[flagc]
    sub cl,1
    sbb [xa],al
    flagsetnvzcs8b
    endloop
%endmacro

%macro SBCMacro16bnd 0
    mov cl,[flagc]
    sub cl,1
    sbb [xa],ax
    flagsetnvzcs16b
    endloop
%endmacro

%macro SBCMacro8bd 0
    mov cl,[flagc]
    sub cl,1
    mov cl,al
    mov al,[xa]
    sbb al,cl
    das
    mov [xa],al
    flagsetnvzcs8b
    endloop
%endmacro

%macro SBCMacro16bd 0
    mov cl,[flagc]
    sub cl,1
    mov cx,ax
    mov al,[xa]
    sbb al,cl
    das
    mov [xa],al
    mov al,[xa+1]
    sbb al,ch
    das
    mov [xa+1],al
    cmc
    flagsetnvzcd
    endloop
%endmacro

%macro SEPMacro 0
    mov al,[esi]
    inc esi
    test al,11000011b
    jnz near .extraflags
    or dl,al
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    test dl,00010000b
    jnz .setx
    endloop
.setx
    mov byte[xx+1],0
    mov byte[xy+1],0
    endloop
.extraflags
    makedl
    or dl,al
    restoredl
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    test dl,00010000b
    jnz .setx2
    endloop
.setx2
    mov byte[xx+1],0
    mov byte[xy+1],0
    endloop
%endmacro

%macro STAMacro8b 0
    mov al,[xa]
%endmacro

%macro STAMacro16b 0
    mov eax,[xa]
%endmacro

%macro STXMacro8b 0
    mov al,[xx]
%endmacro

%macro STXMacro16b 0
    mov eax,[xx]
%endmacro

%macro STYMacro8b 0
    mov al,[xy]
%endmacro

%macro STYMacro16b 0
    mov eax,[xy]
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
    mov byte[doirqnext],0
    endloop
%endmacro

%macro XCEMacro 0
    mov al,[flagc]
    mov dword[flagc],0
    and al,00000001b
    cmp [xe],al
    jne .nochange
    endloop
.nochange
    test byte[xe],0FFh
    jz .noemul
    mov dword[flagc],0FFh
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
    mov edi,[tableadc+ebx*4]
    mov byte[xx+1],0
    mov byte[xy+1],0
    mov byte[xs+1],1
    mov word[stackand],01FFh
    mov word[stackor],0100h
    endloop
%endmacro



