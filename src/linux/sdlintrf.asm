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

EXTSYM ZFileSystemInit,getcmdline,GUIRestoreVars,getcfg,obtaindir
EXTSYM ConvertJoyMap,tparms,SBHDMA,spcon,cfgsoundon,cfgcvidmode,InitDir
EXTSYM InitDrive,SRAMChdir,DOScreatenewcfg,ExecGUISaveVars,allocptr,putchar
EXTSYM getchar,ZOpenFile,ZOpenMode,ZFileSeek,ZOpenFileName,ZFileSeekMode
EXTSYM ZFileSeekPos,ZFileSeekHandle,ZFileWriteHandle,ZFileWriteSize
EXTSYM ZFileWriteBlock,ZFileWrite,ZFileReadHandle,ZFileReadSize,ZFileReadBlock
EXTSYM ZFileRead,ZFileDelFName,ZFileDelete,ZCloseFileHandle,ZCloseFile
EXTSYM ZFileTellHandle,ZFileTell,ZFFTimeFName,ZFTime,ZFDate,ZFileGetFTime
EXTSYM GetTime,GetDate,GUIkeydelay2,ZFileCHDir,CHPath
EXTSYM ZFileGetDir,DirName,DTALoc,DTALocPos,ZFileFindATTRIB
EXTSYM ZFileFindFirst,ZFileFindNext,ZFileFindPATH,soundon,DSPDisable,Start60HZ
EXTSYM pressed,RaisePitch,AdjustFrequency,vidbufferofsb,vidbuffer,clearwin
EXTSYM Stop60HZ,initwinvideo,vesa2_rpos,vesa2_gpos,vesa2_bpos,vesa2_rposng
EXTSYM vesa2_gposng,vesa2_bposng,vesa2_usbit,vesa2_clbit,vesa2_clbitng
EXTSYM vesa2_clbitng2,vesa2_clbitng3,vesa2red10,res640,res480,cbitmode,cvidmode
EXTSYM vesa2_bits,vesa2_x,vesa2_y,genfulladdtab,GUICPC,drawscreenwin
EXTSYM ConvertToAFormat,HalfTrans,UnusedBit,UnusedBitXor
EXTSYM ngrposng,nggposng,ngbposng,HalfTransB,HalfTransC,UpdateVFrame,GetMouseX
EXTSYM GetMouseY,GetMouseMoveX,GetMouseMoveY,GetMouseButton,T36HZEnabled
EXTSYM MouseButton,Start36HZ,Stop36HZ,BufferSizeW,BufferSizeB,ProcessSoundBuffer
EXTSYM CheckTimers,vesa2_rfull,vesa2_rtrcl,vesa2_rtrcla,vesa2_gfull,vesa2_gtrcl
EXTSYM vesa2_gtrcla,vesa2_bfull,vesa2_btrcl,vesa2_btrcla,Init_2xSaIMMXW
EXTSYM GetLocalTime,V8Mode,GrayscaleMode
EXTSYM PrevWinMode,PrevFSMode,sem_sleep,ZsnesPage

; NOTE: For timing, Game60hzcall should be called at 50hz or 60hz (depending
;   on romispal) after a call to InitPreGame and before DeInitPostGame are
;   made.  GUI36hzcall should be called at 36hz after a call GUIInit and
;   before GUIDeInit.

SECTION .text

NEWSYM StartUp
    call ZFileSystemInit
    ret

; SystemInit - Initialize all Joystick stuff, load in all configuration data,
;   parse commandline data, obtain current directory (One time initialization)

NEWSYM SystemInit
    ; Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    push es
    mov byte[cfgcvidmode],2
    mov byte[cvidmode],2
    call getcmdline
    mov dword[esi],'zsne'
    mov dword[esi+4],'sl.c'
    mov word[esi+8],'fg'
    mov byte[esi+10],0
    mov dword[esi+256],'zgui'
    mov dword[esi+4+256],'cfgl'
    mov dword[esi+8+256],'.dat'
    mov byte[esi+12+256],0

    mov byte[spcon],1
    mov byte[soundon],1
    mov byte[cfgsoundon],1

    ; Get and set the initial directory
