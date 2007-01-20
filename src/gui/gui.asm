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

EXTSYM curblank,vidpastecopyscr,frameskip,newengen,cvidmode,antienab
EXTSYM soundon,StereoSound,SoundQuality,MusicRelVol,endprog
EXTSYM continueprog,spcBuffera,cbitmode,t1cc
EXTSYM romloadskip,romdata,init65816,current_zst
EXTSYM procexecloop,SPCRAM,spcPCRam,spcS,spcRamDP,spcA,spcX,spcY,spcP,spcNZ
EXTSYM Voice0Status,Voice1Status,Voice2Status,Voice3Status,Voice4Status
EXTSYM Voice5Status,Voice6Status,Voice7Status,statesaver,loadstate2
EXTSYM vidbuffer,ASCII2Font,hirestiledat,showallext,scanlines
EXTSYM sprlefttot,spritetablea,KeyRTRCycle
EXTSYM cgram,tempco0,prevbright,maxbr,prevpal,coladdr,coladdg
EXTSYM coladdb,scaddtype,initvideo,pressed,UpdateDevices,memtabler8
EXTSYM memtablew8,writeon,JoyRead,SetInputDevice,delay,FPSOn,RevStereo,WDSPReg0C
EXTSYM WDSPReg1C,pl12s34,resolutn,Makemode7Table,vidbufferofsb,wramdata,bgfixer
EXTSYM videotroub,CheatCodeSave,CheatCodeLoad,LoadCheatSearchFile
EXTSYM SaveCheatSearchFile,Get_Date,Check_Key,Get_Key,sram
EXTSYM TripBufAvail,ResetTripleBuf,ScanCodeListing
EXTSYM AdjustFrequency,GUISaveVars,Init_Mouse,Get_MouseData,Set_MouseXMax
EXTSYM Set_MouseYMax,Set_MousePosition,Get_MousePositionDisplacement,GUIInit
EXTSYM GUIDeInit,SpecialLine,DrawWater,DrawBurn,DrawSmoke
EXTSYM GetDate,horizon_get,ErrorPointer,MessageOn,GetTime
EXTSYM GetScreen,Clear2xSaIBuffer,MouseWindow,ExitFromGUI
EXTSYM newgfx16b,NumVideoModes,MusicVol,DSPMem,NumInputDevices
EXTSYM GUIInputNames,GUIVideoModeNames,GameSpecificInput,device1,device2,TwelveHourClock
EXTSYM GUIM7VID,GUINTVID,GUIHQ2X,RawDumpInProgress
EXTSYM MultiTap,SFXEnable,RestoreSystemVars
EXTSYM nssdip1,nssdip2,nssdip3,nssdip4,nssdip5,nssdip6
EXTSYM SkipMovie,MovieStop,MoviePlay,MovieRecord
EXTSYM MovieInsertChapter,MovieSeekAhead,MovieSeekBehind,ResetDuringMovie
EXTSYM MovieDumpRaw,MovieAppend,AutoLoadCht,GUIQuickLoadUpdate,GUILoadData

EXTSYM GUIwinposx,GUIwinposy,maxskip,GUIEffect,hqFilter,En2xSaI,NTSCFilter
EXTSYM NTSCBlend,NTSCHue,NTSCSat,NTSCCont,NTSCBright,NTSCSharp,NTSCRef
EXTSYM NTSCGamma,NTSCRes,NTSCArt,NTSCFringe,NTSCBleed,NTSCWarp
EXTSYM LowPassFilterType,MovieStartMethod,MovieDisplayFrame,savewinpos
EXTSYM SnapPath,SPCPath,BSXPath,SGPath,STPath,GNextPath,FEOEZPath,SJNSPath
EXTSYM MDHPath,SPL4Path,SRAMPath,CheatSrcByteSize,prevloadfnamel
EXTSYM prevloadiname,prevloaddnamel,prevlfreeze,FirstTimeData,MMXSupport
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
EXTSYM KeyWinDisble,KeyOffsetMSw,JoyPad1Move,init_save_paths,loadquickfname
EXTSYM mousewrap,GUIRClick,SaveSramData,SwapMouseButtons
EXTSYM FPSAtStart,Turbo30hz,TimerEnable,SmallMsgText,mouse1lh,mouse2lh
EXTSYM AutoPatch,RomInfo,AllowUDLR,GrayscaleMode,GUIMovieForcedText
EXTSYM Mode7HiRes16b,FFRatio,SDRatio,EmuSpeed,mouseshad,MovieForcedLengthEnabled
EXTSYM esctomenu,GUILoadKeysJumpTo,lhguimouse,MZTForceRTR,GetMovieForcedLength
EXTSYM GUIEnableTransp,FilteredGUI,Surround,SPCDisable,nosaveSRAM
EXTSYM FastFwdToggle,gui_key,gui_key_extended,GUILoadKeysNavigate
EXTSYM KeyDisplayBatt,KeyIncreaseGamma,KeyDecreaseGamma
EXTSYM MovieVideoMode,MovieAudio,MovieVideoAudio,MovieAudioCompress,newfont
EXTSYM d_names,selected_names,GUIfileentries,GUIdirentries,GUIcurrentdirviewloc
EXTSYM GUIcurrentfilewin,GUIcurrentcursloc,GUIcurrentviewloc,SetMovieForcedLength
EXTSYM GUIcurrentdircursloc,GetLoadData,ZRomPath,SaveSecondState,ClockBox,DisplayInfo
EXTSYM GUIJT_currentviewloc,GUIJT_currentcursloc,GUIJT_entries,ScreenShotFormat
EXTSYM GUIJT_offset,GUIJT_viewable,GUIGenericJumpTo,SSAutoFire,SSPause

%ifdef __UNIXSDL__
EXTSYM numlockptr
%elifdef __WIN32__
EXTSYM initDirectDraw,reInitSound,CheckAlwaysOnTop,CheckPriority,AlwaysOnTop
EXTSYM CheckScreenSaver,MouseWheel,TrapMouseCursor,AllowMultipleInst,TripleBufferWin
EXTSYM HighPriority,DisableScreenSaver,SaveMainWindowPos,PrimaryBuffer
EXTSYM CBBuffer,CBLength,PasteClipBoard,ctrlptr,PauseFocusChange
%elifdef __MSDOS__
EXTSYM dssel,SetInputDevice209,initvideo2,Force8b,SBHDMA,vibracard,smallscreenon
EXTSYM pl1p209,pl2p209,pl3p209,pl4p209,pl5p209,SidewinderFix,Triplebufen,ScreenScale
EXTSYM GUIEAVID,GUIFSVID,GUIWSVID,GUISSVID,GUITBVID,GUISLVID,GUIHSVID,GUI2xVID
EXTSYM JoyMinX209,JoyMaxX209,JoyMinY209,JoyMaxY209,DOSClearScreen,dosmakepal
%endif

%ifndef __MSDOS__
EXTSYM ZsnesPage,DocsPage,GUICustomX,GUICustomY,GetCustomXY,SetCustomXY,initwinvideo
EXTSYM Keep4_3Ratio,PrevFSMode,PrevWinMode,NTSCFilterInit,hqFilterlevel
EXTSYM GUIWFVID,GUIDSIZE,GUIHQ3X,GUIHQ4X,GUIKEEP43,Keep43Check,changeRes
%endif

%ifndef __WIN32__
EXTSYM GUII2VID
%endif

%ifndef __UNIXSDL__
EXTSYM vsyncon
%endif

%ifdef __OPENGL__
EXTSYM BilinearFilter,GUIBIFIL,drawscreenwin,blinit
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

;The first byte is the number of fields on the right not including the seperators
MenuDat1 db 12, 3,1,1,1,1,1,1,1,1,1,0,1,2,0
MenuDat2 db 8,  3,1,1,0,1,1,1,0,2,0
MenuDat3 db 10, 3,0,1,1,0,1,1,1,1,1,2,0
MenuDat4 db 2,  3,1,2,0
%ifndef __MSDOS__
MenuDat5 db 0,  2,0,0
%else
MenuDat5 db 1,  3,2,0
%endif
MenuDat6 db 6,  3,1,1,1,1,0,2,0

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
GUIGameMenuData:
  db 1,'LOAD        ',0
  db 1,'RUN  [ESC]  ',0
  db 1,'RESET       ',0
  db 0,'------------',0
  db 1,'SAVE STATE  ',0
  db 1,'OPEN STATE  ',0
  db 1,'PICK STATE  ',0
  db 0,'------------',0
  db 1,'QUIT        ',0
GUIConfigMenuData:
  db 1,'INPUT       ',0
  db 0,'------------',0
  db 1,'DEVICES     ',0
  db 1,'CHIP CFG    ',0
  db 0,'------------',0
  db 1,'OPTIONS     ',0
  db 1,'VIDEO       ',0
  db 1,'SOUND       ',0
  db 1,'PATHS       ',0
  db 1,'SAVES       ',0
  db 1,'SPEED       ',0
GUICheatMenuData:
  db 1,'ADD CODE    ',0
  db 1,'BROWSE      ',0
  db 1,'SEARCH      ',0
GUINetPlayMenuData:
%ifndef __MSDOS__
  db 1,'INTERNET    ',0
  db 0,'------------',0
%else
  db 1,'MODEM       ',0
  db 1,'IPX         ',0
%endif
GUIMiscMenuData:
  db 1,'MISC KEYS   ',0
  db 1,'GUI OPTS    ',0
  db 1,'MOVIE OPT   ',0
  db 1,'KEY COMB.   ',0
  db 1,'SAVE CFG    ',0
  db 0,'------------',0
  db 1,'ABOUT       ',0

; Window sizes and positions
;                LOAD STAT INPT OPT  VID  SND  CHT  NET  GMKEY GUIOP ABT  RSET SRC  STCN MOVE CMBO ADDO CHIP PATH SAVE SPED
GUIwinposxo dd 0,6   ,65  ,33  ,42  ,5   ,34  ,6   ,64  ,8    ,5    ,33  ,56  ,64  ,56  ,5   ,3   ,28  ,48  ,6    ,28  ,53
GUIwinposyo dd 0,20  ,70  ,20  ,20  ,20  ,20  ,20  ,30  ,30   ,20   ,20  ,60  ,30  ,60  ,20  ,20  ,60  ,60  ,20   ,30  ,20
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
RestoreValues db 0
NEWSYM numdrives, dd 26
SubPalTable times 256 db 1      ; Corresponding Gray Scale Color

