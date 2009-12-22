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



; Sorry.  The GUI code is a total mess.  One problem I encountered is that
;   there seems to be a bug in nasm with using math calculations combined
;   with macros within macros, so in some macro definitions, I had to
;   add/subtract extra values in certain macros to get the GUI to align
;   correctly.
; The GUI is drawn is mostly manually.  What I mean is such as
;   coding 5 boxes to represent a shaded box for each box.  Although that
;   can be simplified using macros to draw 5 boxes, that unknown bug in nasm
;   showed up which prevented me from using macros extensively.  I was
;   thinking of changing it to an object-based GUI for easier coding,
;   but that was decided far into the development of the GUI itself
;   and that I also never expected this code to become open-source, so I
;   decided not to proceed with it.

; Routine StartGUI is the function called to start the GUI and also
;   contains the execution main loop of the GUI.  If you want to completely
;   replace the GUI, just add a function call from that function, then
;   jump to endprog to exit ZSNES or continueprog to continue with the
;   gameplay.  Do not replace StartGUI with a function since it is not
;   a function, but rather a label that is being jumped to.

%include "macros.mac"

EXTSYM vidpastecopyscr,frameskip,newengen,cvidmode,antienab
EXTSYM soundon,StereoSound,SoundQuality,MusicRelVol
EXTSYM cbitmode
EXTSYM romloadskip,romdata,current_zst
EXTSYM vidbuffer,ASCII2Font,showallext,scanlines
EXTSYM spritetablea,KeyRTRCycle
EXTSYM cgram,tempco0,prevbright,maxbr,prevpal,coladdr,coladdg
EXTSYM coladdb,scaddtype,initvideo,pressed,UpdateDevices,memtabler8
EXTSYM memtablew8,writeon,JoyRead,SetInputDevice,delay,FPSOn,RevStereo,WDSPReg0C
EXTSYM WDSPReg1C,pl12s34,vidbufferofsb,wramdata,bgfixer
EXTSYM videotroub,CheatCodeSave,CheatCodeLoad
EXTSYM Check_Key,Get_Key,sram,ScanCodeListing,RelPathBase
EXTSYM Get_MouseData,Set_MouseXMax
EXTSYM Set_MouseYMax,Set_MousePosition,Get_MousePositionDisplacement
EXTSYM MessageOn,GetTime
EXTSYM Clear2xSaIBuffer,MouseWindow,Show224Lines
EXTSYM newgfx16b,NumVideoModes,MusicVol,DSPMem,NumInputDevices
EXTSYM GUIInputNames,GUIVideoModeNames,GameSpecificInput,device1,device2,TwelveHourClock
EXTSYM GUIM7VID,GUINTVID,GUIHQ2X,RawDumpInProgress
EXTSYM MultiTap,SFXEnable
EXTSYM nssdip1,nssdip2,nssdip3,nssdip4,nssdip5,nssdip6
EXTSYM SkipMovie,MovieStop,MoviePlay,MovieRecord
EXTSYM MovieInsertChapter,MovieSeekAhead,MovieSeekBehind
EXTSYM MovieDumpRaw,MovieAppend,AutoLoadCht,GUILoadData
EXTSYM CheckMenuItemHelp
EXTSYM GUITryMenuItem,GUIProcStates,GUIProcReset,GUISetPal

