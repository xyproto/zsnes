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

%include "macros.mac"

EXTSYM CMDLineStr,JoyAC,JoyBC,JoyLC,JoyRC,JoySelec,JoyStart
EXTSYM JoyXC,JoyYC,LoadDir,LoadDrive,SRAMDrive,mode7tab
EXTSYM pl1selk,pl1startk,pl1upk,pl2Ak,pl2Bk,pl2Lk,pl2Rk
EXTSYM pl2Xk,pl2Yk,pl2contrl,pl2downk,pl2leftk,pl2rightk
EXTSYM pl2selk,pl2startk,pl2upk,DontSavePath
EXTSYM FPUCopy,Force8b,MusicRelVol,SRAMDir,SoundCompD,SoundQuality
EXTSYM StereoSound,antienab,cvidmode,enterpress,frameskip,guioff
EXTSYM newengen,per2exec,pl1Ak,pl1Bk,pl1Lk,pl1Rk,pl1Xk,pl1Yk
EXTSYM pl1contrl,pl1downk,pl1leftk,pl1rightk,scanlines,soundon
EXTSYM spcon,vsyncon,Open_File,Read_File
EXTSYM Create_File,Write_File,Close_File
%ifdef __LINUX__
; if TextFile==0, zlib functions aren't used
; useful to save the config file
EXTSYM TextFile, InitDir, InitDrive, Change_Dir
%endif

NEWSYM CfgLoadAsmStart



; [BITS 32]
; [ORG 0]

; .CFG variables
SECTION .data
NEWSYM cfgsoundon,      db 0
NEWSYM cfgSoundQuality, db 2
NEWSYM cfgStereoSound,  db 0
NEWSYM cfgguioff,       db 0
NEWSYM cfgper2exec,     db 100
NEWSYM cfgcvidmode,     db 1
NEWSYM cfgscanline,     db 0
NEWSYM cfginterp,       db 0
NEWSYM cfgenterskip,    db 0
NEWSYM cfgforce8b,      db 0
NEWSYM cfgloadsdir,     db 0
NEWSYM cfgloadgdir,     db 0
NEWSYM cfgnewgfx,       db 0
NEWSYM cfgcopymethod,   db 0
NEWSYM cfgvsync,        db 0
NEWSYM cfgvolume,       db 75
NEWSYM cfgecho,         db 0
NEWSYM RevStereo,       db 0
NEWSYM JoyStatRead,     db 0
NEWSYM pl12s34,         db 0
NEWSYM cfgdontsave,     db 0
NEWSYM cfgreinittime,   db 30
NEWSYM LoadDriveB, db 2
NEWSYM LoadDirB, times 128 db 0
SECTION .text

%macro checkitemcfg 3
    cmp al,%1
    jne %3
    mov al,%2
    jmp .insertnum
%3
%endmacro

%macro checkitemcfg2 3
    cmp al,%1
    jne %3
    mov eax,%2
    jmp .insertnum2
%3
%endmacro


%macro ConvertJoyMapHelp 2
    mov cl,[%1]
    cmp cl,bl
    ja %%skip
    or cl,cl
    jz %%skip
    add cl,81h
    mov [%2],cl
%%skip
%endmacro

NEWSYM ConvertJoyMap
    cmp byte[JoyStatRead],1
    je .convert
    ret
.convert
    call ConvertJoyMap1
    call ConvertJoyMap2
    ret

NEWSYM ConvertJoyMap1
    pushad
    mov al,[pl1contrl]
    ; Convert if 2,4,6, or sidewinder
    cmp al,2
    jne .no2button
    mov byte[pl1Bk],83h
    mov byte[pl1Yk],82h
    mov byte[pl1upk],0CCh
    mov byte[pl1downk],0CDh
    mov byte[pl1leftk],0CEh
    mov byte[pl1rightk],0CFh
.no2button
    cmp al,3
    je .4button
    xor bl,bl
    cmp al,4
    jne .no4button
.4button
    mov bl,4
.no4button
    cmp al,5
    jne .no6button
    mov bl,6
.no6button
    cmp bl,0
    je near .nojoy1
    ; Convert button data
    mov byte[pl1upk],0CCh
    mov byte[pl1downk],0CDh
    mov byte[pl1leftk],0CEh
    mov byte[pl1rightk],0CFh
    ConvertJoyMapHelp JoyStart,pl1startk
    ConvertJoyMapHelp JoySelec,pl1selk
    ConvertJoyMapHelp JoyBC,pl1Yk
    ConvertJoyMapHelp JoyYC,pl1Xk
    ConvertJoyMapHelp JoyAC,pl1Bk
    ConvertJoyMapHelp JoyXC,pl1Ak
    ConvertJoyMapHelp JoyLC,pl1Lk
    ConvertJoyMapHelp JoyRC,pl1Rk
.nojoy1
    cmp al,6
    jne .nosw
    mov byte[pl1upk],0D4h
    mov byte[pl1downk],0D5h
    mov byte[pl1leftk],0D6h
    mov byte[pl1rightk],0D7h
    mov byte[pl1startk],0C8h
    mov byte[pl1selk],0C9h
    mov byte[pl1Ak],089h
    mov byte[pl1Bk],088h
    mov byte[pl1Xk],08Ch
    mov byte[pl1Yk],08Bh
    mov byte[pl1Lk],08Eh
    mov byte[pl1Rk],08Fh
.nosw
    popad
    ret

NEWSYM ConvertJoyMap2
    pushad
    mov al,[pl2contrl]
    ; Convert if 2,4,6, or sidewinder
    ;If pl1contrl=2 and pl2contrl=2, then set pl2 buttons to 3 & 4
    cmp al,2
    jne .no2button2
    cmp byte[pl1contrl],2
    je .pl2b2
    mov byte[pl2Bk],83h
    mov byte[pl2Yk],82h
    mov byte[pl2upk],0CCh
    mov byte[pl2downk],0CDh
    mov byte[pl2leftk],0CEh
    mov byte[pl2rightk],0CFh
    jmp .finpl2
.pl2b2
    mov byte[pl2Bk],85h
    mov byte[pl2Yk],84h
    mov byte[pl2upk],0E8h
    mov byte[pl2downk],0E9h
    mov byte[pl2leftk],0EAh
    mov byte[pl2rightk],0EBh
.no2button2
.finpl2
    cmp al,3
    je .4button2
    xor bl,bl
    cmp al,4
    jne .no4button2
.4button2
    mov bl,4
.no4button2
    cmp al,5
    jne .no6button2
    mov bl,6
