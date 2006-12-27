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

EXTSYM initaddrl,wramdata,IRAM,SA1DoIRQ,SNSRegP,SNSRegPCS
EXTSYM SA1Ptr,SNSPtr,snesmap2,SA1tablead,SA1xpb,SA1RegP,wramdataa,SA1TimerVal
EXTSYM SA1RegPCS,SA1BWPtr,SNSBWPtr,CurBWPtr,SA1NMIV,SA1IRQV
EXTSYM membank0w8,SA1LBound,SA1UBound,SA1SH,SA1SHb,stackor,stackand,snesmmap
EXTSYM SA1xs,SA1IRQExec,SA1Message,Sflagnz,Sflagc,Sflago

; In exec loop, jump to execloop if SA1Status != 0
; *** Disable spc700 if possible ***

SECTION .bss
NEWSYM SA1Status, resb 1  ; 0 = 65816, 1 = SA1A, 2 = SA1B

NEWSYM CurrentExecSA1, resb 1
NEWSYM CurrentCPU, resb 1

;ALIGN32
NEWSYM prevedi, resd 1

SECTION .text

%macro SA1Debugb 0
    pushad
    sub esi,[initaddrl]
    mov [SA1xpc],esi
    call nextopcodesa1
    popad
    mov bl,[esi]
    xor dh,dh
    inc esi
    call dword near [edi+ebx*4]
    dec esi
%endmacro

%macro SA1Debug 0
    ; debug version
    test byte[debugds],01h
    jz near .nodebug
    cmp byte[debuggeron],0
    je near .nodebug
    SA1Debugb
    SA1Debugb
    SA1Debugb
    SA1Debugb
    SA1Debugb
    SA1Debugb
    SA1Debugb
    SA1Debugb
    jmp .debug
.nodebug
%endmacro


NEWSYM SA1Swap
    mov ecx,[SA1BWPtr]
    mov eax,[SA1Ptr]        ; small speed hack
    test byte[SA1DoIRQ],1
    jnz near .sa1exec3
    cmp byte[IRAM],0
    jne .sa1exec2
    cmp dword[eax],0FCF000A5h
    je near .nosa1exec
    cmp dword[eax-2],0FCF000A5h
    je near .nosa1exec
.sa1exec2
    cmp byte[SA1SHb],1
    je near .nosa1execb

    cmp word[ecx+72A4h],0
    jnz .sa1exec
    cmp dword[eax],0F072A4ADh
    je near .nosa1execb
.sa1exec
    cmp byte[IRAM+72h],0
    jne .sa1exec3
    cmp dword[eax],0F03072ADh
    je near .nosa1execb
.sa1exec3
.yesdebugr
    xor ecx,ecx
    ; store all snes 65816 stuff
    mov [SNSRegP],dl
    mov eax,[initaddrl]
    mov [prevedi],edi
    mov [SNSRegPCS],eax
    mov [SNSPtr],esi
    ; restore all sa1 65816 stuff
    mov dl,[SA1RegP]
    mov eax,[SA1RegPCS]
    mov [initaddrl],eax
    mov eax,[SA1BWPtr]
    mov [CurBWPtr],eax
    mov esi,[SA1Ptr]
    mov dword[snesmap2],IRAM
    mov dword[wramdata],IRAM
    ; Check if IRQ is executed on SA-1
    xor eax,eax
    mov al,dl
    add dh,20
    mov edi,[SA1tablead+eax*4]
    mov byte[SA1Status],1
    test dword[SA1DoIRQ],0FF000003h
    jnz near .switchirq
.returnirq

;    SA1Debug

;    cmp byte[SA1SH],1
;    je near .speedhack

     ; non debug version
    mov bl,[esi]
    inc esi
    call dword near [edi+ebx*4]
    dec esi
