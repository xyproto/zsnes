;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
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
%%noclick
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
    cmp byte[HoldCommand],2
    jne .nohold
    ret
.nohold
    mov byte[UseExtKey],0
    xor ch,ch
    xor cl,cl
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
%ifdef __LINUX___
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

;    cmp byte[OSPort],3
;    jne near .notwinport
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
%ifdef __LINUX__
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

;    cmp byte[OSPort],3
;    jne near .notwinport2

%ifdef __LINUX__
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
    cmp ebx,8
    jne near .nonetwin
    cmp byte[CNetType],12
    jne .noprocess7
    cmp byte[ModemProcess],7
    jne .noprocess7
    cmp dh,13
    je .ret
    cmp dh,32
    jb .noprocess7
.ret
%ifdef __MSDOS__
    mov al,dh
    call ModemSendChar
%endif
    ret
.noprocess7
    cmp dl,27
    jne .nonetwin
    cmp byte[CNetType],2
    je .yesnetwin
    cmp byte[CNetType],3
    je .yesnetwin
    cmp byte[CNetType],10
    je .yesnetwin
    cmp byte[CNetType],11
    je .yesnetwin
    cmp byte[CNetType],12
    je .yesnetwin
    cmp byte[CNetType],15
    je .yesnet15
    jmp .nonetwin
.yesnet15
    mov byte[CNetType],0
%ifdef __MSDOS__
;    cmp byte[OSPort],1
;    je .nodeinitipx
    call deinitipx
.nodeinitipx
%endif
    jmp .closewin
.yesnetwin
%ifdef __MSDOS__
    cmp byte[ModemInitStat],0
    je .nodeinitmodem
    call DeInitModemC
    mov byte[ModemInitStat],0
.nodeinitmodem
%endif
    mov byte[CNetType],1
    ret
.nonetwin
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
    cmp ebx,12
    je near GUIResetKeys
    cmp ebx,13
    je near GUICheatSearchKeys
    cmp ebx,14
    je near GUIStateKeys
    cmp ebx,16
    je near GUIComboKeys
    cmp ebx,8
    jne .notstartmenu
    cmp byte[CNetType],4
    jne .notcpip
    jmp GUITCPIPKeys
.notcpip
    cmp byte[CNetType],1
    jne .notstartmenu
    jmp GUINetMenuKeys1
.notstartmenu
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
    cmp dh,'a'
    jb .nolower
    cmp dh,'z'
    ja .nolower
    sub dh,'a'-'A'
.nolower
    cmp byte[GUIcmenupos],1
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
    cmp dh,'C'
    jne .noc
    mov byte[GUIcrowpos],12
.noc
.noquickmenu
    cmp byte[GUIcmenupos],2
    jne near .noloadmenu
    cmp dh,'L'
    jne .nol
    mov byte[GUIcrowpos],0
.nol
    cmp dh,'R'
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
.noloadmenu
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
.nocheatmenu
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
%ifdef __LINUX__
    cmp al,92
    je .leftpressed
    cmp dword[numlockptr],1
    je  .noleft
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
%ifdef __LINUX__
    cmp al,94
    je  .rightpressed
    cmp dword[numlockptr],1
    je  .noright
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
%ifdef __LINUX__
    cmp al,96
    je  .downpressed
    cmp dword[numlockptr],1
    je  .nodown
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
%ifdef __LINUX__
    cmp al, 90
    je  .uppressed
    cmp dword[numlockptr],1
    je  .noup
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
    mov byte[GUIcrowpos],al
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

GUITCPIPKeys:
;GUINetTextk2 db '  0.  0.  0.  0',0
;GUINetTextl2 db '7845',0
;GUINetTextm2 db 0,0,0
    cmp byte[GUINetTextm2],1
    je near .section2
    mov edi,TCPIPAddress
    mov esi,26
    cmp byte[GUINetTextm2],2
    jne .notsecondstring
    mov edi,ChatNick
    mov esi,9
.notsecondstring
    mov ebx,edi
.next
    cmp byte[ebx],0
    je .done2
    inc ebx
    jmp .next
.done2
    sub ebx,edi
    mov [GUINetTextm2+1],bl
    xor eax,eax
    mov al,[GUINetTextm2+1]
    shl al,2
    add al,2
    cmp dh,32
    jbe .notnum2
    cmp byte[GUINetTextm2],0
    je .notsecondstringb
    cmp dh,'_'
    je .notsecondstringb
    cmp dh,'-'
    je .notsecondstringb
    cmp dh,'^'
    je .notsecondstringb
    cmp dh,'='
    je .notsecondstringb
    cmp dh,'+'
    je .notsecondstringb
    cmp dh,'['
    je .notsecondstringb
    cmp dh,']'
    je .notsecondstringb
    cmp dh,'0'
    jb .notnum2
    cmp dh,'9'
    jbe .notsecondstringb
    cmp dh,'A'
    jb .notnum2
    cmp dh,'Z'
    jbe .notsecondstringb
    cmp dh,'a'
    jb .notnum2
    cmp dh,'z'
    jbe .notsecondstringb
.notsecondstringb
    cmp ebx,esi
    jae near .donesection1
    add ebx,edi
    mov byte[ebx],dh
    jmp .donesection1
.notnum2
    cmp dh,8
    jne .notbacksp
    or ebx,ebx
    jz near .donesection1
    add ebx,edi
    mov byte[ebx-1],0
    jmp .donesection1
.notbacksp
    cmp dh,'c'
    je .yesc
    cmp dh,'C'
    jne .noc
.yesc
;    mov byte[CNetType],15
;    mov byte[ModemProcess],41
;    mov byte[WhichRemote],4
.noc
    cmp dh,'s'
    je .yess
    cmp dh,'S'
    jne .nos
.yess
;    mov byte[CNetType],15
;    mov byte[ModemProcess],40
;    mov byte[WhichRemote],4
.nos
.donesection1
    ret