EXTSYM GUIwinposx,GUIwinposy,maxskip,GUIEffect,hqFilter,En2xSaI,NTSCFilter
EXTSYM NTSCBlend,NTSCHue,NTSCSat,NTSCCont,NTSCBright,NTSCSharp,NTSCRef
EXTSYM NTSCGamma,NTSCRes,NTSCArt,NTSCFringe,NTSCBleed,NTSCWarp
EXTSYM LowPassFilterType,MovieStartMethod,MovieDisplayFrame,savewinpos
EXTSYM SnapPath,SPCPath,BSXPath,SGPath,STPath,GNextPath
EXTSYM SRAMPath,CheatSrcByteSize
EXTSYM IPSPath,MoviePath,CHTPath,ComboPath,INPPath,SStatePath
EXTSYM MMXSupport
EXTSYM GUIRAdd,GUIGAdd,GUIBAdd,GUITRAdd,GUITGAdd,GUITBAdd,GUIWRAdd
EXTSYM GUIWGAdd,GUIWBAdd,GUIloadfntype,SoundInterpType
EXTSYM CheatSrcByteBase,CheatSrcSearchType,CheatUpperByteOnly,GUIComboGameSpec
EXTSYM KeyStateSlc0,KeyStateSlc1,KeyStateSlc2,KeyStateSlc3,KeyStateSlc4
EXTSYM KeyStateSlc5,KeyStateSlc6,KeyStateSlc7,KeyStateSlc8,KeyStateSlc9
EXTSYM RewindStates,RewindFrames,PauseRewind,PauseLoad,SRAMState,AutoState
EXTSYM LatestSave,SRAMSave5Sec,AutoIncSaveSlot,KeyUsePlayer1234
EXTSYM pl1contrl,pl1selk,pl1startk,pl1upk,pl1downk,pl1leftk,pl1rightk,pl1Xk
EXTSYM pl1Ak,pl1Lk,pl1Yk,pl1Bk,pl1Rk,pl1Xtk,pl1Ytk,pl1Atk,pl1Btk,pl1Ltk,pl1Rtk
EXTSYM pl1ULk,pl1URk,pl1DLk,pl1DRk,pl2contrl,pl2selk,pl2startk,pl2upk,pl2downk
EXTSYM pl2leftk,pl2rightk,pl2Xk,pl2Ak,pl2Lk,pl2Yk,pl2Bk,pl2Rk,pl2Xtk,pl2Ytk
EXTSYM pl2Atk,pl2Btk,pl2Ltk,pl2Rtk,pl2ULk,pl2URk,pl2DLk,pl2DRk,pl3contrl,pl3selk
EXTSYM pl3startk,pl3upk,pl3downk,pl3leftk,pl3rightk,pl3Xk,pl3Ak,pl3Lk,pl3Yk
EXTSYM pl3Bk,pl3Rk,pl3Xtk,pl3Ytk,pl3Atk,pl3Btk,pl3Ltk,pl3Rtk,pl3ULk,pl3URk
EXTSYM pl3DLk,pl3DRk,pl4contrl,pl4selk,pl4startk,pl4upk,pl4downk,pl4leftk
EXTSYM pl4rightk,pl4Xk,pl4Ak,pl4Lk,pl4Yk,pl4Bk,pl4Rk,pl4Xtk,pl4Ytk,pl4Atk,pl4Btk
EXTSYM pl4Ltk,pl4Rtk,pl4ULk,pl4URk,pl4DLk,pl4DRk,pl5contrl,pl5selk,pl5startk
EXTSYM pl5upk,pl5downk,pl5leftk,pl5rightk,pl5Xk,pl5Ak,pl5Lk,pl5Yk,pl5Bk,pl5Rk
EXTSYM pl5Xtk,pl5Ytk,pl5Atk,pl5Btk,pl5Ltk,pl5Rtk,pl5ULk,pl5URk,pl5DLk,pl5DRk
EXTSYM KeyResetAll,KeyExtraEnab1,KeyExtraEnab2,KeyVolDown,KeyVolUp
EXTSYM KeyBGDisble0,KeyBGDisble1,KeyBGDisble2,KeyBGDisble3,KeySprDisble
EXTSYM KeyDisableSC0,KeyDisableSC1,KeyDisableSC2,KeyDisableSC3,KeyQuickSnapShot
EXTSYM KeyDisableSC4,KeyDisableSC5,KeyDisableSC6,KeyDisableSC7,KeyQuickSaveSPC
EXTSYM KeyQuickLoad,KeyQuickRst,KeyQuickExit,KeyQuickClock,KeyQuickChat
EXTSYM KeyInsrtChap,KeyPrevChap,KeyNextChap,KeyDisplayFPS,KeyNewGfxSwt
EXTSYM KeyIncStateSlot,KeyDecStateSlot,KeySaveState,KeyLoadState,KeyStateSelct
EXTSYM KeyRewind,KeyEmuSpeedUp,KeyEmuSpeedDown,KeyFRateUp,KeyFRateDown
EXTSYM KeyFastFrwrd,KeySlowDown,KeyResetSpeed,EMUPauseKey,INCRFrameKey
EXTSYM KeyWinDisble,KeyOffsetMSw,JoyPad1Move,init_save_paths
EXTSYM mousewrap,GUIRClick,SwapMouseButtons
EXTSYM FPSAtStart,Turbo30hz,TimerEnable,SmallMsgText,mouse1lh,mouse2lh
EXTSYM AutoPatch,RomInfo,AllowUDLR,GrayscaleMode,GUIMovieForcedText
EXTSYM Mode7HiRes16b,FFRatio,SDRatio,EmuSpeed,mouseshad,MovieForcedLengthEnabled
EXTSYM esctomenu,GUILoadKeysJumpTo,lhguimouse,MZTForceRTR,GetMovieForcedLength
EXTSYM GUIEnableTransp,FilteredGUI,Surround,SPCDisable,nosaveSRAM
EXTSYM FastFwdToggle,gui_key,gui_key_extended,GUILoadKeysNavigate
EXTSYM KeyDisplayBatt,KeyIncreaseGamma,KeyDecreaseGamma,vsyncon
EXTSYM MovieVideoMode,MovieAudio,MovieVideoAudio,MovieAudioCompress,newfont
EXTSYM d_names,selected_names,GUIfileentries,GUIdirentries,GUIcurrentdirviewloc
EXTSYM GUIcurrentfilewin,GUIcurrentcursloc,GUIcurrentviewloc,SetMovieForcedLength,DisableScreenSaver
EXTSYM GUIcurrentdircursloc,GetLoadData,ZRomPath,ClockBox,DisplayInfo
EXTSYM GUIJT_currentviewloc,GUIJT_currentcursloc,GUIJT_entries,ScreenShotFormat
EXTSYM GUIJT_offset,GUIJT_viewable,GUIGenericJumpTo,SSAutoFire,SSPause

%ifdef __UNIXSDL__
EXTSYM numlockptr
EXTSYM CheckOpenGL
%elifdef __WIN32__
EXTSYM initDirectDraw,reInitSound,CheckPriority,AlwaysOnTop
EXTSYM CheckScreenSaver,MouseWheel,TrapMouseCursor,AllowMultipleInst,TripleBufferWin
EXTSYM HighPriority,SaveMainWindowPos,PrimaryBuffer
EXTSYM CBBuffer,CBLength,PasteClipBoard,ctrlptr,PauseFocusChange
%elifdef __MSDOS__
EXTSYM dssel,SetInputDevice209,initvideo2,Force8b,SBHDMA,vibracard,smallscreenon,ExitFromGUI
EXTSYM pl1p209,pl2p209,pl3p209,pl4p209,pl5p209,SidewinderFix,Triplebufen,ScreenScale
EXTSYM GUIEAVID,GUIWSVID,GUISSVID,GUITBVID,GUISLVID,GUIHSVID,GUI2xVID,TripBufAvail
EXTSYM JoyMinX209,JoyMaxX209,JoyMinY209,JoyMaxY209,DOSClearScreen
EXTSYM GUI36hzcall
%endif

%ifndef __MSDOS__
EXTSYM ZsnesPage,DocsPage,GUICustomX,GUICustomY,GetCustomXY,SetCustomXY,initwinvideo
EXTSYM Keep4_3Ratio,PrevFSMode,PrevWinMode,NTSCFilterInit,hqFilterlevel,BilinearFilter,GUIBIFIL
EXTSYM GUIWFVID,GUIDSIZE,GUIHQ3X,GUIHQ4X,GUIKEEP43,Keep43Check,changeRes,sl_intensity
%endif

%ifndef __WIN32__
EXTSYM GUII2VID
%endif

%ifdef __OPENGL__
EXTSYM allow_glvsync
%endif

%include "gui/guitools.inc"
%include "gui/guimisc.inc"
%include "gui/guimouse.inc"
%include "gui/guiwindp.inc"
%include "gui/guikeys.inc"
%include "gui/guicheat.inc"
%include "gui/guicombo.inc"

; Things to do :
;
; .checkmenuboxclick
; gray scale = 32 .. 63
; shadow = 96 .. 127
; blue scale = 148 .. 167, 168 .. 187
; gray scale = 189 .. 220 (32+137)

