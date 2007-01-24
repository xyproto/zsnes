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

EXTSYM KeyRewind,statesaver,Voice0Status,UpdateDPage
EXTSYM StartGUI,romdata,initvideo,DosExit,sfxramdata,deinitvideo
EXTSYM vidbufferofsa,device2,RawDumpInProgress
EXTSYM KeySaveState,KeyLoadState,KeyQuickExit,KeyQuickLoad,KeyQuickRst
EXTSYM GUIDoReset,GUIReset,KeyOnStA,KeyOnStB,ProcessKeyOn,C4Enable,KeyQuickClock
EXTSYM KeyQuickSaveSPC,TimerEnable,splitflags,joinflags
EXTSYM KeyQuickSnapShot,csounddisable,videotroub,ResetTripleBuf
EXTSYM InitPreGame,Curtableaddr,curcyc,debugdisble,dmadata,guioff,memtabler8
EXTSYM SetupPreGame,memtablew8,regaccessbankr8,showmenu,snesmap2,snesmmap
EXTSYM DeInitPostGame,spcPCRam,xp,xpb,xpc,tablead
EXTSYM tableadc,SA1UpdateDPage,Makemode7Table,nextmenupopup,MovieProcessing
EXTSYM SFXEnable,wramdata,cycpbl,cycpblt,irqon,spcon
EXTSYM multchange,romispal,scrndis,sprlefttot,sprleftpr,processsprites
EXTSYM cachesprites,opcjmptab,CheatOn,Output_Text,Check_Key,Get_Key,
EXTSYM INTEnab,JoyCRead,NMIEnab,NumCheats,CurrentExecSA1,ReadInputDevice
EXTSYM StartDrawNewGfx,VIRQLoc,cachevideo,cfield,cheatdata,curblank,curnmi
EXTSYM curypos,cycpl,doirqnext,drawline,exechdma,hdmadelay,intrset,newengen
EXTSYM oamaddr,oamaddrs,resolutn,showvideo,starthdma,switchtonmi
EXTSYM switchtovirq,totlines,updatetimer,SA1Swap,SA1DoIRQ,JoyAOrig,JoyANow
EXTSYM JoyBOrig,JoyBNow,JoyCOrig,JoyCNow,JoyDOrig,JoyDNow,JoyEOrig,JoyENow
EXTSYM SA1Message,MultiTapStat,idledetectspc,SA1Control,SA1Enable,SA1IRQEnable
EXTSYM SPC700read,SPC700write,numspcvblleft,spc700idle,SA1IRQExec,ForceNewGfxOff
EXTSYM LethEnData,GUIQuit,IRAM,SA1Ptr,SA1BWPtr,outofmemfix
EXTSYM yesoutofmemory,ProcessMovies,MovieStop,ppustatus,C4VBlank
EXTSYM ReturnFromSPCStall,scanlines,MainLoop,MoviePassWaiting,MovieDumpRaw
EXTSYM NumberOfOpcodes,SfxCLSR,SfxSCMR,SfxPOR,sfx128lineloc,sfx160lineloc
EXTSYM sfx192lineloc,sfxobjlineloc,sfxclineloc,PLOTJmpa,PLOTJmpb,FxTable
EXTSYM FxTableb,FxTablec,FxTabled,SfxPBR,SCBRrel,SfxSCBR,SfxCOLR,SFXCounter
EXTSYM fxbit01,fxbit01pcal,fxbit23,fxbit23pcal,fxbit45,fxbit45pcal,fxbit67
EXTSYM fxbit67pcal,SfxSFR,nosprincr,cpucycle,switchtovirqdeb,switchtonmideb
EXTSYM MovieSeekBehind,BackupCVFrame,RestoreCVFrame,loadstate
EXTSYM KeyInsrtChap,KeyNextChap,KeyPrevChap,MovieInsertChapter,MovieSeekAhead
EXTSYM ResetDuringMovie,EMUPauseKey,INCRFrameKey,MovieWaiting,NoInputRead
EXTSYM AllocatedRewindStates,PauseFrameMode,RestorePauseFrame,BackupPauseFrame

%ifndef NO_DEBUGGER
EXTSYM debuggeron,startdebugger
%endif

%ifdef __MSDOS__
EXTSYM dssel,Game60hzcall,NextLineStart,FlipWait,LastLineStart,smallscreenon,ScreenScale
EXTSYM cvidmode,GUI16VID,ScreenShotFormat
%endif

SECTION .data
NEWSYM tempedx, dd 0
NEWSYM tempesi, dd 0
NEWSYM tempedi, dd 0
NEWSYM tempebp, dd 0
NEWSYM RewindTimer, dd 0
NEWSYM BackState, db 1
NEWSYM BackStateSize, dd 6
NEWSYM DblRewTimer, dd 0
SECTION .text

NEWSYM ProcessRewind
    mov eax,[KeyRewind]
    cmp byte[pressed+eax],1
    jne near .notokay
    mov byte[pressed+eax],2

    pushad
    call RestoreCVFrame
    popad

    cmp byte[PauseFrameMode],1
    jne .notpauserewind
    pushad
    call BackupPauseFrame
    popad
.notpauserewind

    call UpdateDPage
    mov esi,[tempesi]
    mov edi,[tempedi]
    mov ebp,[tempebp]
    mov edx,[tempedx]

.notokay
    ret

NEWSYM UpdateRewind
    cmp byte[AllocatedRewindStates],0
    je .norewinds
    cmp dword[KeyRewind],0
    je .norewinds

    dec dword[DblRewTimer]
    dec dword[RewindTimer]
    jnz .checkrewind

    mov [tempedx],edx
    mov [tempesi],esi
    mov [tempedi],edi
    mov [tempebp],ebp

    pushad
    call BackupCVFrame
    popad

.checkrewind
    call ProcessRewind
    call UpdateDPage
.norewinds
    ret

SECTION .data
NEWSYM MuteVoiceF, dd 0
SECTION .text

VoiceEndMute:
    mov byte[MuteVoiceF],0
    ret

%macro StartMute 1
    mov al,[Voice0Status+%1]
    or al,al
    jz %%notmuted
    or byte[MuteVoiceF],1 << %1
%%notmuted
%endmacro

VoiceStartMute:
    mov byte[MuteVoiceF],0
    push eax
    StartMute 0
    StartMute 1
    StartMute 2
    StartMute 3
    StartMute 4
    StartMute 5
    StartMute 6
    StartMute 7
    pop eax
    ret


%macro stim 0
%ifdef __MSDOS__
    sti
%endif
%endmacro

%macro ProcessIRQStuff 0
    ; check for VIRQ/HIRQ
    test dl,04h
    jnz %%virqdo
    cmp byte[doirqnext],1
    je near .virq
%%virqdo
    test byte[INTEnab],20h
    jz near %%novirq
    mov ax,[VIRQLoc]
    cmp ax,[resolutn]
    jne %%notres
    dec ax
;    inc ax
%%notres
    cmp ax,0FFFFh
    jne %%notzero
    xor ax,ax
%%notzero
    cmp word[curypos],ax
    jne near %%noirq
    test byte[INTEnab],10h
    jnz %%tryhirq
%%startirq
    cmp byte[intrset],1
    jne %%nointrseta
    mov byte[intrset],2
