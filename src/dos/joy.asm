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



%include "macros.mac"

EXTSYM pl1contrl,pl2contrl,pl3contrl,pl4contrl,pl5contrl
EXTSYM pressed,CalibXmin,CalibYmin,CalibXmax,CalibYmax
EXTSYM CalibXmin209,CalibYmin209,CalibXmax209,CalibYmax209
EXTSYM read_gpp,_readSideWinder,_SW1,_SW2,_SW3,_SW4,_SWCount
EXTSYM pl1p209,pl2p209,pl3p209,pl4p209,pl5p209,WhichSW,delay
EXTSYM pl1upk,pl1downk,pl1leftk,pl1rightk,pl1startk,pl1selk
EXTSYM pl1Ak,pl1Bk,pl1Xk,pl1Yk,pl1Lk,pl1Rk
EXTSYM pl2upk,pl2downk,pl2leftk,pl2rightk,pl2startk,pl2selk
EXTSYM pl2Ak,pl2Bk,pl2Xk,pl2Yk,pl2Lk,pl2Rk
EXTSYM pl3upk,pl3downk,pl3leftk,pl3rightk,pl3startk,pl3selk
EXTSYM pl3Ak,pl3Bk,pl3Xk,pl3Yk,pl3Lk,pl3Rk
EXTSYM pl4upk,pl4downk,pl4leftk,pl4rightk,pl4startk,pl4selk
EXTSYM pl4Ak,pl4Bk,pl4Xk,pl4Yk,pl4Lk,pl4Rk
EXTSYM pl5upk,pl5downk,pl5leftk,pl5rightk,pl5startk,pl5selk
EXTSYM pl5Ak,pl5Bk,pl5Xk,pl5Yk,pl5Lk,pl5Rk
EXTSYM JoyX,JoyY,JoyX2,JoyY2,JoyMaxX,JoyMaxY,JoyMinX,JoyMinY
EXTSYM JoyExists,JoyExists2,GetCoords,GetCoords3

SECTION .data
NEWSYM JoyAltrn2,  db 2
SECTION .bss

NEWSYM joy4218, resb 1
NEWSYM joy4219, resb 1
NEWSYM joy4218j, resb 1
NEWSYM joy4219j, resb 1
NEWSYM joy421A, resb 1
NEWSYM joy421B, resb 1
NEWSYM joy421Aj, resb 1
NEWSYM joy421Bj, resb 1

NEWSYM JoyCenterX, resd 1
NEWSYM JoyCenterY, resd 1
NEWSYM JoyCenterX2, resd 1
NEWSYM JoyCenterY2, resd 1
NEWSYM JoyMaxX2,    resd 1
NEWSYM JoyMaxY2,    resd 1
NEWSYM JoyMinX2,    resd 1
NEWSYM JoyMinY2,    resd 1
NEWSYM JoyAltrn,   resb 1
NEWSYM JoyCenterX209, resd 1
NEWSYM JoyCenterY209, resd 1
NEWSYM JoyMaxX209,    resd 1
NEWSYM JoyMaxY209,    resd 1
NEWSYM JoyMinX209,    resd 1
NEWSYM JoyMinY209,    resd 1
NEWSYM JoyCenterX2209, resd 1
NEWSYM JoyCenterY2209, resd 1
NEWSYM JoyMaxX2209,    resd 1
NEWSYM JoyMaxY2209,    resd 1
NEWSYM JoyMinX2209,    resd 1
NEWSYM JoyMinY2209,    resd 1

JoyQuant    resb 1
JoyBQuant   resb 1
NumSWs      resb 1
NumGRiPs    resb 1
PPad        resb 1        ; b0 = pp0, b1 = pp1

JoyQuant209    resb 1
JoyBQuant209   resb 1
NumSWs209      resb 1
NumGRiPs209    resb 1
Buttons6       resb 1
Buttons6209    resb 1

SECTION .text

NEWSYM DosUpdateDevices
    mov byte[PPad],0
    mov byte[JoyQuant],0
    mov byte[JoyBQuant],0
    mov byte[NumSWs],0
    mov byte[NumGRiPs],0
    mov byte[JoyQuant209],0
    mov byte[JoyBQuant209],0
    mov byte[NumSWs209],0
    mov byte[NumGRiPs209],0
    mov byte[Buttons6],0
    mov byte[Buttons6209],0
    ; Check for button #'s, joystick types, etc.
    mov al,[pl1contrl]
    mov ah,[pl1p209]
    call .checkdevice
    mov al,[pl2contrl]
    mov ah,[pl2p209]
    call .checkdevice
    mov al,[pl3contrl]
    mov ah,[pl3p209]
    call .checkdevice
    mov al,[pl4contrl]
    mov ah,[pl4p209]
    call .checkdevice
    mov al,[pl5contrl]
    mov ah,[pl5p209]
    call .checkdevice
    ; Auto-Calibrate the joysticks
    cmp byte[JoyQuant],2
    jne .no2joyst
    mov dx,201h
    mov byte[JoyExists2],1
    call GetCoords3
    mov ecx,1000
    call delay
    cmp byte[JoyExists2],0
    jne .no2joyst
    mov byte[JoyQuant],1
.no2joyst
    cmp byte[JoyQuant],1
    jne .no1joyst
    mov dx,201h
    mov byte[JoyExists],1
    call GetCoords
    mov ecx,1000
    call delay
    cmp byte[JoyExists],0
    jne .no1joyst
    mov byte[JoyQuant],0
.no1joyst
    ; set max & mins
    mov ecx,[JoyX2]
    mov [JoyCenterX2], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinX2],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxX2],eax
    mov ecx,[JoyY2]
    mov [JoyCenterY2], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinY2],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxY2],eax
    mov ecx,[JoyX]
    mov [JoyCenterX], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinX],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxX],eax
    mov ecx,[JoyY]
    mov [JoyCenterY], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinY],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxY],eax
    ; Port 209
    ; Auto-Calibrate the joysticks
    cmp byte[JoyQuant209],2
    jne .no2joyst2
    mov dx,209h
    mov byte[JoyExists2],1
    call GetCoords3
    mov ecx,1000
    call delay
    cmp byte[JoyExists2],0
    jne .no2joyst2
    mov byte[JoyQuant209],1