SECTION .bss
NEWSYM CombinDataGlob, resb 3300 ; 20-name, 42-combo, 2-key#, 1-P#, 1-ff
NEWSYM CombinDataLocl, resb 3300

NEWSYM GUIwinorder, resb 22
GUIwinpos   resb 22
NEWSYM GUIwinactiv, resb 22
ViewBuffer  resb 50*32

GUItextcolor resb 5
NEWSYM GUIcmenupos, resb 1
GUIescpress  resb 1
GUIcwinpress resb 1
NEWSYM GUIpmenupos, resb 1
GUIcrowpos   resd 1
GUIpclicked  resb 1
GUImouseposx resd 1
GUImouseposy resd 1
GUICYLocPtr  resd 1
GUIMenuL     resd 1
GUIMenuR     resd 1
GUIMenuD     resd 1
GUIOnMenuItm resb 1
NEWSYM GUIQuit, resb 1
GUIHold      resb 1
GUIHoldx     resd 1
GUIHoldy     resd 1
GUIHoldxm    resd 1
GUIHoldym    resd 1
GUIcolscaleval resd 1
cwindrawn    resb 1
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
GUIScrolTim1 resd 1
GUIScrolTim2 resd 1
BlankVar     resb 1
GUICHold     resd 1
NEWSYM GUICBHold,    resd 1
GUICBHold2   resd 1
GUIDClickTL  resd 1
GUIDClCWin   resd 1
GUIDClCEntry resd 1
GUICResetPos resd 1
GUICStatePos resd 1
GUICCFlash   resb 1
GUILDFlash   resb 1
GUIPalConv   resd 1
PrevResoln   resw 1
SnowMover    resd 1
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

SECTION .text
%macro GUIInitIRQs 0
  call GUIInit
  mov esi,pressed
  mov ecx,256
  mov al,0
.loopa
  mov [esi],al
  inc esi
  dec ecx
  jnz .loopa
%endmacro

%macro GUIDeInitIRQs 0
  call GUIDeInit
%endmacro

SECTION .data
NEWSYM GUIoldhand9o, dd 0
NEWSYM GUIoldhand9s, dw 0
NEWSYM GUIoldhand8o, dd 0
NEWSYM GUIoldhand8s, dw 0
GUIt1cc dd 0
GUIt1ccSwap db 0
GUIskipnextkey42 db 0

SECTION .text
NEWSYM GUIinit18_2hz
  mov al,00110110b
  out 43h,al
  mov ax,0
  out 40h,al
  mov al,ah
  out 40h,al
  ret

NEWSYM GUIinit36_4hz
  mov al,00110110b
  out 43h,al
  mov ax,32768
  out 40h,al
  mov al,ah
  out 40h,al
  ret

NEWSYM GUI36hzcall
  inc dword[GUIt1cc]
  inc dword[SnowMover]
  cmp dword[GUIEditStringLTxt],0
  je .nodec
  dec dword[GUIEditStringLTxt]
.nodec
  cmp dword[GUIScrolTim1],0
  je .nodec4
  dec dword[GUIScrolTim1]
.nodec4
  cmp dword[GUIDClickTL],0
  je .nodec2
  dec dword[GUIDClickTL]
.nodec2
  cmp dword[GUIkeydelay],0
  je .nodec3
  dec dword[GUIkeydelay]
.nodec3
  cmp dword[GUIkeydelay2],0
  je .nodec3b
  dec dword[GUIkeydelay2]
.nodec3b
  cmp dword[GUICTimer],0
  je .nodec6
  dec dword[GUICTimer]
.nodec6
  inc byte[GUICCFlash]
  and byte[GUICCFlash],0Fh
  inc byte[GUILDFlash]
  and byte[GUILDFlash],0Fh
  ret

%ifdef __MSDOS__
NEWSYM GUIhandler8h
  cli
  push ds
  push eax
  mov ax,[cs:dssel]
  mov ds,ax
  call GUI36hzcall
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

%macro loadmenuopen 1
  mov al,[GUIcmenupos]
  mov [GUIpmenupos],al
  mov byte[GUIcmenupos],0
  cmp byte[GUIwinactiv+%1],1
  je %%menuontop
  xor eax,eax
  mov al,[GUIwinptr]
  inc byte[GUIwinptr]
  mov byte[GUIwinorder+eax],%1
  mov byte[GUIwinactiv+%1],1
  cmp byte[savewinpos],0
  jne %%nomenuitem
  mov eax,[GUIwinposxo+%1*4]
  mov [GUIwinposx+%1*4],eax
  mov eax,[GUIwinposyo+%1*4]
  mov [GUIwinposy+%1*4],eax
  jmp %%nomenuitem
%%menuontop
  xor eax,eax
  ; look for match
%%notfoundyet
  mov bl,[GUIwinorder+eax]
  cmp bl,%1
  je %%nextfind
  inc eax
  jmp %%notfoundyet
%%nextfind
  inc eax
  cmp al,[GUIwinptr]
  je %%foundend
  mov cl,[GUIwinorder+eax]
  mov [GUIwinorder+eax-1],cl
  jmp %%nextfind
%%foundend
  mov byte[GUIpclicked],0
  mov [GUIwinorder+eax-1],bl
%%nomenuitem
%endmacro

loadnetopen:
  loadmenuopen 8
  ret

SECTION .bss
MouseInitOkay resb 1
SECTION .text

LoadDetermine:
  mov byte[GUIGameMenuData+14],1
  mov byte[GUIGameMenuData+14*2],1
  mov byte[GUIGameMenuData+14*4],1
  mov byte[GUIGameMenuData+14*5],1
  mov byte[GUIGameMenuData+14*6],1
  mov byte[GUICheatMenuData],1
  mov byte[GUICheatMenuData+14],1
  mov byte[GUICheatMenuData+14*2],1
  mov byte[GUIMiscMenuData+14*2],1
  mov byte[GUINetPlayMenuData],2             ; Gray out Netplay options
%ifdef __MSDOS__
  mov byte[GUINetPlayMenuData+14],2
%endif
  cmp byte[romloadskip],0
  je .noromloaded
  mov byte[GUIGameMenuData+14],2
  mov byte[GUIGameMenuData+14*2],2
  mov byte[GUIGameMenuData+14*4],2
  mov byte[GUIGameMenuData+14*5],2
  mov byte[GUIGameMenuData+14*6],2
  mov byte[GUICheatMenuData],2
  mov byte[GUICheatMenuData+14],2
  mov byte[GUICheatMenuData+14*2],2
  mov byte[GUIMiscMenuData+14*2],2
.noromloaded
  ret

SECTION .data
SantaData:
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0
db 1,0,0,1,0,0,1,0,0,0,1,1,1,0,1,1
db 1,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1
db 1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1
db 1,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1

SantaPos dd 272
SantaNextT dd 36*15
NEWSYM NumSnow, dd 0
NEWSYM SnowTimer, dd 36*30
MsgGiftLeft dd 0
SECTION .text

DrawSnow:
  cmp byte[OkaySC],0
  je near .nosanta
  cmp dword[MsgGiftLeft],0
  je .nodec
  mov edx,20
  mov ebx,210
  mov byte[GUItextcolor],228
  GUIOuttextwin .giftmsg
.nodec
  mov esi,[vidbuffer]
  add esi,[SantaPos]
  add esi,60*288
  mov edx,SantaData
  mov ebx,8
.sloop2
  mov ecx,16
.sloop
  cmp byte[edx],0
  je .transp
  mov byte[esi],0
.transp
  inc esi
  inc edx
  dec ecx
  jnz .sloop
  add esi,272
  dec ebx
  jnz .sloop2
.nosanta
  mov esi,[vidbuffer]
  mov ecx,200
  xor edx,edx
.loop
  xor eax,eax
  mov al,[SnowData+edx*4+3]
  mov ebx,eax
  shl eax,8
  shl ebx,5
  add eax,ebx
  xor ebx,ebx
  mov bl,[SnowData+edx*4+1]
  add eax,ebx
  add eax,16
  mov bl,[SnowVelDist+edx*2]
  and bl,03h
  add bl,228
  test byte[SnowVelDist+edx*2],8
  jz .nosnow
  mov [esi+eax],bl
.nosnow
  inc edx
  dec ecx
  jnz .loop
  ; Change Snow Displacement Values
.next
  cmp dword[SnowMover],0
  je .nomore
  call ProcessSnowVelocity
  dec dword[SnowMover]
  jmp .next
.nomore
  ret

SECTION .data
.giftmsg db 'A GIFT TO YOU IN THE OPTIONS!',0
SECTION .text

ProcessSnowVelocity:
  cmp dword[MsgGiftLeft],0
  je .nodec
  dec dword[MsgGiftLeft]
.nodec
  cmp dword[NumSnow],200
  jne .snowincr
  cmp dword[SantaNextT],0
  je .skip
  dec dword[SantaNextT]
  jmp .notsreset
.skip
  dec dword[SantaPos]
  cmp dword[SantaPos],0
  jne .notsreset
  mov dword[SantaPos],272
  mov dword[SantaNextT],36*60
  jmp .notsreset
.snowincr
  dec dword[SnowTimer]
  jnz .notsreset
  inc dword[NumSnow]
  mov dword[SnowTimer],18
.notsreset

  mov ecx,[NumSnow]
  cmp ecx,0
  jne .okay
  ret
.okay
  xor edx,edx
.loop
  xor eax,eax
  mov al,[SnowVelDist+edx*2]
  mov ebx,100
  sub bl,[MusicRelVol]
  add bx,bx
  add ax,bx
  add ax,bx
  add word[SnowData+edx*4],ax
  xor eax,eax
  mov al,[SnowVelDist+edx*2+1]
  add ax,256
  add word[SnowData+edx*4+2],ax
  cmp word[SnowData+edx*4+2],200h
  ja .nosdata
  or byte[SnowVelDist+edx*2],8
.nosdata
  inc edx
  dec ecx
  jnz .loop
  ret

SECTION .bss
OkaySC resb 1

%macro ProcessOneDigit 1
  cmp dl,9
  jbe %%notover
  add dl,65-48-10
%%notover
  add dl,48
  mov [.message+%1],dl
  xor edx,edx
  div ebx
%endmacro

