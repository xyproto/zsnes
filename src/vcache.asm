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

EXTSYM MessageOn,MsgCount,Msgptr,Voice0Disable,Voice0Status,Voice1Disable
EXTSYM Voice1Status,Voice2Disable,Voice2Status,Voice3Disable,Voice3Status
EXTSYM Voice4Disable,Voice4Status,Voice5Disable,Voice5Status,Voice6Disable
EXTSYM Voice6Status,Voice7Disable,Voice7Status,bgcmsung,bgmode
EXTSYM cgmod,disableeffects,frameskip,frskipper,current_zst
EXTSYM maxbr,modeused,mousexloc,mouseyloc,newengen,KeyRTRCycle
EXTSYM nextdrawallng,pal16b,pal16bxcl,pressed,prevbright,prevpal
EXTSYM scaddsngb,scaddtngb,scaddtngbx,scfbl,scrndis,sprprdrn,t1cc
EXTSYM vidbright,vidbuffer,vidbufferofsa,vidmemch2,MZTForceRTR
EXTSYM GUIRClick,MousePRClick,ngmsdraw,cvidmode,fulladdtab
EXTSYM KeyDisableSC0,KeyDisableSC1,KeyDisableSC2,KeyDisableSC3,KeyDisableSC4
EXTSYM KeyDisableSC5,KeyDisableSC6,KeyDisableSC7,KeyFastFrwrd,SRAMSave5Sec
EXTSYM KeyBGDisble0,KeyBGDisble1,KeyBGDisble2,KeyBGDisble3,KeySprDisble
EXTSYM KeyResetAll,KeyWinDisble,KeyNewGfxSwt,KeyOffsetMSw
EXTSYM KeyStateSlc0,KeyStateSlc1,KeyStateSlc2,KeyStateSlc3,KeyStateSlc4
EXTSYM KeyStateSlc5,KeyStateSlc6,KeyStateSlc7,KeyStateSlc8,KeyStateSlc9
EXTSYM KeyIncStateSlot,KeyDecStateSlot,KeyUsePlayer1234,maxskip,DSPMem
EXTSYM FastFwdToggle,SaveSramData,ngextbg,Mode7HiRes,Check60hz
EXTSYM Get_MouseData,Get_MousePositionDisplacement,scanlines
EXTSYM romispal,MusicRelVol,MusicVol,WDSPReg0C,WDSPReg1C,KeySlowDown
EXTSYM KeyFRateDown,KeyFRateUp,KeyVolUp,KeyVolDown,KeyDisplayFPS
EXTSYM FPSOn,pl12s34,bg1ptr,bg2ptr,bg3ptr,bg4ptr,cachebg1,resolutn,curypos
EXTSYM oamram,objhipr,objptr,objptrn,objsize1,objsize2,spritetablea,sprleftpr
EXTSYM sprlefttot,vcache4b,objadds1,objadds2,objmovs1,objmovs2,tltype4b
EXTSYM vidmemch4,vram,bgptr,bgptrc,bgptrd,curtileptr,vcache2b,vcache8b,vidmemch8
EXTSYM offsetmshl,NextLineCache,tltype2b,tltype8b,objwlrpos
EXTSYM EmuSpeed,SDRatio,FFRatio,DisplayBatteryStatus,lhguimouse,SwapMouseButtons
EXTSYM KeyResetSpeed,KeyEmuSpeedUp,KeyEmuSpeedDown,KeyDisplayBatt,EMUPause
EXTSYM device1,device2,snesinputdefault1,snesinputdefault2
EXTSYM KeyExtraEnab1,KeyExtraEnab2,cycleinputdevice1,cycleinputdevice2,MouseDis
EXTSYM KeyIncreaseGamma,KeyDecreaseGamma,gammalevel,gammalevel16b
EXTSYM RawDumpInProgress

%ifndef NO_DEBUGGER
EXTSYM debuggeron
%else
EXTSYM SPCSave
%endif

; Process stuff & Cache sprites

SECTION .data
ALIGN32

NEWSYM fskipped,     db 0
NEWSYM sprprifix,    db 1
NEWSYM OMBGTestVal, dd 0
NEWSYM ngptrdat2, dd 0
NEWSYM ofshvaladd, dd 0
NEWSYM ofsmtptrs, dd 0
NEWSYM ofsmcptr2, dd 0
NEWSYM sramb4save, dd 0
NEWSYM hiresstuff, dd 0
NEWSYM overalltimer, dd 0

SECTION .text

%macro stateselcomp 2
    mov eax,[%1]
    test byte[pressed+eax],1
    je %%nostsl
    mov byte[pressed+eax],2
    mov eax,[current_zst]
    mov cl,10
    div cl
    mov ah,%2
    add al,'0'
    add ah,'0'
    mov [sselm+11],ax
    sub al,'0'
    mul cl
    add al,%2
    mov [current_zst],eax
    mov dword[Msgptr],sselm
    mov eax,[MsgCount]
    mov [MessageOn],eax
%%nostsl
%endmacro

%macro soundselcomp 4
    mov eax,[%1]
    test byte[pressed+eax],1
    je %%nosdis
    xor byte[%2],01h
    mov byte[%3],0
    mov byte[pressed+eax],2
    mov byte[sndchena+9],%4
    mov byte[sndchdis+9],%4
    mov dword[Msgptr],sndchena
    test byte[%2],01h
    jnz %%sen
    mov dword[Msgptr],sndchdis
%%sen
    mov eax,[MsgCount]
    mov [MessageOn],eax
%%nosdis
%endmacro

UpdateVolume:
    pushad
    xor eax,eax
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

    mov al,[DSPMem+0Ch]
    call WDSPReg0C
    mov al,[DSPMem+1Ch]
    call WDSPReg1C

    mov dword[vollv+14],20202020h
    mov edx,vollv+15
    mov al,[MusicRelVol]
    cmp al,100
    jne .no100
    mov byte[edx],49
    mov byte[edx+1],48
    sub al,100
    add edx,2
.no100
    xor ah,ah
    mov bl,10
    div bl
    cmp al,0
    je .no10
    add al,48
    mov [edx],al
    inc edx
.no10
    add ah,48
    mov [edx],ah
    mov dword[Msgptr],vollv
    mov eax,[MsgCount]
    mov [MessageOn],eax
    popad
    ret

ClockCounter:
    inc dword[overalltimer]
    cmp byte[romispal],0
    jne .dopal
    cmp dword[overalltimer],60
    jne .notimer
    sub dword[overalltimer],60
    jmp .notimer
.dopal
    cmp dword[overalltimer],50
    jne .notimer
    sub dword[overalltimer],50
.notimer
    test byte[pressed+2Eh],1
    jz .noclear
    mov dword[overalltimer],0
.noclear
    ret

SECTION .bss
NEWSYM FastForwardLock, resb 1
NEWSYM SlowDownLock, resb 1
NEWSYM CSprWinPtr, resd 1
NEWSYM SloMo, resb 1  ; number of extra times to draw a frame
section .text

NEWSYM cachevideo
    mov byte[NextLineCache],0
    mov dword[objwlrpos],0FFFFFFFFh
    mov dword[CSprWinPtr],0
    mov byte[pressed],0
    mov dword[bgcmsung],0
    mov dword[modeused],0
    mov dword[modeused+4],0
    mov dword[scaddsngb],0
    mov dword[scaddtngb],0
    mov dword[sprprdrn],0
    mov dword[ngmsdraw],0
    mov dword[ngextbg],0
    mov dword[scaddtngbx],0FFFFFFFFh
    mov byte[hiresstuff],0
    mov byte[Mode7HiRes],0

    call ClockCounter

    cmp byte[scanlines],1
    je .nohires
    cmp byte[cvidmode],9
    je .yeshires
    cmp byte[cvidmode],15
    jne .nohires
.yeshires
    mov byte[Mode7HiRes],1
.nohires
    mov dword[scfbl],1
    mov al,[vidbright]
    mov [maxbr],al
    mov byte[cgmod],1
    xor al,al
    mov [curblank],al
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],0
    je .nodebugger
    mov byte[curblank],40h
    mov al,40h
    jmp .nofrskip
.nodebugger
%else
    cmp byte[SPCSave],1
    jne .nospcsave
    mov byte[curblank],40h
    mov al,40h
    jmp .nofrskip
.nospcsave
%endif

    cmp dword[sramb4save],0
    je .nofocussave
    cmp byte[SRAMSave5Sec],0
    je .nofocussaveb
    dec dword[sramb4save]
    cmp dword[sramb4save],1
    jne .nofocussave
    pushad
    call SaveSramData
    popad
    jmp .nofocussave
.nofocussaveb
    mov dword[sramb4save],0
.nofocussave

    ; if emulation paused, don't alter timing
    mov ax,1
    cmp byte[EMUPause],1
    je near .ttldone
    ; fast forward goes over all other throttles
    ; don't fast forward while dumping a movie
    cmp byte[RawDumpInProgress],1
    je .ffskip
    cmp byte[FastFwdToggle],0
    jne .ffmode2
    mov eax,[KeyFastFrwrd]
    test byte[pressed+eax],1
    jnz near .fastfor
    jmp .ffskip
.ffmode2
    mov eax,[KeyFastFrwrd]
    test byte[pressed+eax],1
    jz .nofastfor
    mov byte[pressed+eax],2
    xor byte[FastForwardLock],1
.nofastfor
    cmp byte[FastForwardLock],1
    je near .fastfor
.ffskip
    ; next up, check for slowdown
    cmp byte[FastFwdToggle],0
    jne .sdmode2
    mov eax,[KeySlowDown]
    test byte[pressed+eax],1
    jnz near .slowdwn
    jmp .sdskip
.sdmode2
    mov eax,[KeySlowDown]
    test byte[pressed+eax],1
    jz .noslowdwn
    mov byte[pressed+eax],2
    xor byte[SlowDownLock],1
.noslowdwn
    cmp byte[SlowDownLock],1
    je near .slowdwn
    jmp .sdskip
.slowdwn
    mov al,[SDRatio]          ; 0-28
    inc al                    ; 1-29
    mov [SloMo],al            ; /2-/30 slowmotion
    jmp .throttleskip
.sdskip
    ; now we can look at emuspeed
    cmp byte[EmuSpeed],30     ; 0-28 slow, 29 normal, 30-58 skip
    jb .noskipping
    inc byte[frskipper]
    push ebx
    mov bl,[EmuSpeed]
    sub bl,29                 ; 30-58 -> 1-29 frames to skip, 2x-30x speed
    jmp .fastforb
