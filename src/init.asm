;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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

EXTSYM DosExit,UpdateDevices,Makemode7Table,MusicRelVol,MusicVol,makesprprtable
EXTSYM romloadskip,start65816,startdebugger,SfxR0,showinfogui,inittable
EXTSYM SA1inittable,MessageOn,Msgptr,MsgCount,sndrot,SnowTimer,inittableb
EXTSYM inittablec,newgfx16b,cfgreinittime,Open_File,Read_File,Write_File
EXTSYM Close_File,Output_Text,Change_Dir,Create_File,SPCDisable,osm2dis
EXTSYM BackupSystemVars,SnowData,SnowVelDist,cvidmode,newengen,cfgnewgfx
EXTSYM GUI16VID,NewEngEnForce,TextFile,Setper2exec,JoyRead,pressed,pl3Ak,pl3Bk
EXTSYM pl3Lk,pl3Rk,pl3Xk,pl3Yk,pl3contrl,pl3downk,pl3leftk,pl3rightk,pl3selk
EXTSYM pl3startk,pl3upk,pl4Ak,pl4Bk,pl4Lk,pl4Rk,pl4Xk,pl4Yk,pl4contrl,pl4downk
EXTSYM pl4leftk,pl4rightk,pl4selk,pl4startk,pl4upk,mousebuttons,mousexdir,pl5Ak
EXTSYM pl5Bk,pl5Lk,pl5Rk,pl5Xk,pl5Yk,pl5contrl,pl5downk,pl5leftk,pl5rightk
EXTSYM pl5selk,pl5startk,pl5upk,mouseydir,mousexpos,mouseypos,snesmouse,sram
EXTSYM processmouse,ssautosw,GUIDelayB,pl12s34,pl1Xtk,pl1Ytk,pl1Atk,pl1Btk
EXTSYM pl2Xtk,pl2Ytk,pl2Atk,pl2Btk,pl3Xtk,pl3Ytk,pl3Atk,pl3Btk,pl4Xtk,pl4Ytk
EXTSYM pl4Atk,pl4Btk,pl1ULk,pl1URk,pl1DLk,pl1DRk,pl2ULk,pl2URk,pl2DLk,pl2DRk
EXTSYM pl3ULk,pl3URk,pl3DLk,pl3DRk,pl4ULk,pl4URk,pl4DLk,pl4DRk,pl5ULk,pl5URk
EXTSYM pl5DLk,pl5DRk,pl5Xtk,pl5Ytk,pl5Atk,pl5Btk,pl1Ltk,pl1Rtk,pl2Ltk,pl2Rtk
EXTSYM pl3Ltk,pl3Rtk,pl4Ltk,pl4Rtk,pl5Ltk,pl5Rtk,Turbo30hz,NumComboLocl
EXTSYM ComboBlHeader,ComboHeader,CombinDataLocl,CombinDataGlob,NumCombo
EXTSYM GUIComboGameSpec,mousexloc,mouseyloc,extlatch,FIRTAPVal0,FIRTAPVal1
EXTSYM FIRTAPVal2,FIRTAPVal3,FIRTAPVal4,FIRTAPVal5,FIRTAPVal6,FIRTAPVal7,INTEnab
EXTSYM JoyAPos,JoyBPos,NMIEnab,SPCROM,VIRQLoc,coladdb,coladdg,coladdr,doirqnext
EXTSYM forceblnk,nmiprevaddrh,nmiprevaddrl,nmiprevline,nmirept,nmistatus
EXTSYM opexec268,opexec268b,opexec268cph,opexec268cphb,opexec358,opexec358b
EXTSYM opexec358cph,spcextraram,opexec358cphb,prevoamptr,reg1read,reg2read
EXTSYM reg3read,reg4read,resolutn,romdata,scrndis,spcP,SPCRAM,spcnumread
EXTSYM tableD,timeron,vidbright,SPC700read,SPC700write,spc700read
EXTSYM GUIReset,InitC4,SA1Reset,SetAddressingModesSA1,SDD1BankA,SPC7110init
EXTSYM RTCinit,InitOBC,memaccessspc7110r8,memaccessspc7110r16,memaccessspc7110w8
EXTSYM memaccessspc7110w16,ram7f,snesmap2,snesmmap,MultiTap,memaccessbankr848mb
EXTSYM memaccessbankr1648mb,procexecloop,ram7fa,wramdata,wramdataa,fname,fnames
EXTSYM GetCurDir,SRAMChdir,cfgloadsdir,fnamest,statefileloc,InitDir,InitDrive
EXTSYM curromspace,infoloc,patchfile,romispal,initregr,initregw,memtabler16
EXTSYM memtabler8,memtablew16,memtablew8,sfxramdata,wramreadptr
EXTSYM wramwriteptr,loadstate2,CMovieExt,MoviePlay,MovieDumpRaw,AllowUDLR

;initc.c
EXTSYM clearmem,clearSPCRAM,PatchUsingIPS,ZOpenFileName,loadROM,SPC7110IndexSize
EXTSYM SPC7PackIndexLoad,IntlEHi,C4Enable,SPC7110Enable,RTCEnable,SA1Enable
EXTSYM SDD1Enable,OBCEnable,SFXEnable,BSEnable,clearvidsound,headerhack,SetupROM

%ifdef __UNIXSDL__
EXTSYM LoadDir,popdir,pushdir
%endif

; Initiation

SECTION .data
NEWSYM regsbackup, times 3019 db 0
NEWSYM forceromtype, db 0
NEWSYM loadedfromgui, db 0
NEWSYM SnowOn, db 0
NEWSYM bgfixer, db 0
NEWSYM bgfixer2, db 0
NEWSYM ReInitLength, dd 0
NEWSYM ForceNewGfxOff, dd 0
NEWSYM SfxAC, db 0
blah times 450 db 0
; FIX STATMAT
NEWSYM autoloadstate, db 0        ; auto load state slot number
NEWSYM autoloadmovie, db 0
NEWSYM ZMVRawDump, db 0

NEWSYM EndMessage
 db '                                                                   ',13,10,0

SECTION .text

EXTSYM cpalval
NEWSYM init
    ; prevents a crash if cpalval gets accessed before initializing
    mov eax,cpalval
    mov ecx,256
.looppal
    mov dword[eax],cpalval
    add eax,4
    dec ecx
    jnz .looppal

    ; Initialize snow stuff
    mov ecx,400
    xor edx,edx
.snowloop
    shl word[SnowData+edx*2],8
    and byte[SnowVelDist+edx],0F7h
    cmp dword[SnowTimer],0
    jne .skip
    or byte[SnowVelDist+edx],08h
.skip
    inc edx
    dec ecx
    jnz .snowloop

    pushad
    call BackupSystemVars
    popad

    xor eax,eax
    mov al,[cfgreinittime]
    mov ebx,50
    mul ebx
    mov [ReInitLength],eax

    xor eax,eax
    mov al,[romtype]
    mov [forceromtype],al
    mov byte[romtype],0
    mov ax,ds
    mov es,ax
    mov eax,regsbackup
    mov ebx,sndrot
    mov ecx,3019
.rbackupl
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    dec ecx
    jnz .rbackupl
    mov byte[virqnodisable],0
    pushad
    call clearmem
    popad
    call inittable
    call inittableb
    call inittablec
    call SA1inittable
    ; SPC Init
    pushad
    call procexecloop
    popad
    ; SNES Init
    pushad
    call Setper2exec
    popad
    call Makemode7Table
    call makesprprtable
    cmp byte[fname],0
    jne .found
    cmp byte[romloadskip],1
    je .noloadfile
.found
    mov byte[romloadskip],0
    call loadfile
    pushad
    call SetupROM
    call showinfogui
    popad
.noloadfile
    call UpdateDevices
    call init65816
    call initregr
    call initregw
    call initsnes

    ; Initialize volume
    xor eax,eax
    xor edx,edx
    mov al,[MusicRelVol]
    shl eax,7
    mov ebx,0A3D70A3Dh
    mul ebx
    shr edx,6
    cmp dl,127
    jb .noof
    mov dl,127
.noof
    mov [MusicVol],dl

; FIX STATMAT
    ; Here's the auto-load ZST file stuff
    cmp byte[autoloadstate],1
    jl .noautoloadstate
    je .enddigits
    mov ebx,[statefileloc]
    sub byte[autoloadstate],1
    cmp byte[autoloadstate],10
    jge .2digits
    mov al,[autoloadstate]
    add al,48
    mov [fnamest+ebx],al
    jmp .enddigits
.2digits
    xor eax,eax
    mov al,[autoloadstate]
    mov dl,10
    div dl
    add al,48
    add ah,48
    mov [fnamest+ebx-1],al
    mov [fnamest+ebx],ah
.enddigits

    ; Load the specified state file
    pushad
    call loadstate2
    popad

    ; Just skip the extension re-setup below if we don't need to do it
    cmp byte[autoloadstate],9
    jbe .noautoloadstate

    ; Put back the 'ST' on the end of the extension as we changed it
    ; above (by placing two digits in the extension). This is so
    ; as not to break any other code later on which depends
    ; on it being present.
  mov ebx,[statefileloc]
%ifdef __UNIXSDL__
    mov word[fnamest+ebx-1],'st'
%else
    mov word[fnamest+ebx-1],'ST'
