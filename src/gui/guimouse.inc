;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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



%ifdef __WIN32__
EXTSYM MinimizeWindow
%endif

SECTION .bss
lastmouseholded resb 1
mousebuttonstat resw 1
MouseMoveOkay   resb 1
LastHoldEnable  resb 1
SECTION .text

ProcessMouse:
  ; Process holds
  mov al,[GUIHold]
  cmp al,[LastHoldEnable]
  je near .noholdnecessary
  cmp byte[GUIHold],0
  jne .noholda
  cmp byte[LastHoldEnable],7 ; ntsc sliders
  jne .norelease1
  xor edx,edx
  mov dl,[ntscWhVar]
  mov cl,[ntscCurVar]
  cmp cl,[ntscLastVar+edx]
  je .norelease1
  pushad
  call NTSCFilterInit
  popad
  mov [ntscLastVar+edx],cl
.norelease1
  mov ecx,0
  mov edx,255
  call Set_MouseXMax
  mov ecx,0
  mov edx,223
  call Set_MouseYMax
.noholda
  cmp byte[GUIHold],1 ; GUI Windows
  jne .nohold1
  mov ecx,0
  mov edx,255
  call Set_MouseXMax
  mov ecx,16
  mov edx,223
  call Set_MouseYMax
.nohold1
  cmp byte[GUIHold],5 ; Sound Slider
  je .holdslider
  cmp byte[GUIHold],6 ; Speed Slider
  je .holdslider
  cmp byte[GUIHold],2 ; Colour Slider
  je .holdslider
  cmp byte[GUIHold],7 ; Video Slider
  jne .nohold2m
.holdslider
  mov ecx,[GUIHoldXlimL] ; Sets min/max move range for mouse
  mov edx,[GUIHoldXlimR] ; once holding slider
  call Set_MouseXMax
  mov ecx,[GUIHoldYlim]  ; Locks pointer on slider
  mov edx,[GUIHoldYlim]
  call Set_MouseYMax
.nohold2m
  cmp byte[GUIHold],3    ; Scrollbars
  jne .nohold3m
  mov ecx,[GUIHoldXlimL]
  mov edx,[GUIHoldXlimR]
  call Set_MouseXMax
  mov ecx,[GUIHoldYlim]
  mov edx,[GUIHoldYlimR]
  call Set_MouseYMax
.nohold3m
  mov al,[GUIHold]
  mov [LastHoldEnable],al
.noholdnecessary
  mov byte[MouseMoveOkay],0
  call Get_MouseData
  cmp byte[lhguimouse],1
  jne .notlefthanded
  call SwapMouseButtons
.notlefthanded
  mov [mousebuttonstat],bx
  cmp byte[lastmouseholded],0
  je .noprevhold
  test bx,01h
  jnz .noprevhold
  mov byte[lastmouseholded],0
  mov cx,[GUImouseposx]
  mov dx,[GUImouseposy]
  call Set_MousePosition
.noprevhold
  cmp byte[mousewrap],1
  je near ProcessMouseWrap
  call Get_MouseData
  cmp byte[lhguimouse],1
  jne .notlefthanded2
  call SwapMouseButtons
.notlefthanded2
  cmp word[GUImouseposx],cx
  je .zero2
  mov byte[MouseMoveOkay],1
.zero2
  cmp word[GUImouseposy],dx
  je .zero3
  mov byte[MouseMoveOkay],1
.zero3
  mov [GUImouseposx],cx
  test word[GUImouseposx],8000h
  jz .nowrapleft
  mov word[GUImouseposx],0
.nowrapleft
  cmp word[GUImouseposx],255
  jbe .nowrapright
  mov word[GUImouseposx],255
.nowrapright
  mov [GUImouseposy],dx
  test word[GUImouseposy],8000h
  jz .nowrapup
  mov word[GUImouseposy],0
.nowrapup
  cmp word[GUImouseposy],223
  jbe .nowrapdown
  mov word[GUImouseposy],100
.nowrapdown
  jmp ProcessMouseButtons

ProcessMouseWrap:
  call Get_MousePositionDisplacement
  add word[GUImouseposx],cx
.again
  test word[GUImouseposx],8000h
  jz .nowrapleft
  add word[GUImouseposx],256
  jmp .again
.nowrapleft
  cmp word[GUImouseposx],255
  jbe .nowrapright
  sub word[GUImouseposx],256
  jmp .again
.nowrapright
  add word[GUImouseposy],dx
.again2
  test word[GUImouseposy],8000h
  jz .nowrapup
  add word[GUImouseposy],224
  jmp .again2
.nowrapup
  cmp word[GUImouseposy],223
  jbe .nowrapdown
  cmp byte[GUIHold],1
  jne .nohold
  sub word[GUImouseposy],224-16
  jmp .again2
.nohold
  sub word[GUImouseposy],224
  jmp .again2
.nowrapdown
  cmp dx,0
  jne .nozero
  cmp cx,0
  je .zero
.nozero
  mov byte[MouseMoveOkay],1
.zero
ProcessMouseButtons:
  cmp byte[MouseMoveOkay],1
  jne .zero
  mov byte[GUIOnMenuItm],0
  mov ax,[GUImouseposx]
  cmp ax,[GUIMenuL]
  jbe .zero
  cmp ax,[GUIMenuR]
  jae .zero
  mov ax,[GUImouseposy]
  cmp ax,18
  jbe .zero
  cmp ax,[GUIMenuD]
  jae .zero
  ; Mouse Menu
  xor dx,dx
  sub ax,18
  mov bx,10
  div bx
  and eax,0FFh
  mov ebx,[GUICYLocPtr]
  mov byte[GUIOnMenuItm],2
  cmp byte[eax+ebx+1],0
  je .zero
  mov [GUIcrowpos],al
  mov byte[GUIOnMenuItm],1
.zero
  ; Check if mouse is clicked on menu
  mov bx,[mousebuttonstat]
  cmp byte[GUIClick],1
  jne .norclick2
  test bx,02h
  jz .norclick
  cmp byte[romloadskip],0
  jne .norclick2
  cmp byte[MousePRClick],0
  jne .norclick2
  mov byte[GUIQuit],2
.norclick
  mov byte[MousePRClick],0
.norclick2
  test bx,01h
  jz near .notclicked
  cmp byte[GUIpclicked],1
  je near .noclick
  cmp byte[GUIOnMenuItm],2
  je near .noclick
  cmp byte[GUIOnMenuItm],1
  jne .nomenuitem
  call GUITryMenuItem
  jmp .noclick
.nomenuitem

  cmp word[GUImouseposy],3
  jb near .noclickedw
  cmp word[GUImouseposy],14
  ja near .noclickedw
  cmp word[GUImouseposx],230
  jb near .noclickedw
  cmp word[GUImouseposx],233
  jb near .noclickedw
  cmp word[GUImouseposx],242
  ja .notfound7
%ifdef __UNIXSDL__
  jmp .clickawn
%endif
  cmp word[GUImouseposy],8
  jb .clickawn
  cmp word[GUImouseposy],8
  ja .clickawn2
.clickawn
  mov byte[GUIcwinpress],1
  jmp .noclick
.clickawn2
  mov byte[GUIcwinpress],3
  jmp .noclick
.notfound7
  cmp word[GUImouseposx],244
  jb near .noclickedw
  cmp word[GUImouseposx],253
  ja .notfound8
  mov byte[GUIcwinpress],2
  jmp .noclick
.notfound8
.noclickedw

  cmp word[GUImouseposy],15
  ja near .nomenu
  cmp word[GUImouseposy],3
  jb near .noclicked
  cmp word[GUImouseposy],13
  ja near .noclicked
;    GUIDMHelp 4,12,GUIMenuItem+6,1
  cmp word[GUImouseposx],4
  jb near .noclicked
  cmp word[GUImouseposx],12
  ja .notfound
  mov byte[GUIcmenupos],1
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound
;    GUIDMHelp 17,47,GUIMenuItem,2
  cmp word[GUImouseposx],17
  jb near .noclicked
  cmp word[GUImouseposx],47
  ja .notfound2
  mov byte[GUIcmenupos],2
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound2
;    GUIDMHelp 52,94,GUIMenuItem+7,3
  cmp word[GUImouseposx],52
  jb near .noclicked
  cmp word[GUImouseposx],94
  ja .notfound3
  mov byte[GUIcmenupos],3
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound3
;    GUIDMHelp 99,135,GUIMenuItem+14,4
  cmp word[GUImouseposx],99
  jb near .noclicked
  cmp word[GUImouseposx],135
  ja .notfound4
  mov byte[GUIcmenupos],4
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound4
;    GUIDMHelp 140,188,GUIMenuItem+21,5
  cmp word[GUImouseposx],140
  jb near .noclicked
  cmp word[GUImouseposx],188
  ja .notfound5
  mov byte[GUIcmenupos],5
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound5
;    GUIDMHelp 193,223,GUIMenuItem+29,6
  cmp word[GUImouseposx],193
  jb near .noclicked
  cmp word[GUImouseposx],223
  ja .notfound6
  mov byte[GUIcmenupos],6
  mov byte[GUIcrowpos],0
  jmp .noclick
.notfound6
  jmp .noclick
.noclicked

%ifdef __MSDOS__
  jmp .noclick
%else
  mov byte[GUIpclicked],1
  mov byte[GUIHold],255
  mov eax,[GUImouseposy]
  mov [GUIHoldYlim],eax
  mov eax,[GUImouseposx]
  mov [GUIHoldXlimL],eax
  call MouseWindow
  jmp .noclick
%endif
.nomenu
  cmp byte[GUIcmenupos],0
  je near .checkmenuboxclick
  mov al,[GUIcmenupos]
  mov [GUIpmenupos],al
  mov byte[GUIcmenupos],0
.noclick
  mov byte[GUIpclicked],1
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  xor ebx,ebx
  mov bl,[GUIwinorder+eax]
  cmp byte[GUIHold],2
  je near .hold2
  cmp byte[GUIHold],3
  je near .hold3
  cmp byte[GUIHold],4
  je near .hold4
  cmp byte[GUIHold],5
  je near .hold5
  cmp byte[GUIHold],6
  je near .hold6
  cmp byte[GUIHold],7
  je near .hold7
  cmp byte[GUIHold],1
  je near .hold
  cmp byte[GUIHold],255
  je .holded
  mov byte[GUICHold],0
  ret
.holded
  mov eax,[GUIHoldYlim]
  mov [GUImouseposy],eax
  mov eax,[GUIHoldXlimL]
  mov [GUImouseposx],eax
  mov cx,[GUIHoldXlimL]
  mov dx,[GUIHoldYlim]
  call Set_MousePosition
  ret
.hold2
  ; Colour Slide Bar Hold
  mov eax,[GUIHoldYlim]
  mov [GUImouseposy],eax
  mov ebx,[GUImouseposx]
  mov eax,[GUIHoldXlimL]
  cmp ebx,eax
  jnl .noless2
  mov [GUImouseposx],eax
.noless2
  mov eax,[GUIHoldXlimR]
  cmp ebx,eax
  jng .nogreat2
  mov [GUImouseposx],eax
.nogreat2
  mov byte[lastmouseholded],1
  jmp DisplayGUIOptnsClick
.hold3
  ; Box Hold
  mov ebx,[GUIHoldYlim]
  cmp ebx,[GUImouseposy]
  jl .boxhold1
  mov [GUImouseposy],ebx
.boxhold1
  mov ebx,[GUIHoldXlimL]
  cmp ebx,[GUImouseposx]
  jl .boxhold2
  mov [GUImouseposx],ebx
.boxhold2
  mov ebx,[GUIHoldYlimR]
  cmp ebx,[GUImouseposy]
  jg .boxhold3
  mov [GUImouseposy],ebx
.boxhold3
  mov ebx,[GUIHoldXlimR]
  cmp ebx,[GUImouseposx]
  jg .boxhold4
  mov [GUImouseposx],ebx
.boxhold4
  mov byte[lastmouseholded],1
  jmp GUIWindowMove
.hold4
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+ebx*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+ebx*4]
  cmp eax,[GUIHoldXlimL]
  jl near .nobhold4
  cmp edx,[GUIHoldYlim]
  jl near .nobhold4
  cmp eax,[GUIHoldXlimR]
  jg near .nobhold4
  cmp edx,[GUIHoldYlimR]
  jg near .nobhold4
  mov al,[GUICBHold2]
  mov [GUICBHold],al
  ret
.nobhold4
  mov byte[GUICBHold],0
  ret
.hold5
  ; Sound Slide Bar Hold
  mov eax,[GUIHoldYlim]
  mov [GUImouseposy],eax
  mov ebx,[GUImouseposx]
  mov eax,[GUIHoldXlimL]
  cmp ebx,eax
  jnl .noless5
  mov [GUImouseposx],eax
.noless5
  mov eax,[GUIHoldXlimR]
  cmp ebx,eax
  jng .nogreat5
  mov [GUImouseposx],eax