.no6button2
    cmp bl,0
    je near .nojoy2
    ; Convert button data
    mov byte[pl2upk],0CCh
    mov byte[pl2downk],0CDh
    mov byte[pl2leftk],0CEh
    mov byte[pl2rightk],0CFh
    ConvertJoyMapHelp JoyStart,pl2startk
    ConvertJoyMapHelp JoySelec,pl2selk
    ConvertJoyMapHelp JoyBC,pl2Yk
    ConvertJoyMapHelp JoyYC,pl2Xk
    ConvertJoyMapHelp JoyAC,pl2Bk
    ConvertJoyMapHelp JoyXC,pl2Ak
    ConvertJoyMapHelp JoyLC,pl2Lk
    ConvertJoyMapHelp JoyRC,pl2Rk
.nojoy2
    ;If both sidewinder, set pl2 buttons to sw2
    cmp al,6
    jne near .nosw2
    cmp byte[pl1contrl],6
    je .pl2sw2
    mov byte[pl2upk],0D4h
    mov byte[pl2downk],0D5h
    mov byte[pl2leftk],0D6h
    mov byte[pl2rightk],0D7h
    mov byte[pl2startk],0C8h
    mov byte[pl2selk],0C9h
    mov byte[pl2Ak],089h
    mov byte[pl2Bk],088h
    mov byte[pl2Xk],08Ch
    mov byte[pl2Yk],08Bh
    mov byte[pl2Lk],08Eh
    mov byte[pl2Rk],08Fh
    jmp .nosw2
.pl2sw2
    mov byte[pl2upk],0DCh
    mov byte[pl2downk],0DDh
    mov byte[pl2leftk],0DEh
    mov byte[pl2rightk],0DFh
    mov byte[pl2startk],0D0h
    mov byte[pl2selk],0D1h
    mov byte[pl2Ak],091h
    mov byte[pl2Bk],090h
    mov byte[pl2Xk],094h
    mov byte[pl2Yk],093h
    mov byte[pl2Lk],096h
    mov byte[pl2Rk],097h
    mov byte[pl2contrl],7
.nosw2
    popad
    ret

NEWSYM DOScreatenewcfg
    ; make a new file
    ; copy .cfgfiledata to mode7tab, replacing each %
%ifdef __LINUX__
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
%endif
    mov esi,.cfgfiledata
    mov edi,mode7tab
    xor ecx,ecx
    mov edx,.cfgfilesize
.loop
    mov al,[esi]
    cmp al,'%'
    je .look4item
    mov [edi],al
    inc esi
    inc edi
    inc ecx
    dec edx
    jnz .loop
    cmp byte[cfgdontsave],1
    je .failed
    push ecx
    ; Save .CFG file
    mov edx,CMDLineStr
%ifdef __GZIP__
    mov byte[TextFile], 1
%endif
    call Create_File
    pop ecx
    jc .failed
    ; Save 65816 status, etc.
    mov bx,ax
    mov edx,mode7tab
    call Write_File
    call Close_File
%ifdef __GZIP__
    mov byte[TextFile], 0
%endif
.failed
    ret

.look4item
    inc esi
    dec edx
    mov al,[esi]
    inc esi
    dec edx
    mov byte[.temp],0
    mov byte[.temp2],1
    mov bl,[frameskip]
    cmp bl,0
    je .yframeskip
    dec bl
    mov [.temp],bl
    mov byte[.temp2],0
.yframeskip
    checkitemcfg '1', [.temp],.check1
    checkitemcfg '2', [.temp2],.check2
    checkitemcfg '3', [pl1contrl],.check3
    checkitemcfg '4', [pl2contrl],.check4
    checkitemcfg2 '5', [pl1rightk],.check5
    checkitemcfg2 '6', [pl1leftk],.check6
    checkitemcfg2 '7', [pl1downk],.check7
    checkitemcfg2 '8', [pl1upk],.check8
    checkitemcfg2 '9', [pl1startk],.check9
    checkitemcfg2 'A', [pl1selk],.checkA
    checkitemcfg2 'B', [pl1Bk],.checkB
    checkitemcfg2 'C', [pl1Yk],.checkC
    checkitemcfg2 'D', [pl1Ak],.checkD
    checkitemcfg2 'E', [pl1Xk],.checkE
    checkitemcfg2 'F', [pl1Lk],.checkF
    checkitemcfg2 'G', [pl1Rk],.checkG
    checkitemcfg2 'H', [pl2rightk],.checkH
    checkitemcfg2 'I', [pl2leftk],.checkI
    checkitemcfg2 'J', [pl2downk],.checkJ
    checkitemcfg2 'K', [pl2upk],.checkK
    checkitemcfg2 'L', [pl2startk],.checkL
    checkitemcfg2 'M', [pl2selk],.checkM
    checkitemcfg2 'N', [pl2Bk],.checkN
    checkitemcfg2 'O', [pl2Yk],.checkO
    checkitemcfg2 'P', [pl2Ak],.checkP
    checkitemcfg2 'Q', [pl2Xk],.checkQ
    checkitemcfg2 'R', [pl2Lk],.checkR
    checkitemcfg2 'S', [pl2Rk],.checkS
    checkitemcfg 'T', [cfgper2exec],.checkT
    checkitemcfg 'U', [cfgcvidmode],.checkU
    checkitemcfg 'V', [cfgsoundon],.checkV
    checkitemcfg 'W', [cfgSoundQuality],.checkW
    checkitemcfg 'X', [cfgStereoSound],.checkX
    checkitemcfg 'Y', [cfgguioff],.checkY
    checkitemcfg 'Z', [cfgscanline],.checkZ
    checkitemcfg 'a', [cfginterp],.checka
    checkitemcfg 'b', [cfgenterskip],.checkb
    checkitemcfg 'c', [cfgforce8b],.checkc
    cmp al,'d'
    je near .insertdira
    cmp al,'e'
    je near .insertdirb
    checkitemcfg 'f', [JoyStart],.checkf
    checkitemcfg 'g', [JoySelec],.checkg
    checkitemcfg 'h', [JoyBC],.checkh
    checkitemcfg 'i', [JoyYC],.checki
    checkitemcfg 'j', [JoyAC],.checkj
    checkitemcfg 'k', [JoyXC],.checkk
    checkitemcfg 'l', [JoyLC],.checkl
    checkitemcfg 'm', [JoyRC],.checkm
    checkitemcfg 'n', [cfgnewgfx],.checkn
    checkitemcfg 'o', [cfgcopymethod],.checko
    checkitemcfg 'p', [cfgvsync],.checkp
    checkitemcfg 'q', [cfgvolume],.checkq
    checkitemcfg 'r', [cfgecho],.checkr
    checkitemcfg 's', [RevStereo],.checks
    checkitemcfg 't', [pl12s34],.checkt
    checkitemcfg 'u', [cfgdontsave],.checku
    checkitemcfg 'v', [cfgreinittime],.checkv
    jmp .loop

