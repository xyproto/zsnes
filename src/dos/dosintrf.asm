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

EXTSYM selcA000,selcB800,selc0040,previdmode,DosExit,
EXTSYM V8Mode,getblaster,Force8b,SBHDMA
EXTSYM oldhand9s,oldhand9o,interror,oldhand8s,oldhand8o,oldhandSBs,oldhandSBo
EXTSYM NoSoundReinit,soundon,DSPDisable,SBInt,PICMaskP,SBIrq,SBHandler,InitSB
EXTSYM handler8h,handler9h,init60hz,Interror,init18_2hz,DeInitSPC,GUIinit36_4hz
EXTSYM GUIoldhand9s,GUIoldhand9o,GUIoldhand8s,GUIoldhand8o,GUIhandler9h
EXTSYM GUIhandler8h,GUIinit18_2hz,dosinitvideo
EXTSYM DosDrawScreen,cvidmode,vidbuffer,GUICPC,DosDrawScreenB
EXTSYM DosUpdateDevices,DOSJoyRead,pl1contrl,pl2contrl,pl3contrl,pl4contrl
EXTSYM pl5contrl,GrayscaleMode
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

; NOTE: For timing, Game60hzcall should be called at 50hz or 60hz (depending
;   on romispal) after a call to InitPreGame and before DeInitPostGame are
;   made.  GUI36hzcall should be called at 36hz after a call GUIInit and
;   before GUIDeInit.

SECTION .data
NEWSYM dssel, dw 0
SECTION .text

NEWSYM StartUp
    mov    ax,901h             ;enable interrupts
    int    31h
    mov ax,ds
    mov [dssel],ax
    mov bx,0A000h
    call findselec
    mov [selcA000],ax
    mov bx,0B800h
    call findselec
    mov [selcB800],ax
    mov bx,0040h
    call findselec
    mov [selc0040],ax
    ; get previous video mode
    xor ecx,ecx
    push es
    mov ax,[selc0040]
    mov es,ax
    mov al,[es:49h]
    mov [previdmode],al
    pop es

    ; Get base address
    mov ax,ds
    mov bx,ax
    mov eax,0006h
    int 31h
    jc .FatalError
    mov [ZSNESBase+2],cx                ; These variables are used for
    mov [ZSNESBase],dx                  ; memory allocation so they can be
.FatalError                             ; ignored for non-DOS ports
    ret

; SystemInit - Initialize all Joystick stuff, load in all configuration data,
;   parse commandline data, obtain current directory (One time initialization)
NEWSYM SystemInit
    ; Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    push es

    call getblaster                     ; get set blaster environment
    cmp byte[Force8b],1
    jne .noforce8b
    mov byte[SBHDMA],0
.noforce8b
    pop es
    ret

; Find Selector - DOS only
findselec:
    mov ax, 2
    int 31h
    jnc .proceed
    mov edx, .noselector
    call PrintStr
    jmp DosExit
.proceed
    ret
SECTION .data
.noselector db 'Cannot find selector!',10,13,0
SECTION .text

NEWSYM PrintChar
    ; print character at dl, push all modified registers
    push eax
    mov ah,02h
    int 21h
    pop eax
    ret

NEWSYM PrintStr          ; Print ASCIIZ string
.next
    mov al,[edx]
    or al,al
    jz .finish
    push edx
    mov dl,al
    mov ah,02h
    int 21h
    pop edx
    inc edx
    jmp .next
.finish
    ret

NEWSYM WaitForKey       ; Wait for a key to be pressed
    mov ah,7
    int 21h
    ; return key in al
    ret

%macro PressConv 3
    cmp byte[pressed+%1],0
    je %%nopr
    test byte[prval],%2
    jnz %%prskip
    or byte[prval],%2
    mov byte[prres],%3
;    mov al,0FFh
    jmp .done
    jmp %%prskip
%%nopr
    and byte[prval],%2^0FFh
%%prskip
%endmacro

NEWSYM Check_Key
;    xor al,al
;    PressConv 75,01h,75
;    PressConv 77,02h,77
;    PressConv 80,04h,80
;    PressConv 72,08h,72
;    PressConv 1,80h,27
;.done
;    ret
    ; returns 0 if there are no keys in the keyboard buffer, 0xFF otherwise
    mov ah,0Bh
    int 21h
    ret