%%nointrseta
    mov byte[irqon],80h
    test dl,04h
    jnz %%irqd
    mov byte[doirqnext],1
    jmp .virq
%%novirq
    test byte[INTEnab],10h
    jz %%noirq
%%setagain
    cmp byte[intrset],2
    jbe %%nointrseta3
    dec byte[intrset]
    cmp byte[intrset],2
    ja %%noirq
%%nointrseta3
    cmp byte[intrset],1
    jne %%nointrseta2
    test byte[INTEnab],80h
    jz %%tryhirq
    mov byte[intrset],8
    jmp %%noirq
%%nointrseta2
    test dl,04h
    jnz %%noirq
%%tryhirq
    jmp %%startirq
%%irqd
    mov byte[doirqnext],1
%%noirq
%endmacro


; .returnfromsfx

; pexecs
; *** Copy to PC whenever a non-relative jump is executed

SECTION .data
NEWSYM romloadskip, db 0
NEWSYM SSKeyPressed, dd 0
NEWSYM SPCKeyPressed, dd 0
NEWSYM NoSoundReinit, dd 0
NEWSYM NextNGDisplay, db 0
NEWSYM TempVidInfo, dd 0
NEWSYM tempdh, db 0

SECTION .text


; this wonderful mess starts up the CPU and initialized the emulation state
NEWSYM start65816

    call initvideo

    cmp byte[videotroub],1
    jne .notrouble
    ret
.notrouble

    mov edi,[vidbufferofsa]
    mov ecx,37518
    xor eax,eax
    rep stosd
    cmp byte[romloadskip],1
    je near StartGUI

NEWSYM continueprog
    ; clear keyboard presses
    mov esi,pressed
    mov ecx,256+128+64
    mov al,0
.loopa
    mov [esi],al
    inc esi
    dec ecx
    jnz .loopa

    mov byte[romloadskip],0
%ifndef NO_DEBUGGER
    mov byte[debuggeron],0
%endif
    mov byte[exiter],0

    call InitPreGame
    jmp reexecute

NEWSYM continueprognokeys
    mov byte[romloadskip],0
%ifndef NO_DEBUGGER
    mov byte[debuggeron],0
%endif
    mov byte[exiter],0

    call InitPreGame
    jmp reexecuteb2

; Incorrect

NEWSYM reexecuteb
%ifndef __MSDOS__
    jmp reexecuteb2
%endif
NEWSYM reexecute

    ; clear keyboard presses
    mov esi,pressed
    mov ecx,256+128+64
    mov al,0
.loopa
    cmp byte[esi],2
    jne .notclear
    mov [esi],al
.notclear
    inc esi
    dec ecx
    jnz .loopa
reexecuteb2:
    cmp byte[NoSoundReinit],1
    je .skippregame
    call SetupPreGame
.skippregame

    ; initialize variables (Copy from variables)
    call UpdateDPage
    call SA1UpdateDPage
    call Makemode7Table
    cmp byte[SFXEnable],0
    je .nosfxud
    call UpdateSFX
.nosfxud
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov esi,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov esi,dmadata-4300h
.skiplower
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    mov dl,[xp]                 ; set flags
    mov dh,[curcyc]             ; set cycles

    mov bl,dl

    mov edi,[tableadc+ebx*4]
    or byte[curexecstate],2

    mov ebp,[spcPCRam]

    mov byte[NoSoundReinit],0
    mov byte[csounddisable],0
    mov byte[NextNGDisplay],0

    call splitflags

    call execute

    call joinflags

    ; de-init variables (copy to variables)

    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    call ResetTripleBuf

    mov eax,[KeySaveState]
    test byte[pressed+eax],1
    jnz .soundreinit
    mov eax,[KeyLoadState]
    test byte[pressed+eax],1
    jz .skipsoundreinit
.soundreinit
    mov byte[NoSoundReinit],1
    mov byte[csounddisable],1
.skipsoundreinit

    cmp byte[NoSoundReinit],1
    je .skippostgame
    call DeInitPostGame
.skippostgame

    ;Multipass Movies
    cmp byte[MoviePassWaiting],1
    jne .nomoviepasswaiting
    pushad
    call MovieDumpRaw
    popad
    jmp continueprog
.nomoviepasswaiting

    ; clear all keys
    call Check_Key
    cmp al,0
    je .nokeys
.yeskeys
    call Get_Key
    call Check_Key
    cmp al,0
    jne .yeskeys
.nokeys

    cmp byte[nextmenupopup],1
    je near showmenu
    cmp byte[ReturnFromSPCStall],1
    je near .activatereset
    mov eax,[KeySaveState]
    test byte[pressed+eax],1
    jz .nosavestt
    mov byte[pressed+1],0
    mov byte[pressed+eax],2
    pushad
    call statesaver
    popad
    jmp reexecuteb
.nosavestt
    mov eax,[KeyLoadState]
    test byte[pressed+eax],1
    jz .noloadstt0
    pushad
    call loadstate
    popad
    jmp reexecuteb
.noloadstt0
    mov eax,[KeyInsrtChap]
    test byte[pressed+eax],1
    jz .noinsertchapter
    mov byte[pressed+eax],0
    pushad
    call MovieInsertChapter
    popad
    jmp continueprognokeys
.noinsertchapter
    mov eax,[KeyNextChap]
    test byte[pressed+eax],1
    jz .nonextchapter
    mov byte[pressed+eax],0
    mov byte[multchange],1
    pushad
    call MovieSeekAhead
    popad
    jmp continueprognokeys
.nonextchapter
    mov eax,[KeyPrevChap]
    test byte[pressed+eax],1
    jz .noprevchapter
    mov byte[pressed+eax],0
    mov byte[multchange],1
    pushad
    call MovieSeekBehind
    popad
    jmp continueprognokeys
.noprevchapter
    cmp byte[SSKeyPressed],1
    je near showmenu
    cmp byte[SPCKeyPressed],1
    je near showmenu
%ifndef NO_DEBUGGER
   cmp byte[debugdisble],0
    jne .nodebugger
    test byte[pressed+59],1
    jne near startdebugger
.nodebugger
%endif
    test byte[pressed+59],1
    jne near showmenu
    mov eax,[KeyQuickRst]
    test byte[pressed+eax],1
    jz .noreset
.activatereset
    pushad
    mov byte[GUIReset],1
    cmp byte[MovieProcessing],2 ;Recording
    jne .nomovierecording
    call ResetDuringMovie
    jmp .movieendif
.nomovierecording
    call GUIDoReset
.movieendif
    popad
    mov byte[ReturnFromSPCStall],0
    jmp continueprog
.noreset
    cmp byte[guioff],1
    je near endprog
    mov eax,[KeyQuickExit]
    test byte[pressed+eax],1
    jnz near endprog
    jmp StartGUI

NEWSYM endprog
    call deinitvideo
    pushad
    call MovieStop
    popad

    jmp DosExit

NEWSYM interror
    stim
    call deinitvideo
    mov edx,.nohand         ;use extended
    mov ah,9                ;DOS- API
    call Output_Text                 ;to print a string
    jmp DosExit

SECTION .data
.nohand db 'Cannot process interrupt handler!',13,10,0