%ifdef __UNIXSDL__
    call obtaindir
    mov ebx,InitDir
    mov edx,InitDrive
    call Get_Dir

    pushad
    call SRAMChdir
    popad
%else
    mov ebx,InitDir
    mov edx,InitDrive
    call Get_Dir

    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir
%endif

    pushad
    call GUIRestoreVars                 ; Load GUI stuff
    popad
    call getcfg                         ; Load cfg stuff
    call obtaindir                      ; Get Save/Init Directories
    call ConvertJoyMap                  ; Mini joystick init
    call tparms
    pushad
    call SRAMChdir
    popad
    mov byte[SBHDMA],1
    pop es
    ret

; Configuration save re-routing functions.  You can comment these out
;   for debugging purposes or change it if you're using a different
;   configuration format
NEWSYM createnewcfg
    call DOScreatenewcfg
    ret
NEWSYM GUISaveVars
    pushad
    call ExecGUISaveVars
    popad
    ret

; Allocate memory - see allocptr in ui.asm for details on what to allocate
NEWSYM allocmem
    call allocptr
    ret


NEWSYM PrintChar
    ret
    ; print character at dl, push all modified registers

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
    pop edx
    inc edx
    jmp .next
.finish
    popad
    ret

SECTION .data
NEWSYM wfkey, db 0

SECTION .text
NEWSYM WaitForKey       ; Wait for a key to be pressed
    pushad
    call getchar
    mov [wfkey],al
    popad
    mov al,[wfkey]
    ; return key in al
    ret

SECTION .data
NEWSYM TempHandle, dd 0

SECTION .text
NEWSYM Open_File
    pushad
    mov dword[ZOpenMode],0
    mov [ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov [TempHandle],eax
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov [ZFileSeekHandle],eax
    call ZFileSeek
    popad
    mov ax,[TempHandle]
    clc
    ret
.error
    popad
    stc
    ret

NEWSYM Open_File_Write
    pushad
    mov dword[ZOpenMode],2
    mov [ZOpenFileName],edx
    call ZOpenFile
    cmp eax,0FFFFFFFFh
    je .error
    mov [TempHandle],eax
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekPos],0
    mov [ZFileSeekHandle],eax
    call ZFileSeek
    popad
    mov ax,[TempHandle]
    clc
    ret
.error
    popad
    stc
    ret

NEWSYM Create_File
    pushad
    mov dword[ZOpenMode],1
    mov [ZOpenFileName],edx
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

NEWSYM Delete_File
    mov [ZFileDelFName],edx
    pushad
    call ZFileDelete
    popad
    ret

NEWSYM Close_File
    mov dword[ZCloseFileHandle],0
    mov [ZCloseFileHandle],bx
    pushad
    call ZCloseFile
    popad
    clc
    ret

NEWSYM File_Seek
    mov [ZFileSeekPos+2],cx
    mov [ZFileSeekPos],dx
    mov dword[ZFileSeekMode],0
    mov dword[ZFileSeekHandle],0
    mov [ZFileSeekHandle],bx
    pushad
    call ZFileSeek
    popad
    mov ax,dx
    mov dx,cx
    ret

NEWSYM File_Seek_End
    mov [ZFileSeekPos+2],cx
    mov [ZFileSeekPos],dx
    mov dword[ZFileSeekHandle],0
    mov [ZFileSeekHandle],bx
    mov dword[ZFileSeekMode],1
    mov dword[ZFileTellHandle],0
    mov [ZFileTellHandle],bx
    pushad
    call ZFileSeek
    call ZFileTell
    mov [TempVarSeek],eax
    popad
    mov ax,[TempVarSeek]
    mov dx,[TempVarSeek+2]
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
    pushad
    call GetDate
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    movzx edx,al ;Move day into edx, day is in BCD
    shr edx,4    ;Chop off the second digit
    imul edx,10  ;Multiply first digit by 10, since we want decimal
    and al,0xF   ;Remove first BCD digit
    add dl,al    ;Add second digit to first*10
    mov dh,ah    ;Copy month
    ;Year Calculation
    shr eax,16
    movzx ecx,al
    shr ecx,4
    imul ecx,10
    and al,0xF
    add cl,al
    add cx,1900
    ret