NEWSYM Get_Key
;    mov al,[prres]
;    ret
    ; wait if there are no keys in buffer, then return key in al
    ; for extended keys, return a 0, then the extended key afterwards
    mov ah,07h
    int 21h
    ret

NEWSYM Get_Memfree
    mov ax,0500h
    mov edi,edx
    int 31h
    ret

NEWSYM Output_Text       ; Output character (ah=02h) or string (ah=09h)
    ; This function usually displays an error message on-screen
    cmp ah,02h
    je .char
    cmp ah,09h
    je .string
    ret
.char
    int 21h     ; print dl
    ret
.string
    pushad
    call PrintStr       ; print edx
    popad
    ret

; Delay for CX/65536 of a second

NEWSYM delay
   in al,61h
   and al,10h
   mov ah,al
.loopa
   in al,61h
   and al,10h
   cmp al,ah
   jz .loopa
   mov ah,al
   dec cx
   jnz .loopa
   ret

NEWSYM InitPreGame   ; Executes before starting/continuing a game
    ; set up interrupt handler
    ; get old handler pmode mode address
    ; Process stuff such as sound init, interrupt initialization
    cli
    mov ax,204h
    mov bl,09h
    int 31h
    jc near interror
    mov [oldhand9s],cx
    mov [oldhand9o],edx
    mov ax,204h
    mov bl,08h
    int 31h
    jc near interror
    mov [oldhand8s],cx
    mov [oldhand8o],edx

    mov al,[GrayscaleMode]
    cmp al,[V8Mode]
    je .nochangemode
    xor byte[V8Mode],1
    xor al,al
.nochangemode

.nofs
    cmp byte[NoSoundReinit],1
    je .nosound
    cmp byte[soundon],0
    je .nosound
    cmp byte[DSPDisable],1
    je .nosound
    mov ax,204h
    mov bl,[SBInt]
    int 31h
    jc near Interror
    mov [oldhandSBs],cx
    mov [oldhandSBo],edx
.nosound
    sti
    ret

NEWSYM SetupPreGame   ; Executes after pre-game init, can execute multiple
                      ; times after a single InitPreGame
    ; set new handler
    cmp byte[soundon],0
    je near .nosound2
    cmp byte[DSPDisable],1
    je near .nosound2

    ; Turn off IRQ through controller
    cli
    xor dh,dh
    mov dl,[PICMaskP]
    mov cl,[SBIrq]
    and cl,07h
    mov al,01h
    shl al,cl
    mov bl,al
    in al,dx
    or al,bl
    out dx,al

    mov ax,205h
    mov bl,[SBInt]
    mov cx,cs
    mov edx,SBHandler
    int 31h
    jc near interror

    ; Turn on IRQ through controller
    xor dh,dh
    mov dl,[PICMaskP]
    mov cl,[SBIrq]
    and cl,07h
    mov al,01h
    shl al,cl
    not al
    mov bl,al
    in al,dx
    and al,bl
    out dx,al

    call InitSB
    sti
.nosound2
    cli
    mov ax,205h
    mov bl,09h
    mov cx,cs           ; Requires CS rather than DS
    mov edx,handler9h
    int 31h
    jc near interror

    mov ax,205h
    mov bl,08h
    mov cx,cs           ; Requires CS rather than DS
    mov edx,handler8h
    int 31h
    jc near interror
    call init60hz               ; Set timer to 60/50Hz
.nofs2
    sti
    ret

NEWSYM DeInitPostGame           ; Called after game is ended
    ; de-init interrupt handler
    cli
    mov cx,[oldhand9s]
    mov edx,[oldhand9o]
    mov ax,205h
    mov bl,09h
    int 31h
    jc near interror

    mov cx,[oldhand8s]
    mov edx,[oldhand8o]
    mov ax,205h
    mov bl,08h
    int 31h
    jc near interror
    call init18_2hz               ; Set timer to 18.2Hz
.nofs3
    sti

    ; DeINITSPC
    cmp byte[soundon],0
    je .nosoundb
    cmp byte[DSPDisable],1
    je .nosoundb
    call DeInitSPC
    mov cx,[oldhandSBs]
    mov edx,[oldhandSBo]
    mov ax,205h
    mov bl,[SBInt]
    int 31h
    jc near interror
.nosoundb
    ret