; global variables
NEWSYM invalid, db 0
NEWSYM invopcd, db 0
NEWSYM pressed, times 256+128+64 db 0          ; keyboard pressed keys in scancode
NEWSYM exiter, db 0
NEWSYM oldhand9o, dd 0
NEWSYM oldhand9s, dw 0
NEWSYM oldhand8o, dd 0
NEWSYM oldhand8s, dw 0
NEWSYM opcd,      dd 0
NEWSYM pdh,       dd 0
NEWSYM pcury,     dd 0
NEWSYM timercount, dd 0
NEWSYM initaddrl, dd 0                  ; initial address location
NEWSYM NetSent, dd 0
NEWSYM nextframe, dd 0                  ; tick count for timer
NEWSYM curfps,    db 0                  ; frame/sec for current screen
;NEWSYM newgfxerror, db 'NEED MEMORY FOR GFX ENGINE',0
;NEWSYM newgfxerror2, db 'NEED 320x240 FOR NEW GFX 16B',0
;newgfxerror db 'NEW GFX IN 16BIT IS N/A',0
NEWSYM HIRQCycNext,   dd 0
NEWSYM HIRQNextExe,   db 0


SECTION .text

;*******************************************************
; Int 08h vector
;*******************************************************

; sets to either 60Hz or 50Hz depending on PAL/NTSC
NEWSYM init60hz
    cmp byte[romispal],0
    jne .dopal
    mov al,00110110b
    out 43h,al
    mov ax,19900        ; 65536/(60/((65536*24+175)/(60*60*24)))
    mov dword[timercount],19900
    out 40h,al
    mov al,ah
    out 40h,al
    ret
.dopal
    mov al,00110110b
    out 43h,al
    mov ax,23863        ; 65536/(50/((65536*24+175)/(60*60*24)))
    mov dword[timercount],23863
    out 40h,al
    mov al,ah
    out 40h,al
    ret

NEWSYM init18_2hz
    mov al,00110110b
    out 43h,al
    mov ax,0
    mov dword[timercount],65536
    out 40h,al
    mov al,ah
    out 40h,al
    ret

%ifdef __MSDOS__
NEWSYM handler8h
    cli
    push ds
    push eax
;    mov ax,0
    mov ax,[cs:dssel]
NEWSYM handler8hseg
    mov ds,ax
    call Game60hzcall
    mov eax,[timercount]
    sub dword[timeradj],eax
    jnc .noupd
    add dword[timeradj],65536
    pushf
    call far [oldhand8o]
.noupd
    mov al,20h
    out 20h,al
    pop eax
    pop ds
    sti
    iretd
%endif

SECTION .data
NEWSYM timeradj, dd 65536
NEWSYM t1cc, dw 0
SECTION .text

;*******************************************************
; Int 09h vector
;*******************************************************

%ifdef __MSDOS__
SECTION .bss
NEWSYM skipnextkey42, resb 1
SECTION .text

NEWSYM handler9h
    cli
    push ds
    push eax
    push ebx
    mov ax,[cs:dssel]
    mov ds,ax
    xor ebx,ebx
    in al,60h                 ; get keyboard scan code
    cmp al,42
    jne .no42
    cmp byte[skipnextkey42],0
    je .no42
    mov byte[skipnextkey42],0
    jmp .skipkeyrel
.no42
    cmp al,0E0h
    jne .noE0
    mov byte[skipnextkey42],1
    jmp .skipkeyrel
.noE0
    mov byte[skipnextkey42],0
    mov bl,al
    xor bh,bh
    test bl,80h               ; check if bit 7 is on (key released)
    jnz .keyrel
    cmp byte[pressed+ebx],0
    jne .skipa
    mov byte[pressed+ebx],1        ; if not, set key to pressed
.skipa
    jmp .skipkeyrel
.keyrel
    and ebx,7Fh
    cmp ebx,59
    je .skipkeyrel
    cmp ebx,[KeySaveState]
    je .skipkeyrel
    cmp ebx,[KeyLoadState]
    je .skipkeyrel
    cmp ebx,[KeyQuickExit]
    je .skipkeyrel
    cmp ebx,[KeyQuickLoad]
    je .skipkeyrel
    cmp ebx,[KeyQuickRst]
    je .skipkeyrel
    cmp bl,1
    je .skipkeyrel
    mov byte[pressed+ebx],0        ; if not, set key to pressed
.skipkeyrel
    mov byte[pressed],0
    in al,61h
    mov ah,al
    or al,80h
    out 61h,al
    mov al,20h                ; turn off interrupt mode
    out 20h,al
    pop ebx                          ; Pop registers off
    pop eax                          ; stack in correct
    pop ds
    sti
    iretd
%endif

SECTION .data
ALIGN32
NEWSYM soundcycleft, dd 0
NEWSYM curexecstate, dd 0

NEWSYM nmiprevaddrl, dd 0       ; observed address -5
NEWSYM nmiprevaddrh, dd 0       ; observed address +5
NEWSYM nmirept,      dd 0       ; NMI repeat check, if 6 then okay
NEWSYM nmiprevline,  dd 224     ; previous line
NEWSYM nmistatus,    dd 0       ; 0 = none, 1 = waiting for nmi location,
                        ; 2 = found, disable at next line
NEWSYM joycontren,   dd 0       ; joystick read control check
NEWSYM NextLineCache, db 0
NEWSYM ZMVZClose, db 0

SECTION .text

Donextlinecache:
    cmp word[curypos],0
    je .nocache
    mov ax,[resolutn]
    dec ax
    cmp word[curypos],ax
    jae .nocache
    test byte[scrndis],10h
    jnz .nocache
    cmp byte[curblank],0h
    jne .nocache
    push ecx
    push ebx
    push esi
    push edi
    xor ecx,ecx
    mov cl,[curypos]
    push edx
.next
    mov byte[sprlefttot+ecx],0
    mov dword[sprleftpr+ecx*4],0
    inc cl
    jnz .next
    call processsprites
    call cachesprites
    pop edx
    pop edi
    pop esi
    pop ebx
    pop ecx
.nocache
    mov byte[NextLineCache],0
    ret

;*******************************************************
; 65816 execution
;*******************************************************

SECTION .text

NEWSYM exitloop2
   mov byte[ExecExitOkay],0
NEWSYM exitloop
   ret

ALIGN16

%macro FlipCheck 0
%ifdef __MSDOS__
   cmp byte[FlipWait],0
   je %%noflip
   push edx
   push eax
   mov dx,3DAh             ;VGA status port
   in al,dx
   test al,8
   jz %%skipflip
   push ebx
   push ecx
   mov ax,4F07h
   mov bh,00h
   mov bl,00h
   xor cx,cx
   mov dx,[NextLineStart]
   mov [LastLineStart],dx
   int 10h
   mov byte[FlipWait],0
   pop ecx
   pop ebx
%%skipflip
   pop eax
   pop edx
%%noflip
%endif
%endmacro

NEWSYM execute
NEWSYM execloop
   mov bl,dl
   test byte[curexecstate],2
   jnz .sound
.startagain
   call dword near [edi+ebx*4]
.cpuover
   jmp cpuover
.sound
   mov edi,[tableadc+ebx*4]
%ifdef OPENSPC
   pushad
   mov bl,[esi]
   movzx eax,byte[cpucycle+ebx]
   mov ebx,0xC3A13DE6
   mul ebx
   add [ospc_cycle_frac],eax
   adc [SPC_Cycles],edx
   call OSPC_Run
   popad
