;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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

SECTION .text

CalibrateDispA:
    xor ebx,ebx
    mov ecx,256
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a
    call GUIUnBuffer
    call DisplayBoxes
    call DisplayMenu
    GUIBox 75,103,192,135,160
    GUIBox 75,103,192,103,162
    GUIBox 75,103,75,135,161
    GUIBox 192,103,192,135,159
    GUIBox 75,135,192,135,158
    GUIOuttext 81,109,guicalberror1,220-15
    GUIOuttext 80,108,guicalberror1,220
    GUIOuttext 81,117,guicalberror2,220-15
    GUIOuttext 80,116,guicalberror2,220
    GUIOuttext 81,125,guicalberror3,220-15
    GUIOuttext 80,124,guicalberror3,220
    call vidpastecopyscr
    call WaitForKey
    ret

CalibrateDispB:
    xor ebx,ebx
    mov ecx,256
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a
    call GUIUnBuffer
    call DisplayBoxes
    call DisplayMenu
    GUIBox 75,103,192,143,160
    GUIBox 75,103,192,103,162
    GUIBox 75,103,75,143,161
    GUIBox 192,103,192,143,159
    GUIBox 75,143,192,143,158
    GUIOuttext 81,109,guicalberror4,220-15
    GUIOuttext 80,108,guicalberror4,220
    GUIOuttext 81,117,guicalberror5,220-15
    GUIOuttext 80,116,guicalberror5,220
    GUIOuttext 81,125,guicalberror6,220-15
    GUIOuttext 80,124,guicalberror6,220
    GUIOuttext 81,133,guicalberror7,220-15
    GUIOuttext 80,132,guicalberror7,220
    call vidpastecopyscr
    call WaitForKey
    ret

SECTION .data
guicalberror1 db 'PRESS THE TOP LEFT',0
guicalberror2 db 'CORNER AND PRESS A',0
guicalberror3 db 'BUTTON OR KEY',0
guicalberror4 db 'PRESS THE BOTTOM',0
guicalberror5 db 'RIGHT CORNER AND',0
guicalberror6 db 'PRESS A BUTTON OR',0
guicalberror7 db 'KEY',0
SECTION .text

%macro ConfigureKey2 2
  cmp ebx,0
  jne near %%no0
  mov dword[%2],%1upk    ; Up
%%no0
  cmp ebx,1
  jne near %%no1
  mov dword[%2],%1downk  ; Down
%%no1
  cmp ebx,2
  jne near %%no2
  mov dword[%2],%1leftk  ; Left
%%no2
  cmp ebx,3
  jne near %%no3
  mov dword[%2],%1rightk ; Right
%%no3
  cmp ebx,4
  jne near %%no4
  mov dword[%2],%1startk ; Start
%%no4
  cmp ebx,5
  jne near %%no5
  mov dword[%2],%1selk   ; Select
%%no5
  cmp ebx,6
  jne near %%no6
  mov dword[%2],%1Ak     ; A
%%no6
  cmp ebx,7
  jne near %%no7
  mov dword[%2],%1Bk     ; B
%%no7
  cmp ebx,8
  jne near %%no8
  mov dword[%2],%1Xk     ; X
%%no8
  cmp ebx,9
  jne near %%no9
  mov dword[%2],%1Yk     ; Y
%%no9
  cmp ebx,10
  jne near %%no10
  mov dword[%2],%1Lk     ; L
%%no10
  cmp ebx,11
  jne near %%no11
  mov dword[%2],%1Rk     ; R
%%no11
%endmacro

%macro ConfigureKey 2
  cmp byte[%2],0
  jne near %%nopl1
  ConfigureKey2 pl1,%1
%%nopl1
  cmp byte[%2],1
  jne near %%nopl2
  ConfigureKey2 pl2,%1
%%nopl2
  cmp byte[%2],2
  jne near %%nopl3
  ConfigureKey2 pl3,%1
%%nopl3
  cmp byte[%2],3
  jne near %%nopl4
  ConfigureKey2 pl4,%1
%%nopl4
  cmp byte[%2],4
  jne near %%nopl5
  ConfigureKey2 pl5,%1
%%nopl5
%endmacro

