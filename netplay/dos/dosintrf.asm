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

%include "macros.mac"

EXTSYM selcA000,selcB800,selc0040,previdmode,DosExit,ZFileSystemInit
EXTSYM getcmdline,GUIRestoreVars,getcfg,obtaindir,ConvertJoyMap,tparms
EXTSYM preparedir,getblaster,Force8b,SBHDMA
EXTSYM ccmdline
EXTSYM InitDir,InitDrive
EXTSYM DOScreatenewcfg,ExecGUISaveVars
EXTSYM allocptr
EXTSYM ZOpenFile,ZOpenMode,CurrentHandle,ZFileSeek,ZOpenFileName,ZFileSeekMode
EXTSYM ZFileSeekPos,ZFileSeekHandle
EXTSYM ZFileWriteHandle,ZFileWriteSize,ZFileWriteBlock,ZFileWrite
EXTSYM ZFileReadHandle,ZFileReadSize,ZFileReadBlock,ZFileRead
EXTSYM ZFileDelFName,ZFileDelete
EXTSYM ZCloseFileHandle,ZCloseFile
EXTSYM ZFileTellHandle,ZFileTell
EXTSYM GetTime
EXTSYM GetDate
EXTSYM ZFFTimeFName,ZFTime,ZFDate,ZFileGetFTime
EXTSYM pressed
EXTSYM ZFileMKDir,ZFileCHDir,ZFileRMDir,CHPath,MKPath,RMPath
EXTSYM ZFileGetDir,DriveNumber,DirName
EXTSYM DTALoc,DTALocPos,ZFileFindATTRIB,ZFileFindFirst,ZFileFindNext,ZFileFindPATH
EXTSYM oldhand9s,oldhand9o,interror,oldhand8s,oldhand8o,oldhandSBs,oldhandSBo
EXTSYM NoSoundReinit,soundon,DSPDisable,SBInt,NoSoundReinit,PICMaskP,SBIrq
EXTSYM SBHandler,InitSB,handler8h,handler9h,init60hz,Interror,init18_2hz,DeInitSPC
EXTSYM GUIinit36_4hz,GUIoldhand9s,GUIoldhand9o,GUIoldhand8s,GUIoldhand8o
EXTSYM GUIhandler9h,GUIhandler8h,GUIinit18_2hz
EXTSYM dosmakepal
EXTSYM doschangepal
EXTSYM dosinitvideo, dosinitvideo2
EXTSYM DosDrawScreen,dosvidpastecopyscr,cvidmode,vidbuffer,GUICPC,DosDrawScreenB
EXTSYM DOSClearScreen
EXTSYM DosUpdateDevices
EXTSYM DOSJoyRead
EXTSYM pl1contrl,pl2contrl,pl3contrl,pl4contrl,pl5contrl
EXTSYM BufferSizeW,BufferSizeB,ProcessSoundBuffer
EXTSYM TCPIPPortNum
EXTSYM V8Mode,GrayscaleMode
;    EXTSYM InitTCP
;    EXTSYM StartServer
;    EXTSYM InitTCP
EXTSYM TCPIPAddress
;    EXTSYM ConnectServer
;    EXTSYM SendData
;    EXTSYM GetData
;    EXTSYM DeInitTCP
;    EXTSYM StopServer
;    EXTSYM Disconnect








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
    call ZFileSystemInit
    ret

; SystemInit - Initialize all Joystick stuff, load in all configuration data,
;   parse commandline data, obtain current directory (One time initialization)
NEWSYM SystemInit
    ; Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    push es
    call getcmdline

    ; Get and set the initial directory
    mov ebx,InitDir
    mov edx,InitDrive
    call Get_Dir
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir

    call GUIRestoreVars                 ; Load GUI stuff
    call getcfg                         ; Load cfg stuff
    call obtaindir                      ; Get Save/Init Directories
    call ConvertJoyMap                  ; Mini joystick init
    call ccmdline
    call tparms
    call preparedir
    call getblaster                     ; get set blaster environment
    cmp byte[Force8b],1
    jne .noforce8b
    mov byte[SBHDMA],0
.noforce8b
    pop es
    ret