%else
   sub dword[cycpbl],55
   jnc .skipallspc
   mov eax,[cycpblt]
   mov bl,[ebp]
   add dword[cycpbl],eax
   ; 1260, 10000/12625
   inc ebp
   call dword near [opcjmptab+ebx*4]
   xor ebx,ebx
.skipallspc
%endif
   mov bl,[esi]
   inc esi
   sub dh,[cpucycle+ebx]
   jc .cpuovers
   call dword near [edi+ebx*4]
.cpuovers
   jmp cpuover



SECTION .data
ALIGN32
NEWSYM ExecExitOkay, db 1
NEWSYM JoyABack, dd 0
NEWSYM JoyBBack, dd 0
NEWSYM JoyCBack, dd 0
NEWSYM JoyDBack, dd 0
NEWSYM JoyEBack, dd 0
NEWSYM NetCommand, dd 0
NEWSYM spc700read, dd 0
NEWSYM lowestspc,  dd 0
NEWSYM highestspc, dd 0
NEWSYM SA1UBound,  dd 0
NEWSYM SA1LBound,  dd 0
NEWSYM SA1SH,      dd 0
NEWSYM SA1SHb,     dd 0
NEWSYM NumberOfOpcodes2, dd 0
NEWSYM ChangeOps, dd 0
NEWSYM SFXProc,    dd 0
NEWSYM EMUPause, db 0
NEWSYM INCRFrame, db 0
NEWSYM NoHDMALine, db 0
SECTION .text

NEWSYM cpuover
    dec esi
    cmp byte[HIRQNextExe],0
    je .nohirq
    add dh,[HIRQCycNext]
    mov byte[HIRQCycNext],0
    jmp .hirq
.nohirq
    cmp byte[SA1Enable],0
    je near .nosa1b
    test byte[exiter],01h
    jnz near .nosa1
    mov byte[cycpl],150
    test byte[SA1Control],60h
    jnz near .nosa1
    call SA1Swap
    cmp byte[CurrentExecSA1],15
    ja .nocontinueexec
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain
.nocontinueexec

    ; check for sa-1 speed hacks
    mov byte[SA1SHb],0
    cmp word[IRAM+0A0h],80BFh
    jne .noshb2
    cmp word[IRAM+020h],0
    jne .noshb2
    mov ecx,[SA1Ptr]        ; small speed hack
    sub ecx,[romdata]
    cmp ecx,83h
    jb .skipsh
    cmp ecx,97h
    ja .skipsh
    mov byte[SA1SHb],1
.skipsh
.noshb2

    mov ecx,[SA1Ptr]        ; small speed hack
    cmp dword[ecx],0FCF04BA5h
    je .shm
    cmp dword[ecx-2],0FCF04BA5h
    jne .skipshm
.shm
    cmp byte[IRAM+4Bh],0
    jne .skipshm
    mov byte[SA1SHb],1
.skipshm

    cmp dword[ecx],80602EEEh
    jne .skipshc
    sub ecx,[romdata]
    cmp ecx,4E5h
    jb .skipshc
    cmp ecx,4E8h
    ja .skipshc
    mov byte[SA1SHb],1
    mov ecx,[SA1BWPtr]
    add word[ecx+602Eh],4
.skipshc

    test word[IRAM+0Ah],8000h
    jnz .noshb2b
    test word[IRAM+0Eh],8000h
    jz .noshb2b
    mov ecx,[SA1Ptr]        ; small speed hack
    sub ecx,[romdata]
    cmp ecx,0C93h
    jb .skipshb
    cmp ecx,0C9Bh
    ja .skipshb
    mov byte[SA1SHb],1
.skipshb
    cmp ecx,0CB8h
    jb .skipshb3
    cmp ecx,0CC0h
    ja .skipshb3
    mov byte[SA1SHb],1
.skipshb3
.noshb2b

    sub esi,[wramdata]
    cmp esi,224h
    jb .nosh
    cmp esi,22Eh
    ja .nosh
    mov ecx,[wramdata]
    mov dword[SA1LBound],224h
    mov dword[SA1UBound],22Eh
    add dword[SA1LBound],ecx
    add dword[SA1UBound],ecx
    mov byte[SA1SH],1
.nosh
    cmp esi,1F7C6h
    jb .noshb
    cmp esi,1F7CCh
    ja .noshb
    mov ecx,[wramdata]
    mov dword[SA1LBound],1F7C6h
    mov dword[SA1UBound],1F7CCh
    add dword[SA1LBound],ecx
    add dword[SA1UBound],ecx
    mov byte[SA1SH],1
.noshb
    cmp esi,14h
    jb .noshc
    cmp esi,1Ch
    ja .noshc
    mov ecx,[wramdata]
    cmp dword[ecx+14h],0F023002Ch
    jne .noshc
    mov dword[SA1LBound],14h
    mov dword[SA1UBound],1Ch
    add dword[SA1LBound],ecx
    add dword[SA1UBound],ecx
    mov byte[SA1SH],1
.noshc
    add esi,[wramdata]
    sub esi,[romdata]
    cmp esi,0A56h
    jb .noshbc
    cmp esi,0A59h
    ja .noshbc
    mov ecx,[romdata]
    mov dword[SA1LBound],0A56h
    mov dword[SA1UBound],0A59h
    add dword[SA1LBound],ecx
    add dword[SA1UBound],ecx
    mov byte[SA1SH],1
.noshbc
    xor ecx,ecx
    add esi,[romdata]
    xor dh,dh
    mov byte[cycpl],10
    cmp byte[CurrentExecSA1],255
    jne .notsa1255
    mov byte[cycpl],160
.notsa1255
    mov byte[CurrentExecSA1],0
    test dl,04h
    jnz .nosa1
    test byte[SA1IRQEnable],80h
    jz .nosa1
    test byte[SA1DoIRQ],4
    jz .nosa1
    and byte[SA1DoIRQ],0FBh
    mov al,[SA1Message+1]
    mov [SA1Message+3],al
    or byte[SA1IRQExec],1
    ; Start IRQ
    add dh,10
    jmp .virq
.nosa1
    test byte[SA1IRQEnable],20h
    jz .nosa1chirq
    test byte[SA1DoIRQ],8
    jz .nosa1chirq
;    jmp .nosa1chirq
    and byte[SA1DoIRQ],0F7h
    mov al,[SA1Message+1]
    mov [SA1Message+3],al
    or byte[SA1IRQExec],2
    ; Start IRQ
    add dh,10
    jmp .virq
.nosa1chirq
.nosa1b
    FlipCheck
    cmp byte[NextLineCache],0
    je .nosprcache
    call Donextlinecache
.nosprcache
    cmp byte[KeyOnStB],0
    je .nokeyon
    mov al,[KeyOnStB]
    call ProcessKeyOn
.nokeyon
    mov al,[KeyOnStA]
    mov [KeyOnStB],al
    mov byte[KeyOnStA],0
    test byte[exiter],01h
    jnz near exitloop2
    ;Multipass Movies
    cmp byte[MoviePassWaiting],1
    jne .nomoviepasswaiting
    jmp exitloop2
.nomoviepasswaiting


    test byte[SfxSFR],20h
    jnz near StartSFX