.section2
    mov ebx,GUINetTextl2
.loopz
    cmp byte[ebx],0
    je .donez
    inc ebx
    jmp .loopz
.donez
    cmp byte[GUINetTextl2],'0'
    jne .notzerob
    cmp byte[GUINetTextl2+1],0
    jne .notzerob
    dec ebx
.notzerob
    cmp dh,'0'
    jb .notnum
    cmp dh,'9'
    ja .notnum
    mov byte[ebx],dh
    mov byte[ebx+1],0
    push ebx
    call .convert
    pop ebx
    cmp ecx,65536
    jb .donenumb
    mov byte[ebx],0
.donenumb
    jmp .donenum
.notnum
    cmp dh,8
    jne .donenum
    cmp byte[GUINetTextl2],0
    je .donenum
    mov byte[ebx-1],0
.donenum
    call .convert
    cmp ecx,0
    jne .notzero
    mov byte[GUINetTextl2],'0'
    mov byte[GUINetTextl2+1],0
.notzero
    mov [TCPIPPortNum],ecx
    ret
.convert
    mov ebx,GUINetTextl2
    xor ecx,ecx
.loop
    cmp byte[ebx],0
    je .done
    mov eax,ecx
    push ebx
    mov ebx,10
    mul ebx
    pop ebx
    mov ecx,eax
    xor eax,eax
    mov al,[ebx]
    sub eax,48
    add ecx,eax
    inc ebx
    jmp .loop
.done
    ret


SECTION .data
NEWSYM TCPIPPortNum, dd 7845
SECTION .text

GUIOptionKeys:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,'A'
    jne .noautofr
    mov byte[FPSOn],0
    cmp byte[frameskip],0
    je .zero
    mov byte[frameskip],0
    jmp .notzero
.zero
    mov byte[frameskip],2
.notzero
.noautofr
    cmp dh,'N'
    jne .nonewgfx
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUINGVID+eax],1
    jne near .invalidng
    xor byte[newengen],1
    mov al,[newengen]
    mov [cfgnewgfx],al
.invalidng
.nonewgfx
    cmp dh,'T'
    jne .noautoincsave
    xor byte[AutoIncSaveSlot],1
    mov byte[firstsaveinc],1
.noautoincsave
    cmp dh,'M'
    jne .nommxcopy
    xor byte[MMXSupport],1
    call MMXCheck
.nommxcopy
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
    cmp dh,'S'
    jne .noswfix
    xor byte[SidewinderFix],1
.noswfix
%ifdef __WIN32__
    cmp dh,'H'
    jne .nohighpriority
    xor byte[HighPriority],1
.nohighpriority
%endif
    cmp dh,'F'
    jne .nofpsatstart
    xor byte[FPSAtStart],1
.nofpsatstart
    cmp dh,'E'
    jne .nouse30hz
    xor byte[Turbo30hz],1
.nouse30hz
    cmp dh,'C'
    jne .nogameclock
    xor byte[TimerEnable],1
.nogameclock
    cmp dh,'G'
    jne .nofastforward
    xor byte[FastFwdToggle],1
.nofastforward
    cmp dh,'R'
    jne .nosramcheck
    xor byte[SRAMSave5Sec],1
.nosramcheck
    cmp dh,'O'
    jne .nooldgfx
    xor byte[OldGfxMode2],1
.nooldgfx
    cmp dh,'L'
    jne .nostartsave
    xor byte[LatestSave],1
.nostartsave
    cmp dh,'/'
    jne .noautostate
    xor byte[AutoState],1
.noautostate
    cmp dh,'D'
    jne .nosavepath
    xor byte[DontSavePath],1
.nosavepath
    cmp dh,'S'
    jne .nosmallmsgtext
    xor byte[SmallMsgText],1
.nosmallmsgtext
%ifdef __WIN32__
    cmp dh,'I'
    jne .noscreensaver
    xor byte[DisableScreenSaver],1
.noscreensaver
%endif
    ret

GUIGUIOptnsKeys:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,'M'
    jne .guioptn1
    xor byte[mousewrap],1
.guioptn1
    cmp dh,'S'
    jne .guioptn2
    xor byte[mouseshad],1
.guioptn2
    cmp dh,'K'
    jne .guioptn3
    xor byte[lastcursres],1
.guioptn3
    cmp dh,'W'
    jne .guioptn4
    xor byte[resetposn],1
.guioptn4
    cmp dh,'C'
    jne .guioptn5
    xor byte[GUIClick],1
.guioptn5
    cmp dh,'U'
    jne .guioptn6
    xor byte[JoyPad1Move],1
.guioptn6
    cmp dh,'N'
    jne .guioptn6b
    GUIkeystoggleoptn byte[GUIEffect],1
.guioptn6b
    cmp dh,'O'
    jne .guioptn7
    GUIkeystoggleoptn byte[GUIEffect],4
.guioptn7
    cmp dh,'E'
    jne .guioptn7b
    GUIkeystoggleoptn byte[GUIEffect],2
.guioptn7b
    cmp dh,'B'
    jne .guioptn8
    GUIkeystoggleoptn byte[GUIEffect],3
.guioptn8
%ifdef __WIN32__
    cmp dh,'A'
    jne .guioptn9
    xor byte[AlwaysOnTop],1
.guioptn9
    cmp dh,'V'
    jne .guioptn10
    xor byte[SaveMainWindowPos],1
.guioptn10
    cmp dh,'L'
    jne .guioptn11
    xor byte[AllowMultipleInst],1
.guioptn11
    cmp dh,'H'
    jne .guioptn12
    xor byte[MouseWheel],1
.guioptn12
    cmp dh,'R'
    jne .guioptn13
    xor byte[TrapMouseCursor],1
.guioptn13
%endif
    cmp dh,'T'
    jne .guioptn14
    xor byte[GUIEnableTransp],1
