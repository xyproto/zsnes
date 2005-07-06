;Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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


SetAllKeys:
    xor ebx,ebx
    mov ecx,256+128+64
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a

    mov byte[GUICBHold],0
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

    ;Check if controller is set
    push eax
    mov eax,dword[keycontrolval]
    cmp dword[eax],0
    pop eax
    jne .noLockInputChange
    ret
.noLockInputChange
    mov dword[guicpressptr],edi
    xor ebx,ebx
    mov ecx,12
    mov dword[guipressptr],guipresstext4b
.loop
    mov eax,[.ordercalb+ebx*4]
    shl eax,2
    add dword[guicpressptr],eax
    push ebx
    push ecx
    call guipresstestb
    pop ecx
    pop ebx
    add dword[guipressptr],21
    mov eax,[.ordercalb+ebx*4]
    shl eax,2
    sub dword[guicpressptr],eax
    inc ebx
    dec ecx
    jnz .loop
    ret
SECTION .data
.ordercalb dd 2,3,4,5,1,0,7,10,6,9,8,11
SECTION .text

CalibrateDev1:
    xor eax,eax
    mov al,[cplayernum]
    mov ebx,[GUIInputRefP+eax*4]
    mov bl,[ebx]
    mov byte[GUICBHold],0
%ifdef __MSDOS__
    cmp byte[pl1p209+eax],0
    jne near .port209
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
SECTION .bss
.joyblx resd 1
.joybly resd 1
.joybcx resd 1
.joybcy resd 1
SECTION .text


SetDevice:
    mov byte[GUICBHold],0
    xor eax,eax
    mov al,[cplayernum]
%ifdef __MSDOS__
    cmp byte[pl1p209+eax],0
    jne .port209
%endif
;.notdos
    mov dword[CalibXmin],0
    mov ecx,[DevicePtr+eax*4]
    mov eax,[GUIInputRefP+eax*4]
    mov ebx,[GUIcurrentinputcursloc]
    mov bl,[GUIInputRefF+ebx]
    mov [eax],bl
    mov eax,ecx
    mov bh,[cplayernum]
    call SetInputDevice
    call UpdateDevices
    jmp .procmtap
.port209
    mov dword[CalibXmin209],0
    mov ecx,[DevicePtr+eax*4]
    mov eax,[GUIInputRefP+eax*4]
    mov ebx,[GUIcurrentinputcursloc]
    mov bl,[GUIInputRefF+ebx]
    mov [eax],bl
    mov eax,ecx
    mov bh,[cplayernum]
    call SetInputDevice209
    call UpdateDevices
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
    call ClearScreen
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
    mov edx,spcRam
    mov ax,0FFC0h
    add edx,eax
    mov [spcPCRam],edx
    mov dword[spcS],1EFh
    mov dword[spcRamDP],spcRam
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
