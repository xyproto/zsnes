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



; GUI Keyboard Routines

%macro GUIqcheckkeys 1
  cmp byte[pressed+%1],1
  jne %%skip
  mov dword[GUIfirstkey],1
%%skip
%endmacro

%macro GUIqcheckkeys2 2
  mov eax,%1
  cmp byte[pressed+eax],1
  jne %%skip
  cmp byte[GUIJoyPadnk+%2],2
  je %%skip
  mov dword[GUIfirstkey],1
%%skip
%endmacro

%macro GUIgetprkeys 2
  cmp byte[pressed+%1],1
  je %%okay
  cmp byte[pressed+%1],2
  jne %%nopr
  cmp dword[GUIfirstkey],1
  je %%nopr
  cmp dword[GUIlastkey],%1
  jne %%nopr
  jmp %%skipnext
%%okay
  mov dword[GUInextkeydelay],10
%%skipnext
  mov dword[GUIlastkey],%1
  mov byte[pressed+%1],2
  cmp dword[GUIkeydelay],0
  jne near .done
  mov eax,[GUInextkeydelay]
  mov [GUIkeydelay],eax
  mov dword[GUInextkeydelay],2
  mov eax,%2
  jmp .done
%%nopr
%endmacro

%macro GUIgetprkeysb 2
  cmp byte[pressed+%1],1
  jne %%nopr
  cmp byte[GUIescpress],1
  je %%nopr2
  mov byte[pressed+%1],2
  mov eax,%2
  jmp .done
%%nopr
  cmp byte[pressed+%1],0
  jne %%nopr2
  mov byte[GUIescpress],0
%%nopr2
%endmacro

%macro GUIgetprkeys2 3
  mov ebx,%1
  cmp byte[pressed+ebx],0
  jne %%pressed
  mov byte[GUIJoyPadnk+%3],0
  jmp %%nopr
%%pressed
  cmp byte[GUIJoyPadnk+%3],2
  je %%secondpress
  cmp byte[pressed+ebx],1
  je %%okay
%%secondpress
  cmp dword[GUIfirstkey],1
  je %%nopr
  cmp dword[GUIlastkey],ebx
  jne %%nopr
  jmp %%skipnext
%%okay
  mov dword[GUInextkeydelay],10
%%skipnext
  mov [GUIlastkey],ebx
  mov byte[GUIJoyPadnk+%3],2
  cmp dword[GUIkeydelay],0
  jne near .done
  mov eax,[GUInextkeydelay]
  mov [GUIkeydelay],eax
  mov byte[GUIDelayB],5
  mov dword[GUInextkeydelay],2
  mov eax,%2
  jmp .done
%%nopr
%endmacro

%macro GUIkeystoggleoptn 2
  cmp %1,%2
  jne %%set
  xor %1,%2
  jmp %%clear
%%set
  mov %1,%2
%%clear
  ret
%endmacro

%macro GUIKeyCheckbox 2
  cmp dh,%2
  jne %%nohotkey
  xor byte[%1],1
%%nohotkey
%endmacro

%macro GUIKeyButtonHole 3
  cmp dh,%3
  jne %%nohotkey
  mov byte[%1],%2
%%nohotkey
%endmacro

%macro ToUpperASM 0
  cmp dh,'a'
  jb %%notlcase
  cmp dh,'z'
  ja %%notlcase
  sub dh,'a'-'A'
%%notlcase
%endmacro

SECTION .bss
GUIfirstkey resd 1
GUIlastkey resd 1
NEWSYM GUIkeydelay, resd 1
NEWSYM GUIkeydelay2, resd 1
GUInextkeydelay resd 1
GUIJoyPadnk resb 8
NEWSYM GUIDelayB, resb 1
UseExtKey resb 1

SECTION .text

GUIgetcurrentinput:
  mov byte[UseExtKey],0
  xor cx,cx
.getnextkey
  call Check_Key
  or al,al
  jz .nokeyd
  call Get_Key
  cmp cl,0
  jne .nostorekey
  mov ch,al
.nostorekey
  xor cl,cl
  cmp al,0
  jne .noextkey
  call Get_Key
  mov [UseExtKey],al
  xor al,al
  mov ch,al
  mov cl,1
.noextkey
;    jmp .getnextkey
.nokeyd
  mov dh,ch
  mov byte[GUIDelayB],0
  ; Convert pressed to keys
  mov dword[GUIfirstkey],0
%ifdef __UNIXSDL___
  GUIqcheckkeys 90 ; UP
  GUIqcheckkeys 96 ; DOWN
  GUIqcheckkeys 92 ; LEFT
  GUIqcheckkeys 94 ; RIGHT
  GUIqcheckkeys 89 ; HOME
  GUIqcheckkeys 91 ; PGUP
  GUIqcheckkeys 95 ; END
  GUIqcheckkeys 97 ; PGDOWN
%endif
  GUIqcheckkeys 72 ; NUMPAD STUFF
  GUIqcheckkeys 80
  GUIqcheckkeys 75
  GUIqcheckkeys 77
  GUIqcheckkeys 73
  GUIqcheckkeys 81
  GUIqcheckkeys 71
  GUIqcheckkeys 79
  GUIqcheckkeys 1
  GUIqcheckkeys 1Ch

%ifndef __MSDOS__
  GUIqcheckkeys 0C8h
  GUIqcheckkeys 0D0h
  GUIqcheckkeys 0CBh
  GUIqcheckkeys 0CDh
  GUIqcheckkeys 0C9h
  GUIqcheckkeys 0D1h
  GUIqcheckkeys 0C7h
  GUIqcheckkeys 0CFh
  GUIqcheckkeys 09Ch
%endif
.notwinport

  cmp byte[JoyPad1Move],0
  je near .nopad1
  push edx
  call JoyRead
  pop edx
  GUIqcheckkeys2 [pl1upk],0
  GUIqcheckkeys2 [pl1downk],1
  GUIqcheckkeys2 [pl1leftk],2
  GUIqcheckkeys2 [pl1rightk],3
  GUIqcheckkeys2 [pl1Lk],4
  GUIqcheckkeys2 [pl1Rk],5
  GUIqcheckkeys2 [pl1Bk],6
  GUIqcheckkeys2 [pl1Ak],7
