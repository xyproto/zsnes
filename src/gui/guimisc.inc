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



; Movie, Joystick setting (display) routines, SNES Reset Function

SECTION .bss
NEWSYM MovieRecordWinVal, resb 1
NEWSYM MovieProcessing, resb 1
NEWSYM MovieFileHand, resw 1
NEWSYM RecData, resb 16
NEWSYM NoPictureSave, resb 1

NEWSYM JoyX,       resd 1
NEWSYM JoyY,       resd 1
NEWSYM JoyX2,      resd 1
NEWSYM JoyY2,      resd 1
NEWSYM JoyMaxX,    resd 1
NEWSYM JoyMaxY,    resd 1
NEWSYM JoyMinX,    resd 1
NEWSYM JoyMinY,    resd 1
NEWSYM JoyExists,  resb 1
NEWSYM JoyExists2, resb 1

SECTION .text
NEWSYM GetCoords
  mov dword[JoyX],0
  mov dword[JoyY],0
  cli
  mov al,0
  out dx,al
  mov ecx,00FFFFh
.loopa
  in al,dx
  test al,01h
  jz .YAxis
  test al,02h
  jz .XAxis
  inc dword[JoyX]
  inc dword[JoyY]
  dec ecx
  jnz .loopa
  mov byte[JoyExists], 0
  mov dword[JoyX],0
  mov dword[JoyY],0
  jmp .End
.YAxis
  in al,dx
  test al,02h
  jz .YAxisOk
  nop
  nop
  inc dword[JoyY]
  nop
  dec ecx
  jnz .YAxis
  mov byte[JoyExists], 0
  mov dword[JoyX],0
  mov dword[JoyY],0
.YAxisOk
  jmp .End
.XAxis
  in al,dx
  test al,01h
  jz .XAxisOk
  nop
  nop
  inc dword[JoyX]
  nop
  dec ecx
  jnz .XAxis
  mov byte[JoyExists], 0
  mov dword[JoyX],0
  mov dword[JoyY],0
.XAxisOk
.End
  sti
  ret

; Dual Joysticks

NEWSYM GetCoords3
  mov dword[JoyX],0
  mov dword[JoyY],0
  mov dword[JoyX2],0
  mov dword[JoyY2],0
  cli
  mov al,0
  out dx,al
  mov ecx,01FFFFh
.loopa
  in al,dx
  test al,01h
  jz .YAxis
  inc dword[JoyX]
  nop
.YAxis
  test al,02h
  jz .XAxis
  inc dword[JoyY]
  nop
.XAxis
  test al,04h
  jz .YAxis2
  inc dword[JoyX2]
  nop
.YAxis2
  test al,08h
  jz .XAxis2
  inc dword[JoyY2]
  nop
.XAxis2
  test al,0Fh
  jz .nomore
  dec ecx
  jnz .loopa
  mov byte[JoyExists], 0
  mov dword[JoyX],0
  mov dword[JoyY],0
  mov byte[JoyExists2], 0
  mov dword[JoyX2],0
  mov dword[JoyY2],0
.nomore
  sti
  ret