.no2joyst2
    cmp byte[JoyQuant209],1
    jne .no1joyst2
    mov dx,209h
    mov byte[JoyExists],1
    call GetCoords
    mov ecx,1000
    call delay
    cmp byte[JoyExists],0
    jne .no1joyst2
    mov byte[JoyQuant209],0
.no1joyst2
    ; set max & mins
    mov ecx,[JoyX2]
    mov [JoyCenterX2209], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinX2209],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxX2209],eax
    mov ecx,[JoyY2]
    mov [JoyCenterY2209], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinY2209],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxY2209],eax
    mov ecx,[JoyX]
    mov [JoyCenterX209], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinX209],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxX209],eax
    mov ecx,[JoyY]
    mov [JoyCenterY209], ecx
    mov eax,ecx
    shr ecx,1
    sub eax,ecx
    mov [JoyMinY209],eax
    add eax,ecx
    add eax,ecx
    mov [JoyMaxY209],eax
    cmp dword[CalibXmin],0
    je .nocalib
    mov eax,[CalibXmin]
    mov [JoyMinX],eax
    mov eax,[CalibYmin]
    mov [JoyMinY],eax
    mov eax,[CalibXmax]
    mov [JoyMaxX],eax
    mov eax,[CalibYmax]
    mov [JoyMaxY],eax
.nocalib
    cmp dword[CalibXmin209],0
    je .nocalib209
    mov eax,[CalibXmin209]
    mov [JoyMinX209],eax
    mov eax,[CalibYmin209]
    mov [JoyMinY209],eax
    mov eax,[CalibXmax209]
    mov [JoyMaxX209],eax
    mov eax,[CalibYmax209]
    mov [JoyMaxY209],eax
.nocalib209
    ret
.checkdevice
    ; 1 = keyboard, 2 = 2b joystick, 3 = 4b joystick, 4 = 6b joystick, 5 = 8b joystick
    ; 6 = Sidewinder1, 7 = Sidewinder2, 8 = Sidewinder3, 9 = Sidewinder4
    ; 10 = Grip0, 11 = Grip1, N/A = Grip2, N/A = Grip3, 12 = Parallel pad0
    ; 13 = Parallel pad1, 14 = Parallel pad2, 15 = Parallel pad3, 16 = Parallel pad4

    cmp al,1
    ja .joyokay
    ret
.joyokay
    cmp al,12
    jne .nopp0
    or byte[PPad],1
    ret
.nopp0
    cmp al,13
    jne .nopp1
    or byte[PPad],2
    ret
.nopp1
    cmp al,14
    jne .nopp2
    or byte[PPad],4
    ret
.nopp2
    cmp al,15
    jne .nopp3
    or byte[PPad],8
    ret
.nopp3
    cmp al,16
    jne .nopp4
    or byte[PPad],16
    ret
.nopp4
    cmp ah,0
    jne near .port209
    cmp al,2
    jne .nojoy2b
    inc byte[JoyQuant]
    add byte[JoyBQuant],2
    ret
.nojoy2b
    cmp al,3
    jne .nojoy4b
    mov byte[JoyQuant],1
    mov byte[JoyBQuant],4
    ret
.nojoy4b
    cmp al,4
    jne .nojoy6b
    mov byte[JoyQuant],2
    mov byte[JoyBQuant],6
    mov byte[Buttons6],1
    ret
.nojoy6b
    cmp al,5
    jne .nojoy8b
    mov byte[JoyQuant],2
    mov byte[JoyBQuant],8
    mov byte[Buttons6],2
    ret
.nojoy8b
    cmp al,9
    ja .grip
    sub al,5
    cmp byte[NumSWs],al
    ja .skipswc
    mov [NumSWs],al
.skipswc
    ret
.grip
    cmp al,11
    ja .none
    sub al,9
    cmp byte[NumGRiPs],al
    ja .skipgripc
    mov [NumGRiPs],al
.skipgripc
.none
    ret
.port209
    cmp al,2
    jne .nojoy2b2
    inc byte[JoyQuant209]
    add byte[JoyBQuant209],2
    ret
.nojoy2b2
    cmp al,3
    jne .nojoy4b2
.joy4b2
    mov byte[JoyQuant209],1
    mov byte[JoyBQuant209],4
    ret
.nojoy4b2
    cmp al,4
    jne .nojoy6b2
    mov byte[JoyQuant209],2
    mov byte[JoyBQuant209],6
    mov byte[Buttons6209],1
    ret
.nojoy6b2
    cmp al,5
    jne .nojoy8b2
    mov byte[JoyQuant209],2
    mov byte[JoyBQuant209],8
    mov byte[Buttons6209],2
    ret
.nojoy8b2
    cmp al,9
    ja .grip2
    sub al,5
    cmp byte[NumSWs209],al
    ja .skipswc2
    mov [NumSWs209],al
.skipswc2
    ret
.grip2
    cmp al,11
    ja .none2
    sub al,9
    cmp byte[NumGRiPs209],al
    ja .skipgripc2
    mov [NumGRiPs209],al
.skipgripc2
.none2
    ret

JoyRead209:
   cmp byte[JoyAltrn],1
   jne near .noanalog

   ; Clear Joystick buttons and movements
   mov word[pressed+100h],0      ; B7-8
   mov word[pressed+106h],0      ; B5-6
   mov dword[pressed+14Ch],0    ; Up,Down,Left,Right, pl1
   mov dword[pressed+168h],0    ; Up,Down,Left,Right, pl2

   ; Process Joystick(s)
   cmp byte[JoyQuant209],2
   jne near .no2joyst
   mov dx,209h
   call GetCoords3

   ; Set button 5-6 + 2player Control
   cmp byte[Buttons6209],0
   jne near .6button
   mov eax,[JoyX2]
   cmp eax,[JoyMinX2209]
   jae .noleft2
   mov byte[pressed+16Ah],1
