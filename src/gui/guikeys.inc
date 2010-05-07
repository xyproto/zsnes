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



EXTSYM AddCSCheatCode

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
SECTION .text

NEWSYM GUICheatKeys
  cmp dword[GUIcurrentcheatwin],1
  je near .textboxa
  cmp dword[GUIcurrentcheatwin],2
  je near .textboxb
  ToUpperASM
  cmp dh,'R'               ; Hotkeys
  jne .noremocheat
  jmp CheatCodeRemove
.noremocheat
  cmp dh,'T'
  jne .notoggcheat
  jmp CheatCodeToggle
.notoggcheat
  cmp dh,'S'
  jne .nosavecheat
  ccallv CheatCodeSave
  ret
.nosavecheat
  cmp dh,'L'
  jne .noloadcheat
  ccallv CheatCodeLoad
  ret
.noloadcheat
  cmp dh,'F'
  jne .nofixcheat
  jmp CheatCodeFix
.nofixcheat
  cmp dh,'A'
  jne .noauto
  xor byte[AutoLoadCht],1
.noauto

;Main Cheat Box
  cmp dword[NumCheats],0
  jne .nozero
  ret
.nozero
%ifdef __UNIXSDL__
  cmp al,89
  je .homepressed
  cmp dword[numlockptr],1
  je .nohome
  cmp al,71
  jne .nohome
.homepressed
%else
  cmp al,71
  jne .nohome
%endif
  mov dword[GUIcurrentcheatcursloc],0
  mov dword[GUIcurrentcheatviewloc],0
  ret
.nohome
%ifdef __UNIXSDL__
  cmp al,89
  je .endppressed
  cmp dword[numlockptr],1
  je .noendp
  cmp al,79
  jne .noendp
.endppressed
%else
  cmp al,79
  jne .noendp
%endif
  mov eax,[NumCheats]
  dec eax
  mov [GUIcurrentcheatcursloc],eax
  sub eax,11
  mov [GUIcurrentcheatviewloc],eax
  test dword[GUIcurrentcheatviewloc],8000000h
  jz .posve
  mov dword[GUIcurrentcheatviewloc],0
.posve
  ret
.noendp
%ifdef __UNIXSDL__
  cmp al,90
  je .up
  cmp dword[numlockptr],1
  je .noup
  cmp al,72
  jne .noup
%else
  cmp al,72
  jne .noup
%endif
.up
  cmp dword[GUIcurrentcheatcursloc],0
  je .noup
  mov ebx,[GUIcurrentcheatcursloc]
  cmp [GUIcurrentcheatviewloc],ebx
  jne .noviewdec
  dec dword[GUIcurrentcheatviewloc]
.noviewdec
  dec dword[GUIcurrentcheatcursloc]
.noup
%ifdef __UNIXSDL__
  cmp al, 96
  je .downpressed
  cmp dword[numlockptr],1
  je .nodown
  cmp al,80
  jne .nodown
.downpressed
%else
  cmp al,80
  jne .nodown
%endif
  mov ebx,[GUIcurrentcheatcursloc]
  inc ebx
  cmp ebx,[NumCheats]
  jae .noviewinc
  inc dword[GUIcurrentcheatcursloc]
  sub ebx,12
  cmp ebx,[GUIcurrentcheatviewloc]
  jne .noviewinc
  inc dword[GUIcurrentcheatviewloc]
.noviewinc
.nodown
%ifdef __UNIXSDL__
  cmp al,91
  je .pageuppressed
  cmp dword[numlockptr],1
  je .nopageup
  cmp al,73
  jne .nopageup
.pageuppressed
%else
  cmp al,73
  jne .nopageup
%endif
  sub dword[GUIcurrentcheatviewloc],12
  sub dword[GUIcurrentcheatcursloc],12
  test dword[GUIcurrentcheatviewloc],8000000h
  jz .posv
  mov dword[GUIcurrentcheatviewloc],0
.posv
  test dword[GUIcurrentcheatcursloc],8000000h
  jz .posc
  mov dword[GUIcurrentcheatcursloc],0