.returnfromsfx
;    inc dword[numinst]          ;Temporary
    inc word[curypos]
    add dh,[cycpl]
    mov ax,[totlines]
    cmp word[curypos],ax
    jae near .overy
    cmp byte[spcon],0
    je .nosound
    call updatetimer
.nosound
    mov ax,[resolutn]
    inc ax
    cmp [curypos],ax
    je near .nmi

    mov ax,[resolutn]
    cmp [curypos],ax
    je near .hdma
;    add ax,2
;    cmp [curypos],ax
;    je near .hdma
.hdmacont

    ; check for VIRQ/HIRQ/NMI
    ProcessIRQStuff
    mov ax,[resolutn]
    test byte[nmistatus],0
    jz .drawline2
    cmp [curypos],ax
    je .step2
.drawline2
    test byte[nmistatus],1
    jnz .step2
    cmp [curypos],ax
    jbe .drawline
    jmp .skiphdma
.step2
    cmp [curypos],ax
    jb .drawline
.skiphdma
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.hdma
    call exechdma
    jmp .hdmacont

.drawline
    mov al,[nmiprevline]
    cmp [curypos],al
    jb near .noskip
    cmp byte[nmirept],10
    jb near .noskip
    ; if between correct address, decrease by 2, set nmistatus as 2
    ; if not, set nmistatus as 1, increase by 2
    cmp byte[curexecstate],0
    jne .nn
    xor dh,dh
.nn
    cmp byte[nmistatus],2
    jae near .noskip
    cmp esi,[nmiprevaddrl]
    jb .failcheck2
    cmp esi,[nmiprevaddrh]
    ja .failcheck2
    cmp byte[nmiprevline],20
    jb .nodec
    sub byte[nmiprevline],10
.nodec
    xor eax,eax
    mov al,[esi]
    mov byte[nmistatus],2
    and byte[curexecstate],0FEh
.failcheck2
    add byte[nmiprevline],1
    mov byte[nmistatus],1
.noskip
    cmp byte[hdmadelay],0
    je .dohdma
    dec byte[hdmadelay]
    jmp .nodohdma
.dohdma
    cmp word[curypos],1
    jne .nooffby1line
    test byte[INTEnab],20h
    jz .nooffby1line
    cmp word[VIRQLoc],0
    je .nodohdma
.nooffby1line
    mov ax,[resolutn]
    dec ax
    cmp [curypos],ax
    jae .nodohdma
.dohdma3
    call exechdma
.nodohdma
    cmp word[curypos],1
    jne .nocache
    call cachevideo
.nocache
    cmp byte[curblank],0
    jne .nodrawlineb2
    call drawline
.nodrawlineb2
    cmp byte[curexecstate],2
    je near pexecs
    cmp byte[curexecstate],0
    jne .yesexec
    xor dh,dh
.yesexec
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.nmi
    mov byte[irqon],80h
    mov byte[doirqnext],0
    cmp byte[yesoutofmemory],1
    jne .noout
    call outofmemfix
.noout

    dec word[curypos]
    mov [tempdh],dh
    xor dh,dh

;    mov al,[SFXIRQFlag]
    mov byte[doirqnext],0

    call exechdma
    call exechdma

    mov byte[NextNGDisplay],1
    cmp byte[newengen],0
    je .nonewgfx
    cmp byte[curblank],0
    jne .nonewgfx
    cmp byte[ForceNewGfxOff],0
    jne .nonewgfx
;    cmp byte[NextNGDisplay],0
;    je .nonewgfx
    call StartDrawNewGfx
.nonewgfx
    cmp byte[GUIQuit],1
    je near endprog
    mov eax,[KeyQuickSnapShot]
    or eax,eax
    jz .nosskey
    test byte[pressed+eax],1
    jz .nosskey
%ifdef __MSDOS__
    movzx eax,byte[cvidmode]
    cmp byte[GUI16VID+eax],1
    je .pngok
    mov byte[ScreenShotFormat],0
.pngok
%endif
    mov byte[SSKeyPressed],1
    mov byte[pressed+eax],2
    jmp exitloop
.nosskey
    mov eax,[KeyQuickClock]
    or eax,eax
    jz .noclockkey
    test byte[pressed+eax],1
    jz .noclockkey
    xor byte[TimerEnable],1
    mov byte[pressed+eax],2
.noclockkey
    mov eax,[KeyQuickSaveSPC]
    or eax,eax
    jz .nosavespckey
    test byte[pressed+eax],1
    jz .nosavespckey
    mov byte[SPCKeyPressed],1
    mov byte[pressed+eax],2
    jmp exitloop
.nosavespckey
    mov eax,[EMUPauseKey]
    or eax,eax
    jz .nopausekey
    test byte[pressed+eax],1
    jz .nopausekey
    xor byte[EMUPause],1
    mov byte[pressed+eax],2
.nopausekey
    mov eax,[INCRFrameKey]
    or eax,eax
    jz .noincrframekey
    test byte[pressed+eax],1
    jz .noincrframekey
    xor byte[INCRFrame],1
    mov byte[pressed+eax],2
.noincrframekey
    test byte[pressed+1],01h
    jnz near exitloop
    test byte[pressed+59],01h
    jnz near exitloop
    cmp byte[nextmenupopup],1
    je near exitloop
    cmp byte[nextmenupopup],2
    jb .skipmenupop
    dec byte[nextmenupopup]
.skipmenupop
    mov eax,[KeySaveState]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyLoadState]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyInsrtChap]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyPrevChap]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyNextChap]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyQuickRst]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyQuickExit]
    test byte[pressed+eax],01h
    jnz near exitloop
    mov eax,[KeyQuickLoad]
    test byte[pressed+eax],01h
    jnz near exitloop
    cmp byte[ExecExitOkay],0
    je .returntoloop
    dec byte[ExecExitOkay]
.returntoloop
    mov dh,[tempdh]
    inc word[curypos]
    cmp byte[NoInputRead],1
    je .noinputread
    call ReadInputDevice
.noinputread

    ;Pause and Frame increment
    cmp byte[PauseFrameMode],3
    jne .nopauseframemode3
    pushad
    call RestorePauseFrame
    popad
    mov esi,[tempesi]
    mov edi,[tempedi]
    mov ebp,[tempebp]
    mov edx,[tempedx]
.nopauseframemode3

    cmp byte[EMUPause],1
    jne .noemupause
    cmp byte[RawDumpInProgress],1
    je .noemupause

    cmp byte[PauseFrameMode],1
    jne .nopauseframemode1
    mov [tempedx],edx
    mov [tempesi],esi
    mov [tempedi],edi
    mov [tempebp],ebp
    pushad
    call BackupPauseFrame
    popad
.nopauseframemode1

    call ProcessRewind

    cmp byte[PauseFrameMode],2
    jne .nopauseframemode2
    mov byte[PauseFrameMode],3
    jmp .noprocmovie
.nopauseframemode2

    cmp byte[INCRFrame],1
    jne .noframeincr
    xor byte[INCRFrame],1
    jmp .noemupause
.noframeincr

    ;Update screen - DISABLED FOR NOW
    pushad
    ;call StartDrawNewGfx
    ;call showvideo
    ;call cachevideo
    popad

    jmp .nonewgfx