.noleft2
   mov eax,[JoyX2]
   cmp eax,[JoyMaxX2209]
   jbe .noright2
   mov byte[pressed+16Bh],1
.noright2
   mov eax,[JoyY2]
   cmp eax,[JoyMinY2209]
   jae .noup2
   mov byte[pressed+168h],1
.noup2
   mov eax,[JoyY2]
   cmp eax,[JoyMaxY2209]
   jbe .nodown2
   mov byte[pressed+169h],1
.nodown2
   jmp .1pcoord
.6button
   mov eax,[JoyY2]
   cmp eax,[JoyMinY2209]
   jae .nob5
   mov byte[pressed+106h],1
.nob5
   mov eax,[JoyX2]
   cmp eax,[JoyMinX2209]
   jae .nob6
   mov byte[pressed+107h],1
.nob6
   cmp byte[Buttons6209],2
   jne .no8b
   mov eax,[JoyY2]
   cmp eax,[JoyMaxY2209]
   jbe .nob7
   mov byte[pressed+100h],1
.nob7
   mov eax,[JoyX2]
   cmp eax,[JoyMaxX2209]
   jbe .nob8
   mov byte[pressed+101h],1
.nob8
.no8b
   jmp .1pcoord
.no2joyst

   ; Set 1 player control
   cmp byte[JoyQuant209],1
   jne near .no1joyst
   mov dx,209h
   call GetCoords
.1pcoord
   mov eax,[JoyX]
   cmp eax,[JoyMinX209]
   jae .noleft
   mov byte[pressed+14Eh],1
.noleft
   mov eax,[JoyX]
   cmp eax,[JoyMaxX209]
   jbe .noright
   mov byte[pressed+14Fh],1
.noright
   mov eax,[JoyY]
   cmp eax,[JoyMinY209]
   jae .noup
   mov byte[pressed+14Ch],1
.noup
   mov eax,[JoyY]
   cmp eax,[JoyMaxY209]
   jbe .nodown
   mov byte[pressed+14Dh],1
.nodown
.no1joyst
.noanalog
   test byte[JoyAltrn],1
   jne near .joynotexist
   mov dword[pressed+102h],0     ; B1-4
   cmp byte[JoyQuant209],0
   je .joynotexist
   ; Set buttons 1-4
   mov dx, 0209h
   xor al,al
   out dx, al
   in al, dx
   not al
   test al,10h
   jz .nob1
   mov byte[pressed+102h],1
.nob1
   test al,20h
   jz .nob2
   mov byte[pressed+103h],1
.nob2
   cmp byte[JoyBQuant209],2
   je .joynotexist
   test al,40h
   jz .nob3
   mov byte[pressed+104h],1
.nob3
   test al,80h
   jz .nob4
   mov byte[pressed+105h],1
.nob4
.joynotexist
   ret

NEWSYM DOSJoyRead
   push edx
   push ebx
   push ecx

   inc byte[JoyAltrn]
   and byte[JoyAltrn],3

   cmp byte[JoyAltrn],0
   jne near .noanalog

   ; Clear Joystick buttons and movements
   mov word[pressed+86h],0      ; B5-6
   mov word[pressed+80h],0      ; B7-8
   mov dword[pressed+0CCh],0    ; Up,Down,Left,Right, pl1
   mov dword[pressed+0E8h],0    ; Up,Down,Left,Right, pl2

   ; Process Joystick(s)
   cmp byte[JoyQuant],2
   jne near .no2joyst
   mov dx,201h
   call GetCoords3

   ; Set button 5-6 + 2player Control
   cmp byte[Buttons6],0
   jne near .6button
   mov eax,[JoyX2]
   cmp eax,[JoyMinX2]
   jae .noleft2
   mov byte[pressed+0EAh],1
.noleft2
   mov eax,[JoyX2]
   cmp eax,[JoyMaxX2]
   jbe .noright2
   mov byte[pressed+0EBh],1
.noright2
   mov eax,[JoyY2]
   cmp eax,[JoyMinY2]
   jae .noup2
   mov byte[pressed+0E8h],1
.noup2
   mov eax,[JoyY2]
   cmp eax,[JoyMaxY2]
   jbe .nodown2
   mov byte[pressed+0E9h],1
.nodown2
   jmp .1pcoord
.6button
   mov eax,[JoyY2]
   cmp eax,[JoyMinY2]
   jae .nob5
   mov byte[pressed+086h],1
.nob5
   mov eax,[JoyX2]
   cmp eax,[JoyMinX2]
   jae .nob6
   mov byte[pressed+087h],1
.nob6
   cmp byte[Buttons6],2
   jne .no8b
   mov eax,[JoyY2]
   cmp eax,[JoyMaxY2]
   jbe .nob7
   mov byte[pressed+80h],1
.nob7
   mov eax,[JoyX2]
   cmp eax,[JoyMaxX2]
   jbe .nob8
   mov byte[pressed+81h],1
.nob8
.no8b
   jmp .1pcoord
.no2joyst

   ; Set 1 player control
   cmp byte[JoyQuant],1
   jne near .no1joyst
   mov dx,201h
   call GetCoords
.1pcoord
   mov eax,[JoyX]
   cmp eax,[JoyMinX]
   jae .noleft
   mov byte[pressed+0CEh],1
.noleft
   mov eax,[JoyX]
   cmp eax,[JoyMaxX]
   jbe .noright
   mov byte[pressed+0CFh],1
.noright
   mov eax,[JoyY]
   cmp eax,[JoyMinY]
   jae .noup
   mov byte[pressed+0CCh],1