; Configuration save re-routing functions.  You can comment these out
;   for debugging purposes or change it if you're using a different
;   configuration format
NEWSYM createnewcfg
    call DOScreatenewcfg
    ret
NEWSYM GUISaveVars
    call ExecGUISaveVars
    ret

; Allocate memory - see allocptr in ui.asm for details on what to allocate
NEWSYM allocmem
    call allocptr
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

NEWSYM OSExit
    jmp DosExit

NEWSYM Open_File
    pushad
    mov dword[ZOpenMode],0
    mov dword[ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov dword[ZFileSeekHandle],0
    mov bx,[CurrentHandle]
    dec bx
    mov word[ZFileSeekHandle],bx
    call ZFileSeek
    popad
    mov ax,[CurrentHandle]
    dec ax
    clc
    ret
.error
    popad
    stc
    ret
    mov ax,3D00h
    int 21h
    ; return ax = file handle, carry = error
    ret

NEWSYM Open_File_Write
    pushad
    mov dword[ZOpenMode],2
    mov dword[ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov dword[ZFileSeekHandle],0
    mov bx,[CurrentHandle]
    dec bx
    mov word[ZFileSeekHandle],bx
    call ZFileSeek
    popad
    mov ax,[CurrentHandle]
    dec ax
    clc
    ret
.error
    popad
    stc
    ret
    mov ax,3D01h
    int 21h
    ; return ax = file handle, carry = error
    ret

NEWSYM Create_File
    pushad
    mov dword[ZOpenMode],1
    mov dword[ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    popad
    mov ax,[CurrentHandle]
    dec ax
    clc
    ret
.error
    popad
    stc
    ret
    mov ah,3Ch
    mov cx,0
    int 21h
    ; return ax = file handle
    ret

NEWSYM Write_File
    mov dword[ZFileWriteHandle],0
    mov [ZFileWriteHandle],bx
    mov [ZFileWriteSize],ecx
    mov [ZFileWriteBlock],edx
    pushad
    call ZFileWrite
    cmp eax,0FFFFFFFFh
    je .fail
    popad
    mov eax,1
    clc
    ret
.fail
    popad
    mov eax,0
    stc
    ret
    mov ah,40h
    int 21h
    ret

NEWSYM Read_File
    mov dword[ZFileReadHandle],0
    mov [ZFileReadHandle],bx
    mov [ZFileReadSize],ecx
    mov [ZFileReadBlock],edx
    pushad
    call ZFileRead
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    clc
    ret
    mov ah,3Fh
    int 21h
    ret

NEWSYM Delete_File
    mov [ZFileDelFName],edx
    pushad
    call ZFileDelete
    popad
    ret
    mov ah,41h
    int 21h
    ret

NEWSYM Close_File
    mov dword[ZCloseFileHandle],0
    mov [ZCloseFileHandle],bx
    pushad
    call ZCloseFile
    popad
    clc
    ret
    mov ah,3Eh
    int 21h
    ret

NEWSYM File_Seek
    mov word[ZFileSeekPos+2],cx
    mov word[ZFileSeekPos],dx
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekHandle],0
    mov word[ZFileSeekHandle],bx
    pushad
    call ZFileSeek
    popad
    mov ax,dx
    mov dx,cx
    ret
    ; seek to cx:dx from 0 position, return carry as error
    mov ax,4200h
    int 21h
    ret

NEWSYM File_Seek_End
    mov word[ZFileSeekPos+2],cx
    mov word[ZFileSeekPos],dx
    mov dword[ZFileSeekHandle],0
    mov word[ZFileSeekHandle],bx
    mov dword[ZFileSeekMode],1
    mov dword[ZFileTellHandle],0
    mov word[ZFileTellHandle],bx
    pushad
    call ZFileSeek
    call ZFileTell
    mov [TempVarSeek],eax
    popad
    mov ax,[TempVarSeek]
    mov dx,[TempVarSeek+2]
    clc
    ret
    ; seek to cx:dx from end position, and return file location in dx:ax
    mov ax,4202h
    int 21h
    ret

NEWSYM Get_Time
    pushad
    call GetTime
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    ret

NEWSYM Get_TimeDate
    pushad
    call GetDate
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    ret

NEWSYM Get_Date
    ; dl = day, dh = month, cx = year
    mov ah,2Ah
    int 21h
    ret

NEWSYM Get_File_Date
    mov [ZFFTimeFName],edx
    pushad
    call ZFileGetFTime
    popad
    mov dx,[ZFDate]
    mov cx,[ZFTime]
    ret
    ; return packed date in dx:cx
    mov ah,57h
    mov al,00h
    int 21h
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

NEWSYM Change_Drive
    ; change to drive in dl (0 = A, 1 = B, etc.)
    mov ah,0Eh
    int 21h
    ret

NEWSYM Change_Single_Dir
    mov [CHPath],edx
    pushad
    call ZFileCHDir
    or eax,eax
    jnz .notokay
    popad
    clc
    ret
.notokay
    popad
    stc
    ret
    ; Dir in edx, return error in carry flag
    mov ah,3Bh
    int 21h
    ret

NEWSYM Create_Dir
    ; change to dir in edx
    mov [MKPath],edx
    pushad
    call ZFileMKDir
    or eax,eax
    jnz .notokay
    popad
    clc
    ret
.notokay
    popad
    stc
    ret
    mov ah,39h
    int 21h
    ret

NEWSYM Remove_Dir
    ; remove dir in edx
    mov [RMPath],edx
    pushad
    call ZFileRMDir
    or eax,eax
    jnz .notokay
    popad
    clc
    ret
.notokay
    popad
    stc
    ret
    mov ah,3Ah
    int 21h
    ret

;    mov dl,[LoadDrive]
;    mov ebx,LoadDir
;    call Change_Dir
NEWSYM Change_Dir
    pushad
    mov ah,0Eh
    int 21h
;    jc .fail
    mov dword[CHPath],gotoroot
    call ZFileCHDir
    or eax,eax
    jnz .fail
    popad
    mov [CHPath],ebx
    cmp byte[ebx],0
    je .nocdir
    pushad
    call ZFileCHDir
    or eax,eax
    jnz .fail
    popad
.nocdir
    clc
    ret
.fail
    popad
    stc
    ret

    ; dl = drive, ebx = dir
    push ebx
    mov ah,0Eh
    int 21h
    mov ah,3Bh
    mov edx,gotoroot
    int 21h
    pop ebx
    mov edx,ebx
    cmp byte[edx],0
    je .nodir
    mov ah,3Bh
    int 21h
.nodir
    ret

;    mov ebx,LoadDir
;    mov edx,LoadDrive
;    call Get_Dir
NEWSYM Get_Dir
    mov [DirName],ebx
    pushad
    call ZFileGetDir
    mov eax,[DirName]
    mov ebx,eax
    mov ecx,125
.loop
    mov dl,[eax+3]
    cmp dl,'/'
    jne .noslash
    mov dl,'\'
.noslash
    mov [eax],dl
    inc eax
    dec ecx
    jnz .loop
    popad
    push edx
    mov ah,19h
    int 21h
    pop edx
    mov [edx],al
    ret

    push edx
    mov ah,47h
    mov dl,0
    mov esi,ebx
    int 21h
    mov ah,19h
    int 21h
    pop edx
    mov [edx],al
    ret

NEWSYM Get_First_Entry
    ; cx = attributes, edx = pointer to wildcard
    ; returns : DTALoc+15h, bit 4 = Dir (1) or File (0)
    ;           DTALoc+1Eh = filename, carry flag set = no more entry
    mov [ZFileFindPATH],edx
    mov dword[ZFileFindATTRIB],0
    mov [ZFileFindATTRIB],cx
    mov dword[DTALocPos],DTALoc
    pushad
    call ZFileFindFirst
    or eax,eax
    jnz .end
    popad
    clc
    ret
.end
    popad
    stc
    ret
    mov ah,4Eh
    mov al,0
    int 21h
    ret

NEWSYM Get_Next_Entry
    mov dword[DTALocPos],DTALoc
    pushad
    call ZFileFindNext
    or eax,eax
    jnz .end
    popad
    clc
    ret
.end
    popad
    stc
    ret
    mov ah,04Fh
    int 21h
    ret

NEWSYM Set_DTA_Address
    ; Only needed for dos stuff
;    mov edx,DTALoc
;    mov ah,1Ah
;    int 21h
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
    mov cx,cs			; Requires CS rather than DS
    mov edx,handler9h
    int 31h
    jc near interror

    mov ax,205h
    mov bl,08h
    mov cx,cs			; Requires CS rather than DS
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
NEWSYM makepal  ; 8-bit palette set
    jmp dosmakepal
NEWSYM changepal  ; 8-bit palette set (changes only)
    jmp doschangepal
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
NEWSYM initvideo2 ; ModeQ scanline re-init (Keep blank on non-dos ports)
    jmp dosinitvideo2
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
   xor eax,eax
   mov al,[cvidmode]
   cmp byte[GUI16VID+eax],1
   jne .no16bconv
   mov eax,[vidbuffer]
   mov ecx,224*288
   mov edx,ecx
   sub ecx,288
   dec edx
.loop
   xor ebx,ebx
   mov bl,[eax+edx]
   mov bx,[GUICPC+ebx*2]
   mov [eax+edx*2],bx
   dec edx
   dec ecx
   jnz .loop
.no16bconv
   popad
   jmp DosDrawScreenB

; ** Clear Screen function **
NEWSYM ClearScreen
   call DOSClearScreen
   ret

; ** Video Mode Variables **
SECTION .data

; Total Number of Video Modes
NEWSYM NumVideoModes, dd 19

; GUI Video Mode Names - Make sure that all names are of the same length
; and end with a NULL terminator
NEWSYM GUIVideoModeNames
db '256X224X8B  MODEQ',0    ; 0
db '256X240X8B  MODEQ',0    ; 1
db '256X256X8B  MODEQ',0    ; 2
db '320X224X8B  MODEX',0    ; 3
db '320X240X8B  MODEX',0    ; 4
db '320X256X8B  MODEX',0    ; 5
db '640X480X16B VESA1',0    ; 6
db '320X240X8B  VESA2',0    ; 7
db '320X240X16B VESA2',0    ; 8
db '320X480X8B  VESA2',0    ; 9
db '320X480X16B VESA2',0    ; 10
db '512X384X8B  VESA2',0    ; 11
db '512X384X16B VESA2',0    ; 12
db '640X400X8B  VESA2',0    ; 13
db '640X400X16B VESA2',0    ; 14
db '640X480X8B  VESA2',0    ; 15
db '640X480X16B VESA2',0    ; 16
db '800X600X8B  VESA2',0    ; 17
db '800X600X16B VESA2',0    ; 18
db '                 ',0

; Video Mode Feature Availability (1 = Available, 0 = Not Available)
; Left side starts with Video Mode 0
;                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
NEWSYM GUI16VID,  db 0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0     ; 16-bit mode
NEWSYM GUINGVID,  db 1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0     ; New Graphics Mode Available
NEWSYM GUISLVID,  db 0,0,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0     ; Scanlines
NEWSYM GUIINVID,  db 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0     ; Interpolation
NEWSYM GUIEAVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0     ; Eagle
NEWSYM GUIIEVID,  db 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,0,1,0     ; (Interp | Eagle)
NEWSYM GUIFSVID,  db 0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0     ; Full Screen
NEWSYM GUIWSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0     ; (Full Scr. | Wide Scr.)
NEWSYM GUISSVID,  db 0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,0     ; Small Screen
NEWSYM GUITBVID,  db 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0     ; Triple Buffering
NEWSYM GUIHSVID,  db 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0     ; Half/Quarter Scanlines
NEWSYM GUI2xVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0     ; 2xSaI/Super Eagle Engines
NEWSYM GUIWFVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; If Windows Full Screen
NEWSYM GUII2VID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Super 2xSAI Engine
NEWSYM GUIM7VID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0     ; Hi-Resolution Mode 7
NEWSYM GUIBIFIL,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Bilinear Filtering
NEWSYM GUITBWVID, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; Triple Buffering (Win)
NEWSYM GUIHQ2X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0      ; HQ2X Filter
NEWSYM GUIHQ3X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0      ; HQ3X Filter
NEWSYM GUIHQ4X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0      ; HQ4X Filter
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
        call SoundProcess               ; Put the sound stuff here since it's
                                        ; called 60 times per second
        ret

NEWSYM SetInputDevice
    ; eax = pointer to devices, bl = device #, bh = player # (0-4)
    ; Sets keys according to input device selected
    cmp bl,0
    jne near .nozero
    mov dword[eax],0
    mov dword[eax+4],0
    mov dword[eax+8],0
    mov dword[eax+12],0
    mov dword[eax+16],0
    mov dword[eax+20],0
    mov dword[eax+24],0
    mov dword[eax+28],0
    mov dword[eax+32],0
    mov dword[eax+36],0
    mov dword[eax+40],0
    mov dword[eax+44],0
    ret
.nozero
    cmp bl,1
    jne near .nokeyb
    cmp bh,1
    ja near .exit
    cmp bh,1
    je near .input2
    mov dword[eax],54
    mov dword[eax+4],28
    mov dword[eax+8],72
    mov dword[eax+12],80
    mov dword[eax+16],75
    mov dword[eax+20],77
    mov dword[eax+24],82
    mov dword[eax+28],71
    mov dword[eax+32],73
    mov dword[eax+36],83
    mov dword[eax+40],79
    mov dword[eax+44],81
    ret
.input2
    mov dword[eax],56
    mov dword[eax+4],29
    mov dword[eax+8],37
    mov dword[eax+12],50
    mov dword[eax+16],49
    mov dword[eax+20],51
    mov dword[eax+24],31
    mov dword[eax+28],32
    mov dword[eax+32],33
    mov dword[eax+36],44
    mov dword[eax+40],45
    mov dword[eax+44],46
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
    mov dword[eax+40],83h
    mov dword[eax+36],82h
    mov dword[eax+8],0CCh
    mov dword[eax+12],0CDh
    mov dword[eax+16],0CEh
    mov dword[eax+20],0CFh
    ret
.2ndjoyst
    mov dword[eax+40],85h
    mov dword[eax+36],84h
    mov dword[eax+8],0E8h
    mov dword[eax+12],0E9h
    mov dword[eax+16],0EAh
    mov dword[eax+20],0EBh
    ret
.no2buttons
    cmp bl,3
    je .4buttons
    cmp bl,4
    jne near .no4buttons
.4buttons
    mov dword[eax+40],83h
    mov dword[eax+36],82h
    mov dword[eax+28],85h
    mov dword[eax+24],84h
    mov dword[eax+8],0CCh
    mov dword[eax+12],0CDh
    mov dword[eax+16],0CEh
    mov dword[eax+20],0CFh
    ret
.no4buttons
    cmp bl,18
    je .6buttons
    cmp bl,5
    jne near .no6buttons
.6buttons
    mov dword[eax+40],83h
    mov dword[eax+36],82h
    mov dword[eax+28],85h
    mov dword[eax+24],84h
    mov dword[eax+32],86h
    mov dword[eax+44],87h
    mov dword[eax+8],0CCh
    mov dword[eax+12],0CDh
    mov dword[eax+16],0CEh
    mov dword[eax+20],0CFh
    cmp bl,5
    je .skip8b
    mov dword[eax+44],86h
    mov dword[eax+32],87h
    mov dword[eax+4],080h
    mov dword[eax],081h
.skip8b
    ret
.no6buttons
    cmp bl,6
    jne near .nosw1
    mov dword[eax+0],0C9h
    mov dword[eax+4],0C8h
    mov dword[eax+8],0D4h
    mov dword[eax+12],0D5h
    mov dword[eax+16],0D6h
    mov dword[eax+20],0D7h
    mov dword[eax+24],08Ch
    mov dword[eax+28],089h
    mov dword[eax+32],08Eh
    mov dword[eax+36],08Bh
    mov dword[eax+40],088h
    mov dword[eax+44],08Fh
    ret
.nosw1
    cmp bl,7
    jne near .nosw2
    mov dword[eax+0],0C9h+8
    mov dword[eax+4],0C8h+8
    mov dword[eax+8],0D4h+8
    mov dword[eax+12],0D5h+8
    mov dword[eax+16],0D6h+8
    mov dword[eax+20],0D7h+8
    mov dword[eax+24],08Ch+8
    mov dword[eax+28],089h+8
    mov dword[eax+32],08Eh+8
    mov dword[eax+36],08Bh+8
    mov dword[eax+40],088h+8
    mov dword[eax+44],08Fh+8
    ret
.nosw2
    cmp bl,8
    jne near .nosw3
    mov dword[eax+0],0C9h+8*2
    mov dword[eax+4],0C8h+8*2
    mov dword[eax+8],0D4h+8*2
    mov dword[eax+12],0D5h+8*2
    mov dword[eax+16],0D6h+8*2
    mov dword[eax+20],0D7h+8*2
    mov dword[eax+24],08Ch+8*2
    mov dword[eax+28],089h+8*2
    mov dword[eax+32],08Eh+8*2
    mov dword[eax+36],08Bh+8*2
    mov dword[eax+40],088h+8*2
    mov dword[eax+44],08Fh+8*2
    ret
.nosw3
    cmp bl,9
    jne near .nosw4
    mov dword[eax+0],0C9h+8*3
    mov dword[eax+4],0C8h+8*3
    mov dword[eax+8],0D4h+8*3
    mov dword[eax+12],0D5h+8*3
    mov dword[eax+16],0D6h+8*3
    mov dword[eax+20],0D7h+8*3
    mov dword[eax+24],08Ch+8*3
    mov dword[eax+28],089h+8*3
    mov dword[eax+32],08Eh+8*3
    mov dword[eax+36],08Bh+8*3
    mov dword[eax+40],088h+8*3
    mov dword[eax+44],08Fh+8*3
    ret
.nosw4
    cmp bl,10
    jne near .nogrip0
    mov dword[eax+0],0CAh
    mov dword[eax+4],0CBh
    mov dword[eax+8],0F0h
    mov dword[eax+12],0F1h
    mov dword[eax+16],0F2h
    mov dword[eax+20],0F3h
    mov dword[eax+24],0A9h
    mov dword[eax+28],0ABh
    mov dword[eax+32],0ACh
    mov dword[eax+36],0A8h
    mov dword[eax+40],0AAh
    mov dword[eax+44],0AEh
    ret
.nogrip0
    cmp bl,11
    jne near .nogrip1
    mov dword[eax+0],0CAh+8
    mov dword[eax+4],0CBh+8
    mov dword[eax+8],0F0h+4
    mov dword[eax+12],0F1h+4
    mov dword[eax+16],0F2h+4
    mov dword[eax+20],0F3h+4
    mov dword[eax+24],0A9h+8
    mov dword[eax+28],0ABh+8
    mov dword[eax+32],0ACh+8
    mov dword[eax+36],0A8h+8
    mov dword[eax+40],0AAh+8
    mov dword[eax+44],0AEh+8
    ret
.nogrip1
    cmp bl,14
    jne near .nopp1
    mov dword[eax+40],180h
    mov dword[eax+36],181h
    mov dword[eax+0],182h
    mov dword[eax+4],183h
    mov dword[eax+8],184h
    mov dword[eax+12],185h
    mov dword[eax+16],186h
    mov dword[eax+20],187h
    mov dword[eax+28],188h
    mov dword[eax+24],189h
    mov dword[eax+32],18Ah
    mov dword[eax+44],18Bh
    ret
.nopp1
    cmp bl,15
    jne near .nopp2
    mov dword[eax+40],190h
    mov dword[eax+36],191h
    mov dword[eax+0],192h
    mov dword[eax+4],193h
    mov dword[eax+8],194h
    mov dword[eax+12],195h
    mov dword[eax+16],196h
    mov dword[eax+20],197h
    mov dword[eax+28],198h
    mov dword[eax+24],199h
    mov dword[eax+32],19Ah
    mov dword[eax+44],19Bh
    ret
.nopp2
    cmp bl,16
    jne near .nopp3
    mov dword[eax+40],1A0h
    mov dword[eax+36],1A1h
    mov dword[eax+0],1A2h
    mov dword[eax+4],1A3h
    mov dword[eax+8],1A4h
    mov dword[eax+12],1A5h
    mov dword[eax+16],1A6h
    mov dword[eax+20],1A7h
    mov dword[eax+28],1A8h
    mov dword[eax+24],1A9h
    mov dword[eax+32],1AAh
    mov dword[eax+44],1ABh
    ret
.nopp3
    cmp bl,17
    jne near .nopp4
    mov dword[eax+40],1B0h
    mov dword[eax+36],1B1h
    mov dword[eax+0],1B2h
    mov dword[eax+4],1B3h
    mov dword[eax+8],1B4h
    mov dword[eax+12],1B5h
    mov dword[eax+16],1B6h
    mov dword[eax+20],1B7h
    mov dword[eax+28],1B8h
    mov dword[eax+24],1B9h
    mov dword[eax+32],1BAh
    mov dword[eax+44],1BBh
    ret
.nopp4
.exit
    ret

SECTION .data

; Total Number of Input Devices
NEWSYM NumInputDevices, dd 16

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
db 'PARALLEL LPT2 P1',0
db 'PARALLEL LPT2 P2',0

; GUI Description codes for each corresponding key pressed value
NEWSYM ScanCodeListing
        db '---','ESC',' 1 ',' 2 ',' 3 ',' 4 ',' 5 ',' 6 '
        db ' 7 ',' 8 ',' 9 ',' 0 ',' - ',' = ','BKS','TAB'
        db ' Q ',' W ',' E ',' R ',' T ',' Y ',' U ',' I '
        db ' O ',' P ',' [ ',' ] ','RET','CTL',' A ',' S '
        db ' D ',' F ',' G ',' H ',' J ',' K ',' L ',' : '
        db ' " ',' ~ ','LSH',' \ ',' Z ',' X ',' C ',' V '
        db ' B ',' N ',' M ',' < ',' > ',' / ','RSH',' * '
        db 'ALT','SPC','CAP','F1 ','F2 ','F3 ','F4 ','F5 '
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','HOM'
        db 'UP ','PUP',' - ','LFT',' 5 ','RGT',' + ','END'
        db 'DWN','PDN','INS','DEL','   ','   ','   ','F11'
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
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '

;SECTION .text

;SECTION .data
;NEWSYM ZSNESBase, dd 0
;TempVarSeek dd 0
gotoroot db '\',0

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


NEWSYM SoundProcess     ; This function is called ~60 times/s at full speed
.nosound
    ret
.notdos
    cmp byte[soundon],0
    je .nosound
    cmp byte[DSPDisable],1
    je .nosound
    mov eax,200         ; Size of 32-bit buffer in dwords
    add eax,eax
    mov dword[BufferSizeW],eax
    add eax,eax
    mov dword[BufferSizeB],eax
    pushad
    call ProcessSoundBuffer
    popad
    ; DSPBuffer should contain the processed buffer in the specified size
    ; You will have to convert/clip it to 16-bit for actual sound process
    ret



NEWSYM Check60hz
    ; Call the timer update function here
    ret

; ****************************
; TCP/IP Stuff
; ****************************

SECTION .bss

NEWSYM TCPIPStatus, resb 1
NEWSYM PacketSendSize, resd 1
NEWSYM PacketRecvSize, resd 1
NEWSYM PacketRecvPtr,  resd 1
NEWSYM PacketSendArray, resb 256
NEWSYM PacketRecvArray, resb 256
NEWSYM IPAddrStr, resb 20
NEWSYM RemoteDisconnect, resb 1


SECTION .text

NEWSYM TCPIPStartServer
    mov dword[PacketSendSize],0
    mov dword[PacketRecvSize],0
;    call InitTCP
    or eax,eax
    jnz .failed
    mov byte[TCPIPStatus],1
;    StartServer(unsigned short port)
    mov ax,[TCPIPPortNum]
    push ax
;    call StartServer
    add esp,2
    or eax,eax
    jnz .noserver
    mov byte[TCPIPStatus],2
.noserver
    ret
.failed
    mov eax,1
    ret

NEWSYM TCPIPInitConnectToServer
    mov dword[PacketSendSize],0
    mov dword[PacketRecvSize],0
;    call InitTCP
    or eax,eax
    jnz .failed
    mov byte[TCPIPStatus],1
    ; Convert TCPIPAddress to IPAddrStr
    mov ebx,TCPIPAddress
    mov edx,IPAddrStr
.notend
    mov al,[ebx]
    cmp al,' '
    je .dontinclude
    mov [edx],al
    inc edx
.dontinclude
    inc ebx
    cmp al,0
    jne .notend
; int ConnectServer(char *servername, unsigned short port)
    mov ax,[TCPIPPortNum]
    push ax
    mov eax,IPAddrStr
    push eax
;    call ConnectServer
    add esp,6
    or eax,eax
    jnz .noclient
    mov byte[TCPIPStatus],3
.noclient
    xor eax,eax
    ret
.failed
    mov eax,1
    ret

NEWSYM TCPIPStoreByte
    ; Store al into the array
    cmp dword[PacketSendSize],256
    je .packeterror
    push ebx
    mov ebx,[PacketSendSize]
    mov [PacketSendArray+ebx],al
    pop ebx
    inc dword[PacketSendSize]
    ret
.packeterror
    jmp OSExit

NEWSYM TCPIPGetByte
    ; dh = 0 : No bytes in buffer
    ; dl = resulting character
    cmp dword[PacketRecvSize],0
    jne .bytereceived
    mov dword[PacketRecvPtr],0
    call TCPIPRecvPacket
    cmp dword[PacketRecvSize],0
    jne .bytereceived
    xor dh,dh
    ret
.bytereceived
    push eax
    mov eax,[PacketRecvPtr]
    mov dl,[PacketRecvArray+eax]
    mov dh,1
    inc dword[PacketRecvPtr]
    mov eax,[PacketRecvPtr]
    cmp [PacketRecvSize],eax
    jne .notequal
    mov dword[PacketRecvSize],0
.notequal
    pop eax
    ret

NEWSYM TCPIPSendPacket
    cmp dword[PacketSendSize],0
    je .nopacket
    pushad
    ; Send PacketSendArray with size of PacketSendSize
    ; SendData(int dsize,char *dptr)
    mov eax,PacketSendArray
    push eax
    mov eax,[PacketSendSize]
    push eax
;    call SendData
    or eax,eax
    jnz .failed
    add esp,8
    popad
.nopacket
    ret
.failed
    add esp,8
    popad
    call TCPIPDisconnect
    jmp OSExit

NEWSYM TCPIPRecvPacket
    pushad
    ; Store packet to PacketRecvArray, size at PacketRecvSize
    ; int GetData(int dsize,char *dptr)
    mov eax,PacketRecvArray
    push eax
    mov eax,256
    push eax
;    call GetData
    cmp eax,-1
    je .failed
    mov [PacketRecvSize],eax
    add esp,8
    popad
    ret
.failed
    add esp,8
    popad
    call TCPIPDisconnect
    jmp OSExit

NEWSYM TCPIPDisconnect
;    call DeInitTCP
    cmp byte[TCPIPStatus],3
    jne .notserver
;    call StopServer
.notserver
    cmp byte[TCPIPStatus],3
    jne .notclient
;    call Disconnect
.notclient
    ret

NEWSYM TCPIPPreparePacket
    mov dword[PacketSendSize],0
    ret

NEWSYM TCPIPConnectToServer
    ret

NEWSYM TCPIPWaitForConnection
    ret

NEWSYM TCPIPSendPacketUDP
    ret

NEWSYM TCPIPConnectToServerW
    ret

NEWSYM GetHostName
    ret

NEWSYM hostname
NEWSYM UDPConfig

SECTION .bss

NEWSYM tcperr, resd 1


SECTION .text

NEWSYM ClearUDPStuff
NEWSYM Wait1SecWin
NEWSYM EnableSUDPPacket
NEWSYM DisableSUDPPacket
NEWSYM WinErrorA
NEWSYM WinErrorB
NEWSYM GotoHomepage
    ret

NEWSYM GetTimeInSeconds
    push es
    mov ax,[selc0040]
    mov es,ax
    mov eax,[es:108]
    and eax,0FFFFFFh
    xor edx,edx
    mov ebx,86400
    mul ebx
    mov ebx,1573039
    div ebx
    pop es
    ret