; |  Game        Config     Cheat     MultiPlay    Misc
;-------------------------------------------------------
;    Load        Input      Add Code  Modem        Misc Keys
;    Run         -----      Browse    IPX          GUI Opts
;    Reset       Devices    Search                 Movie Opt
;    -----       Chip Cfg                          Key Comb.
;    Save State  -----                             Save Cfg
;    Load State  Options                           -----
;    Pick State  Video                             About
;    -----       Sound
;    Quit        Paths
;                Saves
;                Speed

; NetPlay only has "Internet" for Windows/Linux

; Windows : 1 = Load
;           2 = Chose State
;           3 = Input Device Window
;           4 = Options
;           5 = Video
;           6 = Sound
;           7 = Cheat
;           8 = Net Options
;           9 = Game Options
;           10 = GUI Options
;           11 = About
;           12 = Reset Confirmation
;           13 = Cheat Search
;           14 = SaveState Confirmation
;           15 = Movies
;           16 = Key Combo
;           17 = Devices
;           18 = Chip Config
;           19 = Paths
;           20 = Saves
;           21 = Speed

SECTION .data

NEWSYM GUIPrevMenuData,
  db 1,'1.                            ',0
  db 1,'2.                            ',0
  db 1,'3.                            ',0
  db 1,'4.                            ',0
  db 1,'5.                            ',0
  db 1,'6.                            ',0
  db 1,'7.                            ',0
  db 1,'8.                            ',0
  db 1,'9.                            ',0
  db 1,'0.                            ',0
  db 0,'------------',0
  db 1,'FREEZE DATA: OFF   ',0
  db 1,'CLEAR ALL DATA     ',0

; Window sizes and positions
;                LOAD STAT INPT OPT  VID  SND  CHT  NET  GMKEY GUIOP ABT  RSET SRC  STCN MOVE CMBO ADDO CHIP PATH SAVE SPED
NEWSYM GUIwinposxo, dd 0,6   ,65  ,33  ,42  ,5   ,34  ,6   ,64  ,8    ,5    ,33  ,56  ,64  ,56  ,5   ,3   ,28  ,48  ,6    ,28  ,53
NEWSYM GUIwinposyo, dd 0,20  ,70  ,20  ,20  ,20  ,20  ,20  ,30  ,30   ,20   ,20  ,60  ,30  ,60  ,20  ,20  ,60  ,60  ,20   ,30  ,20
GUIwinsizex dd 0,244 ,126 ,205 ,180 ,245 ,188 ,244 ,128 ,240  ,245  ,190 ,144 ,128 ,144 ,246 ,250 ,200 ,160 ,244  ,200 ,150
GUIwinsizey dd 0,190 ,68  ,192 ,190 ,190 ,188 ,191 ,40  ,170  ,150  ,190 ,42  ,40  ,42  ,190 ,190 ,120 ,100 ,190  ,168 ,180
NEWSYM GUIwinptr, db 0

NEWSYM WaterOn,  db 1
NEWSYM ForceROMTiming, db 0
NEWSYM ForceHiLoROM, db 0
NEWSYM CalibXmin, dd 0
NEWSYM CalibXmax, dd 0
NEWSYM CalibYmin, dd 0
NEWSYM CalibYmax, dd 0
NEWSYM CalibXmin209, dd 0
NEWSYM CalibXmax209, dd 0
NEWSYM CalibYmin209, dd 0
NEWSYM CalibYmax209, dd 0

EEgg db 0
NEWSYM SubPalTable, times 256 db 1      ; Corresponding Gray Scale Color

SECTION .bss
NEWSYM CombinDataGlob, resb 3300 ; 20-name, 42-combo, 2-key#, 1-P#, 1-ff
NEWSYM CombinDataLocl, resb 3300

NEWSYM GUIwinorder, resb 22
GUIwinpos   resb 22
NEWSYM GUIwinactiv, resb 22
ViewBuffer  resb 50*32

NEWSYM GUItextcolor, resb 5
NEWSYM GUIcmenupos,  resb 1
NEWSYM GUIescpress,  resb 1
NEWSYM GUIcwinpress, resb 1
NEWSYM GUIpmenupos,  resb 1
NEWSYM GUIcrowpos,   resd 1
NEWSYM GUIpclicked,  resb 1
GUImouseposx resd 1
GUImouseposy resd 1
NEWSYM GUICYLocPtr,  resd 1
NEWSYM GUIMenuL,     resd 1
NEWSYM GUIMenuR,     resd 1
NEWSYM GUIMenuD,     resd 1
GUIOnMenuItm resb 1
NEWSYM GUIQuit,      resb 1
NEWSYM GUIHold,      resb 1
GUIHoldx     resd 1
GUIHoldy     resd 1
GUIHoldxm    resd 1
GUIHoldym    resd 1
GUIcolscaleval resd 1
NEWSYM cwindrawn,    resb 1
GUIWincol    resd 1
GUIWincoladd resd 1
GUITemp      resd 1
GUIHoldXlimL resd 1
GUIHoldXlimR resd 1
GUIHoldYlim  resd 1
GUIHoldYlimR resd 1
cloadnpos    resd 1
cloadnposb   resd 1
cloadmaxlen  resd 1
cloadnleft   resd 1
cplayernum   resb 1
vbuflimtop   resd 1
vbuflimbot   resd 1
NEWSYM GUIScrolTim1, resd 1
GUIScrolTim2 resd 1
BlankVar     resb 1
GUICHold     resd 1
NEWSYM GUICBHold,    resd 1
GUICBHold2   resd 1
NEWSYM GUIDClickTL,  resd 1
GUIDClCWin   resd 1
GUIDClCEntry resd 1
NEWSYM GUICResetPos, resd 1
NEWSYM GUICStatePos, resd 1
NEWSYM GUICCFlash,   resb 1
NEWSYM GUILDFlash,   resb 1
NEWSYM GUIPalConv,   resd 1
NEWSYM PrevResoln,   resw 1
keycontrolval resd 1
NEWSYM CheatBDoor,   resb 1
NEWSYM ShowTimer,    resb 1
NEWSYM MousePRClick, resb 1
NEWSYM MouseDis, resb 1

NEWSYM CheatOn, resd 1
NEWSYM NumCheats, resd 1
NEWSYM cheatdataprev, resb 28 ; leave contents blank
NEWSYM cheatdata, resb 28*255+56 ; toggle, value, address, pvalue, name(22)

curgsval resb 1