.noup
   mov eax,[JoyY]
   cmp eax,[JoyMaxY]
   jbe .nodown
   mov byte[pressed+0CDh],1
.nodown
.no1joyst
.noanalog
   test byte[JoyAltrn],1
   jz .joynotexist
   mov dword[pressed+82h],0     ; B1-4
   cmp byte[JoyQuant],0
   je .joynotexist
   ; Set buttons 1-4
   mov dx, 0201h
   xor al,al
   out dx, al
   in al, dx
   not al
   test al,10h
   jz .nob1
   mov byte[pressed+82h],1
.nob1
   test al,20h
   jz .nob2
   mov byte[pressed+83h],1
.nob2
   cmp byte[JoyBQuant],2
   je .joynotexist
   test al,40h
   jz .nob3
   mov byte[pressed+84h],1
.nob3
   test al,80h
   jz .nob4
   mov byte[pressed+85h],1
.nob4
.joynotexist
   ; Process Joystick Buttons
   cmp byte[JoyQuant209],0
   je .no209
   call JoyRead209
.no209
   test byte[JoyAltrn],1
   jz near .noport4
   cmp byte[NumSWs],0
   je .nosw
   call SideWinder
.nosw
   cmp byte[NumSWs209],0
   je .nosw209
   call SideWinder209
.nosw209
   test byte[JoyAltrn],2
   jz .nogrip209
   cmp byte[NumGRiPs],0
   je .nogrip
   call GamePadPro
.nogrip
   cmp byte[NumGRiPs209],0
   je .nogrip209
   call GamePadPro209
.nogrip209
   test byte[PPad],1
   jz .noport1
   call GetParallelPlayer1
.noport1
   test byte[PPad],2
   jz .noport2
   call GetParallelPlayer2
.noport2
   test byte[PPad],4
   jz .noport3
   call GetParallelPlayer3
.noport3
   test byte[PPad],8
   jz .noport4
   call GetParallelPlayer4
.noport4
   test byte[PPad],16
   jz .noport5
   call GetParallelPlayer5
.noport5
   cmp byte[JoyAltrn],0
   je .nogpp2209
   cmp byte[NumGRiPs],2
   jne .nogpp2
   call GamePadPro2
.nogpp2
   cmp byte[NumGRiPs209],2
   jne .nogpp2209
   call GamePadPro2209
.nogpp2209
   pop ecx
   pop ebx
   pop edx
   ret

;bit 1 - left,2 - right,3 - down,4 - up,5 - r1,6 - l1,7 - red,8 - yellow
;    9 - green,10 - l2,11 - blue,12 - r2,13 - start,14 - select

%macro GPProHelp 2
   test eax,%1
   jz %%nope
   mov byte[pressed+ebx+%2],1
%%nope
%endmacro

;A8=buttons, CA=select/start, F0=movement (inc by 4)
GamePadPro:
   mov al,0
   mov dx,201h
   call read_gpp
   cmp eax,1
   je near .error
   xor ebx,ebx
   mov byte[pressed+ebx+0F0h],0
   mov byte[pressed+ebx+0F1h],0
   mov byte[pressed+ebx+0F2h],0
   mov byte[pressed+ebx+0F3h],0
   mov byte[pressed+ebx+0A8h],0
   mov byte[pressed+ebx+0A9h],0
   mov byte[pressed+ebx+0AAh],0
   mov byte[pressed+ebx+0ABh],0
   mov byte[pressed+ebx+0ACh],0
   mov byte[pressed+ebx+0ADh],0
   mov byte[pressed+ebx+0AEh],0
   mov byte[pressed+ebx+0AFh],0
   mov byte[pressed+ebx+0CAh],0
   mov byte[pressed+ebx+0CBh],0
   GPProHelp 0002h,0F2h ; left
   GPProHelp 0004h,0F3h ; right
   GPProHelp 0008h,0F1h ; down
   GPProHelp 0010h,0F0h ; up
   GPProHelp 0020h,0AEh ; r1
   GPProHelp 0040h,0ACh ; l1
   GPProHelp 0080h,0A8h ; red
   GPProHelp 0100h,0AAh ; yellow
   GPProHelp 0200h,0ABh ; green
   GPProHelp 0400h,0ADh ; l2
   GPProHelp 0800h,0A9h ; blue
   GPProHelp 1000h,0AFh ; r2
   GPProHelp 2000h,0CBh ; start
   GPProHelp 4000h,0CAh ; select
.error
   ret

GamePadPro2:
   mov al,1
   mov dx,201h
   call read_gpp
   cmp eax,1
   je near .error
   mov ebx,4
   mov byte[pressed+ebx+0F0h],0
   mov byte[pressed+ebx+0F1h],0
   mov byte[pressed+ebx+0F2h],0
   mov byte[pressed+ebx+0F3h],0
   mov ebx,8
   mov byte[pressed+ebx+0A8h],0
   mov byte[pressed+ebx+0A9h],0
   mov byte[pressed+ebx+0AAh],0
   mov byte[pressed+ebx+0ABh],0
   mov byte[pressed+ebx+0ACh],0
   mov byte[pressed+ebx+0ADh],0
   mov byte[pressed+ebx+0AEh],0
   mov byte[pressed+ebx+0AFh],0
   mov byte[pressed+ebx+0CAh],0
   mov byte[pressed+ebx+0CBh],0
   mov ebx,4
   GPProHelp 0002h,0F2h ; left
   GPProHelp 0004h,0F3h ; right
   GPProHelp 0008h,0F1h ; down
   GPProHelp 0010h,0F0h ; up
   mov ebx,8
   GPProHelp 0020h,0AEh ; r1
   GPProHelp 0040h,0ACh ; l1
   GPProHelp 0080h,0A8h ; red
   GPProHelp 0100h,0AAh ; yellow
   GPProHelp 0200h,0ABh ; green
   GPProHelp 0400h,0ADh ; l2
   GPProHelp 0800h,0A9h ; blue
   GPProHelp 1000h,0AFh ; r2
   GPProHelp 2000h,0CBh ; start
   GPProHelp 4000h,0CAh ; select