.guioptn14
    cmp dh,'F'
    jne .guioptn15
    xor byte[FilteredGUI],1
    cmp byte[En2xSaI],1
    jnae .guioptn15
    call ClearScreen
    call Clear2xSaIBuffer
.guioptn15
    ret

GUIVideoKeys:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISLVID+eax],0
    je .nofullscanlines
    cmp dh,'F'
    jne .nofullscanlines
    cmp byte[En2xSaI],0
    jne .nofullscanlines
    cmp byte[hq3xFilter],0
    jne .nofullscanlines
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
    cmp byte[En2xSaI],0
    jne .nohalfscanlines
    cmp byte[hq3xFilter],0
    jne .nohalfscanlines
    call ClearScreen
    GUIkeystoggleoptn byte[scanlines],3
%ifdef __MSDOS__
    ;cmp byte[cvidmode],2
    ;jne .nohalfscanlines
    ;mov byte[cbitmode],1
    ;call initvideo2
    ;mov byte[cbitmode],0
    ;call GUISetPal
%endif
.nohalfscanlines
    cmp dh,'2'
    jne .noquarterscanlines
    cmp byte[En2xSaI],0
    jne .noquarterscanlines
    cmp byte[hq3xFilter],0
    jne .noquarterscanlines
    call ClearScreen
    GUIkeystoggleoptn byte[scanlines],2
%ifdef __MSDOS__
    ;cmp byte[cvidmode],2
    ;jne .noquarterscanlines
    ;mov byte[cbitmode],1
    ;call initvideo2
    ;mov byte[cbitmode],0
    ;call GUISetPal
%endif
.noquarterscanlines
    cmp dh,'Q'
    jne .nohq3xfilter
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIHQ3X+ebx],0
    je .nohq3xfilter
    xor byte[hq3xFilter],1
    jz .nohq3xfilter
    mov byte[scanlines],0
    mov byte[cfgscanline],0
    mov byte[En2xSaI],0
    mov byte[antienab],0
    mov byte[cfginterp],0
.nohq3xfilter
    cmp dh,'X'
    jne .no2xsai
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUI2xVID+ebx],0
    je near .no2xsai
    call Clear2xSaIBuffer
    GUIkeystoggleoptn byte[En2xSaI],1
.no2xsai
    cmp dh,'E'
    jne .nosupereagle
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUI2xVID+ebx],0
    je near .nosupereagle
    call Clear2xSaIBuffer
    GUIkeystoggleoptn byte[En2xSaI],2
.nosupereagle
    cmp dh,'S'
    jne .nosuper2xsai
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUI2xVID+ebx],0
    je near .nosuper2xsai
    call Clear2xSaIBuffer
    GUIkeystoggleoptn byte[En2xSaI],3
.nosuper2xsai
    cmp dh,'I'
    jne .nointerpolationwin
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUII2VID+eax],0
    je .nointerpolationwin
    xor byte[antienab],1
    mov al,[antienab]
    mov [cfginterp],al
.nointerpolationwin
    cmp dh,'I'
    jne .nointerpolation
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIIEVID+eax],0
    je .nointerpolation
    xor byte[antienab],1
    mov al,[antienab]
    mov [cfginterp],al
.nointerpolation
    cmp dh,'U'
    jne .nofullscreen
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIFSVID+eax],0
    je .nofullscreen
    call ClearScreen
    xor byte[ScreenScale],1
    mov al,[ScreenScale]
    mov [GUIScreenScale],al
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
    mov al,[smallscreenon]
    mov [GUIsmallscreenon],al
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
    cmp dh,'H'
    jne .nohiresmode7
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIM7VID+ebx],0
    je .nohiresmode7
    xor byte[Mode7HiRes16b],1
.nohiresmode7
    cmp dh,'B'
    jne .nobilinearfilter
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIBIFIL+ebx],0
    je .nobilinearfilter
    xor byte[BilinearFilter],1
.nobilinearfilter
%ifdef __WIN32__
    cmp dh,'T'
    jne .notriplebufferwin
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUITBWVID+eax],0
    je near .notriplebufferwin
    xor byte[TripleBufferWin],1
;    mov byte[vsyncon],0
;    mov byte[cfgvsync],0
    call initDirectDraw
 .notriplebufferwin
 %endif
%ifdef __MSDOS__
    cmp dh,'T'
    jne .notriplebuffer
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUITBVID+ebx],0
    je .notriplebuffer
    xor byte[Triplebufen],1
;    mov byte[vsyncon],0
;    mov byte[cfgvsync],0
.notriplebuffer
%endif
    cmp dh,'G'
    jne .nograyscale
    xor byte[GrayscaleMode],1
.nograyscale
%ifndef __LINUX__
    cmp dh,'V'
    jne .novsync
    xor byte[vsyncon],1
    mov al,[vsyncon]
    mov [cfgvsync],al
%ifdef __WIN32__
    call initDirectDraw
%endif
%ifdef __MSDOS__
;    mov byte[Triplebufen],0
%endif
.novsync
    cmp dh,'K'
    jne .nokitchensync
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUITBWVID+ebx],0
    je .nokitchensync
    xor byte[KitchenSync],1
.nokitchensync
%endif
    ret

GUISoundKeys:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,'E'
    jne .notenablesound
    xor byte[cfgsoundon],1
%ifdef __WIN32__
    pushad
    call reInitSound
    popad
%endif
.notenablesound
%ifdef __MSDOS__
    cmp dh,'F'
    jne .notforce8b
    xor byte[cfgforce8b],1
.notforce8b
%endif
    cmp dh,'S'
    jne .notstereo
    xor byte[cfgStereoSound],1
.notstereo
%ifdef __MSDOS__
    cmp dh,'A'
    jne .noraisepitch
    xor byte[RaisePitch],1