.nogreat5
  mov byte[lastmouseholded],1
  jmp DisplayGUISoundClick
.hold6
  ; Speed Slide Bar Hold
  mov eax,[GUIHoldYlim]
  mov [GUImouseposy],eax
  mov ebx,[GUImouseposx]
  mov eax,[GUIHoldXlimL]
  cmp ebx,eax
  jnl .noless6
  mov [GUImouseposx],eax
.noless6
  mov eax,[GUIHoldXlimR]
  cmp ebx,eax
  jng .nogreat6
  mov [GUImouseposx],eax
.nogreat6
  mov byte[lastmouseholded],1
  jmp DisplayGUISpeedClick
.hold7
  ; Video Slide Bar Hold
  mov eax,[GUIHoldYlim]
  mov [GUImouseposy],eax
  mov ebx,[GUImouseposx]
  mov eax,[GUIHoldXlimL]
  cmp ebx,eax
  jnl .noless7
  mov [GUImouseposx],eax
.noless7
  mov eax,[GUIHoldXlimR]
  cmp ebx,eax
  jng .nogreat7
  mov [GUImouseposx],eax
.nogreat7
  mov byte[lastmouseholded],1
  jmp DisplayGUIVideoClick
.hold
  cmp word[GUImouseposy],16
  jge .noover
  test byte[mousewrap],1
  jz .nomwrap
  add word[GUImouseposy],224-16
  jmp .hold
.nomwrap
  mov word[GUImouseposy],16
.noover
  mov cx,[GUImouseposy]
  sub cx,[GUIHoldy]
  add cx,[GUIHoldym]
  movsx ecx,cx
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  xor ebx,ebx
  mov bl,[GUIwinorder+eax]
  mov [GUIwinposy+ebx*4],ecx
  mov cx,[GUImouseposx]
  sub cx,[GUIHoldx]
  add cx,[GUIHoldxm]
  movsx ecx,cx
  mov [GUIwinposx+ebx*4],ecx
  ret
.notclicked
  mov byte[GUICHold],0
  mov byte[GUIpclicked],0
  mov byte[GUIHold],0

%ifdef __UNIXSDL__
  cmp byte[GUIcwinpress],1
  jne .notwinpressa
  mov byte[GUIcwinpress],0
  cmp word[GUImouseposy],3
  jb .notwinpressa
  cmp word[GUImouseposy],13
  ja .notwinpressa
  cmp word[GUImouseposx],233
  jb .notwinpressa
  cmp word[GUImouseposx],242
  ja .notwinpressa
  call SwitchFullScreen
  ret
%endif

%ifdef __WIN32__
  cmp byte[GUIcwinpress],1
  jne .notwinpressa
  mov byte[GUIcwinpress],0
  cmp word[GUImouseposy],3
  jb .notwinpressa
  cmp word[GUImouseposy],7
  ja .notwinpressa
  cmp word[GUImouseposx],233
  jb .notwinpressa
  cmp word[GUImouseposx],242
  ja .notwinpressa
  call SwitchFullScreen
  ret
%endif

.notwinpressa

%ifndef __MSDOS__
  cmp byte[GUIcwinpress],2
  jne .notwinpressb
  mov byte[GUIcwinpress],0
  cmp word[GUImouseposy],3
  jb .notwinpressb
  cmp word[GUImouseposy],13
  ja .notwinpressb
  cmp word[GUImouseposx],244
  jb .notwinpressb
  cmp word[GUImouseposx],253
  ja .notwinpressb
  mov byte[GUIQuit],1
  ret
.notwinpressb

  cmp byte[GUIcwinpress],3
  jne .notwinpressc
  mov byte[GUIcwinpress],0
  cmp word[GUImouseposy],9
  jb .notwinpressc
  cmp word[GUImouseposy],13
  ja .notwinpressc
  cmp word[GUImouseposx],233
  jb .notwinpressc
  cmp word[GUImouseposx],242
  ja .notwinpressc
%endif
%ifdef __WIN32__
  pushad
  call MinimizeWindow
  popad
%endif
%ifndef __MSDOS__
  ret
%endif
.notwinpressc

  ; ButtonProcess
  cmp byte[GUICBHold],1
  je near GUILoadData
  cmp byte[GUICBHold],2
  je near GUIProcReset
  cmp byte[GUICBHold],3
  je near GUIProcReset
  cmp byte[GUICBHold],4  ; set video mode
  je near GUIProcVideo
%ifndef __MSDOS__
  cmp byte[GUICBHold],12  ; set custom video mode
  je near GUIProcCustomVideo
%endif
  cmp byte[GUICBHold],37 ; reset ntsc options
  je near GUIProcVideo
  cmp byte[GUICBHold],10
  je near GUIProcStates
  cmp byte[GUICBHold],11
  je near GUIProcStates
  cmp byte[GUICBHold],5
  je near CheatCodeRemove
  cmp byte[GUICBHold],6
  je near CheatCodeToggle
  cmp byte[GUICBHold],7
  je near CheatCodeSave
  cmp byte[GUICBHold],8
  je near CheatCodeLoad
  cmp byte[GUICBHold],9
  je near ProcessCheatCode
  cmp byte[GUICBHold],33
  je near CheatCodeFix
  cmp byte[GUICBHold],14
  je near SetDevice
  cmp byte[GUICBHold],15
  je near CalibrateDev1
  cmp byte[GUICBHold],16  ; movie replay
  jne .notMoviePlay
  mov byte[GUICBHold],0
  pushad
  call MoviePlay
  popad
  ret
.notMoviePlay
  cmp byte[GUICBHold],17
  jne .notMovieRecord0    ; movie record
  mov byte[GUICBHold],0
  pushad
  call MovieRecord
  popad
  ret
.notMovieRecord0
  cmp byte[GUICBHold],18  ; movie stop
  jne .notMovieStop
  mov byte[GUICBHold],0
  pushad
  call MovieStop
  popad
  ret
.notMovieStop
  cmp byte[GUICBHold],19  ; overwrite zmv ? yes
  jne .notMovieRecord1
  mov byte[GUICBHold],0
  pushad
  call MovieRecord
  popad
  ret
.notMovieRecord1
  cmp byte[GUICBHold],20  ; overwrite zmv ? no
  jne .notSkipMovie
  mov byte[GUICBHold],0
  pushad
  call SkipMovie
  popad
  ret
.notSkipMovie
  cmp byte[GUICBHold],29  ; insert chapter
  jne .noInsertChap
  mov byte[GUICBHold],0
  pushad
  call MovieInsertChapter
  popad
  ret
.noInsertChap
  cmp byte[GUICBHold],30  ; back to previous chapter
  jne .noPrevChap
  mov byte[GUICBHold],0
  mov byte[GUIQuit],2
  pushad
  call MovieSeekBehind
  popad
  ret
.noPrevChap
  cmp byte[GUICBHold],31  ; jump to next chapter
  jne .noNextChap
  mov byte[GUICBHold],0
  mov byte[GUIQuit],2
  pushad
  call MovieSeekAhead
  popad
  ret
.noNextChap
  cmp byte[GUICBHold],32  ; append movie
  jne .noAppend
  mov byte[GUICBHold],0
  mov byte[GUIQuit],2
  pushad
  call MovieAppend
  popad
  ret
.noAppend
  cmp byte[GUICBHold],34  ; dump raw
  jne .noDumpRaw
  mov byte[GUICBHold],0
  mov byte[GUIQuit],2
  pushad
  call MovieDumpRaw
  popad
  cmp byte[MovieVideoMode],2
  jb .nomencoder
  cmp byte[mencoderExists],0
  jne .nomencoder
  call guimencodermsg
.nomencoder
  ret
.noDumpRaw
  cmp byte[GUICBHold],40
  je near SetAllKeys
  cmp byte[GUICBHold],50
  jne .notstartcheat
  call CheatCodeSearchInit
.notstartcheat
  cmp byte[GUICBHold],60
  jne .nocombclear
  mov byte[GUIComboTextH],0
  mov byte[GUINumCombo],0
  mov dword[GUIComboKey],0
.nocombclear
  cmp byte[GUICBHold],61
  jne .nocomboadd
  cmp byte[NumCombo],50
  je .nocomboadd
  call ComboAdder
.nocomboadd
  cmp byte[GUICBHold],62
  jne .nocomboreplace
  cmp byte[NumCombo],0
  je .nocomboreplace
  call ComboReplace
.nocomboreplace
  cmp byte[GUICBHold],63
  jne .nocomboremove
  cmp byte[NumCombo],0
  je .nocomboremove
  call ComboRemoval
.nocomboremove
  cmp byte[GUICBHold],51
  jne .notrestartcheat
  mov byte[CheatWinMode],0
  mov byte[CheatSearchStatus],0
.notrestartcheat
  cmp byte[GUICBHold],52
  jne .notviewcheat
  mov byte[CheatWinMode],2
.notviewcheat
  cmp byte[GUICBHold],53
  jne .notsearchprocess
  call CheatCodeSearchProcess
.notsearchprocess
  cmp byte[GUICBHold],54
  jne .notreturnview
  mov byte[CheatWinMode],1
.notreturnview
  cmp byte[GUICBHold],55
  jne .notadd
  cmp dword[NumCheatSrc],0
  je .notadd
  mov byte[CheatWinMode],3
  mov byte[CurCStextpos],0
  mov byte[CSInputDisplay],'_'
  mov byte[CSInputDisplay+1],0
  mov byte[CSDescDisplay],0
.notadd
  cmp byte[GUICBHold],56
  jne .notreturnadd
  mov byte[CheatWinMode],2
.notreturnadd
  cmp byte[GUICBHold],57
  jne .noinsertscc
  call AddCSCheatCode
;    call InsertSearchDescription
.noinsertscc
%ifndef __MSDOS__
  cmp byte[GUICBHold],65
  jne .nozsnespage
  pushad
  call ZsnesPage
  popad
.nozsnespage
  cmp byte[GUICBHold],66
  jne .nodocspage
  pushad
  call DocsPage
  popad
.nodocspage
%endif
  mov byte[GUICBHold],0
  ret
.checkmenuboxclick
  mov byte[GUIpclicked],1
  xor eax,eax
  mov al,[GUIwinptr]
  or eax,eax
  jz near .nomore
  dec eax
  xor ebx,ebx
  mov bl,[GUIwinorder+eax]
  mov ecx,[GUIwinposx+ebx*4]
  cmp ecx,[GUImouseposx]
  jge .noinsdwin
  mov ecx,[GUIwinposy+ebx*4]
  cmp ecx,[GUImouseposy]
  jge .noinsdwin
  mov ecx,[GUIwinposx+ebx*4]
  add ecx,[GUIwinsizex+ebx*4]
  cmp ecx,[GUImouseposx]
  jle .noinsdwin
  mov ecx,[GUIwinposy+ebx*4]
  add ecx,[GUIwinsizey+ebx*4]
  add ecx,10
  cmp ecx,[GUImouseposy]
  jle .noinsdwin
  jmp GUIWinClicked
.noinsdwin
.nextwindowfind
  or eax,eax
  jz near .nomore
  dec eax
  mov bl,[GUIwinorder+eax]
  mov ecx,[GUIwinposx+ebx*4]
  cmp ecx,[GUImouseposx]
  jge .nextwindowfind
  mov ecx,[GUIwinposy+ebx*4]
  cmp ecx,[GUImouseposy]
  jge .nextwindowfind
  mov ecx,[GUIwinposx+ebx*4]
  add ecx,[GUIwinsizex+ebx*4]
  cmp ecx,[GUImouseposx]
  jle .nextwindowfind
  mov ecx,[GUIwinposy+ebx*4]
  add ecx,[GUIwinsizey+ebx*4]
  add ecx,10
  cmp ecx,[GUImouseposy]
  jle .nextwindowfind
  ; Shift all following windows downwards by 1
.nextfind
  inc eax
  cmp al,[GUIwinptr]
  je .foundend
  mov cl,[GUIwinorder+eax]
  mov [GUIwinorder+eax-1],cl
  jmp .nextfind
.foundend
  mov byte[GUIpclicked],0
  mov [GUIwinorder+eax-1],bl
  ret
.nomore
  cmp dword[SantaPos],272
  je .nosanta
  cmp byte[ShowTimer],0
  jne .nosanta
  mov dword[MsgGiftLeft],36*4
  mov byte[ShowTimer],1
.nosanta
  ret

GUIProcVideo:
  cmp byte[GUICBHold],37
  mov byte[GUICBHold],0
  jne .noreset
  mov byte[NTSCBlend],0
  mov byte[NTSCRef],0
  mov byte[NTSCHue],0
  mov byte[NTSCSat],0
  mov byte[NTSCCont],0
  mov byte[NTSCBright],0
  mov byte[NTSCSharp],0
  mov byte[NTSCWarp],0
  pushad
  call NTSCFilterInit
  popad
  ret