.noemupause

    ;Rewind update must be done before process this frame of movie, so rewind doesn't
    ;back up incremented values (some vars being for the next frame)
    call UpdateRewind

    cmp byte[MovieProcessing],0
    je .noprocmovie
    pushad
    call ProcessMovies
    popad
    cmp byte[GUIReset],1
    jne .notreset
    mov byte[MovieWaiting],1
    mov eax,[KeyQuickRst]
    mov byte[pressed+eax],01h
    jmp near exitloop
.notreset
    cmp byte[MovieProcessing],0
    jne .noprocmovie
    cmp byte[ZMVZClose],1
    jne .noprocmovie
    jmp DosExit
.noprocmovie

    cmp byte[device2],3
    jne .nolethalen1
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen1
    ;Todo, add second gun...
    cmp byte[device2],4
    jne .nolethalen2
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen2


    test byte[INTEnab],1
    jz .noresetjoy
    mov eax,[JoyAOrig]
    rol eax,16
    mov [JoyANow],eax
    mov eax,[JoyBOrig]
    rol eax,16
    mov [JoyBNow],eax
    mov eax,[JoyCOrig]
    rol eax,16
    mov [JoyCNow],eax
    mov eax,[JoyDOrig]
    mov [JoyDNow],eax
    mov eax,[JoyEOrig]
    mov [JoyENow],eax
    mov byte[JoyCRead],0
.noresetjoy
    mov byte[MultiTapStat],80h

    cmp byte[C4Enable],0
    je .noC4
    call C4VBlank
.noC4
;    mov byte[hdmastartsc],0
    mov byte[joycontren],0
    test byte[curexecstate],01h
    jnz .dis65816
    or byte[curexecstate],01h
.dis65816
    cmp byte[CheatOn],1
    je near .cheater
.returncheat
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    jne .novirqz
    mov byte[doirqnext],1
.novirqz
    mov ax,[oamaddrs]
    mov [oamaddr],ax
    mov byte[nosprincr],0
    call showvideo
    xor ebx,ebx
    mov byte[NMIEnab],81h
    test byte[INTEnab],80h
    jz near .nonmi
;    cmp byte[intrset],1
;    je near .nonmi

.nmiokay
    mov byte[curnmi],1
    cmp byte[intrset],1
    jne .nointrset
    mov byte[intrset],2
.nointrset
    cmp byte[nmistatus],1
    jne .notnonmifound
    mov byte[nmirept],0
.notnonmifound
    mov byte[nmistatus],0
    cmp byte[nmirept],0
    jne .nocheck
    mov al,[resolutn]
    sub al,2
    mov [nmiprevline],al
    mov dword[nmiprevaddrl],0FFFFFFFFh
    mov dword[nmiprevaddrh],0
    mov byte[nmirept],1
    mov byte[doirqnext],0
    jmp switchtonmi
.nocheck
    cmp byte[nmirept],10
    je .nextcheck
    cmp esi,[nmiprevaddrl]
    jae .notlower
    mov [nmiprevaddrl],esi
.notlower
    cmp esi,[nmiprevaddrh]
    jbe .notgreater
    mov [nmiprevaddrh],esi
.notgreater
    inc byte[nmirept]
    jmp switchtonmi
.nextcheck
    mov eax,[nmiprevaddrh]
    sub eax,[nmiprevaddrl]
    cmp eax,10
    ja .failcheck
    cmp esi,[nmiprevaddrl]
    jb .failcheck
    cmp esi,[nmiprevaddrh]
    ja .failcheck
    mov byte[doirqnext],0
    jmp switchtonmi
.failcheck
    mov byte[nmirept],0
    mov dword[nmiprevaddrl],0FFFFFFFFh
    mov dword[nmiprevaddrh],0
    mov byte[doirqnext],0
    jmp switchtonmi
.nonmi
    cmp byte[intrset],1
    jne .nointrset2w
    mov byte[intrset],2
.nointrset2w
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain


.overy
    mov dh,80
%ifdef __MSDOS__
    cmp byte[smallscreenon],1
    je .nocfield
    cmp byte[ScreenScale],1
    je .nocfield
%endif
    cmp byte[scanlines],0
    jne .nocfield
    xor byte[cfield],1
.nocfield
    mov word[curypos],0
    xor byte[ppustatus],80h
    cmp dword[numspcvblleft],0
    je near .novblch
    cmp [lowestspc],ebp
    ja .failspc
    cmp [highestspc],ebp
    jb .failspc
    jmp .okayspc
.failspc
    mov eax,ebp
    sub eax,10
    mov [lowestspc],eax
    add eax,20
    mov [highestspc],eax
    mov dword[spc700idle],0
.okayspc
    cmp dword[SPC700write],0
    jne .notwritespc
    cmp dword[spc700read],0
    je .notwritespc
    cmp dword[SPC700read],1500
    jb .notwritespc
    inc dword[spc700idle]
    cmp dword[spc700idle],30
    jne .noidleend
    call idledetectspc
    cmp byte[ReturnFromSPCStall],1
    jne .noidleend
    mov byte[ExecExitOkay],0
    jmp exitloop
.noidleend
    jmp .notidle
.notwritespc
    mov dword[spc700idle],0
.notidle
    dec dword[numspcvblleft]
    mov dword[SPC700write],0
    mov dword[SPC700read],0
    mov dword[spc700read],0
.novblch
    mov byte[NMIEnab],01h
    call starthdma
    ; check for VIRQ/HIRQ/NMI
    ProcessIRQStuff
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.virq
    test byte[curexecstate],01h
    jnz .dis658162
    or byte[curexecstate],01h
.dis658162
    mov byte[doirqnext],0
    xor ebx,ebx
    mov ax,[resolutn]
    cmp word[curypos],ax
    jnb .nodrawline
    cmp byte[hdmadelay],1
    jne .virqstuff
.virqstuff
    cmp byte[hdmadelay],0
    je .dohdma2
    dec byte[hdmadelay]
    jmp .nodohdma2
.dohdma2
    call exechdma
.nodohdma2
    cmp word[curypos],1
    jne .nocache2
    call cachevideo
.nocache2
    cmp byte[curblank],0
    jne .nodrawline
    call drawline
.nodrawline
    cmp byte[intrset],1
    jne .nointrset2
    mov byte[intrset],2
.nointrset2
;    sub dh,8
    jmp switchtovirq

.hirq
    mov byte[HIRQNextExe],0
    test byte[INTEnab],10h
    jz .hirqnotokay
    test byte[curexecstate],01h
    jnz .dis658162h
    or byte[curexecstate],01h
.dis658162h
    mov byte[doirqnext],0
    cmp byte[intrset],1
    jne .nointrset2h
    mov byte[intrset],2
.nointrset2h
    test dl,04h
    jnz .irqd
    jmp switchtovirq
.irqd
    mov byte[doirqnext],1
.hirqnotokay
    jmp .nodrawlineh

.returnfromhirq
    mov ax,[resolutn]
    cmp word[curypos],ax
    jnb .nodrawlineh
    cmp byte[hdmadelay],0
    je .dohdma2h
    dec byte[hdmadelay]
    jmp .nodohdma2h
.dohdma2h
    call exechdma
.nodohdma2h
    cmp word[curypos],1
    jne .nocache2h
    call cachevideo
