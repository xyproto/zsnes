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


%ifdef __WIN32__
EXTSYM CheckPriority
EXTSYM CheckAlwaysOnTop
EXTSYM CheckScreenSaver
EXTSYM MinimizeWindow
%endif

lastmouseholded db 0
mousebuttonstat dw 0
MouseMoveOkay   db 0
LastHoldEnable  db 0

ProcessMouse:
    ; Process holds
    mov al,[GUIHold]
    cmp al,[LastHoldEnable]
    je near .noholdnecessary
    cmp byte[GUIHold],0
    jne .noholda
    mov ecx,0
    mov edx,255
    call Set_MouseXMax
    mov ecx,0
    mov edx,223
    call Set_MouseYMax
.noholda
    cmp byte[GUIHold],1
    jne .nohold1
    mov ecx,0
    mov edx,255
    call Set_MouseXMax
    mov ecx,16
    mov edx,223
    call Set_MouseYMax
.nohold1
    cmp byte[GUIHold],5
    je .dohold5
    cmp byte[GUIHold],2
    jne .nohold2m
.dohold5
    mov ecx,[GUIHoldXlimL]
    mov edx,[GUIHoldXlimR]
    call Set_MouseXMax
    mov ecx,[GUIHoldYlim]
    mov edx,[GUIHoldYlim]
    call Set_MouseYMax
.nohold2m
    cmp byte[GUIHold],3
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
    cmp word[GUImouseposx],cx
    je .zero2
    mov byte[MouseMoveOkay],1
.zero2
    cmp word[GUImouseposy],dx
    je .zero3
    mov byte[MouseMoveOkay],1
.zero3
    mov word[GUImouseposx],cx
    test word[GUImouseposx],8000h
    jz .nowrapleft
    mov word[GUImouseposx],0
.nowrapleft
    cmp word[GUImouseposx],255
    jbe .nowrapright
    mov word[GUImouseposx],255
.nowrapright
    mov word[GUImouseposy],dx
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
    mov byte[GUIcrowpos],al
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
    %ifdef __LINUX__
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

;    cmp byte[OSPort],3
;    jne .noclick
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
    je .hold2
    cmp byte[GUIHold],3
    je near .hold3
    cmp byte[GUIHold],4
    je near .hold4
    cmp byte[GUIHold],5
    je near .hold5
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
    ; Slide Bar Hold
    mov eax,[GUIHoldYlim]
    mov [GUImouseposy],eax
    mov ebx,[GUImouseposx]
    mov eax,[GUIHoldXlimL]
    cmp ebx,eax
    jnl .noless
    mov [GUImouseposx],eax
.noless
    mov eax,[GUIHoldXlimR]
    cmp ebx,eax
    jng .nogreat
    mov [GUImouseposx],eax
.nogreat
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
    jl near .nobhold
    cmp edx,[GUIHoldYlim]
    jl near .nobhold
    cmp eax,[GUIHoldXlimR]
    jg near .nobhold
    cmp edx,[GUIHoldYlimR]
    jg near .nobhold
    mov al,[GUICBHold2]
    mov [GUICBHold],al
    ret
.nobhold
    mov byte[GUICBHold],0
    ret
.hold5
    ; Slide Bar Hold
    mov eax,[GUIHoldYlim]
    mov [GUImouseposy],eax
    mov ebx,[GUImouseposx]
    mov eax,[GUIHoldXlimL]
    cmp ebx,eax
    jnl .nolessa
    mov [GUImouseposx],eax
.nolessa
    mov eax,[GUIHoldXlimR]
    cmp ebx,eax
    jng .nogreata
    mov [GUImouseposx],eax
.nogreata
    mov byte[lastmouseholded],1
    jmp DisplayGUISoundClick
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
    mov cx,word[GUImouseposy]
    sub cx,[GUIHoldy]
    add cx,[GUIHoldym]
    movsx ecx,cx
    xor eax,eax
    mov al,[GUIwinptr]
    dec eax
    xor ebx,ebx
    mov bl,[GUIwinorder+eax]
    mov [GUIwinposy+ebx*4],ecx
    mov cx,word[GUImouseposx]
    sub cx,[GUIHoldx]
    add cx,[GUIHoldxm]
    movsx ecx,cx
    mov [GUIwinposx+ebx*4],ecx
    ret
.notclicked
    mov byte[GUICHold],0
    mov byte[GUIpclicked],0
    mov byte[GUIHold],0

;    cmp byte[OSPort],3
;    jne .notwinpressa

%ifdef __LINUX__
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

;    cmp byte[OSPort],3
;    jne .notwinpressb
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

    ;cmp byte[OSPort],3
    ;jne .notwinpressc
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
    cmp byte[GUICBHold],4
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
    cmp byte[GUICBHold],16
    je near MoviePlay
    cmp byte[GUICBHold],17
    je near MovieRecord
    cmp byte[GUICBHold],18
    je near MovieStop
    cmp byte[GUICBHold],19
    je near MovieRecord
    cmp byte[GUICBHold],20
    je near SkipMovie
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
    cmp byte[GUICBHold],21
    jne .nomodem1
    mov byte[CNetType],3
.nomodem1
    cmp byte[GUICBHold],22
    jne .nomodem2
    mov byte[CNetType],10
    mov byte[ModemProcess],0
.nomodem2
    cmp byte[GUICBHold],23
    jne .nomodem3
    mov byte[CNetType],2
.nomodem3
    cmp byte[GUICBHold],24
    jne .nomodem4
    mov byte[CNetType],12
    mov byte[ModemProcess],0
.nomodem4
    cmp byte[GUICBHold],25
    jne .nomodem4b
    mov byte[CNetType],15
    mov byte[ModemProcess],40
    mov byte[WhichRemote],4
.nomodem4b
    cmp byte[GUICBHold],26
    jne .nomodem4c
    mov byte[CNetType],15
    mov byte[ModemProcess],41
    mov byte[WhichRemote],4
.nomodem4c
    cmp byte[GUICBHold],27
    jne .noinit
    cmp byte[CNetType],20
    jne near .nostat20b2
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
    cmp byte[NetQuitAfter],0
    je .nostat20b2
    mov byte[GUIQuit],1
.nostat20b2
    call RemoteDeInit