.noskipping
    mov byte[SloMo],29
    mov al,[EmuSpeed]
    sub byte[SloMo],al        ; 0-29 -> repeat 29-0 times, /30-1x speed
.throttleskip
    mov ax,[SloMo]
    inc ax                    ; total times frame is drawn
.ttldone

    cmp byte[frameskip],0
    jne near .frameskip
    cmp word[t1cc],ax
    jae .skipt1ccc
.noskip
    push eax
    call Check60hz
    pop eax
    cmp word[t1cc],ax
    jb .noskip
.skipt1ccc
    sub word[t1cc],ax
    cmp word[t1cc],ax
    jb .noskip2
    mov byte[curblank],40h
    inc byte[fskipped]
    mov cl,[maxskip]
    cmp byte[fskipped],cl
    jbe near .nofrskip
    mov word[t1cc],0
    mov byte[curblank],0
.noskip2
    mov byte[fskipped],0
    jmp .nofrskip
.fastfor
    inc byte[frskipper]
    push ebx
    mov bl,[FFRatio]      ; 0-28
    inc bl                    ; 1-29, 2x-30x fastmotion
    jmp .fastforb
.frameskip
    inc byte[frskipper]
    push ebx
    mov bl,[frameskip]
.fastforb
    cmp byte[frskipper],bl
    pop ebx
    jae .nofrskip
    mov byte[curblank],40h
    jmp .frskip
.nofrskip
    mov byte[frskipper],0
.frskip
    push ebx
    push esi
    push edi
    push edx
    cmp byte[MouseDis],1
    je .noclick
    cmp byte[GUIRClick],0
    je .noclick
    cmp byte[device1],0
    jne .noclick
    cmp byte[device2],0
    jne .noclick
    call Get_MouseData
    cmp byte[lhguimouse],1
    jne .notlefthanded
    call SwapMouseButtons
.notlefthanded
    test bx,02h
    jz .norclick
    cmp byte[MousePRClick],0
    jne .noclick
    mov byte[pressed+1],1
.norclick
    mov byte[MousePRClick],0
.noclick
    ; disable all necessary backgrounds
    mov eax,[KeyBGDisble0]
    test byte[pressed+eax],1
    je .nodis1
    xor byte[scrndis],01h
    mov byte[pressed+eax],2
    mov dword[Msgptr],bg1layena
    test byte[scrndis],01h
    jz .en1
    mov dword[Msgptr],bg1laydis
.en1
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis1
    mov eax,[KeyBGDisble1]
    test byte[pressed+eax],1
    je .nodis2
    xor byte[scrndis],02h
    mov byte[pressed+eax],2
    mov dword[Msgptr],bg2layena
    test byte[scrndis],02h
    jz .en2
    mov dword[Msgptr],bg2laydis
.en2
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis2
    mov eax,[KeyBGDisble2]
    test byte[pressed+eax],1
    je .nodis3
    xor byte[scrndis],04h
    mov byte[pressed+eax],2
    mov dword[Msgptr],bg3layena
    test byte[scrndis],04h
    jz .en3
    mov dword[Msgptr],bg3laydis
.en3
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis3
    mov eax,[KeyBGDisble3]
    test byte[pressed+eax],1
    je .nodis4
    xor byte[scrndis],08h
    mov byte[pressed+eax],2
    mov dword[Msgptr],bg4layena
    test byte[scrndis],08h
    jz .en4
    mov dword[Msgptr],bg4laydis
.en4
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis4
    mov eax,[KeySprDisble]
    test byte[pressed+eax],1
    je .nodis5
    xor byte[scrndis],10h
    mov byte[pressed+eax],2
    mov dword[Msgptr],sprlayena
    test byte[scrndis],10h
    jz .en5
    mov dword[Msgptr],sprlaydis
.en5
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis5
    mov eax,[KeyEmuSpeedDown]
    test byte[pressed+eax],1
    jz .nospeeddown
    mov byte[pressed+eax],2
    cmp byte[EmuSpeed],0
    je .nospeeddown
    dec byte[EmuSpeed]
.nospeeddown
    mov eax,[KeyEmuSpeedUp]
    test byte[pressed+eax],1
    jz .nospeedup
    mov byte[pressed+eax],2
    cmp byte[EmuSpeed],58
    je .nospeedup
    inc byte[EmuSpeed]
.nospeedup
    mov eax,[KeyResetSpeed]
    test byte[pressed+eax],1
    jz .nospeedreset
    mov byte[pressed+eax],2
    mov byte[EmuSpeed],29
.nospeedreset
    mov eax,[KeyResetAll]
    test byte[pressed+eax],1
    je near .nodis6
    mov byte[pressed+eax],2
    mov byte[Voice0Disable],1
    mov byte[Voice1Disable],1
    mov byte[Voice2Disable],1
    mov byte[Voice3Disable],1
    mov byte[Voice4Disable],1
    mov byte[Voice5Disable],1
    mov byte[Voice6Disable],1
    mov byte[Voice7Disable],1
    mov byte[scrndis],0
    mov byte[disableeffects],0
    mov byte[osm2dis],0
    mov byte[EmuSpeed],29
    mov al,[snesinputdefault1]
    mov [device1],al
    mov al,[snesinputdefault2]
    mov [device2],al
    mov dword[Msgptr],panickeyp
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis6
    mov eax,[KeyRTRCycle]
    test byte[pressed+eax],1
    je near .nortrcycle
    mov byte[pressed+eax],2
    inc byte[MZTForceRTR]
    cmp byte[MZTForceRTR],3
    jne .notrtrwrap
    mov byte[MZTForceRTR],0
    mov dword[Msgptr],mztrtr0
    jmp .mztrtrmesg
.notrtrwrap
    cmp byte[MZTForceRTR],1
    jne .nomztrtr1
    mov dword[Msgptr],mztrtr1
    jmp .mztrtrmesg
.nomztrtr1
    mov dword[Msgptr],mztrtr2
.mztrtrmesg
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nortrcycle
    mov eax,[KeyExtraEnab1]
    test byte[pressed+eax],1
    je near .nodisd1
    mov byte[pressed+eax],2
    pushad
    call cycleinputdevice1
    popad
    mov dword[Msgptr],snesmousep0
    cmp byte[device1],1
    jne .nom11
    mov dword[Msgptr],snesmousep1
.nom11
    mov eax,[MsgCount]
    mov [MessageOn],eax
    call Get_MousePositionDisplacement
.nodisd1
    mov eax,[KeyExtraEnab2]
    test byte[pressed+eax],1
    je near .nodisd2
    mov byte[pressed+eax],2
    pushad
    call cycleinputdevice2
    popad
    mov dword[Msgptr],snesmousep0
    cmp byte[device2],1
    jne .nom21
    mov dword[Msgptr],snesmousep2
.nom21
    cmp byte[device2],2
    jne .nom22
    mov dword[Msgptr],snesss
    mov word[mousexloc],128
    mov word[mouseyloc],112
.nom22
    cmp byte[device2],3
    jne .nom23
    mov dword[Msgptr],snesle1
.nom23
    cmp byte[device2],4
    jne .nom24
    mov dword[Msgptr],snesle2
.nom24
    mov eax,[MsgCount]
    mov [MessageOn],eax
    call Get_MousePositionDisplacement
.nodisd2
    mov eax,[KeyNewGfxSwt]
    test byte[pressed+eax],1
    je near .nodis8
    mov byte[pressed+eax],2
    mov byte[prevbright],16
    xor byte[newengen],1
    mov dword[Msgptr],ngena
    cmp byte[newengen],1
    je .disng
    mov dword[Msgptr],ngdis
.disng
    mov eax,[MsgCount]
    mov [MessageOn],eax
    mov dword[nextdrawallng],1
    mov edi,vidmemch2
    mov ecx,1024*3
    mov eax,01010101h
    rep stosd
    mov edi,pal16b
    mov ecx,256
    xor eax,eax
    rep stosd
    mov edi,prevpal
    mov ecx,128
    rep stosd
    mov eax,0FFFFh
    cmp byte[newengen],1
    jne .noneweng
    mov eax,0FFFFFFFFh
.noneweng
    mov edi,pal16bxcl
    mov ecx,256
    rep stosd
    pushad
    call genfulladdtab
    popad
.yesng
.disng2
.nodis8
    mov eax,[KeyWinDisble]
    test byte[pressed+eax],1
    je .nodis9
    mov byte[pressed+eax],2
    xor byte[disableeffects],1
    mov dword[Msgptr],windissw
    cmp byte[disableeffects],1
    je .disablew
    mov dword[Msgptr],winenasw
.disablew
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis9
    mov eax,[KeyOffsetMSw]
    test byte[pressed+eax],1
    je .nodis10
    mov byte[pressed+eax],2
    xor byte[osm2dis],1
    mov dword[Msgptr],ofsdissw
    cmp byte[osm2dis],1
    je .disableom
    mov dword[Msgptr],ofsenasw
.disableom
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodis10
    mov eax,[KeyVolUp]
    test byte[pressed+eax],1
    je .novolup
    cmp byte[MusicRelVol],100
    jae .novolup
    inc byte[MusicRelVol]
    call UpdateVolume
.novolup
    mov eax,[KeyVolDown]
    test byte[pressed+eax],1
    je .novoldown
    cmp byte[MusicRelVol],0
    je .novoldown
    dec byte[MusicRelVol]
    call UpdateVolume
.novoldown
    mov eax,[KeyFRateUp]
    test byte[pressed+eax],1
    je .nofrup
    mov byte[pressed+eax],2
    cmp byte[frameskip],10
    je .nofrup
    mov byte[FPSOn],0
    inc byte[frameskip]
    mov al,[frameskip]
    add al,47
    mov [frlev+18],al
    mov dword[Msgptr],frlev
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nofrup
    mov eax,[KeyFRateDown]
    test byte[pressed+eax],1
    je .nofrdown
    mov byte[pressed+eax],2
    cmp byte[frameskip],0
    je .nofrdown
    dec byte[frameskip]
    cmp byte[frameskip],0
    je .min
    mov al,[frameskip]
    add al,47
    mov [frlev+18],al
    mov dword[Msgptr],frlev
    jmp .nomin
.min
    mov dword[Msgptr],frlv0
    mov word[t1cc],0
.nomin
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nofrdown
    mov eax,[KeyDisplayBatt]
    test byte[pressed+eax],1
    je .nodisplaybatt
    mov byte[pressed+eax],2
    pushad
    call DisplayBatteryStatus
    popad