%endif

.noautoloadstate

    cmp byte[autoloadmovie],1
    jb .noautloadmovie
    cmp byte[autoloadmovie],10
    ja .noautloadmovie
    mov al,[autoloadmovie]
    add al,'0'-1
    cmp al,'0'
    jne .notzero1
    mov al,'v'
.notzero1
    mov [CMovieExt],al

    pushad
    cmp byte[ZMVRawDump],1
    jne .norawdump
    call MovieDumpRaw
    jmp .aftermovieplay
.norawdump
    call MoviePlay
.aftermovieplay
    popad

.noautloadmovie

    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUI16VID+eax],1
    je .yes16b
    mov byte[newengen],1
    mov byte[cfgnewgfx],1
.yes16b
    cmp byte[NewEngEnForce],1
    jne .noforce
    mov byte[NewEngEnForce],0
    mov byte[newengen],1
    mov byte[cfgnewgfx],1
.noforce
    mov ebx,ebm
    mov eax,EndMessage
    mov dh,17h
    mov ch,67
    mov cl,1
.loopen
    mov dl,[ebx]
    rol dl,cl
    xor dl,dh
    mov [eax],dl
    rol dh,1
    inc cl
    or cl,cl
    jne .notzero
    inc cl
.notzero
    and cl,07h
    inc eax
    inc ebx
    dec ch
    jnz .loopen

    cmp byte[yesoutofmemory],1
    jne .noout
    call outofmemfix
.noout
    cmp byte[debugger],0
    je near start65816
    cmp byte[romloadskip],1
    je near start65816
    jmp startdebugger

EndMessageB

; global variables

SECTION .data

; Controls
;FIXME: define pl3contrl, pl4contrl and pl5contrl here?
;Christophe 2001/03/10
NEWSYM numjoy,    db 0   ; number of joysticks (1 = 1, 2 = 2)
; 0 = Disable, 1 = Keyboard, 2 = Joystick, 3 = Gamepad
; 4 = 4-button 5 = 6-button 6 = sidewinder
NEWSYM pl1contrl, db 1   ; player 1 device
NEWSYM pl1keya,   dd 0
NEWSYM pl1keyb,   dd 0
%ifdef __MSDOS__
NEWSYM pl1selk,   dd 54
NEWSYM pl1startk, dd 28
NEWSYM pl1upk,    dd 72
NEWSYM pl1downk,  dd 80
NEWSYM pl1leftk,  dd 75
NEWSYM pl1rightk, dd 77
%else
NEWSYM pl1selk,   dd 54
NEWSYM pl1startk, dd 28
NEWSYM pl1upk,    dd 200
NEWSYM pl1downk,  dd 208
NEWSYM pl1leftk,  dd 203
NEWSYM pl1rightk, dd 205
%endif
NEWSYM pl1Xk,     dd 31
NEWSYM pl1Ak,     dd 45
NEWSYM pl1Lk,     dd 32
NEWSYM pl1Yk,     dd 30
NEWSYM pl1Bk,     dd 44
NEWSYM pl1Rk,     dd 46
NEWSYM pl2contrl, db 0   ; player 2 device
NEWSYM pl2keya,   dd 0
NEWSYM pl2keyb,   dd 0
NEWSYM pl2selk,   dd 27
NEWSYM pl2startk, dd 26
NEWSYM pl2upk,    dd 199
NEWSYM pl2downk,  dd 207
NEWSYM pl2leftk,  dd 211
NEWSYM pl2rightk, dd 209
NEWSYM pl2Xk,     dd 24
NEWSYM pl2Ak,     dd 25
NEWSYM pl2Lk,     dd 23
NEWSYM pl2Yk,     dd 37
NEWSYM pl2Bk,     dd 38
NEWSYM pl2Rk,     dd 36
NEWSYM JoyStart,  db 0
NEWSYM JoySelec,  db 0
NEWSYM JoyBC,     db 1
NEWSYM JoyYC,     db 3
NEWSYM JoyAC,     db 2
NEWSYM JoyXC,     db 4
NEWSYM JoyLC,     db 5
NEWSYM JoyRC,     db 6
NEWSYM TurboSw,   db 0

NEWSYM romtype, db 0    ; ROM type in bytes
NEWSYM resetv,  dw 0    ; reset vector
NEWSYM abortv,  dw 0    ; abort vector
NEWSYM nmiv2,   dw 0    ; nmi vector
NEWSYM nmiv,    dw 0    ; nmi vector
NEWSYM irqv,    dw 0    ; irq vector
NEWSYM irqv2,   dw 0    ; irq vector
NEWSYM brkv,    dw 0    ; brk vector
NEWSYM copv,    dw 0    ; cop vector
NEWSYM abortv8, dw 0    ; abort vector emulation mode
NEWSYM nmiv8,   dw 0    ; nmi vector emulation mode
NEWSYM irqv8,   dw 0    ; irq vector emulation mode
NEWSYM brkv8,   dw 0    ; brk vector emulation mode
NEWSYM copv8,   dw 0    ; cop vector emulation mode
NEWSYM cycpb268, db 109  ; 110
NEWSYM cycpb358, db 149  ; 155
NEWSYM cycpbl2,  db 109  ; percentage left of CPU/SPC to run  (3.58 = 175)
NEWSYM cycpblt2, db 149  ; percentage of CPU/SPC to run
NEWSYM writeon, db 0    ; Write enable/disable on snes rom memory
NEWSYM totlines, dw 263 ; total # of lines
NEWSYM soundon, db 0    ; Current sound enabled (1=enabled)

;This is saved in states
NEWSYM curcyc,  db 0    ; cycles left in scanline
NEWSYM curypos, dw 0    ; current y position
NEWSYM cacheud, db 1    ; update cache every ? frames
NEWSYM ccud,    db 0    ; current cache increment
NEWSYM intrset, db 0    ; interrupt set
NEWSYM cycpl,   db 0    ; cycles per scanline
NEWSYM cycphb,  db 0    ; cycles per hblank
NEWSYM spcon,   db 0    ; SPC Enable (1=enabled)
NEWSYM stackand,dw 01FFh; value to and stack to keep it from going to the wrong area
NEWSYM stackor, dw 0100h; value to or stack to keep it from going to the wrong area

; 65816 registers

NEWSYM xat,      dw 0
NEWSYM xdbt,     db 0
NEWSYM xpbt,     db 0
NEWSYM xst,      dw 0
NEWSYM xdt,      dw 0
NEWSYM xxt,      dw 0
NEWSYM xyt,      dw 0
NEWSYM xp,       db 0
NEWSYM xe,       db 0
NEWSYM xpc,      dw 0
NEWSYM xirqb,    db 0           ; which bank the irqs start at
NEWSYM debugger, db 0              ; Start with debugger (1=yes,0=no)
NEWSYM Curtableaddr,  dd 0                 ; Current table address
NEWSYM curnmi,   db 0           ; if in NMI(1) or not(0)
; pharos - equ hack *sigh*
n65816regsize equ $-curcyc

ALIGN32
NEWSYM cycpbl,  dd 110  ; percentage left of CPU/SPC to run  (3.58 = 175)
NEWSYM cycpblt, dd 110  ; percentage of CPU/SPC to run

NEWSYM PH65816regsize, dd n65816regsize

; SNES memory map ROM locations

NEWSYM cpuoverptr, dd 0                 ; pointer to cpuover
;snesmmap times 256 dd 0         ; addresses 8000-FFFF
;snesmap2 times 256 dd 0         ; addresses 0000-7FFF
;NEWSYM exeloopa, times 128 db 0         ; execloop should be stored here
;NEWSYM exeloopb, times 128 db 0         ; execloopns should be stored here
;NEWSYM exeloopc, times 128 db 0         ; execloops should be stored here
;NEWSYM exeloopd, times 128 db 0         ; execloopn should be stored here
;NEWSYM prevcrash, times 250 db 0


ALIGN32
NEWSYM xa,       dd 0
NEWSYM xdb,      dd 0
NEWSYM xpb,      dd 0
NEWSYM xs,       dd 0
NEWSYM xd,       dd 0
NEWSYM xx,       dd 0
NEWSYM xy,       dd 0
NEWSYM flagnz,   dd 0
NEWSYM flago,    dd 0
NEWSYM flagc,    dd 0
NEWSYM bankkp,   dd 0
NEWSYM Sflagnz,  dd 0
NEWSYM Sflago,   dd 0
NEWSYM Sflagc,   dd 0

ebm db 166,95,66,223,17,11,103,180,156,68,108,120,138,55,203,205
    db 178,210,39,252,128,66,65,167,155,151,197,125,176,66,73,230
    db 61,210,7,137,152,110,203,241,50,89,70,29,176,42,99,167
    db 155,92,3,221,224,54,53,167,155,211,70,205,138,202,91,234
    db 178,80,229,13,10

SECTION .text


;*******************************************************
; Read Input Device            Reads from Keyboard, etc.
;*******************************************************

SECTION .bss
NEWSYM WhichSW, resb 1
NEWSYM WhichGR, resb 1
NEWSYM autofr,  resb 1
TurboCB resb 1