NEWSYM GUIInit
    mov ax,204h
    mov bl,09h
    int 31h
    mov [GUIoldhand9s],cx
    mov [GUIoldhand9o],edx
    mov ax,204h
    mov bl,08h
    int 31h
    mov [GUIoldhand8s],cx
    mov [GUIoldhand8o],edx
    mov ax,205h
    mov bl,09h
    mov cx,cs
    mov edx,GUIhandler9h
    int 31h
    mov ax,205h
    mov bl,08h
    mov cx,cs
    mov edx,GUIhandler8h
    int 31h
    call GUIinit36_4hz
    ret

NEWSYM GUIDeInit
    mov cx,[GUIoldhand9s]
    mov edx,[GUIoldhand9o]
    mov ax,205h
    mov bl,09h
    int 31h
    mov cx,[GUIoldhand8s]
    mov edx,[GUIoldhand8o]
    mov ax,205h
    mov bl,08h
    int 31h
    call GUIinit18_2hz
    ret

; ****************************
; Video Stuff
; ****************************

; ** Palette Functions **
NEWSYM displayfpspal
    mov al,128
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,63
    out dx,al
    out dx,al
    out dx,al
    mov al,128+64
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,0
    out dx,al
    out dx,al
    out dx,al
    ret
NEWSYM superscopepal
    mov al,128+16
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,63
    out dx,al
    xor al,al
    out dx,al
    out dx,al
    ret
NEWSYM saveselectpal
    ; set palette of colors 128,144, and 160 to white, blue, and red
    mov al,128
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,63
    out dx,al
    out dx,al
    out dx,al
    mov al,144
    mov dx,03C8h
    out dx,al
    inc dx
    xor al,al
    out dx,al
    out dx,al
    mov al,50
    out dx,al
    mov al,160
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,45
    out dx,al
    xor al,al
    out dx,al
    out dx,al
    mov al,176
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,47
    out dx,al
    xor al,al
    out dx,al
    out dx,al
    mov al,208
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,50
    out dx,al
    mov al,25
    out dx,al
    xor al,al
    out dx,al
    ret

; ** init video mode functions **
NEWSYM initvideo  ; Returns 1 in videotroub if trouble occurs
    jmp dosinitvideo

NEWSYM deinitvideo
    mov al,[previdmode]
    mov ah,0
    int 10h
    ret

; ** copy video mode functions **
NEWSYM DrawScreen               ; In-game screen render w/ triple buffer check
   jmp DosDrawScreen

NEWSYM vidpastecopyscr       ; GUI screen render
;   jmp dosvidpastecopyscr
   pushad
   movzx eax,byte[cvidmode]
   cmp byte[GUI16VID+eax],1
   jne .no16bconv
   mov eax,[vidbuffer]
   mov ecx,224*288
   mov edx,ecx
   sub ecx,288
   dec edx
.loop
   movzx ebx,byte[eax+edx]
   mov bx,[GUICPC+ebx*2]
   mov [eax+edx*2],bx
   dec edx
   dec ecx
   jnz .loop
.no16bconv
   popad
   jmp DosDrawScreenB

; ** Video Mode Variables **
SECTION .data

; Total Number of Video Modes
NEWSYM NumVideoModes, dd 19

; GUI Video Mode Names - Make sure that all names are of the same length
; and end with a NULL terminator
NEWSYM GUIVideoModeNames
db '256x224x8B  MODEQ',0    ; 0
db '256x240x8B  MODEQ',0    ; 1
db '256x256x8B  MODEQ',0    ; 2
db '320x224x8B  MODEX',0    ; 3
db '320x240x8B  MODEX',0    ; 4
db '320x256x8B  MODEX',0    ; 5
db '640x480x16B VESA1',0    ; 6
db '320x240x8B  VESA2',0    ; 7
db '320x240x16B VESA2',0    ; 8
db '320x480x8B  VESA2',0    ; 9
db '320x480x16B VESA2',0    ; 10
db '512x384x8B  VESA2',0    ; 11
db '512x384x16B VESA2',0    ; 12
db '640x400x8B  VESA2',0    ; 13
db '640x400x16B VESA2',0    ; 14
db '640x480x8B  VESA2',0    ; 15
db '640x480x16B VESA2',0    ; 16
db '800x600x8B  VESA2',0    ; 17
db '800x600x16B VESA2',0    ; 18