.nopad1
  xor al,al
%ifdef __UNIXSDL__
  GUIgetprkeys 90,90 ; UP
  GUIgetprkeys 96,96 ; DOWN
  GUIgetprkeys 92,92 ; LEFT
  GUIgetprkeys 94,94 ; RIGHT
  GUIgetprkeys 89,89 ; HOME
  GUIgetprkeys 91,91 ; PGUP
  GUIgetprkeys 95,95 ; END
  GUIgetprkeys 97,97 ; PGDOWN

  GUIgetprkeys 72,72 ; KP8
  GUIgetprkeys 80,80 ; KP2
  GUIgetprkeys 75,75 ; KP4
  GUIgetprkeys 77,77 ; KP6
  GUIgetprkeys 71,71 ; KP7
  GUIgetprkeys 79,79 ; KP9
  GUIgetprkeys 81,81 ; KP1
  GUIgetprkeys 73,73 ; KP3
%else
;    GUIgetprkeys 72,72
;    GUIgetprkeys 80,80
  GUIgetprkeys 75,75
  GUIgetprkeys 77,77
  GUIgetprkeys 71,71
  GUIgetprkeys 79,79
  GUIgetprkeys 81,81
  GUIgetprkeys 73,73
%endif
  GUIgetprkeysb 1,27
  GUIgetprkeys 1Ch,13

  mov cl,[UseExtKey]
  cmp cl,72
  je .okayextk
  cmp cl,80
  jne .notextkb
.okayextk
  mov al,cl
.notextkb

%ifdef __UNIXSDL__
  GUIgetprkeys 90,90 ; UP
  GUIgetprkeys 96,96 ; DOWN
  GUIgetprkeys 92,92 ; LEFT
  GUIgetprkeys 94,94 ; RIGHT
  GUIgetprkeys 89,89 ; HOME
  GUIgetprkeys 91,91 ; PGUP
  GUIgetprkeys 95,95 ; END
  GUIgetprkeys 97,97 ; PGDOWN

  GUIgetprkeys 72,72 ; KP8
  GUIgetprkeys 80,80 ; KP2
  GUIgetprkeys 75,75 ; KP4
  GUIgetprkeys 77,77 ; KP6
  GUIgetprkeys 71,71 ; KP7
  GUIgetprkeys 79,79 ; KP9
  GUIgetprkeys 81,81 ; KP1
  GUIgetprkeys 73,73 ; KP3

  GUIgetprkeys 09Ch,13 ; ENTER
%endif
%ifdef __WIN32__
  GUIgetprkeys 0CBh,75
  GUIgetprkeys 0CDh,77
  GUIgetprkeys 0C9h,73
  GUIgetprkeys 0D1h,81
  GUIgetprkeys 0C7h,71
  GUIgetprkeys 0CFh,79
  GUIgetprkeys 09Ch,13
%endif
;.notwinport2

  cmp byte[JoyPad1Move],0
  je near .nopad1b
  GUIgetprkeys2 [pl1upk],72,0
  GUIgetprkeys2 [pl1downk],80,1
  GUIgetprkeys2 [pl1leftk],75,2
  GUIgetprkeys2 [pl1rightk],77,3
  GUIgetprkeys2 [pl1Lk],73,4
  GUIgetprkeys2 [pl1Rk],81,5
  GUIgetprkeys2 [pl1Bk],27,6
  GUIgetprkeys2 [pl1Ak],13,7
.nopad1b
  cmp dh,' '
  jne .notspace
  mov al,' '
.notspace
  mov dword[GUInextkeydelay],10
  mov dword[GUIkeydelay],0
.done
  cmp byte[GUIcmenupos],0
  jne near .processmenu
  mov dl,al
  xor eax,eax
  mov al,[GUIwinptr]
  cmp dl,27
  jne .noclosewin
  or eax,eax
  jz near .runprog
.noclosewin
  dec eax
  xor ebx,ebx
  mov bl,[GUIwinorder+eax]
  cmp ebx,13
  jne .notcskeys
  cmp byte[CheatWinMode],1
  ja .avoidesc
.notcskeys
  cmp dl,27
  je near .closewin
.avoidesc
  mov al,dl
  cmp ebx,1
  je near GUILoadKeys
  cmp ebx,2
  je near GUIStateSelKeys
  cmp ebx,3
  je near GUIInputKeys
  cmp ebx,4
  je near GUIOptionKeys
  cmp ebx,5
  je near GUIVideoKeys
  cmp ebx,6
  je near GUISoundKeys
  cmp ebx,7
  je near GUICheatKeys
  cmp ebx,10
  je near GUIGUIOptnsKeys
  cmp ebx,11
  je near GUIAboutKeys
  cmp ebx,12
  je near GUIResetKeys
  cmp ebx,13
  je near GUICheatSearchKeys
  cmp ebx,14
  je near GUIStateKeys
  cmp ebx,15
  je near GUIMovieKeys
  cmp ebx,16
  je near GUIComboKeys
  cmp ebx,17
  je near GUIAddonKeys
  cmp ebx,18
  je near GUIChipKeys
  cmp ebx,19
  je near GUIPathKeys
  cmp ebx,20
  je near GUISaveKeys
  cmp ebx,21
  je near GUISpeedKeys
  cmp ebx,8
  je near GUIGetInputLine
  ret
.runprog
  cmp byte[romloadskip],0
  jne .noquit
  mov byte[GUIQuit],2
.noquit
  ret
.closewin
  ; close window
  mov byte[GUIwinorder+eax],0
  mov byte[GUIwinactiv+ebx],0
  dec byte[GUIwinptr]
  cmp byte[GUIwinptr],0
  jne .nowinopen
  mov al,[GUIpmenupos]
  mov [GUIcmenupos],al
.nowinopen
  ret