.nocache2h
    cmp byte[curblank],0
    jne .nodrawlineh
    call drawline
.nodrawlineh
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.cheater
    push eax
    push ebx
    push ecx
    push edx
    mov al,[NumCheats]
    mov [.numcheat],al
    xor edx,edx
.anothercheat
    xor ebx,ebx
    xor ecx,ecx
    test byte[cheatdata+edx],5
    jnz .nonormcheat
    test byte[cheatdata+edx-28],80h
    jnz .nonormcheat
    test byte[cheatdata+edx],80h
    jnz .cheatcodereflect
    mov al,[cheatdata+edx+1]
    mov cx,[cheatdata+edx+2]
    mov bl,[cheatdata+edx+4]
    push edx
    call dword near [memtablew8+ebx*4]
    pop edx
    jmp .nonormcheat
.cheatcodereflect
    cmp byte[.numcheat],1
    je .nonormcheat
    mov cx,[cheatdata+edx+2+28]
    mov bl,[cheatdata+edx+4+28]
    push edx
    call dword near [memtabler8+ebx*4]
    pop edx
    mov cx,[cheatdata+edx+2]
    mov bl,[cheatdata+edx+4]
    push edx
    call dword near [memtablew8+ebx*4]
    pop edx
    add edx,28
    dec byte[.numcheat]
.nonormcheat
    add edx,28
    dec byte[.numcheat]
    jnz near .anothercheat
    pop edx
    pop ecx
    pop ebx
    pop eax
    jmp .returncheat

SECTION .bss
.numcheat resb 1
SECTION .text

ALIGN16

NEWSYM pexecs
   mov byte[soundcycleft],30
.sloop
   mov bl,[ebp]
   ; 1260, 10000/12625
   inc ebp
   call dword near [opcjmptab+ebx*4]
   xor ebx,ebx
   dec byte[soundcycleft]
   jnz .sloop
   xor dh,dh
   xor ebx,ebx
   mov bl,[esi]
   inc esi
   jmp execloop.startagain

NEWSYM pexecs2
.sloop
   mov bl,[ebp]
   ; 1260, 10000/12625
   inc ebp
   call dword near [opcjmptab+ebx*4]
   xor ebx,ebx
   dec dword[soundcycleft]
   jnz .sloop
   ret

NEWSYM UpdatePORSCMR
   push ebx
   push eax
   test byte[SfxPOR],10h
   jnz .objmode
   mov al,[SfxSCMR]
   and al,00100100b     ; 4 + 32
   cmp al,4
   je .lines160
   cmp al,32
   je .lines192
   cmp al,36
   je .objmode
   mov eax,[sfx128lineloc]
   jmp .donelines
.lines160
   mov eax,[sfx160lineloc]
   jmp .donelines
.lines192
   mov eax,[sfx192lineloc]
   jmp .donelines
.objmode
   mov eax,[sfxobjlineloc]
.donelines
   mov [sfxclineloc],eax

   mov al,[SfxSCMR]
   and eax,00000011b
   mov bl,[SfxPOR]
   and bl,0Fh
   shl bl,2
   or al,bl
   mov ebx,[PLOTJmpb+eax*4]
   mov eax,[PLOTJmpa+eax*4]
   mov [FxTable+4Ch*4],eax
   mov [FxTableb+4Ch*4],eax
   mov [FxTablec+4Ch*4],eax
   mov [FxTabled+4Ch*4],ebx
   pop eax
   pop ebx
   ret

NEWSYM UpdateSCBRCOLR
   push eax
   push ebx
   mov ebx,[SfxSCBR]
   shl ebx,10
   add ebx,[sfxramdata]
   mov [SCBRrel],ebx
   mov eax,[SfxCOLR]
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
   pop ebx
   pop eax
   ret

NEWSYM UpdateCLSR
   mov dword[NumberOfOpcodes2],350 ; 0FFFFFFFh;350
   test byte[SfxCLSR],01h
   jz .nohighsfx
   mov dword[NumberOfOpcodes2],700 ;700
.nohighsfx
    cmp byte[SFXCounter],1
    je .noyi
    mov dword[NumberOfOpcodes2],0FFFFFFFh
.noyi
   ret

NEWSYM UpdateSFX
   call UpdatePORSCMR
   call UpdatePORSCMR
   call UpdateCLSR
   ret

NEWSYM StartSFX
    push edx
    push esi
    push edi
    push ebp
    xor ebx,ebx
    mov bl,[SfxPBR]
    mov al,[SfxSCMR]
    and bl,7Fh
    cmp bl,70h
    jae .ram
    test al,10h
    jz .noaccess
    jmp .noram
.ram
    test al,08h
    jz .noaccess
.noram
    mov eax,[NumberOfOpcodes2]
    mov [NumberOfOpcodes],eax
    call MainLoop
.noaccess
    pop ebp
    pop edi
    pop esi
    pop edx
    xor ebx,ebx
    xor ecx,ecx
    jmp cpuover.returnfromsfx

NEWSYM StartSFXdebug
    push edx
    push esi
    push edi
    push ebx
    mov bl,[SfxPBR]
    mov al,[SfxSCMR]
    and bl,7Fh
    cmp bl,70h
    jae .ram
    test al,10h
    jz .noaccess
    jmp .noram
.ram
    test al,08h
    jz .noaccess
.noram
    mov dword[NumberOfOpcodes],350 ; 0FFFFFFFh;350
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword[NumberOfOpcodes],700 ;700
.nohighsfx
    cmp byte[SFXCounter],1
    jne .noyi
    mov dword[NumberOfOpcodes],0FFFFFFFFh
.noyi
;    call SFXDebugLoop
.noaccess
    pop ebx
    pop edi
    pop esi
    pop edx
    xor ecx,ecx
    jmp execsingle.returnfromsfx

NEWSYM StartSFXdebugb
    push edx
    push esi
    push edi
    push ebp
    push ebx

   test byte[SfxPOR],10h
   jnz .objmode
   mov al,[SfxSCMR]
   and al,00100100b     ; 4 + 32
   cmp al,4
   je .lines160
   cmp al,32
   je .lines192
   cmp al,36
   je .objmode
   mov eax,[sfx128lineloc]
   jmp .donelines
.lines160
   mov eax,[sfx160lineloc]
   jmp .donelines
.lines192
   mov eax,[sfx192lineloc]
   jmp .donelines
.objmode
   mov eax,[sfxobjlineloc]
.donelines
   mov [sfxclineloc],eax

   mov al,[SfxSCMR]
   and eax,00000011b
   mov bl,[SfxPOR]
   and bl,0Fh
   shl bl,2
   or al,bl
   mov ebx,[PLOTJmpb+eax*4]
   mov eax,[PLOTJmpa+eax*4]
   mov [FxTable+4Ch*4],eax
   mov [FxTableb+4Ch*4],eax
   mov [FxTablec+4Ch*4],eax
   mov [FxTabled+4Ch*4],ebx

   mov ebx,[SfxSCBR]
   shl ebx,10
   add ebx,[sfxramdata]
   mov [SCBRrel],ebx

   mov eax,[SfxCOLR]
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
   xor ebx,ebx

    mov bl,[SfxPBR]
    mov al,[SfxSCMR]
    and bl,7Fh
    cmp bl,70h
    jae .ram
    test al,10h
    jz .noaccess
    jmp .noram