.temp db 0
.temp2 db 0

.insertnum
    ; preserve ecx and edx, output al into edi
    cmp al,0
    jne .nozero
    mov byte[edi],'0'
    inc edi
    inc ecx
    jmp .loop
.nozero
    cmp al,99
    ja .digits3
    xor ah,ah
    mov bl,10
    div bl
    add al,48
    add ah,48
    cmp al,48
    jne .digits2
    mov byte[edi],ah
    inc edi
    inc ecx
    jmp .loop
.digits2
    mov byte[edi],al
    mov byte[edi+1],ah
    add edi,2
    add ecx,2
    jmp .loop
.digits3
    xor ah,ah
    mov bl,100
    div bl
    add al,48
    mov byte[edi],al
    mov al,ah
    xor ah,ah
    mov bl,10
    div bl
    add al,48
    add ah,48
    mov byte[edi+1],al
    mov byte[edi+2],ah
    add edi,3
    add ecx,3
    jmp .loop

.insertnum2
    ; preserve ecx and edx, output al into edi
    cmp eax,0
    jne .nozeroi
    mov byte[edi],'0'
    inc edi
    inc ecx
    jmp .loop
.nozeroi
    cmp eax,99
    ja .digits3i
    xor ah,ah
    mov bl,10
    div bl
    add al,48
    add ah,48
    cmp al,48
    jne .digits2i
    mov byte[edi],ah
    inc edi
    inc ecx
    jmp .loop
.digits2i
    mov byte[edi],al
    mov byte[edi+1],ah
    add edi,2
    add ecx,2
    jmp .loop
.digits3i
    mov bl,100
    div bl
    add al,48
    mov byte[edi],al
    mov al,ah
    xor ah,ah
    mov bl,10
    div bl
    add al,48
    add ah,48
    mov byte[edi+1],al
    mov byte[edi+2],ah
    add edi,3
    add ecx,3
    jmp .loop

.insertdira
    cmp byte[cfgloadsdir],1
    je .yesloadsdir
    jmp .loop
.yesloadsdir
%ifndef __LINUX__
    mov al,[SRAMDrive]
    add al,65
    mov [edi],al
    mov byte[edi+1],':'
    mov byte[edi+2],'\'
    add edi,3
    add ecx,3
%endif    
    mov ebx,SRAMDir
.nextinsertdira
    mov al,[ebx]
    cmp al,0
    je .nomoredira
    mov [edi],al
    inc edi
    inc ecx
    inc ebx
    jmp .nextinsertdira
.nomoredira
    jmp .loop

.insertdirb
    cmp byte[DontSavePath],1
    je .insertdirc
%ifndef __LINUX__  
    mov al,[LoadDrive]
    add al,65
    mov [edi],al
    mov byte[edi+1],':'
    mov byte[edi+2],'\'
    add edi,3
    add ecx,3
%endif    
    mov ebx,LoadDir
.nextinsertdir
    mov al,[ebx]
    cmp al,0
    je .nomoredir
    mov [edi],al
    inc edi
    inc ecx
    inc ebx
    jmp .nextinsertdir
.nomoredir
    jmp .loop

.insertdirc
%ifndef __LINUX__
    mov al,[LoadDriveB]
    add al,65
    mov [edi],al
    mov byte[edi+1],':'
    mov byte[edi+2],'\'
    add edi,3
    add ecx,3
%endif    
    mov ebx,LoadDirB
.nextinsertdirc
    mov al,[ebx]
    cmp al,0
    je .nomoredirc
    mov [edi],al
    inc edi
    inc ecx
    inc ebx
    jmp .nextinsertdirc
.nomoredirc
    jmp .loop