NEWSYM JoyAOrig, resd 1
NEWSYM JoyBOrig, resd 1
NEWSYM JoyCOrig, resd 1
NEWSYM JoyDOrig, resd 1
NEWSYM JoyEOrig, resd 1
NEWSYM JoyANow, resd 1
NEWSYM JoyBNow, resd 1
NEWSYM JoyCNow, resd 1
NEWSYM JoyDNow, resd 1
NEWSYM JoyENow, resd 1
NEWSYM PJoyAOrig, resd 1
NEWSYM PJoyBOrig, resd 1
NEWSYM PJoyCOrig, resd 1
NEWSYM PJoyDOrig, resd 1
NEWSYM PJoyEOrig, resd 1
NEWSYM LethEnData, resd 1
NEWSYM ComboCounter, resb 1
SECTION .text

%macro PlayerDeviceHelp 3
    mov eax,[%1]
    cmp byte[pressed+eax],1
    jne %%no
    or dword[%2],%3
%%no
%endmacro

%macro ProcSNESMouse 1
    test byte[mousebuttons],02h
    jz %%n
    or dword[%1],00000000100000000000000000000000b
    mov al,1
%%n
    test byte[mousebuttons],01h
    jz %%n2
    or dword[%1],00000000010000000000000000000000b
%%n2
    or dword[%1],00000000000000010000000000000000b
    mov al,[mouseypos]
    and al,7Fh
    or byte[%1+1],al
    mov al,[mousexpos]
    and al,7Fh
    or byte[%1],al
    test byte[mouseydir],01h
    jz %%n3
    or dword[%1],00000000000000001000000000000000b
%%n3
    test byte[mousexdir],01h
    jz %%n4
    or dword[%1],00000000000000000000000010000000b
%%n4
%endmacro

%macro ProcessKeyComb 2
    cmp dword[NumCombo],0
    je near %%nocomb
    mov eax,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je %%local
    mov eax,CombinDataLocl
%%local
    push ecx
    push ebx
    xor ebx,ebx
    cmp byte[ComboProg+%1],0
    jne near %%progressing
    test dword[%2],01000000h
    jz %%noright
    mov dword[CombDirSwap],0
%%noright
    test dword[%2],02000000h
    jz %%noleft
    mov dword[CombDirSwap],1
%%noleft
    mov ecx,[NumCombo]
%%loop
    mov bx,[eax+62]
    cmp byte[pressed+ebx],1
    jne %%nopress
    cmp byte[eax+64],%1
    je %%startprogress
%%nopress
    add eax,66
    dec ecx
    jnz %%loop
    jmp %%endcomb
%%startprogress
    mov byte[pressed+ebx],2
    inc byte[ComboCounter]
    mov byte[ComboProg+%1],1
    mov byte[ComboPtr+%1],0
    mov dword[PressComb+%1*4],0
    mov dword[HoldComb+%1*4],0
    mov dword[CombCont+%1*4],CombContDatN
    cmp dword[CombDirSwap],0
    je %%NoSwapLeftRight
    cmp byte[eax+65],0
    je %%NoSwapLeftRight
    mov dword[CombCont+%1*4],CombContDatR
%%NoSwapLeftRight
    add eax,20
    mov [StartComb+%1*4],eax
%%progressing
    mov ecx,%1
    mov eax,[StartComb+%1*4]
    call ProcessCombo
    mov [StartComb+%1*4],eax
    or ebx,ebx
    jz %%proccomb
    mov byte[ComboProg+%1],0
%%proccomb
    mov dword[%2],0
    mov eax,[HoldComb+%1*4]
    or dword[%2],eax
    mov eax,[PressComb+%1*4]
    or dword[%2],eax
%%endcomb
    pop ebx
    pop ecx
%%nocomb
%endmacro

ProcessCombo:
    mov dword[KeyLPress],0
    cmp dword[CombDelay+ecx*4],0
    jne .delay
.nextsession
    xor ebx,ebx
    cmp byte[eax],0
    je .finish
    mov bl,[eax]
    cmp bl,37
    jb .keypress
    cmp bl,48
    ja .finish
    sub ebx,37
    shl ebx,2
    add ebx,CombTDelN
    cmp byte[romispal],0
    je .ntsc
    sub ebx,CombTDelN
    add ebx,CombTDelP
.ntsc
    mov ebx,[ebx]
    mov [CombDelay+ecx*4],ebx
    inc eax
    inc byte[ComboPtr+ecx]
    cmp byte[ComboPtr+ecx],42
    je .finish
.delay
    dec dword[CombDelay+ecx*4]
    xor ebx,ebx
    ret
.finish
    mov ebx,1
    ret
.keypress
    cmp dword[KeyLPress],0
    jne .alreadyproc
    mov dword[PressComb+ecx*4],0
    mov dword[KeyLPress],1
.alreadyproc
    dec ebx
    cmp ebx,12
    jae .pressonly
    shl ebx,2
    add ebx,[CombCont+ecx*4]
    mov ebx,[ebx]
    or [PressComb+ecx*4],ebx
    jmp .finkeyproc
.pressonly
    sub ebx,12
    cmp ebx,12
    jae .releaseonly
    shl ebx,2
    add ebx,[CombCont+ecx*4]
    mov ebx,[ebx]
    or [HoldComb+ecx*4],ebx
    jmp .finkeyproc
.releaseonly
    sub ebx,12                ; <- bugfix from Maxim
    shl ebx,2
    add ebx,[CombCont+ecx*4]
    mov ebx,[ebx]
    xor ebx,0FFFFFFFFh
    and [HoldComb+ecx*4],ebx
    and [PressComb+ecx*4],ebx  ; <- buxfix from Maxim
.finkeyproc
    inc eax
    inc byte[ComboPtr+ecx]
    cmp byte[ComboPtr+ecx],42
    je near .finish
    jmp .nextsession

SECTION .data
ComboProg times 5 db 0
ComboPtr  times 5 db 0
KeyLPress dd 0
CombDirSwap dd 0
CombDelay times 5 dd 0
StartComb times 5 dd 0
HoldComb times 5 dd 0
PressComb times 5 dd 0
CombCont times 5 dd 0
CombTDelN dd 1,2,3,4,5,9,30,60,120,180,240,300
CombTDelP dd 1,2,3,4,5,9,25,50,100,150,200,250
CombContDatN dd 08000000h,04000000h,02000000h,01000000h,00800000h,80000000h
             dd 00400000h,40000000h,00200000h,00100000h,10000000h,20000000h
CombContDatR dd 08000000h,04000000h,01000000h,02000000h,00800000h,80000000h
             dd 00400000h,40000000h,00200000h,00100000h,10000000h,20000000h

EXTSYM MouseToRead,multiMouseMode

SECTION .text

%macro PlayerDeviceFix 1
   cmp byte[AllowUDLR],1
   je %%noleftright
   mov eax,[%1]
   and eax,0C000000h
   cmp eax,0C000000h
   jne %%noupdown
   and dword[%1],0F3FFFFFFh
%%noupdown
   mov eax,[%1]
   and eax,03000000h
   cmp eax,03000000h
   jne %%noleftright
   and dword[%1],0FCFFFFFFh
%%noleftright
%endmacro

NEWSYM ReadInputDevice
    mov byte[WhichSW],1
    mov byte[WhichGR],0
    inc byte[TurboSw]
    mov byte[TurboCB],01h
    cmp byte[Turbo30hz],0
    je .noturbo30
    mov byte[TurboCB],02h
.noturbo30
    ; Read External Devices (Joystick, PPort, etc.)
    call JoyRead
    ; Process Data
    mov dword[JoyAOrig],0
    mov dword[JoyBOrig],0

    cmp byte[snesmouse],3
    jne .nomultimouse
    mov byte[MouseToRead],1
    call processmouse
    ProcSNESMouse JoyAOrig
    mov byte[MouseToRead],2
    call processmouse
    ProcSNESMouse JoyBOrig
    jmp .noinput2
.nomultimouse

    ; Get Player1 input device
    cmp byte[snesmouse],1
    jne .nomouse1
    call processmouse
    ProcSNESMouse JoyAOrig
    jmp .noinput1
.nomouse1
    PlayerDeviceHelp pl1Bk    ,JoyAOrig,80000000h
    PlayerDeviceHelp pl1Yk    ,JoyAOrig,40000000h
    PlayerDeviceHelp pl1selk  ,JoyAOrig,20000000h
    PlayerDeviceHelp pl1startk,JoyAOrig,10000000h
    PlayerDeviceHelp pl1upk   ,JoyAOrig,08000000h
    PlayerDeviceHelp pl1downk ,JoyAOrig,04000000h
    PlayerDeviceHelp pl1leftk ,JoyAOrig,02000000h
    PlayerDeviceHelp pl1rightk,JoyAOrig,01000000h
    PlayerDeviceHelp pl1Ak    ,JoyAOrig,00800000h
    PlayerDeviceHelp pl1Xk    ,JoyAOrig,00400000h
    PlayerDeviceHelp pl1Lk    ,JoyAOrig,00200000h
    PlayerDeviceHelp pl1Rk    ,JoyAOrig,00100000h
    PlayerDeviceHelp pl1ULk   ,JoyAOrig,0A000000h
    PlayerDeviceHelp pl1URk   ,JoyAOrig,09000000h
    PlayerDeviceHelp pl1DLk   ,JoyAOrig,06000000h
    PlayerDeviceHelp pl1DRk   ,JoyAOrig,05000000h
    PlayerDeviceFix JoyAOrig
    mov al,[TurboCB]
    test byte[TurboSw],al
    jnz near .noswitch
    PlayerDeviceHelp pl1Xtk   ,JoyAOrig,00400000h
    PlayerDeviceHelp pl1Ytk   ,JoyAOrig,40000000h
    PlayerDeviceHelp pl1Atk   ,JoyAOrig,00800000h
    PlayerDeviceHelp pl1Btk   ,JoyAOrig,80000000h
    PlayerDeviceHelp pl1Ltk   ,JoyAOrig,00200000h
    PlayerDeviceHelp pl1Rtk   ,JoyAOrig,00100000h