NEWSYM Get_File_Date
    mov [ZFFTimeFName],edx
    pushad
    call ZFileGetFTime
    popad
    mov edx,[ZFDate]
    mov ecx,[ZFTime]
    ret

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
    sub ebx,80h
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
    inc al
    and al,0Fh
    mov [Keybtail],al
.none
    ret

SECTION .data
Keybhead db 0
Keybtail db 0
HoldKey dd 0
PKeyBuf times 100h db 0
NEWSYM CurKeyPos, dd 0
NEWSYM CurKeyReadPos, dd 0
NEWSYM KeyBuffer, times 16 dd 0

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

NEWSYM Get_Key
    ; wait if there are no keys in buffer, then return key in al
    ; for extended keys, return a 0, then the extended key afterwards
    xor eax,eax
.nokey
    pushad
    call JoyRead
    popad
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
    sub word[KeyBuffer+eax*4],100h
    xor al,al
    ret

NEWSYM Change_Drive
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

NEWSYM Change_Dir
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

NEWSYM Get_Dir
    mov [DirName],ebx
    pushad
    call ZFileGetDir
    mov eax,[DirName]
    mov ebx,eax
    mov ecx,128
.loop
    mov dl,[eax]
;    cmp dl,'/'
;    jne .noslash
    ;mov dl,'\'
;.noslash
    mov [eax],dl
    inc eax
    dec ecx
    jnz .loop
    popad
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

NEWSYM Set_DTA_Address
    ; Only needed for dos stuff
    ret

NEWSYM Get_Memfree
    mov eax,02000000h
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

NEWSYM superscopepal
    ret

NEWSYM saveselectpal
    ret

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
   mov dword[vesa2_bits],16
   mov dword[vesa2_rpos],11
   mov dword[vesa2_gpos],5
   mov dword[vesa2_bpos],0
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
   mov [PrevFSMode],al
   jmp .doneprevmode
.prevwinmode
   mov [PrevWinMode],al
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


NEWSYM initvideo2 ; ModeQ scanline re-init (Keep blank on non-dos ports)
    ret

NEWSYM deinitvideo
    ret

; ** copy video mode functions **
SECTION .data
NEWSYM converta, dd 0

SECTION .text
NEWSYM DrawScreen               ; In-game screen render w/ triple buffer check
    cmp dword[converta],1
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

NEWSYM vidpastecopyscr       ; GUI screen render
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
   jmp DrawScreen


; ** Clear Screen function **
NEWSYM ClearScreen
   ret

; ** Video Mode Variables **
SECTION .data

; Total Number of Video Modes
%ifdef __OPENGL__
NEWSYM NumVideoModes, dd 22
%else
NEWSYM NumVideoModes, dd 6
%endif

; GUI Video Mode Names - Make sure that all names are of the same length
; and end with a NULL terminator
NEWSYM GUIVideoModeNames
db '256x224       R W',0  ;0
db '256x224       R F',0  ;1
db '512x448      DR W',0  ;2
db '512x448      DR F',0  ;3
db '640x480      DR F',0  ;4
db '800x600      DR F',0  ;5
%ifdef __OPENGL__
db '256x224      OR W',0  ;6
db '512x448     ODR W',0  ;7
db '640x480     ODS F',0  ;8
db '640x480     ODS W',0  ;9
db '640x576     ODR W',0  ;10
db '768x672     ODR W',0  ;11
db '800x600     ODS F',0  ;12
db '800x600     ODS W',0  ;13
db '896x784     ODR W',0  ;14
db '1024x768    ODS F',0  ;15
db '1024x768    ODS W',0  ;16
db '1024x896    ODR W',0  ;17
db '1280x960    ODS F',0  ;18
db '1280x1024   ODS F',0  ;19
db '1600x1200   ODS F',0  ;20
db 'VARIABLE    ODR W',0  ;21