.processmenu
  ToUpperASM
  cmp dh,'X'                 ; Main Menu Hotkeys
  jne .noclose
  mov byte[GUIcmenupos],0
  mov byte[GUIcrowpos],0
.noclose
  cmp dh,'R'
  jne .norecent
  mov byte[GUIcmenupos],1
  mov byte[GUIcrowpos],0
.norecent
  cmp dh,'G'
  jne .nogame
  mov byte[GUIcmenupos],2
  mov byte[GUIcrowpos],0
.nogame
  cmp dh,'C'
  jne .noconfig
  mov byte[GUIcmenupos],3
  mov byte[GUIcrowpos],0
.noconfig
  cmp dh,'H'
  jne .nocheat
  mov byte[GUIcmenupos],4
  mov byte[GUIcrowpos],0
.nocheat
  cmp dh,'N'
  jne .nonetplay
  mov byte[GUIcmenupos],5
  mov byte[GUIcrowpos],0
.nonetplay
  cmp dh,'M'
  jne .nomisc
  mov byte[GUIcmenupos],6
  mov byte[GUIcrowpos],0
.nomisc
  cmp byte[GUIcmenupos],1       ; Recently Played Hotkeys
  jne near .noquickmenu
  cmp dh,'1'
  jne .no1
  mov byte[GUIcrowpos],0
.no1
  cmp dh,'2'
  jne .no2
  mov byte[GUIcrowpos],1
.no2
  cmp dh,'3'
  jne .no3
  mov byte[GUIcrowpos],2
.no3
  cmp dh,'4'
  jne .no4
  mov byte[GUIcrowpos],3
.no4
  cmp dh,'5'
  jne .no5
  mov byte[GUIcrowpos],4
.no5
  cmp dh,'6'
  jne .no6
  mov byte[GUIcrowpos],5
.no6
  cmp dh,'7'
  jne .no7
  mov byte[GUIcrowpos],6
.no7
  cmp dh,'8'
  jne .no8
  mov byte[GUIcrowpos],7
.no8
  cmp dh,'9'
  jne .no9
  mov byte[GUIcrowpos],8
.no9
  cmp dh,'0'
  jne .no0
  mov byte[GUIcrowpos],9
.no0
  cmp dh,'F'
  jne .nof
  mov byte[GUIcrowpos],11
.nof
  cmp dh,'L'
  jne .nol
  mov byte[GUIcrowpos],12
.nol
.noquickmenu                        ; Game Hotkeys
  cmp byte[GUIcmenupos],2
  jne near .noloadmenu
  cmp dh,'L'
  jne .noll
  mov byte[GUIcrowpos],0
.noll
  cmp dh,'E'
  jne .nor
  mov byte[GUIcrowpos],2
.nor
  cmp dh,'S'
  jne .nos
  mov byte[GUIcrowpos],4
.nos
  cmp dh,'O'
  jne .noo
  mov byte[GUIcrowpos],5
.noo
  cmp dh,'P'
  jne .nop
  mov byte[GUIcrowpos],6
.nop
  cmp dh,'Q'
  jne .noq
  mov byte[GUIcrowpos],8
.noq
.noloadmenu                            ; Config Hotkeys
  cmp byte[GUIcmenupos],3
  jne near .noconfigmenu
  cmp dh,'I'
  jne .noin
  mov byte[GUIcrowpos],0
.noin
  cmp dh,'D'
  jne .noind
  mov byte[GUIcrowpos],2
.noind
  cmp dh,'F'
  jne .noinc
  mov byte[GUIcrowpos],3
.noinc
  cmp dh,'O'
  jne .noino
  mov byte[GUIcrowpos],5
.noino
  cmp dh,'V'
  jne .noinv
  mov byte[GUIcrowpos],6
.noinv
  cmp dh,'S'
  jne .noins
  mov byte[GUIcrowpos],7
.noins
  cmp dh,'P'
  jne .noinp
  mov byte[GUIcrowpos],8
.noinp
  cmp dh,'A'
  jne .noina
  mov byte[GUIcrowpos],9
.noina
  cmp dh,'E'
  jne .noine
  mov byte[GUIcrowpos],10
.noine
.noconfigmenu                          ; Cheat Hotkeys
  cmp byte[GUIcmenupos],4
  jne near .nocheatmenu
  cmp dh,'A'
  jne .noa
  mov byte[GUIcrowpos],0
.noa
  cmp dh,'B'
  jne .nob
  mov byte[GUIcrowpos],1
.nob
  cmp dh,'S'
  jne .nos2
  mov byte[GUIcrowpos],2
.nos2
.nocheatmenu                           ; Misc Hotkeys
  cmp byte[GUIcmenupos],6
  jne near .nomiscmenu
  cmp dh,'K'
  jne .nomk
  mov byte[GUIcrowpos],0
.nomk
  cmp dh,'U'
  jne .nomu
  mov byte[GUIcrowpos],1
.nomu
  cmp dh,'O'
  jne .nomo
  mov byte[GUIcrowpos],2
.nomo
  cmp dh,'E'
  jne .nome
  mov byte[GUIcrowpos],3
.nome
  cmp dh,'S'
  jne .noms
  mov byte[GUIcrowpos],4
.noms
  cmp dh,'A'
  jne .noma
  mov byte[GUIcrowpos],6
.noma
.nomiscmenu

  cmp byte[romloadskip],0
  jne .dontquit
  cmp al,27
  jne .noexit
  mov byte[GUIQuit],2
.noexit
.dontquit
  cmp al,13
  jne .noenter
  call GUITryMenuItem
.noenter
%ifdef __UNIXSDL__
  cmp al,92
  je .leftpressed
  cmp dword[numlockptr],1
  je .noleft
  cmp al,75
  jne .noleft
.leftpressed
%else
  cmp al,75
  jne .noleft
%endif
  mov byte[GUIcrowpos],0
  cmp byte[GUIcmenupos],1
  jbe .left
  dec byte[GUIcmenupos]
  jmp .noleft
.left
  mov byte[GUIcmenupos],6
.noleft
%ifdef __UNIXSDL__
  cmp al,94
  je .rightpressed
  cmp dword[numlockptr],1
  je .noright
  cmp al,77
  jne .noright
