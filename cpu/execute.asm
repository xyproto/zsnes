;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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

EXTSYM KeyRewind,Voice0Status,UpdateDPage
EXTSYM romdata,DosExit
EXTSYM device2,RawDumpInProgress
EXTSYM KeySaveState,KeyLoadState,KeyQuickExit,KeyQuickLoad,KeyQuickRst
EXTSYM GUIReset,KeyOnStA,KeyOnStB,ProcessKeyOn,KeyQuickClock
EXTSYM KeyQuickSaveSPC,TimerEnable
EXTSYM KeyQuickSnapShot
EXTSYM memtabler8
EXTSYM memtablew8
EXTSYM tablead
EXTSYM tableadc,nextmenupopup,MovieProcessing
EXTSYM wramdata,cycpbl,cycpblt,irqon,spcon
EXTSYM opcjmptab,CheatOn
EXTSYM INTEnab,JoyCRead,NMIEnab,NumCheats,CurrentExecSA1,ReadInputDevice
EXTSYM StartDrawNewGfx,VIRQLoc,cachevideo,cfield,cheatdata,curblank,curnmi
EXTSYM curypos,cycpl,doirqnext,drawline,exechdma,hdmadelay,intrset,newengen
EXTSYM oamaddr,oamaddrs,resolutn,showvideo,starthdma,switchtonmi
EXTSYM switchtovirq,totlines,updatetimer,SA1Swap,SA1DoIRQ,JoyAOrig,JoyANow
EXTSYM JoyBOrig,JoyBNow,JoyCOrig,JoyCNow,JoyDOrig,JoyDNow,JoyEOrig,JoyENow
EXTSYM SA1Message,MultiTapStat,idledetectspc,SA1Control,SA1Enable,SA1IRQEnable
EXTSYM SPC700read,SPC700write,numspcvblleft,spc700idle,SA1IRQExec,ForceNewGfxOff
EXTSYM GUIQuit,IRAM,SA1Ptr,SA1BWPtr,outofmemfix
EXTSYM yesoutofmemory,ProcessMovies,ppustatus
EXTSYM ReturnFromSPCStall,scanlines,MoviePassWaiting
EXTSYM SfxSFR,nosprincr,cpucycle,switchtovirqdeb,switchtonmideb
EXTSYM BackupCVFrame,RestoreCVFrame,xe
EXTSYM KeyInsrtChap,KeyNextChap,KeyPrevChap
EXTSYM EMUPauseKey,INCRFrameKey,MovieWaiting,NoInputRead
EXTSYM AllocatedRewindStates,PauseFrameMode,RestorePauseFrame,BackupPauseFrame
EXTSYM rtoflags,sprcnt,sprtilecnt,endprog
EXTSYM Donextlinecache
EXTSYM StartSFX
EXTSYM StartSFXdebugb

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

    ccallv RestoreCVFrame

    cmp byte[PauseFrameMode],1
    jne .notpauserewind
    ccallv BackupPauseFrame
.notpauserewind

    ccallv UpdateDPage
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

    ccallv BackupCVFrame

.checkrewind
    call ProcessRewind
    ccallv UpdateDPage
.norewinds
    ret

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

; global variables
NEWSYM invalid, db 0
NEWSYM invopcd, db 0
NEWSYM pressed, times 256+128+64 db 0
NEWSYM exiter, db 0
NEWSYM oldhand9o, dd 0
NEWSYM oldhand9s, dw 0
NEWSYM oldhand8o, dd 0
NEWSYM oldhand8s, dw 0
NEWSYM opcd,      dd 0
NEWSYM pdh,       dd 0
NEWSYM pcury,     dd 0
NEWSYM timercount, dd 0
NEWSYM initaddrl, dd 0
NEWSYM NetSent, dd 0
NEWSYM nextframe, dd 0
;NEWSYM newgfxerror, db 'NEED MEMORY FOR GFX ENGINE',0
;NEWSYM newgfxerror2, db 'NEED 320x240 FOR NEW GFX 16B',0
;newgfxerror db 'NEW GFX IN 16BIT IS N/A',0
NEWSYM HIRQCycNext,   dd 0
NEWSYM HIRQNextExe,   db 0


SECTION .text

;*******************************************************
; Int 08h vector
;*******************************************************

%ifdef __MSDOS__
NEWSYM handler8h
    cli
    push ds
    push eax
;    mov ax,0
    mov ax,[cs:dssel]
NEWSYM handler8hseg
    mov ds,ax
    ccallv Game60hzcall
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
NEWSYM execloop
   mov bl,dl
   test byte[curexecstate],2
   jnz .sound