.noinit
    cmp byte[GUICBHold],28
    jne .nomodem5
    mov byte[CNetType],11
    mov byte[ModemProcess],0
.nomodem5
    cmp byte[GUICBHold],34
    jne .noipx1
    mov byte[WhichRemote],2
    mov byte[ModemProcess],30
    mov byte[CNetType],15
.noipx1
    cmp byte[GUICBHold],64
    jne .nostatecancel
    mov byte[NetStateQuit],1
.nostatecancel
    cmp byte[GUICBHold],65
    jne .nohomepage
    call GotoHomepage
.nohomepage
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
    mov al,[GUIcurrentvideocursloc]
;    cmp byte[OSPort],2
;    jae .yes16b
%ifdef __MSDOS__
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
%endif
.yes16b
;    mov byte[newengen],0
;    mov byte[cfgnewgfx],0
;    jmp .skipno16b
.no16b
    mov byte[newengen],1
    mov byte[cfgnewgfx],1
.skipno16b
    mov byte[GUICBHold],0
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
    mov al,[cvidmode]
    mov [cfgcvidmode],al
    call guipostvideo
    cmp dword[GUIkeydelay],0
    je .failed2
    mov dword[GUIkeydelay],0
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIWFVID+eax],1
    je .fullsc
    mov [PrevWinMode],al
    ret
.fullsc
    mov [PrevFSMode],al
    ret
.failed
    mov byte[videotroub],0
    mov al,[.prevvid]
    mov [cvidmode],al
    mov [cfgcvidmode],al
    call initvideo
    call GUISetPal
    call guipostvideofail
    mov dword[GUIkeydelay],0
    ret
.failed2
    mov byte[videotroub],0
    mov al,[.prevvid]
    mov [cvidmode],al
    mov [cfgcvidmode],al
    call initvideo
    call GUISetPal
    mov dword[GUIkeydelay],0
    ret
.prevvid db 0

NEWSYM SwitchFullScreen
    pushad
    mov ecx,255*296
    mov eax,[vidbufferofsb]
.loop
    mov dword[eax],0FEFEFEFEh
    add eax,4
    loop .loop

    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIWFVID+eax],0
    je .fullsc
    mov al,[PrevWinMode]
    mov [cvidmode],al
    mov [cfgcvidmode],al
    call initvideo
    popad
    ret
.fullsc
    mov al,[PrevFSMode]
    mov [cvidmode],al
    mov [cfgcvidmode],al
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
    ret

%macro GUIClickCButton 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
%%noclick
%endmacro

%macro GUIClickCButtonat 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
    call guimustrestartmsg
%%noclick
%endmacro

%macro GUIClickCButtonC 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
    mov dword[GUIccombviewloc],0
    mov dword[GUIccombcursloc],0
    mov dword[GUIccomblcursloc],0
%%noclick
%endmacro

%macro GUIClickCButtonN 4
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    mov %3,%4
%%noclick
%endmacro

%macro GUIClickCButtonO 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    push eax
    xor %3,1
    mov al,%3
    mov [bgfixer],al
    pop eax
%%noclick
%endmacro

%macro GUIClickCButtonS 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    push eax
    xor %3,1
    call initrevst
    mov al,[DSPMem+0Ch]
    call WDSPReg0C
    mov al,[DSPMem+1Ch]
    call WDSPReg1C
    mov al,[DSPMem+2Ch]
    call WDSPReg2C
    mov al,[DSPMem+3Ch]
    call WDSPReg3C
    pop eax
%%noclick
%endmacro

%macro GUIClickCButtonM 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
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
%%noclick
%endmacro

%macro GUIClickCButtonID 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
    call SetDevice
%%noclick
%endmacro

%macro GUIClickCButton2 5
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,%5
    mov al,%3
    mov %4,al
    ret
%%noclick
%endmacro

%macro GUIClickCButton3 5
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
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
    loop %%loop
    pop ecx
    pop eax
    ret
%%noclick
%endmacro

%macro GUIClickCButton5 4
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
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
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
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
    loop %%loop
    pop ecx
    pop eax
%%clear
    ret
%%noclick
%endmacro

%macro GUIClickCButton4 5
    cmp eax,%1+1
    jl near %%noclick
    cmp edx,%2+3
    jl near %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    cmp %3,%5
    jne %%set
    xor %3,%5
    jmp %%done
%%set
    mov %3,%5
%%done
    mov al,%3
    mov %4,al
    cmp byte[En2xSaI],0
    jne %%nomodeq
    call ClearScreen
;    cmp byte[OSPort],2
;    jae %%nomodeq
%ifdef __MSDOS__
    cmp byte[cvidmode],1
    jne %%nomodeq
    mov byte[cbitmode],1
    call initvideo2
    mov byte[cbitmode],0
    call GUISetPal
%endif
%%nomodeq
    ret
%%noclick
%endmacro

%macro GUIClickCButtonfr 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    mov byte[FPSOn],0
    cmp %3,0
    je %%zero
    mov %3,0
    jmp %%notzero
%%zero
    mov %3,2
%%notzero
%%noclick
%endmacro

%macro GUIClickCButtonL 2
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor byte[showallext],1
    xor byte[GUIshowallext],1
    call GetLoadData
    ret
%%noclick
%endmacro

%macro GUIClickCButtonW 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
%%noclick
%ifdef __WIN32__
    pushad
    call CheckPriority
    call CheckScreenSaver
    popad
%endif
%endmacro

%macro GUIClickCButtonI 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,1
    cmp byte[En2xSaI],1
    jnae %%noclick
    call Clear2xSaIBuffer
%%noclick
%endmacro

%macro GUIClickCButtonMMX 3
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+3
    jl %%noclick
    cmp eax,%1+6
    jg %%noclick
    cmp edx,%2+8
    jg %%noclick
    xor %3,2
%%noclick
%endmacro

%macro GUISlidebarPostImpl 13 ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
    cmp dword[%11],0
    je near %%nolower
    cmp dword[GUIdispmode],1
    je near %%slidebar
    cmp eax,%1
    jl near %%noupper
    cmp edx,%2-7
    jl near %%noupper
    cmp eax,%3
    jg near %%noupper
    cmp edx,%2-1
    jg near %%noupper
    mov byte[%12],%13
    mov byte[GUICHold],%7
    jmp %5