.cfgfiledata
db '; ZSNES Configuration file',13,10
db '',13,10
db '; Frame Skip = 0 .. 9',13,10
db '',13,10
db 'FrameSkip = %1',13,10
db '',13,10
db '; Auto Frame Skip = 0 or 1 (1 = ON)',13,10
db '',13,10
db 'AutoFrameSkip = %2',13,10
db '',13,10
db '; Player 1/2 Input Device.  Use the GUI to set these values',13,10
db '; NOTE : Using this to select joysticks manually will NOT work!',13,10
db '',13,10
db 'Player1Device = %3',13,10
db 'Player2Device = %4',13,10
db '',13,10
db '; Keyboard Scancodes/Joystick Mappings for Keyboard 1 & 2',13,10
db '; In order of Right, Left, Down, Up, Start, Select, B, Y, A, X, L, R',13,10
db '; Use the GUI to set these values',13,10
db '',13,10
db 'ScanKey1 = %5, %6, %7, %8, %9, %A, %B, %C, %D, %E, %F, %G',13,10
db 'ScanKey2 = %H, %I, %J, %K, %L, %M, %N, %O, %P, %Q, %R, %S',13,10
db '',13,10
db '; Share Player 3 and 4 control inputs with Player 1 and 2 to allow',13,10
db '; 2 devices to be shared on a single player.  This feature automatically',13,10
db '; disables MultiTap (Multiplayer 5) support.  Set this to 1 to enable.',13,10
db '',13,10
db 'Pl34to12Share = %t',13,10
db '',13,10
db '; Percent to Execute [50 .. 150]',13,10
db '',13,10
db 'Execute = %T',13,10
db '',13,10
;db '; Copy Method, 0 = Normal, 2 = MMX',13,10
;db '',13,10
;db 'CopyMethod = %o',13,10
;db '',13,10
%ifdef __WIN32__
db '; Video Mode, 0 - 21',13,10
db ';   0 = 64x56 R WIN           1 = 128x112 R WIN',13,10
db ';   2 = 256X224 R WIN         3 = 256x224 R FULL',13,10
db ';   4 = 512X448 R WIN         5 = 512X448 DR WIN',13,10
db ';   6 = 640x480 S WIN         7 = 640x480 DR FULL',13,10
db ';   8 = 640X480 DS FULL       9 = 640X480 S FULL',13,10
db ';   10 = 800x600 S WIN        11 = 800x600 DS WIN',13,10
db ';   12 = 800x600 S FULL       13 = 800x600 DS FULL',13,10
db ';   14 = 1024X768 S WIN       15 = 1024X768 DS WIN',13,10
db ';   16 = 1024x768 S FULL      17 = 1024x768 DS FULL',13,10
db ';   18 = 768x672 R WIN        19 = 768x672 DR WIN',13,10
db ';   20 = 1024x896 R WIN       21 = 1024x896 DR WIN',13,10
db '',13,10
db 'VideoMode = %U',13,10
db '',13,10
%elifdef __LINUX__
db '; Video Mode, 0 - 10',13,10
db ';   0 = 320x240x256           1 = 256x256x256',13,10
db ';   2 = 320x240x256 VESA2     3 = 320x240x65536 VESA2',13,10
db ';   4 = 640x480x256 VESA2     5 = 640x480x65536 VESA2',13,10
db ';   6 = 512x384x256 VESA2     7 = 512x384x65536 VESA2',13,10
db ';   8 = 640x480x65536 VESA1.2 9 = 320x480x256 VESA2',13,10
db ';   10 = 320x480x65536 VESA2',13,10
db '',13,10
db 'VideoMode = %U',13,10
db '',13,10
%elifdef __MSDOS__
db '; Video Mode, 0 - 10',13,10
db ';   0 = 320x240x256           1 = 256x256x256',13,10
db ';   2 = 320x240x256 VESA2     3 = 320x240x65536 VESA2',13,10
db ';   4 = 640x480x256 VESA2     5 = 640x480x65536 VESA2',13,10
db ';   6 = 512x384x256 VESA2     7 = 512x384x65536 VESA2',13,10
db ';   8 = 640x480x65536 VESA1.2 9 = 320x480x256 VESA2',13,10
db ';   10 = 320x480x65536 VESA2',13,10
db '',13,10
db 'VideoMode = %U',13,10
db '',13,10
%endif
db '; Sound Emulation = 0 or 1 (1 = ON)',13,10
db '',13,10
db 'Sound = %V',13,10
db '',13,10
db '; Sound Sampling Rate',13,10
db ';   0 =  8000Hz, 1 = 11025Hz, 2 = 22050Hz, 3 = 44100Hz',13,10
db ';   4 = 16000Hz, 5 = 32000Hz',13,10
db '',13,10
db 'SoundRate = %W',13,10
db '',13,10
db '; Stereo (0 = off, 1 = on)',13,10
db '',13,10
db 'Stereo = %X',13,10
db '',13,10
db '; Stereo Reversed.  Swaps left channel with right. (0 = off, 1 = L <-> R)',13,10
db '',13,10
db 'ReverseStereo = %s',13,10
db '',13,10
%ifdef __MSDOS__
db '; GUI Disable (1 = Disable GUI, 0 = Enable GUI)',13,10
db '',13,10
db 'GUIDisable = %Y',13,10
db '',13,10
%endif
db '; New Graphics Engine (1 = Enable, 0 = Disable)',13,10
db '; All 256 color modes and 320x240x65536 supported',13,10
db '',13,10
db 'NewGfx = %n',13,10
db '',13,10
db '; Scanlines (0 = Disable, 1 = Full, 2 = 25%, 3 = 50%)',13,10
db '; 256x256x256 or 640x480 modes only (25% and 50% in 640x480x65536 mode only)',13,10
db '',13,10
db 'Scanlines = %Z',13,10
db '',13,10
db '; Interpolation (1 = Enable, 0 = Disable) - 640x480x65536 mode only',13,10
db '; This option also Enables EAGLE          - 640x480x256 mode only',13,10
db '',13,10
db 'Interpolation = %a',13,10
db '',13,10
%ifdef __MSDOS__
db '; VSync (1 = Enable, 0 = Disable) - Wait for Vertical Sync (Fast cpu reqd)',13,10
db '',13,10
db 'VSync = %p',13,10
db '',13,10
db '; Skip Enter Press at Beginning  (1 = Yes, 0 = No)',13,10
db '',13,10
db 'EnterSkip = %b',13,10
db '',13,10
db '; Force 8-bit sound on  (1 = Yes, 0 = No)',13,10
db '',13,10
db 'Force8bit = %c',13,10
db '',13,10
%endif
db '; Disable Echo  (1 = Yes, 0 = No)',13,10
db '',13,10
db 'EchoDisable = %r',13,10
db '',13,10
db '; Sound Volume Level (0 .. 100)',13,10
db '; Note : Setting this too high can cause sound overflow which degrades quality',13,10
db '',13,10
db 'Volume = %q',13,10
db '',13,10
db '; Set this to 1 if you do not want ZSNES to save the configuration files.',13,10
db '',13,10
db 'DontSave = %u',13,10
db '',13,10
db '; Savefile directory.  Leave it blank if you want the save files to be in the',13,10
db '; same directory as the games.  It should be in a format like : C:\dir\dir',13,10
db '',13,10
db 'SaveDirectory = %d',13,10
db '',13,10
db '; Game directory.  This is the directory where the GUI starts at.',13,10
db '; ZSNES automatically writes the current directory here upon exit.',13,10
db '',13,10
db 'GameDirectory = %e',13,10
.cfgfilesize equ $-.cfgfiledata

NEWSYM getcfg
    mov byte[.forceauto],0
    ; open file
    mov edx,CMDLineStr
    call Open_File
    jc .failed
    mov [.fileloc],ax
    mov byte[.eofile],0
.noteof
    call .readstring
    cmp dword[.stralen],1
    jbe .skipstpr
    call .splitstring
.skipstpr
    cmp byte[.eofile],0
    je .noteof
    
    mov bx,[.fileloc]
    call Close_File
    ret

.failed
    call DOScreatenewcfg
    ret

.readstring:
    mov dword[.strlen],0
    mov dword[.stralen],0
    mov byte[.ignore],0
.startloop
    mov bx,[.fileloc]
    mov ecx,1
    mov edx,.cchar
    call Read_File
    cmp eax,0
    je .endoffile
    cmp byte[.cchar],';'
    je .comment
    cmp byte[.cchar],13
    je .eoline
    cmp byte[.cchar],10
    je .startloop
    mov ecx,[.strlen]
    mov al,[.cchar]
    cmp ecx,127
    ja .nocopy
    cmp byte[.ignore],1
    je .nocopy
    mov [.string+ecx],al
    inc dword[.stralen]
.nocopy
    inc dword[.strlen]
    jmp .startloop
    ret
.comment
    inc dword[.strlen]
    mov byte[.ignore],1
    jmp .startloop
    ret