.noswitch
    mov byte[ComboCounter],0
    ProcessKeyComb 0,JoyAOrig
    or dword[JoyAOrig],00008000h        ; Joystick Enable
    cmp byte[GUIDelayB],0
    je .noinput1
    cmp byte[GUIDelayB],1
    jne .inputbd
    test dword[JoyAOrig],80000000h
    jnz .inputbdcb
    dec byte[GUIDelayB]
    jmp .noinput1
.inputbd
    dec byte[GUIDelayB]
.inputbdcb
    and dword[JoyAOrig],7FFFFFFFh
.noinput1
    mov dword[JoyBOrig],0
    cmp byte[snesmouse],2
    jne .nomouse2
    call processmouse
    ProcSNESMouse JoyBOrig
    jmp .noinput2
.nomouse2
    cmp byte[snesmouse],4
    jne .nosuperscope
    call processmouse
    mov byte[JoyBOrig+2],0FFh
    mov al,[ssautosw]
    test byte[mousebuttons],01h
    jz .nobutton1
    or al,80h
.nobutton1
    cmp byte[pressed+14],0
    jz .nobutton3
    or al,10h
.nobutton3
    test byte[mousebuttons],02h
    jz .nobutton4
    or al,40h
.nobutton4
    mov [JoyBOrig+3],al
    jmp .noinput2
.nosuperscope
    cmp byte[snesmouse],5
    jne .nolethalen
    call processmouse
    mov eax,[romdata]
    cmp dword[eax+1000h],0AD20C203h
    jne .not
    mov eax,[wramdata]
    cmp byte[eax],26
    je .not
    mov bl,[mousexloc]
    mov [eax+40Ah],bl
    mov bl,[mouseyloc]
    mov [eax+40Eh],bl
.not
;    mov word[JoyBOrig+2],000Eh
;    and dword[LethEnData],0000000FFh
;    or dword[LethEnData],055010000h
;    inc byte[LethEnData]
    test byte[LethEnData],1
    jz .n1
;    mov byte[LethEnData+2],8
    mov byte[extlatch],0
.n1
    test byte[mousebuttons],01h
    jz .nobutton1b
;    or dword[LethEnData+2],10h
    or byte[JoyAOrig+3],80h
.nobutton1b
    cmp byte[pressed+14],0
    jz .nobutton3b
;    or byte[LethEnData+2],40h
.nobutton3b
    test byte[mousebuttons],02h
    jz .nobutton4b
;    or byte[LethEnData+2],40h
    or byte[JoyAOrig+2],80h
.nobutton4b
    jmp .noinput2
.nolethalen
    cmp byte[pl2contrl],0
    je near .noinput2
    ; Get Player2 input device
    PlayerDeviceHelp pl2Bk    ,JoyBOrig,80000000h
    PlayerDeviceHelp pl2Yk    ,JoyBOrig,40000000h
    PlayerDeviceHelp pl2selk  ,JoyBOrig,20000000h
    PlayerDeviceHelp pl2startk,JoyBOrig,10000000h
    PlayerDeviceHelp pl2upk   ,JoyBOrig,08000000h
    PlayerDeviceHelp pl2downk ,JoyBOrig,04000000h
    PlayerDeviceHelp pl2leftk ,JoyBOrig,02000000h
    PlayerDeviceHelp pl2rightk,JoyBOrig,01000000h
    PlayerDeviceHelp pl2Ak    ,JoyBOrig,00800000h
    PlayerDeviceHelp pl2Xk    ,JoyBOrig,00400000h
    PlayerDeviceHelp pl2Lk    ,JoyBOrig,00200000h
    PlayerDeviceHelp pl2Rk    ,JoyBOrig,00100000h
    PlayerDeviceHelp pl2ULk   ,JoyBOrig,0A000000h
    PlayerDeviceHelp pl2URk   ,JoyBOrig,09000000h
    PlayerDeviceHelp pl2DLk   ,JoyBOrig,06000000h
    PlayerDeviceHelp pl2DRk   ,JoyBOrig,05000000h
    PlayerDeviceFix JoyBOrig
    mov al,[TurboCB]
    test byte[TurboSw],al
    jnz near .noswitch2
    PlayerDeviceHelp pl2Xtk   ,JoyBOrig,00400000h
    PlayerDeviceHelp pl2Ytk   ,JoyBOrig,40000000h
    PlayerDeviceHelp pl2Atk   ,JoyBOrig,00800000h
    PlayerDeviceHelp pl2Btk   ,JoyBOrig,80000000h
    PlayerDeviceHelp pl2Ltk   ,JoyBOrig,00200000h
    PlayerDeviceHelp pl2Rtk   ,JoyBOrig,00100000h
.noswitch2
    ProcessKeyComb 1,JoyBOrig
    or dword[JoyBOrig],00008000h        ; Joystick Enable
.noinput2
    mov dword[JoyCOrig],0
    cmp byte[pl3contrl],0
    je near .noinput3
    ; Get Player3 input device
    PlayerDeviceHelp pl3Bk    ,JoyCOrig,80000000h
    PlayerDeviceHelp pl3Yk    ,JoyCOrig,40000000h
    PlayerDeviceHelp pl3selk  ,JoyCOrig,20000000h
    PlayerDeviceHelp pl3startk,JoyCOrig,10000000h
    PlayerDeviceHelp pl3upk   ,JoyCOrig,08000000h
    PlayerDeviceHelp pl3downk ,JoyCOrig,04000000h
    PlayerDeviceHelp pl3leftk ,JoyCOrig,02000000h
    PlayerDeviceHelp pl3rightk,JoyCOrig,01000000h
    PlayerDeviceHelp pl3Ak    ,JoyCOrig,00800000h
    PlayerDeviceHelp pl3Xk    ,JoyCOrig,00400000h
    PlayerDeviceHelp pl3Lk    ,JoyCOrig,00200000h
    PlayerDeviceHelp pl3Rk    ,JoyCOrig,00100000h
    PlayerDeviceHelp pl3ULk   ,JoyCOrig,0A000000h
    PlayerDeviceHelp pl3URk   ,JoyCOrig,09000000h
    PlayerDeviceHelp pl3DLk   ,JoyCOrig,06000000h
    PlayerDeviceHelp pl3DRk   ,JoyCOrig,05000000h
    PlayerDeviceFix JoyCOrig
    mov al,[TurboCB]
    test byte[TurboSw],al
    jnz near .noswitch3
    PlayerDeviceHelp pl3Xtk   ,JoyCOrig,00400000h
    PlayerDeviceHelp pl3Ytk   ,JoyCOrig,40000000h
    PlayerDeviceHelp pl3Atk   ,JoyCOrig,00800000h
    PlayerDeviceHelp pl3Btk   ,JoyCOrig,80000000h
    PlayerDeviceHelp pl3Ltk   ,JoyCOrig,00200000h
    PlayerDeviceHelp pl3Rtk   ,JoyCOrig,00100000h
.noswitch3
    ProcessKeyComb 2,JoyCOrig
    or dword[JoyCOrig],00008000h        ; Joystick Enable
.noinput3
    mov dword[JoyDOrig],0
    cmp byte[pl4contrl],0
    je near .noinput4
    ; Get Player4 input device
    PlayerDeviceHelp pl4Bk    ,JoyDOrig,80000000h
    PlayerDeviceHelp pl4Yk    ,JoyDOrig,40000000h
    PlayerDeviceHelp pl4selk  ,JoyDOrig,20000000h
    PlayerDeviceHelp pl4startk,JoyDOrig,10000000h
    PlayerDeviceHelp pl4upk   ,JoyDOrig,08000000h
    PlayerDeviceHelp pl4downk ,JoyDOrig,04000000h
    PlayerDeviceHelp pl4leftk ,JoyDOrig,02000000h
    PlayerDeviceHelp pl4rightk,JoyDOrig,01000000h
    PlayerDeviceHelp pl4Ak    ,JoyDOrig,00800000h
    PlayerDeviceHelp pl4Xk    ,JoyDOrig,00400000h
    PlayerDeviceHelp pl4Lk    ,JoyDOrig,00200000h
    PlayerDeviceHelp pl4Rk    ,JoyDOrig,00100000h
    PlayerDeviceHelp pl4ULk   ,JoyDOrig,0A000000h
    PlayerDeviceHelp pl4URk   ,JoyDOrig,09000000h
    PlayerDeviceHelp pl4DLk   ,JoyDOrig,06000000h
    PlayerDeviceHelp pl4DRk   ,JoyDOrig,05000000h
    PlayerDeviceFix JoyDOrig
    mov al,[TurboCB]
    test byte[TurboSw],al
    jnz near .noswitch4
    PlayerDeviceHelp pl4Xtk   ,JoyDOrig,00400000h
    PlayerDeviceHelp pl4Ytk   ,JoyDOrig,40000000h
    PlayerDeviceHelp pl4Atk   ,JoyDOrig,00800000h
    PlayerDeviceHelp pl4Btk   ,JoyDOrig,80000000h
    PlayerDeviceHelp pl4Ltk   ,JoyDOrig,00200000h
    PlayerDeviceHelp pl4Rtk   ,JoyDOrig,00100000h