.rightpressed
%else
  cmp al,77
  jne .noright
%endif
  mov byte[GUIcrowpos],0
  inc byte[GUIcmenupos]
  cmp byte[GUIcmenupos],7
  jne .noright
  mov byte[GUIcmenupos],1
.noright
  cmp byte[GUIcmenupos],0
  je near .noupdown
%ifdef __UNIXSDL__
  cmp al,96
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
  xor eax,eax
  mov al,[GUIcrowpos]
  mov ebx,[GUICYLocPtr]
  cmp byte[eax+ebx+1],2
  jne .nobottom
  mov byte[GUIcrowpos],0
  jmp .nokey
.nobottom
  inc eax
  cmp byte[eax+ebx+1],0
  je .nobottom
  mov [GUIcrowpos],al
  jmp .nokey
.nodown
%ifdef __UNIXSDL__
  cmp al, 90
  je .uppressed
  cmp dword[numlockptr],1
  je .noup
  cmp al,72
  jne .noup
.uppressed
%else
  cmp al,72
  jne .noup
%endif
  xor eax,eax
  mov al,[GUIcrowpos]
  mov ebx,[GUICYLocPtr]
  or al,al
  jz .top
  cmp byte[eax+ebx+1],3
  jne .notop
.top
  mov al,[ebx]
  mov [GUIcrowpos],al
  jmp .nokey
.notop
  dec eax
  cmp byte[eax+ebx+1],0
  je .notop
  mov [GUIcrowpos],al
  jmp .nokey
.noup
.noupdown
.nokey
  ret

%macro KeyTabInc 1-* ; tab arrays
  push edx
%rep %0
  xor edx,edx
  mov eax,[%1]
  div dword[%1+4]
  mov [%1],edx
  %rotate 1
  or [%1],eax
  cmp eax,1
  je %%exit
  %rotate -1
  sub eax,[%1]
  adc dword[%1],0
  cmp dword[%1],0
  jne %%exit
  %rotate 1
%endrep
%%exit
  pop edx
%endmacro

GUIInputKeys:
  ToUpperASM
  cmp dh,9
  jne .notab
  KeyTabInc GUIInputTabs
  mov byte[GUIFreshInputSelect],1
.notab
  GUIKeyCheckbox AllowUDLR,'A'
  GUIKeyCheckbox Turbo30hz,'T'
  cmp dh,'U'
  jne .nousepl34
  xor byte[pl12s34],1
  mov byte[MultiTap],1
  cmp byte[pl12s34],1
  je .nomtap
  cmp byte[pl3contrl],0
  jne .mtap
  cmp byte[pl4contrl],0
  jne .mtap
  cmp byte[pl5contrl],0
  jne .mtap
.nomtap
  mov byte[MultiTap],0
.mtap
.nousepl34
%ifdef __MSDOS__
  GUIKeyCheckbox SidewinderFix,'S'
%endif
  ret

GUITCPIPKeys:
  ret

GUIOptionKeys:
  ToUpperASM
  cmp dh,'N'                             ; Checkboxes
  jne .nonewgfx
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUINGVID+eax],1
  jne near .invalidng
  xor byte[newengen],1
.invalidng
.nonewgfx
  cmp dh,'M'
  jne .nommxcopy
  xor byte[MMXSupport],1
  call MMXCheck
.nommxcopy
  GUIKeyCheckbox FPSAtStart,'F'
  GUIKeyCheckbox TimerEnable,'C'
  GUIKeyCheckbox ClockBox,'X'
  GUIKeyCheckbox TwelveHourClock,'1'
  GUIKeyCheckbox bgfixer,'A'
  GUIKeyCheckbox SmallMsgText,'S'
  GUIKeyCheckbox AutoPatch,'I'
  GUIKeyCheckbox RomInfo,'G'
  GUIKeyCheckbox DisplayInfo,'W'
  GUIKeyCheckbox GUIEnableTransp,'T'
%ifndef __MSDOS__
  GUIKeyCheckbox PauseFocusChange,'B'
%endif
%ifdef __WIN32__
  GUIKeyCheckbox HighPriority,'H'
  pushad
  call CheckPriority
  popad
  GUIKeyCheckbox DisableScreenSaver,'D'
  pushad
  call CheckScreenSaver
  popad
%endif
  ret

GUIAboutKeys:
  ToUpperASM
  cmp dh,'E'
  jne .noeegg
  xor byte[EEgg],1
.noeegg
  ret

GUISaveKeys:
  ToUpperASM
  cmp dh,'I'             ; Checkboxes
  jne .noautoincsave
  xor byte[AutoIncSaveSlot],1
.noautoincsave

  GUIKeyCheckbox SRAMSave5Sec,'C'
  GUIKeyCheckbox LatestSave,'S'
  GUIKeyCheckbox AutoState,'A'
  GUIKeyCheckbox SRAMState,'L'
  GUIKeyCheckbox PauseLoad,'P'
  GUIKeyCheckbox PauseRewind,'R'
  ret

GUISpeedKeys:
  ToUpperASM
  GUIKeyCheckbox FastFwdToggle,'T'   ; Checkbox

  cmp dh,'A'                 ; Framerate Checkboxes
  jne .noautofr
  cmp byte[frameskip],0  ; 0 = autoframerate / 1-10 = frameskip 0-9
  je .zero
  mov byte[frameskip],0
  jmp .notzero
.zero
  mov byte[FPSOn],0
  mov byte[frameskip],1
.notzero
.noautofr
  ret

GUIAddonKeys:
  ToUpperASM
  GUIKeyButtonHole device1,0,'G'             ;Radio Buttons
  GUIKeyButtonHole device1,1,'M'
  GUIKeyButtonHole device2,0,'A'
  GUIKeyButtonHole device2,1,'O'
  GUIKeyButtonHole device2,2,'S'
  GUIKeyButtonHole device2,3,'1'
  GUIKeyButtonHole device2,4,'2'

  GUIKeyCheckbox mouse1lh,'L'             ; Checkboxes
  GUIKeyCheckbox mouse2lh,'E'
  ret