; Video Mode Feature Availability (1 = Available, 0 = Not Available)
; Left side starts with Video Mode 0
;                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
NEWSYM GUI16VID,  db 0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1 ; 16-bit mode
NEWSYM GUISLVID,  db 0,0,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,1,1 ; Scanlines
NEWSYM GUIHSVID,  db 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0 ; Half/Quarter Scanlines
NEWSYM GUII2VID,  db 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1 ; DOS Interpolation
NEWSYM GUIEAVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 ; DOS Eagle
NEWSYM GUITBVID,  db 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1 ; DOS Triple Buffering
NEWSYM GUIFSVID,  db 0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0 ; DOS Fullscreen
NEWSYM GUIWSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0 ; DOS Widescreen
NEWSYM GUISSVID,  db 0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1 ; DOS Smallscreen
NEWSYM GUI2xVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 ; 2xSaI/Super Engines
NEWSYM GUIM7VID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 ; Hires Mode 7
NEWSYM GUIHQ2X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; (Hq2x Filter)
NEWSYM GUINTVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; (NTSC Filter)

SECTION .text

; ****************************
; Input Device Stuff
; ****************************

; Variables related to Input Device Routines:
;   pl1selk,pl1startk,pl1upk,pl1downk,pl1leftk,pl1rightk,pl1Xk,
;   pl1Ak,pl1Lk,pl1Yk,pl1Bk,pl1Rk
;     (Change 1 to 2,3,4 for other players)
;     Each of these variables contains the corresponding key pressed value
;       for the key data
;   pressed[]
;     - This is an array of pressed/released data (bytes) where the
;       corresponding key pressed value is used as the index.  The value
;       for each entry is 0 for released and 1 for pressed.  Also, when
;       writing keyboard data to this array, be sure to first check if
;       the value of the array entry is 2 or not.  If it is 2, do not write 1
;       to that array entry. (however, you can write 0 to it)
;   As an example, to access Player 1 L button press data, it is
;     done like : pressed[pl1Lk]
;   The 3 character key description of that array entry is accessed by the
;     GUI through ScanCodeListing[pl1Lk*3]

; Note: When storing the input device configuration of a dynamic input
;   device system (ie. Win9x) rather than a static system (ie. Dos), it
;   is best to store in the name of the device and relative button
;   assignments in the configuration file, then convert it to ZSNES'
;   numerical corresponding key format after reading from it. And then
;   convert it back when writing to it back.

NEWSYM UpdateDevices                    ; One-time input device init
        call DosUpdateDevices
        ret

NEWSYM JoyRead
        call DOSJoyRead
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

NEWSYM SetInputDevice
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
    SetDefaultKey 56,29,36,50,49,51,82,71,73,83,79,81
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
    SetDefaultKey 0,0,0CCh,0CDh,0CEh,0CFh,0,0,0,82h,83h,0
    ret
.2ndjoyst
    SetDefaultKey 0,0,0E8h,0E9h,0EAh,0EBh,0,0,0,84h,85h,0
    ret
.no2buttons
    cmp bl,3
    jne near .no4buttons
    SetDefaultKey 0,0,0CCh,0CDh,0CEh,0CFh,84h,85h,0,82h,83h,0
    ret
.no4buttons
    cmp bl,4
    jne near .no6buttons
    SetDefaultKey 0,0,0CCh,0CDh,0CEh,0CFh,84h,85h,86h,82h,83h,87h
    ret
.no6buttons
    cmp bl,5
    jne near .skip8b
    SetDefaultKey 081h,080h,0CCh,0CDh,0CEh,0CFh,84h,85h,87h,82h,83h,86h
    ret
.skip8b
    cmp bl,6
    jne near .nosw1
    SetDefaultKey 0C9h,0C8h,0D4h,0D5h,0D6h,0D7h,08Ch,089h,08Eh,08Bh,088h,08Fh
    ret
.nosw1
    cmp bl,7
    jne near .nosw2
    SetDefaultKey 0C9h+8,0C8h+8,0D4h+8,0D5h+8,0D6h+8,0D7h+8,08Ch+8,089h+8,08Eh+8,08Bh+8,088h+8,08Fh+8
    ret