.posc
.nopageup
%ifdef __UNIXSDL__
  cmp al, 97
  je .pagednpressed
  cmp dword[numlockptr],1
  je .nopagedown
  cmp al,81
  jne .nopagedown
.pagednpressed
%else
  cmp al,81
  jne .nopagedown
%endif
  add dword[GUIcurrentcheatviewloc],12
  add dword[GUIcurrentcheatcursloc],12
  mov ebx,[NumCheats]
  dec ebx
  cmp dword[GUIcurrentcheatcursloc],ebx
  jb .poscd
  mov [GUIcurrentcheatcursloc],ebx
.poscd
  sub ebx,11
  cmp dword[GUIcurrentcheatviewloc],ebx
  jl .posvd
  test ebx,8000000h
  jz .noneg
  mov ebx,0
.noneg
  mov [GUIcurrentcheatviewloc],ebx
.posvd
.nopagedown
  ret

;Enter Code Input Box
.textboxa
  cmp byte[GUICheatPosA],0
  je near .nobacksp
  cmp dh,9
  je .yesret
  cmp dh,13
  jne near .noret
.yesret
  cmp byte[GUICheatPosA],2
  jne near .nocodemod
  mov byte[GUICheatPosA],0
  mov esi,[GUIcurrentcheatcursloc]
  shl esi,5
  sub esi,[GUIcurrentcheatcursloc]
  sub esi,[GUIcurrentcheatcursloc]
  sub esi,[GUIcurrentcheatcursloc]
  sub esi,[GUIcurrentcheatcursloc]
  add esi,cheatdata
  mov al,[GUICheatTextZ1]
  cmp al,'a'
  jae .lowletter
  cmp al,'A'
  jae .letter
  sub al,'0'
  jmp .num
.lowletter
  sub al,'a'-10
  jmp .num
.letter
  sub al,'A'-10
.num
  mov ah,[GUICheatTextZ1+1]
  cmp ah,'a'
  jae .lowletter2
  cmp ah,'A'
  jae .letter2
  sub ah,'0'
  jmp .num2
.lowletter2
  sub ah,'a'-10
  jmp .num2
.letter2
  sub ah,'A'-10
.num2
  shl al,4
  and ah,0Fh
  or al,ah
  mov [esi+1],al
  mov word[GUICheatTextZ1],0
  pushad
  call EnableCheatCodeNoPrevMod
  popad
  jmp .noret
.nocodemod
  inc dword[GUIcurrentcheatwin]
  mov byte[GUICCFlash],0
.noret
  cmp dh,8
  jne .nobacksp
  mov byte[GUICCFlash],0
  dec byte[GUICheatPosA]
  xor eax,eax
  mov al,[GUICheatPosA]
  mov byte[GUICheatTextZ1+eax],'_'
  mov byte[GUICheatTextZ1+eax+1],0
  ret
.nobacksp
  cmp dh,0
  je .nochar
  cmp dh,13
  je .nochar
  cmp dh,8
  je .nochar
  cmp dh,9
  je .nochar
  cmp byte[GUICheatPosA],14
  je .nochar
  mov byte[GUICCFlash],0
  xor eax,eax
  mov al,[GUICheatPosA]
  mov [GUICheatTextZ1+eax],dh
  mov byte[GUICheatTextZ1+eax+1],'_'
  mov byte[GUICheatTextZ1+eax+2],0
  inc byte[GUICheatPosA]
.nochar
  ret

;Description Input Box
.textboxb
  cmp dh,13
  jne .noretb
  mov byte[GUICCFlash],0
  jmp ProcessCheatCode
.noretb
  cmp byte[GUICheatPosB],0
  je .nobackspb
  cmp dh,8
  jne .nobackspb
  mov byte[GUICCFlash],0
  dec byte[GUICheatPosB]
  xor eax,eax
  mov al,[GUICheatPosB]
  mov byte[GUICheatTextZ2+eax],'_'
  mov byte[GUICheatTextZ2+eax+1],0
  ret
