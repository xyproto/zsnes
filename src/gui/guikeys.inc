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

NEWSYM InsertSearchDescription
  mov eax,CSDescDisplay
  xor dl,dl
.next
  cmp byte[eax],0
  je .fin
  inc eax
  inc dl
  jmp .next
.fin
  or dl,dl
  jz .nobackspb
  cmp dh,8
  jne .nobackspb
  mov byte[eax-1],0
  ret
.nobackspb
  cmp dh,0
  je .nocharb
  cmp dh,13
  je .nocharb
  cmp dh,8
  je .nocharb
  cmp dl,18
  je .nocharb
  mov [eax],dh
  mov byte[eax+1],0
.nocharb
  ret

; Allows you to use the arrow keys to select a state number, and Enter to pick
NEWSYM GUIStateSelKeys
  cmp al,13
  jne .noenter
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  mov byte[GUIwinactiv+2],0
  mov byte[GUIwinorder+eax],0
  dec byte[GUIwinptr]
  mov al,[GUIpmenupos]
  mov [GUIcmenupos],al
  ret
.noenter
  mov bl,al
  mov eax,[current_zst]
  mov dl,10
  div dl
  movzx ecx,ah
%ifdef __UNIXSDL__
  cmp bl,92               ; linux keypad left
  je .yesleft
  cmp dword[numlockptr],1 ; if numlock on, disregard numpad
  je .noleft
%endif
  cmp bl,75               ; linux numpad left, non-linux num/keypad left
  jne .noleft
.yesleft
  cmp cl,0
  jne .no0th
  mov cl,4
  jmp .done
.no0th
  cmp cl,5
  jne .no5th
  mov cl,9
  jmp .done
.no5th
  dec cl
  jmp .done
.noleft
%ifdef __UNIXSDL__
  cmp bl,94               ; linux keypad right
  je .yesright
  cmp dword[numlockptr],1 ; if numlock on, disregard numpad
  je .noright
%endif
  cmp bl,77               ; linux numpad right, non-linux num/keypad right
  jne .noright
.yesright
  cmp cl,4
  jne .no4th
  mov cl,0
  jmp .done
.no4th
  cmp cl,9
  jne .no9th
  mov cl,5
  jmp .done
.no9th
  inc cl
  jmp .done
.noright
%ifdef __UNIXSDL__
  cmp bl,90               ; linux keypad up
  je .yesvert
  cmp dword[numlockptr],1 ; if numlock on, disregard numpad
  je .noup
%endif
  cmp bl,72               ; linux numpad up, non-linux num/keypad up
  je .yesvert
.noup
%ifdef __UNIXSDL__
  cmp bl,96               ; linux keypad down
  je .yesvert
  cmp dword[numlockptr],1 ; if numlock on, disregard numpad
  je .novert
%endif
  cmp bl,80               ; linux numpad down, non-linux num/keypad down
  jne .novert
.yesvert
  cmp cl,5
  jge .subme
  add cl,5
  jmp .done
.subme
  sub cl,5
  jmp .done
.novert
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try1
  cmp bl,04Fh
  je .yes1
.try1
%endif
  test byte[pressed+2],1
  jz .no1
.yes1
  mov cl,1
  jmp .done
.no1
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try2
  cmp bl,050h
  je .yes2
.try2
%endif
  test byte[pressed+3],1
  jz .no2
.yes2
  mov cl,2
  jmp .done
.no2
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try3
  cmp bl,051h
  je .yes3
.try3
%endif
  test byte[pressed+4],1
  jz .no3
.yes3
  mov cl,3
  jmp .done
.no3
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try4
  cmp bl,04Bh
  je .yes4
.try4
%endif
  test byte[pressed+5],1
  jz .no4
.yes4
  mov cl,4
  jmp .done
.no4
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try5
  test byte[pressed+04Ch],1
  jnz .yes5
.try5
%endif
  test byte[pressed+6],1
  jz .no5
.yes5
  mov cl,5
  jmp .done
.no5
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try6
  cmp bl,04Dh
  je .yes6
.try6
%endif
  test byte[pressed+7],1
  jz .no6
.yes6
  mov cl,6
  jmp .done
.no6
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try7
  cmp bl,047h
  je .yes7
.try7
%endif
  test byte[pressed+8],1
  jz .no7
.yes7
  mov cl,7
  jmp .done
.no7
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try8
  cmp bl,048h
  je .yes8
.try8
%endif
  test byte[pressed+9],1
  jz .no8
.yes8
  mov cl,8
  jmp .done
.no8
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try9
  cmp bl,049h
  je .yes9
.try9
%endif
  test byte[pressed+10],1
  jz .no9
.yes9
  mov cl,9
  jmp .done
.no9
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .try0
  test byte[pressed+052h],1
  jnz .yes0
.try0
%endif
  test byte[pressed+11],1
  jz .no0
.yes0
  mov cl,0
  jmp .done
.no0
  jmp .nochange
.done
  mul dl
  add al,cl
  mov [current_zst],eax
  ret
.nochange
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .trypgup
  cmp bl,91
  je .yespgup
.trypgup
%endif
  cmp bl,73
  jne .nopgup
.yespgup
  cmp dword[current_zst],90
  jge .nochange2
  add dword[current_zst],10
  ret
.nopgup
%ifdef __UNIXSDL__
  cmp dword[numlockptr],0 ; if numlock on, let's try this first
  je .trypgdn
  cmp bl,97
  je .yespgdn
.trypgdn
%endif
  cmp bl,81
  jne .nopgdn
.yespgdn
  cmp dword[current_zst],10
  jl .nochange2
  sub dword[current_zst],10
  ret
.nopgdn
.nochange2
  ret

; Allows you to select boxes with the arrow keys, Enter to choose
NEWSYM GUIResetKeys
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
  mov byte[GUICResetPos],0
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
  mov byte[GUICResetPos],1
.noright
  cmp al,13
  jne .noconf
  cmp byte[GUICResetPos],0
  je .yesreset
  jmp .noreset
.noconf
  ToUpperASM
  cmp dh,'Y'
  jne .notyesreset
.yesreset
  mov byte[GUICBHold],2
  ccallv GUIProcReset
  ret
.notyesreset
  cmp dh,'N'
  jne .notnoreset
.noreset
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  mov byte[GUIwinactiv+12],0
  mov byte[GUIwinorder+eax],0
  dec byte[GUIwinptr]
  mov al,[GUIpmenupos]
  mov [GUIcmenupos],al
.notnoreset
  ret

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