.noswitch4
    ProcessKeyComb 3,JoyDOrig
    or dword[JoyDOrig],00008000h        ; Joystick Enable
.noinput4
    mov dword[JoyEOrig],0
    cmp byte[pl5contrl],0
    je near .noinput5
    ; Get Player4 input device
    PlayerDeviceHelp pl5Bk    ,JoyEOrig,80000000h
    PlayerDeviceHelp pl5Yk    ,JoyEOrig,40000000h
    PlayerDeviceHelp pl5selk  ,JoyEOrig,20000000h
    PlayerDeviceHelp pl5startk,JoyEOrig,10000000h
    PlayerDeviceHelp pl5upk   ,JoyEOrig,08000000h
    PlayerDeviceHelp pl5downk ,JoyEOrig,04000000h
    PlayerDeviceHelp pl5leftk ,JoyEOrig,02000000h
    PlayerDeviceHelp pl5rightk,JoyEOrig,01000000h
    PlayerDeviceHelp pl5Ak    ,JoyEOrig,00800000h
    PlayerDeviceHelp pl5Xk    ,JoyEOrig,00400000h
    PlayerDeviceHelp pl5Lk    ,JoyEOrig,00200000h
    PlayerDeviceHelp pl5Rk    ,JoyEOrig,00100000h
    PlayerDeviceHelp pl5ULk   ,JoyEOrig,0A000000h
    PlayerDeviceHelp pl5URk   ,JoyEOrig,09000000h
    PlayerDeviceHelp pl5DLk   ,JoyEOrig,06000000h
    PlayerDeviceHelp pl5DRk   ,JoyEOrig,05000000h
    PlayerDeviceFix JoyEOrig
    mov al,[TurboCB]
    test byte[TurboSw],al
    jnz near .noswitch5
    PlayerDeviceHelp pl5Xtk   ,JoyEOrig,00400000h
    PlayerDeviceHelp pl5Ytk   ,JoyEOrig,40000000h
    PlayerDeviceHelp pl5Atk   ,JoyEOrig,00800000h
    PlayerDeviceHelp pl5Btk   ,JoyEOrig,80000000h
    PlayerDeviceHelp pl5Ltk   ,JoyEOrig,00200000h
    PlayerDeviceHelp pl5Rtk   ,JoyEOrig,00100000h
.noswitch5
    ProcessKeyComb 4,JoyEOrig
    or dword[JoyEOrig],00008000h        ; Joystick Enable
.noinput5
    cmp byte[pl12s34],1
    je .pl1234
.nopl1234
    ret
.pl1234
    cmp byte[snesmouse],5
    je .nopl1234
    cmp byte[snesmouse],1
    je .nopl13
    mov eax,[JoyCOrig]
    or [JoyAOrig],eax
.nopl13
    cmp byte[snesmouse],2
    je .nopl24
    cmp byte[snesmouse],4
    je .nopl24
    mov eax,[JoyDOrig]
    or [JoyBOrig],eax
.nopl24
    ret

;*******************************************************
; Init 65816                   Initializes the Registers
;*******************************************************

SECTION .data
NEWSYM disablehdma,    db 0
NEWSYM disableeffects, db 0
NEWSYM hdmaearlstart,  db 0
NEWSYM disable65816sh, db 0
NEWSYM disablespcclr,  db 0
NEWSYM virqnodisable,  db 0
NEWSYM numspcvblleft,  dd 0
NEWSYM spc700idle,     dd 0
NEWSYM IRQHack,        dw 0
NEWSYM CacheCheckSkip,     db 0
NEWSYM HIRQSkip,     db 0
NEWSYM ClearScreenSkip, db 0
NEWSYM ENVDisable, db 0
SECTION .text

NEWSYM idledetectspc
    inc dword[numspcvblleft]
    cmp byte[SPCStallSetting],2
    jae .fullstall
    inc byte[SPCStallSetting]
    mov byte[ReturnFromSPCStall],1
    ret
.fullstall
    mov dword[spc700idle],29
    mov dword[Msgptr],.unableskip
    mov eax,[MsgCount]
    mov [MessageOn],eax
    ret

SECTION .data
.unableskip db 'SPC700 STALL DETECTED.',0

SECTION .bss
NEWSYM ReturnFromSPCStall, resb 1
NEWSYM SPCStallSetting, resb 1
NEWSYM SPCSkipXtraROM, resb 1
NEWSYM WindowDisables, resd 1
SECTION .text

%macro helpclearmem 2
    mov edi,%1
    mov ecx,%2
    rep stosb
%endmacro

NEWSYM init65816
    mov byte[osm2dis],0
    mov byte[bgfixer2],0
    cmp byte[SA1Enable],0
    je .nosa1init
    call SA1Reset
    pushad
    call SetAddressingModesSA1
    popad
.nosa1init
    cmp byte[OBCEnable],0
    je .noobcinit
    call InitOBC
.noobcinit
    cmp byte[C4Enable],0
    je .noc4init
    mov byte[osm2dis],1
    mov byte[bgfixer2],1
    call InitC4
.noc4init
    cmp byte[RTCEnable],0
    je .noRTCinit
    call RTCinit
.noRTCinit
    cmp byte[SPC7110Enable],0
    je .nospc7110init
    call SPC7110init
    mov dword[memtabler8+50h*4],memaccessspc7110r8
    mov dword[memtabler16+50h*4],memaccessspc7110r16
    mov dword[memtablew8+50h*4],memaccessspc7110w8
    mov dword[memtablew16+50h*4],memaccessspc7110w16
    mov eax,[romdata]
    add eax,510000h
    mov [snesmmap+50h*4],eax
    mov [snesmap2+50h*4],eax
    mov ecx,16384
.spc7110clear
    mov dword[eax],0
    add eax,4
    dec ecx
    jnz .spc7110clear
.nospc7110init
    mov byte[cycpb268],117
    mov byte[cycpb358],127
    mov byte[cycpbl2],117
    mov byte[cycpblt2],117
    mov byte[cycpbl],117
    mov byte[cycpblt],117

    mov byte[SPCSkipXtraROM],0
    cmp byte[ReturnFromSPCStall],1
    jne near .nostall
    mov byte[cycpb268],69
    mov byte[cycpb358],81
    mov byte[cycpbl2],69
    mov byte[cycpblt2],69
    mov byte[cycpbl],69
    mov byte[cycpblt],69
    mov byte[SPCSkipXtraROM],1
    cmp byte[SPCStallSetting],2
    jne .nostall2
    mov byte[cycpb268],240
    mov byte[cycpb358],240
    mov byte[cycpbl],240
    mov byte[cycpblt],240
    mov byte[cycpbl2],240
    mov byte[cycpblt2],240
    mov byte[SPCSkipXtraROM],0
.nostall2
    jmp .stalled
.nostall
    mov byte[SPCStallSetting],0
.stalled
    mov dword[numspcvblleft],60*8
    mov dword[SPC700write],0
    mov dword[SPC700read],0
    cmp dword[spc700read],0
    mov dword[spc700idle],0
    xor esi,esi
.loopa
    mov al,[SPCROM+esi]
    mov byte[spcextraram+esi],0FFh
    mov [SPCRAM+0FFC0h+esi],al
    inc esi
    cmp esi,040h
    jne .loopa

    ; Clear SPC Memory
    pushad
    call clearSPCRAM
    call clearvidsound
    popad

    mov byte[prevoamptr],0FFh
    mov byte[disablehdma],0
    mov byte[disableeffects],0
    mov al,[opexec268b]
    mov [opexec268],al
    mov al,[opexec358b]
    mov [opexec358],al
    mov al,[opexec268cphb]
    mov [opexec268cph],al
    mov al,[opexec358cphb]
    mov [opexec358cph],al

    mov dword[FIRTAPVal0],7Fh
    mov dword[FIRTAPVal1],0
    mov dword[FIRTAPVal2],0
    mov dword[FIRTAPVal3],0
    mov dword[FIRTAPVal4],0
    mov dword[FIRTAPVal5],0
    mov dword[FIRTAPVal6],0
    mov dword[FIRTAPVal7],0
    mov byte[disable65816sh],0

    ; Check Headers
    pushad
    call headerhack
    popad

    mov byte[SPCRAM+0F4h],0
    mov byte[SPCRAM+0F5h],0
    mov byte[SPCRAM+0F6h],0
    mov byte[SPCRAM+0F7h],0
    mov byte[reg1read],0
    mov byte[reg2read],0
    mov byte[reg3read],0
    mov byte[reg4read],0
    mov dword[cycpbl],0
    mov byte[spcnumread],0
    mov dword[coladdr],0
    mov byte[NMIEnab],1
    mov word[VIRQLoc],0
    mov byte[doirqnext],0
    mov dword[reg1read],0
    mov word[resolutn],224
    mov byte[vidbright],0
    mov byte[forceblnk],0
    mov byte[spcP],0
    mov byte[timeron],0
    mov byte[JoyAPos],0
    mov byte[JoyBPos],0
    mov byte[coladdr],0
    mov byte[coladdg],0
    mov byte[coladdb],0

    mov byte[INTEnab],0
    mov word[xa],0
    mov byte[xdb],0
    mov byte[xpb],0
    mov byte[xirqb],0
    mov word[xs],01FFh
    mov word[xd],0
    mov word[xx],0
    mov word[xy],0
    mov dword[SDD1BankA],03020100h
    mov byte[xp],00110100b  ; NVMXDIZC

    push ebx
    mov byte[xe],1          ; E
    xor eax,eax
    mov ax,[resetv]
    mov [xpc],ax
    mov ebx,[romdata]
    add eax,ebx
    pop ebx
    mov byte[intrset],0
    cmp byte[romtype],1
    je .nohirom
    mov byte[xpb],00h
    mov byte[xirqb],00h
