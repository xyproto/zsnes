;Copyright (C) 1997-2004 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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

EXTSYM previdmode,DosExit,ZFileSystemInit
EXTSYM getcmdline,GUIRestoreVars,getcfg,obtaindir,ConvertJoyMap,tparms
EXTSYM preparedir,SBHDMA
EXTSYM ccmdline
EXTSYM FilenameStart
EXTSYM spcon
EXTSYM cfgsoundon
EXTSYM cfgcvidmode
EXTSYM pl1contrl,pl2contrl
EXTSYM InitDir,InitDrive
EXTSYM DOScreatenewcfg,ExecGUISaveVars
EXTSYM allocptr
extsym putchar
EXTSYM getch
EXTSYM exit
EXTSYM ZOpenFile,ZOpenMode,CurrentHandle,ZFileSeek,ZOpenFileName,ZFileSeekMode
EXTSYM ZFileSeekPos,ZFileSeekHandle
EXTSYM ZFileWriteHandle,ZFileWriteSize,ZFileWriteBlock,ZFileWrite
EXTSYM ZFileReadHandle,ZFileReadSize,ZFileReadBlock,ZFileRead
EXTSYM ZFileDelFName,ZFileDelete
EXTSYM ZCloseFileHandle,ZCloseFile
EXTSYM ZFileTellHandle,ZFileTell
EXTSYM ZFFTimeFName,ZFTime,ZFDate,ZFileGetFTime
EXTSYM GetTime
EXTSYM GetDate
extsym kbhit
extsym GUIkeydelay2
;extsym _kbhit
;EXTSYM _getch
extsym _chdrive
EXTSYM ZFileMKDir,ZFileCHDir,ZFileRMDir,CHPath,MKPath,RMPath
EXTSYM ZFileGetDir,DriveNumber,DirName
EXTSYM _getdrive
EXTSYM DTALoc,DTALocPos,ZFileFindATTRIB,ZFileFindFirst,ZFileFindNext,ZFileFindPATH
EXTSYM NoSoundReinit,soundon,DSPDisable,NoSoundReinit
EXTSYM init60hz,init18_2hz
EXTSYM Start60HZ
EXTSYM pressed
EXTSYM RaisePitch,AdjustFrequency
EXTSYM vidbufferofsb,vidbuffer
EXTSYM clearwin
EXTSYM Stop60HZ
EXTSYM dosmakepal
EXTSYM doschangepal
EXTSYM dosinitvideo,dosinitvideo2
EXTSYM initwinvideo
EXTSYM vesa2_rpos
EXTSYM vesa2_gpos
EXTSYM vesa2_bpos
EXTSYM vesa2_rposng
EXTSYM vesa2_gposng
EXTSYM vesa2_bposng
EXTSYM vesa2_usbit
EXTSYM vesa2_clbit
EXTSYM vesa2_clbitng
EXTSYM vesa2_clbitng2
EXTSYM vesa2_clbitng3
EXTSYM genfulladdtabng
EXTSYM vesa2red10
EXTSYM res640
EXTSYM res480
EXTSYM AddSub256,InitVesa2,cbitmode,cvidmode
EXTSYM scanlines,vesa2_bits
EXTSYM vesa2_x,vesa2_y
EXTSYM vesa2selec,videotroub
EXTSYM genfulladdtab
EXTSYM GUICPC
EXTSYM drawscreenwin
EXTSYM ConvertToAFormat
EXTSYM UnusedBit,HalfTrans,UnusedBitXor,UnusedBit,UnusedBitXor
EXTSYM ngrposng,nggposng,ngbposng,HalfTransB,HalfTransC
EXTSYM DosUpdateDevices
EXTSYM WinUpdateDevices
EXTSYM DOSJoyRead
EXTSYM UpdateVFrame
EXTSYM GetMouseX
EXTSYM GetMouseY
EXTSYM GetMouseMoveX
EXTSYM GetMouseMoveY
EXTSYM GetMouseButton
EXTSYM SetMouseMinX,SetMouseMaxX
EXTSYM SetMouseMinY,SetMouseMaxY
EXTSYM SetMouseX,SetMouseY
EXTSYM T36HZEnabled
EXTSYM MouseButton
EXTSYM GUIinit36_4hz,GUIoldhand9s,GUIoldhand9o,GUIoldhand8s,GUIoldhand8o
EXTSYM GUIinit18_2hz
EXTSYM Start36HZ
EXTSYM Stop36HZ
EXTSYM BufferSizeW,BufferSizeB,ProcessSoundBuffer
EXTSYM CheckTimers
EXTSYM vesa2_rfull,vesa2_rtrcl,vesa2_rtrcla
EXTSYM vesa2_gfull,vesa2_gtrcl,vesa2_gtrcla
EXTSYM vesa2_bfull,vesa2_btrcl,vesa2_btrcla
EXTSYM Init_2xSaIMMXW
EXTSYM TCPIPPortNum
EXTSYM InitTCP
EXTSYM StartServerCycle
EXTSYM ServerCheckNewClient
EXTSYM acceptzuser
EXTSYM TCPIPAddress
EXTSYM ConnectServer
EXTSYM WaitForServer
EXTSYM SendData
EXTSYM SendDataUDP
EXTSYM GetData
EXTSYM DeInitTCP
EXTSYM StopServer
EXTSYM Disconnect
EXTSYM UDPDisableMode,UDPEnableMode,UDPClearVars,UDPWait1Sec
EXTSYM WinErrorA2,WinErrorB2,WinErrorC2
EXTSYM ZsnesPage
EXTSYM V8Mode,GrayscaleMode
EXTSYM PrevWinMode,PrevFSMode
EXTSYM _imp__GetLocalTime@4

EXTSYM FrameSemaphore

%ifdef __MINGW__
NEWSYM WinIntRFAsmStart
%endif