SetAllKeys:
    xor ebx,ebx
    mov ecx,256+128+64
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a

    mov byte[GUICBHold],0
    mov dword[keycontrolval],pl1contrl
    cmp byte[cplayernum],1
    jne .nopl2
    mov dword[keycontrolval],pl2contrl
.nopl2
    cmp byte[cplayernum],2
    jne .nopl3
    mov dword[keycontrolval],pl3contrl
.nopl3
    cmp byte[cplayernum],3
    jne .nopl4
    mov dword[keycontrolval],pl4contrl
.nopl4
    cmp byte[cplayernum],4
    jne .nopl5
    mov dword[keycontrolval],pl5contrl
.nopl5

    ;Check if controller is set
    push eax
    mov eax,[keycontrolval]
    cmp dword[eax],0
    pop eax
    jne .noLockInputChange
    ret
.noLockInputChange
    xor ebx,ebx
    mov ecx,12
    mov dword[guipressptr],guipresstext4b
.loop
    ConfigureKey guicpressptr,cplayernum
    push ebx
    push ecx
    call guipresstestb
    pop ecx
    pop ebx
    add dword[guipressptr],21
    inc ebx
    dec ecx
    jnz .loop
    ret
SECTION .data
SECTION .text

CalibrateDev1:
    movzx eax,byte[cplayernum]
    mov ebx,[GUIInputRefP+eax*4]
    mov bl,[ebx]
    mov byte[GUICBHold],0
%ifdef __MSDOS__
    cmp al,0
    jne .notpl1
    cmp byte[pl1p209],0
    jne near .port209
.notpl1
    cmp al,1
    jne .notpl2
    cmp byte[pl2p209],0
    jne near .port209
.notpl2
    cmp al,2
    jne .notpl3
    cmp byte[pl3p209],0
    jne near .port209
.notpl3
    cmp al,3
    jne .notpl4
    cmp byte[pl4p209],0
    jne near .port209
.notpl4
    cmp al,4
    jne .notpl5
    cmp byte[pl5p209],0
    jne near .port209
.notpl5
%endif
    cmp bl,1
    jbe .nojoy
    cmp bl,18
    je near .6button
    cmp bl,5
    je near .6button
    cmp bl,6
    jae .nojoy
    mov dx,201h
    call GetCoords
    mov eax,[JoyX]
    mov [.joybcx],eax
    mov eax,[JoyY]
    mov [.joybcy],eax
    call CalibrateDispA
    mov dx,201h
    call GetCoords
    mov eax,[JoyX]
    mov [.joyblx],eax
    mov eax,[JoyY]
    mov [.joybly],eax
    call CalibrateDispB
    mov dx,201h
    call GetCoords
    jmp .procdir
.nojoy
    ret
.6button
    mov dx,201h
    call GetCoords3
    mov eax,[JoyX]
    mov [.joybcx],eax
    mov eax,[JoyY]
    mov [.joybcy],eax
    call CalibrateDispA
    mov dx,201h
    call GetCoords3
    mov eax,[JoyX]
    mov [.joyblx],eax
    mov eax,[JoyY]
    mov [.joybly],eax
    call CalibrateDispB
    mov dx,201h
    call GetCoords3
.procdir
    mov eax,[.joybcx]
    add eax,[.joyblx]
    shr eax,1
    mov [JoyMinX],eax
    mov [CalibXmin],eax
    mov eax,[.joybcy]
    add eax,[.joybly]
    shr eax,1
    mov [JoyMinY],eax
    mov [CalibYmin],eax
    mov eax,[.joybcx]
    add eax,[JoyX]
    shr eax,1
    mov [JoyMaxX],eax
    mov [CalibXmax],eax
    mov eax,[.joybcy]
    add eax,[JoyY]
    shr eax,1
    mov [JoyMaxY],eax
    mov [CalibYmax],eax
    ret
%ifdef __MSDOS__
.port209
    cmp bl,1
    jbe .nojoy209
    cmp bl,18
    je near .6button209
    cmp bl,5
    je near .6button209
    cmp bl,6
    jae .nojoy209
    mov dx,209h
    call GetCoords
    mov eax,[JoyX]
    mov [.joybcx],eax
    mov eax,[JoyY]
    mov [.joybcy],eax
    call CalibrateDispA
    mov dx,209h
    call GetCoords
    mov eax,[JoyX]
    mov [.joyblx],eax
    mov eax,[JoyY]
    mov [.joybly],eax
    call CalibrateDispB
    mov dx,209h
    call GetCoords
    jmp .procdir209