.noraisepitch
%endif
    cmp dh,'R'
    jne .notsrate
    xor eax,eax
    mov al,[cfgSoundQuality]
    mov al,[.sampratenext+eax]
    mov [cfgSoundQuality],al
.notsrate
    cmp dh,'V'
    jne .notrevst
    xor byte[RevStereo],1
.notrevst
    cmp dh,'U'
    jne .nosurround
    xor byte[Surround],1
.nosurround
    cmp dh,'B'
    jne .nosoundbuf
    xor byte[SoundBufEn],1
.nosoundbuf
    cmp dh,'D'
    jne .nospcdisable
    xor byte[SPCDisable],1
.nospcdisable
    cmp dh,'G'
    jne .nogausseninterp
    GUIkeystoggleoptn byte[SoundInterpType],1
.nogausseninterp
    cmp dh,'C'
    jne .nocubicinterp
    GUIkeystoggleoptn byte[SoundInterpType],2
.nocubicinterp
%ifndef __MSDOS__
    cmp dh,'8'
    jne .no8ptinterp
    cmp byte[MMXSupport],0
    je .no8ptinterp
    GUIkeystoggleoptn byte[SoundInterpType],3
.no8ptinterp
%endif
    cmp dh,'I'
    jne .nosimplefilter
    GUIkeystoggleoptn byte[LowPassFilterType],1
.nosimplefilter
    cmp dh,'Y'
    jne .nodynamicfilter
    GUIkeystoggleoptn byte[LowPassFilterType],2
.nodynamicfilter
%ifndef __MSDOS__
    cmp dh,'H'
    jne .nohiqualityfilter
    cmp byte[MMXSupport],0
    je .nohiqualityfilter
    GUIkeystoggleoptn byte[LowPassFilterType],3
.nohiqualityfilter
%endif
    ret

SECTION .data
.sampratenext db 1,4,5,6,2,3,0,0,0,0
SECTION .text

GUINetMenuKeys1:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,'C'
    jne .notconfig
    mov byte[CNetType],2
.notconfig
    cmp dh,'D'
    jne .notdial
    mov byte[CNetType],3
.notdial
    cmp dh,'M'
    jne .notmanual
    mov byte[CNetType],12
    mov byte[ModemProcess],0
%ifdef __MSDOS__
    cmp byte[ModemInitStat],1
    je .noinit
    call InitModem
    mov byte[ModemInitStat],1
.noinit
%endif
    ret
.notmanual
    cmp dh,'W'
    jne .notwait
    mov byte[CNetType],10
    mov byte[ModemProcess],0
%ifdef __MSDOS__
    cmp byte[ModemInitStat],1
    je .noinit2
    call InitModem
    mov byte[ModemInitStat],1
.noinit2
%endif
    ret
.notwait
    ret

GUIGetInputLine:
    cmp ebx,8
    jne .nodial
    cmp byte[CNetType],20
    jne .nochat
    cmp dh,13
    jne .nochat
    mov byte[RemoteCommand],2
    ret
.nochat
    cmp byte[CNetType],3
    jne .nodial
    cmp dh,13
    jne .nodial
    mov byte[CNetType],11
    mov byte[ModemProcess],0
%ifdef __MSDOS__
    cmp byte[ModemInitStat],1
    je .noinit
    call InitModem
    mov byte[ModemInitStat],1
%endif
.noedit
    ret
.noinit
.nodial
    ; search string for 0, record length of string
    cmp dword[GUIEditString],0
    je .noedit
    mov eax,[GUIEditString]
    xor ecx,ecx
    dec ecx
.next
    mov bl,[eax]
    inc eax
    inc ecx
    cmp bl,0
    jne .next
    cmp byte[GUIEditStringcWin],0
    je near .blinkstuff
    cmp ecx,0
    je .nodel
    cmp dh,8
    jne .nodel
    mov byte[eax-2],0
    dec eax
    jmp .blinkstuff
.nodel
    cmp ecx,[GUIEditStringmLen]
    jne .notend
    jmp .blinkstuff
.notend
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,32
    jb .noinsert
    mov [eax-1],dh
    mov byte[eax],0
    inc eax
.noinsert
    jmp .blinkstuff
.blinkstuff
    dec eax
    mov [GUIEditStringcLen],eax
    ret

SECTION .bss
GUIEditString resd 1              ; Pointer to string
GUIEditStringcLen resd 1          ; Pointer to end of string (done in function)
GUIEditStringmLen resd 1          ; Max length of string
GUIEditStringLTxt resd 1          ; blink value, should be set to 0 when box selected
GUIEditStringLstb resd 1          ; if cursor is placed (set in main function)
GUIEditStringcWin resd 1          ; 0 = no text window selected
SECTION .text

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
    mov byte[GUIComboTextH+eax],dh
.nochar
    ret

GUICheatKeys:
    cmp dword[GUIcurrentcheatwin],1
    je near .textboxa
    cmp dword[GUIcurrentcheatwin],2
    je near .textboxb
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp dh,'R'
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
    cmp dword[NumCheats],0
    jne .nozero
    ret
.nozero
%ifdef __LINUX__
    cmp al,89
    je  .homepressed
    cmp dword[numlockptr],1
    je  .nohome
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
%ifdef __LINUX__
    cmp al,89
    je  .endppressed
    cmp dword[numlockptr],1
    je  .noendp
    cmp al,79
    jne .noendp
.endppressed
%else
    cmp al,79
    jne .noendp
%endif
    mov eax,[NumCheats]
    dec eax
    mov dword[GUIcurrentcheatcursloc],eax
    sub eax,11
    mov dword[GUIcurrentcheatviewloc],eax
    test dword[GUIcurrentcheatviewloc],8000000h
    jz .posve
    mov dword[GUIcurrentcheatviewloc],0
.posve
    ret