GUIChipKeys:
  ToUpperASM
  GUIKeyCheckbox nssdip1,'1'             ; Checkboxes
  GUIKeyCheckbox nssdip2,'2'
  GUIKeyCheckbox nssdip3,'3'
  GUIKeyCheckbox nssdip4,'4'
  GUIKeyCheckbox nssdip5,'5'
  GUIKeyCheckbox nssdip6,'6'
  ret

GUIGUIOptnsKeys:
  ToUpperASM
  GUIKeyCheckbox mousewrap,'M'            ; Checkboxes
  GUIKeyCheckbox mouseshad,'S'
  GUIKeyCheckbox esctomenu,'G'
  GUIKeyCheckbox resetposn,'W'
  GUIKeyCheckbox GUIRClick,'C'
  GUIKeyCheckbox JoyPad1Move,'U'
  GUIKeyCheckbox newfont,'O'
  GUIKeyCheckbox lhguimouse,'D'

  cmp dh,'F'
  jne .guioptn8
  xor byte[FilteredGUI],1
  cmp byte[En2xSaI],1
  jnae .guioptn8
  call ClearScreen
  pushad
  call Clear2xSaIBuffer
  popad
.guioptn8
  cmp byte[hqFilter],0
  je .guioptn9
  call ClearScreen
  pushad
  call Clear2xSaIBuffer
  popad
.guioptn9

  GUIKeyButtonHole GUIEffect,0,'E'   ; Start From Radio Buttons
  GUIKeyButtonHole GUIEffect,1,'N'
  GUIKeyButtonHole GUIEffect,2,'A'
  GUIKeyButtonHole GUIEffect,3,'B'
  GUIKeyButtonHole GUIEffect,4,'R'

%ifdef __WIN32__
  GUIKeyCheckbox MouseWheel,'H'
  GUIKeyCheckbox TrapMouseCursor,'P'
  GUIKeyCheckbox AlwaysOnTop,'T'
  GUIKeyCheckbox SaveMainWindowPos,'V'
  GUIKeyCheckbox AllowMultipleInst,'L'
%endif
ret

GUIVideoKeys:
%ifndef __MSDOS__
  call GUIPathKeys
%endif
  cmp dh,9
  jne near .tabdone
  cmp byte[NTSCFilter],0
  je near .nontsc
  movzx eax,byte[cvidmode]
  cmp byte[GUINTVID+eax],0
  je .nontsc
  KeyTabInc GUIVideoTabs,GUIVntscTab
  jmp .tabdone
.nontsc
  KeyTabInc GUIVideoTabs
.tabdone
  ToUpperASM
  xor eax,eax
  cmp dword[GUIVideoTabs],1
  jne near .notmodestab
;   "Home"
%ifdef __UNIXSDL__
  cmp dl,89
  je .yeshome
  cmp dword[numlockptr],1
  je .nohome
  cmp dl,71
  jne .nohome
.yeshome
%else
  cmp dl,71
  jne .nohome
%endif
  mov dword[GUIcurrentvideocursloc],0
  mov dword[GUIcurrentvideoviewloc],0
.nohome
;   "End"
%ifdef __UNIXSDL__
  cmp dl,95
  je .yesendp
  cmp dword[numlockptr],1
  je .noendp
  cmp dl,79
  jne .noendp
.yesendp
%else
  cmp dl,79
  jne .noendp
%endif
  mov eax,[NumVideoModes]
  dec eax
  mov [GUIcurrentvideocursloc],eax
  sub eax,19
  mov [GUIcurrentvideoviewloc],eax
  test dword[GUIcurrentvideoviewloc],8000000h
  jz .noendp
  mov dword[GUIcurrentvideoviewloc],0
.noendp
;   "Up"
%ifdef __UNIXSDL__
  cmp dl,90
  je .up
  cmp dword[numlockptr],1
  je .noup
  cmp dl,72
  jne .noup
%else
  cmp dl,72
  jne .noup
%endif
.up
  cmp dword[GUIcurrentvideocursloc],0
  je .noup
  mov ebx,[GUIcurrentvideocursloc]
  cmp [GUIcurrentvideoviewloc],ebx
  jne .noviewdec
  dec dword[GUIcurrentvideoviewloc]
.noviewdec
  dec dword[GUIcurrentvideocursloc]
.noup
;   "Down"
%ifdef __UNIXSDL__
  cmp dl,96
  je .yesdown
  cmp dword[numlockptr],1
  je .nodown
  cmp dl,80
  jne .nodown
.yesdown
%else
  cmp dl,80
  jne .nodown
%endif
  mov ebx,[GUIcurrentvideocursloc]
  inc ebx
  cmp ebx,[NumVideoModes]
  je .nodown
  inc dword[GUIcurrentvideocursloc]
  sub ebx,20
  cmp ebx,[GUIcurrentvideoviewloc]
  jne .nodown
  inc dword[GUIcurrentvideoviewloc]
.nodown
;   "PageUp"
%ifdef __UNIXSDL__
  cmp dl,91
  je .yespageup
  cmp dword[numlockptr],1
  je .nopageup
  cmp dl,73
  jne .nopageup
.yespageup
%else
  cmp dl,73
  jne .nopageup
%endif
  sub dword[GUIcurrentvideoviewloc],20
  sub dword[GUIcurrentvideocursloc],20
  test dword[GUIcurrentvideoviewloc],8000000h
  jz .posv
  mov dword[GUIcurrentvideoviewloc],0
.posv
  test dword[GUIcurrentvideocursloc],8000000h
  jz .posc
  mov dword[GUIcurrentvideocursloc],0
.posc
.nopageup
;   "PageDown"
%ifdef __UNIXSDL__
  cmp dl,97
  je .yespagedn
  cmp dword[numlockptr],1
  je .nopagedown
  cmp dl,81
  jne .nopagedown
.yespagedn
%else
  cmp dl,81
  jne .nopagedown