.nodisplaybatt
    mov eax,[KeyIncreaseGamma]
    test byte[pressed+eax],1
    je .noincgamma
    mov byte[pressed+eax],2
    cmp byte[gammalevel],15
    jge .noincgamma
    inc byte[gammalevel]
    mov al,[gammalevel]
    mov [gammalevel16b],al
    shr byte[gammalevel16b],1
    cmp byte[gammalevel],10
    jl .gammanot10
    mov byte[gammamsg+13],'1'
    sub al,10
    jmp .postgamma
.gammanot10
    mov byte[gammamsg+13],' '
.postgamma
    add al,'0'
    mov [gammamsg+14],al
    mov dword[Msgptr],gammamsg
    mov eax,[MsgCount]
    mov [MessageOn],eax
.noincgamma
    mov eax,[KeyDecreaseGamma]
    test byte[pressed+eax],1
    je .nodecgamma
    mov byte[pressed+eax],2
    cmp byte [gammalevel],0
    je .nodecgamma
    dec byte[gammalevel]
    mov eax,[gammalevel]
    mov [gammalevel16b],eax
    shr byte[gammalevel16b],1
    cmp byte[gammalevel],10
    jl .gamma2not10
    mov byte[gammamsg+13],'1'
    sub al,10
    jmp .postgamma2
.gamma2not10
    mov byte[gammamsg+13],' '
.postgamma2
    add al,'0'
    mov [gammamsg+14],al
    mov dword[Msgptr],gammamsg
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nodecgamma
    mov eax,[KeyDisplayFPS]
    test byte[pressed+eax],1
    je .nodisplayfps
    mov byte[pressed+eax],2
    cmp byte[frameskip],0
    jne .nodisplayfps
    xor byte[FPSOn],1
.nodisplayfps

    ; do state selects
    stateselcomp KeyStateSlc0,0
    stateselcomp KeyStateSlc1,1
    stateselcomp KeyStateSlc2,2
    stateselcomp KeyStateSlc3,3
    stateselcomp KeyStateSlc4,4
    stateselcomp KeyStateSlc5,5
    stateselcomp KeyStateSlc6,6
    stateselcomp KeyStateSlc7,7
    stateselcomp KeyStateSlc8,8
    stateselcomp KeyStateSlc9,9
    mov eax,[KeyStateSlc0]
    test byte[pressed+eax],1
    je .nostsl0
    mov byte[pressed+eax],2
    mov byte[sselm+11],'0'
    mov dword[Msgptr],sselm
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nostsl0

    mov eax,[KeyIncStateSlot]
    test byte[pressed+eax],1
    je near .noincstateslot
    mov byte[pressed+eax],2
    mov eax,[current_zst]
    inc eax
    cmp eax,100
    jne .notend
    xor eax,eax
.notend
    mov [current_zst],eax
    mov dl,10
    div dl
    add ah,'0'
    add al,'0'
    mov [sselm+11],ax
    mov dword[Msgptr],sselm
    mov eax,[MsgCount]
    mov [MessageOn],eax
    xor dx,dx
.noincstateslot

    mov eax,[KeyDecStateSlot]
    test byte[pressed+eax],1
    je near .nodecstateslot
    mov byte[pressed+eax],2
    mov eax,[current_zst]
    cmp eax,0
    jne .notstart
    mov eax,100
.notstart
    dec eax
    mov [current_zst],eax
    mov dl,10
    div dl
    add ah,'0'
    add al,'0'
    mov [sselm+11],ax
    mov dword[Msgptr],sselm
    mov eax,[MsgCount]
    mov [MessageOn],eax
    xor dx,dx
.nodecstateslot

    mov eax,[KeyUsePlayer1234]
    test byte[pressed+eax],1
    je .nousepl1234
    mov byte[pressed+eax],2
    xor byte[pl12s34],1
    mov dword[Msgptr],pluse1234en
    cmp byte[pl12s34],1
    je .usepl1234
    mov dword[Msgptr],pluse1234dis
.usepl1234
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nousepl1234

    ; do sound disables
    soundselcomp KeyDisableSC0,Voice0Disable,Voice0Status,'1'
    soundselcomp KeyDisableSC1,Voice1Disable,Voice1Status,'2'
    soundselcomp KeyDisableSC2,Voice2Disable,Voice2Status,'3'
    soundselcomp KeyDisableSC3,Voice3Disable,Voice3Status,'4'
    soundselcomp KeyDisableSC4,Voice4Disable,Voice4Status,'5'
    soundselcomp KeyDisableSC5,Voice5Disable,Voice5Status,'6'
    soundselcomp KeyDisableSC6,Voice6Disable,Voice6Status,'7'
    soundselcomp KeyDisableSC7,Voice7Disable,Voice7Status,'8'

    cmp byte[curblank],0h
    jne near yesblank
    ; Swap video addresses
;    mov ebx,[vidbuffer]
;    cmp ebx,[vidbufferofsa]
;    je .useb
    mov ebx,[vidbufferofsa]
    mov [vidbuffer],ebx

NEWSYM docache
    xor ebx,ebx
    mov bl,[bgmode]
    shl bl,2
    add ebx,colormodedef
    mov [colormodeofs],ebx
    xor ebx,ebx
    mov bl,[bgmode]
    mov al,[colormodedef+ebx*4]
    mov [curcolbg1],al
    mov ah,[colormodedef+ebx*4+1]
    mov [curcolbg2],ah
    mov al,[colormodedef+ebx*4]
    mov [curcolbg3],al
    mov ah,[colormodedef+ebx*4+1]
    mov [curcolbg4],ah
    mov ax,[bg1ptr]
    mov [curbgofs1],ax
    mov ax,[bg2ptr]
    mov [curbgofs2],ax
    mov ax,[bg3ptr]
    mov [curbgofs3],ax
    mov ax,[bg4ptr]
    mov [curbgofs4],ax
    push es
    mov ax,ds
    mov es,ax
    ; clear # of sprites & bg cache
    mov edi,cachebg1
    mov ecx,64*5+16*4
    xor eax,eax
    rep stosd
;    cmp byte[sprprifix],0
;    je .nosprfix
;    mov edi,sprlefttotb
;    mov ecx,64*3
;    xor eax,eax
;    rep stosd
;.nosprfix
    ; do sprites
;    test word[scrnon],1010h
;    jz .nosprites
    test byte[scrndis],10h
    jnz .nosprites
    call cachesprites
    call processsprites
;    mov byte[sprprncache],0
;    cmp byte[sprprifix],0
;    je .nosprites
;    call processspritesb

.nosprites
    ; fill background with 0's unless 16-bit/new graphics engine mode is on
    xor ecx,ecx
    pop es
NEWSYM yesblank
    pop edx
    pop edi
    pop esi
    pop ebx
    ret

SECTION .data
NEWSYM osm2dis,      db 0
NEWSYM colormodedef, db 1,1,1,1, 2,2,1,0, 2,2,0,0, 3,2,0,0,
               db 3,1,0,0, 2,1,0,0, 2,0,0,0, 0,0,0,0
NEWSYM colormodeofs, dd 0
NEWSYM curblank,     db 80h             ; current blank state (40h = skip fill)
NEWSYM addr2add,     dd 0
;cachebg1    times 64 db 0
;cachebg2    times 64 db 0
;cachebg3    times 64 db 0
;cachebg4    times 64 db 0
;sprlefttot  times 256 db 0     ; total sprites left
;sprleftpr   times 256 db 0     ; sprites left for priority 0
;sprleftpr1  times 256 db 0     ; sprites left for priority 1
;sprleftpr2  times 256 db 0     ; sprites left for priority 2
;sprleftpr3  times 256 db 0     ; sprites left for priority 3
;spritetable times 256*512 db 0  ; sprite table (flip/pal/xloc/vbufptr)38*7
NEWSYM curbgofs1,   dw 0
NEWSYM curbgofs2,   dw 0
NEWSYM curbgofs3,   dw 0
NEWSYM curbgofs4,   dw 0
NEWSYM curcolbg1,   db 0
NEWSYM curcolbg2,   db 0
NEWSYM curcolbg3,   db 0
NEWSYM curcolbg4,   db 0
NEWSYM panickeyp,   db 'ALL SWITCHES NORMAL',0
NEWSYM mztrtr0, db 'LOAD MZT MODE - OFF',0
NEWSYM mztrtr1, db 'LOAD MZT MODE - RECORD',0
NEWSYM mztrtr2, db 'LOAD MZT MODE - REPLAY',0
NEWSYM snesmousep0, db 'EXTRA DEVICES DISABLED',0
NEWSYM snesmousep1, db 'MOUSE ENABLED IN PORT 1',0
NEWSYM snesmousep2, db 'MOUSE ENABLED IN PORT 2',0
NEWSYM snesss,      db 'SUPER SCOPE ENABLED',0
NEWSYM snesle1,      db '1 JUSTIFIER ENABLED',0
NEWSYM snesle2,      db '2 JUSTIFIERS ENABLED',0
NEWSYM windissw,    db 'WINDOWING DISABLED',0
NEWSYM winenasw,    db 'WINDOWING ENABLED',0
NEWSYM ofsdissw,    db 'OFFSET MODE DISABLED',0
NEWSYM ofsenasw,    db 'OFFSET MODE ENABLED',0
NEWSYM ngena, db 'NEW GFX ENGINE ENABLED',0
NEWSYM ngdis, db 'NEW GFX ENGINE DISABLED',0
NEWSYM sselm, db 'STATE SLOT  0 SELECTED',0
NEWSYM vollv, db 'VOLUME LEVEL :    ',0
NEWSYM frlev, db 'FRAME SKIP SET TO  ',0
NEWSYM frlv0, db 'AUTO FRAMERATE ENABLED',0
NEWSYM pluse1234en, db 'USE PLAYER 1/2 with 3/4 ON',0
NEWSYM pluse1234dis, db 'USE PLAYER 1/2 with 3/4 OFF',0
sndchena db 'SOUND CH   ENABLED',0
sndchdis db 'SOUND CH   DISABLED',0
bg1layena db 'BG1 LAYER ENABLED',0
bg2layena db 'BG2 LAYER ENABLED',0
bg3layena db 'BG3 LAYER ENABLED',0
bg4layena db 'BG4 LAYER ENABLED',0
sprlayena db 'SPRITE LAYER ENABLED',0
bg1laydis db 'BG1 LAYER DISABLED',0
bg2laydis db 'BG2 LAYER DISABLED',0
bg3laydis db 'BG3 LAYER DISABLED',0
bg4laydis db 'BG4 LAYER DISABLED',0
sprlaydis db 'SPRITE LAYER DISABLED',0
gammamsg db 'GAMMA LEVEL:   ',0
section .text