.noreset
%ifdef __MSDOS__
  mov al,[GUIcurrentvideocursloc]
  cmp al,3
  je .yes16b
  cmp al,5
  je .yes16b
  cmp al,7
  je .yes16b
  cmp al,10
  je .yes16b
  cmp al,8
  jne .no16b
.yes16b
%endif
;    mov byte[newengen],0
;    mov byte[cfgnewgfx],0
;    jmp .skipno16b
.no16b
  mov byte[newengen],1
.skipno16b
  call guiprevideo
  mov al,[cvidmode]
  mov [.prevvid],al
  mov al,[GUIcurrentvideocursloc]
  mov [cvidmode],al
  mov byte[ExitFromGUI],1
  call initvideo
  cmp byte[videotroub],1
  je .failed
  call GUISetPal
  call guipostvideo
  cmp dword[GUIkeydelay],0
  je .failed2
  mov dword[GUIkeydelay],0
  ret
.fullsc
  mov [PrevFSMode],al
  ret
.failed
  mov byte[videotroub],0
  mov al,[.prevvid]
  mov [cvidmode],al
  call initvideo
  call GUISetPal
  call guipostvideofail
  mov dword[GUIkeydelay],0
  ret
.failed2
  mov byte[videotroub],0
  mov al,[.prevvid]
  mov [cvidmode],al
  call initvideo
  call GUISetPal
  call vidpastecopyscr
  mov dword[GUIkeydelay],0
  ret
SECTION .bss
.prevvid resb 1
SECTION .text

%ifndef __MSDOS__
GUIProcCustomVideo:
  mov byte[GUICBHold],0
  mov dword[GUIInputBox],0
  pushad
  call SetCustomXY
  popad
%ifdef __WIN32__
  cmp byte[cvidmode],37
%else
  cmp byte[cvidmode],20
%endif
  jl .notcustomres
  pushad
  call initwinvideo
  popad
.notcustomres
  ret
%endif

NEWSYM SwitchFullScreen
  pushad
  call Clear2xSaIBuffer
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUIWFVID+eax],0
  je .fullsc
  mov al,[PrevWinMode]
  mov [cvidmode],al
  call initvideo
  popad
  ret
.fullsc
  mov al,[PrevFSMode]
  mov [cvidmode],al
  call initvideo
  popad
  ret

GUIWindowMove:
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  xor ebx,ebx
  mov bl,[GUIwinorder+eax]
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+ebx*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+ebx*4]
  cmp ebx,13
  je near DisplayGUICheatSearchClick2
  cmp ebx,7
  je near DisplayGUICheatClick2
  cmp ebx,5
  je near DisplayGUIVideoClick2
  cmp ebx,3
  je near DisplayGUIInputClick2
  cmp ebx,16
  je near DisplayGUIComboClick2
  jmp DisplayGUIConfirmClick2

GUIWinClicked:
  mov ecx,[GUImouseposx]
  sub ecx,[GUIwinposx+ebx*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+ebx*4]
  mov esi,ecx
  sub esi,[GUIwinsizex+ebx*4]
  add esi,10
  cmp esi,-1
  jle .noclose
  cmp esi,10
  jge .noclose
  cmp edx,0
  jle .noclose
  cmp edx,10
  jge .noclose
  mov byte[GUIwinorder+eax],0
  mov byte[GUIwinactiv+ebx],0
  dec byte[GUIwinptr]
  ret
.noclose
  cmp edx,10
  jge .nomove
  mov byte[GUIHold],1
  mov ax,[GUIwinposx+ebx*4]
  movsx eax,ax
  mov [GUIHoldxm],eax
  mov ax,[GUIwinposy+ebx*4]
  movsx eax,ax
  mov [GUIHoldym],eax
  xor eax,eax
  mov ax,[GUImouseposx]
  mov [GUIHoldx],eax
  mov ax,[GUImouseposy]
  mov [GUIHoldy],eax
  ret
.nomove
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+ebx*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+ebx*4]
  cmp ebx,1
  je near DisplayGUIConfirmClick
  cmp ebx,2
  je near DisplayGUIChoseSaveClick
  cmp ebx,3
  je near DisplayGUIInputClick
  cmp ebx,4
  je near DisplayGUIOptionClick
  cmp ebx,5
  je near DisplayGUIVideoClick
  cmp ebx,6
  je near DisplayGUISoundClick
  cmp ebx,7
  je near DisplayGUICheatClick
  cmp ebx,8
  je near DisplayNetOptnsClick
  cmp ebx,9
  je near DisplayGameOptnsClick
  cmp ebx,10
  je near DisplayGUIOptnsClick
  cmp ebx,11
  je near DisplayGUIAboutClick
  cmp ebx,12
  je near DisplayGUIResetClick
  cmp ebx,13
  je near DisplayGUICheatSearchClick
  cmp ebx,14
  je near DisplayGUIStatesClick
  cmp ebx,15
  je near DisplayGUIMovieClick
  cmp ebx,16
  je near DisplayGUIComboClick
  cmp ebx,17
  je near DisplayGUIAddOnClick
  cmp ebx,18
  je near DisplayGUIChipClick
  cmp ebx,19
  je near DisplayGUIPathsClick
  cmp ebx,20
  je near DisplayGUISaveClick
  cmp ebx,21
  je near DisplayGUISpeedClick
  ret

%macro GUIClickArea 5
  cmp eax,%1
  jl %5
  cmp edx,%2
  jl %5
  cmp eax,%3
  jg %5
  cmp edx,%4
  jg %5
%endmacro

%macro GUIClickCButton 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%%noclick
%endmacro

%macro GUIClickCButtonf 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%ifdef __WIN32__
  pushad
  call %4
  popad
%endif
%%noclick
%endmacro

%macro GUIClickCButtonK 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  pushad
  call %4
  popad
%%noclick
%endmacro

%macro GUIClickCButtonN 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
%ifdef __WIN32__
  cmp %3,1
  je %%dontenablek43
  mov byte[Keep4_3Ratio],1
%%dontenablek43
%endif
  xor %3,1
%ifdef __WIN32__
  pushad
  call %4
  popad
%endif
%%noclick
%endmacro

%macro GUIClickCButtonC 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  mov dword[GUIccombviewloc],0
  mov dword[GUIccombcursloc],0
  mov dword[GUIccomblcursloc],0
%%noclick
%endmacro

%macro GUIClickCButtonM 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  mov byte[MultiTap],1
  cmp byte[pl12s34],1
  je %%nomtap
  cmp byte[pl3contrl],0
  jne %%noclick
  cmp byte[pl4contrl],0
  jne %%noclick
  cmp byte[pl5contrl],0
  jne %%noclick
%%nomtap
  mov byte[MultiTap],0
%%noclick
%endmacro

%macro GUIClickCButtonID 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  call SetDevice
%%noclick
%endmacro

%macro GUIClickCButton2 5
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,%5
  mov al,%3
  mov %4,al
  ret
%%noclick
%endmacro

%macro GUIClickCButton3 5
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,%5
  mov al,%3
  mov %4,al
  call ClearScreen
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
%%loop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz %%loop
  pop ecx
  pop eax
  ret
%%noclick
%endmacro

%macro GUIClickCButton4 5
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp %3,%5
  jne %%set
  xor %3,%5
  jmp %%done
%%set
  mov %3,%5
%%done
  mov al,%3
  mov %4,al
%ifdef __MSDOS__
  call ClearScreen
  cmp byte[En2xSaI],0
  jne %%nomodeq
  cmp byte[cvidmode],2
  jne %%nomodeq
  jmp %%refresh
%%nomodeq
  cmp byte[cvidmode],5
  jne %%nomodex
%%refresh
  mov byte[cbitmode],1
  call initvideo2
  mov byte[cbitmode],0
  call GUISetPal
%%nomodex
%endif
  ret
%%noclick
%endmacro

%macro GUIClickCButton5 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp %3,%4
  jne %%set
  xor %3,%4
  jmp %%clear
%%set
  mov %3,%4
%%clear
  ret
%%noclick
%endmacro

%macro GUIClickCButton6 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp %3,%4
  jne %%set
  xor %3,%4
  jmp %%clear
%%set
  mov %3,%4
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
%%loop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz %%loop
  pop ecx
  pop eax
%%clear
  ret
%%noclick
%endmacro

%macro GUIClickCButtonfr 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  mov byte[FPSOn],0
  cmp %3,0
  je %%zero
  mov %3,0
  jmp %%notzero
%%zero
  mov %3,1
%%notzero
%%noclick
%endmacro

%macro GUIClickCButtonL 2
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor byte[showallext],1
  mov dword[GUIcurrentfilewin],0
  call GetLoadData
  ret
%%noclick
%endmacro

%macro GUIClickCButtonI 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  pushad
  call Clear2xSaIBuffer
  popad
%%noclick
%endmacro

%macro GUIClickCButtonAS 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%%noclick
%endmacro

%macro GUIClickCButtonT 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  mov %4,0
%%noclick
%endmacro

%macro GUIClickCButtonMMX 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  call MMXCheck
%%noclick
%endmacro

%macro GUIClickCButton5MMXC 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp byte[MMXSupport],0
  je %%clear
  cmp %3,%4
  jne %%set
  xor %3,%4
  jmp %%clear
%%set
  mov %3,%4
%%clear
  ret
%%noclick
%endmacro

%macro GUISlidebarPostImpl 13 ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  cmp dword[%11],0
  je near %%nolower
  cmp dword[GUIdispmode],1
  je near %%slidebar
  GUIClickArea %1,%2-7,%3,%2-1,%%noupper
  mov byte[%12],%13
  mov byte[GUICHold],%7
  jmp %5
%%noupper
  GUIClickArea %1,%4+1,%3,%4+7,{near %%nolower}
  mov byte[%12],%13
  mov byte[GUICHold],%7+1
  jmp %6
%%slidebar
  GUIClickArea %1,%2,%3,%4,{near %%nolower}
  mov byte[%12],%13
  ; displacement = (GUIdispmode * pixeldisp. / (listsize-scsize))
  mov eax,[%11]
  sub eax,%8
  mov ebx,edx
  sub ebx,[GUIlastypos]
  imul ebx
  mov ebx,[GUIlastdispval]
  or ebx,ebx
  jz near %%zerodiv
  test ebx,80000000h
  jnz near %%zerodiv
  idiv ebx
  mov ebx,[GUIlastvpos]
  add ebx,eax
  mov [%9],ebx
  mov ebx,[GUIlastcpos]
  add ebx,eax
  mov [%10],ebx
  test dword[%9],8000000h
  jz %%noneg
  mov dword[%9],0
%%noneg
  test dword[%10],8000000h
  jz %%noneg2
  mov dword[%10],0
%%noneg2
  mov eax,[%11]
  dec eax
  cmp [%10],eax
  jb %%notover
  mov [%10],eax
%%notover
  sub eax,%8-1
  cmp [%9],eax
  jb %%notover2
  mov [%9],eax
%%notover2
%%zerodiv
  ret
%%nolower
%endmacro

SECTION .bss
GUIlastdispval resd 1
GUIdispmode    resd 1
GUIlastypos    resd 1
GUIlastcpos    resd 1
GUIlastvpos    resd 1
SECTION .text

%macro GUISlidebarImpl 10 ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  mov dword[GUIdispmode],0
  GUIClickArea %1,%2,%3,%4,{near %%nobar}
  sub edx,%2
  cmp [%5+4],edx
  ja near %%above
  cmp [%5+8],edx
  jb near %%below
  add edx,%2
  mov [GUIlastypos],edx
  mov dword[GUIdispmode],1
  mov eax,[GUIwinposy+%10*4]
  add eax,%2
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%4
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov eax,[%5]
  mov [GUIlastdispval],eax
  mov eax,[%8]
  mov [GUIlastcpos],eax
  mov eax,[%7]
  mov [GUIlastvpos],eax
  mov byte[GUIHold],3
  ret
%%above
  sub dword[%7],%6
  sub dword[%8],%6
  test dword[%7],8000000h
  jz %%noneg
  mov dword[%7],0
  mov dword[%8],0
%%noneg
  ret
%%below
  add dword[%7],%6
  add dword[%8],%6
  mov ebx,[%9]
  dec ebx
  cmp [%8],ebx
  jb %%notoverflow
  mov [%8],ebx
  sub ebx,%6-1
  mov [%7],ebx
%%notoverflow
  mov ebx,[%9]
  sub ebx,%6
  cmp [%7],ebx
  jb %%notoverflow2
  mov [%7],ebx
%%notoverflow2
  ret
%%nobar
  ; upper arrow
  GUIClickArea %1,%2-7,%3,%2-1,%%noupper
  mov eax,[GUIwinposy+%10*4]
  add eax,%2-7
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%2-1
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%noupper
  ; lower arrow
  GUIClickArea %1,%4+1,%3,%4+7,%%nolower
  mov eax,[GUIwinposy+%10*4]
  add eax,%4+1
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%4+7
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%nolower
%endmacro