%endif
  add dword[GUIcurrentvideoviewloc],20
  add dword[GUIcurrentvideocursloc],20
  mov ebx,[NumVideoModes]
  dec ebx
  cmp dword[GUIcurrentvideocursloc],ebx
  jb .poscd
  mov [GUIcurrentvideocursloc],ebx
.poscd
  sub ebx,19
  cmp dword[GUIcurrentvideoviewloc],ebx
  jl .posvd
  test ebx,8000000h
  jz .noneg
  mov ebx,0
.noneg
  mov [GUIcurrentvideoviewloc],ebx
.posvd
.nopagedown
; "Return"
  cmp dl,13
  jne .noreturn
  mov byte[GUICBHold],4
.noreturn
.notmodestab

  cmp dword[GUIVideoTabs],2
  jne near .notfilterstab
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUISLVID+eax],0
  je .nofullscanlines
  cmp dh,'F'
  jne .nofullscanlines
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[NTSCFilter],0
  call ClearScreen
  GUIkeystoggleoptn byte[scanlines],1
%ifdef __MSDOS__
  cmp byte[cvidmode],2
  jne .nomodeq
  jmp .refresh
.nomodeq
  cmp byte[cvidmode],5
  jne .nofullscanlines
.refresh
  mov byte[cbitmode],1
  call initvideo2
  mov byte[cbitmode],0
  call GUISetPal
%endif
.nofullscanlines
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUIHSVID+eax],0
  je near .noquarterscanlines
  cmp dh,'5'
  jne .nohalfscanlines
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[NTSCFilter],0
  call ClearScreen
  GUIkeystoggleoptn byte[scanlines],3
.nohalfscanlines
  cmp dh,'2'
  jne .noquarterscanlines
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[NTSCFilter],0
  call ClearScreen
  GUIkeystoggleoptn byte[scanlines],2
.noquarterscanlines
  cmp dh,'Q'
  jne .nohqfilter
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIHQ2X+ebx],0
  jne .hqfilter
  cmp byte[GUIHQ3X+ebx],0
  jne .hqfilter
  cmp byte[GUIHQ4X+ebx],0
  jne .hqfilter
  jmp .nohqfilter
.hqfilter
  pushad
  call Clear2xSaIBuffer
  popad
  xor byte[hqFilter],1
  jz .nohqfilter
  mov byte[scanlines],0
  mov byte[En2xSaI],0
  mov byte[antienab],0
  mov byte[NTSCFilter],0
.nohqfilter
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUI2xVID+ebx],0
  je near .nosuper2xsai
  cmp dh,'X'
  jne .no2xsai
  mov eax,1
  jmp .yesfilter
.no2xsai
  cmp dh,'E'
  jne .nosupereagle
  mov eax,2
  jmp .yesfilter
.nosupereagle
  cmp dh,'S'
  jne .nosuper2xsai
  mov eax,3
.yesfilter
  pushad
  call Clear2xSaIBuffer
  popad
  mov byte[hqFilter],0
  mov byte[scanlines],0
  mov byte[antienab],0
  mov byte[NTSCFilter],0
  GUIkeystoggleoptn byte[En2xSaI],al
.nosuper2xsai
  cmp dh,'I'
  jne .nointerpolation
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUII2VID+eax],0
  jne .interpolation
  cmp byte[GUIIEVID+eax],0
  je .nointerpolation
.interpolation
  xor byte[antienab],1
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[NTSCFilter],0
.nointerpolation
%ifdef __MSDOS__
  cmp dh,'C'
  jne .nofullscreen
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUIFSVID+eax],0
  je .nofullscreen
  call ClearScreen
  xor byte[ScreenScale],1
  mov byte[smallscreenon],0
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
.fsloop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz .fsloop
  pop ecx
  pop eax
.nofullscreen
  cmp dh,'M'
  jne .nosmallscreen
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUISSVID+eax],0
  je .nosmallscreen
  call ClearScreen
  xor byte[smallscreenon],1
  mov byte[ScreenScale],0
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
.ssloop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz .ssloop
  pop ecx
  pop eax
.nosmallscreen
%endif
  cmp dh,'H'
  jne .nohiresmode7
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIM7VID+ebx],0
  je .nohiresmode7
  xor byte[Mode7HiRes16b],1
.nohiresmode7
  cmp dh,'N'
  jne .nontscfilter
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUINTVID+ebx],0
  je .nontscfilter
  xor byte[NTSCFilter],1
  jz .nontscfilter
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
  mov byte[scanlines],0
  mov byte[antienab],0
  mov byte[BilinearFilter],0
%ifdef __WIN32__
  cmp byte[NTSCFilter],0
  je .noforce4_3
  mov byte[Keep4_3Ratio],1
.noforce4_3
%endif
  pushad
  call NTSCFilterInit
  popad
.nontscfilter
%ifdef __UNIXSDL__
  cmp dh,'I'
  jne .nobilinearfilter
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIBIFIL+ebx],0
  je .nobilinearfilter
  xor byte[BilinearFilter],1
  mov byte[NTSCFilter],0
.nobilinearfilter
%endif
%ifdef __WIN32__
  cmp dh,'T'
  jne .notriplebufferwin
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUITBWVID+eax],0
  je near .notriplebufferwin
  xor byte[TripleBufferWin],1
  call initDirectDraw
  pushad
  call Clear2xSaIBuffer
  popad
.notriplebufferwin
%elifdef __MSDOS__
  cmp dh,'T'
  jne .notriplebuffer
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUITBVID+ebx],0
  je .notriplebuffer
  xor byte[Triplebufen],1
;    mov byte[vsyncon],0
.notriplebuffer
%endif
%ifndef __UNIXSDL__
  cmp dh,'V'
  jne .novsync
  xor byte[vsyncon],1
%ifdef __WIN32__
  call initDirectDraw
  pushad
  call Clear2xSaIBuffer
  popad
%else
;    mov byte[Triplebufen],0
%endif
.novsync
%endif
  cmp dh,'G'
  jne .nograyscale
  xor byte[GrayscaleMode],1
.nograyscale
%ifndef __MSDOS__
  cmp dh,'R'
  jne .nokeep4_3
  xor ebx,ebx
  mov bl,[cvidmode]