;*******************************************************
; Process Sprites
;*******************************************************
; Use oamram for object table
NEWSYM processsprites
;    cmp byte[cbitmode],1
;    je .skipnewspr
;    cmp byte[newengen],1
;    je .skipnewspr
    cmp byte[sprprifix],0
    jne near processspritesb
.skipnewspr
    ; set obj pointers
    cmp byte[objsize1],1
    jne .16dot1
    mov ebx,.process8x8sprite
    mov [.size1ptr],ebx
    jmp .fin1
.16dot1
    cmp byte[objsize1],4
    jne .32dot1
    mov ebx,.process16x16sprite
    mov [.size1ptr],ebx
    jmp .fin1
.32dot1
    cmp byte[objsize1],16
    jne .64dot1
    mov ebx,.process32x32sprite
    mov [.size1ptr],ebx
    jmp .fin1
.64dot1
    mov ebx,.process64x64sprite
    mov [.size1ptr],ebx
.fin1
    cmp byte[objsize2],1
    jne .16dot2
    mov ebx,.process8x8sprite
    mov [.size2ptr],ebx
    jmp .fin2
.16dot2
    cmp byte[objsize2],4
    jne .32dot2
    mov ebx,.process16x16sprite
    mov [.size2ptr],ebx
    jmp .fin2
.32dot2
    cmp byte[objsize2],16
    jne .64dot2
    mov ebx,.process32x32sprite
    mov [.size2ptr],ebx
    jmp .fin2
.64dot2
    mov ebx,.process64x64sprite
    mov [.size2ptr],ebx
.fin2
    ; set pointer adder
    xor eax,eax
    xor ebx,ebx
    mov al,[objhipr]
    shl ax,2
    mov ebx,eax
    sub bx,4
    and bx,01FCh
    mov dword[addr2add],0
    mov byte[.prileft],4
    mov byte[.curpri],0
    ; do 1st priority
    mov ecx,[objptr]
    shl ecx,1
    mov [.objvramloc],ecx
    mov ecx,[objptrn]
    sub ecx,[objptr]
    shl ecx,1
    mov [.objvramloc2],ecx
    push ebp
    mov ebp,[spritetablea]
.startobject
    mov byte[.objleft],128
.objloop
    xor ecx,ecx
    mov cx,[oamram+ebx+2]
    mov dl,ch
    shr dl,4
    and dl,03h
    cmp dl,[.curpri]
    jne near .nextobj
    ; get object information
    push ebx
    mov dl,[oamram+ebx+1]       ; y
    inc dl
    ; set up pointer to esi
    mov dh,ch
    and ch,01h
    shr dh,1
    shl ecx,6
    add ecx,[.objvramloc]
    test byte[oamram+ebx+3],01h
    jz .noloc2
    add ecx,[.objvramloc2]
.noloc2
    and ecx,01FFFFh
    add ecx,[vcache4b]
    mov esi,ecx
    ; get x
    mov al,[oamram+ebx]         ; x
    ; get double bits
    mov cl,bl
    shr ebx,4           ; /16
    shr cl,1
    and cl,06h
    mov ah,[oamram+ebx+512]
    shr ah,cl
    and ah,03h
    mov ch,ah
    and ch,01h
    mov cl,al
    ; process object
    ; esi = pointer to 8-bit object, dh = stats (1 shifted to right)
    ; cx = x position, dl = y position
    cmp cx,384
    jb .noadder
    add cx,65535-511
.noadder
    cmp cx,256
    jge .returnfromptr
    cmp cx,-64
    jle .returnfromptr
    test ah,02h
    jz .size1
    jmp dword near [.size2ptr]
.size1
    jmp dword near [.size1ptr]
.returnfromptr
    pop ebx
    ; next object
.nextobj
    sub bx,4
    and bx,01FCh
    dec byte[.objleft]
    jnz near .objloop
    add dword[addr2add],256
    inc byte[.curpri]
    dec byte[.prileft]
    jnz near .startobject
    pop ebp
    ret

SECTION .bss
.objvramloc resd 1
.objvramloc2 resd 1
.curpri  resd 1
.trypri  resd 1
.objleft resd 1
.prileft resd 1
.size1ptr resd 1
.size2ptr resd 1
SECTION .text

.reprocesssprite
    cmp cx,-8
    jle .next
    cmp cx,256
    jge .next
    add cx,8
.reprocessspriteb
    cmp dl,[resolutn]
    jae .overflow
    xor ebx,ebx
    mov bl,dl
    xor eax,eax
    cmp bx,[curypos]
    jb .overflow
    mov al,[sprlefttot+ebx]
    cmp al,45
    ja near .overflow
    inc byte[sprlefttot+ebx]
    add ebx,[addr2add]
    inc byte[sprleftpr+ebx]
    sub ebx,[addr2add]
    shl ebx,9
    shl eax,3
    add ebx,eax
    mov [ebp+ebx],cx
    mov [ebp+ebx+2],esi
    mov al,[.statusbit]
    mov [ebp+ebx+6],dh
    mov [ebp+ebx+7],al
.overflow
    inc dl
    add esi,8
    dec byte[.numleft2do]
    jnz .reprocessspriteb
    sub cx,8
    ret
.next
    add dl,8
    add esi,64
    ret

.reprocessspriteflipy
    cmp cx,-8
    jle .nextb
    cmp cx,256
    jge .nextb
    add cx,8
.reprocessspriteflipyb
    cmp dl,[resolutn]
    jae .overflow2
    xor ebx,ebx
    xor eax,eax
    mov bl,dl
    cmp bx,[curypos]
    jb .overflow2
    mov al,[sprlefttot+ebx]
    cmp al,45
    ja near .overflow2
    inc byte[sprlefttot+ebx]
    add ebx,[addr2add]
    inc byte[sprleftpr+ebx]
    sub ebx,[addr2add]
    shl ebx,9
    shl eax,3
    add ebx,eax
    mov [ebp+ebx],cx
    mov [ebp+ebx+2],esi
    mov al,[.statusbit]
    mov [ebp+ebx+6],dh
    mov [ebp+ebx+7],al
.overflow2
    inc dl
    sub esi,8
    dec byte[.numleft2do]
    jnz .reprocessspriteflipyb
    sub cx,8
    ret
.nextb
    add dl,8
    sub esi,64
    ret

section .bss
.statusbit resb 1
section .text

.process8x8sprite:
    test dh,40h
    jnz .8x8flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    jmp .returnfromptr
.8x8flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add esi,56
    call .reprocessspriteflipy
    jmp .returnfromptr

section .bss
.numleft2do resb 1
section .text

.process16x16sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .16x16flipx
    test dh,40h
    jnz .16x16flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub cx,8
    add esi,64*14
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    jmp .returnfromptr
.16x16flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,8
    add esi,56
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,16
    sub cx,8
    add esi,64*14
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    jmp .returnfromptr
.16x16flipx
    test dh,40h
    jnz .16x16flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,8
    call .reprocesssprite
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    add esi,64*14
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    jmp .returnfromptr
.16x16flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,8
    add dl,8
    add esi,56
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    add esi,64*14
    sub dl,16
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    jmp .returnfromptr

;*******************************************************
; Sprite increment/draw macros
;*******************************************************

%macro nextsprite2right 0
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
%endmacro

%macro nextsprite2rightflipy 0
    add esi,128
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
%endmacro

%macro nextsprite2rightflipx 0
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
%endmacro

%macro nextsprite2rightflipyx 0
    add esi,128
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
%endmacro

;*******************************************************
; 32x32 sprites routines
;*******************************************************

%macro nextline32x32 0
    sub cx,24
    add esi,64*12
    mov byte[.numleft2do],8
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
%endmacro

.process32x32sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .32x32flipx
    test dh,40h
    jnz near .32x32flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextline32x32
    nextline32x32
    nextline32x32
    jmp .returnfromptr

%macro nextline32x32flipy 0
    sub cx,24
    add esi,64*12+128
    sub dl,16
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
%endmacro

.32x32flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,24
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextline32x32flipy
    nextline32x32flipy
    nextline32x32flipy
    jmp .returnfromptr

%macro nextline32x32flipx 0
    add cx,24
    add esi,64*12
    mov byte[.numleft2do],8
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
%endmacro

.32x32flipx
    test dh,40h
    jnz near .32x32flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,24
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextline32x32flipx
    nextline32x32flipx
    nextline32x32flipx
    jmp .returnfromptr

%macro nextline32x32flipyx 0
    add cx,24
    add esi,64*12+128
    sub dl,16
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
%endmacro

.32x32flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,24
    add dl,24
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextline32x32flipyx
    nextline32x32flipyx
    nextline32x32flipyx
    jmp .returnfromptr

;*******************************************************
; 64x64 sprites routines
;*******************************************************

%macro nextline64x64 0
    sub cx,56
    add esi,64*8
    mov byte[.numleft2do],8
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
%endmacro

.process64x64sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .64x64flipx
    test dh,40h
    jnz near .64x64flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    jmp .returnfromptr

%macro nextline64x64flipy 0
    sub cx,56
    add esi,64*8+128
    sub dl,16
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
%endmacro

.64x64flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,56
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    jmp .returnfromptr

%macro nextline64x64flipx 0
    add cx,56
    add esi,64*8
    mov byte[.numleft2do],8
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
%endmacro

.64x64flipx
    test dh,40h
    jnz near .64x64flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,56
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    jmp .returnfromptr

%macro nextline64x64flipyx 0
    add cx,56
    add esi,64*8+128
    sub dl,16
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
%endmacro

.64x64flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,56
    add dl,56
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    jmp .returnfromptr

;*******************************************************
; Process Sprites B - Process
;*******************************************************
; Use oamram for object table

NEWSYM processspritesb
    ; set obj pointers
    cmp byte[objsize1],1
    jne .16dot1
    mov ebx,.process8x8sprite
    mov [.size1ptr],ebx
    jmp .fin1
.16dot1
    cmp byte[objsize1],4
    jne .32dot1
    mov ebx,.process16x16sprite
    mov [.size1ptr],ebx
    jmp .fin1
.32dot1
    cmp byte[objsize1],16
    jne .64dot1
    mov ebx,.process32x32sprite
    mov [.size1ptr],ebx
    jmp .fin1
.64dot1
    mov ebx,.process64x64sprite
    mov [.size1ptr],ebx
.fin1
    cmp byte[objsize2],1
    jne .16dot2
    mov ebx,.process8x8sprite
    mov [.size2ptr],ebx
    jmp .fin2