.error
   ret

SideWinder:
  mov al,[NumSWs]
  mov [_SWCount],al
  mov dx,201h
  call _readSideWinder
  ;bit 0=error 1=up 2=dn 3=rt 4=lt 5=A 6=B 7=C 8=X 9=Y 10=Z 11=L 12=R 13=St 14=M
  ;k....L=L, R=R, start=start, M=select, X=y, Y=x, B=a, A=b
  ;sidewinder=snes
.loop
  mov eax,[_SW1]
  xor ebx,ebx
  cmp byte[WhichSW],2
  jne .noSW2
  mov eax,[_SW2]
  mov ebx,08h
.noSW2
  cmp byte[WhichSW],3
  jne .noSW3
  mov eax,[_SW3]
  mov ebx,10h
.noSW3
  cmp byte[WhichSW],4
  jne .noSW4
  mov eax,[_SW4]
  mov ebx,18h
.noSW4
  inc byte[WhichSW]
  mov byte[pressed+ebx+0D4h],0
  mov byte[pressed+ebx+0D5h],0
  mov byte[pressed+ebx+0D6h],0
  mov byte[pressed+ebx+0D7h],0
  mov byte[pressed+ebx+088h],0
  mov byte[pressed+ebx+089h],0
  mov byte[pressed+ebx+08Ah],0
  mov byte[pressed+ebx+08Bh],0
  mov byte[pressed+ebx+08Ch],0
  mov byte[pressed+ebx+08Dh],0
  mov byte[pressed+ebx+08Eh],0
  mov byte[pressed+ebx+08Fh],0
  mov byte[pressed+ebx+0C8h],0
  mov byte[pressed+ebx+0C9h],0
  test ax,02h   ; up
  jz .noup
  mov byte[pressed+ebx+0D4h],1
.noup
  test ax,04h   ; down
  jz .nodown
  mov byte[pressed+ebx+0D5h],1
.nodown
  test ax,08h   ; right
  jz .noright
  mov byte[pressed+ebx+0D7h],1
.noright
  test ax,10h   ; left
  jz .noleft
  mov byte[pressed+ebx+0D6h],1
.noleft
  test ax,20h   ; A
  jz .noa
  mov byte[pressed+ebx+088h],1
.noa
  test ax,40h   ; B
  jz .nob
  mov byte[pressed+ebx+089h],1
.nob
  test ax,80h  ; C
  jz .noc
  mov byte[pressed+ebx+08Ah],1
.noc
  test ax,100h  ; X
  jz .nox
  mov byte[pressed+ebx+08Bh],1
.nox
  test ax,200h  ; Y
  jz .noy
  mov byte[pressed+ebx+08Ch],1
.noy
  test ax,400h ; Z
  jz .noz
  mov byte[pressed+ebx+08Dh],1
.noz
  test ax,800h ; L
  jz .nol
  mov byte[pressed+ebx+08Eh],1
.nol
  test ax,1000h ; R
  jz .nor
  mov byte[pressed+ebx+08Fh],1
.nor
  test ax,2000h ; start
  jz .nostart
  mov byte[pressed+ebx+0C8h],1
.nostart
  test ax,4000h ; M
  jz .noselect
  mov byte[pressed+ebx+0C9h],1
.noselect
  mov al,[WhichSW]
  cmp al,[NumSWs]
  jbe near .loop
  ret                         ;return to calling procedure

;A8=buttons, CA=select/start, F0=movement (inc by 4)
GamePadPro209:
   mov al,0
   mov dx,209h
   call read_gpp
   cmp eax,1
   je near .error
   xor ebx,ebx
   mov byte[pressed+ebx+170h],0
   mov byte[pressed+ebx+171h],0
   mov byte[pressed+ebx+172h],0
   mov byte[pressed+ebx+173h],0
   mov byte[pressed+ebx+128h],0
   mov byte[pressed+ebx+129h],0
   mov byte[pressed+ebx+12Ah],0
   mov byte[pressed+ebx+12Bh],0
   mov byte[pressed+ebx+12Ch],0
   mov byte[pressed+ebx+12Dh],0
   mov byte[pressed+ebx+12Eh],0
   mov byte[pressed+ebx+12Fh],0
   mov byte[pressed+ebx+14Ah],0
   mov byte[pressed+ebx+14Bh],0
   GPProHelp 0002h,1F2h ; left
   GPProHelp 0004h,1F3h ; right
   GPProHelp 0008h,1F1h ; down
   GPProHelp 0010h,1F0h ; up
   GPProHelp 0020h,1AEh ; r1
   GPProHelp 0040h,1ACh ; l1
   GPProHelp 0080h,1A8h ; red
   GPProHelp 0100h,1AAh ; yellow
   GPProHelp 0200h,1ABh ; green
   GPProHelp 0400h,1ADh ; l2
   GPProHelp 0800h,1A9h ; blue
   GPProHelp 1000h,1AFh ; r2
   GPProHelp 2000h,1CBh ; start
   GPProHelp 4000h,1CAh ; select
.error
   ret