.ram
    test al,08h
    jz .noaccess
.noram
    mov dword[NumberOfOpcodes],420 ;678
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword[NumberOfOpcodes],800 ;678*2
.nohighsfx
    cmp byte[SFXCounter],1
    jne .noyi
    mov dword[NumberOfOpcodes],0FFFFFFFh
.noyi
    call MainLoop
.noaccess
    pop ebx
    pop ebp
    pop edi
    pop esi
    pop edx
    xor ecx,ecx
    jmp execsingle.returnfromsfx

NEWSYM StartSFXret
    test byte[SfxSFR],20h
    jz .endfx
    pushad
    mov bl,[SfxPBR]
    mov al,[SfxSCMR]
    and bl,7Fh
    cmp bl,70h
    jae .ram
    test al,10h
    jz .noaccess
    jmp .noram
.ram
    test al,08h
    jz .noaccess
.noram
    mov dword[NumberOfOpcodes],420 ;678
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword[NumberOfOpcodes],800 ;678*2
.nohighsfx
    mov dword[NumberOfOpcodes],0FFFFFFFFh
    call MainLoop
.noaccess
    popad
.endfx
    ret

;*******************************************************
; Execute a Single 65816 instruction (debugging purpose)
;*******************************************************
NEWSYM execloopdeb
    jmp exitloop2

NEWSYM execsingle

    xor ebx,ebx
    test byte[curexecstate],2
    jz .nosoundb
    sub dword[cycpbl],55
    jnc .skipallspc
    mov eax,[cycpblt]
    mov bl,[ebp]
    add dword[cycpbl],eax
    ; 1260, 10000/12625
    inc ebp
    call dword near [opcjmptab+ebx*4]
    xor ebx,ebx
.skipallspc
.nosoundb

    mov bl,dl
    mov byte[exiter],01h
    mov edi,[tablead+ebx*4]
    mov bl,[esi]
    inc esi
    sub dh,[cpucycle+ebx]
    jc .cpuover
    mov [pdh],dh
    xor dh,dh
    jmp dword near [edi+ebx*4]
.cpuover

    cmp byte[SA1Enable],0
    je near .nosa1
    mov byte[cycpl],150
    test byte[SA1Control],60h
    jnz near .nosa1
    dec esi
    call SA1Swap

    mov bl,[esi]
    inc esi
    mov [pdh],dh
    xor dh,dh
    cmp byte[CurrentExecSA1],17
    jb near cpuover
    mov byte[CurrentExecSA1],0
    mov byte[cycpl],5
    jmp .nosa1
.nosa1

    cmp byte[KeyOnStB],0
    je .nokeyon
    mov al,[KeyOnStB]
    call ProcessKeyOn
.nokeyon
    mov al,[KeyOnStA]
    mov [KeyOnStB],al
    mov byte[KeyOnStA],0
    test byte[SfxSFR],20h
    jnz near StartSFXdebugb
.returnfromsfx
    add dh,[cycpl]
    mov [pdh],dh

    cmp byte[spcon],0
    je .nosound
    call updatetimer
    push ebx
    xor ebx,ebx
    mov bl,dl
    mov edi,[tablead+ebx*4]
    pop ebx
.nosound
    xor dh,dh
    inc word[curypos]
    mov ax,[resolutn]
    inc ax
    cmp word[curypos],ax
    je near .nmi
    mov ax,[totlines]
    cmp word[curypos],ax
    jae near .overy
    ; check for VIRQ/HIRQ/NMI
    ProcessIRQStuff

;    test dl,04h
;    jnz .noirq
;    test byte[INTEnab],20h
;    jz .novirq
;    mov ax,[VIRQLoc]
;    cmp word[curypos],ax
;    je near .virq
;    jmp .noirq
;.novirq
;    test byte[INTEnab],10h
;    jnz near .virq
;.noirq
;    test byte[INTEnab],20h
;    jz .novirq2b
;    mov ax,[VIRQLoc]
;    cmp word[curypos],ax
;    jne .novirq2b
;    cmp byte[intrset],1
;    jne .nointrset2b
;    mov byte[intrset],2
;.nointrset2b
;.novirq2b
    mov ax,[resolutn]
    cmp word[curypos],ax
    jb .drawline
    jmp dword near [edi+ebx*4]

.drawline
    cmp byte[hdmadelay],0
    je .dohdma
    dec byte[hdmadelay]
    jmp .nodohdma
.dohdma
    call exechdma
.nodohdma
    cmp byte[curblank],0
    jne .nodrawlineb
    call drawline
.nodrawlineb
    jmp dword near [edi+ebx*4]

.nmi
    mov byte[irqon],80h
    cmp byte[C4Enable],0
    je .noC4
    call C4VBlank
.noC4
;    mov byte[hdmastartsc],0
    mov byte[joycontren],0
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    jne .novirqz
    inc word[VIRQLoc]
.novirqz

    call ReadInputDevice

    test byte[INTEnab],1
    jz .noresetjoy
    mov eax,[JoyAOrig]
    rol eax,16
    mov [JoyANow],eax
    mov eax,[JoyBOrig]
    rol eax,16
    mov [JoyBNow],eax
    mov eax,[JoyCOrig]
    rol eax,16
    mov [JoyCNow],eax
    mov eax,[JoyDOrig]
    mov [JoyDNow],eax
    mov byte[JoyCRead],0
.noresetjoy

    cmp byte[device2],3
    jne .nolethalen1
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen1
    cmp byte[device2],4
    jne .nolethalen2
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen2

    mov byte[MultiTapStat],80h
    mov byte[NMIEnab],81h
    test byte[INTEnab],80h
    jz .nonmi
    mov byte[curnmi],1
    dec esi
    cmp byte[intrset],1
    jne .nointrset
    mov byte[intrset],2
.nointrset
    jmp switchtonmideb
.nonmi
    cmp byte[intrset],1
    jne .nointrset2w
    mov byte[intrset],2
.nointrset2w
    cmp byte[esi],0CBh
    jne .nowai
    and dl,0FBh
.nowai
    jmp dword near [edi+ebx*4]

.overy
    mov dh,80
    mov word[curypos],0
    xor byte[ppustatus],80h
    mov byte[NMIEnab],01h
    add dword[opcd],170*262
    call cachevideo
    call starthdma

    ProcessIRQStuff

;    test dl,04h
;    jnz .novirq2
;    test byte[INTEnab],20h
;    jz .novirq2
;    mov ax,[VIRQLoc]
;    cmp word[curypos],ax
;    je near .virq
;    mov ax,[VIRQLoc]
;    cmp ax,[totlines]
;    jae .virq
;.novirq2
    jmp dword near [edi+ebx*4]

.virq
    mov ax,[resolutn]
    cmp word[curypos],ax
    jnb .nodrawline
    cmp byte[hdmadelay],0
    je .dohdma2
    dec byte[hdmadelay]
    jmp .nodohdma2
.dohdma2
    call exechdma
.nodohdma2
    cmp byte[curblank],0
    jne .nodrawline
    call drawline
.nodrawline
    dec esi
    cmp byte[intrset],1
    jne .nointrset2
    mov byte[intrset],2
.nointrset2
    jmp switchtovirqdeb
