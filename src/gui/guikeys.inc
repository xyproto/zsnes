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



; GUI Keyboard Routines

SECTION .text

GUITCPIPKeys:
  ret

GUINetMenuKeys1:
  ret

SECTION .bss
GUIEditString resd 1              ; Pointer to string
NEWSYM GUIEditStringcLen, resd 1          ; Pointer to end of string (done in function)
GUIEditStringmLen resd 1          ; Max length of string
NEWSYM GUIEditStringLTxt, resd 1          ; blink value, should be set to 0 when box selected
NEWSYM GUIEditStringLstb, resd 1          ; if cursor is placed (set in main function)
NEWSYM GUIEditStringcWin, resd 1          ; 0 = no text window selected

NEWSYM GUIInputBox, resd 1
NEWSYM GUIInputLimit, resd 1

NEWSYM CSOverValue, resb 1
NEWSYM CSCurValue,  resd 1
SECTION .text

NEWSYM GUIWaitForKey
.again3
  mov edx,10
.again4
  ccallv delay, 1000
  ccallv JoyRead
  xor ebx,ebx
  mov ecx,256+128+64
.b2
  cmp byte[pressed+ebx],0
  jne near .again3
  inc ebx
  dec ecx
  jnz .b2
  dec edx
  jnz near .again4
.again
  ccallv JoyRead
  xor ebx,ebx
  mov ecx,256+128+64
.b
  cmp byte[pressed+ebx],0
  jne .pressedokay
  inc ebx
  dec ecx
  jnz .b
  jmp .again
.pressedokay
.again2
  ccall Check_Key
  or al,al
  jz .nokey
  ccall Get_Key
  jmp .again2
.nokey
  cmp byte[MouseDis],1
  je .mousedis2
  push ebx
;    mov eax,0Bh
;    int 33h
  pop ebx
.mousedis2
  mov byte[GUIpclicked],1
  ret