SECTION .data
.message db 0,0,0,0,' ',0,0,0,0,0,0,0
SECTION .text

NEWSYM StartGUI
%ifdef __OPENGL__
  cmp byte[BilinearFilter],1
  jne near .skipbl
  mov byte[blinit],1
.skipbl
%endif
  mov byte[GUILoadPos],0
  cmp byte[TripBufAvail],0
  jne .notexttb
%ifdef __MSDOS__
  mov byte[Triplebufen],0
%endif
.notexttb
  cmp byte[MMXSupport],1
  jne .2xSaIdis
  cmp byte[newgfx16b],0
  je .2xSaIdis
  jmp .no2xSaIdis
.2xSaIdis
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
.no2xSaIdis
  cmp byte[En2xSaI],0
  je .no2xsaien
%ifdef __MSDOS__
  mov byte[Triplebufen],0
%endif
  mov byte[hqFilter],0
  mov byte[scanlines],0
  mov byte[antienab],0
.no2xsaien
  cmp byte[hqFilter],0
  je .nohqen
  mov byte[En2xSaI],0
  mov byte[scanlines],0
  mov byte[antienab],0
.nohqen

  mov ecx,64
  mov eax,SpecialLine
.slloop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz .slloop
.okayow

  mov byte[GUIOn],1
  mov byte[GUIOn2],1
  mov eax,[NumComboLocl]
  cmp byte[GUIComboGameSpec],0
  jne .local
  mov eax,[NumComboGlob]
.local
  mov [NumCombo],eax
  call ResetTripleBuf

  cmp dword[GUIwinposx+16*4],0
  jne .notzero
  mov dword[GUIwinposx+16*4],3
  mov dword[GUIwinposy+16*4],22
.notzero

  mov dword[GUICTimer],0
  ; Initialize volume
  xor eax,eax
  xor edx,edx
  mov al,[MusicRelVol]
  shl eax,7
  mov ebx,100
  div ebx
  cmp al,127
  jb .noofb
  mov al,127
.noofb
  mov [MusicVol],al
  mov byte[CheatSearchStatus],0
  cmp byte[newgfx16b],0
  je .nong
  mov ecx,256*144
  mov eax,[vidbufferofsb]
.loop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz .loop
.nong
  mov byte[ShowTimer],1
  call Get_Date
  cmp dh,12
  jne .noxmas
  cmp dl,25
  jne .noxmas
  mov byte[OkaySC],1
.noxmas
  mov byte[lastmouseholded],1
  cmp dword[GUIwinposx+15*4],0
  jne .nomoviemenufix
  mov dword[GUIwinposx+15*4],50
  mov dword[GUIwinposy+15*4],50
.nomoviemenufix
  mov ax,[resolutn]
  mov [PrevResoln],ax
  mov word[resolutn],224

  mov byte[GUIPalConv],0
  mov byte[MousePRClick],1

  pushad
  cmp byte[MouseInitOkay],1
  je near .mousedone
  mov byte[MouseInitOkay],1
  cmp byte[MouseDis],1
  je .mousedone
  call Init_Mouse
  cmp ax,0
  jne .mousedone
  mov byte[MouseDis],1
.mousedone
  popad

  mov eax,[KeyQuickLoad]
  test byte[pressed+eax],1
  jz near .noquickload
  mov byte[GUIcmenupos],0
  loadmenuopen 1
.noquickload
  mov esi,pressed
  mov ecx,64+32+8
.pclear
  mov dword[esi],0
  add esi,4
  dec ecx
  jnz .pclear
  mov byte[pressed+1],2
  mov byte[GUIescpress],1

  ; set Video cursor location
  xor eax,eax
  mov al,[cvidmode]
  mov [GUIcurrentvideocursloc],eax
  mov ebx,[NumVideoModes]
  cmp ebx,20
  ja .viewloc
  mov dword[GUIcurrentvideoviewloc],0
  jmp .skip
.viewloc
  sub ebx,20
  cmp eax,ebx
  jbe .noof
  mov eax,ebx
.noof
  mov [GUIcurrentvideoviewloc],eax
.skip

  pushad
  call SaveSramData
  call GUIQuickLoadUpdate
  popad

  call LoadDetermine

  cmp byte[AutoState],0
  je .noautostate
  cmp byte[romloadskip],0
  jne .noautostate
  pushad
  call SaveSecondState
  popad
.noautostate

  GUIInitIRQs

  cmp byte[GUIwinptr],0
  jne .nomenuopen
  cmp byte[esctomenu],0
  je .nomenuchange
  mov byte[GUIcmenupos],2
  mov byte[GUIcrowpos],0
  mov dword[GUICYLocPtr],MenuDat2
  cmp byte[esctomenu],1
  je .nomenuchange
.nomenuopen
  mov byte[GUIcmenupos],0
.nomenuchange
  cmp byte[GUIwinactiv+1],0
  je .noloadrefresh
  mov dword[GUIcurrentfilewin],0
  call GetLoadData
.noloadrefresh
  mov byte[GUIHold],0
  ; clear 256 bytes from hirestiledat
  mov esi,hirestiledat
  mov ecx,256
.loophires
  mov byte[esi],0
  inc esi
  dec ecx
  jnz .loophires
  mov byte[curblank],00h
  call InitGUI

  cmp byte[CheatWinMode],0
  je near .csskip

  ; Load Cheat Search File
  pushad
  call LoadCheatSearchFile
  popad

.csskip

  mov byte[GUIQuit],0
.nokey
  cmp byte[GUIQuit],2
  je near .exit
  cmp byte[GUIQuit],1
  je near .exitgui
  mov byte[GUIQuit],0
  cmp byte[MouseDis],1
  je .mousedis2
  call ProcessMouse
  cmp byte[videotroub],1
  jne .notrouble
  ret
.notrouble
.mousedis2
  call GUIUnBuffer
  cmp byte[GUIEffect],1
  jne .nosnow
  call DrawSnow
.nosnow
  cmp byte[GUIEffect],2
  jne .nowater
  call DrawWater
.nowater
  cmp byte[GUIEffect],3
  jne .nowater2
  call DrawWater
.nowater2
  cmp byte[GUIEffect],4
  jne .noburn
  call DrawBurn
.noburn
  cmp byte[GUIEffect],5
  jne .nosmoke
  call DrawSmoke
.nosmoke

  cmp dword[GUIEditStringcWin],0
  je .noblink
  cmp dword[GUIEditStringcLen],0
  je .noblink
  mov eax,[GUIEditStringcLen]
  cmp dword[GUIEditStringLTxt],8
  jb .noblinka
  mov byte[eax],'_'
  mov byte[eax+1],0
  mov dword[GUIEditStringLstb],1
.noblinka
  cmp dword[GUIEditStringLTxt],0
  jne .noblink
  mov dword[GUIEditStringLTxt],16
.noblink

  call DisplayBoxes

  cmp dword[GUIEditStringLstb],1
  jne .notblinked
  mov dword[GUIEditStringLstb],0
  mov eax,[GUIEditStringcLen]
  mov byte[eax],0
.notblinked

  call DisplayMenu
  cmp byte[MouseDis],1
  je .mousedis3
  call DrawMouse
.mousedis3
  cmp byte[FirstTimeData],0
  jne .nofirsttime
  call guifirsttimemsg
  mov byte[FirstTimeData],1
.nofirsttime
  cmp byte[guimsgptr],0
  jne .nohorizon
  pushad
  call GetDate
  cmp ax,1025
  popad
  jne .nohorizon
  pushad
  call GetTime
  push eax
  call horizon_get
  mov [guimsgptr],eax
  popad
  call horizonfixmsg
.nohorizon
  cmp dword[GUICTimer],0
  je .notimer
  GUIOuttext 21,211,[GUICMessage],50
  GUIOuttext 20,210,[GUICMessage],63
.notimer
  call vidpastecopyscr
  call GUIgetcurrentinput
  jmp .nokey
.exitgui
  GUIDeInitIRQs

  mov ax,[PrevResoln]
  mov [resolutn],ax
  jmp endprog
.exit
  mov edi,[spcBuffera]
  mov ecx,65536
  xor eax,eax
  rep stosd
  GUIDeInitIRQs
%ifdef __MSDOS__
  call DOSClearScreen
  cmp byte[cbitmode],0
  jne .nomakepal
  call dosmakepal
.nomakepal
%endif
  mov word[t1cc],1

  pushad
  call GUISaveVars
  popad

  mov byte[MousePRClick],1
  mov byte[prevbright],0
  mov ax,[PrevResoln]
  mov [resolutn],ax

  mov byte[CheatOn],0
  cmp dword[NumCheats],0
  je .nocheats
  mov byte[CheatOn],1
.nocheats

  cmp byte[CopyRamToggle],1
  jne .nocopyram
  mov byte[CopyRamToggle],0
  mov eax,[vidbuffer]
  add eax,129600
  ; copy 128k ram
  mov ebx,[wramdata]
  mov ecx,32768
.loopcr
  mov edx,[ebx]
  mov [eax],edx
  add ebx,4
  add eax,4
  dec ecx
  jnz .loopcr
.nocopyram

  cmp byte[CheatWinMode],2
  jne .notview
  mov byte[CheatWinMode],1
.notview

  cmp byte[CheatWinMode],0
  je .csskip2
  ;Save Cheat Search File
  pushad
  call SaveCheatSearchFile
  popad
  .csskip2

  mov edi,[vidbuffer]
  mov ecx,288*120
  xor eax,eax
  rep stosd

  mov ecx,256*144
  mov eax,[vidbufferofsb]
.loopcl
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz .loopcl

  call AdjustFrequency
  mov byte[GUIOn],0
  mov byte[GUIOn2],0
  mov byte[GUIReset],0
  mov dword[StartLL],0
  mov dword[StartLR],0
  jmp continueprog

guimencodermsg:
  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  mov byte[pressed+2Ch],0
.again
  GUIBox 43,75,213,163,160
  GUIBox 43,75,213,75,162
  GUIBox 43,75,43,163,161
  GUIBox 213,75,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 52,96,guimencodert1,220-15
  GUIOuttext 51,95,guimencodert1,220
  GUIOuttext 52,134,guimencodert2,220-15
  GUIOuttext 51,133,guimencodert2,220
  call vidpastecopyscr
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  call JoyRead
  cmp byte[pressed+39h],0
  jne .pressedokay
  jmp .again
.pressedokay
  ret