.nobackspb
  cmp dh,0
  je .nocharb
  cmp dh,13
  je .nocharb
  cmp dh,8
  je .nocharb
  cmp byte[GUICheatPosB],18
  je .nocharb
  xor eax,eax
  mov byte[GUICCFlash],0
  mov al,[GUICheatPosB]
  mov [GUICheatTextZ2+eax],dh
  mov byte[GUICheatTextZ2+eax+1],'_'
  mov byte[GUICheatTextZ2+eax+2],0
  inc byte[GUICheatPosB]
.nocharb
  ret

%macro CompareKeyMacro 3
  cmp dh,%1
  jne %%skip
  mov byte[%2],%3
%%skip
%endmacro

InsertSearchCharacter:
  push edx
  push eax
  ; Send character into CSInputDisplay
  ; Find location
  mov ecx,CSInputDisplay
.next
  cmp byte[ecx],0
  je .found
  cmp byte[ecx],'_'
  je .found
  inc ecx
  jmp .next
.found
  ; Delete if necessary
  cmp dh,8
  jne .notdelete
  cmp ecx,CSInputDisplay
  je near .done
  dec ecx
  jmp .placecursor
.notdelete
  cmp ecx,CSInputDisplay+10
  je near .done
  ; Add character if necessary
  cmp dh,'0'
  jb near .done
  cmp dh,'9'
  jbe .okay
  cmp byte[CheatSrcByteBase],0
  je near .done
  cmp dh,'F'
  ja near .done
  cmp dh,'A'
  jb near .done
.okay
  mov [ecx],dh
  inc ecx
  ; Process cursor if over the window
.placecursor
  mov byte[ecx],'_'
  mov byte[ecx+1],0

  mov byte[CSOverValue],0
  ; Find overall value and delete if over
  mov ecx,CSInputDisplay
  mov ebx,10
  cmp byte[CheatSrcByteBase],0
  je .dec
  mov ebx,16
.dec
  xor eax,eax
.next2
  cmp byte[ecx],0
  je .found2
  cmp byte[ecx],'_'
  je .found2
  mul ebx
  or edx,edx
  jz .notoverflow
  mov byte[CSOverValue],1
.notoverflow
  xor edx,edx
  mov dl,[ecx]
  cmp dl,'A'
  jb .below
  sub dl,'A'-10-'0'
.below
  sub dl,'0'
  add eax,edx
  jnc .notoverflowb
  mov byte[CSOverValue],1
.notoverflowb
  inc ecx
  jmp .next2
.found2
  xor ebx,ebx
  mov bl,[CheatSrcByteSize]
  mov ebx,[SrcMask+ebx*4]
  mov [CSCurValue],eax
  cmp eax,ebx
  jbe .done
  mov byte[CSOverValue],1
.done
  pop eax
  pop edx
  ret

SECTION .bss
NEWSYM CSOverValue, resb 1
NEWSYM CSCurValue,  resd 1
SECTION .text

InsertSearchDescription:
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

NEWSYM GUICheatSearchKeys
  ToUpperASM
  cmp byte[CheatWinMode],3
  je near .add
  cmp byte[CheatWinMode],2
  je near .view
  cmp byte[CheatWinMode],1
  je near .search

  CompareKeyMacro '1',CheatSrcByteSize,0    ; Main Menu
  CompareKeyMacro '2',CheatSrcByteSize,1
  CompareKeyMacro '3',CheatSrcByteSize,2
  CompareKeyMacro '4',CheatSrcByteSize,3
  CompareKeyMacro 'D',CheatSrcByteBase,0
  CompareKeyMacro 'H',CheatSrcByteBase,1
  CompareKeyMacro 'E',CheatSrcSearchType,0
  CompareKeyMacro 'C',CheatSrcSearchType,1

  cmp dh,13                ; Start Button
  je .start
  cmp dh,'S'
  jne .notstart
.start
  call CheatCodeSearchInit
  ret
.notstart
  ret