.nohirom
    cmp word[xpc],8000h
    jae .n
    add word[xpc],8000h
;    mov byte[xpb],40h
.n
    mov al,[opexec268]
    mov [cycpl],al      ; 2.68 Mhz  / 3.58 Mhz = 228
    mov [curcyc],al
    mov al,[opexec268cph]
    mov [cycphb],al     ; 2.68 Mhz  / 3.58 Mhz = 56
    mov byte[cycpbl],110        ; 3.58Mhz = 175
    mov byte[cycpblt],110
    mov word[curypos],0
    mov eax,tableD
    mov [Curtableaddr],eax
    mov byte[scrndis],00h
    mov word[stackand],01FFh
    mov word[stackor],0100h

    mov dword[nmiprevaddrl],0
    mov dword[nmiprevaddrh],0
    mov byte[nmirept],0
    mov byte[nmiprevline],224
    mov byte[nmistatus],0

    cmp byte[GUIReset],1
    jne .notreseting
    mov byte[GUIReset],0
    jmp .afterramclear
.notreseting
    mov eax,055555555h
    helpclearmem wramdataa, 65536
    helpclearmem ram7fa, 65536
.afterramclear
    cmp byte[BSEnable],1
    jne .notbsx2
    mov eax,0FFFFFFFFh
    helpclearmem wramdataa, 65536
    helpclearmem ram7fa, 65536
    cmp byte[romtype],1 ;Hack for BS HiROMs
    jne .notbsx2
    mov dword[ram7fa+65528],01010101h
    mov dword[ram7fa+65532],01010101h
.notbsx2
    mov dword[wramreadptr],getwram1fff
    mov dword[wramwriteptr],setwram1fff
    ret

SECTION .data
.boffound db '.....',0
SECTION .text

getwram1fff:
    mov al,[wramdataa+1fffh]
    ret
setwram1fff:
    mov [wramdata+1fffh],al
    ret

;*******************************************************
; Init SNES                      Sets the pointers, etc.
;*******************************************************
; Set banks according to :
;   Banks 00-3F,80-BF : WRAM (0000h-7FFFh), ROM Data (8000h-FFFFh)
;   Banks 40-7F,C0-FF : ROM Data (0000h-FFFFh)
;   Bank  70-77       : SRAM (0000h-7FFFh)
;   Bank  7E          : WRAM (0000h-FFFFh)
;   Bank  7F          : ExtendRAM (0000h-FFFFh)

SECTION .bss
NEWSYM curromsize, resb 1
NEWSYM cromptradd, resd 1
NEWSYM lorommapmode2, resb 1
SECTION .text

NEWSYM initsnes
    mov byte[ForceNewGfxOff],0

    mov esi,[romdata]
    add esi,7FC0h
    cmp byte[BSEnable],0
    je .notbsx3
    cmp byte[romtype],1 ;Hack for BS HiROMs
    je near .bslorom
.notbsx3
    mov esi,[romdata]
    add esi,[infoloc]
    add esi,22
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

    cmp byte[romtype],1
    jne .nosfx
    cmp byte[SFXEnable],1
    je near .sfx
.nosfx
    cmp byte[SA1Enable],1
    je near SA1memmap
    cmp byte[SDD1Enable],1
    je near SDD1memmap

    cmp byte[SPC7110Enable],1
    je near .hirom
    ;Should catch DKJM2 here, but need to fix mem map as well
    cmp byte[curromsize],13
    je near .lorom48
    cmp byte[romtype],1
    jne near .hirom

    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopa
    stosd
    add eax,8000h
    dec ecx
    jnz .loopa
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov ecx,40h
.loopb
    stosd
    add eax,8000h
    dec ecx
    jnz .loopb
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
    cmp byte[lorommapmode2],0
    je .notlorommode2
    add eax,200000h
.notlorommode2
    sub eax,8000h
    mov cx,20h
.loopc
    stosd
    add eax,8000h
    dec ecx
    jnz .loopc
    cmp byte[lorommapmode2],0
    je .notlorommode2b
    sub eax,200000h
.notlorommode2b
    mov cx,20h
.loopclr
    stosd
    add eax,8000h
    dec ecx
    jnz .loopclr
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov ecx,40h
.loopd
    stosd
    add eax,8000h
    dec ecx
    jnz .loopd
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa2
    stosd
    dec ecx
    jnz .loopa2
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,200000h
    mov ecx,40h
.loopb2
    stosd
    add eax,8000h
    dec ecx
    jnz .loopb2
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc2
    stosd
    dec ecx
    jnz .loopc2
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,200000h
    mov ecx,40h
.loopd2
    stosd
    add eax,8000h
    dec ecx
    jnz .loopd2
    ; set bank 70
    mov eax,[sram]
    xor ebx,ebx
    mov bl,70h
.nextsram
    mov [snesmap2+ebx*4],eax
    inc bl
    cmp bl,77h
    jbe .nextsram
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    ret

.bslorom
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopas
    stosd
    add eax,8000h
    dec ecx
    jnz .loopas
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov ecx,40h
.loopbs
    stosd
    add eax,8000h
    dec ecx
    jnz .loopbs
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
    sub eax,8000h
    mov cx,40h
.loopcs
    stosd
    add eax,8000h
    dec ecx
    jnz .loopcs
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,8000h
    mov ecx,40h
.loopds
    stosd
    add eax,8000h
    dec ecx
    jnz .loopds
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa2s
    stosd
    dec ecx
    jnz .loopa2s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,200000h
    mov ecx,40h
.loopb2s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopb2s
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc2s
    stosd
    dec ecx
    jnz .loopc2s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,8000h
    mov ecx,40h
.loopd2s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopd2s
    ; set bank 70
    mov eax,[sram]
    xor ebx,ebx
    mov bl,70h
.nextsrams
    mov [snesmap2+ebx*4],eax
    inc bl
    cmp bl,77h
    jbe .nextsrams
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    ret

.lorom48
    mov byte[cycpb268],94
    mov byte[cycpb358],94
    mov byte[cycpbl2],94
    mov byte[cycpblt2],94
    mov byte[cycpbl],94
    mov byte[cycpblt],94
    mov byte[opexec268],183
    mov byte[opexec358],187
    mov byte[opexec268cph],30
    mov byte[opexec358cph],30

    mov edi,memtabler8+40h*4
    mov ecx,30h
    mov eax,memaccessbankr848mb
    rep stosd
    mov edi,memtabler16+40h*4
    mov ecx,30h
    mov eax,memaccessbankr1648mb
    rep stosd
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopa3
    stosd
    add eax,8000h
    dec ecx
    jnz .loopa3
    ; set banks 40-6F (30h x 64KB ROM banks @ 8000h)
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopb32
    stosd
    add eax,8000h
    dec ecx
    jnz .loopb32
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
    sub eax,8000h
    mov cx,40h
.loopc3
    stosd
    add eax,8000h
    dec ecx
    jnz .loopc3
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,200000h
    mov ecx,40h
.loopd3
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd3
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa23
    stosd
    dec ecx
    jnz .loopa23
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,100000h
    mov ecx,40h
.loopb3
    stosd
    add eax,8000h
    dec ecx
    jnz .loopb3
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc23
    stosd
    dec ecx
    jnz .loopc23
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,200000h
    mov ecx,40h
.loopd23
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd23
    ; set bank 70
    mov eax,[sram]
    xor ebx,ebx
    mov bl,70h
.nextsram3
    mov [snesmap2+ebx*4],eax
    inc bl
    cmp bl,77h
    jbe .nextsram3
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    pushad
    call IntlEHi ;Interleave 48Mb ROM, because the map is broken
    popad
    ret
.hirom
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    mov ecx,40h
.loopab
    stosd
    add eax,10000h
    dec ecx
    jnz .loopab
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    mov ecx,40h
.loopbb
    stosd
    add eax,10000h
    dec ecx
    jnz .loopbb
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
    mov ecx,40h
.loopcb
    stosd
    add eax,10000h
    dec ecx
    jnz .loopcb
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    mov ecx,40h
.loopdb
    stosd
    add eax,10000h
    dec ecx
    jnz .loopdb
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa2b
    stosd
    dec ecx
    jnz .loopa2b
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    mov cx,40h
.loopb2b
    stosd
    add eax,10000h
    dec ecx
    jnz .loopb2b
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov cx,40h
.loopc2b
    stosd
    dec ecx
    jnz .loopc2b
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    mov cx,40h
.loopd2b
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd2b
    ; set bank 70
    mov eax,[sram]
    xor ebx,ebx
    mov bl,70h
