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

EXTSYM StringLength
EXTSYM Get_Time
EXTSYM objhipr
EXTSYM KeyRewind
EXTSYM xa,timer2upd,prevoamptr,ReadHead
EXTSYM prevedi,SA1xpc,SA1RAMArea,sa1dmaptr
EXTSYM DSP1COp,C4WFXVal,C41FXVal,Op00Multiplicand,Op10Coefficient,Op04Angle
EXTSYM Op08X,Op18X,Op28X,Op0CA,Op02FX,Op0AVS,Op06X,Op0DX,Op03F,Op14Zr
EXTSYM Op0EH,DSP1Type,Op01m
EXTSYM Voice0Status
EXTSYM UpdateDPage
EXTSYM MessageOn,MsgCount,Msgptr,StartGUI,cbitmode,debuggeron,romdata
EXTSYM frameskip,initvideo,newgfx16b,soundon,cvidmode
EXTSYM vidbuffer,vidbufferofsa,vidbufferofsb,disable65816sh,GUISaveVars,virqnodisable
EXTSYM KeySaveState,KeyLoadState,KeyQuickExit,KeyQuickLoad,KeyQuickRst,GUIDoReset
EXTSYM KeyOnStA,KeyOnStB,ProcessKeyOn,printnum,sramsavedis,DSPDisable,C4Enable
EXTSYM KeyQuickClock,KeyQuickSaveSPC,TimerEnable,AutoIncSaveSlot
EXTSYM IRQHack,HIRQLoc,Offby1line,splitflags,joinflags,KeyQuickSnapShot
EXTSYM csounddisable,videotroub,Open_File,Close_File,Read_File,ResetTripleBuf
EXTSYM Write_File,Output_Text,Create_File,Check_Key,Get_Key,Change_Dir,InitPreGame
;EXTSYM OSPort
;    EXTSYM tempblah,romdata
EXTSYM Curtableaddr
EXTSYM curcyc,debugdisble,dmadata,guioff,memtabler8,SetupPreGame
EXTSYM memtablew8,regaccessbankr8,showmenu,snesmap2,snesmmap,DeInitPostGame
EXTSYM spcPCRam,startdebugger,xp,xpb,xpc,tablead,tableadb,tableadc
;    EXTSYM oamram
EXTSYM SA1UpdateDPage,Makemode7Table
EXTSYM memtabler16,memaccessbankr848mb,memaccessbankr1648mb
EXTSYM nextmenupopup
EXTSYM MovieProcessing
EXTSYM MovieFileHand, PrintStr
EXTSYM OSExit,DosExit,InitDir,InitDrive,createnewcfg,fnames,gotoroot,previdmode
EXTSYM ramsize,sfxramdata,sram,SRAMDrive,SRAMDir,welcome
;    EXTSYM tempstore
EXTSYM printhex
%ifdef __MSDOS__
EXTSYM ModemInitStat, DeInitModem
EXTSYM deinitipx
%endif
EXTSYM deinitvideo
EXTSYM BRRBuffer,DSPMem,PrepareSaveState,ResetState,SFXEnable,PHdspsave
EXTSYM fnamest,sndrot,spcRam,spcRamDP,tableA,unpackfunct,vram,wramdata
EXTSYM zsmesg,PHnum2writesfxreg,SfxR0,PHnum2writecpureg,PHspcsave
EXTSYM C4Ram
EXTSYM SPC7110Enable
EXTSYM SA1Mode,PHnum2writesa1reg,SaveSA1,RestoreSA1,UpdateBanksSDD1
EXTSYM SDD1Enable
EXTSYM CapturePicture,PrevPicture,NoPictureSave
EXTSYM BRRPlace0,SfxCPB,SfxCROM,SfxLastRamAdr,SfxMemTable,Totalbyteloaded
EXTSYM SfxRAMBR,SfxRAMMem,SfxROMBR,SfxRomBuffer,Voice0Freq
EXTSYM cycpbl,cycpbl2,cycpblt,cycpblt2,irqon,nexthdma
EXTSYM repackfunct,spcnumread,spchalted,spcon,versn,headerhack,initpitch
EXTSYM SPCMultA,PHnum2writespc7110reg
EXTSYM multchange,procexecloop,vidmemch2
EXTSYM romispal
EXTSYM scrndis,sprlefttot,sprleftpr,processsprites,cachesprites
EXTSYM NextLineStart,FlipWait,LastLineStart
EXTSYM opcjmptab
EXTSYM cpuoverptr
EXTSYM CheatOn,INTEnab,JoyAPos,JoyBPos,JoyCRead,NMIEnab,NumCheats,CurrentExecSA1
EXTSYM ReadInputDevice,StartDrawNewGfx,VIRQLoc,cachevideo,cfield
EXTSYM cheatdata,curblank,curnmi,curypos,cycpl,doirqnext,drawline
EXTSYM execatzerovirq,exechdma,hdmadelay,intrset,newengen,oamaddr
EXTSYM oamaddrs,processmouse,resolutn,showvideo,snesmouse,starthdma
EXTSYM switchtonmi,switchtovirq,totlines,updatetimer,SA1Swap,SA1DoIRQ
EXTSYM JoyAOrig,JoyANow,JoyBOrig,JoyBNow,JoyCOrig,JoyCNow,JoyDOrig,JoyDNow
EXTSYM JoyEOrig,JoyENow,chaton,chatstrL,chatRTL,chatstrR,SA1Message
EXTSYM MultiTapStat,MovieCounter,idledetectspc,SA1Control,SA1Enable,SA1IRQEnable
EXTSYM SPC700read,SPC700write,numspcvblleft,spc700idle,SA1Status,SA1IRQExec
EXTSYM ForceNewGfxOff,LethEnData,C4Pause,GUIQuit
EXTSYM IRAM,SA1Ptr,SA1BWPtr
EXTSYM scrnon,scaddset
EXTSYM outofmemfix,yesoutofmemory
EXTSYM CNetType,Latency,LatencyLeft,NetSwap
;    EXTSYM vesa2selec
EXTSYM RemoteSendChar,RemoteGetChar,pl1neten,pl2neten,pl3neten,pl4neten
EXTSYM pl5neten,RemoteSendEAX,prevp1net,prevp2net,prevp3net,prevp4net
EXTSYM RemoteGetEAX,cnetplaybuf,netdelayed,cnetptrtail,cnetptrhead
EXTSYM ChatProgress,RecvProgress,chatTL,WritetochatBuffer,NetAddChar
EXTSYM PreparePacket, SendPacket, NoInputRead, RemoteDisconnect
EXTSYM SendPacketUDP
EXTSYM ChatNick
EXTSYM JoyRead,ChatType2,chatstrR2,chatstrR3,chatstrR4,chatstrR5
EXTSYM chatRTL2,chatRTL3,chatRTL4,chatRTL5
EXTSYM NetLoadState
EXTSYM ProcessMovies
EXTSYM ioportval
EXTSYM C4VBlank
EXTSYM dsp1teststuff
EXTSYM ReturnFromSPCStall,SPCStallSetting,cycpb268,cycpb358,HIRQSkip,scanlines
EXTSYM smallscreenon,ScreenScale
EXTSYM MainLoop,NumberOfOpcodes,SfxCLSR,SfxSCMR,SfxPOR
EXTSYM sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc,sfxclineloc
EXTSYM PLOTJmpa,PLOTJmpb,FxTable,FxTableb,FxTablec,FxTabled
EXTSYM SfxPBR,SCBRrel,SfxSCBR,SfxCOLR,hdmaearlstart,SFXCounter
EXTSYM fxbit01,fxbit01pcal,fxbit23,fxbit23pcal,fxbit45,fxbit45pcal,fxbit67,fxbit67pcal
EXTSYM SfxSFR,nosprincr,hirqmode2
EXTSYM cpucycle,debstop,switchtovirqdeb,debstop3,switchtonmideb
EXTSYM ReadSPC7110log,WriteSPC7110log
EXTSYM NetPlayNoMore
EXTSYM statefileloc

%ifdef OPENSPC
EXTSYM OSPC_Run, ospc_cycle_frac
%endif

%ifdef __MSDOS__
EXTSYM dssel
%endif

NEWSYM ExecuteAsmStart

%macro BackupCVMacM 2
    mov edx,%1
    mov ecx,%2
%%loop
    movq mm0,[edx]
    movq mm1,[edx+8]
    movq [ebx],mm0
    movq [ebx+8],mm1
    add edx,16
    add ebx,16
    dec ecx
    jnz %%loop
%endmacro

%macro BackupCVMac 2
    mov edx,%1
    mov ecx,%2
%%loop
    mov eax,[edx]
    mov [ebx],eax
    add edx,4
    add ebx,4
    dec ecx
    jnz %%loop
%endmacro

%macro BackupCVMacB 2
    mov edx,%1
    mov ecx,%2
%%loop
    mov al,[edx]
    mov [ebx],al
    inc edx
    inc ebx
    dec ecx
    jnz %%loop
%endmacro

%macro BackupCVRMacM 2
    mov edx,%1
    mov ecx,%2
%%loop
    movq mm0,[ebx]
    movq mm1,[ebx+8]
    movq [edx],mm0
    movq [edx+8],mm1
    add edx,16
    add ebx,16
    dec ecx
    jnz %%loop
%endmacro

%macro BackupCVRMac 2
    mov edx,%1
    mov ecx,%2
%%loop
    mov eax,[ebx]
    mov [edx],eax
    add edx,4
    add ebx,4
    dec ecx
    jnz %%loop
%endmacro

%macro BackupCVRMacB 2
    mov edx,%1
    mov ecx,%2
%%loop
    mov al,[ebx]
    mov [edx],al
    inc edx
    inc ebx
    dec ecx
    jnz %%loop
%endmacro

SECTION .bss
NEWSYM CBackupPos, resd 1
NEWSYM StateBackup, resd 1
NEWSYM PBackupPos, resd 1
NEWSYM PPValue, resd 1   ; Previous PValue
NEWSYM DPValue, resd 1   ; Destination PValue
NEWSYM CurRecv, resd 1   ; Set to 1 if Recovery mode is on
; if CurRecv=1, then do not send tcp/ip data, always frame skip, do not
;   draw to screen, do not key on, restore previous local key presses,
;   when disabling key ons, divert dspmem write/read to a different
;   array temporarly, then re-copy back in when finished
NEWSYM PPContrl, resd 16   ; Previous Controller 1 Data
NEWSYM PPContrl2, resd 16   ; Previous Controller 2 Data
NEWSYM PPContrl3, resd 16   ; Previous Controller 3 Data
NEWSYM PPContrl4, resd 16   ; Previous Controller 4 Data
NEWSYM PPContrl5, resd 16   ; Previous Controller 5 Data
NEWSYM tempedx, resd 1
NEWSYM NetSent2, resd 1
NEWSYM NetQuitter, resd 1
NEWSYM QBackupPos, resd 1
NEWSYM LatencyV, resb 256
NEWSYM LatencyRecvPtr, resd 1
NEWSYM LatencySendPtr, resd 1
NEWSYM latencytimer, resd 1

SECTION .data
NEWSYM BackState, db 1
NEWSYM BackStateSize, dd 6

SECTION .bss
NEWSYM nojoystickpoll, resd 1
NEWSYM RemoteLValue, resb 1
NEWSYM LocalLValue, resb 1
NEWSYM chatstrLt, resb 15
NEWSYM RewindOldPos, resd 1
NEWSYM RewindPos, resd 1
NEWSYM RewindTimer, resd 1