.eoline
    ret
.endoffile
    mov byte[.eofile],1
    ret

.splitstring:
    ; search for ='s
    mov ecx,[.stralen]
    dec ecx
    xor eax,eax
    xor ebx,ebx
.next
    cmp byte[.string+eax],'='
    jne .noeq
    inc ebx
.noeq
    inc eax
    dec ecx
    jnz .next
    cmp ebx,1
    je .onequal
    ret

.onequal
    xor eax,eax
    xor ebx,ebx
    mov ecx,[.stralen]
    mov dword[.strlena],0
    mov dword[.strlenb],0
.loopa
    mov dl,[.string+eax]
    cmp dl,'='
    je .nextb
    cmp dl,' '
    je .skipcopy
    cmp dl,'a'
    jb .nocapneeded
    cmp dl,'z'
    ja .nocapneeded
    sub dl,'a'-'A'
.nocapneeded
    mov [.stringa+ebx],dl
    inc dword[.strlena]
    inc ebx
.skipcopy
    inc eax
    dec ecx
    jnz .loopa
    ret
.nextb
    inc eax
    dec ecx
    jnz .noblank
.yesblank
    ret
.noblank
    xor ebx,ebx
    mov byte[.usespace],0
.loopb
    mov dl,[.string+eax]
    cmp byte[.usespace],0
    jne .nospaceskip
    cmp dl,' '
    je .skipcopyb
.nospaceskip
%ifndef __LINUX__
    cmp dl,'a'
    jb .nocapneededb
    cmp dl,'z'
    ja .nocapneededb
    sub dl,'a'-'A'
%endif    
.nocapneededb
    mov byte[.usespace],1
    mov [.stringb+ebx],dl
    inc dword[.strlenb]
    inc ebx
.skipcopyb
    inc eax
    dec ecx
    jnz .loopb
    cmp dword[.strlena],0
    je .nostr
    cmp dword[.strlenb],0
    je .nostr
    jmp .process
.nostr
    ret

.process
    mov ecx,[.strlena]
    cmp ecx,[.stra]
    jne .nostra
    mov edx,.stra+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostra
    call .getframeskip
.nostra
    mov ecx,[.strlena]
    cmp ecx,[.strb]
    jne .nostrb
    mov edx,.strb+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrb
    call .getautoframeskip
.nostrb
    mov ecx,[.strlena]
    cmp ecx,[.strc]
    jne .nostrc
    mov edx,.strc+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrc
    call .getplayer1device
.nostrc
    mov ecx,[.strlena]
    cmp ecx,[.strd]
    jne .nostrd
    mov edx,.strd+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrd
    call .getplayer2device
.nostrd
    mov ecx,[.strlena]
    cmp ecx,[.stre]
    jne .nostre
    mov edx,.stre+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostre
    call .getscankey1
.nostre
    mov ecx,[.strlena]
    cmp ecx,[.strf]
    jne .nostrf
    mov edx,.strf+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrf
    call .getscankey2
.nostrf
    mov ecx,[.strlena]
    cmp ecx,[.strg]
    jne .nostrg
    mov edx,.strg+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrg
    call .getsound
.nostrg
    mov ecx,[.strlena]
    cmp ecx,[.strh]
    jne .nostrh
    mov edx,.strh+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrh
    call .getsoundrate
.nostrh
    mov ecx,[.strlena]
    cmp ecx,[.stri]
    jne .nostri
    mov edx,.stri+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostri
    call .getvideo
.nostri
    mov ecx,[.strlena]
    cmp ecx,[.strj]
    jne .nostrj
    mov edx,.strj+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrj
    call .getexecute
.nostrj
    mov ecx,[.strlena]
    cmp ecx,[.strk]
    jne .nostrk
    mov edx,.strk+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrk
    call .getbufdisable
.nostrk
    mov ecx,[.strlena]
    cmp ecx,[.strm]
    jne .nostrm
    mov edx,.strm+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrm
    call .getstereo
.nostrm
    mov ecx,[.strlena]
    cmp ecx,[.strn]
    jne .nostrn
    mov edx,.strn+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrn
    call .guidisable
.nostrn
    mov ecx,[.strlena]
    cmp ecx,[.stro]
    jne .nostro
    mov edx,.stro+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostro
    call .scanlines
.nostro
    mov ecx,[.strlena]
    cmp ecx,[.strp]
    jne .nostrp
    mov edx,.strp+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrp
    call .interpolation
.nostrp
    mov ecx,[.strlena]
    cmp ecx,[.strq]
    jne .nostrq
    mov edx,.strq+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrq
    call .enterskip
.nostrq
    mov ecx,[.strlena]
    cmp ecx,[.strr]
    jne .nostrr
    mov edx,.strr+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrr
    call .force8b
.nostrr
    mov ecx,[.strlena]
    cmp ecx,[.strs]
    jne .nostrs
    mov edx,.strs+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrs
    call .getsavedir
.nostrs
    mov ecx,[.strlena]
    cmp ecx,[.strt]
    jne .nostrt
    mov edx,.strt+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrt
    call .getloaddir
.nostrt
    mov ecx,[.strlena]
    cmp ecx,[.stru]
    jne .nostru
    mov edx,.stru+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostru
    call .getjoymapping
.nostru
    mov ecx,[.strlena]
    cmp ecx,[.strv]
    jne .nostrv
    mov edx,.strv+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrv
    call .getnewgfx
.nostrv
    mov ecx,[.strlena]
    cmp ecx,[.strw]
    jne .nostrw
    mov edx,.strw+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrw
    call .getcopymethod
.nostrw
    mov ecx,[.strlena]
    cmp ecx,[.strx]
    jne .nostrx
    mov edx,.strx+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrx
    call .getvsync
.nostrx
    mov ecx,[.strlena]
    cmp ecx,[.stry]
    jne .nostry
    mov edx,.stry+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostry
    call .getvolume
.nostry
    mov ecx,[.strlena]
    cmp ecx,[.strz]
    jne .nostrz
    mov edx,.strz+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostrz
    call .getecho
.nostrz
    mov ecx,[.strlena]
    cmp ecx,[.str1]
    jne .nostr1
    mov edx,.str1+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostr1
    call .getreverses
.nostr1
    mov ecx,[.strlena]
    cmp ecx,[.str2]
    jne .nostr2
    mov edx,.str2+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostr2
    call .getplay1234
.nostr2
    mov ecx,[.strlena]
    cmp ecx,[.str3]
    jne .nostr3
    mov edx,.str3+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostr3
    call .getdontsave