%macro GUIPostWinBoxMove 11 ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  cmp eax,%1
  jl near %%noload
  cmp eax,%3
  jg near %%noload
  cmp edx,%2
  je near %%scrollup
  cmp edx,%4
  jne near %%noload
  ; Scroll Down
  cmp dword[%7],%11
  jbe %9
  mov ebx,[%5]
  add ebx,%11-1
  mov [%6],ebx
%9
  cmp dword[GUIScrolTim1],0
  jne near %%donescrol
  mov ebx,[%7]
  dec ebx
  cmp ebx,[%6]
  jbe %%donescrolldown
  inc dword[%6]
  cmp dword[%7],%11
  jbe near %%donescrol
  inc dword[%5]
  mov ebx,[%7]
  sub ebx,%11
  cmp ebx,[%5]
  ja near %%donescrolldown
  mov [%5],ebx
  jmp %%donescrolldown
%%scrollup
  mov ebx,[%5]
  mov [%6],ebx
%8
  cmp dword[GUIScrolTim1],0
  jne near %%donescrol
  cmp dword[%6],0
  je %%donescrolldown
  dec dword[%6]
  cmp dword[%5],0
  je %%donescrolldown
  dec dword[%5]
%%donescrolldown
  mov dword[GUIScrolTim1],1
  cmp dword[GUIScrolTim2],4
  jb %%noscrol1
  inc dword[GUIScrolTim1]
%%noscrol1
  cmp dword[GUIScrolTim2],0
  je %%noscrol2
  inc dword[GUIScrolTim1]
  dec dword[GUIScrolTim2]
%%noscrol2
%%donescrol
  jmp %10
%%noload
%endmacro

%macro GUIPHoldbutton 5
  GUIClickArea %1,%2,%3,%4,%%nohold
  mov dword[GUIHoldXlimL],%1
  mov dword[GUIHoldXlimR],%3
  mov dword[GUIHoldYlim],%2
  mov dword[GUIHoldYlimR],%4
  mov byte[GUICBHold2],%5
  mov byte[GUIHold],4
%%nohold
%endmacro

%macro GUIPHoldbutton2 8
  GUIClickArea %1,%2,%3,%4,%%nohold
  cmp byte[GUIHold],0
  jne %%nohold
  mov dword[GUIHoldXlimL],%1
  mov dword[GUIHoldXlimR],%3
  mov dword[GUIHoldYlim],%2
  mov dword[GUIHoldYlimR],%4
  mov byte[GUICBHold2],%5
  mov byte[GUIHold],4
  cmp %6,%8
  je %%nohold
  add %6,%7
%%nohold
%endmacro

%macro GUITextBoxInput 6
  GUIClickArea %1,%2,%3,%4,%%noclick
  mov dword[GUIEditString],%5
  mov dword[GUIEditStringmLen],%6
  mov dword[GUIEditStringLTxt],0
  mov dword[GUIEditStringcWin],1
%%noclick
%endmacro

%macro GUITextBoxInputB 5
  GUIClickArea %1,%2,%3,%4,%%noclick
  mov byte[GUINetTextm2],%5
%%noclick
%endmacro

%macro GUITextBoxInputNach 6
  GUIClickArea %1,%2,%3,%4,%%noclick
  pushad
  call init_save_paths
  popad
  mov dword[GUIInputBox],%5
  mov dword[GUIInputLimit],%6-1
%%noclick
%endmacro

%macro GUIPButtonHole 4
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
%%noclick
%endmacro

%macro GUIPButtonHole2 6
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
  mov %5,%6
%%noclick
%endmacro

%macro GUIPButtonHoleLoad 4
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
  pushad
  call GetLoadData
  popad
%%noclick
%endmacro

%macro GUIWinControl 13 ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIClickArea %1,%2,%3,%4,{near %%noload}
  cmp dword[%7],0
  je near %%noload
  mov dword[%5],%11
  mov eax,edx
  sub eax,%8
  xor edx,edx
  mov ebx,%9
  div ebx
  add eax,[%6]
  mov ebx,[%7]
  dec ebx
  cmp eax,ebx
  jg %%nomod
  mov [%10],eax
%%nomod
  cmp byte[GUIHold],0
  jne %%held
  cmp dword[GUIDClickTL],0
  je %%noprevhold
  cmp byte[GUIDClCWin],%11
  jne %%noprevhold
  cmp [GUIDClCEntry],eax
  jne %%noprevhold
  mov dword[GUIDClickTL],0
  mov eax,%12
  cmp eax,1
  jne %%cheat
  call GUILoadData
  ret
%%cheat
  call CheatCodeToggle
  ret
%%noprevhold
  mov dword[GUIDClickTL],%13
  mov byte[GUIDClCWin],%11
  mov [GUIDClCEntry],eax
%%held
  mov eax,[GUIwinposy+%12*4]
  add eax,%2-1
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%12*4]
  add eax,%4+1
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%noload
%endmacro

%macro GUIWinControl2 13 ;Key Combination Editor Exclusive
  GUIClickArea %1,%2,%3,%4,{near %%noload}
  cmp dword[%7],0
  je near %%noload
  mov dword[GUIccomblcursloc],0FFFFFFFFh
  mov dword[%5],%11
  mov eax,edx
  sub eax,%8
  xor edx,edx
  mov ebx,%9
  div ebx
  add eax,[%6]
  mov ebx,[%7]
  dec ebx
  cmp eax,ebx
  jg %%nomod
  mov [%10],eax
%%nomod
  cmp byte[GUIHold],0
  jne %%held
  cmp dword[GUIDClickTL],0
  je %%noprevhold
  cmp byte[GUIDClCWin],%11
  jne %%noprevhold
  cmp [GUIDClCEntry],eax
  jne %%noprevhold
  mov dword[GUIDClickTL],0
  mov eax,%12
  cmp eax,1
  jne %%cheat
  call GUILoadData
  ret
%%cheat
  call CheatCodeToggle
  ret
%%noprevhold
  mov dword[GUIDClickTL],%13
  mov byte[GUIDClCWin],%11
  mov [GUIDClCEntry],eax
%%held
  mov eax,[GUIwinposy+%12*4]
  add eax,%2-1
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%12*4]
  add eax,%4+1
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%noload
%endmacro

%macro DGOptnsProcBox 3
  GUIClickArea %1,%2,%1+19,%2+6,%%skip
  push edi
  call guipresstest
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %3,ebx
%ifndef __MSDOS__
  cmp dword[keycontrolval],0
  je %%notwin32
  push ebx
  mov ebx,[keycontrolval]
  mov byte[ebx],1
  pop ebx
%%notwin32
%endif
  ret
%%cancelkey
  xor ebx,ebx
  mov %3,ebx
  ret
%%skip
%endmacro

%macro DGOptnsProcBoxc 5
  GUIClickArea %1,%2,%3,%4,%%skip
  push edi
  call guipresstest
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %5,ebx
  ret
%%cancelkey
  cmp ebx,1
  jne %%noesc
  mov byte[pressed+1],2
%%noesc
  xor ebx,ebx
  mov %5,ebx
  ret
%%skip
%endmacro

%macro DGOptnsProcBoxb 3
  GUIClickArea %1,%2,%1+19,%2+6,%%skip
  push edi
  call guipresstest
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %3,ebx
  jmp %%donekey
%%cancelkey
  cmp ebx,1
  jne %%noesc
  mov byte[pressed+1],2
%%noesc
  xor ebx,ebx
  mov %3,ebx
%%donekey
  ret
%%skip
%endmacro

%macro FilterExcl 3-* ; minX, minY, bytes to zero
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
%rotate 1
%rep (%0-2)
  %rotate 1
  mov byte[%1],0
%endrep
%%noclick
%endmacro

%macro GUIPTabClick 3-* ; minX, maxX, value, var, vars to zero
  GUIClickArea %1+1,11,%2-1,22,%%noclick
  mov dword[%4],%3
%if (%0>4)
  %rotate 4
%rep (%0-4)
  mov dword[%1],0
  %rotate 1
%endrep
%endif
  mov byte[GUIFreshInputSelect],1
%%noclick
%endmacro

%macro GUIBoxVar 6
  GUIClickArea %1,%2,%3,%4,%%nope
  mov dword[%5],%6
%%nope
%endmacro

%macro ComboBoxProc 5
  GUIClickArea %1,%2,%3,%4,%%nope
  cmp byte[GUINumCombo],42
  je %%nope
  xor ebx,ebx
  mov bl,[GUINumCombo]
  mov byte[GUIComboData+ebx],%5
  inc byte[GUINumCombo]
%%nope
%endmacro

%macro ComboBoxProcD 4
  GUIClickArea %1,%2,%3,%4,%%nope
  cmp byte[GUINumCombo],0
  je %%nope
  dec byte[GUINumCombo]
%%nope
%endmacro

; Mouse implementation

DisplayGUIConfirmClick2:
  cmp dword[GUIfileentries],1
  jbe near .noload2
  GUISlidebarPostImpl 146,33,153,33+93,.scrollupsl,.scrolldownsl,1,15,GUIcurrentviewloc,GUIcurrentcursloc,GUIfileentries,GUIcurrentfilewin,0
  GUIPostWinBoxMove 5,26,144,27+15*7,GUIcurrentviewloc,GUIcurrentcursloc,GUIfileentries,.scrollupsl,.scrolldownsl,DisplayGUIConfirmClick.skipscrol,15
.noload2
  cmp dword[GUIdirentries],1
  jbe near .noload3
  GUISlidebarPostImpl 230,33,237,33+93,.scrollupsld,.scrolldownsld,3,15,GUIcurrentdirviewloc,GUIcurrentdircursloc,GUIdirentries,GUIcurrentfilewin,1
  GUIPostWinBoxMove 160,26,228,27+15*7,GUIcurrentdirviewloc,GUIcurrentdircursloc,GUIdirentries,.scrollupsld,.scrolldownsld,DisplayGUIConfirmClick.skipscrol,15
.noload3
DisplayGUIConfirmClick:
;DrawSlideBarWin 1,133,43,[GUIcurrentviewloc],[GUIfileentries],15,94,[GUILStA]
  ; offset 0 = (ysize-(yend-ystart+1)), offset 1 = starty, offset 2 = endy
  ; SlideBar Implementation
  GUISlidebarImpl 146,33,153,33+93,GUILStA,15,GUIcurrentviewloc,GUIcurrentcursloc,GUIfileentries,1
  GUISlidebarImpl 230,33,237,33+93,GUILStB,15,GUIcurrentdirviewloc,GUIcurrentdircursloc,GUIdirentries,1
  mov dword[GUIScrolTim1],0
  mov dword[GUIScrolTim2],6
.skipscrol
  GUIClickCButtonL 10,187
  GUIWinControl 5,27,144,26+15*7,GUIcurrentfilewin,GUIcurrentviewloc,GUIfileentries,27,7,GUIcurrentcursloc,0,1,30
  GUIWinControl 160,27,228,26+15*7,GUIcurrentfilewin,GUIcurrentdirviewloc,GUIdirentries,27,7,GUIcurrentdircursloc,1,1,30
  GUIPHoldbutton 186,165,228,176,1
  GUIPButtonHoleLoad 9,163,byte[GUIloadfntype],0
  GUIPButtonHoleLoad 9,171,byte[GUIloadfntype],1
%ifdef __MSDOS__
  GUIPButtonHoleLoad 9,179,byte[GUIloadfntype],2
%endif
  GUIClickCButton5 144,177,byte[ForceROMTiming],1
  GUIClickCButton5 144,187,byte[ForceROMTiming],2
  GUIClickCButton5 184,177,byte[ForceHiLoROM],1
  GUIClickCButton5 184,187,byte[ForceHiLoROM],2
  ret

DisplayGUIChoseSaveClick:
  GUIPHoldbutton2 94,59,102,67,80,byte[GUIChoseSlotTextX],1,'9'
  GUIPHoldbutton2 105,59,113,67,81,byte[GUIChoseSlotTextX],-1,'0'
  GUIPButtonHole 10,28,byte[GUIChoseSaveText2],0
  GUIPButtonHole 30,28,byte[GUIChoseSaveText2],1
  GUIPButtonHole 50,28,byte[GUIChoseSaveText2],2
  GUIPButtonHole 70,28,byte[GUIChoseSaveText2],3
  GUIPButtonHole 90,28,byte[GUIChoseSaveText2],4
  GUIPButtonHole 10,43,byte[GUIChoseSaveText2],5
  GUIPButtonHole 30,43,byte[GUIChoseSaveText2],6
  GUIPButtonHole 50,43,byte[GUIChoseSaveText2],7
  GUIPButtonHole 70,43,byte[GUIChoseSaveText2],8
  GUIPButtonHole 90,43,byte[GUIChoseSaveText2],9
  movzx eax,byte[GUIChoseSlotTextX]
  sub al,'0'
  mov dl,10
  mul dl
  add al,byte[GUIChoseSaveText2]
  mov [current_zst],eax
  ret