guilamemsg:
  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  mov byte[pressed+2Ch],0
.again
  GUIBox 43,75,213,163,160
  GUIBox 43,75,213,75,162
  GUIBox 43,75,43,163,161
  GUIBox 213,75,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 52,96,guilamet1,220-15
  GUIOuttext 51,95,guilamet1,220
  GUIOuttext 52,134,guilamet2,220-15
  GUIOuttext 51,133,guilamet2,220
  call vidpastecopyscr
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  call JoyRead
  cmp byte[pressed+39h],0
  jne .pressedokay
  jmp .again
.pressedokay
  ret

SECTION .data
guimencodert1 db ' MENCODER IS MISSING: ',0
guimencodert2 db 'PRESS SPACE TO PROCEED',0
guilamet1 db ' LAME IS MISSING: ',0
guilamet2 db 'PRESS SPACE TO PROCEED',0

SECTION .text

guifirsttimemsg:
  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  mov byte[pressed+2Ch],0
.again
  GUIBox 43,75,213,163,160
  GUIBox 43,75,213,75,162
  GUIBox 43,75,43,163,161
  GUIBox 213,75,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 52,81,guiftimemsg1,220-15
  GUIOuttext 51,80,guiftimemsg1,220
  GUIOuttext 52,96,guiftimemsg2,220-15
  GUIOuttext 51,95,guiftimemsg2,220
  GUIOuttext 52,104,guiftimemsg3,220-15
  GUIOuttext 51,103,guiftimemsg3,220
  GUIOuttext 52,112,guiftimemsg4,220-15
  GUIOuttext 51,111,guiftimemsg4,220
  GUIOuttext 52,120,guiftimemsg5,220-15
  GUIOuttext 51,119,guiftimemsg5,220
  GUIOuttext 52,128,guiftimemsg6,220-15
  GUIOuttext 51,127,guiftimemsg6,220
  GUIOuttext 52,136,guiftimemsg7,220-15
  GUIOuttext 51,135,guiftimemsg7,220
  GUIOuttext 52,151,guiftimemsg8,220-15
  GUIOuttext 51,150,guiftimemsg8,220
  call vidpastecopyscr
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  call JoyRead
  cmp byte[pressed+39h],0
  jne .pressedokay
  jmp .again
.pressedokay
  ret

SECTION .data
guiftimemsg1 db ' ONE-TIME USER REMINDER : ',0
guiftimemsg2 db '  PLEASE BE SURE TO READ  ',0
guiftimemsg3 db 'THE DOCUMENTATION INCLUDED',0
guiftimemsg4 db ' WITH ZSNES FOR IMPORTANT',0
guiftimemsg5 db ' INFORMATION AND ANSWERS',0
guiftimemsg6 db '    TO COMMON PROBLEMS',0
guiftimemsg7 db '      AND QUESTIONS.',0
guiftimemsg8 db 'PRESS SPACEBAR TO PROCEED.',0
SECTION .text

horizonfixmsg:
  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  mov byte[pressed+2Ch],0
.again
  GUIBox 43,75,213,163,160
  GUIBox 43,75,213,75,162
  GUIBox 43,75,43,163,161
  GUIBox 213,75,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 52,81,guimsgmsg,220-15
  GUIOuttext 51,80,guimsgmsg,220
  GUIOuttext 52,96,[guimsgptr],220-15
  GUIOuttext 51,95,[guimsgptr],220
  add dword[guimsgptr],32
  GUIOuttext 52,104,[guimsgptr],220-15
  GUIOuttext 51,103,[guimsgptr],220
  add dword[guimsgptr],32
  GUIOuttext 52,112,[guimsgptr],220-15
  GUIOuttext 51,111,[guimsgptr],220
  add dword[guimsgptr],32
  GUIOuttext 52,120,[guimsgptr],220-15
  GUIOuttext 51,119,[guimsgptr],220
  sub dword[guimsgptr],96
  GUIOuttext 52,151,guiftimemsg8,220-15
  GUIOuttext 51,150,guiftimemsg8,220
  call vidpastecopyscr
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  call JoyRead
  cmp byte[pressed+39h],0
  jne .pressedokay
  jmp .again
.pressedokay
  ret

SECTION .data
guimsgptr dd 0
guimsgmsg db '     WELCOME TO ZSNES',0
SECTION .text

guiprevideo:
  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  GUIBox 43,90,213,163,160
  GUIBox 43,90,213,90,162
  GUIBox 43,90,43,163,161
  GUIBox 213,90,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 56,96,guiprevidmsg1,220-15
  GUIOuttext 55,95,guiprevidmsg1,220
  GUIOuttext 56,104,guiprevidmsg2,220-15
  GUIOuttext 55,103,guiprevidmsg2,220
  GUIOuttext 56,112,guiprevidmsg3,220-15
  GUIOuttext 55,111,guiprevidmsg3,220
  GUIOuttext 56,120,guiprevidmsg4,220-15
  GUIOuttext 55,119,guiprevidmsg4,220
  GUIOuttext 56,128,guiprevidmsg5,220-15
  GUIOuttext 55,127,guiprevidmsg5,220
  GUIOuttext 56,136,guiprevidmsg6,220-15
  GUIOuttext 55,135,guiprevidmsg6,220
  GUIOuttext 56,151,guiprevidmsg7,220-15
  GUIOuttext 55,150,guiprevidmsg7,220
  call vidpastecopyscr
  mov byte[pressed+2Ch],0
.again
  call JoyRead
  xor ebx,ebx
  mov ecx,256+128+64
.b
  cmp byte[pressed+ebx],0
  jne .pressedkey
  inc ebx
  dec ecx
  jnz .b
  cmp byte[MouseDis],1
  je .again
  call Get_MouseData
  cmp byte[lhguimouse],1
  jne .notlefthanded
  call SwapMouseButtons
.notlefthanded
  test bx,01h
  jnz .pressedokay
  jmp .again
.pressedkey
  mov byte[pressed+ebx],0
.pressedokay
  ret

SECTION .data
guiprevidmsg1 db 'ZSNES WILL NOW ATTEMPT',0
guiprevidmsg2 db ' TO CHANGE YOUR VIDEO',0
guiprevidmsg3 db ' MODE.  IF THE CHANGE',0
guiprevidmsg4 db 'IS UNSUCCESSFUL,  WAIT',0
guiprevidmsg5 db ' 10 SECONDS AND VIDEO',0
guiprevidmsg6 db 'MODE WILL BE RESTORED.',0
guiprevidmsg7 db '    PRESS ANY KEY.',0
SECTION .text

guipostvideo:
  mov ecx,256*144
  mov eax,[vidbufferofsb]
.loop
  mov dword[eax],0FFFFFFFFh
  add eax,4
  dec ecx
  jnz .loop

  mov dword[GUIkeydelay],36*10

.pressedfail
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  GUIBox 43,90,213,163,160
  GUIBox 43,90,213,90,162
  GUIBox 43,90,43,163,161
  GUIBox 213,90,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 56,96,guipostvidmsg1,220-15
  GUIOuttext 55,95,guipostvidmsg1,220
  GUIOuttext 56,151,guipostvidmsg2,220-15
  GUIOuttext 55,150,guipostvidmsg2,220
  call vidpastecopyscr
  ; Wait for all mouse and input data to be 0

  cmp dword[GUIkeydelay],0
  je .pressedokay

  ;This is to make all ports not register space bar from being pressed earlier
  mov byte[pressed+2Ch],0

  call JoyRead

  cmp byte[pressed+39h],0
  jne .pressedokay
  jmp .pressedfail
.pressedokay
  mov byte[GUIpclicked],1
  ret

SECTION .data
guipostvidmsg1 db 'VIDEO MODE CHANGED.',0
guipostvidmsg2 db '  PRESS SPACEBAR.',0
SECTION .text

guipostvideofail:
  mov dword[guipostvidptr],guipostvidmsg3b
  mov byte[guipostvidmsg3b],0
  mov byte[guipostvidmsg4b],0
  mov byte[guipostvidmsg5b],0
  mov eax,[ErrorPointer]
  mov ebx,eax
.loop
  cmp byte[ebx],0
  je .found
  cmp byte[ebx],'$'
  je .found
  inc ebx
  jmp .loop
.found
  mov edx,ebx
  sub edx,eax
.detnext
  or edx,edx
  jz .notext
  cmp edx,25
  jbe .copytext
.nospace
  dec edx
  cmp byte[eax+edx],32
  jne .nospace
  jmp .detnext
.copytext
  push ebx
  mov ecx,[guipostvidptr]
.copytextloop
  mov bl,[eax]
  cmp bl,'$'
  jne .notdol
  mov bl,0
.notdol
  mov [ecx],bl
  inc eax
  inc ecx
  dec edx
  jnz .copytextloop
  mov byte[ecx],0
  pop ebx
  add dword[guipostvidptr],26
  cmp byte[eax],0
  je .notext
  cmp byte[eax],'$'
  je .notext
  inc eax
  jmp .found
.notext

  xor ebx,ebx
  mov ecx,256
.a
  mov byte[pressed+ebx],0
  inc ebx
  dec ecx
  jnz .a
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
  GUIBox 43,90,213,163,160
  GUIBox 43,90,213,90,162
  GUIBox 43,90,43,163,161
  GUIBox 213,90,213,163,159
  GUIBox 43,163,213,163,158
  GUIOuttext 56,96,guipostvidmsg1b,220-15
  GUIOuttext 55,95,guipostvidmsg1b,220
  GUIOuttext 56,108,guipostvidmsg2b,220-15
  GUIOuttext 55,107,guipostvidmsg2b,220
  GUIOuttext 56,119,guipostvidmsg3b,220-15
  GUIOuttext 55,118,guipostvidmsg3b,220
  GUIOuttext 56,129,guipostvidmsg4b,220-15
  GUIOuttext 55,128,guipostvidmsg4b,220
  GUIOuttext 56,139,guipostvidmsg5b,220-15
  GUIOuttext 55,138,guipostvidmsg5b,220
  GUIOuttext 56,152,guipostvidmsg8b,220-15
  GUIOuttext 55,151,guipostvidmsg8b,220
  call vidpastecopyscr
  call GUIUnBuffer
  call DisplayBoxes
  call DisplayMenu
%ifndef __UNIXSDL__
  mov dword[GUIkeydelay],0FFFFFFFFh