.noendp
%ifdef __LINUX__
    cmp al,90
    je  .up
    cmp dword[numlockptr],1
    je  .noup
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
%ifdef __LINUX__
    cmp al, 96
    je  .downpressed
    cmp dword[numlockptr],1
    je  .nodown
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
%ifdef __LINUX__
    cmp al,91
    je  .pageuppressed
    cmp dword[numlockptr],1
    je  .nopageup
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
%ifdef __LINUX__
    cmp al, 97
    je  .pagednpressed
    cmp dword[numlockptr],1
    je  .nopagedown
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
    mov dword[GUIcurrentcheatcursloc],ebx
.poscd
    sub ebx,11
    cmp dword[GUIcurrentcheatviewloc],ebx
    jl .posvd
    test ebx,8000000h
    jz .noneg
    mov ebx,0
.noneg
    mov dword[GUIcurrentcheatviewloc],ebx
.posvd
.nopagedown
    ret

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
    mov al,[GUICheatText9]
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
    mov ah,[GUICheatText9+1]
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
    mov word[GUICheatText9],0
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
    mov byte[GUICheatText9+eax],'_'
    mov byte[GUICheatText9+eax+1],0
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
    mov byte[GUICheatText9+eax],dh
    mov byte[GUICheatText9+eax+1],'_'
    mov byte[GUICheatText9+eax+2],0
    inc byte[GUICheatPosA]
.nochar
    ret

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
    mov byte[GUICheatTextA+eax],'_'
    mov byte[GUICheatTextA+eax+1],0
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
    mov byte[GUICheatTextA+eax],dh
    mov byte[GUICheatTextA+eax+1],'_'
    mov byte[GUICheatTextA+eax+2],0
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
    mov byte[ecx],dh
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
    mov byte[eax],dh
    mov byte[eax+1],0
.nocharb
    ret

GUICheatSearchKeys:
    cmp dh,'a'
    jb .nocap
    cmp dh,'z'
    ja .nocap
    sub dh,'z'-'Z'
.nocap
    cmp byte[CheatWinMode],3
    je near .add
    cmp byte[CheatWinMode],2
    je near .view
    cmp byte[CheatWinMode],1
    je near .search
    CompareKeyMacro '1',CheatSrcByteSize,0
    CompareKeyMacro '2',CheatSrcByteSize,1
    CompareKeyMacro '3',CheatSrcByteSize,2
    CompareKeyMacro '4',CheatSrcByteSize,3
    CompareKeyMacro 'D',CheatSrcByteBase,0
    CompareKeyMacro 'H',CheatSrcByteBase,1
    CompareKeyMacro 'E',CheatSrcSearchType,0
    CompareKeyMacro 'C',CheatSrcSearchType,1
    cmp dh,13
    je .start
    cmp dh,'S'
    jne .notstart
.start
    call CheatCodeSearchInit
    ret
.notstart
    ret
.search
    cmp byte[CheatSrcSearchType],1
    je .compare
    call InsertSearchCharacter
    jmp .nomovedown
.compare
    cmp dh,','
    je .lessthan
    cmp dh,'<'
    jne .notlessthan
.lessthan
    mov byte[CheatCompareValue],0
.notlessthan
    cmp dh,'.'
    je .greaterthan
    cmp dh,'>'
    jne .notgreaterthan
.greaterthan
    mov byte[CheatCompareValue],1
.notgreaterthan
    cmp dh,'='
    je .equalthan
    cmp dh,'+'
    jne .notequalthan
.equalthan
    mov byte[CheatCompareValue],2
.notequalthan
    cmp dh,'N'
    jne .notnotequalthan
    mov byte[CheatCompareValue],3
.notnotequalthan
%ifdef __LINUX__
    cmp al,90
    je  .yesmoveup
    cmp dword[numlockptr],1
    je  .nomoveup
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
%ifdef __LINUX__
    cmp al,96
    je  .yesmovedown
    cmp dword[numlockptr],1
    je  .nomovedown
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
.nomovedown
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
.view
    cmp dword[NumCheatSrc],0
    jne .nozero
    cmp dh,27
    je .yesret
    cmp dh,'R'
    jne .notreturnb
.yesret
    mov byte[CheatWinMode],1
.notreturnb
    ret
.nozero
%ifdef __LINUX__
    cmp al,89
    je  .yeshome
    cmp dword[numlockptr],1
    je  .nohome
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
%ifdef __LINUX__
    cmp al,95
    je  .yesendp
    cmp dword[numlockptr],1
    je  .noendp
    cmp al,79
    jne .noendp
.yesendp
%else
    cmp al,79
    jne .noendp
%endif
    mov eax,[NumCheatSrc]
    dec eax
    mov dword[GUIcurrentchtsrccursloc],eax
    sub eax,11
    mov dword[GUIcurrentchtsrcviewloc],eax
    test dword[GUIcurrentchtsrcviewloc],8000000h
    jz .posve
    mov dword[GUIcurrentchtsrcviewloc],0
.posve
    ret
.noendp
%ifdef __LINUX__
    cmp al,90
    je  .up
    cmp dword[numlockptr],1
    je  .noup
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
%ifdef __LINUX__
    cmp al,96
    je  .yesdown
    cmp dword[numlockptr],1
    je  .nodown
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
%ifdef __LINUX__
    cmp al,91
    je  .yespgup
    cmp dword[numlockptr],1
    je  .nopageup
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
%ifdef __LINUX__
    cmp al,97
    je  .yespgdn
    cmp dword[numlockptr],1
    je  .nopagedown
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
    mov dword[GUIcurrentchtsrccursloc],ebx
.poscd
    sub ebx,11
    cmp dword[GUIcurrentchtsrcviewloc],ebx
    jl .posvd
    test ebx,8000000h
    jz .noneg
    mov ebx,0
.noneg
    mov dword[GUIcurrentchtsrcviewloc],ebx