SECTION .data
NEWSYM ResendTimer, dd 60

SECTION .bss
NEWSYM valuea, resd 1
NEWSYM valueb, resd 1
NEWSYM valuet, resd 1
BackupArray resd 3000
SECTION .text

NEWSYM SplitStringChat
    push ebx
    push eax
    push ecx
    mov eax,chatstrR
    call StringLength
    cmp ecx,42
    jbe near .noneed
    mov eax,42
.next2
    cmp byte[chatstrR+eax],' '
    je near .space
    cmp eax,33
    jb .dontclipearly
    dec eax
    jmp .next2
.space
    inc eax
    jmp .processclip
.dontclipearly
    mov eax,42
.processclip
    push eax
    mov ebx,[chatRTL4]
    mov [chatRTL5],ebx
    mov ebx,[chatRTL3]
    mov [chatRTL4],ebx
    mov ebx,[chatRTL2]
    mov [chatRTL3],ebx
    mov ebx,[chatRTL]
    mov [chatRTL2],ebx
    xor ecx,ecx
.chatcpyloop
    mov al,[chatstrR4+ecx]
    mov [chatstrR5+ecx],al
    mov al,[chatstrR3+ecx]
    mov [chatstrR4+ecx],al
    mov al,[chatstrR2+ecx]
    mov [chatstrR3+ecx],al
    mov al,[chatstrR+ecx]
    mov [chatstrR2+ecx],al
    inc ecx
    cmp ecx,100
    jnz .chatcpyloop
    pop eax
    push eax
    xor ecx,ecx
    mov byte[chatstrR],' '
    inc ecx
.next
    mov bl,[chatstrR2+eax]
    mov [chatstrR+ecx],bl
    inc eax
    inc ecx
    or bl,bl
    jnz .next
    pop eax
    mov byte[chatstrR2+eax],0
.noneed
    pop ecx
    pop eax
    pop ebx
    ret

NEWSYM MoveStringChat
    mov ebx,[chatRTL4]
    mov [chatRTL5],ebx
    mov ebx,[chatRTL3]
    mov [chatRTL4],ebx
    mov ebx,[chatRTL2]
    mov [chatRTL3],ebx
    mov ebx,[chatRTL]
    mov [chatRTL2],ebx
    push eax
    push ecx
    xor ecx,ecx
.chatcpyloop
    mov al,[chatstrR4+ecx]
    mov [chatstrR5+ecx],al
    mov al,[chatstrR3+ecx]
    mov [chatstrR4+ecx],al
    mov al,[chatstrR2+ecx]
    mov [chatstrR3+ecx],al
    mov al,[chatstrR+ecx]
    mov [chatstrR2+ecx],al
    inc ecx
    cmp ecx,100
    jnz .chatcpyloop
    pop ecx
    pop eax
    ret

NEWSYM GenLatencyDisplay
    call Get_Time
    mov [.temp],eax
    mov ebx,16
    xor eax,eax
    xor edx,edx
    mov al,[valuea]
    mov al,[.temp]
    div ebx
    add al,48
    add dl,48
    mov byte[chatstrLt+2],32
    cmp al,9
    jbe .bel9
;    sub al,10
.bel9
    mov [chatstrLt+1],dl
    mov [chatstrLt],al
    mov ebx,16
    xor eax,eax
    xor edx,edx
    mov al,[.temp+1]
    div ebx
    add al,48
    add dl,48
    cmp al,9
    jbe .bel9b
;    sub al,10
.bel9b
    mov [chatstrLt+4],dl
    mov [chatstrLt+3],al
    ret
SECTION .bss
.temp resd 1
SECTION .text

NEWSYM ResetExecStuff
  mov dword[soundcycleft],0
  mov dword[curexecstate],0
  mov dword[nmiprevaddrl],0
  mov dword[nmiprevaddrh],0
  mov dword[nmirept],0
  mov dword[nmiprevline],224
  mov dword[nmistatus],0
  mov byte[NextLineCache],0
  mov dword[spcnumread],0
  mov dword[spchalted],-1
  mov dword[timer2upd],0
  mov dword[HIRQCycNext],0
  mov byte[HIRQNextExe],0
  ret

NEWSYM ProcessRewind
    cmp byte[CNetType],20
    jb .okay
.notokayb
    ret
.okay
    mov eax,dword[KeyRewind]
    cmp byte[pressed+eax],1
    jne .notokayb
    mov byte[pressed+eax],2
    mov eax,[RewindOldPos]
    cmp [RewindPos],eax
    je .notokay
    dec dword[RewindPos]
    and dword[RewindPos],0Fh
    mov eax,[RewindOldPos]
    cmp [RewindPos],eax
    je .notokay2
    dec dword[RewindPos]
    and dword[RewindPos],0Fh
    mov eax,[RewindPos]
    mov [PBackupPos],eax
    push ecx
    push ebx
    call RestoreCVFrame
    ; Clear Cache Check
    mov ebx,vidmemch2
    mov ecx,4096+4096+4096
.next
    mov byte[ebx],1
    inc ebx
    dec ecx
    jnz .next
    pop ebx
    pop ecx
    inc dword[RewindPos]
    and dword[RewindPos],0Fh
    mov edx,[tempedx]
    mov dword[RewindTimer],60*3
.notokay
    ret
.notokay2
    inc dword[RewindPos]
    and dword[RewindPos],0Fh
    ret

NEWSYM UpdateRewind
;    cmp byte[OSPort],3
;    je .yeswin32
;    ret

;.yeswin32
%ifndef __MSDOS__
    push eax
    cmp dword[KeyRewind],0
    je .notftimer
    call ProcessRewind
    dec dword[RewindTimer]
    jnz .notftimer
    cmp byte[CNetType],20
    jb .okay
.notftimer
    pop eax
    ret
.okay
    mov eax,[RewindPos]
    mov [CBackupPos],eax
    mov [tempedx],edx
    push ecx
    push ebx
    call BackupCVFrame
    pop ebx
    pop ecx
    inc dword[RewindPos]
    and dword[RewindPos],0Fh
    mov eax,[RewindOldPos]
    cmp [RewindPos],eax
    jne .noteq
    inc dword[RewindOldPos]
    and dword[RewindOldPos],0Fh
.noteq
    mov dword[RewindTimer],60*3
    pop eax
%endif
    ret

NEWSYM BackupSystemVars
    pushad
    mov ebx,BackupArray
    BackupCVMacB zsmesg,[PHnum2writecpureg]
    BackupCVMac cycpbl,2
    BackupCVMacB sndrot,3019
    BackupCVMacB soundcycleft,33
    BackupCVMac spc700read,10
    BackupCVMac timer2upd,1
    BackupCVMac xa,14
    BackupCVMacB spcnumread,4
	BackupCVMacB spchalted,4
    BackupCVMac opcd,6
    BackupCVMacB HIRQCycNext,5
    BackupCVMac oamaddr,14
    BackupCVMacB prevoamptr,1
    BackupCVMac ReadHead,1
    popad
    ret

NEWSYM RestoreSystemVars
    pushad
    mov dword[RewindPos],0
    mov dword[RewindOldPos],0
    mov dword[RewindTimer],60*4
    mov ebx,BackupArray
    BackupCVRMacB zsmesg,[PHnum2writecpureg]
    BackupCVRMac cycpbl,2
    BackupCVRMacB sndrot,3019
    BackupCVRMacB soundcycleft,33
    BackupCVRMac spc700read,10
    BackupCVRMac timer2upd,1
    BackupCVRMac xa,14
    BackupCVRMacB spcnumread,4
	BackupCVRMacB spchalted,4
    BackupCVRMac opcd,6
    BackupCVRMacB HIRQCycNext,5
    BackupCVRMac oamaddr,14
    BackupCVRMacB prevoamptr,1
    BackupCVRMac ReadHead,1
    popad
    ret

NEWSYM BackupCVFrame
;NEWSYM StateBackup, dd 0
;NEWSYM CBackupPos, dd 0
;NEWSYM PBackupPos, dd 0
    push edx
    push eax
    mov ebx,[CBackupPos]
    shl ebx,19
    add ebx,[StateBackup]
    add ebx,1024

    BackupCVMacB zsmesg,[PHnum2writecpureg]
    BackupCVMac cycpbl,2
    BackupCVMacB sndrot,3019
    BackupCVMacM [wramdata],8192
    BackupCVMacM [vram],4096
    cmp byte[spcon],0
    je .nospcon
    BackupCVMacB spcRam,[PHspcsave]
    BackupCVMacM DSPMem,16
.nospcon
    cmp byte[C4Enable],1
    jne .noc4
    BackupCVMac [C4Ram],800h
.noc4
    cmp byte[SFXEnable],1
    jne .nosfx
    BackupCVMacM [sfxramdata],8192
.nosfx
    cmp byte[SA1Enable],1
    jne near .nossa1
    BackupCVMacB SA1Mode,[PHnum2writesa1reg]
    BackupCVMacM [SA1RAMArea],8192
    BackupCVMacB SA1Status,3
    BackupCVMac prevedi,1
    BackupCVMac SA1xpc,1
    BackupCVMac SA1RAMArea,6
    BackupCVMac sa1dmaptr,2
.nossa1
    cmp byte[DSP1Type],0
    je near .nodsp1type
    BackupCVMacB DSP1COp,70+128
    BackupCVMacB C4WFXVal,7*4+7*8+128
    BackupCVMacB C41FXVal,5*4+128
    BackupCVMacB Op00Multiplicand,3*4+128
    BackupCVMacB Op10Coefficient,4*4+128
    BackupCVMacB Op04Angle,4*4+128
    BackupCVMacB Op08X,5*4+128
    BackupCVMacB Op18X,5*4+128
    BackupCVMacB Op28X,4*4+128
    BackupCVMacB Op0CA,5*4+128
    BackupCVMacB Op02FX,11*4+3*4+28*8+128
    BackupCVMacB Op0AVS,5*4+14*8+128
    BackupCVMacB Op06X,6*4+10*8+4+128
    BackupCVMacB Op01m,4*4+128
    BackupCVMacB Op0DX,6*4+128
    BackupCVMacB Op03F,6*4+128
    BackupCVMacB Op14Zr,9*4+128
    BackupCVMacB Op0EH,4*4+128
.nodsp1type
    BackupCVMacB soundcycleft,33
    BackupCVMac spc700read,10
    BackupCVMac timer2upd,1
    BackupCVMac xa,14
    BackupCVMacB spcnumread,4
	BackupCVMacB spchalted,4
    BackupCVMac opcd,6
    BackupCVMacB HIRQCycNext,5
    BackupCVMac oamaddr,14
    BackupCVMacB prevoamptr,1
    BackupCVMac ReadHead,1

    mov edx,[sram]
    mov ecx,[ramsize]
    shr ecx,4
    or ecx,ecx
    jz .end
.loop
    movq mm0,[edx]
    movq mm1,[edx+8]
    movq [ebx],mm0
    movq [ebx+8],mm1
    add edx,16
    add ebx,16
    dec ecx
    jnz .loop
.end

    pop eax
    pop edx
    mov [ebx],esi
    mov [ebx+4],edi
    mov ecx,[tempedx]
    mov [ebx+8],ecx
    mov [ebx+12],ebp
    emms
    ret