.16dot2
    cmp byte[objsize2],4
    jne .32dot2
    mov ebx,.process16x16sprite
    mov [.size2ptr],ebx
    jmp .fin2
.32dot2
    cmp byte[objsize2],16
    jne .64dot2
    mov ebx,.process32x32sprite
    mov [.size2ptr],ebx
    jmp .fin2
.64dot2
    mov ebx,.process64x64sprite
    mov [.size2ptr],ebx
.fin2
    ; set pointer adder
    xor eax,eax
    xor ebx,ebx
    mov al,[objhipr]
    shl ax,2
    mov ebx,eax
    and bx,01FCh
    mov dword[addr2add],0
    ; do 1st priority
    mov ecx,[objptr]
    shl ecx,1
    mov [.objvramloc],ecx
    mov ecx,[objptrn]
    sub ecx,[objptr]
    shl ecx,1
    mov [.objvramloc2],ecx
    push ebp
    mov ebp,[spritetablea]
.startobject
    mov byte[.objleft],128
.objloop
    xor ecx,ecx
    mov cx,[oamram+ebx+2]
    mov dl,ch
    shr dl,4
    and edx,03h
    mov [.cpri],dl
    ; get object information
    push ebx
    mov dl,[oamram+ebx+1]       ; y
    inc dl
    ; set up pointer to esi
    mov dh,ch
    and ch,01h
    shr dh,1
    shl ecx,6
    add ecx,[.objvramloc]
    test byte[oamram+ebx+3],01h
    jz .noloc2
    add ecx,[.objvramloc2]
.noloc2
    and ecx,01FFFFh
    add ecx,[vcache4b]
    mov esi,ecx
    ; get x
    mov al,[oamram+ebx]         ; x
    ; get double bits
    mov cl,bl
    shr ebx,4           ; /16
    shr cl,1
    and cl,06h
    mov ah,[oamram+ebx+512]
    shr ah,cl
    and ah,03h
    mov ch,ah
    and ch,01h
    mov cl,al
    ; process object
    ; esi = pointer to 8-bit object, dh = stats (1 shifted to right)
    ; cx = x position, dl = y position
    cmp cx,384
    jb .noadder
    add cx,65535-511
.noadder
    cmp cx,256
    jge .returnfromptr
    cmp cx,-64
    jle .returnfromptr
    test ah,02h
    jz .size1
    jmp dword near [.size2ptr]
.size1
    jmp dword near [.size1ptr]
.returnfromptr
    pop ebx
    ; next object
.nextobj
    add bx,4
    and bx,01FCh
    dec byte[.objleft]
    jnz near .objloop
    pop ebp
    ret

SECTION .bss
.objvramloc resd 1
.objvramloc2 resd 1
.curpri  resd 1
.trypri  resd 1
.objleft resd 1
.prileft resd 1
.size1ptr resd 1
.size2ptr resd 1
.cpri     resd 1
SECTION .text

.reprocesssprite
    cmp cx,-8
    jle near .next
    cmp cx,256
    jge .next
    add cx,8
.reprocessspriteb
    cmp dl,[resolutn]
    jae .overflow
    xor ebx,ebx
    xor eax,eax
    mov bl,dl
    cmp bx,[curypos]
    jb .overflow
    mov al,[sprlefttot+ebx]
    cmp al,45
    ja near .overflow
    inc byte[sprlefttot+ebx]
    mov edi,[.cpri]
    mov byte[sprleftpr+ebx*4+edi],1
    shl ebx,9
    shl eax,3
    add ebx,eax
    mov [ebp+ebx],cx
    mov [ebp+ebx+2],esi
    mov al,[.statusbit]
    and al,0F8h
    or al,[.cpri]
    mov [ebp+ebx+6],dh
    mov [ebp+ebx+7],al
.overflow
    inc dl
    add esi,8
    dec byte[.numleft2do]
    jnz .reprocessspriteb
    sub cx,8
    ret
.next
    add dl,8
    add esi,64
    ret

.reprocessspriteflipy
    cmp cx,-8
    jle near .nextb
    cmp cx,256
    jge .nextb
    add cx,8
.reprocessspriteflipyb
    cmp dl,[resolutn]
    jae .overflow2
    xor ebx,ebx
    xor eax,eax
    mov bl,dl
    cmp bx,[curypos]
    jb .overflow
    mov al,[sprlefttot+ebx]
    cmp al,45
    ja near .overflow2
    inc byte[sprlefttot+ebx]
    mov edi,[.cpri]
    mov byte[sprleftpr+ebx*4+edi],1
    shl ebx,9
    shl eax,3
    add ebx,eax
    mov [ebp+ebx],cx
    mov [ebp+ebx+2],esi
    mov al,[.statusbit]
    and al,0F8h
    or al,[.cpri]
    mov [ebp+ebx+6],dh
    mov [ebp+ebx+7],al
.overflow2
    inc dl
    sub esi,8
    dec byte[.numleft2do]
    jnz .reprocessspriteflipyb
    sub cx,8
    ret
.nextb
    add dl,8
    sub esi,64
    ret

section .bss
.statusbit resb 1
section .text

.process8x8sprite:
    test dh,40h
    jnz .8x8flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    jmp .returnfromptr
.8x8flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add esi,56
    call .reprocessspriteflipy
    jmp .returnfromptr

section .bss
.numleft2do resb 1
section .text

.process16x16sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .16x16flipx
    test dh,40h
    jnz .16x16flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub cx,8
    add esi,64*14
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    jmp .returnfromptr
.16x16flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,8
    add esi,56
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,16
    sub cx,8
    add esi,64*14
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    jmp .returnfromptr
.16x16flipx
    test dh,40h
    jnz .16x16flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,8
    call .reprocesssprite
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    add esi,64*14
    add cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocesssprite
    jmp .returnfromptr
.16x16flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,8
    add dl,8
    add esi,56
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    add esi,64*14
    sub dl,16
    add cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    add esi,128
    sub dl,8
    sub cx,8
    mov byte[.numleft2do],8
    call .reprocessspriteflipy
    jmp .returnfromptr

;*******************************************************
; 32x32 sprites routines
;*******************************************************

.process32x32sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .32x32flipx
    test dh,40h
    jnz near .32x32flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextline32x32
    nextline32x32
    nextline32x32
    jmp .returnfromptr

.32x32flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,24
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextline32x32flipy
    nextline32x32flipy
    nextline32x32flipy
    jmp .returnfromptr

.32x32flipx
    test dh,40h
    jnz near .32x32flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,24
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextline32x32flipx
    nextline32x32flipx
    nextline32x32flipx
    jmp .returnfromptr

.32x32flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,24
    add dl,24
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextline32x32flipyx
    nextline32x32flipyx
    nextline32x32flipyx
    jmp .returnfromptr

;*******************************************************
; 64x64 sprites routines
;*******************************************************

.process64x64sprite:
    mov [.statusbit],dh
    test dh,20h
    jnz near .64x64flipx
    test dh,40h
    jnz near .64x64flipy
    mov [.statusbit],dh
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    call .reprocesssprite
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextsprite2right
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    nextline64x64
    jmp .returnfromptr

.64x64flipy
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add dl,56
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextsprite2rightflipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    nextline64x64flipy
    jmp .returnfromptr

.64x64flipx
    test dh,40h
    jnz near .64x64flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,56
    call .reprocesssprite
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextsprite2rightflipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    nextline64x64flipx
    jmp .returnfromptr

.64x64flipyx
    and dh,07h
    mov byte[.numleft2do],8
    shl dh,4
    add dh,128
    add cx,56
    add dl,56
    add esi,56
    call .reprocessspriteflipy
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextsprite2rightflipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    nextline64x64flipyx
    jmp .returnfromptr

;*******************************************************
; Cache Process Macros, info from Nerlaska!
;*******************************************************

%macro processcache2b 1
    xor al,al
    add ch,ch
    adc al,al
    add cl,cl
    adc al,al
    mov [edi+%1],al
%endmacro

%macro processcache4b 1
    xor al,al
    add dh,dh
    adc al,al
    add dl,dl
    adc al,al
    add ch,ch
    adc al,al
    add cl,cl
    adc al,al
    mov [edi+%1],al
%endmacro

;*******************************************************
; Cache Sprites
;*******************************************************
; Use oamram for object table, copy from vram -> vcache4b
; 16x16 sprite, to move = 2, to add = 14, 32x32 = 4,12, 64x64 = 8,8

%macro processcache4bs 1
    xor al,al
    add dh,dh
    adc al,al
    add dl,dl
    adc al,al
    add ch,ch
    adc al,al
    add cl,cl
    adc al,al
    mov [edi+%1],al
    or al,al
    jz %%zeroed
    and byte[tiletypec],1
    jmp %%nozeroed
%%zeroed
    and byte[tiletypec],2
%%nozeroed
%endmacro

NEWSYM cachesprites
    ; initialize obj size cache
    mov dword[.objptr],oamram
    add dword[.objptr],512
    mov esi,[.objptr]
    mov al,[esi]
    mov [.curobjtype],al
    mov byte[.objleftinbyte],4
    ; Initialize oamram pointer
    mov esi,oamram
    add esi,2

    ; process pointers (.objptra = source, .objptrb = dest)
.trynextgroup
    xor ebx,ebx
    mov bx,[objptr]
    mov ecx,ebx
    shr ecx,4
    mov [.nbg],cx
    mov edi,[vram]
    add edi,ebx
    mov [.objptra],edi
    shl ebx,1
    add ebx,[vcache4b]
    mov [.objptrb],ebx

    xor ebx,ebx
    mov bx,[objptrn]
    mov ecx,ebx
    shr ecx,4
    mov [.nbg2],cx
    mov edi,[vram]
    add edi,ebx
    mov [.objptra2],edi
    shl ebx,1
    add ebx,[vcache4b]
    mov [.objptrb2],ebx

    xor ebx,ebx

    ; process objects
    mov dword[.sprnum],3
    mov byte[.objleft],128
.nextobj
    ; process sprite sizes
    test byte[.curobjtype],02h
    jz .dosprsize1
    mov al,[objsize2]
    mov [.num2do],al
    mov ax,[objadds2]
    mov [.byte2add],ax
    mov al,[objmovs2]
    mov [.byte2move],al
    mov [.byteb4add],al
    jmp .exitsprsize
.dosprsize1
    mov al,[objsize1]
    mov [.num2do],al
    mov ax,[objadds1]
    mov [.byte2add],ax
    mov al,[objmovs1]
    mov [.byte2move],al
    mov [.byteb4add],al