%%noupper
    cmp eax,%1
    jl near %%nolower
    cmp edx,%4+1
    jl near %%nolower
    cmp eax,%3
    jg near %%nolower
    cmp edx,%4+7
    jg near %%nolower
    mov byte[%12],%13
    mov byte[GUICHold],%7+1
    jmp %6
%%slidebar
    cmp eax,%1
    jl near %%nolower
    cmp edx,%2
    jl near %%nolower
    cmp eax,%3
    jg near %%nolower
    cmp edx,%4
    jg near %%nolower
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

GUIlastdispval dd 0
GUIdispmode    dd 0
GUIlastypos    dd 0
GUIlastcpos    dd 0
GUIlastvpos    dd 0

%macro GUISlidebarImpl 10 ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
    mov dword[GUIdispmode],0
    cmp eax,%1
    jl near %%nobar
    cmp edx,%2
    jl near %%nobar
    cmp eax,%3
    jg near %%nobar
    cmp edx,%4
    jg near %%nobar
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
    mov eax,dword[%8]
    mov [GUIlastcpos],eax
    mov eax,dword[%7]
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
    cmp eax,%1
    jl near %%noupper
    cmp edx,%2-7
    jl near %%noupper
    cmp eax,%3
    jg near %%noupper
    cmp edx,%2-1
    jg near %%noupper
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
    cmp eax,%1
    jl near %%nolower
    cmp edx,%4+1
    jl near %%nolower
    cmp eax,%3
    jg near %%nolower
    cmp edx,%4+7
    jg near %%nolower
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
   cmp eax,%1
   jl near %%nohold
   cmp edx,%2
   jl near %%nohold
   cmp eax,%3
   jg near %%nohold
   cmp edx,%4
   jg near %%nohold
   mov dword[GUIHoldXlimL],%1
   mov dword[GUIHoldXlimR],%3
   mov dword[GUIHoldYlim],%2
   mov dword[GUIHoldYlimR],%4
   mov byte[GUICBHold2],%5
   mov byte[GUIHold],4
%%nohold
%endmacro

%macro GUIPHoldbutton2 8
   cmp eax,%1
   jl near %%nohold
   cmp edx,%2
   jl near %%nohold
   cmp eax,%3
   jg near %%nohold
   cmp edx,%4
   jg near %%nohold
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
    cmp eax,%1
    jl %%noclick
    cmp edx,%2
    jl %%noclick
    cmp eax,%3
    jg %%noclick
    cmp edx,%4
    jg %%noclick
    mov dword[GUIEditString],%5
    mov dword[GUIEditStringmLen],%6
    mov dword[GUIEditStringLTxt],0
    mov dword[GUIEditStringcWin],1
%%noclick
%endmacro

%macro GUITextBoxInputB 5
    cmp eax,%1
    jl %%noclick
    cmp edx,%2
    jl %%noclick
    cmp eax,%3
    jg %%noclick
    cmp edx,%4
    jg %%noclick
    mov byte[GUINetTextm2],%5
%%noclick
%endmacro

%macro GUIPButtonHole 4
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+1
    jl %%noclick
    cmp eax,%1+7
    jg %%noclick
    cmp edx,%2+7
    jg %%noclick
    mov %3,%4
%%noclick
%endmacro

%macro GUIPButtonHole2 6
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+1
    jl %%noclick
    cmp eax,%1+7
    jg %%noclick
    cmp edx,%2+7
    jg %%noclick
    mov %3,%4
    mov %5,%6
%%noclick
%endmacro

%macro GUIPButtonHoleLoad 4
    cmp eax,%1+1
    jl %%noclick
    cmp edx,%2+1
    jl %%noclick
    cmp eax,%1+7
    jg %%noclick
    cmp edx,%2+7
    jg %%noclick
    mov %3,%4
    call GetLoadData
    ret
%%noclick
%endmacro

%macro GUIWinControl 13 ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
    cmp eax,%1
    jl near %%noload
    cmp edx,%2
    jl near %%noload
    cmp eax,%3
    jg near %%noload
    cmp edx,%4
    jg near %%noload
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
    cmp eax,%1
    jl near %%noload
    cmp edx,%2
    jl near %%noload
    cmp eax,%3
    jg near %%noload
    cmp edx,%4
    jg near %%noload
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

; Mouseimplementation

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
    GUIPHoldbutton 186,167,228,178,1
    GUIPButtonHoleLoad 9,163,byte[GUIloadfntype],0
    GUIPButtonHoleLoad 9,171,byte[GUIloadfntype],1
    GUIPButtonHoleLoad 9,179,byte[GUIloadfntype],2
    ret

DisplayGUIChoseSaveClick:
    GUIPButtonHole 10,28,byte[GUIChoseSaveText2],'T'
    GUIPButtonHole 30,28,byte[GUIChoseSaveText2],'1'
    GUIPButtonHole 50,28,byte[GUIChoseSaveText2],'2'
    GUIPButtonHole 70,28,byte[GUIChoseSaveText2],'3'
    GUIPButtonHole 90,28,byte[GUIChoseSaveText2],'4'
    GUIPButtonHole 10,43,byte[GUIChoseSaveText2],'5'
    GUIPButtonHole 30,43,byte[GUIChoseSaveText2],'6'
    GUIPButtonHole 50,43,byte[GUIChoseSaveText2],'7'
    GUIPButtonHole 70,43,byte[GUIChoseSaveText2],'8'
    GUIPButtonHole 90,43,byte[GUIChoseSaveText2],'9'
    mov ebx,[statefileloc]
    mov al,[GUIChoseSaveText2]
    mov [fnamest+ebx],al
    ret

DisplayGUIMovieClick:
    cmp byte[MovieRecordWinVal],0
    je near .displayboxa
    GUIPHoldbutton 17,55,59,66,19
    GUIPHoldbutton 70,55,112,66,20
    ret
.displayboxa
    GUIPHoldbutton 7,17,49,28,16
    GUIPHoldbutton 7,33,49,44,17
    GUIPHoldbutton 7,49,49,60,18
%ifdef __LINUX__
    GUIPButtonHole 60,25,byte[CMovieExt],'v'
%else
    GUIPButtonHole 60,25,byte[CMovieExt],'V'