.posvd
.nopagedown
    cmp dh,27
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

.add
    cmp dh,27
    je .yesretc
    cmp byte[CurCStextpos],1
    je .notreturnc
    cmp dh,'R'
    jne .notreturnc
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
%ifdef __LINUX__
    cmp al,92
    je  .yesleft
    cmp dword[numlockptr],1
    je  .noleft
    cmp al,75
    jne .noleft
.yesleft
%else
    cmp al,75
    jne .noleft
%endif
    mov ebx,[statefileloc]
    mov cl,[fnamest+ebx]
    cmp cl,'t'
    jne .no1st
    mov cl,'4'
    jmp .done
.no1st
    cmp cl,'1'
    jne .no2nd
    mov cl,'t'
    jmp .done
.no2nd
    cmp cl,'5'
    jne .no5th
    mov cl,'9'
    jmp .done
.no5th
    dec cl
    jmp .done
.noleft
%ifdef __LINUX__
    cmp al,94
    je  .yesright
    cmp dword[numlockptr],1
    je  .noright
    cmp al,77
    jne .noright
.yesright
%else
    cmp al,77
    jne .noright
%endif
    mov ebx,[statefileloc]
    mov cl,[fnamest+ebx]
    cmp cl,'t'
    jne .noT
    mov cl,'1'
    jmp .done
.noT
    cmp cl,'4'
    jne .no4th
    mov cl,'t'
    jmp .done
.no4th
    cmp cl,'9'
    jne .no9th
    mov cl,'5'
    jmp .done
.no9th
    inc cl
    jmp .done
.noright
%ifdef __LINUX__
    cmp al,90
    je  .vert
    cmp dword[numlockptr],1
    je  .novert
%endif
    cmp al,72
    je .vert
%ifdef __LINUX__
    cmp al,96
    je  .vert
    cmp dword[numlockptr],1
    je  .novert
%endif
    cmp al,80
    jne .novert
.vert
    mov ebx,[statefileloc]
    mov cl,[fnamest+ebx]
    cmp cl,'t'
    jne .noTv
    mov cl,'5'
    jmp .done
.noTv
    cmp cl,'5'
    jne .no5v
    mov cl,'t'
    jmp .done
.no5v
    cmp cl,'5'
    jb .noabove
    sub cl,5
    jmp .done
.noabove
    add cl,5
    jmp .done
.novert
    ret
.done
    mov [fnamest+ebx],cl
    ret

GUIResetKeys:
%ifdef __LINUX__
    cmp al,92
    je  .yesleft
    cmp dword[numlockptr],1
    je  .noleft
    cmp al,75
    jne .noleft
.yesleft
%else
    cmp al,75
    jne .noleft
%endif
    mov byte[GUICResetPos],0
.noleft
%ifdef __LINUX__
    cmp al,94
    je  .yesright
    cmp dword[numlockptr],1
    je  .noright
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
    cmp dh,'Y'
    je .yesreset
    cmp dh,'y'
    jne .notyesreset
.yesreset
    mov byte[GUICBHold],2
    jmp GUIProcReset
.notyesreset
    cmp dh,'N'
    je .noreset
    cmp dh,'n'
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

GUIStateKeys:
%ifdef __LINUX__
    cmp al,92
    je  .yesleft
    cmp dword[numlockptr],1
    je  .noleft
    cmp al,75
    jne .noleft
.yesleft
%else
    cmp al,75
    jne .noleft
%endif
    mov byte[GUICStatePos],0
.noleft
%ifdef __LINUX__
    cmp al,94
    je  .yesright
    cmp dword[numlockptr],1
    je  .noright
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
    cmp dh,'Y'
    je .yesreset
    cmp dh,'y'
    jne .notyesreset
.yesreset
    mov byte[GUICBHold],10
    jmp GUIProcStates
.notyesreset
    cmp dh,'N'
    je .noreset
    cmp dh,'n'
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
    mov byte[ManualStatus],0
    mov dword[ManualCPtr],GUILoadTextA
    cmp dword[GUIfileentries],0
    je .noright
    cmp dword[GUIdirentries],0
    je .noright
%ifdef __LINUX__
    cmp al,92
    je  .yesleft
    cmp dword[numlockptr],1
    je  .noleft
    cmp al,75
    jne .noleft
.yesleft
%else
    cmp al,75
    jne .noleft
%endif
    mov byte[GUILoadPos],0
    xor dword[GUIcurrentfilewin],1
.noleft
%ifdef __LINUX__
    cmp al,94
    je  .yesright
    cmp dword[numlockptr],1
    je  .noright
    cmp al,77
    jne .noright
.yesright
%else
    cmp al,77
    jne .noright
%endif
    mov byte[GUILoadPos],0
    xor dword[GUIcurrentfilewin],1
.noright
    cmp al,13
    jne .noenter
    mov byte[GUILoadPos],0

    mov ebx,GUILoadTextA
.nextl
    cmp byte[ebx],':'
    je .donel
%ifdef __LINUX__
    cmp byte[ebx],'/'
%else
    cmp byte[ebx],'\'
%endif
    je .donel
    inc ebx
    cmp byte[ebx],0
    je .nofnamel
    jmp .nextl
.donel
    call GUILoadManualDir
    cmp byte[ManualStatus],2
    je near .done
    ret
.nofnamel
    mov ebx,GUILoadTextA
    cmp byte[ebx],0
    je .nodirtry
    mov edx,ebx
    call Change_Single_Dir
    jc .nodirtry
    call GetLoadData.a
    ret
.nodirtry
    call GUILoadData
    ret
.noenter
    cmp dword[GUIcurrentfilewin],1
    je near .dirwin
%ifdef __LINUX__
    cmp al,89
    je  .yeshome
    cmp dword[numlockptr],1
    je  .nohome
    cmp al,71
    jne .nohome