; NOTE: For timing, Game60hzcall should be called at 50hz or 60hz (depending
;   on romispal) after a call to InitPreGame and before DeInitPostGame are
;   made.  GUI36hzcall should be called at 36hz after a call GUIInit and
;   before GUIDeInit.


SECTION .data
;NEWSYM OSPort, db 3      ; 0 = DOS (C), 1 = DOS (ASM), 2 = Linux, 3 = Win95
SECTION .text

NEWSYM StartUp
    call ZFileSystemInit
    ret

; SystemInit - Initialize all Joystick stuff, load in all configuration data,
;   parse commandline data, obtain current directory (One time initialization)

NEWSYM SystemInit
    ; Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    push es
    mov byte[cfgcvidmode],3
    mov byte[cvidmode],3
    call getcmdline

    mov byte[esi],'z'
    mov byte[esi+1],'s'
    mov byte[esi+2],'n'
    mov byte[esi+3],'e'
    mov byte[esi+4],'s'
    mov byte[esi+5],'w'
    mov byte[esi+6],'.'
    mov byte[esi+7],'c'
    mov byte[esi+8],'f'
    mov byte[esi+9],'g'
    mov byte[esi+10],0
    mov byte[esi+256],'z'
    mov byte[esi+1+256],'g'
    mov byte[esi+2+256],'u'
    mov byte[esi+3+256],'i'
    mov byte[esi+4+256],'c'
    mov byte[esi+5+256],'f'
    mov byte[esi+6+256],'g'
    mov byte[esi+7+256],'w'
    mov byte[esi+8+256],'.'
    mov byte[esi+9+256],'d'
    mov byte[esi+10+256],'a'
    mov byte[esi+11+256],'t'
    mov byte[esi+12+256],0

    mov byte[spcon],1
    mov byte[soundon],1
    mov byte[cfgsoundon],1

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
;    call getblaster                     ; get set blaster environment
;    cmp byte[Force8b],1
;    jne .noforce8b
    mov byte[SBHDMA],1
;.noforce8b
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


NEWSYM PrintChar
    ret
    ; print character at dl, push all modified registers
    pushad 
    push eax
    push edx
    call putchar
    pop edx
;    mov ah,02h
;    int 21h
    pop eax
    popad 
    ret

NEWSYM PrintStr          ; Print ASCIIZ string

    pushad

.next
    mov al,[edx]
    or al,al
    jz .finish
    push edx
    mov dl,al
    push edx
    call putchar
    pop edx
;    mov ah,02h
;    int 21h
    pop edx
    inc edx
    jmp .next
.finish
    popad
    ret


SECTION .bss
NEWSYM wfkey, resb 1
SECTION .text

NEWSYM WaitForKey       ; Wait for a key to be pressed
    pushad
    call getch
    mov [wfkey],al
    popad
    mov al,[wfkey]
    ;mov ah,7
    ;int 21h
    ; return key in al
    ret


NEWSYM OsExit
NEWSYM OSExit
    call exit
    jmp DosExit

SECTION .bss
NEWSYM TempHandle, resd 1
SECTION .text