GamePadPro2209:
   mov al,1
   mov dx,209h
   call read_gpp
   cmp eax,1
   je near .error
   mov ebx,4
   mov byte[pressed+ebx+170h],0
   mov byte[pressed+ebx+171h],0
   mov byte[pressed+ebx+172h],0
   mov byte[pressed+ebx+173h],0
   mov ebx,8
   mov byte[pressed+ebx+128h],0
   mov byte[pressed+ebx+129h],0
   mov byte[pressed+ebx+12Ah],0
   mov byte[pressed+ebx+12Bh],0
   mov byte[pressed+ebx+12Ch],0
   mov byte[pressed+ebx+12Dh],0
   mov byte[pressed+ebx+12Eh],0
   mov byte[pressed+ebx+12Fh],0
   mov byte[pressed+ebx+14Ah],0
   mov byte[pressed+ebx+14Bh],0
   mov ebx,4
   GPProHelp 0002h,1F2h ; left
   GPProHelp 0004h,1F3h ; right
   GPProHelp 0008h,1F1h ; down
   GPProHelp 0010h,1F0h ; up
   mov ebx,8
   GPProHelp 0020h,1AEh ; r1
   GPProHelp 0040h,1ACh ; l1
   GPProHelp 0080h,1A8h ; red
   GPProHelp 0100h,1AAh ; yellow
   GPProHelp 0200h,1ABh ; green
   GPProHelp 0400h,1ADh ; l2
   GPProHelp 0800h,1A9h ; blue
   GPProHelp 1000h,1AFh ; r2
   GPProHelp 2000h,1CBh ; start
   GPProHelp 4000h,1CAh ; select
.error
   ret

SideWinder209:
  mov al,[NumSWs209]
  mov [_SWCount],al
  mov dx,209h
  call _readSideWinder
  ;bit 0=error 1=up 2=dn 3=rt 4=lt 5=A 6=B 7=C 8=X 9=Y 10=Z 11=L 12=R 13=St 14=M
  ;k....L=L, R=R, start=start, M=select, X=y, Y=x, B=a, A=b
  ;sidewinder=snes
.loop
  mov eax,[_SW1]
  xor ebx,ebx
  cmp byte[WhichSW],2
  jne .noSW2
  mov eax,[_SW2]
  mov ebx,08h
.noSW2
  cmp byte[WhichSW],3
  jne .noSW3
  mov eax,[_SW3]
  mov ebx,10h
.noSW3
  cmp byte[WhichSW],4
  jne .noSW4
  mov eax,[_SW4]
  mov ebx,18h
.noSW4
  inc byte[WhichSW]
  mov byte[pressed+ebx+154h],0
  mov byte[pressed+ebx+155h],0
  mov byte[pressed+ebx+156h],0
  mov byte[pressed+ebx+157h],0
  mov byte[pressed+ebx+108h],0
  mov byte[pressed+ebx+109h],0
  mov byte[pressed+ebx+10Ah],0
  mov byte[pressed+ebx+10Bh],0
  mov byte[pressed+ebx+10Ch],0
  mov byte[pressed+ebx+10Dh],0
  mov byte[pressed+ebx+10Eh],0
  mov byte[pressed+ebx+10Fh],0
  mov byte[pressed+ebx+148h],0
  mov byte[pressed+ebx+149h],0
  test ax,02h   ; up
  jz .noup
  mov byte[pressed+ebx+154h],1
.noup
  test ax,04h   ; down
  jz .nodown
  mov byte[pressed+ebx+155h],1
.nodown
  test ax,08h   ; right
  jz .noright
  mov byte[pressed+ebx+157h],1
.noright
  test ax,10h   ; left
  jz .noleft
  mov byte[pressed+ebx+156h],1
.noleft
  test ax,20h   ; A
  jz .noa
  mov byte[pressed+ebx+108h],1
.noa
  test ax,40h   ; B
  jz .nob
  mov byte[pressed+ebx+109h],1
.nob
  test ax,80h  ; C
  jz .noc
  mov byte[pressed+ebx+10Ah],1
.noc
  test ax,100h  ; X
  jz .nox
  mov byte[pressed+ebx+10Bh],1
.nox
  test ax,200h  ; Y
  jz .noy
  mov byte[pressed+ebx+10Ch],1
.noy
  test ax,400h ; Z
  jz .noz
  mov byte[pressed+ebx+10Dh],1
.noz
  test ax,800h ; L
  jz .nol
  mov byte[pressed+ebx+10Eh],1
.nol
  test ax,1000h ; R
  jz .nor
  mov byte[pressed+ebx+10Fh],1
.nor
  test ax,2000h ; start
  jz .nostart
  mov byte[pressed+ebx+148h],1
.nostart
  test ax,4000h ; M
  jz .noselect
  mov byte[pressed+ebx+149h],1
.noselect
  mov al,[WhichSW]
  cmp al,[NumSWs209]
  jbe near .loop
  ret                         ;return to calling procedure

; Parallel SNES pad reader routines by Karl Stenerud
; Original design by Benji York:
;

%macro PPortHelp 3
    mov al, %1
    out dx, al
    mov al, 0F8h
    out dx, al
    inc dx
    in  al, dx
    dec dx
    and ax, %2
    jnz %%nobutton
    mov byte[pressed+%3], 1
%%nobutton
%endmacro

%macro PPortHelpInv 3         ;needed for the pad 5
    mov al, %1
    out dx, al
    mov al, 0F8h
    out dx, al
    inc dx
    in  al, dx
    dec dx
    and ax, %2
    jz %%nobutton             ;pad 5 is on pin 11, which is hardware inverted...
    mov byte[pressed+%3], 1
%%nobutton
%endmacro

GetParallelPlayer1:
    mov dx, 0378h
    mov byte[pressed+180h],0
    mov byte[pressed+181h],0
    mov byte[pressed+182h],0
    mov byte[pressed+183h],0
    mov byte[pressed+184h],0
    mov byte[pressed+185h],0
    mov byte[pressed+186h],0
    mov byte[pressed+187h],0
    mov byte[pressed+188h],0
    mov byte[pressed+189h],0
    mov byte[pressed+18Ah],0
    mov byte[pressed+18Bh],0
    PPortHelp 0FAh, 40h, 180h ;Mask 40h (pin 10 of lpt : data for pad 1)
    PPortHelp 0F9h, 40h, 181h
    PPortHelp 0F9h, 40h, 182h
    PPortHelp 0F9h, 40h, 183h
    PPortHelp 0F9h, 40h, 184h
    PPortHelp 0F9h, 40h, 185h
    PPortHelp 0F9h, 40h, 186h
    PPortHelp 0F9h, 40h, 187h
    PPortHelp 0F9h, 40h, 188h
    PPortHelp 0F9h, 40h, 189h
    PPortHelp 0F9h, 40h, 18Ah
    PPortHelp 0F9h, 40h, 18Bh
    ret