.nostr3
    mov ecx,[.strlena]
    cmp ecx,[.str4]
    jne .nostr4
    mov edx,.str4+4
    mov eax,.stringa
    call .cmpstr
    cmp bl,1
    je .nostr4
    call .getreinit
.nostr4
    ret

.cmpstr
    xor bl,bl
.loopcmp
    mov bh,[eax]
    cmp [edx],bh
    jne .nocmp
    inc eax
    inc edx
    dec ecx
    jnz .loopcmp
    ret
.nocmp
    mov bl,1
    ret

.getframeskip
    cmp dword[.strlenb],1
    jne .noframeskip
    mov al,[.stringb]
    cmp al,'0'
    jb .noframeskip
    cmp al,'9'
    ja .noframeskip
    cmp byte[.forceauto],1
    je .noframeskip
    sub al,47
    mov [frameskip],al
.noframeskip
    ret

.getautoframeskip
    cmp dword[.strlenb],1
    jne .noautoframeskip
    cmp byte[.stringb],'1'
    jne .noautoframeskip
    mov byte[.forceauto],1
    mov byte[frameskip],0
.noautoframeskip
    ret

.getplayer1device
    cmp dword[.strlenb],2
    je .double
    cmp dword[.strlenb],1
    jne .noplay1
    mov al,[.stringb]
    cmp al,'0'
    jb .noplay1
    cmp al,'9'
    ja .noplay1
    sub al,48
    mov [pl1contrl],al
.noplay1
    ret
.double
    mov al,[.stringb]
    cmp al,'1'
    jne .noplay1
    mov al,[.stringb+1]
    cmp al,'0'
    jb .noplay1
    cmp al,'8'
    ja .noplay1
    sub al,38
    mov [pl1contrl],al
    ret

.getplayer2device
    cmp dword[.strlenb],2
    je .double2
    cmp dword[.strlenb],1
    jne .noplay2
    mov al,[.stringb]
    cmp al,'0'
    jb .noplay2
    cmp al,'9'
    ja .noplay2
    sub al,48
    mov [pl2contrl],al
.noplay2
    ret
.double2
    mov al,[.stringb]
    cmp al,'1'
    jne .noplay2
    mov al,[.stringb+1]
    cmp al,'0'
    jb .noplay2
    cmp al,'8'
    ja .noplay2
    sub al,38
    mov [pl2contrl],al
    ret

.getscankey1
    mov ecx,.stringb
    xor ebx,ebx
    xor eax,eax
.checknextscan1
    cmp dword[.strlenb],0
    je near .noscankey1
    mov bl,[ecx]
    inc ecx
    dec dword[.strlenb]
    cmp bl,','
    je near .finstr1
    cmp bl,'0'
    jb near .nextstr1
    cmp bl,'9'
    ja near .nextstr1
    sub bl,48
    push ebx
    mov ebx,10
    mul ebx
    pop ebx
    mov edx,ebx
    and edx,0FFh
    add eax,edx
    cmp dword[.strlenb],0
    je near .finstr1
    jmp .nextstr1
.finstr1
    ; determine which variable according to bh & write
    ; In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
    cmp bh,0
    jne .nextaa
    mov [pl1rightk],eax
.nextaa
    cmp bh,1
    jne .nextba
    mov [pl1leftk],eax
.nextba
    cmp bh,2
    jne .nextca
    mov [pl1downk],eax
.nextca
    cmp bh,3
    jne .nextda
    mov [pl1upk],eax
.nextda
    cmp bh,4
    jne .nextea
    mov [pl1startk],eax
.nextea
    cmp bh,5
    jne .nextfa
    mov [pl1selk],eax
.nextfa
    cmp bh,6
    jne .nextga
    mov [pl1Bk],eax
.nextga
    cmp bh,7
    jne .nextha
    mov [pl1Yk],eax
.nextha
    cmp bh,8
    jne .nextia
    mov [pl1Ak],eax
.nextia
    cmp bh,9
    jne .nextja
    mov [pl1Xk],eax
.nextja
    cmp bh,10
    jne .nextka
    mov [pl1Lk],eax
.nextka
    cmp bh,11
    jne .nextla
    mov [pl1Rk],eax
.nextla
    xor eax,eax
    inc bh
.nextstr1
    jmp .checknextscan1
.noscankey1
    ret

.getscankey2
    mov ecx,.stringb
    xor ebx,ebx
    xor eax,eax
.checknextscan2
    cmp dword[.strlenb],0
    je near .noscankey2
    mov bl,[ecx]
    inc ecx
    dec dword[.strlenb]
    cmp bl,','
    je near .finstr2
    cmp bl,'0'
    jb near .nextstr2
    cmp bl,'9'
    ja near .nextstr2
    sub bl,48
    push ebx
    mov ebx,10
    mul ebx
    pop ebx
    mov edx,ebx
    and edx,0FFh
    add eax,edx
    cmp dword[.strlenb],0
    je near .finstr2
    jmp .nextstr2
.finstr2
    ; determine which variable according to bh & write
    ; In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
    cmp bh,0
    jne .nextaa2
    mov [pl2rightk],eax
.nextaa2
    cmp bh,1
    jne .nextba2
    mov [pl2leftk],eax
.nextba2
    cmp bh,2
    jne .nextca2
    mov [pl2downk],eax
.nextca2
    cmp bh,3
    jne .nextda2
    mov [pl2upk],eax
.nextda2
    cmp bh,4
    jne .nextea2
    mov [pl2startk],eax
.nextea2
    cmp bh,5
    jne .nextfa2
    mov [pl2selk],eax
.nextfa2
    cmp bh,6
    jne .nextga2
    mov [pl2Bk],eax
.nextga2
    cmp bh,7
    jne .nextha2
    mov [pl2Yk],eax
.nextha2
    cmp bh,8
    jne .nextia2
    mov [pl2Ak],eax
.nextia2
    cmp bh,9
    jne .nextja2
    mov [pl2Xk],eax
.nextja2
    cmp bh,10
    jne .nextka2
    mov [pl2Lk],eax
.nextka2
    cmp bh,11
    jne .nextla2
    mov [pl2Rk],eax
.nextla2
    xor eax,eax
    inc bh
.nextstr2
    jmp .checknextscan2
.noscankey2
    ret

.getsound
    cmp dword[.strlenb],1
    jne .nosound
    mov al,[.stringb]
    cmp al,'1'
    jne .nosound
    mov byte[spcon],1        ; SPC Enabled
    mov byte[soundon],1
    mov byte[cfgsoundon],1