.nosw2
    cmp bl,8
    jne near .nosw3
    SetDefaultKey 0C9h+8*2,0C8h+8*2,0D4h+8*2,0D5h+8*2,0D6h+8*2,0D7h+8*2,08Ch+8*2,089h+8*2,08Eh+8*2,08Bh+8*2,088h+8*2,08Fh+8*2
    ret
.nosw3
    cmp bl,9
    jne near .nosw4
    SetDefaultKey 0C9h+8*3,0C8h+8*3,0D4h+8*3,0D5h+8*3,0D6h+8*3,0D7h+8*3,08Ch+8*3,089h+8*3,08Eh+8*3,08Bh+8*3,088h+8*3,08Fh+8*3
    ret
.nosw4
    cmp bl,10
    jne near .nogrip0
    SetDefaultKey 0CAh,0CBh,0F0h,0F1h,0F2h,0F3h,0A9h,0ABh,0ACh,0A8h,0AAh,0AEh
    ret
.nogrip0
    cmp bl,11
    jne near .nogrip1
    SetDefaultKey 0CAh+8,0CBh+8,0F0h+4,0F1h+4,0F2h+4,0F3h+4,0A9h+8,0ABh+8,0ACh+8,0A8h+8,0AAh+8,0AEh+8
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

SECTION .data

; Total Number of Input Devices
NEWSYM NumInputDevices, dd 17

; Input Device Names
NEWSYM GUIInputNames
db 'NONE            ',0
db 'KEYBOARD        ',0
db '2BUTTONJOYSTICK ',0
db '4BUTTONJOYSTICK ',0
db '6BUTTONJOYSTICK ',0
db '8BUTTONJOYSTICK ',0
db 'SIDEWINDERPAD1  ',0
db 'SIDEWINDERPAD2  ',0
db 'SIDEWINDERPAD3  ',0
db 'SIDEWINDERPAD4  ',0
db 'GAMEPAD PRO P0  ',0
db 'GAMEPAD PRO P1  ',0
db 'PARALLEL LPT1 P1',0
db 'PARALLEL LPT1 P2',0
db 'PARALLEL LPT1 P3',0
db 'PARALLEL LPT1 P4',0
db 'PARALLEL LPT1 P5',0

