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

EXTSYM MSU_CurrentStatus,MSU_MusicVolume,MSU_Track,MSU_Data_Seek,MSU_DATA,MSU_StatusRead,MSU1HandleTrackChange,MSU1HandleStatusBits,MSU1GetStatusBitsSpecial

SECTION .text

;STATUS read
;Data busy 	Audio busy 	Audio repeat 	Audio playing 	Track missing 	Revision
;No need for busy bits since data will be stored in RAM
NEWSYM msustatusread
	ccall MSU1GetStatusBitsSpecial
	mov al,[MSU_StatusRead]
ret

;DATA read
NEWSYM msudataread
	mov ebx,[MSU_DATA]
	add ebx,[MSU_Data_Seek]
	mov al,[ebx]

	;increment seek
	inc dword[MSU_Data_Seek]
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
	mov [MSU_Data_Seek],al
ret
NEWSYM msudataseek1
	mov [MSU_Data_Seek+1],al
ret
NEWSYM msudataseek2
	mov [MSU_Data_Seek+2],al
ret
NEWSYM msudataseek3
	mov [MSU_Data_Seek+3],al
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
	mov [MSU_MusicVolume],al
ret

;STATE CONTROL
NEWSYM msu1statecontrol
	mov [MSU_CurrentStatus],al
	ccall MSU1HandleStatusBits
ret