%endif
    GUIPButtonHole 80,25,byte[CMovieExt],'1'
    GUIPButtonHole 100,25,byte[CMovieExt],'2'
    GUIPButtonHole 120,25,byte[CMovieExt],'3'
    GUIPButtonHole 60,40,byte[CMovieExt],'4'
    GUIPButtonHole 80,40,byte[CMovieExt],'5'
    GUIPButtonHole 100,40,byte[CMovieExt],'6'
    GUIPButtonHole 120,40,byte[CMovieExt],'7'
    GUIPButtonHole 60,55,byte[CMovieExt],'8'
    GUIPButtonHole 80,55,byte[CMovieExt],'9'
    ret

%macro DGOptnsProcBox 3
    cmp eax,%1
    jl %%skip
    cmp edx,%2
    jl %%skip
    cmp eax,%1+19
    jg %%skip
    cmp edx,%2+6
    jg %%skip
    push edi
    call guipresstest
    pop edi
    cmp ebx,1
    je %%cancelkey
    cmp ebx,3Bh
    je %%cancelkey
    mov %3,ebx
;    cmp byte[OSPort],3
;    jne %%notwin32
%ifndef __MSDOS__
    cmp dword[keycontrolval],0
    je %%notwin32
    push ebx
    mov ebx,[keycontrolval]
    mov byte[ebx],1
    pop ebx
%endif
%%notwin32
    ret
%%cancelkey
    xor ebx,ebx
    mov %3,ebx
    ret
%%skip
%endmacro

%macro DGOptnsProcBoxc 5
    cmp eax,%1
    jl %%skip
    cmp edx,%2
    jl %%skip
    cmp eax,%3
    jg %%skip
    cmp edx,%4
    jg %%skip
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
    cmp eax,%1
    jl near %%skip
    cmp edx,%2
    jl near %%skip
    cmp eax,%1+19
    jg near %%skip
    cmp edx,%2+6
    jg near %%skip
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
    xor ecx,ecx
%%joysloop
    cmp dword[pl1ULk+ecx*4],80h
    jbe %%nojoystick
    mov dword[pl1ULk+ecx*4],0
%%nojoystick
    inc ecx
    cmp ecx,16
    jne %%joysloop
    ret
%%skip
%endmacro

DisplayGUIInputClick2:
    push eax
    mov eax,[NumInputDevices]
    mov dword[GUINumValue],eax
    pop eax
    GUISlidebarPostImpl 109,42,116,69,.scrollup,.scrolldown,9,5,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,GUIBlankVar,1
    GUIPostWinBoxMove 5,35,107,35+5*8,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,.scrollup,.scrolldown,DisplayGUIInputClick.skipscrol,5
DisplayGUIInputClick:
    ; SlideBar Implementation
    push eax
    mov eax,[NumInputDevices]
    mov dword[GUINumValue],eax
    pop eax
    GUISlidebarImpl 109,42,116,69,GUIIStA,5,GUIcurrentinputviewloc,GUIcurrentinputcursloc,GUINumValue,3
.skipscrol
    ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
    GUIWinControl 5,36,107,34+5*8,GUIBlankVar,GUIcurrentinputviewloc,GUINumValue,35,8,GUIcurrentinputcursloc,4,3,0
    GUIPHoldbutton 125,34,155,45,14
;    cmp byte[OSPort],3
;    je near .win32
%ifdef __MSDOS__
    GUIPHoldbutton 125,50,185,61,15
%endif
;.win32
    GUIPHoldbutton 125,66,185,77,40
    mov edi,pl1selk
    mov dword[keycontrolval],pl1contrl
    cmp byte[cplayernum],1
    jne .nopl2
    mov edi,pl2selk
    mov dword[keycontrolval],pl2contrl
.nopl2
    cmp byte[cplayernum],2
    jne .nopl3
    mov edi,pl3selk
    mov dword[keycontrolval],pl3contrl
.nopl3
    cmp byte[cplayernum],3
    jne .nopl4
    mov edi,pl4selk
    mov dword[keycontrolval],pl4contrl
.nopl4
    cmp byte[cplayernum],4
    jne .nopl5
    mov edi,pl5selk
    mov dword[keycontrolval],pl5contrl
.nopl5
    DGOptnsProcBox 45,150,[edi]
    DGOptnsProcBox 45,140,[edi+4]
    DGOptnsProcBox 45,100,[edi+8]
    DGOptnsProcBox 45,110,[edi+12]
    DGOptnsProcBox 45,120,[edi+16]
    DGOptnsProcBox 45,130,[edi+20]
    DGOptnsProcBox 85,120,[edi+24]
    DGOptnsProcBox 85,100,[edi+28]
    DGOptnsProcBox 85,140,[edi+32]
    DGOptnsProcBox 85,130,[edi+36]
    DGOptnsProcBox 85,110,[edi+40]
    DGOptnsProcBox 85,150,[edi+44]

    mov edi,pl1Atk
    cmp byte[cplayernum],1
    jne .nopl2t
    mov edi,pl2Atk
.nopl2t
    cmp byte[cplayernum],2
    jne .nopl3t
    mov edi,pl3Atk
.nopl3t
    cmp byte[cplayernum],3
    jne .nopl4t
    mov edi,pl4Atk
.nopl4t
    cmp byte[cplayernum],4
    jne .nopl5t
    mov edi,pl5Atk
.nopl5t
    DGOptnsProcBox 165,100,[edi]      ; A Turbo
    DGOptnsProcBox 165,110,[edi+4]    ; B Turbo
    DGOptnsProcBox 125,100,[edi+8]    ; X Turbo
    DGOptnsProcBox 125,110,[edi+12]   ; Y Turbo

    mov edi,pl1Ltk
    cmp byte[cplayernum],1
    jne .nopl2tl
    mov edi,pl2Ltk
.nopl2tl
    cmp byte[cplayernum],2
    jne .nopl3tl
    mov edi,pl3Ltk
.nopl3tl
    cmp byte[cplayernum],3
    jne .nopl4tl
    mov edi,pl4Ltk
.nopl4tl
    cmp byte[cplayernum],4
    jne .nopl5tl
    mov edi,pl5Ltk