.nosound
    ret

.getsoundrate
    cmp dword[.strlenb],1
    jne .nosoundrate
    mov al,[.stringb]
    cmp al,'0'
    jb .nosoundrate
    cmp al,'6'
    ja .nosoundrate
    sub al,48
    mov [SoundQuality],al
    mov [cfgSoundQuality],al
.nosoundrate
    ret

.getvideo
    cmp dword[.strlenb],2
    je .videob
    cmp dword[.strlenb],1
    jne .novideo
    mov al,[.stringb]
    cmp al,'0'
    jb .novideo
    cmp al,'9'
    ja .novideo
    sub al,48
    mov [cvidmode],al
    mov [cfgcvidmode],al
.novideo
    ret
.videob
    cmp byte[.stringb],'1'
    jne .novideo2
    mov al,[.stringb+1]
    sub al,38
    mov [cvidmode],al
    mov [cfgcvidmode],al
    ret
.novideo2
    cmp byte[.stringb],'2'
    jne .novideo
    mov al,[.stringb+1]
    sub al,28
    mov [cvidmode],al
    mov [cfgcvidmode],al
    ret

.getexecute
    cmp dword[.strlenb],0
    je .noexecute
    mov cl,[.strlenb]
    mov esi,.stringb
    mov byte[.per2exec],0
.getnextperc
    mov bl,10
    mov al,[.per2exec]
    cmp al,100
    jae .noexecute
    mul bl
    mov [.per2exec],al
    mov al,[esi]
    cmp al,'0'
    jb .noexecute
    cmp al,'9'
    ja .noexecute
    sub al,48
    add [.per2exec],al
    inc esi                     ; next character
    dec cl
    jz .nonextperc
    jmp .getnextperc
.nonextperc
    cmp byte[.per2exec],150
    ja .noexecute
    cmp byte[.per2exec],50
    jb .noexecute
    mov al,[.per2exec]
    mov [per2exec],al
    mov [cfgper2exec],al
.noexecute
    ret

.getreinit
    cmp dword[.strlenb],0
    je .nogetreinit
    mov cl,[.strlenb]
    mov esi,.stringb
    mov byte[.per2exec],0
.getnextgetreinit
    mov bl,10
    mov al,[.per2exec]
    cmp al,100
    jae .nogetreinit
    mul bl
    mov [.per2exec],al
    mov al,[esi]
    cmp al,'0'
    jb .nogetreinit
    cmp al,'9'
    ja .nogetreinit
    sub al,48
    add [.per2exec],al
    inc esi                     ; next character
    dec cl
    jz .nonextgetreinit
    jmp .getnextgetreinit
.nonextgetreinit
    cmp byte[.per2exec],150
    ja .nogetreinit
    cmp byte[.per2exec],5
    jb .nogetreinit
    mov al,[.per2exec]
    mov [cfgreinittime],al
.nogetreinit
    ret

.getbufdisable
    cmp dword[.strlenb],1
    jne .nobufdisable
    cmp byte[.stringb],'1'
    jne .nobufdisable
    mov byte[SoundCompD],1
.nobufdisable
    ret

.getstereo
    cmp dword[.strlenb],1
    jne .nostereo
    cmp byte[.stringb],'1'
    jne .nostereo
    mov byte[StereoSound],1
    mov byte[cfgStereoSound],1
.nostereo
    ret

.guidisable
    cmp dword[.strlenb],1
    jne .noguidisable
    cmp byte[.stringb],'1'
    jne .noguidisable
    mov byte[guioff],1
    mov byte[cfgguioff],1
.noguidisable
    ret

.scanlines
    cmp dword[.strlenb],1
    jne .noscanlines
    cmp byte[.stringb],'1'
    jne .noscanlines
    mov byte[scanlines],1
    mov byte[cfgscanline],1
.noscanlines
    cmp byte[.stringb],'2'
    jne .noscanlinesh2
    mov byte[scanlines],2
    mov byte[cfgscanline],2
.noscanlinesh2
    cmp byte[.stringb],'3'
    jne .noscanlinesh3
    mov byte[scanlines],3
    mov byte[cfgscanline],3
.noscanlinesh3
    ret

.interpolation
    cmp dword[.strlenb],1
    jne .nointerpolation
    cmp byte[.stringb],'1'
    jne .nointerpolation
    mov byte[antienab],1
    mov byte[cfginterp],1
.nointerpolation
    ret

.enterskip
    cmp dword[.strlenb],1
    jne .noenterskip
    cmp byte[.stringb],'1'
    jne .noenterskip
    mov byte[enterpress],1
    mov byte[cfgenterskip],1
.noenterskip
    ret

.force8b
    cmp dword[.strlenb],1
    jne .noforce8b
    cmp byte[.stringb],'1'
    jne .noforce8b
    mov byte[Force8b],1
    mov byte[cfgforce8b],1
.noforce8b
    ret

.getsavedir
    cmp dword[.strlenb],3
    jb .nosavedir
%ifndef __LINUX__
    cmp byte[.stringb+1],':'
    jne .nosavedir
    cmp byte[.stringb+2],'\'
    jne .nosavedir
    mov byte[cfgloadsdir],1
    mov al,[.stringb]
    sub al,65
    mov [SRAMDrive],al
%else
    mov byte[cfgloadsdir],1
%endif    
    push ecx
    push esi
    push edi
    mov ecx,[.strlenb]
%ifndef __LINUX__    
    sub ecx,3
    mov esi,.stringb+3
%else
    mov esi,.stringb
%endif        
    mov edi,SRAMDir
    cmp ecx,0
    je .ndird
.ndirc
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jnz .ndirc
.ndird
    mov byte[edi],0
    pop edi
    pop esi
    pop ecx
.nosavedir
    ret

.getloaddir
    cmp dword[.strlenb],3
    jb .noloaddir
%ifndef __LINUX__    
    cmp byte[.stringb+1],':'
    jne .noloaddir
    cmp byte[.stringb+2],'\'
    jne .noloaddir
    mov byte[cfgloadgdir],1
    mov al,[.stringb]
    sub al,65
    mov [LoadDrive],al
    mov [LoadDriveB],al
%else
    mov byte[cfgloadgdir],1
%endif        
    push ecx
    push esi
    push edi
    push ebx
    mov ecx,[.strlenb]
%ifndef __LINUX__    
    sub ecx,3
    mov esi,.stringb+3
%else
    mov esi,.stringb
%endif        
    mov edi,LoadDir
    mov ebx,LoadDirB
    cmp ecx,0
    je .ndirb