NEWSYM RestoreCVFrame
    push edx
    push eax
    mov ebx,[PBackupPos]
    shl ebx,19
    add ebx,[StateBackup]
    add ebx,1024

    BackupCVRMacB zsmesg,[PHnum2writecpureg]
    BackupCVRMac cycpbl,2
    BackupCVRMacB sndrot,3019
    BackupCVRMacM [wramdata],8192
    BackupCVRMacM [vram],4096
    cmp byte[spcon],0
    je .nospcon
    BackupCVRMacB spcRam,[PHspcsave]
    BackupCVRMacM DSPMem,16
.nospcon
    cmp byte[C4Enable],1
    jne .noc4
    BackupCVRMac [C4Ram],800h
.noc4
    cmp byte[SFXEnable],1
    jne .nosfx
    BackupCVRMacM [sfxramdata],8192
.nosfx
    cmp byte[SA1Enable],1
    jne near .nossa1
    BackupCVRMacB SA1Mode,[PHnum2writesa1reg]
    BackupCVRMacM [SA1RAMArea],8192
    BackupCVRMacB SA1Status,3
    BackupCVRMac prevedi,1
    BackupCVRMac SA1xpc,1
    BackupCVRMac SA1RAMArea,6
    BackupCVRMac sa1dmaptr,2
.nossa1
    cmp byte[DSP1Type],0
    je near .nodsp1type
    BackupCVRMacB DSP1COp,70+128
    BackupCVRMacB C4WFXVal,7*4+7*8+128
    BackupCVRMacB C41FXVal,5*4+128
    BackupCVRMacB Op00Multiplicand,3*4+128
    BackupCVRMacB Op10Coefficient,4*4+128
    BackupCVRMacB Op04Angle,4*4+128
    BackupCVRMacB Op08X,5*4+128
    BackupCVRMacB Op18X,5*4+128
    BackupCVRMacB Op28X,4*4+128
    BackupCVRMacB Op0CA,5*4+128
    BackupCVRMacB Op02FX,11*4+3*4+28*8+128
    BackupCVRMacB Op0AVS,5*4+14*8+128
    BackupCVRMacB Op06X,6*4+10*8+4+128
    BackupCVRMacB Op01m,4*4+128
    BackupCVRMacB Op0DX,6*4+128
    BackupCVRMacB Op03F,6*4+128
    BackupCVRMacB Op14Zr,9*4+128
    BackupCVRMacB Op0EH,4*4+128
.nodsp1type
    BackupCVRMacB soundcycleft,33
    BackupCVRMac spc700read,10
    BackupCVRMac timer2upd,1
    BackupCVRMac xa,14
    BackupCVRMacB spcnumread,4
	BackupCVRMacB spchalted,4
    BackupCVRMac opcd,6
    BackupCVRMacB HIRQCycNext,5
    BackupCVRMac oamaddr,14
    BackupCVRMacB prevoamptr,1
    BackupCVRMac ReadHead,1

    mov edx,[sram]
    mov ecx,[ramsize]
    shr ecx,4
    or ecx,ecx
    jz .end
.loop
    movq mm0,[ebx]
    movq mm1,[ebx+8]
    movq [edx],mm0
    movq [edx+8],mm1
    add edx,16
    add ebx,16
    dec ecx
    jnz .loop
.end

    pop eax
    pop edx
    mov esi,[ebx]
    mov edi,[ebx+4]
    mov ecx,[ebx+8]
    mov [tempedx],ecx
    mov ebp,[ebx+12]
    call UpdateDPage
    call SA1UpdateDPage
    emms
    ret

SECTION .bss
NEWSYM MuteVoiceF, resb 0
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

NetSaveState:
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
    cmp byte[CNetType],20
    je .skipsoundreinit
    test byte[pressed+eax],1
    jnz .soundreinit
    mov eax,[KeyLoadState]
    test byte[pressed+eax],1
    jz .skipsoundreinit
.soundreinit
    mov byte[NoSoundReinit],1
    mov byte[csounddisable],1
.skipsoundreinit

    call statesaver

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
;    cmp byte[CNetType],20
;    je .skipmovie
;    cmp byte[CNetType],21
;    je .skipmovie
;    jmp .skipmovie
;    cmp byte[MovieProcessing],0
;    jne .movie
;.skipmovie
    cmp byte[spcon],0
    je .nosoundta
    mov edi,[tableadc+ebx*4]
    or byte[curexecstate],2
    jmp .soundta
.nosoundta
    mov edi,[tableadb+ebx*4]
    and byte[curexecstate],0FDh
.soundta
    jmp .nomovie
.movie
    mov edi,[tableadc+ebx*4]
    test byte[curexecstate],2
    jnz .nomovie
    mov edi,[tableadb+ebx*4]
.nomovie

    mov ebp,[spcPCRam]

    mov byte[NoSoundReinit],0
    mov byte[csounddisable],0
    mov byte[NextNGDisplay],0

    call splitflags
    ret

%macro stim 0
;    cmp byte[OSPort],1
;    ja %%nosti
%ifdef __MSDOS__
    sti
%endif
;%%nosti
%endmacro

%macro clim 0
;    cmp byte[OSPort],1
;    ja %%nocli
%ifdef __MSDOS__
    cli
%endif
;%%nocli
%endmacro

%macro ProcessIRQStuffB 0
    ; check for VIRQ/HIRQ/NMI
    test dl,04h
    jnz %%noirq
    test byte[INTEnab],20h
    jz %%novirq
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    je near .virq
    jmp %%noirq
%%novirq
    test byte[INTEnab],10h
    jnz near .virq
%%noirq
    test byte[INTEnab],20h
    jz %%novirq2b
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    jne %%novirq2b
    cmp byte[intrset],1
    jne %%nointrset2b
    mov byte[intrset],2
%%nointrset2b
%%novirq2b
%endmacro

%macro ProcessIRQStuffC 0
    ; check for VIRQ/HIRQ
    cmp byte[virqnodisable],1
    je %%virqdo
    test dl,04h
    jnz %%virqdo
    cmp byte[doirqnext],1
    je near .virq
%%virqdo
    test byte[INTEnab],20h
    jz near %%novirq
    mov ax,[VIRQLoc]
    add ax,[IRQHack]
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
%%startirq
    cmp byte[intrset],1
    jne %%nointrseta
    mov byte[intrset],2
%%nointrseta
    mov byte[irqon],80h
    test dl,04h
    jnz %%irqd
    test byte[INTEnab],10h
    jnz %%tryhirq
    jmp .virq
%%novirq
    test byte[INTEnab],10h
    jz %%noirq
%%tryhirq
    jmp .virq
    jmp %%startirq
%%irqd
    mov byte[doirqnext],1
%%noirq
%endmacro

%macro ProcessIRQStuff 0
    ; check for VIRQ/HIRQ
    cmp byte[virqnodisable],1
    je %%virqdo
    test dl,04h
    jnz %%virqdo
    cmp byte[doirqnext],1
    je near .virq
%%virqdo
    test byte[INTEnab],20h
    jz near %%novirq
    mov ax,[VIRQLoc]
    add ax,[IRQHack]
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
    cmp byte[hirqmode2],1
    je %%hirqchange
    mov byte[intrset],8
    jmp %%noirq
%%hirqchange
    mov byte[intrset],3
    jmp %%setagain
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

SECTION .bss
NEWSYM romloadskip, resb 1
NEWSYM abcdefg,     resd 1
NEWSYM abcdefg1,    resd 1
NEWSYM abcdefg2,    resd 1
NEWSYM abcdefg3,    resd 1
NEWSYM SSKeyPressed, resd 1
NEWSYM SPCKeyPressed, resd 1
NEWSYM NoSoundReinit, resd 1
NEWSYM NextNGDisplay, resb 1
NEWSYM TempVidInfo, resd 1


NEWSYM tempdh, resb 1

SECTION .text

NEWSYM start65816

    call initvideo

    cmp byte[videotroub],1
    jne .notrouble
    ret
.notrouble

;    cmp byte[OSPort],2
;    jae .nonewgfxcheck
    jmp .nonewgfxcheck
    cmp byte[cbitmode],1
    jne .nonewgfxcheck
    cmp byte[newengen],1
    jne .nonewgfxcheck
    cmp byte[cvidmode],3
    jne .nocorrectmode
    cmp byte[newgfx16b],1
    je .nonewgfxcheck
    jmp .correctmode
.nocorrectmode
    mov dword[Msgptr],newgfxerror2
    jmp .correctmode
    mov dword[Msgptr],newgfxerror
.correctmode
    mov eax,[MsgCount]
    mov [MessageOn],eax
    mov byte[newengen],0
.nonewgfxcheck
    mov edi,[vidbufferofsa]
    mov ecx,37518
    xor eax,eax
    rep stosd
;    mov edi,[vidbufferofsb]
;    mov ecx,37518
;    xor eax,eax
;    rep stosd
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
    mov byte[debuggeron],0
    mov byte[exiter],0

    call InitPreGame
    jmp reexecute

NEWSYM continueprognokeys
    mov byte[romloadskip],0
    mov byte[debuggeron],0
    mov byte[exiter],0

    call InitPreGame
    jmp reexecutenokeys

; Incorrect

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
reexecutenokeys
    jmp reexecuteb2

NEWSYM reexecuteb
    ;cmp byte[OSPort],1
    ;ja reexecuteb2
%ifdef __MSDOS__
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
%endif 
reexecuteb2:
    ; temporary sprite displayer
;    mov edx,.sdispname
;    call Open_File
;    jc .failedsd
;    mov bx,ax
;    mov ecx,544
;    mov edx,oamram
;    call Read_File
;    call Close_File
;.failedsd
;    jmp .skipsd
;.sdispname db 'MMX3.SPR',0
;.skipsd

    cmp byte[NoSoundReinit],1
    je .skippregame
    call SetupPreGame
.skippregame

    ; initialize variables (Copy from variables)
    call UpdateDPage
    call SA1UpdateDPage
    call Makemode7Table
    call ReadSPC7110log
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
;    cmp byte[CNetType],20
;    je .skipmovie
;    cmp byte[CNetType],21
;    je .skipmovie
;    jmp .skipmovie
;    cmp byte[MovieProcessing],0
;    jne .movie
;.skipmovie
    cmp byte[spcon],0
    je .nosoundta
    mov edi,[tableadc+ebx*4]
    or byte[curexecstate],2
    jmp .soundta
.nosoundta
    mov edi,[tableadb+ebx*4]
    and byte[curexecstate],0FDh
.soundta
    jmp .nomovie
.movie
    mov edi,[tableadc+ebx*4]
    test byte[curexecstate],2
    jnz .nomovie
    mov edi,[tableadb+ebx*4]
.nomovie

    mov ebp,[spcPCRam]

    mov byte[NoSoundReinit],0
    mov byte[csounddisable],0
    mov byte[NextNGDisplay],0
    mov byte[NetPlayNoMore],1

    call splitflags

;    cmp byte[MovieProcessing],0
;    jne .movie2
    call execute
    jmp .nomovie2
.movie2
    call cpuover.returntoloop
.nomovie2

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
    cmp byte[CNetType],20
    je .skipsoundreinit
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

    call WriteSPC7110log

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

;    mov edi,memtabler8+40h*4
;    mov ecx,30h
;    mov eax,memaccessbankr848mb
;    rep stosd
;    mov edi,memtabler16+40h*4
;    mov ecx,30h
;    mov eax,memaccessbankr1648mb
;    rep stosd

    cmp byte[nextmenupopup],1
    je near showmenu
    cmp byte[ReturnFromSPCStall],1
    je near .activatereset
    mov eax,[KeySaveState]
    cmp byte[CNetType],20
    je .net
    test byte[pressed+eax],1
    jnz near savestate
    mov eax,[KeyLoadState]
    test byte[pressed+eax],1
    jnz near loadstate
    cmp byte[SSKeyPressed],1
    je near showmenu
    cmp byte[SPCKeyPressed],1
    je near showmenu
    cmp byte[debugdisble],0
    jne .nodebugger
    test byte[pressed+59],1
    jne near startdebugger