.search                           ; Shortcuts for Select Comparison
  cmp byte[CheatSrcSearchType],1
  je .compare
  call InsertSearchCharacter
  jmp .nomovedown
.compare
  cmp dh,','
  je .lessthan
  cmp dh,'N'
  jne .notlessthan
.lessthan
  mov byte[CheatCompareValue],0
.notlessthan
  cmp dh,'.'
  je .greaterthan
  cmp dh,'E'
  jne .notgreaterthan
.greaterthan
  mov byte[CheatCompareValue],1
.notgreaterthan
  cmp dh,'W'
  je .equalthan
  cmp dh,'+'
  jne .notequalthan
.equalthan
  mov byte[CheatCompareValue],2
.notequalthan
  cmp dh,'A'
  jne .notnotequalthan
  mov byte[CheatCompareValue],3
.notnotequalthan

%ifdef __UNIXSDL__          ; Select with Arrow Keys
  cmp al,90
  je .yesmoveup
  cmp dword[numlockptr],1
  je .nomoveup
  cmp al,72
  jne .nomoveup
.yesmoveup
%else
  cmp al,72
  jne .nomoveup
%endif
  cmp byte[CheatCompareValue],0
  je .nomoveup
  dec byte[CheatCompareValue]
.nomoveup
%ifdef __UNIXSDL__
  cmp al,96
  je .yesmovedown
  cmp dword[numlockptr],1
  je .nomovedown
  cmp al,80
  jne .nomovedown
.yesmovedown
%else
  cmp al,80
  jne .nomovedown
%endif
  cmp byte[CheatCompareValue],3
  je .nomovedown
  inc byte[CheatCompareValue]

.nomovedown                ; Restart/View/Search Buttons
  cmp dh,'R'
  jne .notrestart
  mov byte[CheatWinMode],0
  mov byte[CheatSearchStatus],0
.notrestart
  cmp dh,'V'
  jne .notview
  mov byte[CheatWinMode],2
.notview
  cmp dh,13
  je .searchprocess
  cmp dh,'S'
  jne .nosearchprocess
.searchprocess
  cmp byte[CheatSearchStatus],1
  je .nosearchprocess
  call CheatCodeSearchProcess
.nosearchprocess
  ret

.view                            ; View
  cmp dword[NumCheatSrc],0     ; Return Hotkey
  jne .nozero
  cmp dh,27
  je .yesret
  cmp dh,'R'
  jne .notreturnb
.yesret
  mov byte[CheatWinMode],1
.notreturnb
  ret

.nozero                   ; Select Codes with arrow keys
%ifdef __UNIXSDL__
  cmp al,89
  je .yeshome
  cmp dword[numlockptr],1
  je .nohome
  cmp al,71
  jne .nohome
.yeshome
%else
  cmp al,71
  jne .nohome
%endif
  mov dword[GUIcurrentchtsrccursloc],0
  mov dword[GUIcurrentchtsrcviewloc],0
  ret
.nohome
%ifdef __UNIXSDL__
  cmp al,95
  je .yesendp
  cmp dword[numlockptr],1
  je .noendp
  cmp al,79
  jne .noendp
.yesendp
%else
  cmp al,79
  jne .noendp
%endif
  mov eax,[NumCheatSrc]
  dec eax
  mov [GUIcurrentchtsrccursloc],eax
  sub eax,11
  mov [GUIcurrentchtsrcviewloc],eax
  test dword[GUIcurrentchtsrcviewloc],8000000h
  jz .posve
  mov dword[GUIcurrentchtsrcviewloc],0
.posve
  ret
.noendp
%ifdef __UNIXSDL__
  cmp al,90
  je .up
  cmp dword[numlockptr],1
  je .noup
  cmp al,72
  jne .noup
%else
  cmp al,72
  jne .noup
%endif
.up
  cmp dword[GUIcurrentchtsrccursloc],0
  je .noup
  mov ebx,[GUIcurrentchtsrccursloc]
  cmp [GUIcurrentchtsrcviewloc],ebx
  jne .noviewdec
  dec dword[GUIcurrentchtsrcviewloc]