.nojoy209
    ret
.6button209
    mov dx,209h
    call GetCoords3
    mov eax,[JoyX]
    mov [.joybcx],eax
    mov eax,[JoyY]
    mov [.joybcy],eax
    call CalibrateDispA
    mov dx,209h
    call GetCoords3
    mov eax,[JoyX]
    mov [.joyblx],eax
    mov eax,[JoyY]
    mov [.joybly],eax
    call CalibrateDispB
    mov dx,209h
    call GetCoords3
.procdir209
    mov eax,[.joybcx]
    add eax,[.joyblx]
    shr eax,1
    mov [JoyMinX209],eax
    mov [CalibXmin209],eax
    mov eax,[.joybcy]
    add eax,[.joybly]
    shr eax,1
    mov [JoyMinY209],eax
    mov [CalibYmin209],eax
    mov eax,[.joybcx]
    add eax,[JoyX]
    shr eax,1
    mov [JoyMaxX209],eax
    mov [CalibXmax209],eax
    mov eax,[.joybcy]
    add eax,[JoyY]
    shr eax,1
    mov [JoyMaxY209],eax
    mov [CalibYmax209],eax
    ret
%endif
SECTION .bss
.joyblx resd 1
.joybly resd 1
.joybcx resd 1
.joybcy resd 1
SECTION .text


SetDevice:
    mov byte[GUICBHold],0
    movzx eax,byte[cplayernum]
%ifdef __MSDOS__
    cmp al,0
    jne .notpl1
    cmp byte[pl1p209],0
    jne .port209
.notpl1
    cmp al,1
    jne .notpl2
    cmp byte[pl2p209],0
    jne .port209
.notpl2
    cmp al,2
    jne .notpl3
    cmp byte[pl3p209],0
    jne .port209
.notpl3
    cmp al,3
    jne .notpl4
    cmp byte[pl4p209],0
    jne .port209
.notpl4
    cmp al,4
    jne .notpl5
    cmp byte[pl5p209],0
    jne .port209
.notpl5
%endif
;.notdos
    mov dword[CalibXmin],0
    mov eax,[GUIInputRefP+eax*4]
    mov bl,[GUIcurrentinputcursloc]
    mov [eax],bl
    mov bh,[cplayernum]
    call SetInputDevice
    call UpdateDevices
    jmp .procmtap
%ifdef __MSDOS__
.port209
    mov dword[CalibXmin209],0
    mov eax,[GUIInputRefP+eax*4]
    mov bl,[GUIcurrentinputcursloc]
    mov [eax],bl
    mov bh,[cplayernum]
    call SetInputDevice209
    call UpdateDevices
%endif
.procmtap
    mov byte[MultiTap],1
    cmp byte[SFXEnable],1
    je .dismtap
    cmp byte[pl3contrl],0
    jne .mtap
    cmp byte[pl4contrl],0
    jne .mtap
    cmp byte[pl5contrl],0
    jne .mtap
.dismtap
    mov byte[MultiTap],0
.mtap
    ret

NEWSYM GUIDoReset
%ifdef __MSDOS__
    call DOSClearScreen
%endif
    pushad
    call Clear2xSaIBuffer
    popad
;    mov byte[NetPlayNoMore],1
    mov byte[RestoreValues],0

    pushad
    call MovieStop
    call RestoreSystemVars
    popad

    ; reset the snes
    call init65816
    pushad
    call procexecloop
    popad

    xor eax,eax
    mov edx,SPCRAM
    mov ax,0FFC9h
    add edx,eax
    mov [spcPCRam],edx
    mov dword[spcS],1EFh
    mov dword[spcRamDP],SPCRAM
    mov byte[spcA],0
    mov byte[spcX],0
    mov byte[spcY],0
    mov byte[spcP],0
    mov byte[spcNZ],0
    mov byte[Voice0Status],0
    mov byte[Voice1Status],0
    mov byte[Voice2Status],0
    mov byte[Voice3Status],0
    mov byte[Voice4Status],0
    mov byte[Voice5Status],0
    mov byte[Voice6Status],0
    mov byte[Voice7Status],0
    mov byte[GUIQuit],2
    ret

SECTION .bss
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
