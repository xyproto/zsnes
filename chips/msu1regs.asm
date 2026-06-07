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
%include "macros.mac"

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