%else
  mov dword[GUIkeydelay],0x0
%endif
  jmp guipostvideo.pressedfail

SECTION .data
guipostvidmsg1b db 'VIDEO MODE CHANGE FAILED.',0
guipostvidmsg2b db 'UNABLE TO INIT VESA2:',0
guipostvidmsg3b db 'AAAAAAAAAAAAAAAAAAAAAAAAA',0
guipostvidmsg4b db 'AAAAAAAAAAAAAAAAAAAAAAAAA',0
guipostvidmsg5b db 'AAAAAAAAAAAAAAAAAAAAAAAAA',0
guipostvidmsg8b db 'PRESS ANY KEY',0
SECTION .bss
guipostvidptr resd 1
SECTION .text

NEWSYM guicheaterror
    xor ebx,ebx
    mov ecx,256+128+64
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a
.again
    call GUIUnBuffer
    call DisplayBoxes
    call DisplayMenu
    GUIBox 75,95,192,143,160
    GUIBox 75,95,192,95,162
    GUIBox 75,95,75,143,161
    GUIBox 192,95,192,143,159
    GUIBox 75,143,192,143,158
    GUIOuttext 81,101,guicheaterror1,220-15
    GUIOuttext 80,100,guicheaterror1,220
    GUIOuttext 81,109,guicheaterror2,220-15
    GUIOuttext 80,108,guicheaterror2,220
    GUIOuttext 81,117,guicheaterror3,220-15
    GUIOuttext 80,116,guicheaterror3,220
    GUIOuttext 81,125,guicheaterror4,220-15
    GUIOuttext 80,124,guicheaterror4,220
    GUIOuttext 81,135,guicheaterror5,220-15
    GUIOuttext 80,134,guicheaterror5,220
    call vidpastecopyscr
    call JoyRead
    xor ebx,ebx
    mov ecx,256+128+64
.b
    cmp byte[pressed+ebx],0
    jne .pressedokay
    inc ebx
    dec ecx
    jnz .b
    cmp byte[MouseDis],1
    je .mousedis
    call Get_MouseData
    cmp byte[lhguimouse],1
    jne .notlefthanded
    call SwapMouseButtons
.notlefthanded
    test bx,01h
    jnz .pressedokay
.mousedis
    jmp .again
.pressedokay
.again2
    call Check_Key
    or al,al
    jz .nokey
    call Get_Key
    jmp .again2
.nokey
    cmp byte[MouseDis],1
    je .mousedis2
    push ebx
;    mov eax,0Bh
;    int 33h
    pop ebx
.mousedis2
    mov dword[GUIcurrentcheatwin],1
    mov byte[GUIpclicked],1
    ret

SECTION .data
guicheaterror1 db 'INVALID CODE!  YOU',0
guicheaterror2 db 'MUST ENTER A VALID',0
guicheaterror3 db 'GAME GENIE,PAR, OR',0
guicheaterror4 db 'GOLD FINGER CODE.',0
guicheaterror5 db 'PRESS ANY KEY.',0
SECTION .text

SECTION .bss
ManualCPtr resd 1
ManualStatus resb 1
NEWSYM Totalbyteloaded, resd 1
NEWSYM sramsavedis, resb 1

SECTION .data

SECTION .text

CheckMenuItemHelp:
  mov al,[GUIcmenupos]
  mov [GUIpmenupos],al
  mov byte[GUIcmenupos],0
  cmp byte[GUIwinactiv+edx],1
  je .menuontop
  xor eax,eax
  mov al,[GUIwinptr]
  inc byte[GUIwinptr]
  mov [GUIwinorder+eax],dl
  mov byte[GUIwinactiv+edx],1
  cmp byte[savewinpos],0
  jne .nomenuitem
  mov eax,[GUIwinposxo+edx*4]
  mov [GUIwinposx+edx*4],eax
  mov eax,[GUIwinposyo+edx*4]
  mov [GUIwinposy+edx*4],eax
  jmp .nomenuitem
.menuontop
  xor eax,eax
  ; look for match
.notfoundyet
  mov bl,[GUIwinorder+eax]
  cmp bl,dl
  je .nextfind
  inc eax
  jmp .notfoundyet
.nextfind
  inc eax
  cmp al,[GUIwinptr]
  je .foundend
  mov cl,[GUIwinorder+eax]
  mov [GUIwinorder+eax-1],cl
  jmp .nextfind
.foundend
  mov byte[GUIpclicked],0
  mov [GUIwinorder+eax-1],bl
.nomenuitem
  ret

%macro GUICheckMenuItem 2
  mov edx,%1
  cmp byte[GUIcrowpos],%2
  jne near %%nomenuitem
  call CheckMenuItemHelp
%%nomenuitem
%endmacro

%macro checkqloadvalue 1
  cmp byte[GUIcrowpos],%1
  jne %%skip
  pushad
  push %1
  call loadquickfname
  pop eax
  popad
  ret
%%skip
%endmacro

GUITryMenuItem:                     ; Defines which menu item calls what window number
  cmp byte[GUIcmenupos],1
  jne near .noquickload
  checkqloadvalue 0
  checkqloadvalue 1
  checkqloadvalue 2
  checkqloadvalue 3
  checkqloadvalue 4
  checkqloadvalue 5
  checkqloadvalue 6
  checkqloadvalue 7
  checkqloadvalue 8
  checkqloadvalue 9
  cmp byte[GUIcrowpos],11
  jne .skipswitch
  xor byte[prevlfreeze],1
  cmp byte[prevlfreeze],0
  je .off
  mov dword[GUIPrevMenuData+347],' ON '
  jmp .on
.off
  mov dword[GUIPrevMenuData+347],' OFF'
.on
.skipswitch
  cmp byte[GUIcrowpos],12
  jne .skipclear
  cmp byte[prevlfreeze],0
  jne .skipclear
  mov edi,prevloadiname
  mov eax,20202020h
  mov ecx,70
  rep stosd
  mov edi,prevloaddnamel
  xor eax,eax
  mov ecx,1280
  rep stosd
  mov edi,prevloadfnamel
  mov ecx,1280
  rep stosd
  pushad
  call GUIQuickLoadUpdate
  popad
  ret
.skipclear
.noquickload
  cmp byte[GUIcmenupos],2
  jne near .nomain
  GUICheckMenuItem 1, 0               ; Load
  cmp byte[GUIcrowpos],0
  jne .noloadrefresh
  mov dword[GUIcurrentfilewin],0
  jmp GetLoadData
.noloadrefresh
  cmp byte[romloadskip],0
  jne near .noromloaded
  cmp byte[GUIcrowpos],1              ; Run
  jne .norun
  cmp byte[romloadskip],0
  jne .dontquit
  mov byte[GUIQuit],2
.dontquit
  ret
.norun
  GUICheckMenuItem 12, 2              ; Reset
  cmp byte[GUIcrowpos],2
  jne .noreset
  mov byte[GUICResetPos],1
.noreset
  cmp byte[GUIcrowpos],4
  jne .nosavestate
  mov byte[GUIStatesText5],0
  mov byte[GUICStatePos],1
.nosavestate
  cmp byte[GUIcrowpos],5
  jne .noloadstate
  mov byte[GUIStatesText5],1
  mov byte[GUICStatePos],1
.noloadstate
  GUICheckMenuItem 14, 4              ; Save State
  GUICheckMenuItem 14, 5              ; Load State
  GUICheckMenuItem 2, 6               ; Select State
.noromloaded
  cmp byte[GUIcrowpos],8
  jne .noquit
  mov byte[GUIQuit],1
.noquit
.nomain
  cmp byte[GUIcmenupos],3
  jne near .noconfig
  ;The number on the left is the window to open
  ;the number on the right is where in the drop down box we are
  GUICheckMenuItem 3,0               ; Input #1-5
  GUICheckMenuItem 17,2              ; Devices
  GUICheckMenuItem 18,3              ; Chip Config
  GUICheckMenuItem 4,5               ; Options
  cmp byte[GUIcrowpos],6             ; Video
  jne near .novideo
  ; set Video cursor location
  xor eax,eax
  mov al,[cvidmode]
  mov [GUIcurrentvideocursloc],eax
  mov edx,[NumVideoModes]
  cmp edx,20
  ja .viewloc
  mov dword[GUIcurrentvideoviewloc],0
  jmp .skip
.viewloc
  sub edx,20
  cmp eax,edx
  jbe .noof
  mov eax,edx
.noof
  mov [GUIcurrentvideoviewloc],eax
.skip
  mov edx,5
  call CheckMenuItemHelp
.novideo
  GUICheckMenuItem 6,7             ; Sound
  GUICheckMenuItem 19,8            ; Paths
  GUICheckMenuItem 20,9            ; Saves
  GUICheckMenuItem 21,10           ; Speed
.noconfig
  cmp byte[romloadskip],0
  jne near .nocheat
  cmp byte[GUIcmenupos],4
  jne near .nocheat
  GUICheckMenuItem 7, 0
  GUICheckMenuItem 7, 1
  GUICheckMenuItem 13, 2
  cmp byte[GUIcrowpos],0
  jne .noaddc
  mov dword[GUIcurrentcheatwin],1
.noaddc
  cmp byte[GUIcrowpos],1
  jne .nobrowsec
  mov dword[GUIcurrentcheatwin],0
.nobrowsec
.nocheat
  cmp byte[GUIcmenupos],5
  jne near .nonet
%ifdef __MSDOS__
;    GUICheckMenuItem 8, 0        ; Disable DOS Netplay Options
;    GUICheckMenuItem 8, 1
%endif
;    GUICheckMenuItem 8, 0        ; Disable WIN/SDL Internet Option
  cmp byte[GUIcrowpos],0
  jne near .nonet
.nonet
  cmp byte[GUIcmenupos],6
  jne near .nomisc
  GUICheckMenuItem 9, 0
  GUICheckMenuItem 10, 1
  cmp byte[romloadskip],0
  jne near .nomovie
  GUICheckMenuItem 15, 2
  cmp byte[GUIcrowpos],2
  jne .nomovie
  mov byte[MovieRecordWinVal],0
.nomovie
  GUICheckMenuItem 16, 3        ; Save Config
  cmp byte[GUIcrowpos],4
  jne .nosavestuff

  mov byte[savecfgforce],1
  pushad
  call GUISaveVars
  popad
  mov byte[savecfgforce],0

  call Makemode7Table
  mov dword[GUICMessage],.message1
  mov dword[GUICTimer],50