DisplayGUIMovieClick:
  cmp byte[MovieRecordWinVal],0        ; Overwrite Window
  je near .displayboxa
  GUIPHoldbutton 17,55,59,66,19
  GUIPHoldbutton 70,55,112,66,20
  ret

.displayboxa                             ; Main Window
  GUIPHoldbutton 9,50,48,61,16         ; Buttons
  GUIPHoldbutton 55,50,94,61,17
  GUIPHoldbutton 103,50,144,61,18
  GUIPHoldbutton 153,50,193,61,32
  GUIPHoldbutton 9,68,112,79,29
  GUIPHoldbutton 9,84,112,95,30
  GUIPHoldbutton 9,100,112,111,31
  GUIPHoldbutton 132,165,197,176,34

  GUIPButtonHole 10,25,byte[CMovieExt],'v'       ; Radios buttons
  GUIPButtonHole 30,25,byte[CMovieExt],'1'
  GUIPButtonHole 50,25,byte[CMovieExt],'2'
  GUIPButtonHole 70,25,byte[CMovieExt],'3'
  GUIPButtonHole 90,25,byte[CMovieExt],'4'
  GUIPButtonHole 10,35,byte[CMovieExt],'5'
  GUIPButtonHole 30,35,byte[CMovieExt],'6'
  GUIPButtonHole 50,35,byte[CMovieExt],'7'
  GUIPButtonHole 70,35,byte[CMovieExt],'8'
  GUIPButtonHole 90,35,byte[CMovieExt],'9'

  GUIPButtonHole 115,25,byte[MovieStartMethod],0    ; Start From
  GUIPButtonHole 152,25,byte[MovieStartMethod],1
  GUIPButtonHole 200,25,byte[MovieStartMethod],2
  GUIPButtonHole 115,35,byte[MovieStartMethod],3

  GUIPButtonHole 9,128,byte[MovieVideoMode],0      ; Movie Options
  GUIPButtonHole 9,138,byte[MovieVideoMode],1
  GUIPButtonHole 9,148,byte[MovieVideoMode],2
  GUIPButtonHole 9,158,byte[MovieVideoMode],3
  GUIPButtonHole 9,168,byte[MovieVideoMode],4
  ;GUIPButtonHole 9,178,byte[MovieVideoMode],5

  GUIClickCButton 130,128,byte[MovieAudio]
  GUIClickCButton 130,138,byte[MovieAudioCompress]
  GUIClickCButton 130,148,byte[MovieVideoAudio]

  DGOptnsProcBox 116,71,[KeyInsrtChap]           ; Keyboard Shortcut Boxes
  DGOptnsProcBox 116,87,[KeyPrevChap]
  DGOptnsProcBox 116,103,[KeyNextChap]

  GUIClickCButton 144,67,byte[MovieDisplayFrame]   ; Checkbox
  ret

%macro GUIInputSetIndKey 1
  ;Check if controller is set
  push eax
  mov eax,[keycontrolval]
  cmp dword[eax],0
  pop eax
  je near %%lockInputChange

  DGOptnsProcBox 45,105,[%1upk]    ; Up
  DGOptnsProcBox 45,115,[%1downk]  ; Down
  DGOptnsProcBox 45,125,[%1leftk]  ; Left
  DGOptnsProcBox 45,135,[%1rightk] ; Right
  DGOptnsProcBox 45,145,[%1startk] ; Start
  DGOptnsProcBox 45,155,[%1selk]   ; Select
  DGOptnsProcBox 85,105,[%1Ak]     ; A
  DGOptnsProcBox 85,115,[%1Bk]     ; B
  DGOptnsProcBox 85,125,[%1Xk]     ; X
  DGOptnsProcBox 85,135,[%1Yk]     ; Y
  DGOptnsProcBox 85,145,[%1Lk]     ; L
  DGOptnsProcBox 85,155,[%1Rk]     ; R

  DGOptnsProcBox 125,105,[%1Xtk]   ; X Turbo
  DGOptnsProcBox 125,115,[%1Ytk]   ; Y Turbo
  DGOptnsProcBox 125,125,[%1Ltk]   ; L Turbo
  DGOptnsProcBox 165,105,[%1Atk]   ; A Turbo
  DGOptnsProcBox 165,115,[%1Btk]   ; B Turbo
  DGOptnsProcBox 165,125,[%1Rtk]   ; R Turbo

  DGOptnsProcBox 125,145,[%1DLk]   ; Down-Left
  DGOptnsProcBox 125,155,[%1DRk]   ; Down-Right
  DGOptnsProcBox 165,145,[%1ULk]   ; Up-Left
  DGOptnsProcBox 165,155,[%1URk]   ; Up-Right
%%lockInputChange
%endmacro

DisplayGUIInputClick2:
  push eax
  mov eax,[NumInputDevices]
  mov [GUINumValue],eax
  pop eax
  GUISlidebarPostImpl 109,42,116,69,.scrollup,.scrolldown,9,5,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,GUIBlankVar,1
  GUIPostWinBoxMove 5,35,107,35+5*8,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,.scrollup,.scrolldown,DisplayGUIInputClick.skipscrol,5
DisplayGUIInputClick:
  ; SlideBar Implementation
  push eax
  mov eax,[NumInputDevices]
  mov [GUINumValue],eax
  pop eax
  GUISlidebarImpl 109,42,116,69,GUIIStA,5,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,3
.skipscrol
  ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIWinControl 5,36,107,34+5*8,GUIBlankVar,GUIcurrentinputviewloc,GUINumValue,35,8,GUIcurrentinputcursloc,4,3,0

  GUIPTabClick 0,21,1,GUIInputTabs
  GUIPTabClick 22,43,2,GUIInputTabs
  GUIPTabClick 44,65,3,GUIInputTabs
  GUIPTabClick 66,87,4,GUIInputTabs
  GUIPTabClick 88,109,5,GUIInputTabs

  GUIPHoldbutton 123,34,153,45,14            ; Buttons
  GUIPHoldbutton 123,50,177,61,40
%ifdef __MSDOS__
  GUIPHoldbutton 123,66,183,77,15
%endif

  cmp byte[cplayernum],0
  jne near .nopl1
  mov dword[keycontrolval],pl1contrl
  GUIInputSetIndKey pl1
.nopl1
  cmp byte[cplayernum],1
  jne near .nopl2
  mov dword[keycontrolval],pl2contrl
  GUIInputSetIndKey pl2
.nopl2
  cmp byte[cplayernum],2
  jne near .nopl3
  mov dword[keycontrolval],pl3contrl
  GUIInputSetIndKey pl3
.nopl3
  cmp byte[cplayernum],3
  jne near .nopl4
  mov dword[keycontrolval],pl4contrl
  GUIInputSetIndKey pl4
.nopl4
  cmp byte[cplayernum],4
  jne near .nopl5
  mov dword[keycontrolval],pl5contrl
  GUIInputSetIndKey pl5
.nopl5

%ifdef __MSDOS__
  cmp byte[cplayernum],0
  jne .notpl1
  GUIClickCButtonID 5,160,byte[pl1p209]
.notpl1
  cmp byte[cplayernum],1
  jne .notpl2
  GUIClickCButtonID 5,160,byte[pl2p209]
.notpl2
  cmp byte[cplayernum],2
  jne .notpl3
  GUIClickCButtonID 5,160,byte[pl3p209]
.notpl3
  cmp byte[cplayernum],3
  jne .notpl4
  GUIClickCButtonID 5,160,byte[pl4p209]
.notpl4
  cmp byte[cplayernum],4
  jne .notpl5
  GUIClickCButtonID 5,160,byte[pl5p209]
.notpl5

  GUIClickCButton 11,190,byte[SidewinderFix]
%endif

  GUIClickCButtonM 5,170,byte[pl12s34]
  GUIClickCButton 5,180,byte[AllowUDLR]
  GUIClickCButton 105,180,byte[Turbo30hz]
  ret

DisplayGUIOptionClick:
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUINGVID+ebx],1
  jne near .invalidng
  GUIClickCButton2 11,11,byte[newengen],byte[newengen],1
.invalidng
  GUIClickCButtonMMX 11,21,byte[MMXSupport]
%ifdef __MSDOS__
%else
  GUIClickCButton 11,41,byte[PauseFocusChange]
%endif
  GUIClickCButton 11,51,byte[FPSAtStart]

  GUIClickCButton 11,71,byte[TimerEnable]
  GUIClickCButton 11,81,byte[TwelveHourClock]
  GUIClickCButton 11,91,byte[ClockBox]
  GUIClickCButton 11,101,byte[bgfixer]
  GUIClickCButton 11,111,byte[SmallMsgText]
  GUIClickCButton 11,121,byte[AutoPatch]
  GUIClickCButton 11,131,byte[RomInfo]
  GUIClickCButton 11,141,byte[DisplayInfo]

%ifdef __WIN32__
  GUIClickCButton 11,161,byte[HighPriority]
  pushad
  call CheckPriority
  popad
  GUIClickCButton 11,171,byte[DisableScreenSaver]
  pushad
  call CheckScreenSaver
  popad
%endif
  ret

DisplayGUIVideoClick2:
  cmp dword[GUIVideoTabs],1  ; modes
  jne near .notmodestab2
  push eax
  mov eax,[NumVideoModes]
  mov [GUINumValue],eax
  pop eax
  GUISlidebarPostImpl 117,33,124,182,.scrollup,.scrolldown,5,20,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,GUIBlankVar,1
  GUIPostWinBoxMove 5,27,115,27+20*8,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,.scrollup,.scrolldown,DisplayGUIVideoClick.skipscrol,20
.notmodestab2
DisplayGUIVideoClick:
  GUIPTabClick 0,39,1,GUIVideoTabs,GUIVntscTab
  GUIPTabClick 40,91,2,GUIVideoTabs,GUIVntscTab
  cmp byte[NTSCFilter],1
  jne .nontscf
  GUIPTabClick 92,125,1,GUIVntscTab,GUIVideoTabs
.nontscf

  cmp dword[GUIVideoTabs],1  ; modes
  jne near .notmodestab
  ; SlideBar Implementation
  push eax
  mov eax,[NumVideoModes]
  mov [GUINumValue],eax
  pop eax
  GUISlidebarImpl 117,33,124,182,GUIVStA,20,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,5
.skipscrol
  GUIWinControl 5,27,115,27+20*8,GUIBlankVar,GUIcurrentvideoviewloc,GUINumValue,27,8,GUIcurrentvideocursloc,2,5,0

  GUIPHoldbutton 130,31,166,41,4

%ifndef __MSDOS__
  GUIPHoldbutton 182,116,218,126,12

  GUITextBoxInputNach 130,130,178,140,12,5
  GUITextBoxInputNach 191,130,239,140,13,5
%endif

.notmodestab

  cmp dword[GUIVideoTabs],2  ; filters tab
  jne near .notfilterstab
  pushad
  call Clear2xSaIBuffer
  popad
  xor ebx,ebx                             ; scanlines
  mov bl,[cvidmode]
  cmp byte[GUISLVID+ebx],0
  je near .nocheckboxb
  FilterExcl 21,34,En2xSaI,hqFilter,NTSCFilter
  GUIClickCButton4 21,34,byte[scanlines],byte[scanlines],1
.nocheckboxb
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIHSVID+ebx],0
  je near .nohsline
  FilterExcl 61,34,En2xSaI,hqFilter,NTSCFilter
  FilterExcl 101,34,En2xSaI,hqFilter,NTSCFilter
  GUIClickCButton4 61,34,byte[scanlines],byte[scanlines],3
  GUIClickCButton4 101,34,byte[scanlines],byte[scanlines],2
.nohsline

  xor ebx,ebx                              ; 2xsai and supers
  mov bl,[cvidmode]
  cmp byte[GUI2xVID+ebx],0
  je near .no2xSaI
  FilterExcl 11,63,hqFilter,scanlines,antienab,NTSCFilter
  FilterExcl 121,63,hqFilter,scanlines,antienab,NTSCFilter
  FilterExcl 11,73,hqFilter,scanlines,antienab,NTSCFilter
  GUIClickCButton6 11,63,byte[En2xSaI],1
  GUIClickCButton6 121,63,byte[En2xSaI],2
  GUIClickCButton6 11,73,byte[En2xSaI],3
.no2xSaI

  xor ebx,ebx                             ; interpolations
  mov bl,[cvidmode]
  cmp byte[GUIIEVID+ebx],0
  je near .nocheckbox1b
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[NTSCFilter],0
  FilterExcl 121,73,hqFilter,NTSCFilter,En2xSaI
  GUIClickCButton3 121,73,byte[antienab],byte[antienab],1
.nocheckbox1b
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUII2VID+ebx],0
  je near .nocheckboxi2
  FilterExcl 121,73,hqFilter,NTSCFilter,En2xSaI
  GUIClickCButton3 121,73,byte[antienab],byte[antienab],1
.nocheckboxi2