.nodebugger
    test byte[pressed+59],1
    jne near showmenu
    mov eax,[KeyQuickRst]
.net
    test byte[pressed+eax],1
    jz .noreset
.activatereset
    pushad
    call GUIDoReset
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

SECTION .data
NEWSYM EndMessage
db '                                                                   ',13,10,0
SECTION .text

NEWSYM endprog
    call deinitvideo

    cmp byte[previdmode],3
    jne .noendmessage
    mov byte[EndMessage+13],','
    mov eax,[welcome+9]
    mov [EndMessage+9],eax
    mov ax,[welcome+7]
    mov [EndMessage+7],ax
    mov edx,EndMessage
    call PrintStr
.noendmessage

;    mov eax,[opcd]
;    mov eax,[numinst]          ;Temporary
;    mov eax,[NumBRRconv]
;    call printnum
    ; save sram

    ; change to sram dir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir

    EXTSYM SDD1Array,SDD1Entry,SDD1Sort
    call SDD1Sort
;    jmp .nodecomppack
    cmp byte[SDD1Enable],0
    jne .yesdecomppack
    cmp dword[SDD1Entry],0
    je .nodecomppack
.yesdecomppack
    mov edx,sdd1fname
    call Create_File
    mov bx,ax
    mov edx,SDD1Array
    mov ecx,[SDD1Entry]
    call Write_File
    call Close_File
.nodecomppack

    cmp byte[sramsavedis],1
    je .nosram
    cmp dword[ramsize],0
    je .nosram
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    xor esi,esi
    xor edi,edi
    mov edx,fnames+1
    call Create_File
    jc .nosram
    mov bx,ax
    xor ecx,ecx
    mov ecx,[ramsize]
    mov edx,[sram]
    call Write_File
    call Close_File
.nosram
    cmp byte[SFXSRAM],0
    je .nosfxsram
    mov edx,fnames+1
    call Create_File
    jc .nosfxsram
    mov bx,ax
    mov ecx,65536
    mov edx,[sfxramdata]
    call Write_File
    call Close_File
.nosfxsram
    cmp byte[SA1Enable],1
    jne .nosa1
    mov edx,fnames+1
    call Create_File
    jc .nosa1
    mov bx,ax
    mov ecx,65536*2
    mov edx,[SA1RAMArea]
    call Write_File
    call Close_File
.nosa1

    ; change dir to InitDrive/InitDir
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir

    call createnewcfg
    call GUISaveVars

    cmp byte[MovieProcessing],0
    je .nomovieclose
    mov bx,[MovieFileHand]
    mov byte[MovieProcessing],0
    call Close_File
.nomovieclose
;NEWSYM tempstore, times 1024*1024 db 0
;    mov esi,tempstore
;    mov ecx,1024*1024
;.loop
;    mov al,[esi]
;    cmp al,0
;    jne .faulty
;    inc esi
;    dec ecx
;    jnz .loop
;    xor eax,eax
;    jmp .notfaulty
;.faulty
;    mov eax,1
;.notfaulty
;    mov eax,[vidbufferofsa]
;    call printnum
;    mov dl,32
;    mov ah,02h
;    call Output_Text
;    mov eax,[vidbufferofsb]
;    call printnum
;    mov dl,32
;    mov ah,02h
;    call Output_Text
;    mov eax,[romdata]
;    call printnum

    ; change dir to InitDrive/InitDir
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir

%ifdef __MSDOS__
    ; Deinit modem if necessary
    cmp byte[ModemInitStat],0
    je .nodeinitmodem
    call DeInitModem
.nodeinitmodem
;    cmp byte[OSPort],1
;    jae .nodeinitipx
    call deinitipx
;.nodeinitipx
%endif
    jmp OSExit
SECTION .data
NEWSYM sdd1fname, db 'sdd1dat.dat',0,0
SECTION .text

NEWSYM interror
    stim
    call deinitvideo
    mov edx,.nohand         ;use extended
    mov ah,9                ;DOS- API
    call Output_Text                 ;to print a string
    jmp DosExit

SECTION .data
.nohand db 'Cannot process interrupt handler!',13,10,0

SECTION .bss
; global variables
NEWSYM invalid, resb 1
NEWSYM invopcd, resb 1
NEWSYM pressed, resb 256+128+64          ; keyboard pressed keys in scancode
NEWSYM exiter, resb 1
NEWSYM oldhand9o, resd 1
NEWSYM oldhand9s, resw 1
NEWSYM oldhand8o, resd 1
NEWSYM oldhand8s, resw 1
NEWSYM opcd,      resd 1
NEWSYM pdh,       resd 1
NEWSYM pcury,     resd 1
NEWSYM timercount, resd 1
NEWSYM initaddrl, resd 1                  ; initial address location
NEWSYM NetSent, resd 1
NEWSYM nextframe, resd 1                  ; tick count for timer
NEWSYM curfps,    resb 1                  ; frame/sec for current screen
NEWSYM SFXSRAM,   resb 1

SECTION .data
NEWSYM newgfxerror, db 'NEED MEMORY FOR GFX ENGINE',0
NEWSYM newgfxerror2, db 'NEED 320x240 FOR NEW GFX 16B',0
;newgfxerror db 'NEW GFX IN 16BIT IS N/A',0

SECTION .bss
NEWSYM HIRQCycNext,   resd 1
NEWSYM HIRQNextExe,   resb 1
SECTION .text

;*******************************************************
; Save/Load States
;*******************************************************

NEWSYM sramsave
    mov byte[pressed+1],0
    mov byte[pressed+41],0
    ; save sram
    cmp byte[sramsavedis],1
    je .nosram
    cmp word[ramsize],0
    je .nosram
    mov edx,fnames+1
    call Create_File
    jc .nosram
    mov bx,ax
    xor ecx,ecx
    mov cx,[ramsize]
    mov edx,[sram]
    call Write_File
    call Close_File
    mov dword[Msgptr],.savesrmmsg1
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp reexecute
.nosram
    mov dword[Msgptr],.savesrmmsg2
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp reexecute

SECTION .data
.savesrmmsg1 db 'SRAM DATA SAVED.',0
.savesrmmsg2 db 'NO SRAM DATA.',0

SECTION .bss
NEWSYM firstsaveinc, resb 1
SECTION .text

NEWSYM statesaver
    clim

    sub dword[Curtableaddr],tableA
    sub dword[spcPCRam],spcRam
    sub dword[spcRamDP],spcRam
    call PrepareSaveState
    call unpackfunct

    ; Auto increment save state slot

    cmp byte[AutoIncSaveSlot],0
    je .donesaveinc
    cmp byte[firstsaveinc],1
    je .clearfirstinc
    mov eax,[statefileloc]
    mov dh,[fnamest+eax]
    cmp dh,'t'
    je .secondstate
    cmp dh,'9'
    je .jumptofirststate
    inc dh
    jmp .donextstate
.secondstate
    mov dh,'1'
    jmp .donextstate
.jumptofirststate
    mov dh,'t'
.donextstate
    mov byte[fnamest+eax],dh
    xor dh,dh
    jmp .donesaveinc
.clearfirstinc
    mov byte[firstsaveinc],0
.donesaveinc

;    jmp .skipsaves
    ; Save State
%ifdef __LINUX__
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
%endif
    mov edx,fnamest+1
    call Create_File
    jc near .nosavestuff
    ; Save 65816 status, etc.
    mov bx,ax
    mov ecx,[PHnum2writecpureg]
    mov edx,zsmesg
    call Write_File
    mov ecx,8
    mov edx,cycpbl
    call Write_File
    ; Save SNES PPU Register status
    mov ecx,3019
    mov edx,sndrot
    call Write_File
    ; Save RAM (WRAM(128k),VRAM(64k),SRAM)
;    xor ecx,ecx
;    mov cx,[ramsize]
    mov ecx,65536+65536
    mov edx,[wramdata]
    call Write_File
    mov ecx,65536
    mov edx,[vram]
    call Write_File
    cmp byte[spcon],0
    je .nospcon
    ; Save SPC stuff
    mov ecx,[PHspcsave]
    mov edx,spcRam
    call Write_File
    ; Save DSP stuff
    mov ecx,[PHdspsave]
    mov edx,BRRBuffer
    call Write_File
    ; Save DSP Mem
    mov ecx,256
    mov edx,DSPMem
    call Write_File
.nospcon
    cmp byte[C4Enable],1
    jne .noc4
    mov ecx,2000h
    mov edx,[C4Ram]
    call Write_File
.noc4
    cmp byte[SFXEnable],1
    jne .nosfx
    mov ecx,[SfxCROM]
    sub [SfxRomBuffer],ecx
    mov ecx,[SfxRAMMem]
    sub [SfxLastRamAdr],ecx
    mov ecx,65536*2
    mov edx,[sfxramdata]
    call Write_File
    ; uncomment the following
    mov ecx,[PHnum2writesfxreg]
    mov edx,SfxR0
    call Write_File
    mov ecx,[SfxCROM]
    add [SfxRomBuffer],ecx
    mov ecx,[SfxRAMMem]
    add [SfxLastRamAdr],ecx
.nosfx
    cmp byte[SPC7110Enable],1
    jne .nospc7110
    mov edx,[romdata]
    add edx,510000h
    mov ecx,65536
    call Write_File
    mov edx,SPCMultA
    mov ecx,[PHnum2writespc7110reg]
    call Write_File
.nospc7110
    cmp byte[SA1Enable],1
    jne .nossa1
    ; Convert SA-1 stuff to standard, non displacement format
    call SaveSA1
    mov ecx,[PHnum2writesa1reg]
    mov edx,SA1Mode
    call Write_File
    mov ecx,65536*2
    mov edx,[SA1RAMArea]
    call Write_File
    ; Convert back SA-1 stuff
    call RestoreSA1
.nossa1
    cmp byte[cbitmode],0
    je .nocapturepicture
    cmp byte[NoPictureSave],1
    je .nocapturepicture
    call CapturePicture
    mov ecx,64*56*2
    mov edx,PrevPicture
    call Write_File
.nocapturepicture
    call Close_File
    add dword[Curtableaddr],tableA
    add dword[spcPCRam],spcRam
    add dword[spcRamDP],spcRam
    call ResetState
    stim

    ; Get the state number
    mov ebx,[statefileloc]
    mov cl,[fnamest+ebx]
    cmp cl,'t'
    jne .writewhichstate
    mov cl,'0'
.writewhichstate
    mov [.savemsg+6],cl
	
    mov dword[Msgptr],.savemsg
    mov eax,[MsgCount]
    mov [MessageOn],eax
    ret

.nosavestuff
    add dword[Curtableaddr],tableA
    add dword[spcPCRam],spcRam
    add dword[spcRamDP],spcRam
    call ResetState
    stim
    mov dword[Msgptr],.savemsgfail
    mov eax,[MsgCount]
    mov [MessageOn],eax
    ret

SECTION .data
.savemsg db 'STATE - SAVED.',0
.savemsgfail db 'UNABLE TO SAVE.',0
SECTION .text

NEWSYM savestate
    jmp .save
    mov byte[pressed+1],0
    mov byte[pressed+60],0
    mov edx,.fname2+1
    call Create_File
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    mov edi,[vidbuffer]
    mov dl,[resolutn]
    mov bx,ax
    ; move data to a linear address from esi to edi