NEWSYM GUICMessage, resd 1
NEWSYM GUICTimer,   resd 1
NEWSYM GUIOn,       resb 1
NEWSYM GUIOn2,      resb 1
NEWSYM GUIReset,    resb 1
NEWSYM CurPalSelect, resb 1
NEWSYM MotionBlur, resb 1

NEWSYM StartLL, resd 1
NEWSYM StartLR, resd 1

NEWSYM TRVal, resw 1
NEWSYM TGVal, resw 1
NEWSYM TBVal, resw 1
NEWSYM TRVali, resw 1
NEWSYM TGVali, resw 1
NEWSYM TBVali, resw 1
NEWSYM TRVal2, resw 1
NEWSYM TGVal2, resw 1
NEWSYM TBVal2, resw 1

SECTION .data
NEWSYM ComboHeader, db 'Key Combination File',26,1,0
NEWSYM ComboBlHeader, times 23 db 0

NEWSYM GUIoldhand9o, dd 0
NEWSYM GUIoldhand9s, dw 0
NEWSYM GUIoldhand8o, dd 0
NEWSYM GUIoldhand8s, dw 0
NEWSYM GUIt1cc,      dd 0
GUIt1ccSwap db 0
GUIskipnextkey42 db 0

SECTION .text
%ifdef __MSDOS__
NEWSYM GUIhandler8h
  cli
  push ds
  push eax
  mov ax,[cs:dssel]
  mov ds,ax
  ccallv GUI36hzcall
  xor byte[GUIt1ccSwap],1
  cmp byte[GUIt1ccSwap],0
  je .nocall
  pushf
  call far [GUIoldhand8o]
.nocall
  mov al,20h
  out 20h,al
  pop eax
  pop ds
  sti
  iretd

NEWSYM GUIhandler9h
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
  cmp byte[GUIskipnextkey42],0
  je .no42
  mov byte[GUIskipnextkey42],0
  jmp .skipkeyrel
.no42
  cmp al,0E0h
  jne .noE0
  mov byte[GUIskipnextkey42],1
  jmp .skipkeyrel
.noE0
  mov byte[GUIskipnextkey42],0
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
  and bl,7Fh
  mov byte[pressed+ebx],0        ; if not, set key to pressed
.skipkeyrel
  mov byte[pressed],0

  pushf
  call far [GUIoldhand9o]
  mov al,20h
  out 20h,al
  pop ebx
  pop eax
  pop ds
  sti
  iretd
%endif

SECTION .data

NEWSYM SantaPos, dd 272
NEWSYM MsgGiftLeft, dd 0

SECTION .bss

NEWSYM Totalbyteloaded, resd 1
NEWSYM sramsavedis, resb 1

NEWSYM GUICPC, resw 256
SECTION .text

NEWSYM GUIUnBuffer
  mov ecx,16384
  ; copy from spritetable
  mov edi,[vidbuffer]
  mov esi,[spritetablea]
  add esi,8*288
  rep movsd
  mov eax,01010101h
  mov ecx,2*288
  rep stosd
  ret

NEWSYM GUIconvpal
  mov ax,[cgram]
  mov [tempco0],ax
  test byte[scaddtype],00100000b
  jz near .noaddition
  test byte[scaddtype],10000000b
  jnz near .noaddition
  mov cx,[cgram]
  mov ax,cx
  and ax,001Fh
  add al,[coladdr]
  cmp al,01Fh
  jb .noadd
  mov al,01Fh
.noadd
  mov bx,ax
  mov ax,cx
  shr ax,5
  and ax,001Fh
  add al,[coladdg]
  cmp al,01Fh
  jb .noaddb
  mov al,01Fh
.noaddb
  shl ax,5
  add bx,ax
  mov ax,cx
  shr ax,10
  and ax,001Fh
  add al,[coladdb]
  cmp al,01Fh
  jb .noaddc
  mov al,01Fh
.noaddc
  shl ax,10
  add bx,ax
  mov [cgram],bx
.noaddition
  mov edi,cgram
  mov ebx,prevpal
  xor ah,ah
.loopa
  mov cx,[edi]
  push eax
  push ebx
  mov [ebx],cx
  mov al,ah
  mov ax,cx
  and al,01Fh
  mov bh,[maxbr]
  mov bl,bh
  mul bl
  mov bl,15
  div bl
  mov [curgsval],al
  mov ax,cx
  shr ax,5
  and al,01Fh
  mov bl,bh
  mul bl
  mov bl,15
  div bl
  add [curgsval],al
  mov ax,cx
  shr ax,10
  and al,01Fh
  mov bl,bh
  mul bl
  mov bl,15
  div bl
  add [curgsval],al
  pop ebx
  pop eax
  add edi,2
  add ebx,2
  push eax
  push ebx
  mov al,ah
  and eax,0FFh
  mov bl,[curgsval]
  push eax
  push ebx
  mov al,bl
  mov bl,3
  xor ah,ah
  div bl
  pop ebx
  mov bl,al
  pop eax
  cmp byte[MessageOn],0
  je .nochange128
  cmp al,128
  jne .nochange128
  mov bl,31
.nochange128
  or bl,bl
  jnz .noadder
  inc bl
.noadder
  mov [SubPalTable+eax],bl
  pop ebx
  pop eax
  inc ah
  jnz near .loopa
  mov al,[maxbr]
  mov [prevbright],al
  mov ax,[tempco0]
  mov [cgram],ax
  ret

convertnum:
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
.loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
.loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    mov [esi],dl
    inc esi
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    mov byte[esi],0
    ret

; eax = value, ecx = # of bytes
converthex:
    mov ebx,ecx
    mov ecx,4
    sub ecx,ebx
    shl ecx,3
    shl eax,cl
    mov ecx,ebx
    xor ebx,ebx
    add ecx,ecx
.loopb
    mov ebx,eax
    and ebx,0F0000000h
    shr ebx,28
    mov dl,[.hexdat+ebx]
    mov [esi],dl
    inc esi
    shl eax,4
    dec ecx
    jnz .loopb
    mov byte[esi],0
    ret

SECTION .data
.hexdat db '0123456789ABCDEF'
GUIMousePtr:
  db 50,47,45,43,40,0 ,0 ,0
  db 53,52,46,42,0 ,0 ,0 ,0
  db 55,54,54,44,0 ,0 ,0 ,0
  db 57,57,56,52,45,0 ,0 ,0
  db 59,0 ,0 ,55,50,45,0 ,0
  db 0 ,0 ,0 ,0 ,55,50,45,0
  db 0 ,0 ,0 ,0 ,0 ,55,50,47
  db 0 ,0 ,0 ,0 ,0 ,0 ,52,0

