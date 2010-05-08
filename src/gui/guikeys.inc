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

%macro ToUpperASM 0
  cmp dh,'a'
  jb %%notlcase
  cmp dh,'z'
  ja %%notlcase
  sub dh,'a'-'A'
%%notlcase
%endmacro

SECTION .text

GUITCPIPKeys:
  ret

GUINetMenuKeys1:
  ret

NEWSYM GUIGetInputLine
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

; Allows you to select boxes with the arrow keys, Enter to choose
NEWSYM GUIStateKeys
%ifdef __UNIXSDL__
  cmp al,92
  je .yesleft
  cmp dword[numlockptr],1
  je .noleft
  cmp al,75
  jne .noleft
.yesleft
%else
  cmp al,75
  jne .noleft
%endif
  mov byte[GUICStatePos],0
.noleft
%ifdef __UNIXSDL__
  cmp al,94
  je .yesright
  cmp dword[numlockptr],1
  je .noright
  cmp al,77
  jne .noright
.yesright
%else
  cmp al,77
  jne .noright
%endif
  mov byte[GUICStatePos],1
.noright
  cmp al,13
  jne .noconf
  cmp byte[GUICStatePos],0
  je .yesreset
  jmp .noreset
.noconf
  ToUpperASM
  cmp dh,'Y'
  jne .notyesreset
.yesreset
  mov byte[GUICBHold],10
  ccallv GUIProcStates
  ret
.notyesreset
  cmp dh,'N'
  jne .notnoreset
.noreset
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  mov byte[GUIwinactiv+14],0
  mov byte[GUIwinorder+eax],0
  dec byte[GUIwinptr]
  mov al,[GUIpmenupos]
  mov [GUIcmenupos],al
.notnoreset
  ret

NEWSYM GUILoadKeys
  cmp dx,0
  jne near .keypressed
.nokeypressed
  ret
.keypressed
  mov [gui_key],dh
  mov [gui_key_extended],dl
  push eax
  ccall GUILoadKeysNavigate
  cmp eax,1
  pop eax
  je .nokeypressed
  cmp byte[GUILoadPos],0
  je near .nobacksp
  cmp dh,8
  jne .nobacksp
  mov byte[GUILDFlash],0
  dec byte[GUILoadPos]
  xor eax,eax
  mov al,[GUILoadPos]
  ret
.nobacksp
  cmp dh,8
  je near .nokeypressed
  cmp byte[GUILoadPos],36
  je near .nokeypressed
  mov byte[GUILDFlash],0
  xor eax,eax
  mov al,[GUILoadPos]
  mov [GUILoadTextA+eax],dh
  inc byte[GUILoadPos]
  ccallv GUILoadKeysJumpTo
  ret

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