%ifdef __UNIXSDL__
  xor ebx,ebx                             ; bilinear
  mov bl,[cvidmode]
  cmp byte[GUIBIFIL+ebx],0
  je .nocheckbox1bf
  FilterExcl 121,73,NTSCFilter
  GUIClickCButton 121,73,byte[BilinearFilter]
.nocheckbox1bf
%endif

  xor ebx,ebx                             ; hq filters
  mov bl,[cvidmode]
  cmp byte[GUIHQ2X+ebx],0
  jne .checkboxhq
  cmp byte[GUIHQ3X+ebx],0
  jne .checkboxhq
  cmp byte[GUIHQ4X+ebx],0
  jne .checkboxhq
  jmp .nocheckboxhq
.checkboxhq
  FilterExcl 11,83,En2xSaI,scanlines,antienab,NTSCFilter
  GUIClickCButton 11,83,byte[hqFilter]
.nocheckboxhq

  xor ebx,ebx                             ; ntsc effect
  mov bl,[cvidmode]
  cmp byte[GUINTVID+ebx],0
  je near .nontscfilter
  FilterExcl 121,83,En2xSaI,hqFilter,scanlines,antienab,BilinearFilter
  GUIClickCButtonN 121,83,byte[NTSCFilter],NTSCFilterInit
.nontscfilter

  xor ebx,ebx                             ; hires m7
  mov bl,[cvidmode]
  cmp byte[GUIM7VID+ebx],0
  je .nocheckboxtm7
  GUIClickCButton5 11,113,byte[Mode7HiRes16b],1
.nocheckboxtm7

  GUIClickCButton 121,113,byte[GrayscaleMode] ; Grayscale

%ifdef __MSDOS__
  xor ebx,ebx                            ; triple buffs/vsyncs
  mov bl,[cvidmode]
  cmp byte[GUITBVID+ebx],0
  je .nocheckboxttb
  GUIClickCButtonT 121,123,byte[Triplebufen],byte[vsyncon]
.nocheckboxttb
  GUIClickCButtonT 11,123,byte[vsyncon],byte[Triplebufen]
%else
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUITBWVID+ebx],0
  je near .notriplebufferwin
  GUIClickCButtonf 121,123,byte[TripleBufferWin],initDirectDraw
.notriplebufferwin
  GUIClickCButtonf 11,123,byte[vsyncon],initDirectDraw
%endif

%ifdef __MSDOS__
  xor ebx,ebx                            ; small/fullscreen
  mov bl,[cvidmode]
  cmp byte[GUISSVID+ebx],0
  je .nocheckbox2b
  FilterExcl 11,133,ScreenScale
  GUIClickCButton3 11,133,byte[smallscreenon],byte[smallscreenon],1
.nocheckbox2b
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIFSVID+ebx],0
  je .nocheckbox3b
  FilterExcl 121,133,smallscreenon
  GUIClickCButton3 121,133,byte[ScreenScale],byte[ScreenScale],1
.nocheckbox3b
%endif

%ifndef __MSDOS__
  xor ebx,ebx                             ; Keep 4:3 Ratio
  mov bl,[cvidmode]
%ifdef __WIN32__
  cmp byte[GUIDSMODE+ebx],0
  jne near .checkboxk43
  cmp byte[GUISMODE+ebx],0
  jne near .checkboxk43
  jmp .nocheckboxk43
%else
  cmp bl,21
  jl .nocheckboxk43
%endif
.checkboxk43
  GUIClickCButtonK 11,143,byte[Keep4_3Ratio],initwinvideo
.nocheckboxk43
%endif

.notfilterstab

  cmp dword[GUIVntscTab],1  ; NTSC Tab
  jne near .notntsctab
  mov eax,[GUImouseposx]    ; update mouse location
  sub eax,[GUIwinposx+5*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+5*4]

  GUIClickCButton 5,25,byte[NTSCBlend]     ; Checkboxes
  GUIClickCButton 135,25,byte[NTSCRef]

  GUIPHoldbutton 5,166,42,177,37  ; button

  ;Sliders
  cmp eax,8                      ; X-Range for click-area
  jl near .nomovebar
  cmp eax,8+200
  jg near .nomovebar
  sub al,108
  cmp edx,54                     ; Y-Range for click-area
  jl near .nomovebar
  cmp edx,58
  jg .nohuebar
  mov [NTSCHue],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],0
  mov byte[GUIHold],7            ; Lock mouse to bar when clicked
  mov eax,[GUIwinposy+5*4]
  add eax,56
  mov [GUIHoldYlim],eax
  jmp .nowarpbar
.nohuebar
  cmp edx,74
  jl near .nomovebar
  cmp edx,78
  jg .nosatbar
  mov [NTSCSat],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],1
  mov byte[GUIHold],7
  mov eax,[GUIwinposy+5*4]
  add eax,76
  mov [GUIHoldYlim],eax
  jmp .nowarpbar
.nosatbar
  cmp edx,94
  jl near .nomovebar
  cmp edx,98
  jg .nocontbar
  mov [NTSCCont],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],2
  mov byte[GUIHold],7
  mov eax,[GUIwinposy+5*4]
  add eax,96
  mov [GUIHoldYlim],eax
  jmp .nowarpbar
.nocontbar
  cmp edx,114
  jl near .nomovebar
  cmp edx,118
  jg .nobrightbar
  mov [NTSCBright],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],3
  mov byte[GUIHold],7
  mov eax,[GUIwinposy+5*4]
  add eax,116
  mov [GUIHoldYlim],eax
  jmp .nowarpbar
.nobrightbar
  cmp edx,134
  jl near .nomovebar
  cmp edx,138
  jg .nosharpbar
  mov [NTSCSharp],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],4
  mov byte[GUIHold],7
  mov eax,[GUIwinposy+5*4]
  add eax,136
  mov [GUIHoldYlim],eax
  jmp .nowarpbar
.nosharpbar
  cmp edx,154
  jl .nomovebar
  cmp edx,158
  jg .nomovebar
  mov [NTSCWarp],al
  mov [ntscCurVar],al
  mov byte[ntscWhVar],5
  mov byte[GUIHold],7
  mov eax,[GUIwinposy+5*4]
  add eax,156
  mov [GUIHoldYlim],eax
.nowarpbar
  mov eax,[GUIwinposx+5*4]
  add eax,8
  mov [GUIHoldXlimL],eax
  add eax,200
  mov [GUIHoldXlimR],eax
.nomovebar
  cmp byte[NTSCRef],0
  je .notntsctab
  pushad
  call NTSCFilterInit
  popad
.notntsctab
  xor eax,eax
  ret

SECTION .bss
GUINumValue resd 1
GUIBlankVar resd 1
ntscLastVar resb 6
ntscCurVar resb 1
ntscWhVar resb 1
SECTION .text

DisplayGUISoundClick:
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+6*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+6*4]
  GUIClickCButtonf 11,21,byte[soundon],reInitSound
%ifdef __MSDOS__
  GUIClickCButton 102,21,byte[Force8b]
%endif
  GUIClickCButton 11,30,byte[StereoSound]
  GUIClickCButton 11,111,byte[RevStereo]
  GUIClickCButton 11,121,byte[Surround]
  GUIClickCButton 11,131,byte[SoundBufEn]
  GUIClickCButton 11,141,byte[SPCDisable]
  GUIClickCButton5 11,163,byte[SoundInterpType],1
  GUIClickCButton5 11,173,byte[SoundInterpType],2
  GUIClickCButton5MMXC 11,183,byte[SoundInterpType],3
  GUIClickCButton5 111,163,byte[LowPassFilterType],1
  GUIClickCButton5 111,173,byte[LowPassFilterType],2
  GUIClickCButton5MMXC 111,183,byte[LowPassFilterType],3
%ifdef __WIN32__
  GUIClickCButton 11,101,byte[PrimaryBuffer]
%endif
  GUIClickArea 15,61,69,69,.nosrate
  mov eax,[SoundQuality]
  mov al,[sampratenext+eax]
  mov [SoundQuality],al
.nosrate
  GUIClickArea 15,89,115,93,{near .nomovebar}
  sub eax,15
  mov [MusicRelVol],al
  mov byte[GUIHold],5
  mov eax,[GUIwinposy+6*4]
  add eax,91
  mov [GUIHoldYlim],eax
  xor eax,eax
  xor edx,edx
  mov al,[MusicRelVol]
  shl eax,7
  mov ebx,100
  div ebx
  cmp al,127
  jb .noof
  mov al,127
.noof
  mov [MusicVol],al
  mov al,[DSPMem+0Ch]
  call WDSPReg0C
  mov al,[DSPMem+1Ch]
  call WDSPReg1C

  mov eax,[GUIwinposx+6*4]
  add eax,15
  mov [GUIHoldXlimL],eax
  add eax,100
  mov [GUIHoldXlimR],eax
.nomovebar
  ret

SECTION .data
sampratenext db 1,4,5,6,2,3,0,0
SECTION .text

DisplayGUICheatClick2:
  GUISlidebarPostImpl 231,28,238,100,.scrollup,.scrolldown,7,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,GUIBlankVar,1
  GUIPostWinBoxMove 5,22,229,22+12*7,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,.scrollup,.scrolldown,DisplayGUICheatClick.skipscrol,12

DisplayGUICheatClick:
  GUIBoxVar 5,20,181,110,GUIcurrentcheatwin,0      ; Main Box
  GUIBoxVar 82,129,173,136,GUIcurrentcheatwin,1    ; Enter Code Box
  GUIBoxVar 82,140,196,147,GUIcurrentcheatwin,2    ; Description Box
  ; SlideBar Implementation
  GUISlidebarImpl 231,28,238,100,GUICStA,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,7
.skipscrol
  cmp byte[GUIcurrentcheatwin],0
  jne near .nowin
  GUIWinControl 5,23,229,21+12*7,GUIBlankVar,GUIcurrentcheatviewloc,NumCheats,22,7,GUIcurrentcheatcursloc,3,7,30
.nowin
  GUIPHoldbutton 5,113,47,124,5                  ; Buttons
  GUIPHoldbutton 52,113,94,124,6
  GUIPHoldbutton 99,113,141,124,7
  GUIPHoldbutton 146,113,188,124,8
  GUIPHoldbutton 212,134,236,145,9
  GUIPHoldbutton 193,113,235,124,33
  GUIClickCButton5 11,186,byte[AutoLoadCht],1    ; Checkbox
  ret

DisplayNetOptnsClick:
  ret

guipresstest:
  xor ebx,ebx
  mov ecx,256+128+64
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  GUIBox 75,95,180,131,160
  GUIBox 75,95,180,95,162
  GUIBox 75,95,75,131,161
  GUIBox 180,95,180,131,159
  GUIBox 75,131,180,131,158
  GUIOuttext 81,101,guipresstext1,220-15
  GUIOuttext 80,100,guipresstext1,220
  GUIOuttext 81,111,guipresstext2,220-15
  GUIOuttext 80,110,guipresstext2,220
  GUIOuttext 81,121,guipresstext3,220-15
  GUIOuttext 80,120,guipresstext3,220
  call vidpastecopyscr
.again
  call JoyRead
;    mov dword[pressed+0CCh],0
;    mov dword[pressed+0CCh+80h],0
;    mov dword[pressed+0E8h],0
;    mov dword[pressed+0E8h+80h],0
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
  push ebx
  xor ebx,ebx
  mov ecx,256+128+64
.a2
  cmp byte[pressed+ebx],0
  je .pressedokayb
  mov byte[pressed+ebx],2
.pressedokayb
  inc ebx
  dec ecx
  jnz .a2
  pop ebx
.again2
  call Check_Key
  or al,al
  jz .nokey
  call Get_Key
  jmp .again2
.nokey
;    push ebx
;    mov eax,0Bh
;    int 33h
;    pop ebx
  ret

SECTION .data
guipresstext1 db 'ENTER THE KEY',0
guipresstext2 db 'OR BUTTON TO USE',0
guipresstext3 db '(ESC TO CLEAR)',0
SECTION .text

guipresstestb:
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  GUIBox 65,80,194,126,160
  GUIBox 65,80,194,80,162
  GUIBox 65,80,65,126,161
  GUIBox 194,80,194,126,159
  GUIBox 65,126,194,126,158
  GUIOuttext 71,86,guipresstext1,220-15
  GUIOuttext 70,85,guipresstext1,220
  GUIOuttext 71,96,guipresstext2,220-15
  GUIOuttext 70,95,guipresstext2,220
  GUIOuttext 71,116,guipresstext3b,220-15
  GUIOuttext 70,115,guipresstext3b,220
  mov eax,[guipressptr]
  GUIOuttext 71,106,eax,220-15
  mov eax,[guipressptr]
  GUIOuttext 70,105,eax,220
  call vidpastecopyscr
  mov ecx,8192
  call delay
.tryagain
  call JoyRead
  xor ebx,ebx
  mov ecx,256+128+64
.b2
  cmp byte[pressed+ebx],0
  jne .tryagain
  inc ebx
  dec ecx
  jnz .b2

.again
  call JoyRead
