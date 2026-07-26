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
EXTSYM SA1switchtonmi,SA1switchtovirq,SA1SwapEnter,SA1SwapLeave

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

; SA1Swap gives the SA-1 one instruction slot. The decision logic and the
; 65816 <-> SA-1 context switch live in chips/c_sa1proc.c; all that has to stay
; here is the opcode dispatch itself, which runs with the core's register ABI
; live (notably ebp, the SPC program counter). pushad hands the whole register
; file to the C halves, which read and write it in place.
NEWSYM SA1Swap
    pushad
    mov eax, esp
    ccall SA1SwapEnter, eax
    test eax, eax
    jz .done
    popad
    mov bl,[esi]
    inc esi
    call dword near [edi+ebx*4]
    dec esi
    pushad
    mov eax, esp
    ccall SA1SwapLeave, eax
.done
    popad
    ret

SECTION .bss
NEWSYM SA1xpc, resd 1
SECTION .text

; SA1switchtonmi and SA1switchtovirq (and the makedl macro) have been ported
; to C (chips/c_sa1proc.c).