%endif

; Video Mode Feature Availability (1 = Available, 0 = Not Available)
; Left side starts with Video Mode 0
; vid mode column =  0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1
NEWSYM GUI16VID,  db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; 16-bit mode
NEWSYM GUINGVID,  db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; New Graphics Mode Available
NEWSYM GUISLVID,  db 0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; Scanlines
NEWSYM GUIINVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Interpolation
NEWSYM GUII2VID,  db 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Interpolation(w)
NEWSYM GUIEAVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Eagle
NEWSYM GUIIEVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; (Interp | Eagle)
NEWSYM GUIFSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Full Screen
NEWSYM GUIWSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; (Full Scr. | Wide Scr.)
NEWSYM GUISSVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Small Screen
NEWSYM GUITBVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Triple Buffering
NEWSYM GUIHSVID,  db 0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; Half/Quarter Scanlines
NEWSYM GUI2xVID,  db 0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; 2xSaI/Super Eagle Engines
NEWSYM GUIM7VID,  db 0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; ?Mode 7 video thing?
NEWSYM GUIWFVID,  db 0,1,0,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,1,1,1,0     ; If Windows Full Screen
NEWSYM GUIDSIZE,  db 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NEWSYM GUIRATIO,  db 0,1,0,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0
NEWSYM GUIBIFIL,  db 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1     ; binlinear = 1, nearest = 0
NEWSYM GUITBWVID, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; Triple Buffering (Win)
NEWSYM GUIMBVID,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NEWSYM GUIHQ2X,   db 0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1     ; hq2x filter
NEWSYM GUIHQ3X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; hq3x filter
NEWSYM GUIHQ4X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     ; hq4x filter
NEWSYM GUIRESIZE, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1     ; resizable

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
        ret

NEWSYM JoyRead
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
db 'KEYB/GAMEPAD    ',0
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
%ifdef __UNIXSDL__
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','KP7'  ; 40h
        db 'KP8','KP9','KP-','KP4','KP5','KP6','KP+','KP1'
        db 'KP2','KP3','KP0','KP.','   ','   ','OEM','F11'  ; 50h
        db 'F12','HOM',' UP','PGU','LFT','5DH','RGT','END'
        db 'DWN','PGD','INS','DEL','64H','65H','66H','67H'  ; 60h
%else
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','N 7'  ; 40h
        db 'N 8','N 9','N -','N 4','N 5','N 6','N +','N 1'
        db 'N 2','N 3','N 0','N .','   ','   ','OEM','F11'  ; 50h
        db 'F12','59h','5Ah','5BH','5CH','5DH','5EH','5FH'
        db '60H','61H','62H','63H','64H','65H','66H','67H'  ; 60h
%endif
        db '68H','69H','6AH','6BH','6CH','6DH','6EH','6FH'
        db '70H','71H','72H','73H','74H','75H','76H','77H'  ; 70h
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
        ; Joystick Stuff
        db 'J00','J01','J02','J03','J04','J05','J06','J07'  ; 100h
        db 'J08','J09','J0A','J0B','J0C','J0D','J0E','J0F'
        db 'J10','J11','J12','J13','J14','J15','J16','J17'  ; 110h
        db 'J18','J19','J1A','J1B','J1C','J1D','J1E','J1F'
        db 'J20','J21','J22','J23','J24','J25','J26','J27'  ; 120h
        db 'J28','J29','J2A','J2B','J2C','J2D','J2E','J2F'
        db 'J30','J31','J32','J33','J34','J35','J36','J37'  ; 130h
        db 'J38','J39','J3A','J3B','J3C','J3D','J3E','J3F'
        db 'J40','J41','J42','J43','J44','J45','J46','J47'  ; 140h
        db 'J48','J49','J4A','J4B','J4C','J4D','J4E','J4F'
        db 'J50','J51','J52','J53','J54','J55','J56','J57'  ; 150h
        db 'J58','J59','J5A','J5B','J5C','J5D','J5E','J5F'
        db 'J60','J61','J62','J63','J64','J65','J66','J67'  ; 160h
        db 'J68','J69','J6A','J6B','J6C','J6D','J6E','J6F'
        db 'J70','J71','J72','J73','J74','J75','J76','J77'  ; 170h
        db 'J78','J79','J7A','J7B','J7C','J7D','J7E','J7F'