.nopl5tl
    DGOptnsProcBox 125,120,[edi]      ; L Turbo
    DGOptnsProcBox 165,120,[edi+4]    ; R Turbo

    mov edi,pl1ULk
    cmp byte[cplayernum],1
    jne .nopl2tb
    mov edi,pl2ULk
.nopl2tb
    cmp byte[cplayernum],2
    jne .nopl3tb
    mov edi,pl3ULk
.nopl3tb
    cmp byte[cplayernum],3
    jne .nopl4tb
    mov edi,pl4ULk
.nopl4tb
    cmp byte[cplayernum],4
    jne .nopl5tb
    mov edi,pl5ULk
.nopl5tb
    DGOptnsProcBoxb 165,140,[edi]
    DGOptnsProcBoxb 165,150,[edi+4]
    DGOptnsProcBoxb 125,140,[edi+8]
    DGOptnsProcBoxb 125,150,[edi+12]
%ifdef __MSDOS__
    xor ebx,ebx
    mov bl,[cplayernum]
    GUIClickCButtonID 5,162,byte[pl1p209+ebx]
%endif
    ret


DisplayGUIOptionClick:
    GUIClickCButtonfr 11,26,byte[frameskip]
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUINGVID+ebx],1
    jne near .invalidng
    GUIClickCButton2 11,36,byte[newengen],byte[cfgnewgfx],1
.invalidng
;    GUIClickCButton2 11,46,byte[vsyncon],byte[cfgvsync],1
    GUIClickCButtonMMX 11,56,byte[FPUCopy]
    GUIClickCButtonM 11,66,byte[pl12s34]
%ifdef __MSDOS__
    GUIClickCButton 11,76,byte[SidewinderFix]
%endif
%ifdef __WIN32__
    GUIClickCButtonW 11,76,byte[HighPriority]
%endif
    GUIClickCButton 11,86,byte[FPSAtStart]
    GUIClickCButton 11,96,byte[Turbo30hz]
    GUIClickCButton 11,106,byte[TimerEnable]
    GUIClickCButton 11,116,byte[FastFwdToggle]
    GUIClickCButton 11,126,byte[SRAMSave5Sec]
    GUIClickCButtonO 11,136,byte[OldGfxMode2]
    GUIClickCButton 11,146,byte[LatestSave]
    GUIClickCButton 11,156,byte[AutoState]
    GUIClickCButton 11,166,byte[DontSavePath]
%ifdef __WIN32__
    GUIClickCButtonW 11,176,byte[DisableScreenSaver]
    GUIClickCButtonat 11,186,byte[AlternateTimer]
%endif
    ; Frameskip = 0
    cmp byte[frameskip],0
    je near .autopress
    GUIPHoldbutton2 97,12,105,22,12,byte[frameskip],1,10
    GUIPHoldbutton2 108,12,116,22,13,byte[frameskip],-1,1
    ret
.autopress
    GUIPHoldbutton2 118,12,126,22,12,byte[maxskip],1,9
    GUIPHoldbutton2 129,12,137,22,13,byte[maxskip],-1,0
    ret


DisplayGUIVideoClick2:
    push eax
    mov eax,[NumVideoModes]
    mov dword[GUINumValue],eax
    pop eax
    GUISlidebarPostImpl 117,33,124,62,.scrollup,.scrolldown,5,5,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,GUIBlankVar,1
    GUIPostWinBoxMove 5,27,115,27+5*8,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,.scrollup,.scrolldown,DisplayGUIVideoClick.skipscrol,5
DisplayGUIVideoClick:
    ; SlideBar Implementation
    push eax
    mov eax,[NumVideoModes]
    mov dword[GUINumValue],eax
    pop eax
    GUISlidebarImpl 117,33,124,62,GUIVStA,5,GUIcurrentvideoviewloc,GUIcurrentvideocursloc,GUINumValue,5
.skipscrol
    GUIWinControl 5,28,115,26+5*8,GUIBlankVar,GUIcurrentvideoviewloc,GUINumValue,27,8,GUIcurrentvideocursloc,2,5,0

    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUISLVID+ebx],0
    je near .nocheckboxb
.nocheckboxb
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUITBVID+ebx],0
    je .nocheckboxttb
    GUIClickCButton5 11,163,byte[Triplebufen],1
.nocheckboxttb
    GUIClickCButton 11,183,byte[GrayscaleMode]
%ifndef __LINUX__
    GUIClickCButton2 115,183,byte[vsyncon],byte[cfgvsync],1
%endif
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIM7VID+ebx],0
    je .nocheckboxtm7
    GUIClickCButton5 11,173,byte[Mode7HiRes16b],1
.nocheckboxtm7
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIHSVID+ebx],0
    je near .nohsline
    GUIClickCButton4 21,114,byte[scanlines],byte[cfgscanline],1
    GUIClickCButton4 61,114,byte[scanlines],byte[cfgscanline],3
    GUIClickCButton4 101,114,byte[scanlines],byte[cfgscanline],2
.nohsline
%ifdef __WIN32__
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUITBWVID+ebx],0
    je near .notriplebufferwin
    GUIClickCButton 11,123,byte[TripleBufferWin]
.notriplebufferwin
%endif
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUI2xVID+ebx],0
    je near .no2xSaI
    GUIClickCButton6 11,133,byte[En2xSaI],1
    GUIClickCButton6 11,143,byte[En2xSaI],2
    GUIClickCButton6 11,153,byte[En2xSaI],3
.no2xSaI
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIIEVID+ebx],0
    je .nocheckbox1b
    GUIClickCButton3 11,78,byte[antienab],byte[cfginterp],1
.nocheckbox1b
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIBIFIL+ebx],0
    je .nocheckbox1bf
    GUIClickCButton 11,163,byte[BilinearFilter]
.nocheckbox1bf
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUIFSVID+ebx],0
    je .nocheckbox2b
    GUIClickCButton3 11,88,byte[ScreenScale],byte[GUIScreenScale],1
.nocheckbox2b
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUISSVID+ebx],0
    je .nocheckbox3b
    GUIClickCButton3 11,98,byte[smallscreenon],byte[GUIsmallscreenon],1
.nocheckbox3b
    xor ebx,ebx
    mov bl,[cvidmode]
    cmp byte[GUII2VID+ebx],0
    je .nocheckboxi2
    GUIClickCButton3 11,163,byte[antienab],byte[cfginterp],1
