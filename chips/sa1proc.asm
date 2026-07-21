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
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif
EXTSYM initaddrl,wramdata,IRAM,SA1DoIRQ,SNSRegP,SNSRegPCS
EXTSYM SA1Ptr,SNSPtr,snesmap2,SA1tablead,SA1xpb,SA1RegP,wramdataa,SA1TimerVal
EXTSYM SA1RegPCS,SA1BWPtr,SNSBWPtr,CurBWPtr,SA1NMIV,SA1IRQV
EXTSYM membank0w8,SA1LBound,SA1UBound,SA1SH,SA1SHb,stackor,stackand,snesmmap
EXTSYM SA1xs,SA1IRQExec,SA1Message,Sflagnz,Sflagc,Sflago
EXTSYM SA1switchtonmi,SA1switchtovirq

%macro ccall 1-*
	push ecx
	push edx
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%if %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1-*
	push eax
	ccall %1
	pop eax
%endmacro

; In exec loop, jump to execloop if SA1Status != 0
; *** Disable spc700 if possible ***

SECTION .bss
NEWSYM SA1Status, resb 1

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
    push edx
    mov edx, esp
    push esi
    mov esi, esp
    ccallv SA1switchtovirq, edx, esi
    pop esi
    pop edx
    jmp .returnirq
.nmi
    and byte[SA1DoIRQ],0FDh
    push edx
    mov edx, esp
    push esi
    mov esi, esp
    ccallv SA1switchtonmi, edx, esi
    pop esi
    pop edx
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

; SA1switchtonmi and SA1switchtovirq (and the makedl macro) have been ported
; to C (chips/c_sa1proc.c).