%ifdef __UNIXSDL__
        ; Extra Joystick Stuff
        db 'J80','J81','J82','J83','J84','J85','J86','J87'  ; 180h
        db 'J88','J89','J8A','J8B','J8C','J8D','J8E','J8F'
        db 'J90','J91','J92','J93','J94','J95','J96','J97'  ; 190h
        db 'J98','J99','J9A','J9B','J9C','J9D','J9E','J9F'
        db 'JA0','JA1','JA2','JA3','JA4','JA5','JA6','JA7'  ; 2A0h
        db 'JA8','JA9','JAA','JAB','JAC','JAD','JAE','JAF'
        db 'JB0','JB1','JB2','JB3','JB4','JB5','JB6','JB7'  ; 2B0h
        db 'JB8','JB9','JBA','JBB','JBC','JBD','JBE','JBF'
%else
        ; Extra Stuff (180h) (Parallel Port)
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'  ; 180h
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'  ; 190h
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'  ; 2A0h
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'  ; 2B0h
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
%endif

NEWSYM ZSNESBase, dd 0
TempVarSeek dd 0
gotoroot db '/',0
SECTION .text

; ****************************
; Mouse Stuff
; ****************************

NEWSYM Init_Mouse
    ; return non-zero if successful
    mov eax,01h
    ret

SECTION .data
NEWSYM WMouseX, dd 0
NEWSYM WMouseY, dd 0
NEWSYM WMouseMoveX, dd 0
NEWSYM WMouseMoveY, dd 0
NEWSYM WMouseButton, dd 0

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

NEWSYM Set_MouseXMax
    ret

NEWSYM Set_MouseYMax
    ret

NEWSYM Set_MousePosition
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
    mov dword[BufferSizeB],256
    mov dword[BufferSizeW],512
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
    call sem_sleep
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
        mov [vesa2_rfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov [vesa2_rtrcl],bx
        xor bx,0FFFFh
        mov [vesa2_rtrcla],bx

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
        mov [vesa2_gfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov [vesa2_gtrcl],bx
        xor bx,0FFFFh
        mov [vesa2_gtrcla],bx

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
        mov [vesa2_bfull],bx
        add al,5
        mov bx,1
        mov cl,al
        shl bx,cl
        mov [vesa2_btrcl],bx
        xor bx,0FFFFh
        mov [vesa2_btrcla],bx

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
%ifdef __UNIXSDL__
    mov dword[eax+8],90
    mov dword[eax+12],96
    mov dword[eax+16],92
    mov dword[eax+20],94
%else
    mov dword[eax+8],200
    mov dword[eax+12],208
    mov dword[eax+16],203
    mov dword[eax+20],205
%endif
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

NEWSYM GotoHomepage
    pushad
    call ZsnesPage
    popad
    ret

EXTSYM SystemTimewHour
EXTSYM SystemTimewMinute
EXTSYM SystemTimewSecond

NEWSYM GetTimeInSeconds
    call GetLocalTime
    movzx eax,word[SystemTimewHour]
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTimewMinute]
    add eax,ebx
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTimewSecond]
    add eax,ebx
    ret