.exitsprsize
    shr byte[.curobjtype],2
    dec byte[.objleftinbyte]
    jnz .skipobjproc
    mov byte[.objleftinbyte],4
    inc dword[.objptr]
    mov ebx,[.objptr]
    mov al,[ebx]
    mov [.curobjtype],al
.skipobjproc
    mov bx,[esi]
    and bh,1h
    mov [.curobj],bx
.nextobject
    mov ebx,[.sprnum]
    mov cl,[oamram+ebx-2]
    mov ch,[curypos]
    dec ch
    cmp cl,ch
    jb near .nocache
    test byte[oamram+ebx],01h
    jnz .namebase
    xor ebx,ebx
    mov bx,[.curobj]
    mov cx,bx
    add bx,bx
    add bx,[.nbg]
    and bx,4095
    test word[vidmemch4+ebx],0101h
    jz near .nocache
    mov word[vidmemch4+ebx],0000h
    mov [.sprfillpl],ebx
    push esi
    shl bx,4
    mov esi,[vram]
    add esi,ebx
    add ebx,ebx
    mov edi,[vcache4b]
    add edi,ebx
    jmp .nonamebase
.namebase
    xor ebx,ebx
    mov bx,[.curobj]
    mov cx,bx
    shl bx,1
    add bx,[.nbg2]
    and bx,4095
    test word[vidmemch4+ebx],0101h
    jz near .nocache
    mov word[vidmemch4+ebx],0000h
    mov [.sprfillpl],ebx
    push esi
    shl bx,4
    mov esi,[vram]
    add esi,ebx
    add ebx,ebx
    mov edi,[vcache4b]
    add edi,ebx
.nonamebase
    ; convert from [esi] to [edi]
    mov byte[.rowleft],8
    mov byte[tiletypec],3
.donext

    mov cx,[esi]
    mov dx,[esi+16]

    processcache4bs 0
    processcache4bs 1
    processcache4bs 2
    processcache4bs 3
    processcache4bs 4
    processcache4bs 5
    processcache4bs 6
    processcache4bs 7

    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    mov ebx,[.sprfillpl]
    mov al,[tiletypec]
    shr ebx,1
    pop esi
    mov [tltype4b+ebx],al
.nocache
    inc word[.curobj]
    dec byte[.byteb4add]
    jnz .skipbyteadd
    mov ax,[.byte2add]
    add word[.curobj],ax
    mov al,[.byte2move]
    mov [.byteb4add],al
.skipbyteadd
    dec byte[.num2do]
    jnz near .nextobject
    add esi,4
    add dword[.sprnum],4
    dec byte[.objleft]
    jnz near .nextobj
    ret

SECTION .data
.num2do dd 1
.byteb4add dd 2

SECTION .bss
.objptra resd 1
.objptrb resd 1
.nbg     resd 1
.objptra2 resd 1
.objptrb2 resd 1
.nbg2     resd 1
.objleft resb 1
.rowleft resb 1
.a       resd 1
.objptr resd 1
.objleftinbyte resd 1
.curobjtype resd 1
.curobj resd 1
.byte2move resd 1
.byte2add  resd 1
.sprnum    resd 1
.sprcheck  resd 1
.sprfillpl resd 1

section .text

;*******************************************************
; Cache 2-Bit
;*******************************************************
NEWSYM cachetile2b
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,4
    mov byte[.count],32
    mov [.nbg],bx
    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    and esi,4095
    test byte[vidmemch2+esi],01h
    jz near .nocache
    mov byte[vidmemch2+esi],00h
    mov edi,esi
    shl esi,4
    shl edi,6
    add esi,[vram]
    add edi,[vcache2b]
    push eax
    mov byte[.rowleft],8
.donext
    mov cx,[esi]
    processcache2b 0
    processcache2b 1
    processcache2b 2
    processcache2b 3
    processcache2b 4
    processcache2b 5
    processcache2b 6
    processcache2b 7
    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
.nocache
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg     resw 1
.count   resb 1
.a       resb 1
.rowleft resb 1
.nextar  resb 1

section .text

NEWSYM cache2bit
    ret

;*******************************************************
; Cache 4-Bit
;*******************************************************

; esi = pointer to tile location vram
; edi = pointer to graphics data (cache & non-cache)
; ebx = external pointer
; tile value : bit 15 = flipy, bit 14 = flipx, bit 10-12 = palette, 0-9=tile#

NEWSYM cachetile4b
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,5
    mov byte[.count],32
    mov [.nbg],bx

    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    shl esi,1
    and esi,4095
    test word[vidmemch4+esi],0101h
    jz near .nocache
    mov word[vidmemch4+esi],0000h
    mov edi,esi
    shl esi,4
    shl edi,5
    add esi,[vram]
    add edi,[vcache4b]
    push eax
    mov byte[.rowleft],8
.donext

    mov cx,[esi]
    mov dx,[esi+16]
    processcache4b 0
    processcache4b 1
    processcache4b 2
    processcache4b 3
    processcache4b 4
    processcache4b 5
    processcache4b 6
    processcache4b 7

    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
.nocache
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg     resw 1
.count   resb 1
.rowleft resb 1
.nextar  resb 1

section .text

NEWSYM cache4bit
    ret
;*******************************************************
; Cache 8-Bit
;*******************************************************
; tile value : bit 15 = flipy, bit 14 = flipx, bit 10-12 = palette, 0-9=tile#
NEWSYM cachetile8b
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,6
    mov byte[.count],32
    mov [.nbg],bx

    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    shl esi,2
    and esi,4095
    test dword[vidmemch8+esi],01010101h
    jz near .nocache
    mov dword[vidmemch8+esi],00000000h
    mov edi,esi
    shl esi,4
    shl edi,4
    add esi,[vram]
    add edi,[vcache8b]
    push eax
    mov byte[.rowleft],8
.donext
    xor ah,ah
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov byte[.a],0

    mov al,[esi]                ; bitplane 0
    cmp al,0
    je .skipconva
    test al,01h
    jz .skipa0
    or ah,01h
.skipa0
    test al,02h
    jz .skipa1
    or bl,01h
.skipa1
    test al,04h
    jz .skipa2
    or bh,01h
.skipa2
    test al,08h
    jz .skipa3
    or cl,01h
.skipa3
    test al,10h
    jz .skipa4
    or ch,01h
.skipa4
    test al,20h
    jz .skipa5
    or dl,01h
.skipa5
    test al,40h
    jz .skipa6
    or dh,01h
.skipa6
    test al,80h
    jz .skipa7
    or byte[.a],01h
.skipa7
.skipconva

    mov al,[esi+1]                ; bitplane 1
    cmp al,0
    je .skipconvb
    test al,01h
    jz .skipb0
    or ah,02h
.skipb0
    test al,02h
    jz .skipb1
    or bl,02h
.skipb1
    test al,04h
    jz .skipb2
    or bh,02h
.skipb2
    test al,08h
    jz .skipb3
    or cl,02h
.skipb3
    test al,10h
    jz .skipb4
    or ch,02h
.skipb4
    test al,20h
    jz .skipb5
    or dl,02h
.skipb5
    test al,40h
    jz .skipb6
    or dh,02h
.skipb6
    test al,80h
    jz .skipb7
    or byte[.a],02h
.skipb7
.skipconvb

    mov al,[esi+16]                ; bitplane 2
    cmp al,0
    je .skipconvc
    test al,01h
    jz .skipc0
    or ah,04h
.skipc0
    test al,02h
    jz .skipc1
    or bl,04h
.skipc1
    test al,04h
    jz .skipc2
    or bh,04h
.skipc2
    test al,08h
    jz .skipc3
    or cl,04h
.skipc3
    test al,10h
    jz .skipc4
    or ch,04h
.skipc4
    test al,20h
    jz .skipc5
    or dl,04h
.skipc5
    test al,40h
    jz .skipc6
    or dh,04h
.skipc6
    test al,80h
    jz .skipc7
    or byte[.a],04h
.skipc7
.skipconvc

    mov al,[esi+17]                ; bitplane 3
    cmp al,0
    je .skipconvd
    test al,01h
    jz .skipd0
    or ah,08h
.skipd0
    test al,02h
    jz .skipd1
    or bl,08h
.skipd1
    test al,04h
    jz .skipd2
    or bh,08h
.skipd2
    test al,08h
    jz .skipd3
    or cl,08h
.skipd3
    test al,10h
    jz .skipd4
    or ch,08h
.skipd4
    test al,20h
    jz .skipd5
    or dl,08h
.skipd5
    test al,40h
    jz .skipd6
    or dh,08h
.skipd6
    test al,80h
    jz .skipd7
    or byte[.a],08h
.skipd7
.skipconvd

    mov al,[esi+32]                ; bitplane 4
    cmp al,0
    je .skipconve
    test al,01h
    jz .skipe0
    or ah,10h
.skipe0
    test al,02h
    jz .skipe1
    or bl,10h
.skipe1
    test al,04h
    jz .skipe2
    or bh,10h
.skipe2
    test al,08h
    jz .skipe3
    or cl,10h
.skipe3
    test al,10h
    jz .skipe4
    or ch,10h
.skipe4
    test al,20h
    jz .skipe5
    or dl,10h
.skipe5
    test al,40h
    jz .skipe6
    or dh,10h
.skipe6
    test al,80h
    jz .skipe7
    or byte[.a],10h
.skipe7
.skipconve

    mov al,[esi+33]                ; bitplane 5
    cmp al,0
    je .skipconvf
    test al,01h
    jz .skipf0
    or ah,20h
.skipf0
    test al,02h
    jz .skipf1
    or bl,20h
.skipf1
    test al,04h
    jz .skipf2
    or bh,20h
.skipf2
    test al,08h
    jz .skipf3
    or cl,20h
.skipf3
    test al,10h
    jz .skipf4
    or ch,20h
.skipf4
    test al,20h
    jz .skipf5
    or dl,20h
.skipf5
    test al,40h
    jz .skipf6
    or dh,20h
.skipf6
    test al,80h
    jz .skipf7
    or byte[.a],20h
.skipf7
.skipconvf

    mov al,[esi+48]                ; bitplane 6
    cmp al,0
    je .skipconvg
    test al,01h
    jz .skipg0
    or ah,40h
.skipg0
    test al,02h
    jz .skipg1
    or bl,40h
.skipg1
    test al,04h
    jz .skipg2
    or bh,40h
.skipg2
    test al,08h
    jz .skipg3
    or cl,40h
.skipg3
    test al,10h
    jz .skipg4
    or ch,40h
.skipg4
    test al,20h
    jz .skipg5
    or dl,40h