.nocheckboxi2

    GUIPHoldbutton 128,60,164,71,4
    ret

GUINumValue dd 0
GUIBlankVar dd 0

DisplayGUISoundClick:
    mov eax,[GUImouseposx]
    sub eax,[GUIwinposx+6*4]
    mov edx,[GUImouseposy]
    sub edx,[GUIwinposy+6*4]
    GUIClickCButton 11,21,byte[cfgsoundon]
    GUIClickCButton 11,30,byte[cfgStereoSound]
    GUIClickCButton 11,39,byte[RaisePitch]
    GUIClickCButton 11,101,byte[SoundNoiseDis]
    GUIClickCButtonS 11,111,byte[RevStereo]
;    cmp byte[OSPort],3
;    je near .win32sound
%ifdef __MSDOS__
    GUIClickCButton 11,121,byte[Surround]
%endif
;.win32sound
%ifdef __WIN32__
    GUIClickCButton 11,121,byte[LargeSoundBuf]
%endif
    GUIClickCButton 11,131,byte[InterSound]
    GUIClickCButton 11,141,byte[LowPassFilter]
    GUIClickCButton 11,151,byte[SoundBufEn]
    GUIClickCButton 11,161,byte[UseCubicSpline]
    GUIClickCButton 11,171,byte[SPCDisable]
%ifdef __WIN32__
    GUIClickCButton 11,181,byte[ExclusiveSound]
%endif
    cmp eax,15
    jl near .nosrate
    cmp edx,61
    jl near .nosrate
    cmp eax,69
    jg near .nosrate
    cmp edx,69
    jg near .nosrate
    xor eax,eax
    mov al,[cfgSoundQuality]
    mov al,[sampratenext+eax]
    mov [cfgSoundQuality],al
.nosrate
    cmp eax,15
    jl near .nomovebar
    cmp eax,15+100
    jg near .nomovebar
    cmp edx,89
    jl near .nomovebar
    cmp edx,93
    jg near .nomovebar
    sub eax,15
    mov [MusicRelVol],al
    mov [cfgvolume],al
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

sampratenext db 1,4,5,6,2,3,0,0,0,0

%macro GUIBoxVar 6
    cmp eax,%1
    jl %%nope
    cmp edx,%2
    jl %%nope
    cmp eax,%3
    jg %%nope
    cmp edx,%4
    jg %%nope
    mov dword[%5],%6
%%nope
%endmacro

DisplayGUICheatClick2:
    GUISlidebarPostImpl 231,28,238,100,.scrollup,.scrolldown,7,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,GUIBlankVar,1
    GUIPostWinBoxMove 5,22,229,22+12*7,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,.scrollup,.scrolldown,DisplayGUICheatClick.skipscrol,12
DisplayGUICheatClick:
    GUIBoxVar 5,20,181,110,GUIcurrentcheatwin,0
    GUIBoxVar 82,129,173,136,GUIcurrentcheatwin,1
    GUIBoxVar 82,140,196,147,GUIcurrentcheatwin,2
    ; SlideBar Implementation
    GUISlidebarImpl 231,28,238,100,GUICStA,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,7
.skipscrol
    cmp byte[GUIcurrentcheatwin],0
    jne near .nowin
    GUIWinControl 5,23,229,21+12*7,GUIBlankVar,GUIcurrentcheatviewloc,NumCheats,22,7,GUIcurrentcheatcursloc,3,7,30
.nowin
    GUIPHoldbutton 5,113,47,124,5
    GUIPHoldbutton 52,113,94,124,6
    GUIPHoldbutton 99,113,141,124,7
    GUIPHoldbutton 146,113,188,124,8
    GUIPHoldbutton 212,134,236,145,9
    GUIPHoldbutton 193,113,235,124,33

    GUIClickCButton5 11,186,byte[AutoLoadCht],1
    ret

DisplayNetOptnsClick:
    cmp byte[CNetType],1
    jne near .nomodem
    GUIPHoldbutton 5,15,89,26,21
    GUIPHoldbutton 5,30,89,41,22
    GUIPHoldbutton 5,45,89,56,23
    GUIPHoldbutton 5,60,89,71,24
.nomodem
    cmp byte[CNetType],2
    jne near .nomodemc
    GUIPButtonHole2 9,25,byte[ComNum],1,byte[ComIRQ],4
    GUIPButtonHole2 9,35,byte[ComNum],2,byte[ComIRQ],3
    GUIPButtonHole2 9,45,byte[ComNum],3,byte[ComIRQ],4
    GUIPButtonHole2 9,55,byte[ComNum],4,byte[ComIRQ],3
    GUIPButtonHole 94,25,byte[BaudRate],0
    GUIPButtonHole 94,35,byte[BaudRate],1
    GUIPButtonHole 94,45,byte[BaudRate],2
    GUIPButtonHole 94,55,byte[BaudRate],3
    GUIPButtonHole 94,65,byte[BaudRate],4
    GUIPButtonHole 94,75,byte[BaudRate],5
    GUIPButtonHole 94,85,byte[BaudRate],6
    GUIPButtonHole 9,85,byte[GUIDialSt+3],'T'
    GUIPButtonHole 9,95,byte[GUIDialSt+3],'P'
    GUIPHoldbutton2 55,69,63,79,25,byte[ComIRQ],1,15
    GUIPHoldbutton2 66,69,74,79,26,byte[ComIRQ],-1,2
    GUITextBoxInput 21,125,175,133,GUIInitSt1,24
    GUITextBoxInput 21,136,175,144,GUIInitSt2,24
    GUIClickCButton 12,147,byte[FossilUse]
.nomodemc
    cmp byte[CNetType],3
    jne near .nomodemd
    GUIPHoldbutton 141,38,175,49,28
.nomodemd
    cmp byte[CNetType],4
    jne near .notcpip
    cmp byte[NetPlayNoMore],1
    je near .notcpip
    GUIPHoldbutton 5,35,129,46,25
    GUIPHoldbutton 5,70,129,81,26
    GUITextBoxInputB 55,13,95,21,1
    GUITextBoxInputB 26,58,191,66,0
    GUITextBoxInputB 55,23,119,31,2

    GUIClickCButton 5,93,byte[UDPConfig]