.debug

    ; store all sa1 65816 stuff
    mov [SA1RegP],dl
    mov eax,[initaddrl]
    mov [SA1RegPCS],eax
    mov [SA1Ptr],esi
    ; restore all snes 65816 stuff
    mov dl,[SNSRegP]
    mov eax,[SNSRegPCS]
    mov [initaddrl],eax
    mov eax,[SNSBWPtr]
    mov [CurBWPtr],eax
    mov dword[wramdata],wramdataa
    mov esi,[SNSPtr]
    mov eax,[wramdata]
    mov [snesmap2],eax
    mov edi,[prevedi]
    xor eax,eax
    add dh,11
    inc byte[CurrentExecSA1]
    mov byte[SA1Status],0
    add dword[SA1TimerVal],23
    ret

.speedhack
    add dh,90

    mov bl,[esi]
    inc esi
    call dword near [edi+ebx*4]
    dec esi
    ; store all sa1 65816 stuff
    mov [SA1RegP],dl
    mov eax,[initaddrl]
    mov [SA1RegPCS],eax
    mov [SA1Ptr],esi
    ; restore all snes 65816 stuff
    mov dl,[SNSRegP]
    mov eax,[SNSRegPCS]
    mov [initaddrl],eax
    mov eax,[SNSBWPtr]
    mov [CurBWPtr],eax
    mov dword[wramdata],wramdataa
    mov esi,[SNSPtr]
    mov eax,[wramdata]
    mov [snesmap2],eax
    mov edi,[prevedi]
    xor eax,eax
    add byte[CurrentExecSA1],4
    mov byte[SA1Status],0
    add dword[SA1TimerVal],23
    xor dh,dh
    mov dh,18
    cmp esi,dword[SA1LBound]
    jb .stoph
    cmp esi,dword[SA1UBound]
    ja .stoph
    ret
.stoph
    mov byte[SA1SH],0
    ret
.nosa1execb
    xor ecx,ecx
    add dh,15
    add byte[CurrentExecSA1],2
    mov byte[SA1Status],0
    ret
.nosa1exec
    xor ecx,ecx
    add dh,18
    add byte[CurrentExecSA1],2
    mov byte[SA1Status],0
    ret
.switchirq
    test dword[SA1DoIRQ],3
    jz .notirq
    test dword[SA1DoIRQ],1
    jz .nmi
    and byte[SA1DoIRQ],0FEh
    call SA1switchtovirq
    jmp .returnirq
.nmi
    and byte[SA1DoIRQ],0FDh
    call SA1switchtonmi
    jmp .returnirq
.notirq
    dec byte[SA1DoIRQ+3]
    jz .hack
    jmp .returnirq
.hack
    or byte[SA1DoIRQ],8
    jmp .returnirq

SECTION .bss
NEWSYM SA1xpc, resd 1
SECTION .text

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

NEWSYM SA1switchtonmi
    mov al,[SA1Message]
    mov [SA1Message+2],al
    mov byte[SA1IRQExec+2],1
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [SA1xpc],bx

    xor ecx,ecx
    mov cx,[SA1xs]
    mov al,[SA1xpb]
    call membank0w8

    dec cx
    and cx,word[stackand]
    or cx,word[stackor]
    mov al,[SA1xpc+1]
    call membank0w8

    dec cx
    and cx,word[stackand]
    or cx,word[stackor]
    mov al,[SA1xpc]
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
    mov [SA1xs],cx

    xor ebx,ebx
    mov [SA1xpb],bl
    xor eax,eax
    mov ax,[SA1NMIV]
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

NEWSYM SA1switchtovirq
    mov al,[SA1Message]
    mov [SA1Message+2],al
    mov byte[SA1IRQExec+1],1
    mov ebx,esi
    sub ebx,[initaddrl]
    mov [SA1xpc],bx

    xor ecx,ecx
    mov cx,[SA1xs]
    mov al,[SA1xpb]
    call membank0w8

    dec cx
    and cx,word[stackand]
    or cx,word[stackor]
    mov al,[SA1xpc+1]
    call membank0w8

    dec cx
    and cx,word[stackand]
    or cx,word[stackor]
    mov al,[SA1xpc]
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
    mov [SA1xs],cx
    xor ebx,ebx
    mov [SA1xpb],bl
    xor eax,eax
    mov ax,[SA1IRQV]
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