.skipg5
    test al,40h
    jz .skipg6
    or dh,40h
.skipg6
    test al,80h
    jz .skipg7
    or byte[.a],40h
.skipg7
.skipconvg

    mov al,[esi+49]                ; bitplane 7
    cmp al,0
    je .skipconvh
    test al,01h
    jz .skiph0
    or ah,80h
.skiph0
    test al,02h
    jz .skiph1
    or bl,80h
.skiph1
    test al,04h
    jz .skiph2
    or bh,80h
.skiph2
    test al,08h
    jz .skiph3
    or cl,80h
.skiph3
    test al,10h
    jz .skiph4
    or ch,80h
.skiph4
    test al,20h
    jz .skiph5
    or dl,80h
.skiph5
    test al,40h
    jz .skiph6
    or dh,80h
.skiph6
    test al,80h
    jz .skiph7
    or byte[.a],80h
.skiph7
.skipconvh

    ; move all bytes into [edi]
    mov [edi+7],ah
    mov [edi+6],bl
    mov [edi+5],bh
    mov [edi+4],cl
    mov [edi+3],ch
    mov [edi+2],dl
    mov [edi+1],dh
    mov al,[.a]
    mov [edi],al
    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
.nocache
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg     resw 1
.count   resb 1
.a       resb 1
.rowleft resb 1
.nextar  resb 1

section .text

NEWSYM cache8bit
    ret

;*******************************************************
; Cache 2-Bit 16x16 tiles
;*******************************************************

NEWSYM cachetile2b16x16
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,4
    mov byte[.count],32
    mov [.nbg],bx
    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    mov byte[.tileleft],4
.nextof4
    and esi,4095
    test byte[vidmemch2+esi],01h
    jz near .nocache
    mov byte[vidmemch2+esi],00h
    push esi
    mov edi,esi
    shl esi,4
    shl edi,6
    add esi,[vram]
    add edi,[vcache2b]
    push eax
    mov byte[.rowleft],8
.donext
    mov cx,[esi]
    processcache2b 0
    processcache2b 1
    processcache2b 2
    processcache2b 3
    processcache2b 4
    processcache2b 5
    processcache2b 6
    processcache2b 7
    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
    pop esi
.nocache
    inc esi
    cmp byte[.tileleft],3
    jne .noadd
    add esi,14
.noadd
    dec byte[.tileleft]
    jnz near .nextof4
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg      resw 1
.count    resb 1
.a        resb 1
.rowleft  resb 1
.nextar   resb 1
.tileleft resb 1

section .text

NEWSYM cache2bit16x16
    ret

;*******************************************************
; Cache 4-Bit 16x16 tiles
;*******************************************************

NEWSYM cachetile4b16x16
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,5
    mov byte[.count],32
    mov [.nbg],bx

    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    shl esi,1
    mov byte[.tileleft],4
.nextof4
    and esi,4095
    test word[vidmemch4+esi],0101h
    jz near .nocache
    mov word[vidmemch4+esi],0000h
    push esi
    mov edi,esi
    shl esi,4
    shl edi,5
    add esi,[vram]
    add edi,[vcache4b]
    push eax
    mov byte[.rowleft],8
.donext
    mov cx,[esi]
    mov dx,[esi+16]

    processcache4b 0
    processcache4b 1
    processcache4b 2
    processcache4b 3
    processcache4b 4
    processcache4b 5
    processcache4b 6
    processcache4b 7

    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
    pop esi
.nocache
    add esi,2
    cmp byte[.tileleft],3
    jne .noadd
    add esi,28
.noadd
    dec byte[.tileleft]
    jnz near .nextof4
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg     resw 1
.count   resb 1
.rowleft resb 1
.nextar  resb 1
.tileleft resb 1

section .text

NEWSYM cache4bit16x16
    ret

;*******************************************************
; Cache 8-Bit 16x16 tiles
;*******************************************************

NEWSYM cachetile8b16x16
    ; Keep high word ecx 0
    push eax
    xor ecx,ecx
    push edx
    mov byte[.nextar],1
    push ebx
    ; get tile info location
    test al,20h
    jnz .highptr
    shl eax,6   ; x 64 for each line
    add ax,[bgptr]
    jmp .loptr
.highptr
    and al,1Fh
    shl eax,6   ; x 64 for each line
    add ax,[bgptrc]
.loptr
    add eax,[vram]
    mov bx,[curtileptr]
    shr bx,6
    mov byte[.count],32
    mov [.nbg],bx

    ; do loop
.cacheloop
    mov si,[eax]
    and esi,03FFh
    add si,[.nbg]
    shl esi,2
    mov byte[.tileleft],4
.nextof4
    and esi,4095
    test dword[vidmemch8+esi],01010101h
    jz near .nocache
    mov dword[vidmemch8+esi],00000000h
    push esi
    mov edi,esi
    shl esi,4
    shl edi,4
    add esi,[vram]
    add edi,[vcache8b]
    push eax
    mov byte[.rowleft],8
.donext
    xor ah,ah
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov byte[.a],0

    mov al,[esi]                ; bitplane 0
    cmp al,0
    je .skipconva
    test al,01h
    jz .skipa0
    or ah,01h
.skipa0
    test al,02h
    jz .skipa1
    or bl,01h
.skipa1
    test al,04h
    jz .skipa2
    or bh,01h
.skipa2
    test al,08h
    jz .skipa3
    or cl,01h
.skipa3
    test al,10h
    jz .skipa4
    or ch,01h
.skipa4
    test al,20h
    jz .skipa5
    or dl,01h
.skipa5
    test al,40h
    jz .skipa6
    or dh,01h
.skipa6
    test al,80h
    jz .skipa7
    or byte[.a],01h
.skipa7
.skipconva

    mov al,[esi+1]                ; bitplane 1
    cmp al,0
    je .skipconvb
    test al,01h
    jz .skipb0
    or ah,02h
.skipb0
    test al,02h
    jz .skipb1
    or bl,02h
.skipb1
    test al,04h
    jz .skipb2
    or bh,02h
.skipb2
    test al,08h
    jz .skipb3
    or cl,02h
.skipb3
    test al,10h
    jz .skipb4
    or ch,02h
.skipb4
    test al,20h
    jz .skipb5
    or dl,02h
.skipb5
    test al,40h
    jz .skipb6
    or dh,02h
.skipb6
    test al,80h
    jz .skipb7
    or byte[.a],02h
.skipb7
.skipconvb

    mov al,[esi+16]                ; bitplane 2
    cmp al,0
    je .skipconvc
    test al,01h
    jz .skipc0
    or ah,04h
.skipc0
    test al,02h
    jz .skipc1
    or bl,04h
.skipc1
    test al,04h
    jz .skipc2
    or bh,04h
.skipc2
    test al,08h
    jz .skipc3
    or cl,04h
.skipc3
    test al,10h
    jz .skipc4
    or ch,04h
.skipc4
    test al,20h
    jz .skipc5
    or dl,04h
.skipc5
    test al,40h
    jz .skipc6
    or dh,04h
.skipc6
    test al,80h
    jz .skipc7
    or byte[.a],04h
.skipc7
.skipconvc

    mov al,[esi+17]                ; bitplane 3
    cmp al,0
    je .skipconvd
    test al,01h
    jz .skipd0
    or ah,08h
.skipd0
    test al,02h
    jz .skipd1
    or bl,08h
.skipd1
    test al,04h
    jz .skipd2
    or bh,08h
.skipd2
    test al,08h
    jz .skipd3
    or cl,08h
.skipd3
    test al,10h
    jz .skipd4
    or ch,08h
.skipd4
    test al,20h
    jz .skipd5
    or dl,08h
.skipd5
    test al,40h
    jz .skipd6
    or dh,08h
.skipd6
    test al,80h
    jz .skipd7
    or byte[.a],08h
.skipd7
.skipconvd

    mov al,[esi+32]                ; bitplane 4
    cmp al,0
    je .skipconve
    test al,01h
    jz .skipe0
    or ah,10h
.skipe0
    test al,02h
    jz .skipe1
    or bl,10h
.skipe1
    test al,04h
    jz .skipe2
    or bh,10h
.skipe2
    test al,08h
    jz .skipe3
    or cl,10h
.skipe3
    test al,10h
    jz .skipe4
    or ch,10h
.skipe4
    test al,20h
    jz .skipe5
    or dl,10h
.skipe5
    test al,40h
    jz .skipe6
    or dh,10h
.skipe6
    test al,80h
    jz .skipe7
    or byte[.a],10h
.skipe7
.skipconve

    mov al,[esi+33]                ; bitplane 5
    cmp al,0
    je .skipconvf
    test al,01h
    jz .skipf0
    or ah,20h
.skipf0
    test al,02h
    jz .skipf1
    or bl,20h
.skipf1
    test al,04h
    jz .skipf2
    or bh,20h
.skipf2
    test al,08h
    jz .skipf3
    or cl,20h
.skipf3
    test al,10h
    jz .skipf4
    or ch,20h
.skipf4
    test al,20h
    jz .skipf5
    or dl,20h
.skipf5
    test al,40h
    jz .skipf6
    or dh,20h
.skipf6
    test al,80h
    jz .skipf7
    or byte[.a],20h
.skipf7
.skipconvf

    mov al,[esi+48]                ; bitplane 6
    cmp al,0
    je .skipconvg
    test al,01h
    jz .skipg0
    or ah,40h
.skipg0
    test al,02h
    jz .skipg1
    or bl,40h
.skipg1
    test al,04h
    jz .skipg2
    or bh,40h
.skipg2
    test al,08h
    jz .skipg3
    or cl,40h
.skipg3
    test al,10h
    jz .skipg4
    or ch,40h
.skipg4
    test al,20h
    jz .skipg5
    or dl,40h
.skipg5
    test al,40h
    jz .skipg6
    or dh,40h
.skipg6
    test al,80h
    jz .skipg7
    or byte[.a],40h
.skipg7
.skipconvg

    mov al,[esi+49]                ; bitplane 7
    cmp al,0
    je .skipconvh
    test al,01h
    jz .skiph0
    or ah,80h
.skiph0
    test al,02h
    jz .skiph1
    or bl,80h
.skiph1
    test al,04h
    jz .skiph2
    or bh,80h
.skiph2
    test al,08h
    jz .skiph3
    or cl,80h
.skiph3
    test al,10h
    jz .skiph4
    or ch,80h
.skiph4
    test al,20h
    jz .skiph5
    or dl,80h
.skiph5
    test al,40h
    jz .skiph6
    or dh,80h
.skiph6
    test al,80h
    jz .skiph7
    or byte[.a],80h