GetParallelPlayer2:
    mov dx, 0378h
    mov byte[pressed+190h],0
    mov byte[pressed+191h],0
    mov byte[pressed+192h],0
    mov byte[pressed+193h],0
    mov byte[pressed+194h],0
    mov byte[pressed+195h],0
    mov byte[pressed+196h],0
    mov byte[pressed+197h],0
    mov byte[pressed+198h],0
    mov byte[pressed+199h],0
    mov byte[pressed+19Ah],0
    mov byte[pressed+19Bh],0
    PPortHelp 0FAh, 20h, 190h ;Mask 20h (pin 12 of lpt : data for pad 2)
    PPortHelp 0F9h, 20h, 191h
    PPortHelp 0F9h, 20h, 192h
    PPortHelp 0F9h, 20h, 193h
    PPortHelp 0F9h, 20h, 194h
    PPortHelp 0F9h, 20h, 195h
    PPortHelp 0F9h, 20h, 196h
    PPortHelp 0F9h, 20h, 197h
    PPortHelp 0F9h, 20h, 198h
    PPortHelp 0F9h, 20h, 199h
    PPortHelp 0F9h, 20h, 19Ah
    PPortHelp 0F9h, 20h, 19Bh
    ret

GetParallelPlayer3:
    mov dx, 0378h
    mov byte[pressed+1A0h],0
    mov byte[pressed+1A1h],0
    mov byte[pressed+1A2h],0
    mov byte[pressed+1A3h],0
    mov byte[pressed+1A4h],0
    mov byte[pressed+1A5h],0
    mov byte[pressed+1A6h],0
    mov byte[pressed+1A7h],0
    mov byte[pressed+1A8h],0
    mov byte[pressed+1A9h],0
    mov byte[pressed+1AAh],0
    mov byte[pressed+1ABh],0
    PPortHelp 0FAh, 10h, 1A0h ;Mask 10h (pin 13 of lpt : data for pad 3)
    PPortHelp 0F9h, 10h, 1A1h
    PPortHelp 0F9h, 10h, 1A2h
    PPortHelp 0F9h, 10h, 1A3h
    PPortHelp 0F9h, 10h, 1A4h
    PPortHelp 0F9h, 10h, 1A5h
    PPortHelp 0F9h, 10h, 1A6h
    PPortHelp 0F9h, 10h, 1A7h
    PPortHelp 0F9h, 10h, 1A8h
    PPortHelp 0F9h, 10h, 1A9h
    PPortHelp 0F9h, 10h, 1AAh
    PPortHelp 0F9h, 10h, 1ABh
    ret

GetParallelPlayer4:
    mov dx, 0378h
    mov byte[pressed+1B0h],0
    mov byte[pressed+1B1h],0
    mov byte[pressed+1B2h],0
    mov byte[pressed+1B3h],0
    mov byte[pressed+1B4h],0
    mov byte[pressed+1B5h],0
    mov byte[pressed+1B6h],0
    mov byte[pressed+1B7h],0
    mov byte[pressed+1B8h],0
    mov byte[pressed+1B9h],0
    mov byte[pressed+1BAh],0
    mov byte[pressed+1BBh],0
    PPortHelp 0FAh, 08h, 1B0h
    PPortHelp 0F9h, 08h, 1B1h ;Mask 08h (pin 15 of lpt : data for pad 4)
    PPortHelp 0F9h, 08h, 1B2h
    PPortHelp 0F9h, 08h, 1B3h
    PPortHelp 0F9h, 08h, 1B4h
    PPortHelp 0F9h, 08h, 1B5h
    PPortHelp 0F9h, 08h, 1B6h
    PPortHelp 0F9h, 08h, 1B7h
    PPortHelp 0F9h, 08h, 1B8h
    PPortHelp 0F9h, 08h, 1B9h
    PPortHelp 0F9h, 08h, 1BAh
    PPortHelp 0F9h, 08h, 1BBh
    ret

GetParallelPlayer5:
    mov dx, 0378h
    mov byte[pressed+1c0h],0
    mov byte[pressed+1c1h],0
    mov byte[pressed+1c2h],0
    mov byte[pressed+1c3h],0
    mov byte[pressed+1c4h],0
    mov byte[pressed+1c5h],0
    mov byte[pressed+1c6h],0
    mov byte[pressed+1c7h],0
    mov byte[pressed+1c8h],0
    mov byte[pressed+1c9h],0
    mov byte[pressed+1cAh],0
    mov byte[pressed+1cBh],0
    PPortHelpInv 0FAh, 80h, 1c0h
    PPortHelpInv 0F9h, 80h, 1c1h ;Mask 80h (pin 11 of lpt : data for pad 5)
    PPortHelpInv 0F9h, 80h, 1c2h
    PPortHelpInv 0F9h, 80h, 1c3h
    PPortHelpInv 0F9h, 80h, 1c4h
    PPortHelpInv 0F9h, 80h, 1c5h
    PPortHelpInv 0F9h, 80h, 1c6h
    PPortHelpInv 0F9h, 80h, 1c7h
    PPortHelpInv 0F9h, 80h, 1c8h
    PPortHelpInv 0F9h, 80h, 1c9h
    PPortHelpInv 0F9h, 80h, 1cAh
    PPortHelpInv 0F9h, 80h, 1cBh
    ret