.nosavestuff
  GUICheckMenuItem 11, 6
.nomisc
  ret

SECTION .data
.message1 db 'CONFIGURATION FILES SAVED.',0
NEWSYM savecfgforce, db 0
SECTION .text

DisplayBoxes:                        ; Displays window when item is clicked
  xor esi,esi
.next2
  mov al,[GUIwinorder+esi]
  cmp al,0
  je .done
  inc esi
  jmp .next2
.done
  mov eax,esi
  dec eax
  mov [cwindrawn],al
  xor eax,eax
  xor esi,esi
.next
  mov al,[GUIwinorder+esi]
  cmp al,0
  je near .nomore
  push esi
  cmp al,1
  jne .noguiconfirm
  cmp byte[GUIReset],1
  je near .finstuff
  call DisplayGUILoad
  jmp .finstuff
.noguiconfirm
  cmp al,2
  jne .noguichosesave
  call DisplayGUIChoseSave
  jmp .finstuff
.noguichosesave
  cmp al,3
  jne .noguiinput
  call DisplayGUIInput
  jmp .finstuff
.noguiinput
  cmp al,4
  jne .noguioption
  call DisplayGUIOption
  jmp .finstuff
.noguioption
  cmp al,5
  jne .noguivideo
  call DisplayGUIVideo
  jmp .finstuff
.noguivideo
  cmp al,6
  jne .noguisound
  call DisplayGUISound
  jmp .finstuff
.noguisound
  cmp al,7
  jne .noguicheat
  call DisplayGUICheat
  jmp .finstuff
.noguicheat
  cmp al,8
  jne .noguinet
  call DisplayNetOptns
  jmp .finstuff
.noguinet
  cmp al,9
  jne .noguigameop
  call DisplayGameOptns
  jmp .finstuff
.noguigameop
  cmp al,10
  jne .noguiconf
  call DisplayGUIOptns
%ifdef __WIN32__
  pushad
  call CheckAlwaysOnTop
  popad
%endif
  jmp .finstuff
.noguiconf
  cmp al,11
  jne .noguiconf2
  call DisplayGUIAbout
  jmp .finstuff
.noguiconf2
  cmp al,12
  jne .noguireset
  call DisplayGUIReset
  jmp .finstuff
.noguireset
  cmp al,13
  jne .noguisearch
  call DisplayGUISearch
  jmp .finstuff
.noguisearch
  cmp al,14
  jne .noguistates
  call DisplayGUIStates
  jmp .finstuff
.noguistates
  cmp al,15
  jne .noguimovies
  call DisplayGUIMovies
  jmp .finstuff
.noguimovies
  cmp al,16
  jne .noguicombo
  call DisplayGUICombo
  jmp .finstuff
.noguicombo
  cmp al,17
  jne .noaddon
  call DisplayGUIAddOns
  jmp .finstuff
.noaddon
  cmp al,18
  jne .nochipconfig
  call DisplayGUIChipConfig
  jmp .finstuff
.nochipconfig
  cmp al,19
  jne .nopaths
  call DisplayGUIPaths
  jmp .finstuff
.nopaths
  cmp al,20
  jne .nosave
  call DisplayGUISave
  jmp .finstuff
.nosave
  cmp al,21
  jne .nospeed
  call DisplayGUISpeed
  jmp .finstuff
.nospeed
.finstuff
  pop esi
  inc esi
  dec byte[cwindrawn]
  jmp .next
.nomore
  ret

GUIProcStates:
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  mov byte[GUIwinactiv+14],0
  mov byte[GUIwinorder+eax],0
  dec byte[GUIwinptr]
  cmp byte[GUICBHold],10
  je .yesstate
  mov byte[GUICBHold],0
  ret
.yesstate
  mov byte[GUICBHold],0
  cmp byte[GUIStatesText5],1
  je .loadstate
  pushad
  call statesaver
  popad
  jmp .changedir
.loadstate
  pushad
  call loadstate2
  popad
.changedir
  ret

GUIProcReset:
  cmp byte[GUICBHold],2
  jne .noreset
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
.noreset
  mov byte[GUICBHold],0
  xor eax,eax
  mov al,[GUIwinptr]
  dec eax
  mov byte[GUIwinactiv+12],0
  mov byte[GUIwinorder+eax],0
  dec byte[GUIwinptr]
  ret

%macro GUIDMHelp 4
  mov byte[GUItextcolor],46
  mov byte[GUItextcolor+1],42
  mov byte[GUItextcolor+2],38
  mov byte[GUItextcolor+3],44
  mov byte[GUItextcolor+4],40
  cmp byte[GUIcmenupos],%4
  jne %%nohighlight
  mov byte[GUItextcolor],38
  mov byte[GUItextcolor+1],40
  mov byte[GUItextcolor+2],46
  mov byte[GUItextcolor+3],40
  mov byte[GUItextcolor+4],44
%%nohighlight
  GUIBox %1,3,%2,3,[GUItextcolor]
  GUIBox %1,4,%2,12,[GUItextcolor+1]
  GUIBox %1,13,%2,13,[GUItextcolor+2]
  GUIBox %1,3,%1,12,[GUItextcolor+3]
  GUIBox %2,4,%2,13,[GUItextcolor+4]
  GUIOuttext %1+5,7,%3,44
  GUIOuttext %1+4,6,%3,62
%endmacro

%macro GUIDMHelpB 4
  mov byte[GUItextcolor],46
  mov byte[GUItextcolor+1],42
  mov byte[GUItextcolor+2],38
  mov byte[GUItextcolor+3],44
  mov byte[GUItextcolor+4],40
  cmp byte[GUIcwinpress],%4
  jne %%nohighlight
  mov byte[GUItextcolor],38
  mov byte[GUItextcolor+1],40
  mov byte[GUItextcolor+2],46
  mov byte[GUItextcolor+3],40
  mov byte[GUItextcolor+4],44
%%nohighlight
  GUIBox %1,3,%2,3,[GUItextcolor]
  GUIBox %1,4,%2,13,[GUItextcolor+1]
  GUIBox %1,14,%2,14,[GUItextcolor+2]
  GUIBox %1,3,%1,13,[GUItextcolor+3]
  GUIBox %2,4,%2,14,[GUItextcolor+4]
  GUIOuttext %1+3,7,%3,44
  GUIOuttext %1+2,6,%3,62
%endmacro

%macro GUIDMHelpB2 4
  mov byte[GUItextcolor],46
  mov byte[GUItextcolor+1],42
  mov byte[GUItextcolor+2],38
  mov byte[GUItextcolor+3],44
  mov byte[GUItextcolor+4],40
  cmp byte[GUIcwinpress],%4
  jne %%nohighlight
  mov byte[GUItextcolor],38
  mov byte[GUItextcolor+1],40
  mov byte[GUItextcolor+2],46
  mov byte[GUItextcolor+3],40
  mov byte[GUItextcolor+4],44
%%nohighlight
  GUIBox %1,3,%2,3,[GUItextcolor]
  GUIBox %1,4,%2,6,[GUItextcolor+1]
  GUIBox %1,7,%2,7,[GUItextcolor+2]
  GUIBox %1,3,%1,6,[GUItextcolor+3]
  GUIBox %2,4,%2,7,[GUItextcolor+4]
  GUIOuttext %1+3,5,%3,44
  GUIOuttext %1+2,4,%3,62
%endmacro

%macro GUIDMHelpB3 4
  mov byte[GUItextcolor],46
  mov byte[GUItextcolor+1],42
  mov byte[GUItextcolor+2],38
  mov byte[GUItextcolor+3],44
  mov byte[GUItextcolor+4],40
  cmp byte[GUIcwinpress],%4
  jne %%nohighlight
  mov byte[GUItextcolor],38
  mov byte[GUItextcolor+1],40
  mov byte[GUItextcolor+2],46
  mov byte[GUItextcolor+3],40
  mov byte[GUItextcolor+4],44
%%nohighlight
  GUIBox %1,9,%2,9,[GUItextcolor]
  GUIBox %1,10,%2,12,[GUItextcolor+1]
  GUIBox %1,13,%2,13,[GUItextcolor+2]
  GUIBox %1,9,%1,12,[GUItextcolor+3]
  GUIBox %2,10,%2,13,[GUItextcolor+4]
  GUIOuttext %1+3,11,%3,44
  GUIOuttext %1+2,10,%3,62
%endmacro

%macro GUIDrawMenuM 10
  GUIShadow %7,%8,%7+4+%3*6,%8+3+%4*10
  GUIBox %1,%2,%1+4+%3*6,%2+3+%4*10,43

  mov edi,[GUIcrowpos]
  mov ecx,edi
  shl edi,8
  shl ecx,5
  add edi,ecx
  lea edi,[edi*5]
  shl edi,1
  add edi,[vidbuffer]
  add edi,%1+17+18*288
  mov ecx,6*%3+3
  mov edx,1
  mov al,73
  push edi
  call GUIDrawBox
  pop edi
  add edi,288
  mov ecx,6*%3+3
  mov edx,7
  mov al,72
  push edi
  call GUIDrawBox
  pop edi
  add edi,288*7
  mov ecx,6*%3+3
  mov edx,1
  mov al,73
  call GUIDrawBox

  GUIBox %1+%10,%2,%1+4+%3*6,%2,47
  GUIBox %1,%2,%1,%9,45
  GUIBox %1,%9,%1+4+%3*6,%9,39
  GUIBox %1+4+%3*6,1+%2,%1+4+%3*6,%9,41
  mov edi,%5
  mov esi,[vidbuffer]
  add esi,16+%6+20*288
  mov ecx,%4
  mov edx,6*%3
  call GUIMenuDisplay

  mov dword[GUIMenuL],%1+1
  mov dword[GUIMenuR],%1+6*%3+3
  mov dword[GUIMenuD],18+%4*10
%endmacro

DisplayMenu:
  ; Draw Shadow
  GUIShadow 5,7,235,21
  ; Display Top Border
  GUIBox 0,1,229,1,71
  GUIBox 0,2,229,2,70
  GUIBox 0,3,229,3,69
  GUIBox 0,4,229,4,68
  GUIBox 0,5,229,5,67
  GUIBox 0,6,229,6,66
  GUIBox 0,7,229,7,65
  GUIBox 0,8,229,8,64
  GUIBox 0,9,229,9,65
  GUIBox 0,10,229,10,66
  GUIBox 0,11,229,11,67
  GUIBox 0,12,229,12,68
  GUIBox 0,13,229,13,69
  GUIBox 0,14,229,14,70
  GUIBox 0,15,229,15,71