.ndir
    mov al,[esi]
    mov [edi],al
    mov [ebx],al
    inc esi
    inc edi
    inc ebx
    dec ecx
    jnz .ndir
.ndirb
    mov byte[edi],0
    pop ebx
    pop edi
    pop esi
    pop ecx
.noloaddir
    ret

.getjoymapping
    mov byte[JoyStatRead],1
    mov ecx,.stringb
    xor ebx,ebx
    xor eax,eax
.checknextscan3
    cmp dword[.strlenb],0
    je near .noscankey3
    mov bl,[ecx]
    inc ecx
    dec dword[.strlenb]
    cmp bl,','
    je near .finstr3
    cmp bl,'0'
    jb near .nextstr3
    cmp bl,'9'
    ja near .nextstr3
    sub bl,48
    mov dl,10
    mul dl
    add al,bl
    cmp dword[.strlenb],0
    je near .finstr3
    jmp .nextstr3
.finstr3
    ; determine which variable according to bh & write
    ; In order of Start, Select, B, Y, A, X, L, R
    cmp bh,0
    jne .nextaa3
    mov byte[JoyStart],al
.nextaa3
    cmp bh,1
    jne .nextba3
    mov byte[JoySelec],al
.nextba3
    cmp bh,2
    jne .nextca3
    mov byte[JoyBC],al
.nextca3
    cmp bh,3
    jne .nextda3
    mov byte[JoyYC],al
.nextda3
    cmp bh,4
    jne .nextea3
    mov byte[JoyAC],al
.nextea3
    cmp bh,5
    jne .nextfa3
    mov byte[JoyXC],al
.nextfa3
    cmp bh,6
    jne .nextga3
    mov byte[JoyLC],al
.nextga3
    cmp bh,7
    jne .nextha3
    mov byte[JoyRC],al
.nextha3
    xor eax,eax
    inc bh
.nextstr3
    jmp .checknextscan3
.noscankey3
    ret

.getnewgfx
    cmp dword[.strlenb],1
    jne .nonewgfx
    cmp byte[.stringb],'1'
    jne .nonewgfx
    mov byte[newengen],1
    mov byte[cfgnewgfx],1
.nonewgfx
    ret

.getcopymethod
    cmp dword[.strlenb],1
    jne .nocopymethod
    cmp byte[.stringb],'1'
    jne .nofpu
    mov byte[FPUCopy],0
    mov byte[cfgcopymethod],1
    jmp .nocopymethod
.nofpu
    cmp byte[.stringb],'2'
    jne .nocopymethod
    mov byte[FPUCopy],2
    mov byte[cfgcopymethod],2
.nocopymethod
    ret

.getvsync
    cmp dword[.strlenb],1
    jne .novsync
    cmp byte[.stringb],'1'
    jne .novsync
    mov byte[vsyncon],1
    mov byte[cfgvsync],1
.novsync
    ret

.getecho
    cmp dword[.strlenb],1
    jne .nodecho
    cmp byte[.stringb],'1'
    jne .nodecho
    mov byte[cfgecho],1
.nodecho
    ret

.getplay1234
    cmp dword[.strlenb],1
    jne .nop1234
    cmp byte[.stringb],'1'
    jne .nop1234
    mov byte[pl12s34],1
.nop1234
    ret

.getdontsave
    cmp dword[.strlenb],1
    jne .nodontsave
    cmp byte[.stringb],'1'
    jne .nodontsave
    mov byte[cfgdontsave],1
.nodontsave
    ret

.getreverses
    cmp dword[.strlenb],1
    jne .nors
    cmp byte[.stringb],'1'
    jne .nors
    mov byte[RevStereo],1
.nors
    ret

.getvolume
    cmp dword[.strlenb],0
    je .novolume
    mov cl,[.strlenb]
    mov esi,.stringb
    mov byte[.volume],0
.getnextvol
    mov bl,10
    mov al,[.volume]
    cmp al,100
    jae .novolume
    mul bl
    mov [.volume],al
    mov al,[esi]
    cmp al,'0'
    jb .novolume
    cmp al,'9'
    ja .novolume
    sub al,48
    add [.volume],al
    inc esi                     ; next character
    dec cl
    jz .nonextvol
    jmp .getnextvol
.nonextvol
    cmp byte[.volume],100
    ja .novolume
    mov al,[.volume]
    mov [MusicRelVol],al
    mov [cfgvolume],al
.novolume
    ret

SECTION .data

.per2exec db 0
.volume   db 0
.fileloc dw 0
.eofile  db 0
.ignore  db 0
.strlen  dd 0
.stralen dd 0    ; actual string length
.strlena dd 0
.strlenb dd 0
.cchar   db 0
.forceauto db 0
.string  times 128 db 0  ; full string
.stringa times 128 db 0
.stringb times 128 db 0
.cfgfname db 'zsnes.cfg',0
.stra dd 9
      db 'FRAMESKIP'
.strb dd 13
      db 'AUTOFRAMESKIP'
.strc dd 13
      db 'PLAYER1DEVICE'
.strd dd 13
      db 'PLAYER2DEVICE'
.stre dd 8
      db 'SCANKEY1'
.strf dd 8
      db 'SCANKEY2'
.strg dd 5
      db 'SOUND'
.strh dd 9
      db 'SOUNDRATE'
.stri dd 9
      db 'VIDEOMODE'
.strj dd 7
      db 'EXECUTE'
.strk dd 15
      db 'SOUNDBUFDISABLE'
.strm dd 6
      db 'STEREO'
.strn dd 10
      db 'GUIDISABLE'
.stro dd 9
      db 'SCANLINES'
.strp dd 13
      db 'INTERPOLATION'
.strq dd 9
      db 'ENTERSKIP'
.strr dd 9
      db 'FORCE8BIT'
.strs dd 13
      db 'SAVEDIRECTORY'
.strt dd 13
      db 'GAMEDIRECTORY'
.stru dd 7
      db 'JOYMAP1'
.strv dd 6
      db 'NEWGFX'
.strw dd 10
      db 'COPYMETHOD'
.strx dd 5
      db 'VSYNC'
.stry dd 6
      db 'VOLUME'
.strz dd 11
      db 'ECHODISABLE'
.str1 dd 13
      db 'REVERSESTEREO'
.str2 dd 13
      db 'PL34TO12SHARE'
.str3 dd 8
      db 'DONTSAVE'
.str4 dd 10
      db 'REINITTIME'
.usespace db 0

SECTION .text

NEWSYM CfgLoadAsmEnd