.startagain
   cmp byte[xe],1
   je .notxe
   test dl,1
   jz .notxe
   test byte[INTEnab],0C0h
   jnz .notxe
   sub dh,50h
.notxe
   cmp byte[doirqnext],1
   je .noirq
   cmp byte[SA1IRQEnable],0
   je .noirq
   cmp byte[irqon],0
   je .noirq
   sub dh,12
.noirq
   call dword near [edi+ebx*4]
.cpuover
   jmp cpuover
.sound
   mov edi,[tableadc+ebx*4]
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
    cmp word[curypos],0
    jne .nortoreset
    mov byte[rtoflags],0
.nortoreset
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
;    add dh,10
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
    ccallv Donextlinecache
.nosprcache
    cmp byte[KeyOnStB],0
    je .nokeyon
    mov al,[KeyOnStB]
    ccallv ProcessKeyOn, eax
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
    jz .noStartSFX
    ccallv StartSFX
    xor ebx,ebx
    xor ecx,ecx
.noStartSFX
;    inc dword[numinst]          ;Temporary
    inc word[curypos]
    xor eax,eax
    mov ax,[curypos]
    cmp ax,[resolutn]
    ja .norangeover
    cmp byte[sprtilecnt+eax],34
    jbe .notimeover
    or byte[rtoflags],80h
.notimeover
    cmp byte[sprcnt+eax],32
    jbe .norangeover
    or byte[rtoflags],40h
.norangeover
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
    ccallv exechdma
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
    ccallv exechdma
.nodohdma
    cmp word[curypos],1
    jne .nocache
    ccallv cachevideo
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
    ccallv outofmemfix
.noout

    dec word[curypos]
    mov [tempdh],dh
    xor dh,dh

;    mov al,[SFXIRQFlag]
    mov byte[doirqnext],0

    ccallv exechdma
    ccallv exechdma

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
    jne .notGUIQuit
    ccallv endprog
.notGUIQuit
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
    ccallv ReadInputDevice
.noinputread

    ;Pause and Frame increment
    cmp byte[PauseFrameMode],3
    jne .nopauseframemode3
    ccallv RestorePauseFrame
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
    ccallv BackupPauseFrame
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

    pushad
    call StartDrawNewGfx
    ccallv showvideo
    ccallv cachevideo
    popad

    jmp .nonewgfx
.noemupause

    ;Rewind update must be done before process this frame of movie, so rewind doesn't
    ;back up incremented values (some vars being for the next frame)
    call UpdateRewind

    cmp byte[MovieProcessing],0
    je .noprocmovie
    ccallv ProcessMovies
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
    ccallv DosExit
    ret
.noprocmovie

    cmp byte[device2],3
    jne .nolethalen1
    mov [JoyBNow], dword 0
.nolethalen1
    ;Todo, add second gun...
    cmp byte[device2],4
    jne .nolethalen2
    mov [JoyBNow], dword 0
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
    ccallv showvideo
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
    ccallv idledetectspc
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
    ccallv starthdma
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
    ccallv exechdma
.nodohdma2
    cmp word[curypos],1
    jne .nocache2
    ccallv cachevideo
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
    push edx
    mov edx, esp
    push esi
    mov esi, esp
    ccallv switchtovirq, edx, esi
    pop esi
    pop edx
    xor ebx, ebx
    jmp execloop

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
    push edx
    mov edx, esp
    push esi
    mov esi, esp
    ccallv switchtovirq, edx, esi
    pop esi
    pop edx
    xor ebx, ebx
    jmp execloop
.irqd
    mov byte[doirqnext],1
.hirqnotokay
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
    ccallv ProcessKeyOn, eax
.nokeyon
    mov al,[KeyOnStA]
    mov [KeyOnStB],al
    mov byte[KeyOnStA],0
    test byte[SfxSFR],20h
    jz .noStartSFXdebugb
    ccallv StartSFXdebugb
    xor ecx,ecx
.noStartSFXdebugb
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
    ccallv exechdma
.nodohdma
    cmp byte[curblank],0
    jne .nodrawlineb
    call drawline
.nodrawlineb
    jmp dword near [edi+ebx*4]

.nmi
    mov byte[irqon],80h
;    mov byte[hdmastartsc],0
    mov byte[joycontren],0
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    jne .novirqz
    inc word[VIRQLoc]
.novirqz

    ccallv ReadInputDevice

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
    mov [JoyBNow], dword 0
.nolethalen1
    cmp byte[device2],4
    jne .nolethalen2
    mov [JoyBNow], dword 0
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
    ccallv cachevideo
    ccallv starthdma

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
    ccallv exechdma
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