.loopa
    mov ecx,128
    add esi,64+128*4
    dec dl
    jnz .loopa
    push edx
    push esi
    mov ecx,128*4
    mov edx,esi
    call Write_File
    pop esi
    pop edx
    add esi,64+128*4
    call Close_File
    jmp reexecuteb
.save
    mov byte[pressed+1],0
    mov eax,[KeySaveState]
    mov byte[pressed+eax],2
    call statesaver
    jmp reexecuteb

SECTION .data
.fname2 db 9,'image.dat',0

SECTION .bss
cycpblblah resd 2
SECTION .text

    ; Load State
NEWSYM stateloader
    mov byte[MovieProcessing],0
    mov byte[prevoamptr],0FFh
    ; Load 65816 status, etc.
    mov bx,ax
    mov ecx,[PHnum2writecpureg]
    add dword[Totalbyteloaded],ecx
    mov edx,zsmesg
    call Read_File
    cmp byte[versn],60
    jb near .convert
    ; Load SPC timers
    mov ecx,8
    add dword[Totalbyteloaded],ecx
    mov edx,cycpbl
    call Read_File
    ; Load SNES PPU Register status
    mov ecx,3019    ; 3019
    add dword[Totalbyteloaded],ecx
    mov edx,sndrot
    call Read_File
    cmp byte[versn],60
    jne .not60
    mov byte[ioportval],0FFh
.not60
    ; Load RAM (WRAM(128k),VRAM(64k),SRAM)
    mov ecx,65536+65536
    add dword[Totalbyteloaded],ecx
    mov edx,[wramdata]
    call Read_File
    mov ecx,65536
    add dword[Totalbyteloaded],ecx
    mov edx,[vram]
    call Read_File
    cmp byte[spcon],0
    je .nospcon
    ; Load SPC stuff
    mov ecx,[PHspcsave]
    add dword[Totalbyteloaded],ecx
    mov edx,spcRam
    call Read_File
    ; Load DSP stuff
    mov ecx,[PHdspsave]
    add dword[Totalbyteloaded],ecx
    mov edx,BRRBuffer
    call Read_File
    ; Load DSP Mem
    mov ecx,256
    add dword[Totalbyteloaded],ecx
    mov edx,DSPMem
    call Read_File
.nospcon
    cmp byte[SFXEnable],1
    jne near .nosfx
    mov ecx,65536*2
    add dword[Totalbyteloaded],ecx
    mov edx,[sfxramdata]
    call Read_File

    mov ecx,[PHnum2writesfxreg]
    add dword[Totalbyteloaded],ecx
    mov edx,SfxR0
    call Read_File

    xor ecx,ecx
    mov cl,[SfxPBR]
    mov ecx,[SfxMemTable+ecx*4]
    mov [SfxCPB],ecx

    xor ecx,ecx
    mov cl,[SfxROMBR]
    mov ecx,[SfxMemTable+ecx*4]
    mov [SfxCROM],ecx

    xor ecx,ecx
    mov cl,[SfxRAMBR]
    shl ecx,16
    add ecx,[sfxramdata]
    mov dword [SfxRAMMem],ecx

    mov ecx,[SfxCROM]
    add [SfxRomBuffer],ecx
    mov ecx,[SfxRAMMem]
    add [SfxLastRamAdr],ecx

.nosfx

    cmp byte[C4Enable],1
    jne .noc4
    mov ecx,2000h
    add dword[Totalbyteloaded],ecx
    mov edx,[C4Ram]
    call Read_File
.noc4
    cmp byte[SPC7110Enable],1
    jne .nospc7110
    mov edx,[romdata]
    add edx,510000h
    mov ecx,65536
    call Read_File
    mov edx,SPCMultA
    mov ecx,[PHnum2writespc7110reg]
    call Read_File
.nospc7110
    cmp byte[SA1Enable],1
    jne .nossa1
    mov ecx,[PHnum2writesa1reg]
    add dword[Totalbyteloaded],ecx
    mov edx,SA1Mode
    call Read_File
    mov ecx,65536*2
    add dword[Totalbyteloaded],ecx
    mov edx,[SA1RAMArea]
    call Read_File
    ; Convert back SA-1 stuff
    push ebx
    call RestoreSA1
    pop ebx
    call SA1UpdateDPage
.nossa1
    cmp byte[SDD1Enable],1
    jne .nosdd1
    call UpdateBanksSDD1
.nosdd1
    call Close_File
    call repackfunct
    mov dword[spcnumread],0
	mov dword[spchalted],-1
    mov byte[nexthdma],0

;    call headerhack

    call initpitch
    ret

.convert
    ; Load SNES PPU Register status
    mov ecx,3019
    mov edx,sndrot
    call Read_File
    ; Load RAM (WRAM(128k),VRAM(64k),SRAM)
    mov dword[cycpbl],0
    mov dword[cycpblt],0
    mov ah,[cycpbl2]
    mov [cycpbl],ah
    mov ah,[cycpblt2]
    mov [cycpblt],ah

    mov ecx,[ramsize]
    add ecx,65536+65536+65536
    mov edx,[wramdata]
    call Read_File
    cmp byte[spcon],0
    je .nospconb
    ; Load SPC stuff
    mov ecx,[PHspcsave]
    mov edx,spcRam
    call Read_File
    ; Load DSP stuff
    mov ecx,32
    mov edx,BRRBuffer
    call Read_File
    ; Convert old to new data
    ; read/write 6, jump 2 for 8 times
    mov edx,BRRPlace0
    mov ecx,8
.loopconv
    push edx
    push ecx
    mov ecx,4
    call Read_File
    pop ecx
    pop edx
    add edx,8
    dec ecx
    jnz .loopconv
;dspsave equ $-BRRBuffer
;dspconvb equ $-Voice0Freq
    ; Load DSP stuff
    mov ecx,[PHdspsave]
    sub ecx,64+32
    sub ecx,8
    mov edx,Voice0Freq
    call Read_File
    ; Load DSP Mem
    mov ecx,256
    mov edx,DSPMem
    call Read_File
.nospconb
    call Close_File
    call repackfunct
    mov dword[cycpbl],0
    mov dword[spcnumread],0
	mov dword[spchalted],-1
    mov byte[nexthdma],0
    call headerhack
    call initpitch
    ret
NEWSYM loadstate
    mov byte[pressed+1],0
    mov eax,[KeyLoadState]
    mov byte[pressed+eax],2
    mov byte[multchange],1
    clim
%ifdef __LINUX__
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
%endif
    ; Get the state number
    mov ebx,[statefileloc]
    mov cl,[fnamest+ebx]
    cmp cl,'t'
    jne .writewhichstate
    mov cl,'0'
.writewhichstate
    mov [.loadmsg+6],cl
    mov [.convmsg+6],cl
    mov [.nfndmsg+21],cl
    mov edx,fnamest+1
    call Open_File
    jc near .nofile
    call stateloader

    ; Clear Cache Check
    mov esi,vidmemch2
    mov ecx,4096+4096+4096
.next
    mov byte[esi],1
    inc esi
    dec ecx
    jnz .next
    cmp byte[versn],60
    jne near .convert
    mov dword[Msgptr],.loadmsg
    jmp .noconvert
.convert
    mov dword[Msgptr],.convmsg
    mov byte[versn],60
    mov byte[versn-2],'6'
.noconvert
    mov eax,[MsgCount]
    mov [MessageOn],eax
    add dword[Curtableaddr],tableA
    add dword[spcPCRam],spcRam
    add dword[spcRamDP],spcRam
    call ResetState
    call procexecloop
    stim
    jmp reexecuteb
.nofile
    mov dword[Msgptr],.nfndmsg
    mov eax,[MsgCount]
    mov [MessageOn],eax
    stim
    jmp reexecuteb

SECTION .data
.loadmsg db 'STATE - LOADED.',0
.convmsg db 'STATE - LOADED/CONVERTED',0
.nfndmsg db 'UNABLE TO LOAD STATE -.',0
SECTION .text

NEWSYM loadstate2
    mov edx,fnamest+1
NEWSYM loadstate3
    call Open_File
    jc near .nofile
    mov dword[Totalbyteloaded],0
    call stateloader

    ; Clear Cache Check
    mov esi,vidmemch2
    mov ecx,4096+4096+4096
.next
    mov byte[esi],1
    inc esi
    dec ecx
    jnz .next
    add dword[Curtableaddr],tableA
    add dword[spcPCRam],spcRam
    add dword[spcRamDP],spcRam
    call ResetState
    call procexecloop
    ret
.nofile
    ret

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
    out 40H,al
    ret
.dopal
    mov al,00110110b
    out 43h,al
    mov ax,23863        ; 65536/(50/((65536*24+175)/(60*60*24)))
    mov dword[timercount],23863
    out 40h,al
    mov al,ah
    out 40H,al
    ret

NEWSYM init18_2hz
    mov al,00110110b
    out 43H,al
    mov ax,0
    mov dword[timercount],65536
    out 40H,al
    mov al,ah
    out 40H,al
    ret

NEWSYM Game60hzcall
    inc word[t1cc]
    inc byte[nextframe]
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
SECTION .bss
NEWSYM t1cc,     resw 1
SECTION .text

;*******************************************************
; Int 09h vector
;*******************************************************

SECTION .bss
NEWSYM skipnextkey42, resb 1
SECTION .text

%ifdef __MSDOS__
NEWSYM handler9h
    cli
    push ds
    push eax
    push ebx
    mov ax,[cs:dssel]
    mov ds,ax
    xor ebx,ebx
    in al,60H                 ; get keyboard scan code
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
    mov al,20H                ; turn off interrupt mode
    out 20H,al
    pop ebx                          ; Pop registers off
    pop eax                          ; stack in correct
    pop ds
    sti
    iretd
%endif

SECTION .bss ;ALIGN=32
NEWSYM soundcycleft, resd 1
NEWSYM curexecstate, resd 1

NEWSYM nmiprevaddrl, resd 1       ; observed address -5
NEWSYM nmiprevaddrh, resd 1       ; observed address +5
NEWSYM nmirept,      resd 1       ; NMI repeat check, if 6 then okay

SECTION .data
NEWSYM nmiprevline,  dd 224     ; previous line

SECTION .bss
NEWSYM nmistatus,    resd 1       ; 0 = none, 1 = waiting for nmi location,
                        ; 2 = found, disable at next line
NEWSYM joycontren,   resd 1       ; joystick read control check
NEWSYM NextLineCache, resb 1
NEWSYM NetQuit, resb 1
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

%macro NetHelpExecSend 1
    cmp byte[pl1neten+%1],1
    jne %%nopl
    mov eax,[ecx]
    mov [cnetplaybuf+ebx],al
    inc ebx
    and ebx,1FFh
    mov [cnetplaybuf+ebx],ah
    inc ebx
    and ebx,1FFh
    ror eax,16
    mov [cnetplaybuf+ebx],al
    inc ebx
    and ebx,1FFh
    mov [cnetplaybuf+ebx],ah
    inc ebx
    and ebx,1FFh
    ror eax,16
    call RemoteSendEAX
    add ecx,4
%%nopl
%endmacro

%macro NetHelpExecRecv 1
    cmp byte[pl1neten+%1],2
    jne %%nopl
    call RemoteGetEAX
    mov [ecx],eax
%%nopl
    add ecx,4
%endmacro