%ifdef __WIN32__
  cmp byte[GUIDSMODE+ebx],0
  jne near .allowk43
  cmp byte[GUISMODE+ebx],0
  jne near .allowk43
  jmp .nokeep4_3
%else
  cmp bl,21
  jl .nokeep4_3
%endif
.allowk43
  xor byte[Keep4_3Ratio],1
  pushad
  call initwinvideo
  call Clear2xSaIBuffer
  popad
.nokeep4_3
%endif
.notfilterstab

  cmp dword[GUIVntscTab],1
  jne .notntsctab
  cmp dh,'B'
  jne .noblend
  xor byte[NTSCBlend],1
.noblend
  cmp dh,'R'
  jne .norefresh
  xor byte[NTSCRef],1
.norefresh
.notntsctab
  ret

GUISoundKeys:
  ToUpperASM
  cmp dh,'E'
  jne .notenablesound
  xor byte[soundon],1
%ifdef __WIN32__
  pushad
  call reInitSound
  popad
%endif
.notenablesound

  GUIKeyCheckbox StereoSound,'S'
  GUIKeyCheckbox RevStereo,'V'
  GUIKeyCheckbox Surround,'M'
  GUIKeyCheckbox SPCDisable,'D'
%ifdef __MSDOS__
  GUIKeyCheckbox Force8b,'F'
%endif
%ifdef __WIN32__
  GUIKeyCheckbox PrimaryBuffer,'P'
%endif

  cmp dh,'R'                 ; Sampling Rate
  jne .notsrate
  mov eax,[SoundQuality]
  mov al,[.sampratenext+eax]
  mov [SoundQuality],al
.notsrate

  GUIKeyButtonHole SoundInterpType,0,'N'
  GUIKeyButtonHole SoundInterpType,1,'G'
  GUIKeyButtonHole SoundInterpType,2,'C'
  cmp byte[MMXSupport],0
  je .no8ptinterp
  GUIKeyButtonHole SoundInterpType,3,'8'
.no8ptinterp

  GUIKeyButtonHole LowPassFilterType,0,'O'
  GUIKeyButtonHole LowPassFilterType,1,'I'
  GUIKeyButtonHole LowPassFilterType,2,'Y'
  cmp byte[MMXSupport],0
  je .nohiqualityfilter
  GUIKeyButtonHole LowPassFilterType,3,'H'
.nohiqualityfilter
  ret

SECTION .data
.sampratenext db 1,4,5,6,2,3,0,0
SECTION .text

GUINetMenuKeys1:
  ret

GUIGetInputLine:
  ret

SECTION .bss
GUIEditString resd 1              ; Pointer to string
GUIEditStringcLen resd 1          ; Pointer to end of string (done in function)
GUIEditStringmLen resd 1          ; Max length of string
GUIEditStringLTxt resd 1          ; blink value, should be set to 0 when box selected
GUIEditStringLstb resd 1          ; if cursor is placed (set in main function)
GUIEditStringcWin resd 1          ; 0 = no text window selected
SECTION .text

GUIPathKeys:
  cmp dh,0
  je near .nokeypressed
  cmp dword[GUIInputBox],0
  je near .nokeypressed

  cmp dword[GUIInputBox],1
  jne .notBox1
  mov eax,SRAMPath
  jmp .boxselected
.notBox1
  cmp dword[GUIInputBox],2
  jne .notBox2
  mov eax,SnapPath
  jmp .boxselected
.notBox2
  cmp dword[GUIInputBox],3
  jne .notBox3
  mov eax,SPCPath
  jmp .boxselected
.notBox3
  cmp dword[GUIInputBox],4
  jne .notBox4
  mov eax,BSXPath
  jmp .boxselected
.notBox4
  cmp dword[GUIInputBox],5
  jne .notBox5
  mov eax,SGPath
  jmp .boxselected
.notBox5
  cmp dword[GUIInputBox],6
  jne .notBox6
  mov eax,STPath
  jmp .boxselected
.notBox6
  cmp dword[GUIInputBox],7
  jne .notBox7
  mov eax,GNextPath
  jmp .boxselected
.notBox7
  cmp dword[GUIInputBox],8
  jne .notBox8
  mov eax,FEOEZPath
  jmp .boxselected
.notBox8
  cmp dword[GUIInputBox],9
  jne .notBox9
  mov eax,SJNSPath
  jmp .boxselected
.notBox9
  cmp dword[GUIInputBox],10
  jne .notBox10
  mov eax,MDHPath
  jmp .boxselected
.notBox10
  cmp dword[GUIInputBox],11
  jne .notBox11
  mov eax,SPL4Path
  jmp .boxselected
.notBox11
%ifndef __MSDOS__
  cmp dword[GUIInputBox],12
  jne .notBox12
  mov eax,GUICustomX
  jmp .boxselected
.notBox12
  cmp dword[GUIInputBox],13
  jne .notBox13
  mov eax,GUICustomY
  jmp .boxselected
.notBox13
%endif

.boxselected
  mov ecx,eax

%ifdef __WIN32__
  ;Look for paste request
  cmp byte[ctrlptr],0
  je .nopaste
  cmp dh,56h ;V
  jne .nopaste
  mov [CBBuffer],ecx
  mov eax,[GUIInputLimit]
  mov [CBLength],eax
  pushad
  call PasteClipBoard
  popad
  xor dx,dx
  ret
.nopaste
%endif

  ;Find end of string
.findend
  cmp byte[eax],0
  je .foundend
  inc eax
  jmp .findend
.foundend

  cmp dh,8    ;Backspace
  jne .notbackspace
  cmp eax,ecx
  je .cantback
  mov byte[eax-1],0
.cantback
  xor dx,dx
  ret
.notbackspace

  cmp dh,13    ;Enter
  jne .notenter
  mov dword[GUIInputBox],0
  pushad
  call init_save_paths
  popad
  xor dx,dx
  ret
.notenter

  ;check if we're at the end
  add ecx,dword[GUIInputLimit]
  cmp eax,ecx
  jne .stillroom
  xor dx,dx
  ret