.skiph7
.skipconvh

    ; move all bytes into [edi]
    mov [edi+7],ah
    mov [edi+6],bl
    mov [edi+5],bh
    mov [edi+4],cl
    mov [edi+3],ch
    mov [edi+2],dl
    mov [edi+1],dh
    mov al,[.a]
    mov [edi],al
    add edi,8
    add esi,2
    dec byte[.rowleft]
    jnz near .donext
    pop eax
    pop esi
.nocache
    add esi,4
    cmp byte[.tileleft],3
    jne .noadd
    add esi,56
.noadd
    dec byte[.tileleft]
    jnz near .nextof4
    add eax,2
    dec byte[.count]
    jnz near .cacheloop

    cmp byte[.nextar],0
    je .skipall
    mov bx,[bgptrc]
    cmp [bgptrd],bx
    je .skipall
    add eax,2048-64
    mov byte[.count],32
    mov byte[.nextar],0
    jmp .cacheloop
.skipall
    pop ebx
    pop edx
    pop eax
    ret

section .bss

.nbg      resw 1
.count    resb 1
.a        resb 1
.rowleft  resb 1
.nextar   resb 1
.tileleft resb 1

section .text

NEWSYM cache8bit16x16
    ret

NEWSYM cachesingle
    cmp byte[offsetmshl],1
    je near cachesingle4b
    cmp byte[offsetmshl],2
    je near cachesingle2b
    ret

%macro processcache4b2 1
    xor al,al
    add dh,dh
    adc al,al
    add dl,dl
    adc al,al
    add ch,ch
    adc al,al
    add cl,cl
    adc al,al
    mov [edi+%1],al
%endmacro

NEWSYM cachesingle4b
    mov word[ebx],0
    sub ebx,vidmemch4
    push edi
    mov edi,ebx
    shl edi,5           ; cached ram
    shl ebx,4           ; vram
    add edi,[vcache4b]
    add ebx,[vram]
    push eax
    push edx
    mov byte[scacheloop],8
.nextline
    mov cx,[ebx]
    mov dx,[ebx+16]
    processcache4b2 0
    processcache4b2 1
    processcache4b2 2
    processcache4b2 3
    processcache4b2 4
    processcache4b2 5
    processcache4b2 6
    processcache4b2 7
    add ebx,2
    add edi,8
    dec byte[scacheloop]
    jnz near .nextline
    pop edx
    pop eax
    pop edi
    ret

NEWSYM cachesingle2b
    ret

section .bss

NEWSYM scacheloop, resb 1
NEWSYM tiletypec, resb 1

section .text

%macro processcache4b3 1
    xor al,al
    add dh,dh
    adc al,al
    add dl,dl
    adc al,al
    add bh,bh
    adc al,al
    add bl,bl
    adc al,al
    mov [edi+%1],al
    or al,al
    jz %%zeroed
    and byte[tiletypec],1
    jmp %%nozeroed
%%zeroed
    and byte[tiletypec],2
%%nozeroed
%endmacro

NEWSYM cachesingle4bng
    mov word[vidmemch4+ecx*2],0
    mov byte[tiletypec],3
    push edi
    push eax
    push ecx
    push ebx
    push edx
    mov edi,ecx
    shl edi,6           ; cached ram
    shl ecx,5           ; vram
    add edi,[vcache4b]
    add ecx,[vram]
    mov byte[scacheloop],8
.nextline
    mov bx,[ecx]
    mov dx,[ecx+16]
    processcache4b3 0
    processcache4b3 1
    processcache4b3 2
    processcache4b3 3
    processcache4b3 4
    processcache4b3 5
    processcache4b3 6
    processcache4b3 7
    add ecx,2
    add edi,8
    dec byte[scacheloop]
    jnz near .nextline
    pop edx
    pop ebx
    pop ecx
    mov al,[tiletypec]
    mov [tltype4b+ecx],al
    pop eax
    pop edi
    ret

%macro processcache2b3 1
    xor al,al
    add bh,bh
    adc al,al
    add bl,bl
    adc al,al
    mov [edi+%1],al
    or al,al
    jz %%zeroed
    and byte[tiletypec],1
    jmp %%nozeroed
%%zeroed
    and byte[tiletypec],2
%%nozeroed
%endmacro

NEWSYM cachesingle2bng
    mov byte[vidmemch2+ecx],0
    mov byte[tiletypec],3
    push edi
    push eax
    push ecx
    push ebx
    push edx
    mov edi,ecx
    shl edi,6           ; cached ram
    shl ecx,4           ; vram
    add edi,[vcache2b]
    add ecx,[vram]
    mov byte[scacheloop],8
.nextline
    mov bx,[ecx]
    processcache2b3 0
    processcache2b3 1
    processcache2b3 2
    processcache2b3 3
    processcache2b3 4
    processcache2b3 5
    processcache2b3 6
    processcache2b3 7
    add ecx,2
    add edi,8
    dec byte[scacheloop]
    jnz near .nextline
    pop edx
    pop ebx
    pop ecx
    mov al,[tiletypec]
    mov [tltype2b+ecx],al
    pop eax
    pop edi
    ret

%macro processcache8b3 1
    xor esi,esi
    add ch,ch
    adc esi,esi
    add cl,cl
    adc esi,esi
    add dh,dh
    adc esi,esi
    add dl,dl
    adc esi,esi
    add ah,ah
    adc esi,esi
    add al,al
    adc esi,esi
    add bh,bh
    adc esi,esi
    add bl,bl
    adc esi,esi
    push eax
    mov eax,esi
    mov [edi+%1],al
    or al,al
    jz %%zeroed
    and byte[tiletypec],1
    jmp %%nozeroed
%%zeroed
    and byte[tiletypec],2
%%nozeroed
    pop eax
%endmacro

NEWSYM cachesingle8bng
    mov dword[vidmemch8+ecx*4],0
    mov byte[tiletypec],3
    push esi
    push edi
    push eax
    push ecx
    push ebx
    push edx
    mov edi,ecx
    shl edi,6           ; cached ram
    shl ecx,6           ; vram
    add edi,[vcache8b]
    add ecx,[vram]
    mov byte[scacheloop],8
.nextline
    mov bx,[ecx]
    mov ax,[ecx+16]
    mov dx,[ecx+32]
    push ecx
    mov cx,[ecx+48]
    processcache8b3 0
    processcache8b3 1
    processcache8b3 2
    processcache8b3 3
    processcache8b3 4
    processcache8b3 5
    processcache8b3 6
    processcache8b3 7
    pop ecx
    add ecx,2
    add edi,8
    dec byte[scacheloop]
    jnz near .nextline
    pop edx
    pop ebx
    pop ecx
    mov al,[tiletypec]
    mov [tltype8b+ecx],al
    pop eax
    pop edi
    pop esi
    ret

SECTION .bss
NEWSYM dcolortab, resd 256

SECTION .data
NEWSYM ExitFromGUI,     db 0
NEWSYM videotroub,      dd 0
NEWSYM TripBufAvail,    db 0
NEWSYM vesa2_clbit,     dd 0            ; clear all bit 0's if AND is used
NEWSYM vesa2_rpos,      dd 0            ; Red bit position
NEWSYM vesa2_gpos,      dd 0            ; Green bit position
NEWSYM vesa2_bpos,      dd 0            ; Blue bit position
NEWSYM vesa2_clbitng,   dd 0            ; clear all bit 0's if AND is used
NEWSYM vesa2_clbitng2,  dd 0,0          ; clear all bit 0's if AND is used
NEWSYM vesa2_clbitng3,  dd 0            ; clear all bit 0's if AND is used
NEWSYM vesa2red10,      dd 0            ; red position at bit 10
NEWSYM vesa2_rtrcl,     dd 0            ; red transparency clear     (bit+4)
NEWSYM vesa2_rtrcla,    dd 0            ; red transparency (AND) clear (not(bit+4))
NEWSYM vesa2_rfull,     dd 0            ; red max (or bit*1Fh)
NEWSYM vesa2_gtrcl,     dd 0            ; red transparency clear     (bit+4)
NEWSYM vesa2_gtrcla,    dd 0            ; red transparency (AND) clear (not(bit+4))
NEWSYM vesa2_gfull,     dd 0            ; red max (or bit*1Fh)
NEWSYM vesa2_btrcl,     dd 0            ; red transparency clear     (bit+4)
NEWSYM vesa2_btrcla,    dd 0            ; red transparency (AND) clear (not(bit+4))
NEWSYM vesa2_bfull,     dd 0            ; red max (or bit*1Fh)
NEWSYM vesa2_x,         dd 320          ; Desired screen width
NEWSYM vesa2_y,         dd 240          ; Height
NEWSYM vesa2_bits,      dd 8            ; Bits per pixel
NEWSYM vesa2_rposng,    dd 0            ; Red bit position
NEWSYM vesa2_gposng,    dd 0            ; Green bit position
NEWSYM vesa2_bposng,    dd 0            ; Blue bit position
NEWSYM vesa2_usbit,     dd 0            ; Unused bit in proper bit location
NEWSYM ErrorPointer,    dd 0

SECTION .text
NEWSYM genfulladdtab
    ; Write to buffer
    cmp byte[newengen],1
    jne .notneweng
    cmp byte[vesa2red10],0
    jne near genfulladdtabred
.notneweng
    xor ecx,ecx
.loopers
    mov ax,cx
    test [vesa2_rtrcl],cx
    jz .nor
    and ax,[vesa2_rtrcla]
    or ax,[vesa2_rfull]
.nor
    test [vesa2_gtrcl],cx
    jz .nog
    and ax,[vesa2_gtrcla]
    or ax,[vesa2_gfull]
.nog
    test [vesa2_btrcl],cx
    jz .nob
    and ax,[vesa2_btrcla]
    or ax,[vesa2_bfull]
.nob
    shl ax,1
    mov [fulladdtab+ecx*2],ax
    dec cx
    jnz .loopers
    ret

NEWSYM genfulladdtabred
NEWSYM genfulladdtabng
    ; Write to buffer
    xor ecx,ecx
.loopers
    mov ax,cx
    test cx,0100000000000000b
    jz .nor
    and ax,1011111111111111b
    or ax, 0011110000000000b
.nor
    test cx,0000001000000000b
    jz .nog
    and ax,1111110111111111b
    or ax, 0000000111100000b
.nog
    test cx,0000000000010000b
    jz .nob
    and ax,1111111111101111b
    or ax, 0000000000001111b
.nob
    shl ax,1
    mov [fulladdtab+ecx*2],ax
    dec cx
    jnz .loopers
    ret