%macro NetHelpExecRecv2 1
    cmp byte[pl1neten+%1],1
    jne %%nopl
    mov al,[cnetplaybuf+ebx]
    inc ebx
    and ebx,1FFh
    mov ah,[cnetplaybuf+ebx]
    inc ebx
    and ebx,1FFh
    ror eax,16
    mov al,[cnetplaybuf+ebx]
    inc ebx
    and ebx,1FFh
    mov ah,[cnetplaybuf+ebx]
    inc ebx
    and ebx,1FFh
    ror eax,16
    mov [ecx],eax
%%nopl
    add ecx,4
%endmacro

;*******************************************************
; 65816 execution
;*******************************************************

SECTION .data

SpeedHackSafeTable
       db 1,0,1,0,0,0,1,0,1,0,1,1,0,0,0,0
       db 0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0
       db 0,0,0,0,0,0,1,0,1,0,1,1,0,0,1,0
       db 0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0
       db 0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1
       db 0,1,1,1,0,1,1,1,0,1,1,0,0,0,1,0
       db 0,1,0,1,0,1,1,1,0,1,1,0,0,1,1,1
       db 0,1,1,1,0,1,1,1,0,1,0,0,0,1,1,1
       db 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
       db 0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0
       db 0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0
       db 0,1,0,1,0,1,1,1,1,1,0,0,0,1,1,1
       db 0,1,1,1,0,1,1,1,0,1,0,0,0,1,1,1
SECTION .text

NEWSYM exitloop2
   mov byte[ExecExitOkay],0
NEWSYM exitloop
   ret
   cmp byte[nextmenupopup],1
   je near .okay
   cmp byte[ExecExitOkay],0
   je near .okay
   mov byte[pressed+1],0
   mov byte[pressed+59],0
   mov eax,[KeySaveState]
   mov byte[pressed+eax],0
   mov eax,[KeyLoadState]
   mov byte[pressed+eax],0
   mov eax,[KeyQuickExit]
   mov byte[pressed+eax],0
   mov eax,[KeyQuickLoad]
   mov byte[pressed+eax],0
   mov eax,[KeyQuickRst]
   mov byte[pressed+eax],0
   mov byte[ExecExitOkay],5
   mov eax,[KeyQuickSnapShot]
   mov byte[pressed+eax],0
   mov eax,[KeyQuickClock]
   mov byte[pressed+eax],0
   mov eax,[KeyQuickSaveSPC]
   mov byte[pressed+eax],0
   mov byte[SSKeyPressed],0
   mov byte[SPCKeyPressed],0
   jmp cpuover.returntoloop
.okay
   mov byte[ExecExitOkay],5
   ret

ALIGN16

%macro FlipCheck 0
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
%endmacro

NEWSYM execute
NEWSYM execloop
   mov bl,dl
   test byte[curexecstate],2
   jnz .sound
   mov edi,[tableadb+ebx*4]
   mov bl,[esi]
   inc esi
   sub dh,[cpucycle+ebx]
   jc .cpuover
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



SECTION .data ;ALIGN=32
ALIGN32
NEWSYM ExecExitOkay, db 1

SECTION .bss ;ALIGN=32
NEWSYM JoyABack, resd 1
NEWSYM JoyBBack, resd 1
NEWSYM JoyCBack, resd 1
NEWSYM JoyDBack, resd 1
NEWSYM JoyEBack, resd 1
NEWSYM NetCommand, resd 1
NEWSYM spc700read, resd 1
NEWSYM lowestspc,  resd 1
NEWSYM highestspc, resd 1
NEWSYM SA1UBound,  resd 1
NEWSYM SA1LBound,  resd 1
NEWSYM SA1SH,      resd 1
NEWSYM SA1SHb,     resd 1
NEWSYM NumberOfOpcodes2, resd 1
NEWSYM ChangeOps, resd 1
NEWSYM SFXProc,    resd 1
SECTION .text


%macro C4Paused 0
;  cmp byte[C4Pause],0
;  je %%notpaused
;  inc esi
;  xor dh,dh
;  jmp cpuover
;%%notpaused
%endmacro

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
    mov al,byte[SA1Message+1]
    mov byte[SA1Message+3],al
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
    mov al,byte[SA1Message+1]
    mov byte[SA1Message+3],al
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

    cmp byte[curypos],100
    jne .noline100
    mov ax,[scrnon]
    mov [TempVidInfo],ax
    mov ax,[scaddset]
    mov [TempVidInfo+2],ax
.noline100

    ; check for VIRQ/HIRQ/NMI
    ProcessIRQStuff
    mov ax,[resolutn]
    dec ax
    cmp [curypos],ax
    jb .drawline


;    mov ax,[resolutn]
;    cmp [curypos],ax
;    jb .drawline
    C4Paused
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
    cmp byte[disable65816sh],1
    je .ohno
    cmp byte[SpeedHackSafeTable+eax],1
    jne .okay
.ohno
    mov byte[nmirept],0
    mov dword[nmiprevaddrl],0FFFFFFFFh
    mov dword[nmiprevaddrh],0
    jmp .noskip
.okay
    mov byte[nmistatus],2
    and byte[curexecstate],0FEh
.nodis65816
    jmp .noskip
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
    C4Paused
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.nmi
    mov byte[irqon],80h
    mov byte[doirqnext],0
    inc dword[NetSent]
    cmp byte[yesoutofmemory],1
    jne .noout
    call outofmemfix
.noout

;    pushad
;    call GenLatencyDisplay
;    popad


    ; NetCommand : bit 0 = Okay (should be 1), bit 1 = control update,
    ;  bit 2 = print update, bit 3 = quit, bit 4 = reset

    dec word[curypos]
    mov [tempdh],dh
    xor dh,dh
    mov byte[doirqnext],0

    call exechdma
    call exechdma

;    push es
;    cmp byte[cbitmode],1
;    jne .nodisptest
;    mov es,[vesa2selec]
;    mov word[es:10+640],1111111111111111b
;.nodisptest
;    pop es

    mov byte[NetCommand],0
    mov byte[NextNGDisplay],1
    cmp byte[newengen],0
    je .nonewgfx
    cmp byte[curblank],0h
    jne .nonewgfx
    cmp byte[ForceNewGfxOff],0
    jne .nonewgfx
;    cmp byte[NextNGDisplay],0
;    je .nonewgfx
    call StartDrawNewGfx
.nonewgfx
    cmp byte[chaton],1
    je near .nonet
    cmp byte[CNetType],20
    je near .net
    cmp byte[GUIQuit],1
    je near endprog
    mov eax,dword[KeyQuickSnapShot]
    or eax,eax
    jz .nosskey
    test byte[pressed+eax],1
    jz .nosskey
    mov byte[SSKeyPressed],1
    mov byte[pressed+eax],2
    jmp exitloop
.nosskey
    mov eax,dword[KeyQuickClock]
    or eax,eax
    jz .noclockkey
    test byte[pressed+eax],1
    jz .noclockkey
    xor byte[TimerEnable],1
    mov byte[pressed+eax],2
.noclockkey
    mov eax,dword[KeyQuickSaveSPC]
    or eax,eax
    jz .nosavespckey
    test byte[pressed+eax],1
    jz .nosavespckey
    mov byte[SPCKeyPressed],1
    mov byte[pressed+eax],2
    jmp exitloop
.nosavespckey
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
    jmp .nonet
.net
    test byte[pressed+1],01h
    jz .nonetexit
    or byte[NetCommand],08h
.nonetexit
    mov eax,[KeySaveState]
    test byte[pressed+eax],01h
    jz .notnetsave
    mov byte[pressed+eax],2
    call NetSaveState
.notnetsave
    mov eax,[KeyLoadState]
    test byte[pressed+eax],01h
    jz .notnetload
    or byte[NetCommand],88h
.notnetload
.nonet
    mov dh,[tempdh]
    inc word[curypos]
    cmp byte[CurRecv],1
    je .noinputread
    cmp byte[NoInputRead],1
    je .noinputread
    call ReadInputDevice
.noinputread

    call UpdateRewind

    mov byte[NetQuit],0
    cmp byte[CNetType],20
    jne near .nozerons
    test byte[NetSwap],1
    jnz near .noonens

    cmp byte[CurRecv],1
    je near .noreceiveb2

    mov eax,[JoyAOrig]
    cmp eax,[prevp1net]
    je .nochange1
    or byte[NetCommand],02h
    mov [prevp1net],eax
.nochange1
    mov eax,[JoyBOrig]
    cmp eax,[prevp2net]
    je .nochange2
    or byte[NetCommand],02h
    mov [prevp2net],eax
.nochange2
    mov eax,[JoyCOrig]
    cmp eax,[prevp3net]
    je .nochange3
    or byte[NetCommand],02h
    mov [prevp3net],eax
.nochange3
    mov eax,[JoyDOrig]
    cmp eax,[prevp4net]
    je .nochange4
    or byte[NetCommand],02h
    mov [prevp4net],eax
.nochange4
    test byte[NetCommand],02h
    jz .nochangeatall
    mov dword[ResendTimer],60
    jmp .yeschanged
.nochangeatall
    dec dword[ResendTimer]
    jnz .yeschanged
    or byte[NetCommand],02h
    mov dword[ResendTimer],60
.yeschanged
    ; Send command & store command
    call PreparePacket
    push ebx
    push ecx
    mov al,[NetCommand]
    mov ebx,[cnetptrhead]
    mov [cnetplaybuf+ebx],al
    call RemoteSendChar
    ; ##################
    ; Send latency value
    ;cmp byte[OSPort],3
    ;jne .nolatencysend
%ifndef __MSDOS__ 
    cmp byte[BackState],1
    jne .nolatencysend
    mov ebx,[LatencySendPtr]
    and ebx,0FFh
    inc dword[LatencySendPtr]
    mov byte[LatencyV+ebx],0
    mov ebx,[PBackupPos]
    mov al,[LocalLValue]
;    inc al
    call RemoteSendChar
%endif
.nolatencysend
    mov ebx,[cnetptrhead]
    mov ecx,JoyAOrig
    inc ebx
    and ebx,1FFh
    test byte[NetCommand],02h
    jz near .nosendextra
    NetHelpExecSend 0
    NetHelpExecSend 1
    NetHelpExecSend 2
    NetHelpExecSend 3
    NetHelpExecSend 4
.nosendextra
    mov [cnetptrhead],ebx

    call SendPacketUDP

    cmp byte[chaton],0
    jne .nosendchats
    cmp byte[chatstrL],0
    je .nosendchats
    cmp dword[chatTL],0
    jne .nosendchats
    mov byte[NetCommand],04h
.nosendchats

    ; send chat string
    test byte[NetCommand],04h
    jz near .nosendchatsend
    call PreparePacket
    mov al,04h
    call RemoteSendChar

    call MoveStringChat

    push esi
    mov esi,chatstrR

    ;cmp byte[OSPort],2
    ;jae .notwin32b
%ifdef __MSDOS__
    mov byte[esi],'L'
    mov byte[esi+1],'>'
    add esi,2
%else
    ;jmp .skipsendnick
;.notwin32b
    cmp dword[chatstrL+1],'/ME '
    jne .noaction
    mov al,'*'
    push ebx
    push eax
    mov [esi],al
    inc esi
    call RemoteSendChar
    pop eax
    pop ebx
.noaction
    mov ebx,ChatNick
.nextchatc2
    mov al,[ebx]
    cmp al,0
    je .nonextchat
    push ebx
    push eax
    mov [esi],al
    inc esi
    call RemoteSendChar
    pop eax
    pop ebx
    inc ebx
    cmp byte[ebx-1],0
    jne .nextchatc2