.nextsramb
    mov [snesmap2+ebx*4],eax
    inc bl
    cmp bl,77h
    jbe .nextsramb
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    ret

.sfx
    mov byte[MultiTap],0
    ; Clear sfxregisters
    mov edi,SfxR0
    mov ecx,16
    xor eax,eax
    rep stosd
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
;    sub eax,8000h
    mov ecx,40h
.loopa3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopa3s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
;    add eax,200000h
    add eax,8000h
    mov ecx,40h
.loopb3s
    stosd
    add eax,20000h
    dec ecx
    jnz .loopb3s
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
;    sub eax,8000h
    mov cx,40h
.loopc3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopc3s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
;    add eax,200000h
    add eax,8000h
    mov ecx,40h
.loopd3s
    stosd
    add eax,20000h
    dec ecx
    jnz .loopd3s
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa23s
    stosd
    dec ecx
    jnz .loopa23s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,8000h
    mov ecx,40h
.loopb23s
    stosd
    add eax,20000h
    dec ecx
    jnz .loopb23s
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc23s
    stosd
    dec ecx
    jnz .loopc23s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,8000h
    mov ecx,40h
.loopd23s
    stosd
    add eax,20000h
    dec ecx
    jnz .loopd23s
    ; set bank 70
    mov eax,[sram]
    mov [snesmap2+78h*4],eax
    mov [snesmap2+79h*4],eax
    mov eax,[sfxramdata]
    mov [snesmap2+70h*4],eax
    add eax,65536
    mov [snesmap2+71h*4],eax
    add eax,65536
    mov [snesmap2+72h*4],eax
    add eax,65536
    mov [snesmap2+73h*4],eax
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    call preparesfx
    ret

SA1memmap:
    mov eax,[romdata]
    cmp dword[eax+0B95h],0ADCF10A9h
    jne .nosuccess
    mov byte[eax+0B96h],0
.nosuccess
    mov byte[MultiTap],0
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopa3s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopa3s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
;    add eax,400000h
    mov ecx,40h
.loopb3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopb3s
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
    add eax,200000h
    sub eax,8000h
    mov cx,40h
.loopc3s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopc3s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,00000h
    mov ecx,40h
.loopd3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd3s
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa23s
    stosd
    dec ecx
    jnz .loopa23s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,400000h
    mov ecx,40h
.loopb23s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopb23s
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc23s
    stosd
    dec ecx
    jnz .loopc23s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,00000h
    mov ecx,40h
.loopd23s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd23s
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    ret

SDD1memmap:
    mov eax,[romdata]
    cmp dword[eax+0B95h],0ADCF10A9h
    jne .nosuccess
    mov byte[eax+0B96h],0
.nosuccess
    mov byte[MultiTap],0
    ; set addresses 8000-FFFF
    ; set banks 00-3F (40h x 32KB ROM banks @ 8000h)
    mov edi,snesmmap
    mov eax,[romdata]
    sub eax,8000h
    mov ecx,40h
.loopa3s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopa3s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
;    add eax,400000h
    mov ecx,40h
.loopb3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopb3s
    ; set banks 80-BF (40h x 32KB ROM banks @ 8000h)
    mov eax,[romdata]
;    add eax,200000h
    sub eax,8000h
    mov cx,40h
.loopc3s
    stosd
    add eax,8000h
    dec ecx
    jnz .loopc3s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,00000h
    mov ecx,40h
.loopd3s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd3s
    ; set addresses 0000-7FFF (01h x 32KB WRAM @ 0000h)
    ; set banks 00-3F
    mov edi,snesmap2
    mov eax,[wramdata]
    mov ecx,40h