;    mov dword[pressed+0CCh],0
;    mov dword[pressed+0CCh+80h],0
;    mov dword[pressed+0E8h],0
;    mov dword[pressed+0E8h+80h],0

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
  call Check_Key
  or al,al
  jz .nokey
  call Get_Key
  jmp .again2
.nokey
  cmp ebx,1
  je .skip
  cmp ebx,3Bh
  je .skip
  mov eax,[guicpressptr]
  mov [eax],ebx
.skip
;    push ebx
;    mov eax,0Bh
;    int 33h
;    pop ebx
  ret

SECTION .data
guipresstext3b db '(ESC TO SKIP)',0
guipresstext4b db 'FOR UP              ',0
  db 'FOR DOWN            ',0
  db 'FOR LEFT            ',0
  db 'FOR RIGHT           ',0
  db 'FOR START           ',0
  db 'FOR SELECT          ',0
  db 'FOR A (RIGHT BUTTON)',0
  db 'FOR B (DOWN BUTTON) ',0
  db 'FOR X (TOP BUTTON)  ',0
  db 'FOR Y (LEFT BUTTON) ',0
  db 'FOR THE L BUTTON    ',0
  db 'FOR THE R BUTTON    ',0
SECTION .bss
guipressptr resd 1
guicpressptr resd 1
SECTION .text

DisplayGameOptnsClick:
  mov dword[keycontrolval],0                  ; Shortcut Boxes
  DGOptnsProcBox 27,23,[KeyBGDisble0]
  DGOptnsProcBox 27+45,23,[KeyBGDisble1]
  DGOptnsProcBox 27+45*2,23,[KeyBGDisble2]
  DGOptnsProcBox 27+45*3,23,[KeyBGDisble3]
  DGOptnsProcBox 27+45*4,23,[KeySprDisble]
  DGOptnsProcBox 27,41,[KeyDisableSC0]
  DGOptnsProcBox 27+45,41,[KeyDisableSC1]
  DGOptnsProcBox 27+45*2,41,[KeyDisableSC2]
  DGOptnsProcBox 27+45*3,41,[KeyDisableSC3]
  DGOptnsProcBox 34+45*4,41,[KeyVolUp]
  DGOptnsProcBox 27,50,[KeyDisableSC4]
  DGOptnsProcBox 27+45,50,[KeyDisableSC5]
  DGOptnsProcBox 27+45*2,50,[KeyDisableSC6]
  DGOptnsProcBox 27+45*3,50,[KeyDisableSC7]
  DGOptnsProcBox 34+45*4,50,[KeyVolDown]

  DGOptnsProcBox 8+25,70,[KeyQuickLoad]
  DGOptnsProcBox 8+57+26,70,[KeyQuickRst]
  DGOptnsProcBox 8+114+20,70,[KeyQuickExit]
  DGOptnsProcBox 8+114+78,70,[KeyQuickClock]
  DGOptnsProcBox 8+25,80,[KeyQuickChat]
  DGOptnsProcBox 8+57+45,80,[KeyQuickSnapShot]
  DGOptnsProcBox 8+108+70,80,[KeyQuickSaveSPC]

  DGOptnsProcBox 78,100,[KeyUsePlayer1234]
  DGOptnsProcBox 78,110,[KeyResetAll]
  DGOptnsProcBox 78,120,[KeyDisplayFPS]
%ifndef __MSDOS__
  DGOptnsProcBox 78,130,[KeyDisplayBatt]
%endif

  DGOptnsProcBox 191,100,[KeyNewGfxSwt]
  DGOptnsProcBox 191,110,[KeyWinDisble]
  DGOptnsProcBox 191,120,[KeyOffsetMSw]
  DGOptnsProcBox 191,130,[KeyIncreaseGamma]
  DGOptnsProcBox 191,140,[KeyDecreaseGamma]
  ret

DisplayGUIOptnsClick:
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+10*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+10*4]
  GUIClickCButton 124,13,byte[newfont]
  GUIClickCButton 12,23,byte[mousewrap]     ; Checkboxes
  GUIClickCButton 12,33,byte[mouseshad]
  GUIClickCButton 12,43,byte[esctomenu]
  GUIClickCButton 12,53,byte[resetposn]
  GUIClickCButton 12,63,byte[GUIClick]
  GUIClickCButton 12,73,byte[JoyPad1Move]
  GUIClickCButton 124,23,byte[GUIEnableTransp]
  GUIClickCButtonI 124,33,byte[FilteredGUI]
  GUIClickCButton6 124,43,byte[GUIEffect],1
  GUIClickCButton6 124,53,byte[GUIEffect],2
  GUIClickCButton6 124,63,byte[GUIEffect],3
  GUIClickCButton6 124,73,byte[GUIEffect],4
%ifdef __WIN32__
  GUIClickCButton 12,83,byte[MouseWheel]
  GUIClickCButton 124,83,byte[TrapMouseCursor]
  GUIClickCButton 12,168,byte[AlwaysOnTop]
  GUIClickCButton 12,178,byte[SaveMainWindowPos]
  GUIClickCButton 12,188,byte[AllowMultipleInst]
%endif

  GUIClickCButton 12,93,byte[lhguimouse]

  GUIPButtonHole 48,108,byte[CurPalSelect],0    ; Radio Buttons
  GUIPButtonHole 88,108,byte[CurPalSelect],1
  GUIPButtonHole 135,108,byte[CurPalSelect],2

  push eax                          ; Sliders
  cmp byte[CurPalSelect],1
  je .yespal2b
  cmp byte[CurPalSelect],2
  je .yespal3b
  mov al,[GUIRAdd]
  mov [TRVal2],al
  mov al,[GUIGAdd]
  mov [TGVal2],al
  mov al,[GUIBAdd]
  mov [TBVal2],al
  jmp .nopal1b
.yespal2b
  mov al,[GUITRAdd]
  mov [TRVal2],al
  mov al,[GUITGAdd]
  mov [TGVal2],al
  mov al,[GUITBAdd]
  mov [TBVal2],al
  jmp .nopal1b
.yespal3b
  mov al,[GUIWRAdd]
  mov [TRVal2],al
  mov al,[GUIWGAdd]
  mov [TGVal2],al
  mov al,[GUIWBAdd]
  mov [TBVal2],al
.nopal1b
  pop eax

  cmp eax,25
  jl near .nomovebar
  cmp eax,25+127
  jg near .nomovebar
  sub eax,25
  shr eax,2
  cmp edx,122
  jl .nomovebar1
  cmp edx,126
  jg .nomovebar1
  cmp byte[TRVal2],al
  je .nomovebar1b
  mov [TRVal2],al
  mov byte[TRVal2+1],1
.nomovebar1b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,124
  mov [GUIHoldYlim],eax
  jmp .nomovebar
.nomovebar1
  cmp edx,134
  jl .nomovebar2
  cmp edx,138
  jg .nomovebar2
  cmp byte[TGVal2],al
  je .nomovebar2b
  mov [TGVal2],al
  mov byte[TRVal2+1],1
.nomovebar2b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,136
  mov [GUIHoldYlim],eax
  jmp .nomovebar
.nomovebar2
  cmp edx,146
  jl .nomovebar3
  cmp edx,150
  jg .nomovebar3
  cmp byte[TBVal2],al
  je .nomovebar3b
  mov [TBVal2],al
  mov byte[TRVal2+1],1
.nomovebar3b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,148
  mov [GUIHoldYlim],eax
.nomovebar3
.nomovebar

  push eax
  cmp byte[CurPalSelect],1
  je .yespal2
  cmp byte[CurPalSelect],2
  je .yespal3
  mov al,[TRVal2]
  mov [GUIRAdd],al
  mov al,[TGVal2]
  mov [GUIGAdd],al
  mov al,[TBVal2]
  mov [GUIBAdd],al
  jmp .nopal1
.yespal2
  mov al,[TRVal2]
  mov [GUITRAdd],al
  mov al,[TGVal2]
  mov [GUITGAdd],al
  mov al,[TBVal2]
  mov [GUITBAdd],al
  jmp .nopal1
.yespal3
  mov al,[TRVal2]
  mov [GUIWRAdd],al
  mov al,[TGVal2]
  mov [GUIWGAdd],al
  mov al,[TBVal2]
  mov [GUIWBAdd],al
.nopal1
  pop eax

  cmp byte[TRVal2+1],1
  jne .nopalchange
  call GUISetPal
  mov byte[TRVal2+1],0
.nopalchange

  cmp byte[GUIHold],2
  jne .nohold
  mov eax,[GUIwinposx+10*4]
  add eax,25
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+10*4]
  add eax,25+127
  mov [GUIHoldXlimR],eax
.nohold
  ret

DisplayGUIAboutClick:
%ifndef __MSDOS__
  cmp byte[EEgg],1
  je near .eegg
  GUIPHoldbutton 90,22,175,32,65
  GUIPHoldbutton 90,33,175,43,66
.eegg
%endif
  ret

; DrawSlideBar 16,192,20,[GUIccombviewloc],[NumCombo],8,61,GUICSStC,13,14
;    DrawBorderedBox 16,10,20,190,80,19

;    DrawGUIWinBox 3,5,34,107,77,167
;    DrawSlideBarWin 3,109,42,[GUIcurrentinputviewloc],16,5,28,GUIIStA

DisplayGUIComboClick2:
  mov dword[keycontrolval],0
  ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  GUISlidebarPostImpl 192,28,199,72,.scrollup,.scrolldown,13,8,GUIccombviewloc,GUIccombcursloc,NumCombo,GUIBlankVar,1
  ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  GUIPostWinBoxMove 10,22,190,23+8*7,GUIccombviewloc,GUIccombcursloc,NumCombo,.scrollup,.scrolldown,DisplayGUIComboClick.skipscrol,8
DisplayGUIComboClick:
  ; SlideBar Implementation
  ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  GUISlidebarImpl 192,28,199,72,GUICSStC,8,GUIccombviewloc,GUIccombviewloc,NumCombo,16
.skipscrol
  ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIWinControl2 10,23,190,22+8*7,GUIBlankVar,GUIccombviewloc,NumCombo,23,7,GUIccombcursloc,6,16,0

  GUIPButtonHole 158,82,byte[GUIComboPNum],0
  GUIPButtonHole 176,82,byte[GUIComboPNum],1
  GUIPButtonHole 194,82,byte[GUIComboPNum],2
  GUIPButtonHole 212,82,byte[GUIComboPNum],3
  GUIPButtonHole 230,82,byte[GUIComboPNum],4
  GUIClickCButton 163,88,byte[GUIComboLHorz]
  GUIClickCButtonC 163,96,byte[GUIComboGameSpec]
  GUIPHoldbutton 202,20,246,31,60
  GUIPHoldbutton 202,35,246,46,61
  GUIPHoldbutton 202,50,246,61,62
  GUIPHoldbutton 202,65,246,76,63
  DGOptnsProcBoxc 10,91,32,99,[GUIComboKey]

  ComboBoxProc 75,150,85,157,1
  ComboBoxProc 89,150,99,157,2
  ComboBoxProc 103,150,113,157,3
  ComboBoxProc 117,150,127,157,4
  ComboBoxProc 131,150,141,157,5
  ComboBoxProc 145,150,155,157,6
  ComboBoxProc 159,150,169,157,7
  ComboBoxProc 173,150,183,157,8
  ComboBoxProc 187,150,197,157,9
  ComboBoxProc 201,150,211,157,10
  ComboBoxProc 215,150,227,157,11
  ComboBoxProc 231,150,243,157,12
  ComboBoxProc 75,160,85,167,13
  ComboBoxProc 89,160,99,167,14
  ComboBoxProc 103,160,113,167,15
  ComboBoxProc 117,160,127,167,16
  ComboBoxProc 131,160,141,167,17
  ComboBoxProc 145,160,155,167,18
  ComboBoxProc 159,160,169,167,19
  ComboBoxProc 173,160,183,167,20
  ComboBoxProc 187,160,197,167,21
  ComboBoxProc 201,160,211,167,22
  ComboBoxProc 215,160,227,167,23
  ComboBoxProc 231,160,243,167,24
  ComboBoxProc 75,170,85,177,25
  ComboBoxProc 89,170,99,177,26
  ComboBoxProc 103,170,113,177,27
  ComboBoxProc 117,170,127,177,28
  ComboBoxProc 131,170,141,177,29
  ComboBoxProc 145,170,155,177,30
  ComboBoxProc 159,170,169,177,31
  ComboBoxProc 173,170,183,177,32
  ComboBoxProc 187,170,197,177,33
  ComboBoxProc 201,170,211,177,34
  ComboBoxProc 215,170,227,177,35
  ComboBoxProc 231,170,243,177,36

  ComboBoxProc 10,189,20,196,37
  ComboBoxProc 24,189,34,196,38
  ComboBoxProc 38,189,48,196,39
  ComboBoxProc 52,189,62,196,40
  ComboBoxProc 66,189,76,196,41
  ComboBoxProc 80,189,90,196,42
  ComboBoxProc 107,189,117,196,43
  ComboBoxProc 121,189,131,196,44
  ComboBoxProc 135,189,145,196,45
  ComboBoxProc 149,189,159,196,46
  ComboBoxProc 163,189,173,196,47
  ComboBoxProc 177,189,187,196,48
  ComboBoxProcD 204,189,218,196
  mov ebx,[NumComboLocl]
  cmp byte[GUIComboGameSpec],0
  jne .local
  mov ebx,[NumComboGlob]