.nonextchat
    mov al,'>'
    cmp dword[chatstrL+1],'/ME '
    jne .noaction2
    mov al,' '
.noaction2
    push ebx
    push eax
    mov [esi],al
    inc esi
    call RemoteSendChar
    pop eax
    pop ebx
%endif
.skipsendnick
    mov ebx,chatstrL+1
    ;cmp byte[OSPort],2
    ;jb .noaction3
%ifndef __MSDOS__
    cmp dword[chatstrL+1],'/ME '
    jne .noaction3
    mov ebx,chatstrL+5
%endif
.noaction3
.nextchatc
    mov al,[ebx]
    push ebx
    push eax
    mov [esi],al
    inc esi
    call RemoteSendChar
    pop eax
    pop ebx
    inc ebx
    cmp byte[ebx-1],0
    jne .nextchatc
    mov byte[esi],0
    pop esi
    mov ecx,45
    mov ebx,chatstrL
.chatsendloop
    mov al,[ebx+1]
    mov [ebx],al
    inc ebx
    dec ecx
    jnz .chatsendloop
    mov byte[chatstrL],0
    mov dword[chatTL],10
    mov dword[chatRTL],8*60
    call SplitStringChat
    call SendPacket
.nosendchatsend
    pop ecx
    pop ebx
    jmp .noreceiveb3
.noreceiveb2
.noreceiveb3

    mov dword[JoyAOrig],0
    mov dword[JoyBOrig],0
    mov dword[JoyCOrig],0
    mov dword[JoyDOrig],0
    mov dword[JoyEOrig],0

    cmp byte[LatencyLeft],0
    jne near .latencyleft

    mov [tempedx],edx

    push ecx
    push ebx
;    cmp byte[OSPort],3
;    jne .nobackstate
%ifndef __MSDOS__
    cmp byte[BackState],1
    jne .nobackstate
    call BackupCVFrame
.nobackupcvframe
    mov ebx,[CBackupPos]
    inc ebx
    and ebx,0Fh
    mov [CBackupPos],ebx
%endif
.nobackstate
    pop ebx
    pop ecx

    push ebx
    push ecx

    cmp dword[CurRecv],1
    je .yesreceive2
    xor ebx,ebx
.latencyloop
    add dword[LatencyV+ebx*4],01010101h
    inc ebx
    cmp ebx,64
    jne .latencyloop
.yesreceive2

    cmp dword[CurRecv],1
    jne near .notreceive
    mov ebx,[PPValue]
    inc ebx
    and ebx,0Fh
    mov [PPValue],ebx
    cmp ebx,[DPValue]
    jne near .notreceive
    mov dword[CurRecv],0
    call VoiceEndMute
    cmp byte[t1cc],4
    jbe .noframesskippedb
    mov byte[t1cc],4
.noframesskippedb
    jmp .received
.notreceive
.received
    pop ecx
    pop ebx

    ; Receive command from net and process
    push ebx
    push ecx

    cmp byte[CurRecv],1
    je near .nocrupdate3b

    xor bl,bl
    mov cx,[t1cc]
    add cx,60*15
.notfoundchar
.onlychatchar
    call RemoteGetChar
;    cmp [t1cc],cx
;    jne .notor
    jmp .notor
.netquit2
    or byte[NetQuit],80h
    mov byte[RemoteDisconnect],1
    jmp .skipallnet
.notor
    cmp dh,0
    jne .foundchar
    ;cmp byte[OSPort],3
    ;jne .notwin32
%ifndef __MSDOS__
    push ebx
    cmp byte[BackState],1
    jne .nobackstate2
    mov ebx,[CBackupPos]
    dec ebx
    sub ebx,[PBackupPos]
    sub ebx,[BackStateSize]
    and ebx,0Fh
;    inc ebx
;    and ebx,0Fh
;    cmp ebx,[PBackupPos]
    jne near .nocrupdate3       ; *************************
.nobackstate2
    pop ebx
;NEWSYM StateBackup, dd 0
;NEWSYM CBackupPos, dd 0
;NEWSYM PBackupPos, dd 0
;NEWSYM PPValue, dd 0   ; Previous PValue
;NEWSYM DPValue, dd 0   ; Destination PValue
;NEWSYM CurRecv, dd 0   ; Set to 1 if Recovery mode is on
; if CurRecv=1, then do not send tcp/ip data, always frame skip, do not
;   draw to screen, do not key on, restore previous local key presses,
;   when disabling key ons, divert dspmem write/read to a different
;   array temporarly, then re-copy back in when finished
;NEWSYM PPContrl, times 16 dd 0   ; Previous Controller 1 Data
;NEWSYM PPContrl2, times 16 dd 0   ; Previous Controller 2 Data
;NEWSYM PPContrl3, times 16 dd 0   ; Previous Controller 3 Data
;NEWSYM PPContrl4, times 16 dd 0   ; Previous Controller 4 Data
;NEWSYM PPContrl5, times 16 dd 0   ; Previous Controller 5 Data

    pushad
    mov byte[nojoystickpoll],1
    call JoyRead
    mov byte[nojoystickpoll],0
;    call ChatType2
    popad
    cmp word[t1cc],4*60
    jb .notwin32
    cmp byte[pressed+1],1
    je .netquit2
%endif
.notwin32
    mov bl,1
    jmp .notfoundchar
.foundchar
    cmp dl,04h
    jne .notchatchar
    jmp .recvchats
.notchatchar
    cmp byte[t1cc],4
    jbe .noframesskipped
    mov byte[t1cc],4
.noframesskipped

    inc byte[netdelayed]
    cmp bl,0
    jne .yesdelay
    mov byte[netdelayed],0
.yesdelay
    cmp byte[netdelayed],10
    jne .nodelayfix
    mov byte[netdelayed],0
    cmp byte[t1cc],0
    je .nodelayfix
    dec byte[t1cc]
.nodelayfix
    mov [NetCommand],dl

    pushad
    ; Receive latency value
    ; #####################
    ;cmp byte[OSPort],3
    ;jne near .nolatencyrecv2
%ifndef __MSDOS__
    cmp byte[BackState],1
    jne near .nolatencyrecv2
.tryagainlatency
    call RemoteGetChar         ; **********
    cmp byte[RemoteDisconnect],1
    je near .netquit
    cmp dh,0
    je .tryagainlatency
    mov ebx,[LatencyRecvPtr]
    and ebx,0FFh
    inc dword[LatencyRecvPtr]
    mov al,[LatencyV+ebx]
    mov [RemoteLValue],dl
    mov [LocalLValue],al
    cmp al,dl
    jbe .nolatencyrecv3
    ; incr t1cc -> make local speed faster if local latency > remote
    inc dword[latencytimer]
    cmp dword[latencytimer],5
    jb .nolatencyrecv2
    mov dword[latencytimer],0
    cmp byte[t1cc],0
    je .nolatencyrecv2
    dec byte[t1cc]
    jmp .nolatencyrecv2
.nolatencyrecv3
    cmp al,dl
    je .nolatencyrecv2
    inc dword[latencytimer]
    cmp dword[latencytimer],5
    jb .nolatencyrecv2
    mov dword[latencytimer],0
    inc byte[t1cc]
%endif
.nolatencyrecv2
    popad
    inc dword[NetSent2]

    test dl,01h
    jnz near .netfailed
    test dl,070h
    jnz near .netfailed

    cmp dword[NetQuitter],0
    je .noforcequit
    mov ebx,[PBackupPos]
    inc ebx
    and ebx,0Fh
    cmp ebx,[QBackupPos]
    je .forcequit
.noforcequit
    test dl,08h
    jz .noquit
.forcequit
    test dl,80h
    jz .noloadstate
    mov byte[NetLoadState],1
.noloadstate
    or byte[NetQuit],1
    cmp byte[BackState],1
    jne .noquit
    push edx
    mov ebx,[PBackupPos]
    inc ebx
    and ebx,0Fh
    cmp ebx,[CBackupPos]
    je .noquit2
    call RestoreCVFrame
    add dword[NetSent],1000
.noquit2
    pop edx
.noquit
    test dl,02h
    jz near .nocrupdate2

;    cmp byte[OSPort],3
;    jne .notwin32d
%ifndef __MSDOS__
    cmp byte[BackState],1
    jne .notwin32d
    push edx
    mov ebx,[PBackupPos]
    inc ebx
    and ebx,0Fh
    cmp ebx,[CBackupPos]
    je .noupdate
    call RestoreCVFrame
    mov ebx,[PBackupPos]                ; *****************
    inc ebx
    and ebx,0Fh
    mov [PPValue],ebx
    mov ebx,[CBackupPos]
    mov [DPValue],ebx
    mov ebx,[PPValue]
    mov [CBackupPos],ebx
    mov dword[CurRecv],1               ; *****************
    call VoiceStartMute
.noupdate
    mov ebx,[PBackupPos]
    inc ebx
    and ebx,0Fh
    mov [PBackupPos],ebx
    pop edx
%endif
.notwin32d
    mov ecx,JoyAOrig
    NetHelpExecRecv 0
    NetHelpExecRecv 1
    NetHelpExecRecv 2
    NetHelpExecRecv 3
    NetHelpExecRecv 4
    jmp .donecrupdate
.nocrupdate3
    pop ebx
.nocrupdate3b
    mov byte[NetCommand],0
    jmp .nocrupdate
.nocrupdate2
    mov ebx,[PBackupPos]
    inc ebx
    and ebx,0Fh
    mov [PBackupPos],ebx
.nocrupdate
    cmp byte[pl1neten],2
    jne .nopl1recv
    mov eax,[JoyABack]
    mov [JoyAOrig],eax
.nopl1recv
    cmp byte[pl2neten],2
    jne .nopl2recv
    mov eax,[JoyBBack]
    mov [JoyBOrig],eax
.nopl2recv
    cmp byte[pl3neten],2
    jne .nopl3recv
    mov eax,[JoyCBack]
    mov [JoyCOrig],eax
.nopl3recv
    cmp byte[pl4neten],2
    jne .nopl4recv
    mov eax,[JoyDBack]
    mov [JoyDOrig],eax
.nopl4recv
    cmp byte[pl5neten],2
    jne .nopl5recv
    mov eax,[JoyEBack]
    mov [JoyEOrig],eax
.nopl5recv
.donecrupdate

    jmp .norecvchats
.recvchats
    push ebx

    call MoveStringChat

    mov ebx,chatstrR
    push edx
.nextchatcr
    push ebx
    push ecx
    mov cx,[t1cc]
    add cx,60*10
.tryagainchatc
    call RemoteGetChar         ; **********
    cmp byte[RemoteDisconnect],1
    je .netquit
    cmp [t1cc],cx
    jne .noto
.netquit
    or byte[NetQuit],80h
    mov dh,1
.noto
    cmp dh,0
    je .tryagainchatc          ; *********
    pop ecx
    pop ebx
    mov [ebx],dl
    inc ebx
    cmp dl,0
    jne .nextchatcr            ; *********
    pop edx
    pushad
    ;cmp byte[OSPort],2
    ;jae .notwin32e
%ifdef __MSDOS__
    mov dl,'R'
    call NetAddChar
    mov dl,'>'
    call NetAddChar
%endif
.notwin32e
    mov esi,chatstrR
    call WritetochatBuffer
    mov dl,13
    call NetAddChar
    mov dl,10
    call NetAddChar
    popad
    mov dword[chatRTL],60*8
    call SplitStringChat
    pop ebx
    jmp .onlychatchar