.yeshome
%else
    cmp al,71
    jne .nohome
%endif
    mov byte[GUILoadPos],0
    mov dword[GUIcurrentcursloc],0
    mov dword[GUIcurrentviewloc],0
    ret
.nohome
%ifdef __LINUX__
    cmp al,95
    je  .yesendp
    cmp dword[numlockptr],1
    je  .noendp
    cmp al,79
    jne .noendp
.yesendp
%else
    cmp al,79
    jne .noendp
%endif
    mov byte[GUILoadPos],0
    mov eax,[GUIfileentries]
    dec eax
    mov dword[GUIcurrentcursloc],eax
    sub eax,14
    mov dword[GUIcurrentviewloc],eax
    test dword[GUIcurrentviewloc],8000000h
    jz .posve
    mov dword[GUIcurrentviewloc],0
.posve
    ret
.noendp
%ifdef __LINUX__
    cmp al,90
    je  .up
    cmp dword[numlockptr],1
    je  .noup
    cmp al,72
    jne .noup
%else
    cmp al,72
    jne .noup
%endif
.up
    mov byte[GUILoadPos],0
    cmp dword[GUIcurrentcursloc],0
    je .noup
    mov ebx,[GUIcurrentcursloc]
    cmp [GUIcurrentviewloc],ebx
    jne .noviewdec
    dec dword[GUIcurrentviewloc]
.noviewdec
    dec dword[GUIcurrentcursloc]
.noup
%ifdef __LINUX__
    cmp al,96
    je  .yesdown
    cmp dword[numlockptr],1
    je  .nodown
    cmp al,80
    jne .nodown
.yesdown
%else
    cmp al,80
    jne .nodown
%endif
    mov byte[GUILoadPos],0
    mov ebx,[GUIcurrentcursloc]
    inc ebx
    cmp ebx,[GUIfileentries]
    je .noviewinc
    inc dword[GUIcurrentcursloc]
    sub ebx,15
    cmp ebx,[GUIcurrentviewloc]
    jne .noviewinc
    inc dword[GUIcurrentviewloc]
.noviewinc
.nodown
%ifdef __LINUX__
    cmp al,91
    je  .yespageup
    cmp dword[numlockptr],1
    je  .nopageup
    cmp al,73
    jne .nopageup
.yespageup
%else
    cmp al,73
    jne .nopageup
%endif
    mov byte[GUILoadPos],0
    sub dword[GUIcurrentviewloc],15
    sub dword[GUIcurrentcursloc],15
    test dword[GUIcurrentviewloc],8000000h
    jz .posv
    mov dword[GUIcurrentviewloc],0
.posv
    test dword[GUIcurrentcursloc],8000000h
    jz .posc
    mov dword[GUIcurrentcursloc],0
.posc
.nopageup
%ifdef __LINUX__
    cmp al,97
    je  .yespagedn
    cmp dword[numlockptr],1
    je  .nopagedown
    cmp al,81
    jne .nopagedown
.yespagedn
%else
    cmp al,81
    jne .nopagedown
%endif
    mov byte[GUILoadPos],0
    add dword[GUIcurrentviewloc],15
    add dword[GUIcurrentcursloc],15
    mov ebx,[GUIfileentries]
    dec ebx
    cmp dword[GUIcurrentcursloc],ebx
    jb .poscd
    mov dword[GUIcurrentcursloc],ebx
.poscd
    sub ebx,14
    cmp dword[GUIcurrentviewloc],ebx
    jl .posvd
    test ebx,8000000h
    jz .noneg
    mov ebx,0
.noneg
    mov dword[GUIcurrentviewloc],ebx
.posvd
.nopagedown

.textboxa
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
    cmp dh,0
    je near .posvl2
    cmp dh,13
    je near .posvl2
    cmp dh,8
    je near .posvl2
    cmp dh,9
    je near .posvl2
    cmp byte[GUILoadPos],36
    je near .posvl2
    mov byte[GUILDFlash],0
    xor eax,eax
    mov al,[GUILoadPos]
    mov byte[GUILoadTextA+eax],dh
    inc byte[GUILoadPos]
    ; Go to closest matching filename if there are no :, /, or \ in the filename
    mov ebx,[ManualCPtr]
    cmp byte[ebx],0
    je near .posvl2
.next
    cmp byte[ebx],':'
    je near .posvl2
    cmp byte[ebx],'/'
    je near .posvl2
    cmp byte[ebx],'\'
    je near .posvl2
    inc ebx
    cmp byte[ebx],0
    je .done
    jmp .next
.done

    mov ebx,[GUIfileentries]
    xor esi,esi
    cmp dword[GUIcurrentfilewin],1
    jne near .notdir
    mov esi,[GUIfileentries]
    mov ebx,[GUIfileentries]
    add ebx,[GUIdirentries]
.notdir
    mov ecx,esi
    mov dword[.foundval],esi
    mov dword[.maxfound],0
.loop2
    mov dword[.numfound],0
    cmp byte[GUIloadfntype],0
    jne .nottype02
    mov eax,[spcRamcmp+esi*4]
    jmp .type02
.nottype02
    mov eax,esi
    shl eax,5
    add eax,[spcBuffera]
.type02
    inc eax
    mov edi,[ManualCPtr]
    cmp byte[eax],'['
    jne .nodrive
    inc eax
.nodrive
.loop3
    mov dl,[eax]
    cmp dl,'a'
    jb .nolowerb2
    cmp dl,'z'
    ja .nolowerb2
    sub dl,'a'-'A'
.nolowerb2
    mov dh,[edi]
    cmp dh,'a'
    jb .nolowerb3
    cmp dh,'z'
    ja .nolowerb3
    sub dh,'a'-'A'
.nolowerb3
    cmp dl,dh
    jne .notfound
    cmp byte[ManualStatus],2
    jne .notfullload
    or dh,dh
    jnz .notfullload
    or dl,dl
    jnz .notfullload
    mov byte[ManualStatus],3