.stillroom
  mov [eax],dh

  xor dx,dx
.nokeypressed
  ret

SECTION .bss
GUIInputBox resd 1
GUIInputLimit resd 1
SECTION .text

GUIMovieKeys:
  ToUpperASM
  GUIKeyButtonHole CMovieExt,'v','0'         ; Select Radio Buttons
  GUIKeyButtonHole CMovieExt,'1','1'
  GUIKeyButtonHole CMovieExt,'2','2'
  GUIKeyButtonHole CMovieExt,'3','3'
  GUIKeyButtonHole CMovieExt,'4','4'
  GUIKeyButtonHole CMovieExt,'5','5'
  GUIKeyButtonHole CMovieExt,'6','6'
  GUIKeyButtonHole CMovieExt,'7','7'
  GUIKeyButtonHole CMovieExt,'8','8'
  GUIKeyButtonHole CMovieExt,'9','9'

  GUIKeyButtonHole MovieStartMethod,0,'N'   ; Start From Radio Buttons
  GUIKeyButtonHole MovieStartMethod,1,'P'
  GUIKeyButtonHole MovieStartMethod,2,'R'
  GUIKeyButtonHole MovieStartMethod,3,'S'

  GUIKeyButtonHole MovieVideoMode,0,'O'
  GUIKeyButtonHole MovieVideoMode,1,'W'
  GUIKeyButtonHole MovieVideoMode,2,'F'
  GUIKeyButtonHole MovieVideoMode,3,'L'
  GUIKeyButtonHole MovieVideoMode,4,'X'
  ;GUIKeyButtonHole MovieVideoMode,5,'C'

  GUIKeyCheckbox MovieAudio,'A'
  GUIKeyCheckbox MovieVideoAudio,'V'
  GUIKeyCheckbox MovieAudioCompress,'M'

  GUIKeyCheckbox MovieDisplayFrame,'D'      ; Checkbox
  ret

GUIComboKeys:
  ; Calculate Position
  xor eax,eax
.nozero
  cmp byte[GUIComboTextH+eax],0
  je .zero
  inc eax
  jmp .nozero
.zero
  mov [GUIComboPos],al
  cmp dh,8
  jne .nobacksp
  cmp byte[GUIComboPos],0
  je .nochar
  mov byte[GUICCFlash],0
  xor eax,eax
  mov al,[GUIComboPos]
  dec eax
  mov byte[GUIComboTextH+eax],0
  ret
.nobacksp
  cmp dh,0
  je .nochar
  cmp dh,13
  je .nochar
  cmp byte[GUIComboPos],19
  je .nochar
  xor eax,eax
  mov byte[GUICCFlash],0
  mov al,[GUIComboPos]
  mov [GUIComboTextH+eax],dh
.nochar
  ret

GUICheatKeys:
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
  jmp CheatCodeSave
.nosavecheat
  cmp dh,'L'
  jne .noloadcheat
  jmp CheatCodeLoad
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
CSOverValue resb 1
CSCurValue  resd 1
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

GUICheatSearchKeys:
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
  jmp AddCSCheatCode
.noenter
  call InsertSearchDescription
  ret
.nosecondbox
  ret

; Allows you to use the arrow keys to select a state number, and Enter to pick
GUIStateSelKeys:
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
  pushad
  call zst_name
  popad
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
  pushad
  call zst_name
  popad
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
  pushad
  call zst_name
  popad
  ret
.nopgdn
.nochange2
  ret

; Allows you to select boxes with the arrow keys, Enter to choose
GUIResetKeys:
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
  jmp GUIProcReset
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
GUIStateKeys:
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
  jmp GUIProcStates
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

GUILoadKeys:
  cmp dx,0
  jne near .keypressed
.nokeypressed
  ret
.keypressed
  mov [gui_key],dh
  mov [gui_key_extended],dl
  pushad
  call GUILoadKeysNavigate
  cmp eax,1
  popad
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
  pushad
  call GUILoadKeysJumpTo
  popad
  ret

WaitForKey:
.again3
  mov edx,10
.again4
  mov cx,1000
  push edx
  call delay
  call JoyRead
;    mov dword[pressed+0CCh],0    ; Up,Down,Left,Right, pl1
;    mov dword[pressed+0E8h],0    ; Up,Down,Left,Right, pl2
;    mov dword[pressed+14Ch],0    ; Up,Down,Left,Right, pl1
;    mov dword[pressed+168h],0    ; Up,Down,Left,Right, pl2
;    mov dword[pressed+080h],0    ; Button 7
;    mov dword[pressed+081h],0    ; Button 8
;    mov dword[pressed+086h],0    ; Button 5
;    mov dword[pressed+087h],0    ; Button 6
;    mov dword[pressed+100h],0    ; Button 7
;    mov dword[pressed+101h],0    ; Button 8
;    mov dword[pressed+106h],0    ; Button 5
;    mov dword[pressed+107h],0    ; Button 6
  pop edx
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
  call JoyRead
;    mov dword[pressed+0CCh],0    ; Up,Down,Left,Right, pl1
;    mov dword[pressed+0E8h],0    ; Up,Down,Left,Right, pl2
;    mov dword[pressed+14Ch],0    ; Up,Down,Left,Right, pl1
;    mov dword[pressed+168h],0    ; Up,Down,Left,Right, pl2
;    mov dword[pressed+080h],0    ; Button 7
;    mov dword[pressed+081h],0    ; Button 8
;    mov dword[pressed+086h],0    ; Button 5
;    mov dword[pressed+087h],0    ; Button 6
;    mov dword[pressed+100h],0    ; Button 7
;    mov dword[pressed+101h],0    ; Button 8
;    mov dword[pressed+106h],0    ; Button 5
;    mov dword[pressed+107h],0    ; Button 6
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
  cmp byte[MouseDis],1
  je .mousedis2
  push ebx
;    mov eax,0Bh
;    int 33h
  pop ebx
.mousedis2
  mov byte[GUIpclicked],1
  ret