; GUI Description codes for each corresponding key pressed value
NEWSYM ScanCodeListing
        db '---','ESC',' 1 ',' 2 ',' 3 ',' 4 ',' 5 ',' 6 '
        db ' 7 ',' 8 ',' 9 ',' 0 ',' - ',' = ','BKS','TAB'
        db ' Q ',' W ',' E ',' R ',' T ',' Y ',' U ',' I '
        db ' O ',' P ',' [ ',' ] ','RET','CTL',' A ',' S '
        db ' D ',' F ',' G ',' H ',' J ',' K ',' L ',' : '
        db ' " ',' ~ ','LSH',' \ ',' Z ',' X ',' C ',' V '
        db ' B ',' N ',' M ',' , ',' . ',' / ','RSH',' * '
        db 'ALT','SPC','CAP','F1 ','F2 ','F3 ','F4 ','F5 '
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','HOM'
        db 'UP ','PGU',' - ','LFT',' 5 ','RGT',' + ','END'
        db 'DWN','PGD','INS','DEL','   ','   ','   ','F11'
        db 'F12','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        ; Joystick Stuff, Port 201h (80h)
        db 'JB7','JB8','JB1','JB2','JB3','JB4','JB5','JB6'
        db 'SWA','SWB','SWC','SWX','SWY','SWZ','SWL','SWR'
        db 'S2A','S2B','S2C','S2X','S2Y','S2Z','S2L','S2R'
        db 'S3A','S3B','S3C','S3X','S3Y','S3Z','S3L','S3R'
        db 'S4A','S4B','S4C','S4X','S4Y','S4Z','S4L','S4R'
        db 'GRR','GRB','GRY','GRG','GL1','GL2','GR1','GR2'
        db 'G2R','G2B','G2Y','G2G','2L1','2L2','2R1','2R2'
        db 'G3R','G3B','G3Y','G3G','3L1','3L2','3R1','3R2'
        db 'G4R','G4B','G4Y','G4G','4L1','4L2','4R1','4R2'
        db 'SWS','SWM','GSL','GST','JUP','JDN','JLF','JRG'
        db 'S2S','S2M','2SL','2ST','SWU','SWD','SWL','SWR'
        db 'S3S','S3M','3SL','3ST','S2U','S2D','S2L','S2R'
        db 'S4S','S4M','4SL','4ST','S3U','S3D','S3L','S3R'
        db 'J2U','J2D','J2L','J2R','S4U','S4D','S4L','S4R'
        db 'GRU','GRD','GRL','GRR','G2U','G2D','G2L','G2R'
        db 'G3U','G3D','G3L','G3R','G4U','G4D','G4L','G4R'
        ; Joystick Stuff, Port 209h (100h)
        db 'JB7','JB8','JB1','JB2','JB3','JB4','JB5','JB6'
        db 'SWA','SWB','SWC','SWX','SWY','SWZ','SWL','SWR'
        db 'S2A','S2B','S2C','S2X','S2Y','S2Z','S2L','S2R'
        db 'S3A','S3B','S3C','S3X','S3Y','S3Z','S3L','S3R'
        db 'S4A','S4B','S4C','S4X','S4Y','S4Z','S4L','S4R'
        db 'GRR','GRB','GRY','GRG','GL1','GL2','GR1','GR2'
        db 'G2R','G2B','G2Y','G2G','2L1','2L2','2R1','2R2'
        db 'G3R','G3B','G3Y','G3G','3L1','3L2','3R1','3R2'
        db 'G4R','G4B','G4Y','G4G','4L1','4L2','4R1','4R2'
        db 'SWS','SWM','GSL','GST','JUP','JDN','JLF','JRG'
        db 'S2S','S2M','2SL','2ST','SWU','SWD','SWL','SWR'
        db 'S3S','S3M','3SL','3ST','S2U','S2D','S2L','S2R'
        db 'S4S','S4M','4SL','4ST','S3U','S3D','S3L','S3R'
        db 'J2U','J2D','J2L','J2R','S4U','S4D','S4L','S4R'
        db 'GRU','GRD','GRL','GRR','G2U','G2D','G2L','G2R'
        db 'G3U','G3D','G3L','G3R','G4U','G4D','G4L','G4R'
        ; Extra Stuff (180h) (Parallel Port)
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
        db 'P3B','P3Y','P3S','P3T','P3U','P3D','P3L','P3R'
        db 'P3A','P3X','P3L','P3R','   ','   ','   ','   '
        db 'P4B','P4Y','P4S','P4T','P4U','P4D','P4L','P4R'
        db 'P4A','P4X','P4L','P4R','   ','   ','   ','   '
        db 'P5B','P5Y','P5S','P5T','P5U','P5D','P5L','P5R'
        db 'P5A','P5X','P5L','P5R','   ','   ','   ','   '

SECTION .bss

NEWSYM ZSNESBase, resd 1
TempVarSeek resd 1


SECTION .text

; ****************************
; Mouse Stuff
; ****************************

NEWSYM Init_Mouse
    ; return non-zero if successful
    mov eax,00h
    int 33h
    cmp ax,0
    je .nomouse
    mov eax,07h
    mov ecx,0
    mov edx,255
    int 33h
    mov eax,08h
    mov ecx,0
    mov edx,223
    int 33h
    mov eax,0Fh
    mov ecx,8
    mov edx,8
    int 33h
    mov eax,04h
    mov ecx,0
    mov edx,0
    int 33h
    mov ax,1
.nomouse
    ret

NEWSYM Get_MouseData         ; Returns both pressed and coordinates
    mov eax,03h
    int 33h
    ; bx : bit 0 = left button, bit 1 = right button
    ; cx = Mouse X Position, dx = Mouse Y Position
    ret

NEWSYM Set_MouseXMax    ; Sets the X boundaries (ecx = left, edx = right)
    mov eax,07h
    int 33h
    ret

NEWSYM Set_MouseYMax    ; Sets the Y boundaries (ecx = left, edx = right)
    mov eax,08h
    int 33h
    ret

NEWSYM Set_MousePosition        ; Sets Mouse Position (x:cx,y:dx)
    mov eax,04h
    int 33h
    ret

NEWSYM Get_MousePositionDisplacement
    ; returns x,y displacement in pixel in cx,dx
    mov eax,0Bh
    int 33h
    ret

NEWSYM MouseWindow
    ret

; ****************************
; Sound Stuff
; ****************************

NEWSYM StopSound
    ret

NEWSYM StartSound
    ret

NEWSYM Check60hz
    ; Call the timer update function here
    ret