%macro SetDefaultKey2 13
  mov dword[%1upk],%4    ; Up
  mov dword[%1downk],%5  ; Down
  mov dword[%1leftk],%6  ; Left
  mov dword[%1rightk],%7 ; Right
  mov dword[%1startk],%3 ; Start
  mov dword[%1selk],%2   ; Select
  mov dword[%1Ak],%9     ; A
  mov dword[%1Bk],%12    ; B
  mov dword[%1Xk],%8     ; X
  mov dword[%1Yk],%11    ; Y
  mov dword[%1Lk],%10    ; L
  mov dword[%1Rk],%13    ; R
%endmacro

%macro SetDefaultKey 12
  cmp bh,0
  jne %%nopl1
  SetDefaultKey2 pl1,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12
%%nopl1
  cmp bh,1
  jne %%nopl2
  SetDefaultKey2 pl2,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12
%%nopl2
  cmp bh,2
  jne %%nopl3
  SetDefaultKey2 pl3,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12
%%nopl3
  cmp bh,3
  jne %%nopl4
  SetDefaultKey2 pl4,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12
%%nopl4
  cmp bh,4
  jne %%nopl5
  SetDefaultKey2 pl5,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12
%%nopl5
%endmacro

NEWSYM SetInputDevice209
    ; bl = device #, bh = player # (0-4)
    ; Sets keys according to input device selected
    cmp bl,0
    jne near .nozero
    SetDefaultKey 0,0,0,0,0,0,0,0,0,0,0,0
    ret
.nozero
    cmp bl,1
    jne near .nokeyb
    cmp bh,1
    ja near .exit
    cmp bh,1
    je near .input2
    SetDefaultKey 54,28,72,80,75,77,31,45,32,30,44,46
    ret
.input2
    SetDefaultKey 56,29,37,50,49,51,82,71,73,83,79,81
    ret
.nokeyb
    cmp bl,2
    jne near .no2buttons
    xor bl,bl
    cmp byte[pl1contrl],2
    jne .nopl2a
    inc bl
.nopl2a
    cmp byte[pl2contrl],2
    jne .nopl2b
    inc bl
.nopl2b
    cmp byte[pl3contrl],2
    jne .nopl2c
    inc bl
.nopl2c
    cmp byte[pl4contrl],2
    jne .nopl2d
    inc bl
.nopl2d
    cmp byte[pl5contrl],2
    jne .nopl2e
    inc bl
.nopl2e
    cmp bl,2
    jae .2ndjoyst
    SetDefaultKey 0,0,14Ch,14Dh,14Eh,14Fh,0,0,0,102h,103h,0
    ret
.2ndjoyst
    SetDefaultKey 0,0,168h,169h,16Ah,16Bh,0,0,0,104h,105h,0
    ret
.no2buttons
    cmp bl,3
    jne near .no4buttons
    SetDefaultKey 0,0,14Ch,14Dh,14Eh,14Fh,104h,105h,0,102h,103h,0
    ret
.no4buttons
    cmp bl,4
    jne near .no6buttons
    SetDefaultKey 0,0,14Ch,14Dh,14Eh,14Fh,104h,105h,106h,102h,103h,107h
    ret
.no6buttons
    cmp bl,5
    jne near .skip8b
    SetDefaultKey 101h,100h,14Ch,14Dh,14Eh,14Fh,104h,105h,107h,102h,103h,106h
    ret
.skip8b
    cmp bl,6
    jne near .nosw1
    SetDefaultKey 149h,148h,154h,155h,156h,157h,10Ch,109h,10Eh,10Bh,108h,10Fh
    ret
.nosw1
    cmp bl,7
    jne near .nosw2
    SetDefaultKey 149h+8,148h+8,154h+8,155h+8,156h+8,157h+8,10Ch+8,109h+8,10Eh+8,10Bh+8,108h+8,10Fh+8
    ret
.nosw2
    cmp bl,8
    jne near .nosw3
    SetDefaultKey 149h+8*2,148h+8*2,154h+8*2,155h+8*2,156h+8*2,157h+8*2,10Ch+8*2,109h+8*2,10Eh+8*2,10Bh+8*2,108h+8*2,10Fh+8*2
    ret
.nosw3
    cmp bl,9
    jne near .nosw4
    SetDefaultKey 149h+8*3,148h+8*3,154h+8*3,155h+8*3,156h+8*3,157h+8*3,10Ch+8*3,109h+8*3,10Eh+8*3,10Bh+8*3,108h+8*3,10Fh+8*3
    ret
.nosw4
    cmp bl,10
    jne near .nogrip0
    SetDefaultKey 14Ah,14Bh,170h,171h,172h,173h,129h,12Bh,12Ch,128h,12Ah,12Eh
    ret
.nogrip0
    cmp bl,11
    jne near .nogrip1
    SetDefaultKey 14Ah+8,14Bh+8,170h+4,171h+4,172h+4,173h+4,129h+8,12Bh+8,12Ch+8,128h+8,12Ah+8,12Eh+8
    ret
.nogrip1
    cmp bl,12
    jne near .nopp1
    SetDefaultKey 182h,183h,184h,185h,186h,187h,189h,188h,18Ah,181h,180h,18Bh
    ret
.nopp1
    cmp bl,13
    jne near .nopp2
    SetDefaultKey 192h,193h,194h,195h,196h,197h,199h,198h,19Ah,191h,190h,19Bh
    ret
.nopp2
    cmp bl,14
    jne near .nopp3
    SetDefaultKey 1A2h,1A3h,1A4h,1A5h,1A6h,1A7h,1A9h,1A8h,1AAh,1A1h,1A0h,1ABh
    ret
.nopp3
    cmp bl,15
    jne near .nopp4
    SetDefaultKey 1B2h,1B3h,1B4h,1B5h,1B6h,1B7h,1B9h,1B8h,1BAh,1B1h,1B0h,1BBh
    ret
.nopp4
    cmp bl,16
    jne near .nopp5
    SetDefaultKey 1C2h,1C3h,1C4h,1C5h,1C6h,1C7h,1C9h,1C8h,1CAh,1C1h,1C0h,1CBh
    ret
.nopp5
.exit
    ret