.local
  mov [NumCombo],ebx
  xor ebx,ebx
  ret

DisplayGUIResetClick:
  GUIPHoldbutton 20,30,56,41,2
  GUIPHoldbutton 80,30,116,41,3
  ret

DisplayGUIStatesClick:
  GUIPHoldbutton 20,30,56,41,10
  GUIPHoldbutton 80,30,116,41,11
  ret

DisplayGUICheatSearchClick2:
  cmp byte[CheatWinMode],2
  je near DisplayGUICheatSearchClick.preview
DisplayGUICheatSearchClick:
  cmp byte[CheatWinMode],1
  je near .incheatmode
  cmp byte[CheatWinMode],2
  je near .view
  cmp byte[CheatWinMode],3
  je near .add

  GUIPHoldbutton 95,140,140,152,50         ; Main Menu
  GUIPButtonHole 11,28,byte[CheatSrcByteSize],0
  GUIPButtonHole 11,38,byte[CheatSrcByteSize],1
  GUIPButtonHole 11,48,byte[CheatSrcByteSize],2
  GUIPButtonHole 11,58,byte[CheatSrcByteSize],3
  GUIPButtonHole 11,73,byte[CheatSrcByteBase],0
  GUIPButtonHole 11,83,byte[CheatSrcByteBase],1
  GUIPButtonHole 11,113,byte[CheatSrcSearchType],0
  GUIPButtonHole 11,123,byte[CheatSrcSearchType],1
  ret

.incheatmode                            ; Exact Value
  GUIPHoldbutton 30,140,80,152,51
  cmp byte[CheatSearchStatus],1
  jne near .cheatsearching
  jmp .nosearch

.cheatsearching                         ; Return and Re-Search
  cmp byte[CheatSrcSearchType],1
  je near .cheatsearchingcomp
  jmp .searched

.cheatsearchingcomp                     ; Select Comparison
  GUIPButtonHole 11,33,byte[CheatCompareValue],0
  GUIPButtonHole 11,43,byte[CheatCompareValue],1
  GUIPButtonHole 11,53,byte[CheatCompareValue],2
  GUIPButtonHole 11,63,byte[CheatCompareValue],3

.searched                               ; Restart/View/Search Buttons
  GUIPHoldbutton 120,140,170,152,53
.nosearch
  GUIPHoldbutton 10,140,60,152,51
  GUIPHoldbutton 70,140,110,152,52
  ret

; View Cheat
.preview                     ; Preview Box
  ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  GUISlidebarPostImpl 173,28,180,100,.scrollup,.scrolldown,11,12,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,GUIBlankVar,1
  ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  GUIPostWinBoxMove 5,22,171,22+12*7,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,.scrollup,.scrolldown,DisplayGUICheatSearchClick.skipscrol,12
.view
  ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  GUISlidebarImpl 173,28,180,100,GUICSStA,12,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,13
.skipscrol
  ; x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIWinControl 5,23,171,21+12*7,GUIBlankVar,GUIcurrentchtsrcviewloc,NumCheatSrc,22,7,GUIcurrentchtsrccursloc,5,13,30

  GUIPHoldbutton 70,140,130,152,54         ; Return/Add Buttons
  GUIPHoldbutton 140,140,180,152,55
  ret

;    DrawSlideBarWin 13,173,28,[GUIcurrentchtsrcviewloc],[NumCheatSrc],12,73,GUICSStA
;    DrawGUIWinBox 13,5,20,171,108,167

.add                         ; Add Cheat
  GUIPHoldbutton 60,155,120,167,56                 ; Return/Add Buttons
  GUIPHoldbutton 130,155,160,167,57
  GUIClickCButton 8,139,byte[CheatUpperByteOnly]   ; Checkbox
  GUIBoxVar 10,30,80,37,CurCStextpos,0             ; Input Boxes
  GUIBoxVar 10,55,126,62,CurCStextpos,1
  ret

DisplayGUIAddOnClick:
  GUIPButtonHole 9,23,byte[device1],0
  GUIPButtonHole 9,33,byte[device1],1

  GUIPButtonHole 100,23,byte[device2],0
  GUIPButtonHole 100,33,byte[device2],1
  GUIPButtonHole 100,43,byte[device2],2
  GUIPButtonHole 100,53,byte[device2],3
  GUIPButtonHole 100,63,byte[device2],4

  DGOptnsProcBox 74,76,[KeyExtraEnab1]
  DGOptnsProcBox 165,76,[KeyExtraEnab2]

  GUIClickCButton 9,83,byte[mouse1lh]
  GUIClickCButton 100,83,byte[mouse2lh]

  DGOptnsProcBox 74,108,[SSAutoFire]
  DGOptnsProcBox 165,108,[SSPause]
  ret

DisplayGUIChipClick:
  GUIClickCButton 9,43,byte[nssdip1]
  GUIClickCButton 59,43,byte[nssdip2]
  GUIClickCButton 109,43,byte[nssdip3]
  GUIClickCButton 9,53,byte[nssdip4]
  GUIClickCButton 59,53,byte[nssdip5]
  GUIClickCButton 109,53,byte[nssdip6]
  ret

%ifdef __MSDOS__
%define path_length 256
%else
%define path_length 1024
%endif

DisplayGUIPathsClick:
  GUITextBoxInputNach 75,15,230,25,1,path_length       ;SRAMPath
  GUITextBoxInputNach 75,35,230,45,2,path_length       ;SnapPath
  GUITextBoxInputNach 75,55,230,65,3,path_length       ;SPCPath
  GUITextBoxInputNach 8,95,100,105,4,path_length       ;BSXPath
  GUITextBoxInputNach 8,120,100,130,5,path_length      ;SGPath
  GUITextBoxInputNach 135,95,235,105,6,path_length     ;STPath
  GUITextBoxInputNach 135,120,235,130,7,path_length    ;GNextPath
  GUITextBoxInputNach 8,155,100,165,8,path_length      ;FEOEZPath
  GUITextBoxInputNach 135,155,235,165,9,path_length    ;SJNSPath
  GUITextBoxInputNach 8,180,100,190,10,path_length     ;MDHPath
  GUITextBoxInputNach 135,180,235,190,11,path_length   ;SPL4Path
  ret

DisplayGUISaveClick:
  GUIClickCButtonAS 11,40,byte[AutoIncSaveSlot]
  GUIClickCButton 11,50,byte[SRAMSave5Sec]
  GUIClickCButton 11,60,byte[LatestSave]
  GUIClickCButton 11,70,byte[AutoState]
  GUIClickCButton 11,80,byte[SRAMState]
  GUIClickCButton 11,90,byte[PauseLoad]
  GUIClickCButton 11,100,byte[PauseRewind]
  GUIPHoldbutton2 173,17,181,24,70,byte[RewindStates],1,99
  GUIPHoldbutton2 184,17,192,24,71,byte[RewindStates],-1,0
  GUIPHoldbutton2 173,29,181,36,72,byte[RewindFrames],1,99
  GUIPHoldbutton2 184,29,192,36,73,byte[RewindFrames],-1,1
  DGOptnsProcBox 27,121,[KeyStateSlc0]
  DGOptnsProcBox 27+45,121,[KeyStateSlc1]
  DGOptnsProcBox 27+45*2,121,[KeyStateSlc2]
  DGOptnsProcBox 27+45*3,121,[KeyStateSlc3]
  DGOptnsProcBox 27,130,[KeyStateSlc4]
  DGOptnsProcBox 27+45,130,[KeyStateSlc5]
  DGOptnsProcBox 27+45*2,130,[KeyStateSlc6]
  DGOptnsProcBox 27+45*3,130,[KeyStateSlc7]
  DGOptnsProcBox 27,139,[KeyStateSlc8]
  DGOptnsProcBox 27+45,139,[KeyStateSlc9]
  DGOptnsProcBox 27+45*2,139,[KeyIncStateSlot]
  DGOptnsProcBox 27+45*3,139,[KeyDecStateSlot]
  DGOptnsProcBox 8+25,148,[KeySaveState]
  DGOptnsProcBox 8+57+25,148,[KeyLoadState]
  DGOptnsProcBox 8+114+25,148,[KeyStateSelct]
  DGOptnsProcBox 8+38,157,[KeyRewind]
  ret

DisplayGUISpeedClick:
  mov eax,[GUImouseposx]    ; this updates the mouse location
  sub eax,[GUIwinposx+21*4] ; effectively 'grabbing' the cursor !
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+21*4]

  GUIClickCButton 11,135,byte[FastFwdToggle]     ; Checkboxes
  GUIClickCButtonfr 11,145,byte[frameskip]

  cmp byte[frameskip],0
  jne near .frate
  DGOptnsProcBox 12,78,[KeyEmuSpeedUp]
  DGOptnsProcBox 12,98,[KeyEmuSpeedDown]
  jmp near .endsc
.frate
  DGOptnsProcBox 12,78,[KeyFRateUp]
  DGOptnsProcBox 12,98,[KeyFRateDown]
.endsc

  DGOptnsProcBox 12,58,[KeyFastFrwrd]        ; Shortcut Boxes
  DGOptnsProcBox 12,68,[KeySlowDown]
  DGOptnsProcBox 12,88,[KeyResetSpeed]
  DGOptnsProcBox 12,108,[EMUPauseKey]
  DGOptnsProcBox 12,118,[INCRFrameKey]

  GUIPHoldbutton2 118,24,126,32,74,byte[FFRatio],1,28
  GUIPHoldbutton2 129,24,137,32,75,byte[FFRatio],-1,0
  GUIPHoldbutton2 118,35,126,43,76,byte[SDRatio],1,28
  GUIPHoldbutton2 129,35,137,43,77,byte[SDRatio],-1,0
  cmp byte[frameskip],0
  je near .autopress
  GUIPHoldbutton2 118,13,126,21,78,byte[frameskip],1,10
  GUIPHoldbutton2 129,13,137,21,79,byte[frameskip],-1,1
  ret

.autopress
  GUIPHoldbutton2 118,13,126,21,78,byte[maxskip],1,9
  GUIPHoldbutton2 129,13,137,21,79,byte[maxskip],-1,0

  ; Speed Slider
  cmp eax,7                      ; X-Range for click-area
  jl near .nomovebar
  cmp eax,7+116
  jg near .nomovebar
  sub eax,7
  shr eax,1
  cmp edx,173                    ; Y-Range for click-area
  jl near .nomovebar
  cmp edx,177
  jg near .nomovebar
  mov [EmuSpeed],al
  mov byte[GUIHold],6            ; Lock mouse to bar when clicked
  mov eax,[GUIwinposy+21*4]
  add eax,175
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposx+21*4]
  add eax,7
  mov [GUIHoldXlimL],eax
  add eax,116
  mov [GUIHoldXlimR],eax
.nomovebar
  ret

DrawMouse:
  mov edi,[vidbuffer]
  add edi,16
  add edi,[GUImouseposx]
  mov eax,[GUImouseposy]
  shl eax,8
  mov ebx,[GUImouseposy]
  shl ebx,5
  add eax,ebx
  add edi,eax
  mov esi,GUIMousePtr
  mov edx,8
.next2
  mov ecx,8
.next
  mov al,[esi]
  or al,al
  jz near .nodraw
  test byte[mouseshad],1
  jz near .finishadow

  mov bl,[edi+288*10+8]
  cmp bl,32
  jae .noprimshad
  add byte[edi+288*10+8],96
  jmp .finishadow
.noprimshad

  mov bl,[edi+288*4+3]
  cmp bl,32
  jb .nogray
  cmp bl,63
  ja .nogray
  sub bl,32
  shr bl,1
  add bl,32
  mov [edi+288*4+3],bl
  jmp .finishadow
.nogray
  and bl,11110000b
  cmp bl,64
  jne .nomidshad2
  add byte[edi+288*4+3],16
  jmp .finishadow
.nomidshad2

  mov bl,[edi+288*7+5]
  cmp bl,148
  jb .nomidshad
  cmp bl,167
  ja .mid2
  add byte[edi+288*7+5],20
  jmp .finishadow
.mid2
  cmp bl,189
  jb .nomidshad
  cmp bl,220
  ja .nomidshad
  sub bl,189
  shr bl,1
  add bl,189
  mov [edi+288*7+5],bl
  jmp .finishadow
.nomidshad

.finishadow
  add al,88
  mov [edi],al
.nodraw
  inc esi
  inc edi
  dec ecx
  jnz near .next
  add edi,288-8
  dec edx
  jnz near .next2
  ret