.notcpip
    cmp byte[CNetType],10
    je .init
    cmp byte[CNetType],11
    je .init
    cmp byte[CNetType],15
    je .init
    cmp byte[CNetType],12
    jne near .noinit
.init
    GUIPHoldbutton 120,135,181,146,27
.noinit
    cmp byte[CNetType],7
    jne near .noipx
    GUIPHoldbutton 53,48,87,59,34
.noipx
    cmp byte[CNetType],20
    jne near .nochat
    GUIPHoldbutton 111,169,181,180,27
    GUIClickCButtonN 7,135,byte[RemoteCommand],3
    GUIClickCButtonN 32,135,byte[RemoteCommand],4
    GUIClickCButtonN 57,135,byte[RemoteCommand],5
    GUIClickCButtonN 7,145,byte[RemoteCommand],6
    GUIClickCButtonN 32,145,byte[RemoteCommand],7
    GUIClickCButtonN 7,175,byte[RemoteCommand],8

    GUIClickCButtonN 111,135,byte[RemoteCommand],50
    GUIClickCButtonN 111,145,byte[RemoteCommand],41
    GUIClickCButtonN 111,155,byte[RemoteCommand],42
    GUIPHoldbutton2 79,156,87,166,29,byte[Latencytochange],1,19
    GUIPHoldbutton2 90,156,98,166,30,byte[Latencytochange],-1,1
;    DrawSlideBarWin 8,181,15,[GUIcurrentvideoviewloc],[TViewLoc],12,97,GUINCStA
;    GUISlidebarImpl 181,15,188,112,GUINCStA,12,GUIcurrentcheatviewloc,TViewCLoc,TViewLoc,12

    mov al,[Latency]
    cmp byte[Latencytochange],al
    je .nochat
    mov byte[RemoteCommand],20
.nochat

    cmp byte[CNetType],22
    je .yesstate
    cmp byte[CNetType],21
    jne near .nostate
.yesstate
    GUIPHoldbutton 73,48,127,59,64
.nostate
    ret

guipresstest:
    xor ebx,ebx
    mov ecx,256+128+64
.a
    mov byte[pressed+ebx],0
    inc ebx
    loop .a
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
    loop .b
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
    loop .a2
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

guipresstext1 db 'ENTER THE KEY',0
guipresstext2 db 'OR BUTTON TO USE',0
guipresstext3 db '(ESC TO CLEAR)',0

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
    loop .b2

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
    loop .b
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
guipressptr dd 0
guicpressptr dd 0

DisplayGameOptnsClick:
    ; Process boxes
    mov dword[keycontrolval],0
    DGOptnsProcBox 27,24,[KeyBGDisble0]
    DGOptnsProcBox 27+45,24,[KeyBGDisble1]
    DGOptnsProcBox 27+45*2,24,[KeyBGDisble2]
    DGOptnsProcBox 27+45*3,24,[KeyBGDisble3]
    DGOptnsProcBox 27,32,[KeySprDisble]
    DGOptnsProcBox 78,52,[KeyResetAll]
    DGOptnsProcBox 78,60,[KeyExtraEnab]
    DGOptnsProcBox 78,68,[KeyNewGfxSwt]
    DGOptnsProcBox 78,76,[KeyWinDisble]
    DGOptnsProcBox 78,84,[KeyOffsetMSw]
    DGOptnsProcBox 78,92,[KeySlowDown]
    DGOptnsProcBox 178,60,[KeyVolUp]
    DGOptnsProcBox 178,68,[KeyVolDown]
    DGOptnsProcBox 178,76,[KeyFRateUp]
    DGOptnsProcBox 178,84,[KeyFRateDown]
;    cmp byte[OSPort],3
;    jne near .notwin32
%ifndef __MSDOS__
    DGOptnsProcBox 178,44,[KeyRewind]
%endif
;.notwin32
    DGOptnsProcBox 178,52,[KeyFastFrwrd]
    DGOptnsProcBox 27,112,[KeyDisableSC0]
    DGOptnsProcBox 27+45,112,[KeyDisableSC1]
    DGOptnsProcBox 27+45*2,112,[KeyDisableSC2]
    DGOptnsProcBox 27+45*3,112,[KeyDisableSC3]
    DGOptnsProcBox 27,120,[KeyDisableSC4]
    DGOptnsProcBox 27+45,120,[KeyDisableSC5]
    DGOptnsProcBox 27+45*2,120,[KeyDisableSC6]
    DGOptnsProcBox 27+45*3,120,[KeyDisableSC7]
    DGOptnsProcBox 27+45*2,132,[KeyStateSlc0]
    DGOptnsProcBox 27+45*3,132,[KeyStateSlc1]
    DGOptnsProcBox 27,140,[KeyStateSlc2]
    DGOptnsProcBox 27+45,140,[KeyStateSlc3]
    DGOptnsProcBox 27+45*2,140,[KeyStateSlc4]
    DGOptnsProcBox 27+45*3,140,[KeyStateSlc5]
    DGOptnsProcBox 27,148,[KeyStateSlc6]
    DGOptnsProcBox 27+45,148,[KeyStateSlc7]
    DGOptnsProcBox 27+45*2,148,[KeyStateSlc8]
    DGOptnsProcBox 27+45*3,148,[KeyStateSlc9]
    DGOptnsProcBox 8+25,158,[KeySaveState]
    DGOptnsProcBox 8+57+25,158,[KeyLoadState]
    DGOptnsProcBox 8+114+25,158,[KeyStateSelct]
    DGOptnsProcBox 8+25,178,[KeyQuickLoad]
    DGOptnsProcBox 8+57+26,178,[KeyQuickRst]
    DGOptnsProcBox 8+114+20,178,[KeyQuickExit]
    DGOptnsProcBox 8+114+78,178,[KeyQuickClock]
    DGOptnsProcBox 8+25,188,[KeyQuickChat]
    DGOptnsProcBox 8+57+45,188,[KeyQuickSnapShot]
%ifdef __WIN32__
    DGOptnsProcBox 8+108+70,188,[KeyQuickMinimize]
%endif
    ret