NEWSYM Open_File
    pushad
    mov dword[ZOpenMode],0
    mov dword[ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov [TempHandle],eax
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov dword[ZFileSeekHandle],eax
    call ZFileSeek
    popad
    mov ax,[TempHandle]
    clc
    ret
.error
    popad
    stc
    ret
;    mov ax,3D00h
;    int 21h
    ; return bx = file handle, carry = error
;    ret

NEWSYM Open_File_Write
    pushad
    mov dword[ZOpenMode],2
    mov dword[ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov [TempHandle],eax
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov dword[ZFileSeekHandle],eax
    call ZFileSeek
    popad
    mov ax,[TempHandle]
    clc
    ret
.error
    popad
    stc
    ret
;    mov ax,3D01h
;    int 21h
    ; return bx = file handle, carry = error
;    ret

NEWSYM Create_File
    pushad
    mov dword[ZOpenMode],1
    mov dword[ZOpenFileName],edx    
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov [TempHandle],eax
    popad
    mov ax,[TempHandle]
    clc
    ret
.error
    popad
    stc
    ret
;    mov ah,3Ch
;    mov cx,0
;    int 21h
    ; return bx = file handle
;    ret

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
;    mov ah,40h
;    int 21h
;    ret

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
;    mov ah,3Fh
;    int 21h
;    ret

NEWSYM Delete_File
    mov [ZFileDelFName],edx
    pushad
    call ZFileDelete
    popad
    ret
;    mov ah,41h
;    int 21h
;    ret

NEWSYM Close_File
    mov dword[ZCloseFileHandle],0
    mov [ZCloseFileHandle],bx
    pushad
    call ZCloseFile
    popad
    clc
    ret
;    mov ah,3Eh
;    int 21h
;    ret

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
;    mov ax,4200h
;    int 21h
;    ret

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
    ret
    ; seek to cx:dx from end position, and return file location in dx:ax
;    mov ax,4202h
;    int 21h
;    ret

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
    mov dx,0
    mov cx,0
;    mov ah,2Ah
;    int 21h
    ret

NEWSYM Get_File_Date
    mov [ZFFTimeFName],edx
    pushad
    call ZFileGetFTime
    popad
    mov edx,[ZFDate]
    mov ecx,[ZFTime]
    ret
    ; return packed date in dx:cx
;    mov ah,57h
;    mov al,00h
;    int 21h
;    ret


RefreshKeybBuffer:
    call JoyRead
    mov ebx,[HoldKey]
    cmp byte[pressed+ebx],0
    jne .holding
    mov dword[HoldKey],0
.holding
    xor eax,eax
    xor ebx,ebx
.loop
    cmp byte[PKeyBuf+eax],0
    jne .not1
    cmp byte[pressed+eax],0
    je .not1
    mov byte[PKeyBuf+eax],1
    mov ebx,eax
.not1
    cmp byte[pressed+eax],0
    jne .not0
    mov byte[PKeyBuf+eax],0
.not0
    inc eax
    cmp eax,100h
    jne .loop
    or ebx,ebx
    jz .notpressed
    mov [HoldKey],ebx
    mov byte[GUIkeydelay2],14
    call .processkey
.notpressed
    ; Execute the following at 36hz
    cmp dword[HoldKey],0
    je .noholder
    cmp byte[GUIkeydelay2],0
    jne .noholder
    mov byte[GUIkeydelay2],3
    call .processkey
.noholder
    ret
.processkey
    mov ebx,[HoldKey]
    cmp ebx,0A8h
    jb .skipdecval
    add ebx,-80h
.skipdecval
    cmp ebx,58h
    jae .none
    xor eax,eax
    mov al,[Keybtail]
    inc al
    and al,0Fh
    cmp al,[Keybhead]
    je .none
    mov al,[Keybtail]
    mov cl,[KeyConvTable+ebx]
    cmp byte[pressed+2Ah],0
    jne .shift
    cmp byte[pressed+36h],0
    je .noshift
.shift
    mov cl,[KeyConvTableS+ebx]
.noshift
    mov [HoldKeyBuf+eax],cl
    inc al
    and al,0Fh
    mov [Keybtail],al
.none
    ret

SECTION .bss
Keybhead resb 1
Keybtail resb 1
HoldKey resd 1
HoldKeyBuf resb 16
PKeyBuf resb 100h

NEWSYM CurKeyPos, resd 1
NEWSYM CurKeyReadPos, resd 1
NEWSYM KeyBuffer, resd 16
SECTION .text

NEWSYM Check_Key
    mov al,[CurKeyPos]
    cmp al,[CurKeyReadPos]
    jne .yeskey
    xor al,al
    ret
.yeskey
    mov al,0FFh
    ret
    ; returns 0 if there are no keys in the keyboard buffer, 0xFF otherwise
    pushad
;    call kbhit
    call RefreshKeybBuffer
    mov byte[wfkey],0
    mov al,[Keybhead]
    cmp al,[Keybtail]
    je .nokeys
    mov byte[wfkey],0FFh
.nokeys
    popad
    mov al,[wfkey]
;    mov ah,0Bh
;    int 21h
    ret

NEWSYM Get_Key
    ; wait if there are no keys in buffer, then return key in al
    ; for extended keys, return a 0, then the extended key afterwards
    xor eax,eax
.nokey
    call JoyRead
    mov al,[CurKeyReadPos]
    cmp al,[CurKeyPos]
    je .nokey
    test word[KeyBuffer+eax*4],100h
    jnz .upper
    mov al,[KeyBuffer+eax*4]
    inc dword[CurKeyReadPos]
    and dword[CurKeyReadPos],0Fh
    ret
.upper
    add word[KeyBuffer+eax*4],-100h
    xor al,al
    ret

    pushad
.nonewkey
    call RefreshKeybBuffer
    xor eax,eax
    mov al,[Keybhead]
    cmp al,[Keybtail]
    je .nonewkey
    mov bl,[HoldKeyBuf+eax]
    test bl,80h
    jz .notupperkey
    xor bl,bl
    add byte[HoldKeyBuf+eax],-80h
    jmp .yesupperkey
.notupperkey
    inc al
    and al,0Fh
    mov [Keybhead],al
.yesupperkey
;    call getch
    mov [wfkey],bl
    popad
    mov al,[wfkey]
    ;mov ah,7
    ;int 21h
    ; return key in al
    ret

SECTION .data
KeyConvTable
   db 255,27 ,'1','2','3','4','5','6'  ; 00h
   db '7','8','9','0','-','=',8  ,9 
   db 'Q','W','E','R','T','Y','U','I'  ; 10h
   db 'O','P','[',']',13 ,255,'A','S'
   db 'D','F','G','H','J','K','L',';'  ; 20h
   db 39 ,'`',255,'\','Z','X','C','V'
   db 'B','N','M',',','.','/',255,'*'  ; 30h
   db 255,32 ,255,255,255,255,255,255
   db 255,255,255,255,255,255,255,255  ; 40h
   db 200,201,202,203,204,205,206,207
   db 208,209,210,211,255,255,255,255  ; 50h
KeyConvTableS
   db 255,27 ,'!','@','#','$','%','^'  ; 00h
   db '&','*','(',')','_','+',8  ,9 
   db 'Q','W','E','R','T','Y','U','I'  ; 10h
   db 'O','P','{','}',13 ,255,'A','S'
   db 'D','F','G','H','J','K','L',':'  ; 20h
   db '"','~',255,'|','Z','X','C','V'
   db 'B','N','M','<','>','?',255,'*'  ; 30h
   db 255,32 ,255,255,255,255,255,255
   db 255,255,255,255,255,255,255,255  ; 40h
   db 200,201,202,203,204,205,206,207
   db 208,209,210,211,255,255,255,255  ; 50h
SECTION .text

;    mov dl,[SRAMDrive]
;    mov ebx,SRAMDir
;    call Change_Dir

NEWSYM Change_Drive
    ; change to drive in dl (0 = A, 1 = B, etc.)
    and edx,0FFh
    add edx,1
    push edx
    call _chdrive
    pop edx
;    mov ah,0Eh
;    int 21h
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
;    mov ah,3Bh
;    int 21h
;    ret

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
;    mov ah,39h
;    int 21h
;    ret

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
;    mov ah,3Ah
;    int 21h
;    ret

;    mov dl,[LoadDrive]
;    mov ebx,LoadDir
;    call Change_Dir
NEWSYM Change_Dir
    pushad

    and edx,0FFh
    add edx,1
    push edx
    call _chdrive
    pop edx

;    mov ah,0Eh
;    int 21h
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
;    push ebx
;    mov ah,0Eh
;    int 21h
;    mov ah,3Bh
;    mov edx,gotoroot
;    int 21h
;    pop ebx
;    mov edx,ebx
;    cmp byte[edx],0
;    je .nodir
;    mov ah,3Bh
;    int 21h
;.nodir
;    ret

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
    call _getdrive
;    mov ah,19h
;    int 21h
    add al,-1
    pop edx
    mov [edx],al
    ret

;    push edx
;    mov ah,47h
;    mov dl,0
;    mov esi,ebx
;    int 21h
;    mov ah,19h
;    int 21h
;    pop edx
;    mov [edx],al
;    ret

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
;    mov ah,4Eh
;    mov al,0
;    int 21h
;    ret

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
;    mov ah,04Fh
;    int 21h
;    ret

NEWSYM Set_DTA_Address
    ; Only needed for dos stuff
;    mov edx,DTALoc
;    mov ah,1Ah
;    int 21h
    ret

NEWSYM Get_Memfree
    mov eax,02000000h
;    mov ax,0500h
;    mov edi,edx
;    int 31h
    ret

NEWSYM Output_Text       ; Output character (ah=02h) or string (ah=09h)
    pushad 

    ; This function usually displays an error message on-screen
    cmp ah,02h
    je .char
    cmp ah,09h
    je .string
    ret
.char
    push edx
    call putchar
    pop edx
;    int 21h     ; print dl
    popad
    ret
.string
    pushad
    call PrintStr       ; print edx
    popad
    popad
    ret


NEWSYM InitPreGame   ; Executes before starting/continuing a game
    mov byte[pressed+1],2
    pushad
    call Start60HZ
    popad

    pushad
    call initwinvideo
    popad

    mov al,[GrayscaleMode]
    cmp al,[V8Mode]
    je .nochangemode
    xor byte[V8Mode],1
    xor al,al
.nochangemode

    mov byte[RaisePitch],1
    pushad
    call AdjustFrequency
    popad

    pushad
    xor eax,eax
    mov edi,[vidbufferofsb]
    mov ecx,228*120
    rep stosd
    popad

    pushad
    call clearwin
    popad
    ret

    ; set up interrupt handler
    ; get old handler pmode mode address
    ; Process stuff such as sound init, interrupt initialization
    ret

NEWSYM SetupPreGame   ; Executes after pre-game init, can execute multiple
                      ; times after a single InitPreGame
    mov byte[pressed+1],2
    ret


NEWSYM DeInitPostGame           ; Called after game is ended
    pushad
    call Stop60HZ
    popad
    ret

; ****************************
; Video Stuff
; ****************************

; ** Palette Functions **
NEWSYM makepal  ; 8-bit palette set
    ret
;    jmp dosmakepal
NEWSYM changepal  ; 8-bit palette set (changes only)
    ret
;    jmp doschangepal
NEWSYM displayfpspal
    ret

;    mov al,128
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,63
;    out dx,al
;    out dx,al
;    out dx,al
;    mov al,128+64
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,0
;    out dx,al
;    out dx,al
;    out dx,al
;    ret

NEWSYM superscopepal
    ret

;    mov al,128+16
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,63
;    out dx,al
;    xor al,al
;    out dx,al
;    out dx,al
;    ret

NEWSYM saveselectpal
    ret

    ; set palette of colors 128,144, and 160 to white, blue, and red
;    mov al,128
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,63
;    out dx,al
;    out dx,al
;    out dx,al
;    mov al,144
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    xor al,al
;    out dx,al
;    out dx,al
;    mov al,50
;    out dx,al
;    mov al,160
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,45
;    out dx,al
;    xor al,al
;    out dx,al
;    out dx,al
;    mov al,176
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,47
;    out dx,al
;    xor al,al
;    out dx,al
;    out dx,al
;    mov al,208
;    mov dx,03C8h
;    out dx,al
;    inc dx
;    mov al,50
;    out dx,al
;    mov al,25
;    out dx,al
;    xor al,al
;    out dx,al
;    ret

; ** init video mode functions **
SECTION .data
NEWSYM firstvideo, dd 1
SECTION .text


NEWSYM initvideo  ; Returns 1 in videotroub if trouble occurs
   mov byte[res640],1
   mov byte[res480],1
   mov byte[cbitmode],1
   mov word[vesa2_x],512
   mov word[vesa2_y],480
   mov byte[vesa2_bits],16
   mov dword [vesa2_bits],16
   mov dword [vesa2_rpos],11
   mov dword [vesa2_gpos],5
   mov dword [vesa2_bpos],0
   mov byte[vesa2red10],0
   mov byte[vesa2_rposng],11
   mov byte[vesa2_gposng],5
   mov byte[vesa2_bposng],0
   mov dword[vesa2_clbitng],1111011111011110b
   mov dword[vesa2_clbitng2],11110111110111101111011111011110b
   mov dword[vesa2_clbitng2+4],11110111110111101111011111011110b
   mov dword[vesa2_clbitng3],0111101111101111b

   pushad
   call initwinvideo
   popad

   xor eax,eax
   mov al,[cvidmode]
   cmp byte[GUIWFVID+eax],0
   je .prevwinmode
   mov byte[PrevFSMode],al
   jmp .doneprevmode
.prevwinmode
   mov byte[PrevWinMode],al
.doneprevmode

   cmp dword[firstvideo],1
   je .skipinitgfx
   pushad
   call InitializeGfxStuff
   popad

.skipinitgfx
   mov dword[firstvideo],0

   pushad
   call InitializeGfxStuff
   popad

   ret


;   pushad
;   call genfulladdtabng
;   popad
;    jmp dosinitvideo
NEWSYM initvideo2 ; ModeQ scanline re-init (Keep blank on non-dos ports)
    ret
;    jmp dosinitvideo2
NEWSYM deinitvideo
;    mov al,[previdmode]
;    mov ah,0
;    int 10h
    ret

; ** copy video mode functions **
SECTION .bss
NEWSYM converta, resd 1
SECTION .text

NEWSYM DrawScreen               ; In-game screen render w/ triple buffer check
    cmp dword [converta],1
    jne near .skipconv
    pushad
        mov dword[UnusedBit],     10000000000000001000000000000000b
        mov dword[HalfTrans],     01111011110111100111101111011110b
        mov dword[UnusedBitXor],  01111111111111110111111111111111b
        mov dword[UnusedBit+4],   10000000000000001000000000000000b
        mov dword[HalfTrans+4],   01111011110111100111101111011110b
        mov dword[UnusedBitXor+4],01111111111111110111111111111111b
        mov dword[HalfTransB],    00000100001000010000010000100001b
        mov dword[HalfTransB+4],  00000100001000010000010000100001b
        mov dword[HalfTransC],    01111011110111100111101111011110b
        mov dword[HalfTransC+4],  01111011110111100111101111011110b
        mov dword[ngrposng],10
        mov dword[nggposng],5
        mov dword[ngbposng],0

    call ConvertToAFormat

    popad

.skipconv
    pushad
    call drawscreenwin
    popad

    ret
;   jmp DosDrawScreen

NEWSYM vidpastecopyscr       ; GUI screen render
   pushad
   xor eax,eax
   mov al,[cvidmode]
   cmp byte[GUI16VID+eax],1
   jne .no16bconv
   mov eax,[vidbuffer]
   mov ecx,224*288
   mov edx,ecx
   add ecx,-288
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
   jmp DrawScreen


; ** Clear Screen function **
NEWSYM ClearScreen
   ret


; ** Video Mode Variables **
SECTION .data

; Total Number of Video Modes
NEWSYM NumVideoModes, dd 33

; GUI Video Mode Names - Make sure that all names are of the same length
; and end with a NULL terminator
NEWSYM GUIVideoModeNames
db '256x224    R WIN ',0  ;0
db '256x224    R FULL',0  ;1
db '512x448    R WIN ',0  ;2
db '512x448   DR WIN ',0  ;3
db '640x480    S WIN ',0  ;4
db '640x480   DS WIN ',0  ;5
db '640x480   DR FULL',0  ;6
db '640x480   DS FULL',0  ;7
db '640x480    S FULL',0  ;8
db '768x672    R WIN ',0  ;9
db '768x672   DR WIN ',0  ;10
db '800x600    S WIN ',0  ;11
db '800x600   DS WIN ',0  ;12
db '800x600    S FULL',0  ;13
db '800x600   DR FULL',0  ;14
db '800x600   DS FULL',0  ;15
db '1024x768   S WIN ',0  ;16
db '1024x768  DS WIN ',0  ;17
db '1024x768   S FULL',0  ;18
db '1024x768  DR FULL',0  ;19
db '1024x768  DS FULL',0  ;20
db '1024x896   R WIN ',0  ;21
db '1024x896  DR WIN ',0  ;22
db '1280x960   S WIN ',0  ;23
db '1280x960  DS WIN ',0  ;24
db '1280x960   S FULL',0  ;25
db '1280x960  DR FULL',0  ;26
db '1280x960  DS FULL',0  ;27
db '1280x1024  S WIN ',0  ;28
db '1280x1024 DS WIN ',0  ;29
db '1280x1024  S FULL',0  ;30
db '1280x1024 DR FULL',0  ;31
db '1280x1024 DS FULL',0  ;32

; Video Mode Feature Availability (1 = Available, 0 = Not Available)
; Left side starts with Video Mode 0
;                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
NEWSYM GUI16VID,  db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1       ; 16-bit mode
NEWSYM GUINGVID,  db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1       ; New Graphics Mode Available
NEWSYM GUISLVID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1       ; Scanlines
NEWSYM GUIINVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; Interpolation
NEWSYM GUII2VID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1       ; Interpolation(w)
NEWSYM GUIEAVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; Eagle
NEWSYM GUIIEVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; (Interp | Eagle)
NEWSYM GUIFSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; Full Screen
NEWSYM GUIWSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; (Full Scr. | Wide Scr.)
NEWSYM GUISSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; Small Screen
NEWSYM GUITBVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; Triple Buffering
NEWSYM GUIHSVID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1       ; Half/Quarter Scanlines
NEWSYM GUI2xVID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1       ; 2xSaI/Super Eagle Engines
NEWSYM GUIM7VID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1       ; ?Mode 7 video thing?
NEWSYM GUIWFVID,  db 0,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1       ; If Windows Full Screen
NEWSYM GUIDSIZE,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1
NEWSYM GUIRATIO,  db 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NEWSYM GUIBIFIL,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NEWSYM GUITBWVID, db 0,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1      ; Triple Buffering (Win)
NEWSYM GUISMODE,  db 0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1,0,1,0,0
NEWSYM GUIDSMODE, db 0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1
NEWSYM GUIHQ2X,   db 0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0       ; hq2x filter
NEWSYM GUIHQ3X,   db 0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0       ; hq3x filter
NEWSYM GUIHQ4X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,0,1,1       ; hq4x filter


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
        call WinUpdateDevices   
;        call DosUpdateDevices
        ret

NEWSYM JoyRead
;        call SoundProcess               ; Put the sound stuff here since it's
                                        ; called 60 times per second
;        call DOSJoyRead
        pushad
        call UpdateVFrame
        popad
        ret

SECTION .data

; Total Number of Input Devices
NEWSYM NumInputDevices, dd 2

; Input Device Names
NEWSYM GUIInputNames
db 'NONE            ',0
db 'KEYB/JOYSTICK   ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
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
        db '---','ESC',' 1 ',' 2 ',' 3 ',' 4 ',' 5 ',' 6 '  ; 00h
        db ' 7 ',' 8 ',' 9 ',' 0 ',' - ',' = ','BKS','TAB'
        db ' Q ',' W ',' E ',' R ',' T ',' Y ',' U ',' I '  ; 10h
        db ' O ',' P ',' [ ',' ] ','RET','LCT',' A ',' S '
        db ' D ',' F ',' G ',' H ',' J ',' K ',' L ',' : '  ; 20h
        db ' " ',' ~ ','LSH',' \ ',' Z ',' X ',' C ',' V '
        db ' B ',' N ',' M ',' , ',' . ',' / ','RSH',' * '  ; 30h
        db 'LAL','SPC','CAP','F1 ','F2 ','F3 ','F4 ','F5 '
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','N 7'  ; 40h
        db 'N 8','N 9','N -','N 4','N 5','N 6','N +','N 1'
        db 'N 2','N 3','N 0','N .','   ','   ','OEM','F11'  ; 50h
        db 'F12','59h','5Ah','5BH','5CH','5DH','5EH','5FH'
        db '60H','61H','62H','63H','64H','65H','66H','67H'  ; 80h
        db '68H','69H','6AH','6BH','6CH','6DH','6EH','6FH'
        db '70H','71H','72H','73H','74H','75H','76H','77H'  ; 90h
        db '78H','79H','7AH','7BH','7CH','7DH','7EH','7FH'
        ; Keyboard continued (Direct Input)
        db '80H','81H','82H','83H','84H','85H','86H','87H'  ; 80h
        db '88H','89H','8AH','8BH','8CH','8DH','8EH','8FH'
        db '90H','91H','92H','93H','94H','95H','96H','97H'  ; 90h
        db '98H','99H','9AH','9BH','9CH','9DH','9EH','9FH'
        db 'A0H','A1H','A2H','A3H','A4H','A5H','A6H','A7H'  ; A0h
        db 'A8H','A9H','AAH','ABH','ACH','ADH','AEH','AFH'
        db 'B0H','B1H','B2H','B3H','B4H','B5H','B6H','B7H'  ; B0h
        db 'B8H','B9H','BAH','BBH','BCH','BDH','BEH','BFH'  
        db 'C0H','C1H','C2H','C3H','C4H','C5H','C6H','C7H'  ; C0h
        db 'C8H','C9H','CAH','CBH','CCH','CDH','CEH','CFH'
        db 'D0H','D1H','D2H','D3H','D4H','D5H','D6H','D7H'  ; D0h
        db 'D8H','D9H','DAH','DBH','DCH','DDH','DEH','DFH'
        db 'E0H','E1H','E2H','E3H','E4H','E5H','E6H','E7H'  ; E0h
        db 'E8H','E9H','EAH','EBH','ECH','EDH','EEH','EFH'
        db 'F0H','F1H','F2H','F3H','F4H','F5H','F6H','F7H'  ; F0h
        db 'F8H','F9H','FAH','FBH','FCH','FDH','FEH','FFH'
        ; Joystick Stuff (Direct Input)
        db 'J00','J01','J02','J03','J04','J05','J06','J07'
        db 'J08','J09','J0A','J0B','J0C','J0D','J0E','J0F'
        db 'J10','J11','J12','J13','J14','J15','J16','J17'
        db 'J18','J19','J1A','J1B','J1C','J1D','J1E','J1F'
        db 'J20','J21','J22','J23','J24','J25','J26','J27'
        db 'J28','J29','J2A','J2B','J2C','J2D','J2E','J2F'
        db 'J30','J31','J32','J33','J34','J35','J36','J37'
        db 'J38','J39','J3A','J3B','J3C','J3D','J3E','J3F'
        db 'J40','J41','J42','J43','J44','J45','J46','J47'
        db 'J48','J49','J4A','J4B','J4C','J4D','J4E','J4F'
        db 'J50','J51','J52','J53','J54','J55','J56','J57'
        db 'J58','J59','J5A','J5B','J5C','J5D','J5E','J5F'
        db 'J60','J61','J62','J63','J64','J65','J66','J67'
        db 'J68','J69','J6A','J6B','J6C','J6D','J6E','J6F'
        db 'J70','J71','J72','J73','J74','J75','J76','J77'
        db 'J78','J79','J7A','J7B','J7C','J7D','J7E','J7F'
        ; Extra Stuff (180h) (Parallel Port)
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '

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
    mov eax,01h
    ret

SECTION .bss
NEWSYM WMouseX, resd 1
NEWSYM WMouseY, resd 1
NEWSYM WMouseMoveX, resd 1
NEWSYM WMouseMoveY, resd 1
NEWSYM WMouseButton, resd 1

SECTION .text


NEWSYM Get_MouseData         ; Returns both pressed and coordinates
    ; bx : bit 0 = left button, bit 1 = right button
    ; cx = Mouse X Position, dx = Mouse Y Position
    pushad
    call GetMouseX
    mov [WMouseX],eax
    call GetMouseY
    mov [WMouseY],eax
    call GetMouseButton
    mov [WMouseButton],eax
    popad
    mov cx,[WMouseX]
    mov dx,[WMouseY]
    mov bx,[WMouseButton]
    ret

NEWSYM Set_MouseXMax    ; Sets the X boundaries (ecx = left, edx = right)
    pushad
    push ecx
    call SetMouseMinX
    pop ecx
    push edx
    call SetMouseMaxX
    pop edx
    popad
    ret

NEWSYM Set_MouseYMax    ; Sets the Y boundaries (ecx = left, edx = right)
    pushad
    push ecx
    call SetMouseMinY
    pop ecx
    push edx
    call SetMouseMaxY
    pop edx
    popad
    ret

NEWSYM Set_MousePosition        ; Sets Mouse Position (x:cx,y:dx)
    pushad
    push ecx
    call SetMouseX
    pop ecx
    push edx
    call SetMouseY
    pop edx
    popad
    ret

NEWSYM Get_MousePositionDisplacement
    ; returns x,y displacement in pixel in cx,dx
    pushad
    call GetMouseMoveX
    mov [WMouseMoveX],eax
    call GetMouseMoveY
    mov [WMouseMoveY],eax
    popad
    mov cx,[WMouseMoveX]
    mov dx,[WMouseMoveY]
    ret

NEWSYM MouseWindow
    pushad
    or byte[MouseButton],2
    mov byte[T36HZEnabled],1
    call GetMouseButton
    and byte[MouseButton],0FDh
    popad
    ret

NEWSYM GUIInit
    pushad
    call Start36HZ
    popad
    ret

NEWSYM GUIDeInit
    pushad
    call Stop36HZ
    popad
    ret

; ****************************
; Sound Stuff
; ****************************

NEWSYM StopSound
    call Start36HZ
    call JoyRead
    ret

NEWSYM StartSound
    call Start60HZ
    call JoyRead
    ret


NEWSYM SoundProcess     ; This function is called ~60 times/s at full speed
    cmp byte[soundon],0
    je .nosound
    cmp byte[DSPDisable],1
    je .nosound
    mov eax,256         ; Size
    mov dword[BufferSizeB],eax
    add eax,eax
    mov dword[BufferSizeW],eax
    pushad
    call ProcessSoundBuffer
    popad
    ; DSPBuffer should contain the processed buffer in the specified size
    ; You will have to convert/clip it to 16-bit for actual sound process
.nosound    
    ret

NEWSYM delay
   ret

NEWSYM Check60hz
    ; Call the timer update function here
    pushad
    call CheckTimers
    call FrameSemaphore
    popad
    ret

SECTION .data
BitPosR db 11 
BitPosG db 5
BitPosB db 0
BitSizeR db 5
BitSizeG db 6
BitSizeB db 5
SECTION .text


InitializeGfxStuff:
        ; Process Red Stuff
        mov al,[BitPosR]
        mov cl,al
        mov bx,1
        shl bx,cl
        cmp byte[BitSizeR],6
        jne .no6bit
        mov [vesa2_usbit],bx
        inc al
.no6bit
        or [vesa2_clbit],bx
        mov [vesa2_rpos],al
        dec al
        mov cl,al
        mov bx,001Fh
        cmp cl,0FFh
        je .shrr
        shl bx,cl
        jmp .shlr
.shrr
        shr bx,1
.shlr
        mov word[vesa2_rfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov word[vesa2_rtrcl],bx
        xor bx,0FFFFh
        mov word[vesa2_rtrcla],bx

        ; Process Green Stuff
        mov al,[BitPosG]
        mov cl,al
        mov bx,1
        shl bx,cl
        cmp byte[BitSizeG],6
        jne .no6bitb
        mov [vesa2_usbit],bx
        inc al
.no6bitb
        or [vesa2_clbit],bx
        mov [vesa2_gpos],al
        dec al
        mov cl,al
        mov bx,001Fh
        cmp cl,0FFh
        je .shrg
        shl bx,cl
        jmp .shlg
.shrg
        shr bx,1
.shlg
        mov word[vesa2_gfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov word[vesa2_gtrcl],bx
        xor bx,0FFFFh
        mov word[vesa2_gtrcla],bx

        ; Process Blue Stuff
        mov al,[BitPosB]
        mov cl,al
        mov bx,1
        shl bx,cl
        cmp byte[BitSizeB],6
        jne .no6bitc
        mov [vesa2_usbit],bx
        inc al
.no6bitc
        or [vesa2_clbit],bx
        mov [vesa2_bpos],al
        dec al
        mov cl,al
        mov bx,001Fh
        cmp cl,0FFh
        je .shrb
        shl bx,cl
        jmp .shlb
.shrb
        shr bx,1
.shlb
        mov word[vesa2_bfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov word[vesa2_btrcl],bx
        xor bx,0FFFFh
        mov word[vesa2_btrcla],bx

        xor word[vesa2_clbit],0FFFFh
        call genfulladdtab
        cmp byte[converta],1
         je .red10
         mov eax,565
         jmp .red11
         .red10
         mov eax,555
         .red11
         push eax
         call Init_2xSaIMMXW
         pop eax


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
    cmp bh,1
    je near .input2
    mov dword[eax],54
    mov dword[eax+4],28
    mov dword[eax+8],200
    mov dword[eax+12],208
    mov dword[eax+16],203
    mov dword[eax+20],205
    mov dword[eax+24],31
    mov dword[eax+28],45
    mov dword[eax+32],32
    mov dword[eax+36],30
    mov dword[eax+40],44
    mov dword[eax+44],46
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


SECTION .bss
; ****************************
; TCP/IP Stuff
; ****************************

; TCPIPPortNum
NEWSYM TCPIPStatus, resb 1
NEWSYM PacketSendSize, resd 1
NEWSYM PacketRecvSize, resd 1
NEWSYM PacketRecvPtr,  resd 1
NEWSYM PacketSendArray, resb 2048+256
NEWSYM PacketRecvArray, resb 2048+256
NEWSYM IPAddrStr, resb 20
NEWSYM RemoteDisconnect, resb 1
SECTION .text

NEWSYM TCPIPStartServer
    mov byte[RemoteDisconnect],0
    pushad
    mov dword[PacketSendSize],0
    mov dword[PacketRecvSize],0
    call InitTCP
    or eax,eax
    jnz .failed
    mov byte[TCPIPStatus],1
;    StartServer(unsigned short port)
    xor eax,eax
    mov ax,[TCPIPPortNum]
    push eax
    call StartServerCycle
    add esp,4
    or eax,eax
    jnz .failed
    mov byte[TCPIPStatus],2
    popad
    xor eax,eax
    ret
.failed
    popad
    mov eax,-1
    ret

NEWSYM TCPIPWaitForConnection
    mov byte[RemoteDisconnect],0
    pushad
    call ServerCheckNewClient
    mov [.temp],eax
    cmp eax,1
    jne .notwaiting
    call acceptzuser
    or eax,eax
    jnz .failed
.notwaiting
    popad
    mov eax,[.temp]
    ret
.failed
    popad
    mov eax,-1
    ret
SECTION .bss
.temp resd 1
SECTION .text

NEWSYM TCPIPInitConnectToServer
    pushad
    mov dword[PacketSendSize],0
    mov dword[PacketRecvSize],0
    call InitTCP
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
    popad
    xor eax,eax
    ret
.failed
    popad
    mov eax,1
    ret

NEWSYM TCPIPConnectToServer
; int ConnectServer(char *servername, unsigned short port)
    pushad
    xor eax,eax
    mov ax,[TCPIPPortNum]
    push eax
    mov eax,IPAddrStr
    push eax
    xor eax,eax
    call ConnectServer
    add esp,8
    or eax,eax
    jnz .noclient
    mov byte[TCPIPStatus],3
    popad
    xor eax,eax
    ret
.noclient
    mov [.temp],eax
    popad
    mov eax,[.temp]
    ret
SECTION .bss
.temp resd 1
SECTION .text

NEWSYM TCPIPConnectToServerW
; int ConnectServer(char *servername, unsigned short port)
    pushad
    xor eax,eax
    call WaitForServer
    or eax,eax
    jnz .foundclient
    mov byte[TCPIPStatus],3
    popad
    xor eax,eax
    ret
.foundclient
    mov [.temp],eax
    popad
    mov eax,[.temp]
    ret
SECTION .bss
.temp resd 0
SECTION .text

NEWSYM TCPIPStoreByte
    ; Store al into the array
;    cmp dword[PacketSendSize],2048
;    je .packeterror
    push ebx
    mov ebx,[PacketSendSize]
    mov [PacketSendArray+ebx],al
    pop ebx
    inc dword[PacketSendSize]
    ret
.packeterror
    mov byte[RemoteDisconnect],1
    ret

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
    call SendData
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
    mov byte[RemoteDisconnect],1
    ret

NEWSYM TCPIPSendPacketUDP
    cmp dword[PacketSendSize],0
    je .nopacket
    pushad
    ; Send PacketSendArray with size of PacketSendSize
    ; SendData(int dsize,char *dptr)
    mov eax,PacketSendArray
    push eax
    mov eax,[PacketSendSize]
    push eax
    call SendDataUDP
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
    mov byte[RemoteDisconnect],1
    ret

NEWSYM TCPIPRecvPacket
    pushad
    ; Store packet to PacketRecvArray, size at PacketRecvSize
    ; int GetData(int dsize,char *dptr)
    mov eax,PacketRecvArray
    push eax
    mov eax,2048
    push eax
    call GetData
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
    mov byte[RemoteDisconnect],1
    ret

NEWSYM TCPIPDisconnect
    call DeInitTCP
    cmp byte[TCPIPStatus],2
    jne .notserver
    call StopServer
.notserver
    cmp byte[TCPIPStatus],3
    jne .notclient
    call Disconnect
.notclient
    mov byte[TCPIPStatus],0
    ret

NEWSYM TCPIPPreparePacket
    mov dword[PacketSendSize],0
    ret

NEWSYM ClearUDPStuff
    pushad
    call UDPClearVars
    popad
    ret

NEWSYM Wait1SecWin
    pushad
    call UDPWait1Sec
    popad
    ret

NEWSYM EnableSUDPPacket
    pushad
    call UDPEnableMode
    popad
    ret

NEWSYM DisableSUDPPacket
    pushad
    call UDPDisableMode
    popad
    ret

NEWSYM WinErrorA
    call WinErrorA2
    ret
NEWSYM WinErrorB
    call WinErrorB2
    ret
NEWSYM WinErrorC
    call WinErrorC2
    ret

NEWSYM GotoHomepage
    pushad
    call ZsnesPage
    popad
    ret

NEWSYM GetTimeInSeconds
    push dword SystemTime
    call [_imp__GetLocalTime@4]
    movzx eax,word[SystemTime.wHour]
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTime.wMinute]
    add eax,ebx
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTime.wSecond]
    add eax,ebx
    ret

SECTION .bss

SystemTime:
.wYear                  resw    1
.wMonth                 resw    1
.wDayOfWeek             resw    1
.wDay                   resw    1
.wHour                  resw    1
.wMinute                resw    1
.wSecond                resw    1
.wMilliseconds          resw    1


SECTION .text
%ifdef __MINGW__
NEWSYM WinIntRFAsmEnd
%endif

%if 0

; here's some code to blur a 512x448 region inside a 640x wide buffer
; code "borrowed" from KEGA 0.04b

%macro blurcrap 0
	mov			eax,ebx
	mov			ebx,ecx
	mov			ecx,dword [edi+4]
	shr			ecx,1
	and			ecx,ebp
	mov			edx,ecx
	add			edx,eax
	shr			edx,1
	and			edx,ebp
	add			edx,ebx
	mov			word [edi+2],dx
	push		edx
	shr			edx,1
	shr			esi,1
	and			edx,ebp
	and			esi,ebp
	add			edx,esi
	mov			word [edi],dx
	pop			esi
	add			edi,4
%endmacro

NEWSYM fastblur
	push		ebx
	push		ecx
	push		edx
	push		esi
	push		edi
	push		ebp
	mov			ebp, 0111101111101111b
	cmp			dword [converta], 1
	jne			.565
	mov			ebp, 0011110111101111b
.565
	mov			edi, [esp+1Ch]
	mov			ecx, 448

.loop
	push		ecx

	xor			ebx, ebx
	mov			ecx, dword [edi]
	xor			esi, esi
	shr			ecx, 1
	and			ecx, ebp

%rep 256
	blurcrap
%endrep

	pop			ecx

	add			edi, 256

	dec			ecx
	jne			.loop

	pop			ebp
	pop			edi
	pop			esi
	pop			edx
	pop			ecx
	pop			ebx
	ret

%endif