.norecvchats

    jmp .skipfailed
.netfailed
    or byte[NetQuit],80h
.skipfailed

    cmp byte[CurRecv],1
    je near .noreceiveb
    ; Process previous command from buffer
    mov ebx,[cnetptrtail]
    mov ecx,JoyAOrig
    mov dl,[cnetplaybuf+ebx]
    inc ebx
    and ebx,1FFh
    test dl,08h
    jz .noquit2b
    push ebx
    mov ebx,[PBackupPos]
    cmp ebx,[CBackupPos]
    je .yesquit
    cmp dword[NetQuitter],1
    je .quitlater
    test dl,80h
    jz .noloadstateb
    mov byte[NetLoadState],2
.noloadstateb
    mov dword[NetQuitter],1
    mov ebx,[CBackupPos]
    mov [QBackupPos],ebx
    jmp .quitlater
.yesquit
    test dl,80h
    jz .noloadstateb2
    mov byte[NetLoadState],2
.noloadstateb2
    or byte[NetQuit],1
.quitlater
    pop ebx
;NEWSYM NetQuitter, dd 0
;NEWSYM QBackupPos, dd 0
.noquit2b
    test dl,02h
    jz near .nocrupdate2b
    NetHelpExecRecv2 0
    NetHelpExecRecv2 1
    NetHelpExecRecv2 2
    NetHelpExecRecv2 3
    NetHelpExecRecv2 4
    jmp .donecrupdate2
.nocrupdate2b
    cmp byte[pl1neten],1
    jne .nopl1recv2
    mov eax,[JoyABack]
    mov [JoyAOrig],eax
.nopl1recv2
    cmp byte[pl2neten],1
    jne .nopl2recv2
    mov eax,[JoyBBack]
    mov [JoyBOrig],eax
.nopl2recv2
    cmp byte[pl3neten],1
    jne .nopl3recv2
    mov eax,[JoyCBack]
    mov [JoyCOrig],eax
.nopl3recv2
    cmp byte[pl4neten],1
    jne .nopl4recv2
    mov eax,[JoyDBack]
    mov [JoyDOrig],eax
.nopl4recv2
    cmp byte[pl5neten],1
    jne .nopl5recv2
    mov eax,[JoyEBack]
    mov [JoyEOrig],eax
.nopl5recv2
.donecrupdate2
    mov [cnetptrtail],ebx
.noreceiveb

    ; backup keyboard presses if recv=1 (delayed by 1 frame)
    ; else restore
    ; Restore previous keys if CurRecv = 1
    ; ************************************
    cmp byte[CurRecv],1
    je near .noreceive
    ; backup keypresses
    mov ebx,[CBackupPos]
    dec ebx
    and ebx,0Fh
    mov ecx,[JoyAOrig]
    mov [PPContrl+ebx*4],ecx
    mov ecx,[JoyBOrig]
    mov [PPContrl2+ebx*4],ecx
    mov ecx,[JoyCOrig]
    mov [PPContrl3+ebx*4],ecx
    mov ecx,[JoyDOrig]
    mov [PPContrl4+ebx*4],ecx
    mov ecx,[JoyEBack]
    mov [PPContrl4+ebx*4],ecx
    jmp .yesreceive
.noreceive
    mov ebx,[PPValue]
    dec ebx
    and ebx,0Fh
    cmp byte[pl1neten],1
    jne .nopl1recv3
    mov ecx,[PPContrl+ebx*4]
    mov [JoyAOrig],ecx
.nopl1recv3
    cmp byte[pl2neten],1
    jne .nopl2recv3
    mov ecx,[PPContrl2+ebx*4]
    mov [JoyBOrig],ecx
.nopl2recv3
    cmp byte[pl3neten],1
    jne .nopl3recv3
    mov ecx,[PPContrl3+ebx*4]
    mov [JoyCOrig],ecx
.nopl3recv3
    cmp byte[pl4neten],1
    jne .nopl4recv3
    mov ecx,[PPContrl4+ebx*4]
    mov [JoyDOrig],ecx
.nopl4recv3
    cmp byte[pl5neten],1
    jne .nopl5recv3
    mov ecx,[PPContrl5+ebx*4]
    mov [JoyEOrig],ecx
.nopl5recv3
.yesreceive

.skipallnet
    pop ecx
    pop ebx
    mov edx,[tempedx]
    xor ebx,ebx
    jmp .donelatency
.latencyleft
    dec byte[LatencyLeft]
.donelatency
    mov eax,[JoyAOrig]
    mov [JoyABack],eax
    mov eax,[JoyBOrig]
    mov [JoyBBack],eax
    mov eax,[JoyCOrig]
    mov [JoyCBack],eax
    mov eax,[JoyDOrig]
    mov [JoyDBack],eax
    mov eax,[JoyEOrig]
    mov [JoyEBack],eax
    jmp .reprocjoy
.noonens
    test byte[NetSwap],1
    jz .nozerons
    ; copy previous frame values into JoyxOrig and JoyxNow
    mov eax,[JoyABack]
    mov [JoyAOrig],eax
    mov eax,[JoyBBack]
    mov [JoyBOrig],eax
    mov eax,[JoyCBack]
    mov [JoyCOrig],eax
    mov eax,[JoyDBack]
    mov [JoyDOrig],eax
    mov eax,[JoyEBack]
    mov [JoyEOrig],eax
.reprocjoy
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
.nozerons

    cmp byte[snesmouse],4
    jne .nolethalen
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen

    xor byte[NetSwap],1

    cmp byte[NetQuit],0
    je .noquitb
    pushad
    mov esi,[wramdata]
    mov ecx,65536*2
    xor eax,eax
    xor ebx,ebx
.quitloop
    mov al,[esi]
    add bx,ax
    inc esi
    dec ecx
    jnz .quitloop
    mov [valuea],bx
    popad
    add [valuea],dh

    mov byte[ExecExitOkay],0
    mov byte[pressed+1],01h
    jmp exitloop
.noquitb

    cmp byte[MovieProcessing],0
    je .noprocmovie
    call ProcessMovies
.noprocmovie

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
;    call changeexecloop
.dis65816
    cmp byte[CheatOn],1
    je near .cheater
.returncheat
    mov ax,[VIRQLoc]
    cmp word[curypos],ax
    jne .novirqz
    test byte[INTEnab],80h
    jz .novirqz
    inc word[VIRQLoc]
.novirqz
    mov ax,[oamaddrs]
    mov [oamaddr],ax
    mov byte[nosprincr],0
    call showvideo
;    call dsp1teststuff
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
    C4Paused
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
    C4Paused
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
    C4Paused
    jmp switchtonmi
.failcheck
    mov byte[nmirept],0
    mov dword[nmiprevaddrl],0FFFFFFFFh
    mov dword[nmiprevaddrh],0
    mov byte[doirqnext],0
    C4Paused
    jmp switchtonmi
.nonmi
    cmp byte[intrset],1
    jne .nointrset2w
    mov byte[intrset],2
.nointrset2w
    cmp byte[esi],0CBh
    jne .nowai
    jmp .nowai
    test dl,04h
    jz .nowai
    or byte[INTEnab],80h
.nowai
    C4Paused
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain


.overy
    shr dh,1
    cmp byte[smallscreenon],1
    je .nocfield
    cmp byte[ScreenScale],1
    je .nocfield
    cmp byte[scanlines],0
    jne .nocfield
    xor byte[cfield],1
.nocfield
    mov word[curypos],0

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

;    call cachevideo
    call starthdma
;    cmp byte[Offby1line],1
;    je .noirqhack
;    cmp byte[IRQHack],0
;    je .noirqhack
;    call exechdma
.noirqhack


    ; check for VIRQ/HIRQ/NMI
    cmp byte[execatzerovirq],1
    je near .noexecatzero
    ProcessIRQStuff
.noexecatzero

    C4Paused
    xor ebx,ebx
    mov bl,[esi]
    inc esi
    jmp execloop.startagain

.virq
    C4Paused
    test byte[INTEnab],10h
    jz .skiphirq
    cmp word[HIRQLoc],0
    je .skiphirq
    jmp .skiphirq
    cmp word[HIRQLoc],339
    jbe .hirqnotover
    mov word[HIRQLoc],339
.hirqnotover
    ; first dh = HIRQLoc*DHAdd/340, second dh = DHAdd-first dh
    push edx
    mov ax,[HIRQLoc]
    xor ecx,ecx
    mov cl,[cycpl]
    mul cx
    mov cx,340
    div cx
    pop edx
    mov dh,al
    mov cl,[cycpl]
    sub cl,al
    xor cl,cl
    mov [HIRQCycNext],cl
    mov byte[HIRQNextExe],1
;    jmp .hirq
    jmp .returnfromhirq
.skiphirq
    test byte[curexecstate],01h
    jnz .dis658162
    or byte[curexecstate],01h
;    call changeexecloop
.dis658162
    mov byte[doirqnext],0
    xor ebx,ebx
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
    C4Paused
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
    mov byte[.numcheat],al
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

SECTION .bss ;ALIGN=32
.numcheat resb 1
SECTION .text ;ALIGN=32

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
   mov dword [FxTable+4Ch*4],eax
   mov dword [FxTableb+4Ch*4],eax
   mov dword [FxTablec+4Ch*4],eax
   mov dword [FxTabled+4Ch*4],ebx
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
   mov dword [NumberOfOpcodes2],350 ; 0FFFFFFFh;350
   test byte[SfxCLSR],01h
   jz .nohighsfx
   mov dword [NumberOfOpcodes2],700 ;700
.nohighsfx
    cmp byte[SFXCounter],1
    je .noyi
    mov dword [NumberOfOpcodes2],0FFFFFFFh
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
    mov dword [NumberOfOpcodes],350 ; 0FFFFFFFh;350
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword [NumberOfOpcodes],700 ;700
.nohighsfx
    cmp byte[SFXCounter],1
    jne .noyi
    mov dword [NumberOfOpcodes],0FFFFFFFFh
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
   mov dword [FxTable+4Ch*4],eax
   mov dword [FxTableb+4Ch*4],eax
   mov dword [FxTablec+4Ch*4],eax
   mov dword [FxTabled+4Ch*4],ebx

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
    mov dword [NumberOfOpcodes],400 ;678
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword [NumberOfOpcodes],800 ;678*2
.nohighsfx
    cmp byte[SFXCounter],1
    jne .noyi
    mov dword [NumberOfOpcodes],0FFFFFFFh
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
    mov dword [NumberOfOpcodes],400 ;678
    test byte[SfxCLSR],01h
    jz .nohighsfx
    mov dword [NumberOfOpcodes],800 ;678*2
.nohighsfx
    mov dword [NumberOfOpcodes],0FFFFFFFFh
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

;    cmp byte[SA1Enable],0
;    je .nosa1
;    test byte[SA1Control],60h
;    jnz .nosa1
;    dec esi
;    call SA1Swap
;    mov bl,[esi]
;    inc esi
;    cmp byte[SA1Status],0
;    je near .nosa1
;    mov [pdh],dh
;    xor dh,dh
;    jmp cpuover
;.nosa1

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

    cmp byte[snesmouse],4
    jne .nolethalen
    mov eax,[LethEnData]
    mov [JoyBNow],eax
.nolethalen

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
;    mov byte[debstop3],1
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
    shr dh,1
    mov word[curypos],0
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
;    mov byte[debstop3],1
    jmp switchtovirqdeb


NEWSYM ExecuteAsmEnd