DisplayGUIOptnsClick:
    mov eax,[GUImouseposx]
    sub eax,[GUIwinposx+10*4]
    mov edx,[GUImouseposy]
    sub edx,[GUIwinposy+10*4]
    GUIClickCButton 12,23,byte[mousewrap]
    GUIClickCButton 12,33,byte[mouseshad]
    GUIClickCButton 12,43,byte[lastcursres]
    GUIClickCButton 12,53,byte[resetposn]
    GUIClickCButton 12,63,byte[GUIClick]
    GUIClickCButton 12,73,byte[JoyPad1Move]
    GUIClickCButton6 12,83,byte[GUIEffect],1
    GUIClickCButton6 12,93,byte[GUIEffect],2
    GUIClickCButton 124,23,byte[GUIEnableTransp]
%ifdef __WIN32__
    GUIClickCButton 12,168,byte[AlwaysOnTop]
    GUIClickCButton 12,178,byte[SaveMainWindowPos]
    GUIClickCButton 12,188,byte[AllowMultipleInst]
    GUIClickCButton 124,33,byte[MouseWheel]
    GUIClickCButton 124,53,byte[TrapMouseCursor]
%endif
    GUIClickCButtonI 124,43,byte[FilteredGUI]
    GUIPButtonHole 48,108,byte[CurPalSelect],0
    GUIPButtonHole 88,108,byte[CurPalSelect],1
    GUIPButtonHole 135,108,byte[CurPalSelect],2

    push eax
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
    cmp eax,25+32*4-1
    jg near .nomovebar
    sub eax,25
    shr eax,2
    cmp edx,122
    jl .nomovebar1
    cmp edx,126
    jg .nomovebar1
    cmp byte[TRVal2],al
    je .nomovebar1b
    mov byte[TRVal2],al
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
    mov byte[TGVal2],al
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
    mov byte[TBVal2],al
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
    add eax,25+32*4-1
    mov [GUIHoldXlimR],eax
.nohold
    ret

DisplayGUIAboutClick:
;    cmp byte[OSPort],3
;    jne near .nowin32
%ifndef __MSDOS__
    GUIPHoldbutton 5,117,74,128,65
%endif
;.nowin32
    ret

%macro ComboBoxProc 5
    cmp eax,%1
    jl %%nope
    cmp edx,%2
    jl %%nope
    cmp eax,%3
    jg %%nope
    cmp edx,%4
    jg %%nope
    cmp byte[GUINumCombo],42
    je %%nope
    xor ebx,ebx
    mov bl,[GUINumCombo]
    mov byte[GUIComboData+ebx],%5
    inc byte[GUINumCombo]
%%nope
%endmacro

%macro ComboBoxProcD 4
    cmp eax,%1
    jl %%nope
    cmp edx,%2
    jl %%nope
    cmp eax,%3
    jg %%nope
    cmp edx,%4
    jg %%nope
    cmp byte[GUINumCombo],0
    je %%nope
    dec byte[GUINumCombo]
%%nope
%endmacro

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

    GUIPButtonHole 178,82,byte[GUIComboPNum],0
    GUIPButtonHole 196,82,byte[GUIComboPNum],1
    GUIPButtonHole 214,82,byte[GUIComboPNum],2
    GUIPButtonHole 232,82,byte[GUIComboPNum],3
    GUIClickCButton 163,88,byte[GUIComboLHorz]
    GUIClickCButtonC 163,96,byte[GUIComboGameSpec]
    GUIPHoldbutton 202,20,246,31,60
    GUIPHoldbutton 202,35,246,46,61
    GUIPHoldbutton 202,50,246,61,62
    GUIPHoldbutton 202,65,246,76,63
    DGOptnsProcBoxc 135,91,157,99,[GUIComboKey]

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
    GUIPHoldbutton 95,140,140,152,50
    GUIPButtonHole 11,28,byte[CheatSrcByteSize],0
    GUIPButtonHole 11,38,byte[CheatSrcByteSize],1
    GUIPButtonHole 11,48,byte[CheatSrcByteSize],2
    GUIPButtonHole 11,58,byte[CheatSrcByteSize],3
    GUIPButtonHole 11,73,byte[CheatSrcByteBase],0
    GUIPButtonHole 11,83,byte[CheatSrcByteBase],1
    GUIPButtonHole 11,113,byte[CheatSrcSearchType],0
    GUIPButtonHole 11,123,byte[CheatSrcSearchType],1
    ret
.incheatmode
    GUIPHoldbutton 30,140,80,152,51
    cmp byte[CheatSearchStatus],1
    jne near .cheatsearching
    jmp .nosearch
.cheatsearching
    cmp byte[CheatSrcSearchType],1
    je near .cheatsearchingcomp
    jmp .searched
.cheatsearchingcomp
    GUIPButtonHole 11,33,byte[CheatCompareValue],0
    GUIPButtonHole 11,43,byte[CheatCompareValue],1
    GUIPButtonHole 11,53,byte[CheatCompareValue],2
    GUIPButtonHole 11,63,byte[CheatCompareValue],3

.searched
    GUIPHoldbutton 120,140,170,152,53
.nosearch
    GUIPHoldbutton 10,140,60,152,51
    GUIPHoldbutton 70,140,110,152,52
    ret

.preview
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
    GUIPHoldbutton 70,140,130,152,54
    GUIPHoldbutton 140,140,180,152,55
    ret
;    DrawSlideBarWin 13,173,28,[GUIcurrentchtsrcviewloc],[NumCheatSrc],12,73,GUICSStA
;    DrawGUIWinBox 13,5,20,171,108,167
.add
    GUIPHoldbutton 60,155,120,167,56
    GUIPHoldbutton 130,155,160,167,57
    GUIClickCButton 8,139,byte[CheatUpperByteOnly]
    GUIBoxVar 10,30,80,37,CurCStextpos,0
    GUIBoxVar 10,55,126,62,CurCStextpos,1
    ret

DisplayGUIAddOnClick:
    GUIPButtonHole 9,15,byte[snesmouse],0
    GUIPButtonHole 9,25,byte[snesmouse],1
    GUIPButtonHole 9,35,byte[snesmouse],2
    GUIPButtonHole 9,45,byte[snesmouse],3
    GUIPButtonHole 9,55,byte[snesmouse],4
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
;    test bl,01h
;    jnz .nomidshad
    shr bl,1
    add bl,189
    mov byte[edi+288*7+5],bl
    jmp .finishadow
.nomidshad

.finishadow
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

