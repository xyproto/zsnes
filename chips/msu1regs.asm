;Copyright (C) 2023 Sneed, ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro
EXTSYM MSU_StateControl,MSU_AudioVolume,MSU_Track,MSU_Data_SeekPort,MSU_Data_Addr
EXTSYM MSU_DATA,MSU_StatusRead,MSU1HandleTrackChange,MSU1HandleControlBits

SECTION .text

;STATUS read
;Data busy 	Audio busy 	Audio repeat 	Audio playing 	Track missing 	Revision
;No need for busy bits since data will be stored in RAM
NEWSYM msustatusread
	mov al,[MSU_StatusRead]
ret

;DATA read
NEWSYM msudataread
	mov ebx,[MSU_DATA]
	add ebx,[MSU_Data_Addr]

; Reads have no effect when data busy bit set
	mov	al,[MSU_StatusRead]
	and al,80h
	jnz .dontincrement

; Increment data address
	inc dword[MSU_Data_Addr]

.dontincrement
	mov al,[ebx]
ret

;MSU-1 ID
NEWSYM msuid1
	mov al,'S'
ret
NEWSYM msuid2
	mov al,'-'
ret
NEWSYM msuid3
	mov al,'M'
ret
NEWSYM msuid4
	mov al,'S'
ret
NEWSYM msuid5
	mov al,'U'
ret
NEWSYM msuid6
	mov al,'1'
ret

;--------------------------- Write Registers ---------------------------

;DATA Seek Pointer
NEWSYM msudataseek0
	mov [MSU_Data_SeekPort],al
ret
NEWSYM msudataseek1
	mov [MSU_Data_SeekPort+1],al
ret
NEWSYM msudataseek2
	mov [MSU_Data_SeekPort+2],al
ret
NEWSYM msudataseek3
	; Writing to $2003 triggers seek
	mov [MSU_Data_SeekPort+3],al

	; writes have no effect if data busy bit set
	mov al,[MSU_StatusRead]
	and al,80h
	jnz .dontseek

	; Start seek, set data busy bit
	or byte[MSU_StatusRead],80h
	; Set data address to seek port
	mov eax,[MSU_Data_SeekPort]
	mov [MSU_Data_Addr],eax
	; Seek finished, clear data busy bit
	and byte[MSU_StatusRead],~80h

.dontseek
ret

;TRACK
NEWSYM msu1track0
	mov [MSU_Track],al
ret
NEWSYM msu1track1
	mov [MSU_Track+1],al
	ccall MSU1HandleTrackChange
ret

;VOLUME
NEWSYM msu1volume
	mov [MSU_AudioVolume],al
ret

;STATE CONTROL
NEWSYM msu1statecontrol
	mov [MSU_StateControl],al
	ccall MSU1HandleControlBits
ret