.loopa23s
    stosd
    dec ecx
    jnz .loopa23s
    ; set banks 40-6F (30h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,400000h
    mov ecx,40h
.loopb23s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopb23s
    ; set banks 80-BF (01h x 32KB WRAM @ 0000h)
    mov eax,[wramdata]
    mov ecx,40h
.loopc23s
    stosd
    dec ecx
    jnz .loopc23s
    ; set banks C0-FF (40h x 64KB ROM banks @ 0000h)
    mov eax,[romdata]
    add eax,00000h
    mov ecx,40h
.loopd23s
    stosd
    add eax,10000h
    dec ecx
    jnz .loopd23s
    ; set bank 7E
    mov eax,[wramdata]
    mov [snesmmap+7Eh*4],eax
    mov [snesmap2+7Eh*4],eax
    ; set bank 7F
    mov eax,[ram7f]
    mov [snesmmap+7Fh*4],eax
    mov [snesmap2+7Fh*4],eax
    ret

SECTION .data
NEWSYM SFXCounter, dd 0

SECTION .text

NEWSYM preparesfx
    mov byte[SFXCounter],0
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'FX S'
    je .yessfxcounter
    cmp dword[esi],'DIRT'
    je .yessfxcounter
    cmp dword[esi],'Stun'
    jne .nosfxcounter
    mov byte[ForceNewGfxOff],1
.yessfxcounter
    mov byte[SFXCounter],1
.nosfxcounter

    ; make table
    mov byte[SfxAC],0
    mov eax,[romdata]
    cmp dword[eax+02B80h],0AB6CAB6Ch
    jne .noac
    mov byte[SfxAC],1
.noac
    ; duplicate sfx data
    mov esi,[romdata]
    mov edi,[romdata]
    add esi,1F8000h
    add edi,3F0000h
    mov dl,40h
.swaploopb
    mov ecx,32768
.swaploop
    mov al,[esi]
    mov [edi],al
    mov [edi+32768],al
    inc esi
    inc edi
    dec ecx
    jnz .swaploop
    sub edi,65536+32768
    sub esi,65536
    dec dl
    jnz .swaploopb
    ret

    ; copy  address 0 to 200000h
    ; make table 1 (0 .. 127)
    mov esi,[romdata]
    mov edi,[romdata]
    add edi,200000h
    mov ecx,200000h
.n
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jnz .n
    ret

;*******************************************************
; Print Hexadecimal (16-bit/8-bit)
;*******************************************************
NEWSYM printhex
    mov ecx,4
    xor ebx,ebx
.loopa
    mov bx,ax
    and bx,0F000h
    shr bx,12
    mov dl,[.hexdat+ebx]
    push ax
    mov ah,02h
    call Output_Text
    pop ax
    shl ax,4
    dec ecx
    jnz .loopa
    ret

SECTION .data
.hexdat db '0123456789ABCDEF'
SECTION .text

NEWSYM printhex8
    mov ecx,2
    xor ebx,ebx
.loopb
    mov bx,ax
    and bx,0F0h
    shr bx,4
    mov dl,[printhex.hexdat+ebx]
    push ax
    mov ah,02h
    call Output_Text
    pop ax
    shl ax,4
    dec ecx
    jnz .loopb
    ret

;*******************************************************
; Load File
;*******************************************************
; Search for header size first which is filesize MOD 32768

NEWSYM PatchIPS
%ifdef __UNIXSDL__
    pushad
    call pushdir
    popad
%endif
    mov eax,fname+1
    ; search for . or 0
.next
    cmp byte[eax],0
    je .foundend
    inc eax
    jmp .next
.foundend
    mov ebx,eax
.findnext2
    dec eax
    cmp eax,fname
    je .failfound
%ifdef __UNIXSDL__
    cmp byte[eax],'/'
%else
    cmp byte[eax],'\'
%endif
    je .failfound
    cmp byte[eax],'.'
    je .foundokay
    jmp .findnext2
.failfound
    mov eax,ebx
.foundokay
    mov ebx,[eax]
    mov [Prevextn],ebx
    mov dword[eax],'.ips'
    mov byte[eax+4],0
    push eax
%ifdef __UNIXSDL__
    cmp byte[ZipSupport], 1
    je .nochangedir
    mov ebx,LoadDir
    call Change_Dir
.nochangedir:
%endif
    mov edx,fname+1
    mov [patchfile],edx
    pushad
    call PatchUsingIPS
    popad
    pop eax
    mov ebx,[Prevextn]
    mov [eax],ebx
%ifdef __UNIXSDL__
    pushad
    call popdir
    popad
%endif
    ret

SECTION .bss
NEWSYM Prevextn,  resd 1
NEWSYM IPSPatched, resb 1
SECTION .text

OpenCombFile:
    mov edx,fnames+1
.next
    cmp byte[edx],0
    je .found
    inc edx
    jmp .next
.found
    dec edx
    cmp byte[edx],'.'
    je .found2
    jmp .found
.found2
    mov dword[edx],'.cmb'
    push edx
    mov dword[NumComboLocl],0
    mov edx,fnames+1
    call Open_File
    jc .failb
    mov bx,ax
    mov edx,ComboBlHeader
    mov ecx,23
    call Read_File
    mov al,[ComboBlHeader+22]
    or al,al
    jz .done
    mov [NumComboLocl],al
    mov ecx,[NumComboLocl]
    mov edx,ecx
    shl ecx,6
    add ecx,edx
    add ecx,edx
    mov edx,CombinDataLocl
    call Read_File
.done
    call Close_File
.failb
    pop edx
    mov dword[edx],'.srm'
    ret

NEWSYM SaveCombFile
    cmp byte[romloadskip],0
    jne near .notfound
    mov edx,fnames+1
.next
    cmp byte[edx],0
    je .found
    inc edx
    jmp .next
.found
    dec edx
    cmp byte[edx],'.'
    je .found2
    jmp .found
.found2
    mov dword[edx],'.cmb'
    push edx
    mov al,[NumComboLocl]
    or al,al
    jz .failb
    mov [ComboHeader+22],al
    mov edx,fnames+1
    call Create_File
    jc .failb
    mov bx,ax
    mov edx,ComboHeader
    mov ecx,23
    call Write_File
    mov ecx,[NumComboLocl]
    mov edx,ecx
    shl ecx,6
    add ecx,edx
    add ecx,edx
    mov edx,CombinDataLocl
    call Write_File
    call Close_File
.failb
    pop edx
    mov dword[edx],'.srm'
.notfound
    ret

NEWSYM loadfile
    mov byte[TextFile], 0
    call GetCurDir
    mov byte[InGUI],0
%ifdef __UNIXSDL__
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir
%endif

    jmp loadfileGUI.nogui

SECTION .data
.failop   db 'Error opening file!',13,10,0
.opened db 'File opened successfully!',13,10,0
.mult   db 'Multiple file format detected.',13,10,13,10,0
SECTION .bss
.cchar resb 1
.dotpos resd 1
.curfileofs resd 1
.filehand resw 1
.temp   resb 1
.fail   resb 1


NEWSYM Checksumvalue, resw 1
NEWSYM Checksumvalue2, resw 1 ;outdated, but needed for the command line loader
NEWSYM CRC32, resd 1
NEWSYM SramExists,    resb 1
NEWSYM NumofBanks,    resd 1
NEWSYM NumofBytes,    resd 1
NEWSYM ZipSupport, resb 1
InGUI resb 1

SECTION .data

PrevDir db '..',0
GUIfindBlank db '*.',0

SECTION .bss
ZipError resb 1

SECTION .data

SPC7110DirEntry db '*.bin',0
spc7110notfound db 'DECOMPRESSED PACK NOT FOUND',0

SECTION .bss

SPC7110Allocated resb 1
SPC7110CPtr resd 1
SPC7110CPtr2 resd 1
NEWSYM SPC7110Entries, resd 1

SECTION .text

NEWSYM SPC7110Load
    mov dword[SPC7110Entries],0
    mov esi,[romdata]
    add esi,32704+22
    add esi,8000h
    mov al,[esi]
    cmp byte[romtype],2
    jne .nothirom
    cmp al,0F9h
    je .spc7110
    cmp al,0F5h
    je .spc7110
.nothirom
    ret
.spc7110
    pushad
    call SPC7PackIndexLoad
    popad
    cmp dword[SPC7110IndexSize],0
    je .nodir
    ret
.nodir
    mov dword[Msgptr],spc7110notfound
    mov dword[MessageOn],60*6
    ret

NEWSYM loadfileGUI
    mov byte[InGUI],1
.nogui
    mov byte[spcon],0
    cmp byte[SPCDisable],1
    je .nosound
    mov byte[spcon],1
.nosound

    mov byte[TextFile], 0
    mov dword[MessageOn],0
    mov byte[loadedfromgui],1
    mov byte[yesoutofmemory],0
    mov byte[.fail],0
    mov byte[IPSPatched],0
    mov byte[GUIloadfailed],0

    mov edx,fname+1
    mov [ZOpenFileName],edx

    pushad
    call loadROM
    popad

    cmp dword[curromspace],0
    je near .failed

    call convertsram
    mov byte[SramExists],0

    ; change to sram dir
    pushad
    call SRAMChdir
    popad

    ; open .srm file
    mov edx,fnames+1
    call Open_File
    jc .notexist
    mov byte[SramExists],1
    mov bx,ax
    mov ecx,65536
    mov edx,[sram]
    call Read_File
    call Close_File
    jc near .failed2
.notexist
    call OpenCombFile
    cmp byte[InGUI],1
    je .inguib
    mov edx,.opened
    mov ah,9
    call Output_Text
.inguib

    mov eax,[curromspace]
    mov [.curfileofs],eax
    mov [NumofBytes],eax
    shr eax,15
    mov [NumofBanks],eax

    cmp byte[.fail],0
    je .notfailed
    mov byte[yesoutofmemory],1
.notfailed

    ; copy fnames to fname
    cmp byte[InGUI],1
    je .nosramtof
    mov eax,fname+1
    mov ebx,fnames+1
.loopsc
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    or dl,dl
    jnz .loopsc
.nosramtof
    mov byte[TextFile], 1
    cmp byte[IPSPatched],0
    jne .patched
    call PatchIPS
.patched
    ret

.failed
.failed2
    mov byte[TextFile], 1
    cmp byte[InGUI],1
    je .noguic
    mov edx,.failop
    mov ah,9
    call Output_Text
    jmp DosExit
.noguic
    mov byte[GUIloadfailed],1
    ret

SECTION .data
.failop   db 'Error opening file!',13,10,0
.opened db 'File opened successfully!',13,10,0
.mult   db 'Multiple file format detected.',13,10,13,10,0

SECTION .bss

.cchar resb 1
.dotpos resd 1
.curfileofs resd 1
.filehand resw 1
.temp resb 1
.fail resb 1
NEWSYM GUIloadfailed, resb 1

SECTION .text

NEWSYM convertsram
    cmp byte[cfgloadsdir],1
    je .sdrivechange
    ret
.sdrivechange
    ; copy fnames/fnamest to not have any '\' in them
    mov esi,fnames+1
    mov ebx,0
.next
    mov al,[esi]
    cmp al,0
    je .fincutoff
    cmp al,'\'
    je .cutoff
    cmp al,'/'
    je .cutoff
    cmp al,':'
    je .cutoff
    inc esi
    jmp .next
.cutoff
    inc esi
    mov ebx,esi
    jmp .next
.fincutoff
    cmp ebx,0
    je .nocutoff
    mov esi,ebx
    mov edi,fnames+1
.next2
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    cmp al,0
    jne .next2
.nocutoff
    mov esi,fnamest+1
    mov ebx,0
.nextb
    mov al,[esi]
    cmp al,0
    je .fincutoffb
    cmp al,'\'
    je .cutoffb
    cmp al,'/'
    je .cutoffb
    cmp al,':'
    je .cutoffb
    inc esi
    jmp .nextb
.cutoffb
    inc esi
    mov ebx,esi
    jmp .nextb
.fincutoffb
    cmp ebx,0
    je .nocutoffb
    mov esi,ebx
    sub esi,fnamest+1
    sub [statefileloc],esi
    mov esi,ebx
    mov edi,fnamest+1
.next2b
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    cmp al,0
    jne .next2b
.nocutoffb
    ; change to sram directory
    pushad
    call SRAMChdir
    popad
    ret

SECTION .data
NEWSYM CSStatus,  db '                        TYPE:           ',0
NEWSYM CSStatus2, db 'INTERLEAVED:      BANK:      CHKSUM:    ',0
NEWSYM CSStatus3, db 'VIDEO:                    CRC32:        ',0

;*******************************************************
; Show Information
;*******************************************************
;
; Maker Code = FFB0-FFB1
; Game Code = FFB2-FFB5
; Expansion RAM Size = FFBD (0=none, 1=16kbit, 3=64kbit, 5=256kbit,etc.
; Map Mode = FFD5 2.68-20h=map20h,21h=map21h,22h=reserved,23h=SA-1,25h=map25h
;                 3.58-30h=map20h,31h=map21h,35h=map25h,highspeed
; Rom Mask Version = FFDB
; FFD6 (ROM Type) : 0*=DSP,1*=SFX,2*=OBC1,3*=SA-1,E*-F*=other
;                   *3=ROM,*4=ROM+RAM,*5=ROM+RAM+BATTERY,*6=ROM+BATTERY
;                   F3=C4


SECTION .bss
NEWSYM DSP1Type, resb 1
NEWSYM intldone, resb 1
NEWSYM C4RamR,   resd 1
NEWSYM C4RamW,   resd 1
NEWSYM C4Ram,   resd 1
NEWSYM ROMTypeNOTFound, resb 1
NEWSYM Interleaved, resb 1
SECTION .text

NEWSYM outofmemfix
    mov esi,[romdata]
    cmp byte[romtype],2
    jne .nhirom
    add esi,8000h
.nhirom
    mov word[resetv],8000h
    mov word[xpc],8000h
    mov byte[esi],58h
    mov byte[esi+1],80h
    mov byte[esi+2],0FEh
    mov dword[Msgptr],outofmemoryerror
    cmp byte[newgfx16b],1
    jne .notso
    mov dword[Msgptr],outofmemoryerror2
.notso
    mov dword[MessageOn],0FFFFFFFFh
    ret

SECTION .bss
NEWSYM yesoutofmemory, resb 1
SECTION .data
NEWSYM outofmemoryerror, db 'OUT OF MEMORY.',0
NEWSYM outofmemoryerror2, db 'ROM IS TOO BIG.',0