%ifdef __UNIXSDL__
  GUIShadow 238,9,247,20
  GUIShadow 249,9,257,20
%endif
%ifdef __WIN32__
  GUIShadow 238,9,247,14
  GUIShadow 238,16,247,20
  GUIShadow 249,9,257,20
%endif
.notwinpressa

%ifdef __UNIXSDL__
  mov byte[GUIMenuItem+36],247
  GUIDMHelpB 233,242,GUIMenuItem+36,1
  mov byte[GUIMenuItem+36],'x'
  GUIDMHelpB 244,253,GUIMenuItem+36,2
%endif

%ifdef __WIN32__
  mov byte[GUIMenuItem+36],249
  GUIDMHelpB2 233,242,GUIMenuItem+36,1
  mov byte[GUIMenuItem+36],248
  GUIDMHelpB3 233,242,GUIMenuItem+36,3
  mov byte[GUIMenuItem+36],'x'
  GUIDMHelpB 244,253,GUIMenuItem+36,2
%endif
.notwinpressb

  ; Display upper-left box
  mov byte[GUIMenuItem+36],25
  GUIDMHelp 4,12,GUIMenuItem+6,1
  GUIOuttext 4+3,7,GUIMenuItem+36,44
  GUIOuttext 4+2,6,GUIMenuItem+36,62
  ; Display boxes
  GUIDMHelp 17,47,GUIMenuItem,2
  GUIDMHelp 52,94,GUIMenuItem+7,3
  GUIDMHelp 99,135,GUIMenuItem+14,4
  GUIDMHelp 140,188,GUIMenuItem+21,5
  GUIDMHelp 193,223,GUIMenuItem+29,6

  mov dword[GUIMenuL],0
  mov dword[GUIMenuR],0
  mov dword[GUIMenuD],0

  ; format : x pos, y pos, #charx, #chary, name, xpos+2, xpos+5,22,
  ;          19+#chary*10, length of top menu box
  cmp byte[GUIcmenupos],1
  jne near .nomenu1
  GUIDrawMenuM 4,16,30,13,GUIPrevMenuData,6,9,22,149,8 ;19+13*10
  mov dword[GUICYLocPtr],MenuDat1
.nomenu1
  cmp byte[GUIcmenupos],2
  jne near .nomenu2
  GUIDrawMenuM 17,16,10,9,GUIGameMenuData,19,22,22,109,30 ;19+9*10
  mov dword[GUICYLocPtr],MenuDat2
.nomenu2
  cmp byte[GUIcmenupos],3
  jne near .nomenu3
  GUIDrawMenuM 52,16,8,11,GUIConfigMenuData,54,57,22,129,42 ;19+11*10
  mov dword[GUICYLocPtr],MenuDat3
.nomenu3
  cmp byte[GUIcmenupos],4
  jne near .nomenu4
  GUIDrawMenuM 99,16,8,3,GUICheatMenuData,101,104,22,49,36 ;19+3*10
  mov dword[GUICYLocPtr],MenuDat4
.nomenu4
  cmp byte[GUIcmenupos],5
  jne near .nomenu5
%ifdef __MSDOS__
  GUIDrawMenuM 140,16,10,2,GUINetPlayMenuData,142,145,22,39,48 ;19+2*10
%else
  GUIDrawMenuM 140,16,10,1,GUINetPlayMenuData,142,145,22,29,48 ;19+1*10
%endif
  mov dword[GUICYLocPtr],MenuDat5
.nomenu5
  cmp byte[GUIcmenupos],6
  jne near .nomenu6
  GUIDrawMenuM 193,16,9,7,GUIMiscMenuData,195,198,22,89,30 ;19+5*10
  mov dword[GUICYLocPtr],MenuDat6
.nomenu6
  ret

GUIMenuDisplay:
  xor ebx,ebx
.next
  mov al,[edi]
  push ebx
  push ecx
  push esi
  cmp al,0
  je near .notext
  cmp al,2
  je .darktext
  inc edi
  mov byte[GUItextcolor],44
  cmp byte[GUIcrowpos],bl
  je .nodrawshadow
  push edi
  push esi
  add esi,289
  call GUIOutputString
  pop esi
  pop edi
.nodrawshadow
  mov byte[GUItextcolor],63
  call GUIOutputString
  inc edi
  jmp .text
.darktext
  inc edi
  mov byte[GUItextcolor],42
  cmp byte[GUIcrowpos],bl
  je .nodrawshadow2
  push edi
  push esi
  add esi,289
  call GUIOutputString
  pop esi
  pop edi
.nodrawshadow2
  mov byte[GUItextcolor],57
  call GUIOutputString
  inc edi
  jmp .text
.notext
  add esi,4*288
  mov ecx,edx
.loop
  mov byte[esi],45
  mov byte[esi-289],40
  mov byte[esi+289],42
  inc esi
  dec ecx
  jnz .loop
  add edi,14
.text
  pop esi
  pop ecx
  pop ebx
  add esi,10*288
  inc ebx
  dec ecx
  jnz near .next
  ret

InitGUI:
  cmp byte[newengen],0
  je .nong16b
  cmp byte[cbitmode],0
  je .nong16b
  call GetScreen
.nong16b
%ifdef __MSDOS__
  call DOSClearScreen
%endif
  pushad
  call Clear2xSaIBuffer
  popad
  call GUISetPal
  call GUIBufferData
  ret

GUISetPal:
  cmp byte[cbitmode],1
  je near GUISetPal16
  ; set palette
  ; Fixed Color Scale = 0 .. 31
  mov dx,03C8h
  mov al,0
  out dx,al
  inc dx
  out dx,al
  out dx,al
  out dx,al

  inc al
  mov dx,03C8h
  mov bl,1
  out dx,al
  inc dx
.loopd
  mov al,bl
  add al,[GUIRAdd]
  out dx,al
  mov al,bl
  add al,[GUIGAdd]
  out dx,al
  mov al,bl
  add al,[GUIBAdd]
  out dx,al
  inc bl
  cmp bl,32
  jne .loopd
  ; gray scale = 32 .. 63
  mov dx,03C8h
  mov bl,32
  mov al,32
  out dx,al
  inc dx
.loopc
  mov al,bl
  add al,al
  out dx,al
  out dx,al
  out dx,al
  inc bl
  cmp bl,64
  jne .loopc
  ; shadow = 96 .. 127
  inc al
  mov al,96
  mov dx,03C8h
  mov bl,0
  out dx,al
  inc dx