.notfullload
    or dh,dh
    jz .notfound
    or dl,dl
    jz .notfound
    inc dword[.numfound]
    inc eax
    inc edi
    jmp .loop3
.notfound
    mov edx,[.numfound]
    cmp edx,[.maxfound]
    jbe .nfound
    mov [.maxfound],edx
    mov [.foundval],esi
.nfound
    inc esi
    cmp esi,ebx
    jne .noend2
    mov esi,[GUIfileentries]
    cmp dword[GUIcurrentfilewin],1
    je .noend2
    xor esi,esi
.noend2
    cmp esi,ecx
    jne near .loop2
.skipall
    mov esi,[.foundval]
    cmp dword[GUIcurrentfilewin],1
    jne .notdir2
    sub esi,[GUIfileentries]
    mov [GUIcurrentdircursloc],esi
    mov [GUIcurrentdirviewloc],esi
    mov ebx,[GUIdirentries]
    sub ebx,15
    cmp dword[GUIcurrentdirviewloc],ebx
    jb .posvl2
    mov dword[GUIcurrentdirviewloc],ebx
    jmp .posvl2
.notdir2
    mov [GUIcurrentcursloc],esi
    mov [GUIcurrentviewloc],esi
    mov ebx,[GUIfileentries]
    sub ebx,15
    cmp dword[GUIcurrentviewloc],ebx
    jb .posvl2
    mov dword[GUIcurrentviewloc],ebx
.posvl2
    cmp byte[ManualStatus],3
    jne .notdirectload
    call GUILoadData
.notdirectload
    ret
SECTION .bss
.foundval resd 1
.numfound resd 1
.maxfound resd 1
SECTION .text

.dirwin
%ifdef __LINUX__
    cmp al,89
    je  .yeshome2
    cmp dword[numlockptr],1
    je  .nohome2
    cmp al,71
    jne .nohome2
.yeshome2
%else
    cmp al,71
    jne .nohome2
%endif
    mov byte[GUILoadPos],0
    mov dword[GUIcurrentdircursloc],0
    mov dword[GUIcurrentdirviewloc],0
    ret
.nohome2
%ifdef __LINUX__
    cmp al,95
    je  .yesendp2
    cmp dword[numlockptr],1
    je  .noendp2
    cmp al,79
    jne .noendp2
.yesendp2
%else
    cmp al,79
    jne .noendp2
%endif
    mov byte[GUILoadPos],0
    mov eax,[GUIdirentries]
    dec eax
    mov dword[GUIcurrentdircursloc],eax
    sub eax,14
    mov dword[GUIcurrentdirviewloc],eax
    test dword[GUIcurrentdirviewloc],8000000h
    jz .posve2
    mov dword[GUIcurrentdirviewloc],0
.posve2
    ret
.noendp2
%ifdef __LINUX__
    cmp al,90
    je  .yesup2
    cmp dword[numlockptr],1
    je  .noup2
    cmp al,72
    jne .noup2
.yesup2
%else
    cmp al,72
    jne .noup2
%endif
    mov byte[GUILoadPos],0
    cmp dword[GUIcurrentdircursloc],0
    je .noup2
    mov ebx,[GUIcurrentdircursloc]
    cmp [GUIcurrentdirviewloc],ebx
    jne .noviewdec2
    dec dword[GUIcurrentdirviewloc]
.noviewdec2
    dec dword[GUIcurrentdircursloc]
.noup2
%ifdef __LINUX__
    cmp al,96
    je  .yesdown2
    cmp dword[numlockptr],1
    je  .nodown2
    cmp al,80
    jne .nodown2
.yesdown2
%else
    cmp al,80
    jne .nodown2
%endif
    mov byte[GUILoadPos],0
    mov ebx,[GUIcurrentdircursloc]
    inc ebx
    cmp ebx,[GUIdirentries]
    je .noviewinc2
    inc dword[GUIcurrentdircursloc]
    sub ebx,15
    cmp ebx,[GUIcurrentdirviewloc]
    jne .noviewinc2
    inc dword[GUIcurrentdirviewloc]
.noviewinc2
.nodown2
%ifdef __LINUX__
    cmp al,91
    je  .yespgup2
    cmp dword[numlockptr],1
    je  .nopageup2
    cmp al,73
    jne .nopageup2
.yespgup2
%else
    cmp al,73
    jne .nopageup2
%endif
    mov byte[GUILoadPos],0
    sub dword[GUIcurrentdirviewloc],15
    sub dword[GUIcurrentdircursloc],15
    test dword[GUIcurrentdirviewloc],8000000h
    jz .posv2
    mov dword[GUIcurrentdirviewloc],0
.posv2
    test dword[GUIcurrentdircursloc],8000000h
    jz .posc2
    mov dword[GUIcurrentdircursloc],0
.posc2
.nopageup2
%ifdef __LINUX__
    cmp al,97
    je  .yespgdn2
    cmp dword[numlockptr],1
    je  .nopagedown2
    cmp al,81
    jne .nopagedown2
.yespgdn2
%else
    cmp al,81
    jne .nopagedown2
%endif
    mov byte[GUILoadPos],0
    add dword[GUIcurrentdirviewloc],15
    add dword[GUIcurrentdircursloc],15
    mov ebx,[GUIdirentries]
    dec ebx
    cmp dword[GUIcurrentdircursloc],ebx
    jb .poscd2
    mov dword[GUIcurrentdircursloc],ebx
.poscd2
    sub ebx,14
    cmp dword[GUIcurrentdirviewloc],ebx
    jb .posvd2
    test ebx,8000000h
    jz .noneg2
    mov ebx,0
.noneg2
    mov dword[GUIcurrentdirviewloc],ebx
.posvd2
.nopagedown2
    jmp .textboxa

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