.noviewdec
  dec dword[GUIcurrentchtsrccursloc]
.noup
%ifdef __UNIXSDL__
  cmp al,96
  je .yesdown
  cmp dword[numlockptr],1
  je .nodown
  cmp al,80
  jne .nodown
.yesdown
%else
  cmp al,80
  jne .nodown
%endif
  mov ebx,[GUIcurrentchtsrccursloc]
  inc ebx
  cmp ebx,[NumCheatSrc]
  jae .noviewinc
  inc dword[GUIcurrentchtsrccursloc]
  sub ebx,12
  cmp ebx,[GUIcurrentchtsrcviewloc]
  jne .noviewinc
  inc dword[GUIcurrentchtsrcviewloc]
.noviewinc
.nodown

%ifdef __UNIXSDL__            ; Pageup/Down to select Cheats
  cmp al,91
  je .yespgup
  cmp dword[numlockptr],1
  je .nopageup
  cmp al,73
  jne .nopageup
.yespgup
%else
  cmp al,73
  jne .nopageup
%endif
  sub dword[GUIcurrentchtsrcviewloc],12
  sub dword[GUIcurrentchtsrccursloc],12
  test dword[GUIcurrentchtsrcviewloc],8000000h
  jz .posv
  mov dword[GUIcurrentchtsrcviewloc],0
.posv
  test dword[GUIcurrentchtsrccursloc],8000000h
  jz .posc
  mov dword[GUIcurrentchtsrccursloc],0
.posc
.nopageup
%ifdef __UNIXSDL__
  cmp al,97
  je .yespgdn
  cmp dword[numlockptr],1
  je .nopagedown
  cmp al,81
  jne .nopagedown
.yespgdn
%else
  cmp al,81
  jne .nopagedown
%endif
  add dword[GUIcurrentchtsrcviewloc],12
  add dword[GUIcurrentchtsrccursloc],12
  mov ebx,[NumCheatSrc]
  dec ebx
  cmp dword[GUIcurrentchtsrccursloc],ebx
  jb .poscd
  mov [GUIcurrentchtsrccursloc],ebx
.poscd
  sub ebx,11
  cmp dword[GUIcurrentchtsrcviewloc],ebx
  jl .posvd
  test ebx,8000000h
  jz .noneg
  mov ebx,0
.noneg
  mov [GUIcurrentchtsrcviewloc],ebx
.posvd
.nopagedown

  cmp dh,27          ; Return/Add Buttons
  je .yesretb
  cmp dh,'R'
  jne .notreturn
.yesretb
  mov byte[CheatWinMode],1
.notreturn
  cmp dh,13
  je .yesadd
  cmp dh,'A'
  jne .notadd
.yesadd
  mov byte[CheatWinMode],3
  mov byte[CurCStextpos],0
  mov byte[CSInputDisplay],'_'
  mov byte[CSInputDisplay+1],0
  mov byte[CSDescDisplay],0
.notadd
  ret

.add                                 ; Add Cheat Window
  cmp dh,27                        ; Button Hotkey
  je .yesretc
  cmp byte[CurCStextpos],1
  je .notreturnc
  cmp dh,'R'
  jne .notreturnc
.nofastforward
.yesretc
  mov byte[CheatWinMode],2
.notreturnc
  cmp byte[CurCStextpos],0
  jne near .nofirstbox
  cmp dh,9
  je .yestab
  cmp byte[CSOverValue],1
  je .noret
  cmp dh,13
  jne .noret
.yestab
  mov byte[CurCStextpos],1
.noret
  call InsertSearchCharacter
  ret
.nofirstbox
  cmp byte[CurCStextpos],1
  jne near .nosecondbox
  cmp dh,9
  jne .notab
  mov byte[CurCStextpos],0
.notab
  cmp dh,13
  jne .noenter
  ccallv AddCSCheatCode
  ret
.noenter
  call InsertSearchDescription
  ret
.nosecondbox
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