.loope
  mov al,bl
  add al,[GUIRAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  out dx,al
  mov al,bl
  add al,[GUIGAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  out dx,al
  mov al,bl
  add al,[GUIBAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  out dx,al
  inc bl
  cmp bl,32
  jne .loope

  ; 0,10,31
  mov al,[GUITRAdd]
  mov [TRVal],al
  mov al,[GUITGAdd]
  mov [TGVal],al
  mov al,[GUITBAdd]
  mov [TBVal],al
  mov ax,[TRVal]
  inc ax
  shr ax,3
  mov [TRVali],ax
  shl ax,3
  add [TRVal],ax
  mov ax,[TGVal]
  inc ax
  shr ax,3
  mov [TGVali],ax
  shl ax,3
  add [TGVal],ax
  mov ax,[TBVal]
  inc ax
  shr ax,3
  mov [TBVali],ax
  shl ax,3
  add [TBVal],ax

  GUIPal 64,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 65,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 66,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 67,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 68,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 69,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 70,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 71,[TRVal],[TGVal],[TBVal]

  GUIPal 72,40,0,20
  GUIPal 73,34,0,21

  GUIPal 80,0,10,28
  GUIPal 81,0,10,27
  GUIPal 82,0,10,25
  GUIPal 83,0,09,24
  GUIPal 84,0,08,22
  GUIPal 85,0,07,20
  GUIPal 86,0,06,18
  GUIPal 87,0,05,15
  GUIPal 88,20,0,10
  GUIPal 89,17,0,10

  ; Orange Scale
  mov dx,03C8h
  mov al,128
  mov cl,20
  out dx,al
  mov bh,0
  mov ah,0
  inc dx
.loopf
  add bh,2
  inc ah
  mov al,63
  out dx,al
  mov al,bh
  out dx,al
  mov al,ah
  out dx,al
  dec cl
  jnz .loopf

  ; Blue scale = 148 .. 167
  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4

  GUIPal 152,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 151,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 150,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 149,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 148,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4
  call DecPalVal
  call DecPalVal

  GUIPal 157,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 156,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 155,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 154,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 153,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4
  call DecPalVal
  call DecPalVal
  call DecPalVal
  call DecPalVal

  GUIPal 162,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 161,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 160,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 159,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 158,[TRVal],[TGVal],[TBVal]

  GUIPal 163,40,40,00
  GUIPal 164,30,30,00
  GUIPal 165,50,00,00
  GUIPal 166,35,00,00
  GUIPal 167,00,00,00

  ; Blue scale shadow
  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2

  GUIPal 172,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 171,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 170,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 169,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 168,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2
  call DecPalVal
  call DecPalVal

  GUIPal 177,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 176,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 175,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 174,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 173,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2
  call DecPalVal
  call DecPalVal
  call DecPalVal
  call DecPalVal

  GUIPal 182,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 181,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 180,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 179,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal 178,[TRVal],[TGVal],[TBVal]

  GUIPal 183,20,20,00
  GUIPal 184,15,15,00
  GUIPal 185,25,00,00
  GUIPal 186,17,00,00
  GUIPal 187,00,00,00

  ; gray scale2 = 189 .. 220
  mov dx,03C8h
  mov al,189
  mov bl,0
  out dx,al
  inc dx
.loopi
  mov al,bl
  add al,al
  mov ah,bl
  shr ah,1
  sub al,ah
  out dx,al
  out dx,al
  add al,ah
  out dx,al
  inc bl
  cmp bl,64
  jne .loopi

  GUIPal 221,00,55,00
  GUIPal 222,00,45,00
  GUIPal 223,00,25,00

  GUIPal 224,40,0,20
  GUIPal 225,32,0,15

  GUIPal 226,20,0,10
  GUIPal 227,16,0,07

  GUIPal 228,45,45,50
  GUIPal 229,40,40,45
  GUIPal 230,35,35,40
  GUIPal 231,30,30,35

  GUIPal 232,35,15,15

  GUIPal 233,50,12,60
  GUIPal 234,30,14,60

  cmp byte[GUIPalConv],0
  je .convert
  ret
.convert
  mov byte[GUIPalConv],1

  ; Convert Image data to Gray Scale
  ; Create Palette Table
  call GUIconvpal
  ; Convert Current Image in Buffer
  mov esi,[vidbuffer]
  mov ecx,288*240
  xor eax,eax
.next
  mov al,[esi]
  mov bl,[SubPalTable+eax]
  mov [esi],bl
  inc esi
  dec ecx
  jnz .next
  ret

SECTION .bss
NEWSYM GUICPC, resw 256
SECTION .text

%macro GUIPal16b 4
  mov ax,%2
  shr ax,1
  shl ax,11
  mov bx,%3
  shl bx,5
  or ax,bx
  mov bx,%4
  shr bx,1
  or ax,bx
  mov [GUICPC+%1*2],ax
%endmacro

DecPalVal:
  mov ax,[TRVali]
  sub word[TRVal],ax
  mov ax,[TGVali]
  sub word[TGVal],ax
  mov ax,[TBVali]
  sub word[TBVal],ax
  test word[TRVal],8000h
  jz .notnegr
  mov word[TRVal],0
.notnegr
  test word[TGVal],8000h
  jz .notnegg
  mov word[TGVal],0
.notnegg
  test word[TBVal],8000h
  jz .notnegb
  mov word[TBVal],0
.notnegb
  ret

GUISetPal16:
  ; set palette
  ; Fixed Color Scale = 0 .. 31
  mov word[GUICPC],0
  inc al
  xor ebx,ebx
  mov bl,1
.loopd
  xor ecx,ecx
  mov cl,bl
  add cl,[GUIRAdd]
  shr cl,1
  shl ecx,11
  xor eax,eax
  mov al,bl
  add al,[GUIGAdd]
  shl eax,5
  or ecx,eax
  xor eax,eax
  mov al,bl
  add al,[GUIBAdd]
  shr eax,1
  or ecx,eax
  mov [GUICPC+ebx*2],cx
  inc bl
  cmp bl,32
  jne .loopd

  ; gray scale = 32 .. 63
  mov bl,32
  mov al,32
.loopc
  mov al,bl
  add al,al
  xor ecx,ecx
  mov cl,al
  shr ecx,1
  shl ecx,11
  xor edx,edx
  mov dl,al
  shl edx,5
  or ecx,edx
  xor edx,edx
  mov dl,al
  shr edx,1
  or ecx,edx
  mov [GUICPC+ebx*2],cx
  inc bl
  cmp bl,64
  jne .loopc

  ; shadow = 96 .. 127
  xor ebx,ebx
.loope
  xor ecx,ecx
  mov al,bl
  add al,[GUIRAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  shr al,1
  or cl,al
  shl ecx,6
  mov al,bl
  add al,[GUIGAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  or cl,al
  shl ecx,5
  mov al,bl
  add al,[GUIBAdd]
  mov ah,al
  add al,al
  add al,ah
  shr al,2
  shr al,1
  or cl,al
  mov [GUICPC+ebx*2+96*2],cx
  inc bl
  cmp bl,32
  jne .loope

  ; 0,10,31
  mov al,[GUITRAdd]
  mov [TRVal],al
  mov al,[GUITGAdd]
  mov [TGVal],al
  mov al,[GUITBAdd]
  mov [TBVal],al
  mov ax,[TRVal]
  inc ax
  shr ax,3
  mov [TRVali],ax
  shl ax,3
  add [TRVal],ax
  mov ax,[TGVal]
  inc ax
  shr ax,3
  mov [TGVali],ax
  shl ax,3
  add [TGVal],ax
  mov ax,[TBVal]
  inc ax
  shr ax,3
  mov [TBVali],ax
  shl ax,3
  add [TBVal],ax

  GUIPal16b 64,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 65,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 66,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 67,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 68,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 69,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 70,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 71,[TRVal],[TGVal],[TBVal]

  GUIPal16b 72,40,0,20
  GUIPal16b 73,34,0,21

  GUIPal16b 80,0,10,28
  GUIPal16b 81,0,10,27
  GUIPal16b 82,0,10,25
  GUIPal16b 83,0,09,24
  GUIPal16b 84,0,08,22
  GUIPal16b 85,0,07,20
  GUIPal16b 86,0,06,18
  GUIPal16b 87,0,05,15
  GUIPal16b 88,20,0,10
  GUIPal16b 89,17,0,10

  ; Orange Scale
  mov cl,20
  mov bh,0
  mov ah,0
  inc dx
  mov esi,128
.loopf
  add bh,2
  inc ah
  mov edx,1Fh << 6
  or dl,bh
  shl edx,5
  mov al,ah
  shr al,1
  or dl,al
  mov [GUICPC+esi*2],dx
  inc esi
  dec cl
  jnz .loopf

  ; Blue scale = 148 .. 167
  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4

  GUIPal16b 152,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 151,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 150,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 149,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 148,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4
  mov al,[TRVal]
  shr al,2
  sub [TRVal],al
  mov al,[TGVal]
  shr al,2
  sub [TGVal],al
  mov al,[TBVal]
  shr al,2
  sub [TBVal],al

  GUIPal16b 157,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 156,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 155,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 154,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 153,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  add al,al
  mov [TRVal],al
  mov al,[GUIWGAdd]
  add al,al
  mov [TGVal],al
  mov al,[GUIWBAdd]
  add al,al
  mov [TBVal],al
  mov byte[TRVali],4
  mov byte[TGVali],4
  mov byte[TBVali],4
  mov al,[TRVal]
  shr al,1
  sub [TRVal],al
  mov al,[TGVal]
  shr al,1
  sub [TGVal],al
  mov al,[TBVal]
  shr al,1
  sub [TBVal],al

  GUIPal16b 162,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 161,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 160,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 159,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 158,[TRVal],[TGVal],[TBVal]

  GUIPal16b 163,40,40,00
  GUIPal16b 164,30,30,00
  GUIPal16b 165,50,00,00
  GUIPal16b 166,35,00,00
  GUIPal16b 167,00,00,00

  ; Blue scale shadow
  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2

  GUIPal16b 172,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 171,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 170,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 169,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 168,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2
  call DecPalVal
  call DecPalVal

  GUIPal16b 177,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 176,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 175,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 174,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 173,[TRVal],[TGVal],[TBVal]

  mov al,[GUIWRAdd]
  mov [TRVal],al
  mov al,[GUIWGAdd]
  mov [TGVal],al
  mov al,[GUIWBAdd]
  mov [TBVal],al
  mov byte[TRVali],2
  mov byte[TGVali],2
  mov byte[TBVali],2
  call DecPalVal
  call DecPalVal
  call DecPalVal
  call DecPalVal

  GUIPal16b 182,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 181,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 180,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 179,[TRVal],[TGVal],[TBVal]
  call DecPalVal
  GUIPal16b 178,[TRVal],[TGVal],[TBVal]

  GUIPal16b 183,20,20,00
  GUIPal16b 184,15,15,00
  GUIPal16b 185,25,00,00
  GUIPal16b 186,17,00,00
  GUIPal16b 187,00,00,00

  ; gray scale2 = 189 .. 220
  mov bl,0
  mov esi,189
.loopi
  xor edx,edx
  mov al,bl
  add al,al
  mov ah,bl
  shr ah,1
  sub al,ah
  mov dl,al
  shr dl,1
  shl edx,6
  or dl,al
  shl edx,5
  add al,ah
  shr al,1
  or dl,al
  mov [GUICPC+esi*2],dx
  inc esi
  inc bl
  cmp bl,64
  jne .loopi

  GUIPal16b 221,00,55,00
  GUIPal16b 222,00,45,00
  GUIPal16b 223,00,25,00

  GUIPal16b 224,40,0,20
  GUIPal16b 225,32,0,15

  GUIPal16b 226,20,0,10
  GUIPal16b 227,16,0,07

  GUIPal16b 228,45,45,50
  GUIPal16b 229,40,40,45
  GUIPal16b 230,35,35,40
  GUIPal16b 231,30,30,35

  GUIPal16b 232,35,15,15

  GUIPal16b 233,50,12,60
  GUIPal16b 234,30,14,60
  GUIPal16b 235,12,60,25
  GUIPal16b 236,14,42,25
  GUIPal16b 237,60,20,25
  GUIPal16b 238,42,20,25

  cmp byte[GUIPalConv],0
  je .convert
  ret
.convert
  mov byte[GUIPalConv],1
  mov esi,[vidbuffer]
  mov edi,288*240
  xor ebx,ebx
.next
  mov ax,[esi+ebx*2]
  mov ecx,eax
  shr ecx,11
  and ecx,1Fh
  mov edx,eax
  shr edx,6
  and edx,1Fh
  add ecx,edx
  mov edx,eax
  and edx,1Fh
  add ecx,edx
  shr ecx,1
  mov al,[.multab+ecx]
  mov [esi+ebx],al
  inc ebx
  dec edi
  jnz .next
  ret

SECTION .data
.multab db 1,1,1,2,2,3,4,4,5,6,6,7,8,8,9,10,10,11,12,12,13,14,14,15,16,16,
  db 17,18,18,19,20,20,21,22,22,23,24,24,25,26,26,27,28,28,29,30,30,31
SECTION .text

GUIBufferData:
  mov ecx,16384
  cmp byte[cbitmode],1
  jne near .16b
  add ecx,16384
  cmp word[PrevResoln],224
  je .nobufb
  add esi,288*8
.nobufb
.16b
  ; copy to spritetable
  mov esi,[vidbuffer]
  cmp word[PrevResoln],224
  je .nobufa
  add esi,288*8
.nobufa
  mov edi,[spritetablea]
  add edi,8*288
.loop
  mov eax,[esi]
  mov [edi],eax
  add esi,4
  add edi,4
  dec ecx
  jnz .loop
  mov edi,sprlefttot
  mov ecx,64*5
.a
  mov dword[edi],0
  add edi,4
  dec ecx
  jnz .a
  ret

GUIUnBuffer:
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

GUIconvpal:
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