NEWSYM GUIFontData
; bitmap 5x5 font; char - offset for ASCII2Font
  db 0,0,0,0,0; empty space 0x0
  db 01110000b,10011000b,10101000b,11001000b,01110000b; 0 0x01
  db 00100000b,01100000b,00100000b,00100000b,01110000b; 1 0x02
  db 01110000b,10001000b,00110000b,01000000b,11111000b; 2 0x03
  db 01110000b,10001000b,00110000b,10001000b,01110000b; 3 0x04
  db 01010000b,10010000b,11111000b,00010000b,00010000b; 4 0x05
  db 11111000b,10000000b,11110000b,00001000b,11110000b; 5 0x06
  db 01110000b,10000000b,11110000b,10001000b,01110000b; 6 0x07
  db 11111000b,00001000b,00010000b,00010000b,00010000b; 7 0x08
  db 01110000b,10001000b,01110000b,10001000b,01110000b; 8 0x09
  db 01110000b,10001000b,01111000b,00001000b,01110000b; 9 0x0A
  db 01110000b,10001000b,11111000b,10001000b,10001000b; A 0x0B
  db 11110000b,10001000b,11110000b,10001000b,11110000b; B 0x0C
  db 01110000b,10001000b,10000000b,10001000b,01110000b; C 0x0D
  db 11110000b,10001000b,10001000b,10001000b,11110000b; D 0x0E
  db 11111000b,10000000b,11110000b,10000000b,11111000b; E 0x0F
  db 11111000b,10000000b,11110000b,10000000b,10000000b; F 0x10
  db 01111000b,10000000b,10011000b,10001000b,01110000b; G 0x11
  db 10001000b,10001000b,11111000b,10001000b,10001000b; H 0x12
  db 11111000b,00100000b,00100000b,00100000b,11111000b; I 0x13
  db 01111000b,00010000b,00010000b,10010000b,01100000b; J 0x14
  db 10010000b,10100000b,11100000b,10010000b,10001000b; K 0x15
  db 10000000b,10000000b,10000000b,10000000b,11111000b; L 0x16
  db 11011000b,10101000b,10101000b,10101000b,10001000b; M 0x17
  db 11001000b,10101000b,10101000b,10101000b,10011000b; N 0x18
  db 01110000b,10001000b,10001000b,10001000b,01110000b; O 0x19
  db 11110000b,10001000b,11110000b,10000000b,10000000b; P 0x1A
  db 01110000b,10001000b,10101000b,10010000b,01101000b; Q 0x1B
  db 11110000b,10001000b,11110000b,10010000b,10001000b; R 0x1C
  db 01111000b,10000000b,01110000b,00001000b,11110000b; S 0x1D
  db 11111000b,00100000b,00100000b,00100000b,00100000b; T 0x1E
  db 10001000b,10001000b,10001000b,10001000b,01110000b; U 0x1F
  db 10001000b,10001000b,01010000b,01010000b,00100000b; V 0x20
  db 10001000b,10101000b,10101000b,10101000b,01010000b; W 0x21
  db 10001000b,01010000b,00100000b,01010000b,10001000b; X 0x22
  db 10001000b,01010000b,00100000b,00100000b,00100000b; Y 0x23
  db 11111000b,00010000b,00100000b,01000000b,11111000b; Z 0x24
  db 00000000b,00000000b,11111000b,00000000b,00000000b; - 0x25
  db 00000000b,00000000b,00000000b,00000000b,11111000b; _ 0x26
  db 01101000b,10010000b,00000000b,00000000b,00000000b; ~ 0x27
  db 00000000b,00000000b,00000000b,00000000b,00100000b; . 0x28
  db 00001000b,00010000b,00100000b,01000000b,10000000b; / 0x29
  db 00010000b,00100000b,01000000b,00100000b,00010000b; < 0x2A
  db 01000000b,00100000b,00010000b,00100000b,01000000b; > 0x2B
  db 01110000b,01000000b,01000000b,01000000b,01110000b; [ 0x2C
  db 01110000b,00010000b,00010000b,00010000b,01110000b; ] 0x2D
  db 00000000b,00100000b,00000000b,00100000b,00000000b; : 0x2E
  db 01100000b,10011000b,01110000b,10011000b,01101000b; & 0x2F
  db 00100000b,00100000b,10101000b,01110000b,00100000b; arrow down 0x30
  db 01010000b,11111000b,01010000b,11111000b,01010000b; # 0x31
  db 00000000b,11111000b,00000000b,11111000b,00000000b; = 0x32
  db 01001000b,10010000b,00000000b,00000000b,00000000b; " 0x33
  db 10000000b,01000000b,00100000b,00010000b,00001000b; \ 0x34
  db 10101000b,01110000b,11111000b,01110000b,10101000b; * 0x35
  db 01110000b,10001000b,00110000b,00000000b,00100000b; ? 0x36
  db 10001000b,00010000b,00100000b,01000000b,10001000b; % 0x37
  db 00100000b,00100000b,11111000b,00100000b,00100000b; + 0x38
  db 00000000b,00000000b,00000000b,00100000b,01000000b; , 0x39
  db 00110000b,01000000b,01000000b,01000000b,00110000b; ( 0x3A
  db 01100000b,00010000b,00010000b,00010000b,01100000b; ) 0x3B
  db 01110000b,10011000b,10111000b,10000000b,01110000b; @ 0x3C
  db 00100000b,01000000b,00000000b,00000000b,00000000b; ' 0x3D
  db 00100000b,00100000b,00100000b,00000000b,00100000b; ! 0x3E
  db 01111000b,10100000b,01110000b,00101000b,11110000b; $ 0x3F
  db 00000000b,00100000b,00000000b,00100000b,01000000b; ; 0x40
  db 01000000b,00100000b,00000000b,00000000b,00000000b; ` 0x41
  db 00100000b,01010000b,00000000b,00000000b,00000000b; ^ 0x42
  db 00110000b,01000000b,11000000b,01000000b,00110000b; { 0x43
  db 01100000b,00010000b,00011000b,00010000b,01100000b; } 0x44
  db 00100000b,00100000b,01110000b,01110000b,11111000b; up 0x45
  db 11111000b,01110000b,01110000b,00100000b,00100000b; down 0x46
  db 00001000b,00111000b,11111000b,00111000b,00001000b; left 0x47
  db 10000000b,11100000b,11111000b,11100000b,10000000b; right 0x48
  db 00100000b,01100000b,11111000b,01100000b,00100000b; arrow left 0x49
  db 00111000b,00100000b,00110000b,00001000b,10110000b; .5 0x4A
  db 11111100b,10000100b,11111100b,00000000b,00000000b; maximize (Win) 0x4B
  db 00000000b,11111100b,00000000b,00000000b,00000000b; minimize (Win) 0x4C
  db 11111000b,10001000b,10001000b,10001000b,11111000b; maximize (SDL) 0x4D
  db 00000000b,00000000b,00100000b,01010000b,00100000b; shw fullstop 0x4E
  db 01110000b,01000000b,01000000b,01000000b,00000000b; shw left bracket 0x4F
  db 00000000b,00010000b,00010000b,00010000b,01110000b; shw right bracket 0x50
  db 00000000b,00000000b,00000000b,01000000b,00100000b; shw comma 0x51
  db 00000000b,00100000b,01110000b,00100000b,00000000b; shw mid-dot 0x52
  db 11111000b,00001000b,11110000b,00100000b,11000000b; shw wo 0x53
  db 00000000b,11111000b,01010000b,01100000b,01000000b; shw mini a 0x54
  db 00000000b,00010000b,00100000b,11100000b,00100000b; shw mini i 0x55
  db 00000000b,00100000b,11111000b,10001000b,00110000b; shw mini u 0x56
  db 00000000b,00000000b,11111000b,00100000b,11111000b; shw mini e 0x57
  db 00000000b,00010000b,11111000b,00110000b,11010000b; shw mini o 0x58
  db 00000000b,01000000b,11111000b,01010000b,01000000b; shw mini ya 0x59
  db 00000000b,00000000b,11110000b,00010000b,11111000b; shw mini yu 0x5A
  db 00000000b,11111000b,00001000b,01111000b,11111000b; shw mini yo 0x5B
  db 00000000b,10101000b,10101000b,00010000b,01100000b; shw mini tsu 0x5C
  db 00000000b,10000000b,01111000b,00000000b,00000000b; shw prolong 0x5D
  db 11111000b,00101000b,00110000b,00100000b,11000000b; shw a 0x5E
  db 00001000b,00110000b,11100000b,00100000b,00100000b; shw i 0x5F
  db 00100000b,11111000b,10001000b,00010000b,01100000b; shw u 0x60
  db 11111000b,00100000b,00100000b,00100000b,11111000b; shw e 0x61
  db 00010000b,11111000b,00110000b,01010000b,10010000b; shw o 0x62
  db 01000000b,11111000b,01001000b,01001000b,10011000b; shw ka 0x63
  db 00100000b,11111000b,00100000b,11111000b,00100000b; shw ki 0x64
  db 01000000b,01111000b,10001000b,00010000b,01100000b; shw ku 0x65
  db 01000000b,01111000b,10010000b,00010000b,01100000b; shw ke 0x66 ^^
  db 11111000b,00001000b,00001000b,00001000b,11111000b; shw ko 0x67
  db 01010000b,11111000b,01010000b,00010000b,01100000b; shw sa 0x68
  db 01000000b,10101000b,01001000b,00010000b,11100000b; shw shi 0x69
  db 11111000b,00001000b,00010000b,00110000b,11001000b; shw su 0x6A
  db 01000000b,11111000b,01010000b,01000000b,00111000b; shw se 0x6B
  db 10001000b,01001000b,00001000b,00010000b,01100000b; shw so 0x6C
  db 01000000b,01111000b,11001000b,00110000b,01100000b; shw ta 0x6D
  db 11111000b,00100000b,11111000b,00100000b,01000000b; shw chi 0x6E
  db 10101000b,10101000b,00001000b,00010000b,01100000b; shw tsu 0x6F
  db 11111000b,00000000b,11111000b,00100000b,11000000b; shw te 0x70
  db 01000000b,01000000b,01100000b,01010000b,01000000b; shw to 0x71
  db 00100000b,11111000b,00100000b,00100000b,01000000b; shw na 0x72
  db 11110000b,00000000b,00000000b,00000000b,11111000b; shw ni 0x73
  db 11111000b,00001000b,00101000b,00010000b,01101000b; shw nu 0x74
  db 00100000b,11111000b,00001000b,01110000b,10101000b; shw ne 0x75
  db 00001000b,00001000b,00001000b,00010000b,01100000b; shw no 0x76
  db 01010000b,01010000b,01010000b,10001000b,10001000b; shw ha 0x77
  db 10000000b,10011000b,11100000b,10000000b,01111000b; shw hi 0x78
  db 11111000b,00001000b,00001000b,00010000b,01100000b; shw hu 0x79
  db 01000000b,10100000b,10010000b,00001000b,00000000b; shw he 0x7A
  db 00100000b,11111000b,01110000b,10101000b,00100000b; shw ho 0x7B
  db 11111000b,00001000b,10010000b,01100000b,00100000b; shw ma 0x7C
  db 11111000b,00000000b,11111000b,00000000b,11111000b; shw mi 0x7D
  db 00100000b,01000000b,01000000b,10010000b,11111000b; shw mu 0x7E
  db 00001000b,01001000b,00110000b,00110000b,11001000b; shw me 0x7F
  db 11111000b,00100000b,11111000b,00100000b,00111000b; shw mo 0x80
  db 01000000b,11111100b,01001000b,00100000b,00100000b; shw ya 0x81
  db 11110000b,00010000b,00010000b,00010000b,11111000b; shw yu 0x82
  db 11111000b,00001000b,11111000b,00001000b,11111000b; shw yo 0x83
  db 11111000b,00000000b,11111000b,00010000b,01100000b; shw ra 0x84
  db 10001000b,10001000b,10001000b,00010000b,01100000b; shw ri 0x85
  db 01100000b,01100000b,01101000b,01101000b,10110000b; shw ru 0x86
  db 10000000b,10000000b,10001000b,10001000b,11110000b; shw re 0x87
  db 11111000b,10001000b,10001000b,10001000b,11111000b; shw ro 0x88
  db 11111000b,10001000b,00001000b,00010000b,01100000b; shw wa 0x89
  db 10000000b,01001000b,00001000b,00010000b,11100000b; shw n 0x8A
  db 10100000b,10100000b,00000000b,00000000b,00000000b; shw voiced 0x8B
  db 01000000b,10100000b,01000000b,00000000b,00000000b; shw halfvoiced 0x8C

NEWSYM GUIFontData1, times 705 db 0

; 189 .. 220
GUIIconDataClose:
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,216,216,216,216,216,216,216,0  ,0
  db 214,212,202,212,212,212,202,212,210,0
  db 214,212,212,200,212,200,212,212,210,202
  db 214,212,212,212,198,212,212,212,210,202
  db 214,212,212,196,212,196,212,212,210,200
  db 214,212,194,212,212,212,194,212,210,200
  db 0  ,208,208,208,208,208,208,208,198,198
  db 0  ,0  ,198,198,198,198,198,198,198,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

GUIIconDataButtonHole:
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,207,205,207,0  ,0  ,0  ,0
  db 0  ,0  ,207,203,202,203,207,0  ,0  ,0
  db 0  ,207,203,200,198,200,203,207,0  ,0
  db 0  ,207,202,198,197,198,202,207,0  ,0
  db 0  ,207,203,200,198,200,203,207,0  ,0
  db 0  ,0  ,207,203,202,203,207,0  ,0  ,0
  db 0  ,0  ,0  ,207,205,207,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

GUIIconDataButtonFill:
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,207,209,207,0  ,0  ,0  ,0
  db 0  ,0  ,207,211,212,211,207,0  ,0  ,0
  db 0  ,207,211,214,216,214,211,207,0  ,0
  db 0  ,207,212,216,217,216,212,207,0  ,0
  db 0  ,207,211,214,216,214,211,207,0  ,0
  db 0  ,0  ,207,211,212,211,207,0  ,0  ,0
  db 0  ,0  ,0  ,207,209,207,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

GUIIconDataSlideBar:
  db 0  ,0  ,0  ,0  ,216,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,212,216,220,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,212,216,220,202,0  ,0  ,0
  db 0  ,0  ,212,212,216,218,220,0  ,0  ,0
  db 0  ,0  ,212,214,216,218,220,202,0  ,0
  db 0  ,0  ,212,214,216,218,220,202,0  ,0
  db 0  ,0  ,0  ,212,216,220,202,202,0  ,0
  db 0  ,0  ,0  ,212,216,220,202,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,216,202,202,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,202,0  ,0  ,0  ,0

GUIIconDataCheckBoxUC:
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,220,219,218,217,216,215,0  ,0  ,0
  db 0  ,219,218,217,216,215,214,202,0  ,0
  db 0  ,218,217,216,215,214,213,202,0  ,0
  db 0  ,217,216,215,214,213,212,202,0  ,0
  db 0  ,216,215,214,213,212,211,202,0  ,0
  db 0  ,215,214,213,212,211,210,202,0  ,0
  db 0  ,0  ,202,202,202,202,202,202,0  ,0

GUIIconDataCheckBoxC:
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,165,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,165,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,165,0  ,0  ,0
  db 0  ,220,219,218,217,165,215,0  ,0  ,0
  db 0  ,165,165,217,165,165,214,202,0  ,0
  db 0  ,218,165,216,165,214,213,202,0  ,0
  db 0  ,217,165,165,165,213,212,202,0  ,0
  db 0  ,216,215,165,213,212,211,202,0  ,0
  db 0  ,215,214,165,212,211,210,202,0  ,0
  db 0  ,0  ,202,202,202,202,202,202,0  ,0

GUIIconDataUpArrow:
  db 201,209,209,209,209,209,209,200,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,205,201,202,203,202,205,203,0  ,0
  db 207,200,205,202,203,205,201,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 199,201,201,201,201,201,201,198,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

GUIIconDataDownArrow:
  db 201,209,209,209,209,209,209,200,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 207,200,205,202,203,205,201,203,0  ,0
  db 207,205,201,202,203,202,205,203,0  ,0
  db 207,205,205,202,203,205,205,203,0  ,0
  db 199,201,201,201,201,201,201,198,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

NEWSYM SnowData
dw 161,251,115,211,249,87,128,101,232,176,51,180,108,193,224,112,254,159,102,238
dw 223,123,218,42,173,160,143,170,64,1,174,29,34,187,194,199,40,89,232,32
dw 7,195,141,67,216,48,234,1,243,116,164,182,146,136,66,70,36,43,98,208
dw 63,240,216,253,147,36,33,253,98,80,228,156,73,82,85,1,97,72,187,239
dw 18,196,127,182,22,22,101,25,124,145,240,213,186,22,7,161,30,98,90,197
dw 22,205,32,150,59,133,49,140,10,128,142,185,176,142,220,195,100,102,105,194
dw 43,139,184,153,1,95,176,169,192,201,233,243,73,65,188,14,194,39,251,140
dw 239,181,142,160,242,248,82,49,9,157,233,162,254,121,112,6,118,24,56,121
dw 74,209,1,223,145,6,75,73,18,168,194,168,58,39,222,170,214,75,45,218
dw 39,197,242,98,22,90,255,5,144,244,252,55,98,18,135,101,27,85,215,207
dw 183,28,201,142,45,122,145,159,41,243,109,29,117,203,7,234,231,214,131,133
dw 217,8,74,207,130,77,21,229,167,78,218,109,142,58,134,238,29,182,178,14
dw 144,129,196,219,60,128,30,105,57,53,76,122,242,208,101,241,246,99,248,67
dw 137,244,70,51,202,94,164,125,115,72,61,72,129,169,155,122,91,154,160,83
dw 41,102,223,218,140,40,132,16,223,92,50,230,168,47,126,117,242,136,1,245
dw 171,0,36,98,73,69,14,229,66,177,108,92,39,250,243,161,111,85,211,99
dw 52,98,121,188,128,201,90,205,223,92,177,19,87,18,75,54,6,81,235,137
dw 247,66,211,129,247,39,119,206,116,250,113,231,190,196,53,51,34,114,39,22
dw 192,33,249,151,26,22,139,97,171,238,182,88,22,176,157,255,178,199,138,98
dw 140,36,112,90,25,245,134,64,48,190,165,113,24,195,84,70,175,9,179,69
dw 13,26,167,237,163,159,185,128,109,114,86,74,188,103,141,48,188,203,205,191
dw 215,193,224,4,153,36,108,3,172,235,56,251,211,115,173,216,240,33,78,150
dw 133,64,51,103,56,26,165,222,70,148,115,119,246,229,181,63,109,49,228,108
dw 126,10,170,48,87,42,193,24,28,255,176,176,209,181,97,93,61,241,201,137
dw 129,97,24,159,168,215,61,113,104,143,168,7,196,216,149,239,110,65,75,143
dw 238,0,37,19,8,56,65,234,228,72,42,5,226,95,243,51,55,231,114,90
dw 160,141,171,108,218,252,154,64,175,142,214,211,180,129,217,118,33,130,213,2
dw 73,145,93,21,162,141,97,225,112,253,49,43,113,208,131,104,31,51,192,37
dw 117,186,16,45,61,114,220,6,89,163,197,203,142,80,89,115,190,190,228,15
dw 166,145,59,139,120,79,104,252,246,73,113,144,224,65,204,155,221,85,31,99
dw 48,253,94,159,215,31,123,204,248,153,31,210,174,178,54,146,152,88,56,92
dw 197,35,124,104,211,118,1,207,108,68,123,161,107,69,143,13,79,170,130,193
dw 214,153,219,247,227,2,170,208,248,139,118,241,247,183,18,135,246,126,201,46
dw 70,234,171,72,18,135,236,216,32,178,148,231,161,15,6,254,34,181,5,71
dw 2,219,71,87,252,16,202,190,180,83,99,209,75,134,78,84,114,32,171,246
dw 125,11,57,200,102,29,176,26,205,151,152,108,100,146,117,95,71,77,158,207
dw 60,192,50,135,223,237,231,53,27,195,170,146,155,160,92,224,247,187,14,50
dw 203,5,153,42,17,75,109,14,78,160,236,114,131,105,189,209,233,135,221,207
dw 226,119,104,10,178,107,77,160,233,179,120,227,133,241,32,223,63,247,66,157
dw 140,81,118,81,63,193,173,228,214,78,124,123,222,149,9,242,0,128,194,110

NEWSYM SnowVelDist
db 57,92,100,19,100,184,238,225,55,240,255,221,215,105,226,153,164,41,22,93
db 176,203,155,199,244,52,233,219,110,227,229,227,152,240,83,248,226,31,163,22
db 28,156,18,10,248,67,123,167,25,138,90,10,79,107,208,229,248,233,185,10
db 167,21,19,178,132,154,81,70,20,71,95,147,72,27,91,189,13,189,102,84
db 195,123,251,93,68,36,178,59,107,99,104,191,76,110,44,206,123,46,98,112
db 26,50,1,35,150,17,242,208,69,23,202,197,59,80,136,124,40,89,11,40
db 1,136,90,72,198,83,2,174,174,4,28,205,135,35,194,54,22,40,4,132
db 191,88,163,66,204,230,35,111,9,177,254,174,163,68,5,88,111,235,58,236
db 4,248,172,154,101,164,43,223,10,13,210,125,146,73,192,57,117,152,128,36
db 106,21,253,113,110,133,244,4,150,32,76,71,22,106,210,244,46,128,27,215
db 231,112,177,196,198,120,196,57,234,74,235,108,64,181,209,188,177,63,197,200
db 126,164,136,163,48,62,225,223,212,201,195,121,90,7,10,196,88,53,39,249
db 147,98,65,253,246,3,152,125,242,105,44,129,94,232,13,4,86,220,194,67
db 186,210,171,197,64,138,89,78,58,150,52,79,138,201,244,111,106,181,192,69
db 234,253,239,113,98,37,209,151,60,47,241,235,185,52,173,94,172,182,47,150
db 80,118,10,58,161,237,10,64,238,198,14,74,132,250,234,63,169,86,158,170
db 76,168,124,133,28,203,246,140,228,77,50,53,115,113,157,218,90,192,28,209
db 72,117,156,101,226,99,11,245,69,59,17,175,164,59,8,166,163,185,10,60
db 100,19,26,38,114,232,180,115,238,184,88,103,178,67,212,21,87,64,85,1
db 62,87,155,62,21,96,205,195,131,97,191,252,218,209,179,201,12,2,234,110
db 162,14,145,170,156,105,85,132,132,60,239,14,80,129,225,144,149,244,188,8
db 13,168,181,168,30,142,24,110,26,172,231,182,50,214,66,193,100,45,132,144
db 205,190,16,133,45,250,83,183,140,229,117,226,68,59,163,96,235,227,25,155
db 209,105,41,214,30,107,2,85,180,23,241,39,113,63,75,44,107,142,93,29
db 62,240,235,152,147,52,54,146,109,112,139,162,238,198,201,8,141,115,112,106
db 4,99,25,155,111,161,114,253,75,100,28,59,101,150,2,122,228,6,12,59
db 249,181,67,136,227,227,199,46,75,203,50,25,50,61,62,22,238,124,218,134
db 243,21,243,222,94,138,161,234,133,23,138,45,4,226,154,227,8,84,105,126
db 200,127,240,144,124,197,102,144,53,29,94,231,108,175,136,37,44,183,178,95
db 41,196,214,12,42,221,106,225,151,32,53,130,24,211,88,14,135,18,90,219
db 177,129,90,217,162,181,199,133,116,56,36,100,230,91,220,83,41,65,20,64
db 177,197,249,24,242,62,26,234,92,44,167,153,243,94,179,163,103,29,220,199
db 128,94,236,152,53,32,77,78,228,89,124,85,87,50,197,116,179,105,236,139
db 102,17,159,66,176,27,205,36,113,80,60,6,61,174,254,174,246,72,154,31
db 97,40,10,8,114,203,238,26,89,51,134,110,118,176,87,32,192,210,146,207
db 88,45,156,179,61,224,87,107,107,1,252,187,203,100,169,211,205,105,12,231
db 137,176,166,37,192,241,169,84,32,85,112,168,154,7,247,146,183,225,246,173
db 57,103,110,236,113,118,203,200,22,87,251,7,138,37,12,84,221,171,51,209
db 242,37,89,73,151,162,139,189,131,209,221,96,107,144,175,79,199,123,98,138
db 226,86,221,254,72,14,126,180,200,171,85,94,120,124,196,225,150,57,219,158
