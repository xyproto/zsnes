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

; **************************************
;  GUI.ASM
; **************************************
;
; Associated files :
;   GUIWinDp.inc - Window Display Routines
;   GUITools.inc - Misc routines for the GUI


; Quick Searches :
;   DisplayMenu           - routines to display top menu bar
;   Incomp                - Search for start of modem processing
;   ProcessModem
;   GUIgetcurrentinput
;   GUITryMenuItem        - Processes the menu item when user clicks item
;   Mouseimplementation
;   DGUIDisplayer
;   LGUILoadData
;   SetInputDevice
;   CalibrateDev1
;   ButtonProcess         - routines that processes boxed buttons
;   CheatCodeSearchInit
;   guiwincontrol
 

%include "macros.mac"

EXTSYM curblank, vidpastecopyscr, frameskip, newengen, vsyncon
EXTSYM cvidmode, antienab, smallscreenon, smallscreence,NetQuit
EXTSYM soundon, StereoSound, SoundQuality, MusicRelVol
EXTSYM endprog, continueprog, spcBuffera, spcRamcmp, cbitmode, makepal
EXTSYM t1cc, LoadDir, SRAMDir, LoadDrive,SRAMDrive, initsnes, romloadskip
EXTSYM fname, makeextension, sram, loadfileGUI, GUIloadfailed
EXTSYM SetupROM,CheckROMType, romdata, ForcePal, ramsize, ramsizeand, curromsize
EXTSYM romispal, totlines, cfgloadsdir, init65816, procexecloop
EXTSYM spcRam, spcPCRam, spcS, spcRamDP, spcA, spcX, spcY, spcP, spcNZ
EXTSYM Voice0Status, Voice1Status, Voice2Status, Voice3Status, Voice4Status
EXTSYM Voice5Status, Voice6Status, Voice7Status, romtype, SetIRQVectors
EXTSYM ClearScreen, statesaver, loadstate2, vidbuffer, ASCII2Font, hirestiledat
EXTSYM showallext, ROMTypeNOTFound, scanlines,statefileloc,pl1selk,pl2selk
EXTSYM fnamest,sprlefttot,spritetablea,fnames,SFXBATT,sfxramdata,setaramdata,SETAEnable,cgram,srama
EXTSYM tempco0,prevbright,maxbr,prevpal,coladdr,coladdg,coladdb
EXTSYM scaddtype,ScreenScale,vesa2red10,initvideo2,initvideo,pressed,UpdateDevices
EXTSYM memtabler8,memtablew8,writeon,pl1contrl,pl2contrl,JoyRead,SetInputDevice
EXTSYM SetInputDevice209,FPSOn,RevStereo,WDSPReg0C,WDSPReg1C,WDSPReg2C
EXTSYM WDSPReg3C,pl12s34,resolutn,delay,chaton,chatstrL,chatLpos,chatstrR,chatRTL
EXTSYM InitDrive,InitDir,createnewcfg,Makemode7Table,SnowOn,MovieBuffSize
EXTSYM MovieBuffFrame,vidbufferofsb,ZipSupport,wramdata,bgfixer,cfgnewgfx
EXTSYM cfgdontsave,videotroub,Open_File,Read_File,Close_File,Write_File,Create_File
EXTSYM File_Seek,File_Seek_End,Open_File_Write,Get_Date,Check_Key,Get_Key
EXTSYM Change_Drive,Change_Single_Dir,Change_Dir,Get_Dir,Get_First_Entry
EXTSYM Get_Next_Entry,Set_DTA_Address,timer2upd,curexecstate,TripBufAvail
EXTSYM nmiprevaddrl,nmiprevaddrh,nmirept,nmiprevline,nmistatus,spcnumread,spchalted
EXTSYM NextLineCache,VidStartDraw,ResetTripleBuf,GUINGVID
EXTSYM ScanCodeListing,AdjustFrequency,GUISaveVars,Init_Mouse
EXTSYM Get_MouseData,Set_MouseXMax,Set_MouseYMax,Set_MousePosition,Get_MousePositionDisplacement
EXTSYM GUIInit,GUIDeInit,SpecialLine
EXTSYM DrawWater,DrawBurn,RemoteDisconnect,loadstate3
EXTSYM SA1Enable,SA1RAMArea
EXTSYM GUIFName,GUICName
EXTSYM printnum
EXTSYM MMXCheck
EXTSYM SaveCombFile
EXTSYM NetSent,valuea
EXTSYM welcome
EXTSYM showinfogui
EXTSYM BackupCVFrame
EXTSYM Wait1SecWin,ClearUDPStuff
EXTSYM DisableSUDPPacket,EnableSUDPPacket
EXTSYM BackStateSize
EXTSYM ResetExecStuff
EXTSYM RestoreCVFrame
EXTSYM CurRecv,BackState,CBackupPos,PBackupPos,PPValue,DPValue,NetQuitter
EXTSYM LatencyV
EXTSYM LatencyRecvPtr,LatencySendPtr
EXTSYM NumofBanks
EXTSYM WinErrorA,WinErrorB,WinErrorC
EXTSYM ErrorPointer
EXTSYM MessageOn,Msgptr,MsgCount
EXTSYM PJoyAOrig,PJoyBOrig,PJoyCOrig,PJoyDOrig,PJoyEOrig
EXTSYM GetHostName
EXTSYM vramaddr,curypos,ClearRegs,vram,sndrot,regsbackup
EXTSYM GetScreen,GUITBWVID
EXTSYM Clear2xSaIBuffer
EXTSYM MouseWindow
EXTSYM GotoHomepage
EXTSYM cfgcvidmode, ExitFromGUI
EXTSYM GUIWFVID
EXTSYM cfgvsync,newgfx16b
EXTSYM cfgscanline,cfginterp
EXTSYM NumVideoModes
EXTSYM cfgvolume, MusicVol, DSPMem
EXTSYM NumInputDevices,GUIInputNames
EXTSYM GUIVideoModeNames
EXTSYM GUISLVID,GUIINVID,GUIEAVID,GUIIEVID,GUIFSVID,GUIWSVID
EXTSYM GUISSVID,GUITBVID,GUIHSVID,GUI2xVID,GUII2VID,GUIM7VID
EXTSYM cfgsoundon,cfgSoundQuality,cfgStereoSound,cfgforce8b
EXTSYM Force8b,convertnum,converthex
EXTSYM per2exec
EXTSYM hostname
EXTSYM UDPConfig
EXTSYM snesmouse
EXTSYM pl1upk,pl1downk,pl1leftk,pl1rightk,pl1Lk,pl1Rk,pl1Ak,pl1Bk
EXTSYM outofmemfix,yesoutofmemory
EXTSYM CReadHead,ReadHead,CFWriteHead,CFWriteStart
EXTSYM JoyX,JoyY,JoyMinX,JoyMinY,JoyMaxX,JoyMaxY,JoyMinX209,JoyMaxX209
EXTSYM JoyMinY209,JoyMaxY209,GetCoords,GetCoords3
EXTSYM MultiTap,SFXEnable
EXTSYM RestoreSystemVars 
EXTSYM TCPIPStartServer
EXTSYM TCPIPInitConnectToServer
EXTSYM TCPIPWaitForConnection
EXTSYM tcperr
EXTSYM TCPIPConnectToServer
EXTSYM TCPIPConnectToServerW
EXTSYM selc0040
EXTSYM TCPIPPreparePacket
EXTSYM TCPIPSendPacket,TCPIPSendPacketUDP
EXTSYM TCPIPDisconnect,TCPIPStatus
EXTSYM TCPIPStoreByte
EXTSYM TCPIPGetByte,GUIBIFIL
EXTSYM GUIHQ2X
EXTSYM GUIHQ3X
EXTSYM GUIHQ4X
EXTSYM firstsaveinc
EXTSYM nssdip1,nssdip2,nssdip3,nssdip4,nssdip5,nssdip6
;EXTSYM st010difficulty
%ifdef __LINUX__
EXTSYM numlockptr
%endif
%ifdef __WIN32__
EXTSYM initDirectDraw
EXTSYM reInitSound
%endif

%ifdef __MSDOS__
EXTSYM dssel, cantinitmodem, ModemClearBuffer, ModemGetChar
EXTSYM InitModem, DeInitModem, ModemCheckRing, ModemCheckDCD
EXTSYM DeInitModemC, ModemSendChar, UartType
EXTSYM deinitipx, IPXSearchval, ipxlookforconnect, initipx
EXTSYM PreparePacketIPX,SendPacketIPX,ipxgetchar,ipxsendchar
%endif

NEWSYM GuiAsmStart

%include "gui/guitools.inc"
%include "gui/guimisc.inc"
%include "gui/guimouse.inc"
%include "gui/guiwindp.inc"
%include "gui/guinetpl.inc"
%include "gui/guikeys.inc"
%include "gui/guicheat.inc"
%include "gui/guicombo.inc"
%include "gui/guiload.inc"



SECTION .data


; ProcessRemoteCommand
; NetLoadStuff  ; Send 14 to initiate, Send 15 to cancel (either way)
;    call PreparePacket
;    mov al,253
;    call RemoteSendChar
;    call SendPacket
; NetAddChar

NEWSYM WaterOn,  db 1

; Things to do :
;
; .checkmenuboxclick
; gray scale = 32 .. 63
; shadow = 96 .. 127
; blue scale = 148 .. 167, 168 .. 187
; gray scale = 189 .. 220 (32+137)


; |  Game        Config     Cheat     MultiPlay    Misc
;-------------------------------------------------------
;    Load        Input#1    Add Code  Modem        Game Keys
;    Run         Input#2    Browse    IPX          GUI Opns
;    Reset       Input#3    Search                 About
;    -----       Input#4
;    Save State  -------
;    Load State  Options
;    Chose State Video
;    -----       Sound
;    Quit

; Windows : 1 = Save/Load Confirmation
;           2 = Chose State
;           3 = Input Device Window
;           4 = Options
;           5 = Video
;           6 = Sound
;           7 = Cheat
;           8 = IPX/Modem
;           9 = GameOptions
;           10 = GUI Options
;           11 = About

;The first byte is the number of fields on the right not including the seperators
MenuDat1 db 12, 3,1,1,1,1,1,1,1,1,1,0,1,2,0
MenuDat2 db 8,  3,1,1,0,1,1,1,0,2,0,0
MenuDat3 db 12, 3,1,1,1,1,0,1,1,0,1,1,1,2,0
MenuDat4 db 2,  3,1,2,0
MenuDat5 db 1,  3,2,0
MenuDat6 db 6,  3,1,1,1,1,0,2,0

GUIPrevMenuData
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
.onoff  db 1,'FREEZE DATA : OFF   ',0
        db 1,'CLEAR ALL DATA      ',0
GUIGameMenuData
        db 1,'LOAD        ',0
        db 1,'RUN  [ESC]  ',0
        db 1,'RESET       ',0
        db 0,'------------',0
        db 1,'SAVE STATE  ',0
        db 1,'OPEN STATE  ',0
        db 1,'PICK STATE  ',0
        db 0,'------------',0
        db 1,'QUIT        ',0
GUIConfigMenuData
        db 1,'INPUT #1    ',0
        db 1,'INPUT #2    ',0
        db 1,'INPUT #3    ',0
        db 1,'INPUT #4    ',0
        db 1,'INPUT #5    ',0
        db 0,'------------',0
        db 1,'ADD-ONS     ',0
        db 1,'CHIP CFG    ',0
        db 0,'------------',0
        db 1,'OPTIONS     ',0
        db 1,'VIDEO       ',0
        db 1,'SOUND       ',0
        db 1,'PATHS       ',0
GUICheatMenuData
        db 1,'ADD CODE    ',0
        db 1,'BROWSE      ',0
        db 1,'SEARCH      ',0
GUINetPlayMenuData
        db 1,'MODEM       ',0
        db 1,'IPX         ',0
GUIMiscMenuData
        db 1,'GAME KEYS   ',0
        db 1,'GUI OPNS    ',0
        db 1,'MOVIE OPN   ',0
        db 1,'KEY COMB.   ',0
        db 1,'SAVE CFG    ',0
        db 0,'------------',0
        db 1,'ABOUT       ',0

; Config, Options -> New Gfx Engine, Frame Rate, Fast Forward FrameRate, etc.
; Config, Video -> Video Mode, Interpolation, etc.
; Config, Options -> Set up Save and/or Load State Confirmation
;                  Select Different game key assignments
;                  If menu should go to Load or last position when ESC
;                  is pressed from game

GUIRAdd db 15
GUIGAdd db 10
GUIBAdd db 31
mousewrap db 0          ; 0 = mouse boundries, 1 = mouse wrap
mouseshad db 1          ; 0 = no mouse shadow, 1 = mouse shadow
lastcursres db 0        ; 0 = go to load, 1 = go to previous menu, 2 = no menu
resetposn   db 1        ; 0 = no window reset, 1 = window reset
NEWSYM GUIClick, db 0   ; 1 = mouse click enters/exits gui
GUIwinposx2 dd 0,5   ,60  ,30  ,55  ,50  ,65  ,5   ,30  ,20  ,10   ,80  ,65  ,20  ,70  ,50  ,3 ,0
GUIwinposy2 dd 0,20  ,70  ,30  ,20  ,22  ,36  ,20  ,30  ,20  ,40   ,70  ,60  ,30  ,65  ,50  ,22,0

; Default keys
; Sound Channels 0 .. 7, Save/Select/Load States, Fast Forward
; Exit, Load, Reset, BG Disables, Reset, Windowing, New Gfx, OffsetMode
; State Selection 0 .. 9

NEWSYM KeyDisableSC0, dd 63
NEWSYM KeyDisableSC1, dd 64
NEWSYM KeyDisableSC2, dd 65
NEWSYM KeyDisableSC3, dd 66
NEWSYM KeyDisableSC4, dd 67
NEWSYM KeyDisableSC5, dd 68
NEWSYM KeyDisableSC6, dd 87
NEWSYM KeyDisableSC7, dd 88
NEWSYM KeySaveState,  dd 60
NEWSYM KeyStateSelct, dd 61
NEWSYM KeyLoadState,  dd 62
NEWSYM KeyFastFrwrd,  dd 41
NEWSYM KeyQuickExit,  dd 0
NEWSYM KeyQuickLoad,  dd 0
NEWSYM KeyQuickRst,   dd 0
NEWSYM KeyBGDisble0,  dd 2
NEWSYM KeyBGDisble1,  dd 3
NEWSYM KeyBGDisble2,  dd 4
NEWSYM KeyBGDisble3,  dd 5
NEWSYM KeySprDisble,  dd 6
NEWSYM KeyResetAll,   dd 7
NEWSYM KeyExtraEnab,  dd 8
NEWSYM KeyNewGfxSwt,  dd 9
NEWSYM KeyWinDisble,  dd 10
NEWSYM KeyOffsetMSw,  dd 11
NEWSYM KeyStateSlc0,  dd 0
NEWSYM KeyStateSlc1,  dd 0
NEWSYM KeyStateSlc2,  dd 0
NEWSYM KeyStateSlc3,  dd 0
NEWSYM KeyStateSlc4,  dd 0
NEWSYM KeyStateSlc5,  dd 0
NEWSYM KeyStateSlc6,  dd 0
NEWSYM KeyStateSlc7,  dd 0
NEWSYM KeyStateSlc8,  dd 0
NEWSYM KeyStateSlc9,  dd 0

GUIshowallext db 0
GUIloadfntype    db 0

NEWSYM pl3selk,   dd 0   ; 3SELECT = SHIFT
NEWSYM pl3startk, dd 0   ; 3START = ENTER
NEWSYM pl3upk,    dd 0   ; 3UP = up 
NEWSYM pl3downk,  dd 0   ; 3DOWN = down 
NEWSYM pl3leftk,  dd 0   ; 3LEFT = left 
NEWSYM pl3rightk, dd 0   ; 3RIGHT = right 
NEWSYM pl3Xk,     dd 0   ; 3X = INS
NEWSYM pl3Ak,     dd 0   ; 3A = HOME
NEWSYM pl3Lk,     dd 0   ; 3L = PAGE UP
NEWSYM pl3Yk,     dd 0   ; 3Y = DELETE
NEWSYM pl3Bk,     dd 0   ; 3B = END
NEWSYM pl3Rk,     dd 0   ; 3R = PAGE DOWN
NEWSYM pl4selk,   dd 0   ; 4SELECT = SHIFT
NEWSYM pl4startk, dd 0   ; 4START = ENTER
NEWSYM pl4upk,    dd 0   ; 4UP = up 
NEWSYM pl4downk,  dd 0   ; 4DOWN = down 
NEWSYM pl4leftk,  dd 0   ; 4LEFT = left 
NEWSYM pl4rightk, dd 0   ; 4RIGHT = right 
NEWSYM pl4Xk,     dd 0   ; 4X = INS
NEWSYM pl4Ak,     dd 0   ; 4A = HOME
NEWSYM pl4Lk,     dd 0   ; 4L = PAGE UP
NEWSYM pl4Yk,     dd 0   ; 4Y = DELETE
NEWSYM pl4Bk,     dd 0   ; 4B = END
NEWSYM pl4Rk,     dd 0   ; 4R = PAGE DOWN
NEWSYM TimeChecker,  db 0   ; Future Reserved
GUISoundBuffer db 1 ; Sound Buffer Disabled
prevloadnames times 16*10 db 32
prevloaddname times 128*10 db 0
prevloadfname times 16*10 db 32
prevlfreeze db 0
GUIsmallscreenon db 0
GUIScreenScale db 0

NEWSYM pl3contrl, db 0
NEWSYM pl4contrl, db 0
NEWSYM pl1p209b, db 0
NEWSYM pl2p209b, db 0
NEWSYM pl3p209b, db 0
NEWSYM pl4p209b, db 0
JoyPad1Move db 0
NEWSYM FirstTimeData, db 0
NEWSYM PrevSWFix, db 0
NEWSYM CalibXmin, dd 0
NEWSYM CalibYmin, dd 0
NEWSYM CalibXmax, dd 0
NEWSYM CalibYmax, dd 0
NEWSYM CalibXmin209, dd 0
NEWSYM CalibYmin209, dd 0
NEWSYM CalibXmax209, dd 0
NEWSYM CalibYmax209, dd 0
NEWSYM maxskip,      db 9
NEWSYM FPSAtStart,   db 0
NEWSYM SidewinderFix, db 0
GUIInitSt1 db 'ATZ'
.rest times 47 db 0
GUIInitSt2 db 'AT S0=0'
.rest times 43 db 0
GUIDialSt db 'ATDT ',0
NEWSYM ComNum,  db 2
NEWSYM ComIRQ,  db 3
NEWSYM BaudRate, dd 3
NEWSYM pl1Atk,    dd 0   ; Turbo A
NEWSYM pl1Btk,    dd 0   ; Turbo B
NEWSYM pl1Xtk,    dd 0   ; Turbo X
NEWSYM pl1Ytk,    dd 0   ; Turbo Y
NEWSYM pl2Atk,    dd 0   ; Turbo A
NEWSYM pl2Btk,    dd 0   ; Turbo B
NEWSYM pl2Xtk,    dd 0   ; Turbo X
NEWSYM pl2Ytk,    dd 0   ; Turbo Y
NEWSYM pl3Atk,    dd 0   ; Turbo A
NEWSYM pl3Btk,    dd 0   ; Turbo B
NEWSYM pl3Xtk,    dd 0   ; Turbo X
NEWSYM pl3Ytk,    dd 0   ; Turbo Y
NEWSYM pl4Atk,    dd 0   ; Turbo A
NEWSYM pl4Btk,    dd 0   ; Turbo B
NEWSYM pl4Xtk,    dd 0   ; Turbo X
NEWSYM pl4Ytk,    dd 0   ; Turbo Y
NEWSYM Turbo30hz, db 0   ; Turbo at 30hz instead of 60hz

NEWSYM KeyVolUp,       dd 0
NEWSYM KeyVolDown,     dd 0
NEWSYM KeyFRateUp,     dd 0
NEWSYM KeyFRateDown,   dd 0

NEWSYM KeyQuickChat,  dd 20
NEWSYM FossilUse,     db 0
NEWSYM TimerEnable,   db 0

NEWSYM Surround,      db 0
NEWSYM InterSound,    db 1
NEWSYM FastFwdToggle, db 0
NEWSYM En2xSaI, db 0
NEWSYM AutoLoadCht, db 0
NEWSYM KeyQuickSnapShot,  dd 0

CheatSrcByteSize db 0
CheatSrcByteBase db 0
CheatSrcSearchType db 0
CheatUpperByteOnly db 0
NEWSYM SRAMSave5Sec,   db 0
NEWSYM ReInitSoundC,   db 0
NEWSYM OldGfxMode2 ,   db 0
NEWSYM PitchModEn  ,   db 0
NEWSYM LatestSave  ,   db 0
NEWSYM AutoState   ,   db 0
NEWSYM OldVolume   ,   db 1
NEWSYM BlankVar    ,  db 1

NEWSYM pl1ULk,    dd 0
NEWSYM pl1URk,    dd 0
NEWSYM pl1DLk,    dd 0
NEWSYM pl1DRk,    dd 0
NEWSYM pl2ULk,    dd 0
NEWSYM pl2URk,    dd 0
NEWSYM pl2DLk,    dd 0
NEWSYM pl2DRk,    dd 0
NEWSYM pl3ULk,    dd 0
NEWSYM pl3URk,    dd 0
NEWSYM pl3DLk,    dd 0
NEWSYM pl3DRk,    dd 0
NEWSYM pl4ULk,    dd 0
NEWSYM pl4URk,    dd 0
NEWSYM pl4DLk,    dd 0
NEWSYM pl4DRk,    dd 0

NEWSYM LowPassFilterType,  db 0
NEWSYM DontSavePath,  db 0
NEWSYM ReCalib, db 1
NEWSYM GUIComboGameSpec, db 0
NEWSYM SoundNoiseDis, db 0      ; Disable Noise
NEWSYM Triplebufen, db 0
NEWSYM SoundBufEn, db 0
NEWSYM SPCDisable, db 0
NEWSYM RaisePitch, db 0

prevloadl db 0
prevloaddnamel times 512*10 db 0
prevloadfnamel times 512*10 db 0

%ifdef __WIN32__
NEWSYM PrevWinMode, db 2
NEWSYM PrevFSMode, db 6
%elifdef __LINUX__
NEWSYM PrevWinMode, db 2
NEWSYM PrevFSMode, db 3
%else
NEWSYM PrevWinMode, db 0
NEWSYM PrevFSMode, db 0
%endif

OldWinPos db 0
GUIwinposx  dd 0,5   ,60  ,30  ,55  ,50  ,65  ,5   ,30  ,20  ,10   ,80  ,65  ,20  ,70  ,50  ,3   ,50
GUIwinposxexp times 30 dd 0
GUIwinposy  dd 0,20  ,70  ,30  ,20  ,22  ,36  ,20  ,30  ,20  ,30   ,70  ,60  ,30  ,65  ,50  ,22  ,60
GUIwinposyexp times 30 dd 0

NEWSYM GUIEffect, db 0

NEWSYM pl5selk,   dd 0   ; 4SELECT = SHIFT
NEWSYM pl5startk, dd 0   ; 4START = ENTER
NEWSYM pl5upk,    dd 0   ; 4UP = up 
NEWSYM pl5downk,  dd 0   ; 4DOWN = down 
NEWSYM pl5leftk,  dd 0   ; 4LEFT = left 
NEWSYM pl5rightk, dd 0   ; 4RIGHT = right 
NEWSYM pl5Xk,     dd 0   ; 4X = INS
NEWSYM pl5Ak,     dd 0   ; 4A = HOME
NEWSYM pl5Lk,     dd 0   ; 4L = PAGE UP
NEWSYM pl5Yk,     dd 0   ; 4Y = DELETE
NEWSYM pl5Bk,     dd 0   ; 4B = END
NEWSYM pl5Rk,     dd 0   ; 4R = PAGE DOWN
NEWSYM pl5ULk,    dd 0
NEWSYM pl5URk,    dd 0
NEWSYM pl5DLk,    dd 0
NEWSYM pl5DRk,    dd 0
NEWSYM pl5Atk,    dd 0   ; Turbo A
NEWSYM pl5Btk,    dd 0   ; Turbo B
NEWSYM pl5Xtk,    dd 0   ; Turbo X
NEWSYM pl5Ytk,    dd 0   ; Turbo Y
NEWSYM pl5contrl, db 0
NEWSYM pl1p209,   db 0
NEWSYM pl2p209,   db 0
NEWSYM pl3p209,   db 0
NEWSYM pl4p209,   db 0
NEWSYM pl5p209,   db 0

NEWSYM GUIEnableTransp, db 0
NEWSYM Mode7HiRes16b, dd 0
NEWSYM NewEngEnForce, db 1
NEWSYM KeyRewind, dd 0
NEWSYM ChatNick, times 16 db 0
NEWSYM KeySlowDown, dd 0

NEWSYM UseCubicSpline, db 1

NEWSYM LargeSoundBuf, db 0
NEWSYM HighPriority, db 0
NEWSYM AlwaysOnTop, db 0
NEWSYM SaveMainWindowPos, db 1
NEWSYM MainWindowX, dw -1
NEWSYM MainWindowY, dw -1

NEWSYM ScreenShotFormat, db 0

NEWSYM pl1Ltk,    dd 0   ; Turbo L
NEWSYM pl1Rtk,    dd 0   ; Turbo R
NEWSYM pl2Ltk,    dd 0   ; Turbo L
NEWSYM pl2Rtk,    dd 0   ; Turbo R
NEWSYM pl3Ltk,    dd 0   ; Turbo L
NEWSYM pl3Rtk,    dd 0   ; Turbo R
NEWSYM pl4Ltk,    dd 0   ; Turbo L
NEWSYM pl4Rtk,    dd 0   ; Turbo R
NEWSYM pl5Ltk,    dd 0   ; Turbo L
NEWSYM pl5Rtk,    dd 0   ; Turbo R

NEWSYM GUITRAdd,  db 0
NEWSYM GUITGAdd,  db 10
NEWSYM GUITBAdd,  db 31

NEWSYM GUIWRAdd,  db 8
NEWSYM GUIWGAdd,  db 8
NEWSYM GUIWBAdd,  db 25

NEWSYM GrayscaleMode, db 0
NEWSYM MouseWheel, db 1
NEWSYM SmallMsgText, db 0
NEWSYM AllowMultipleInst, db 0
NEWSYM FilteredGUI, db 0
NEWSYM BilinearFilter, db 0
NEWSYM TripleBufferWin, db 0

NEWSYM ExclusiveSound, db 0
NEWSYM DisableScreenSaver, db 0
NEWSYM MMXSupport, db 1
NEWSYM TrapMouseCursor, db 1
NEWSYM KeyQuickClock, dd 0
NEWSYM KeyQuickSaveSPC, dd 0
NEWSYM AutoIncSaveSlot, db 0
NEWSYM TCPIPAddress, times 29 db 0
NEWSYM SoundInterpType, db 1
NEWSYM KeyDisplayFPS, dd 0
NEWSYM KeyIncStateSlot, dd 0
NEWSYM KeyDecStateSlot, dd 0
NEWSYM KeyUsePlayer1234, dd 0
NEWSYM hqFilter, db 0
NEWSYM reserved, db 0
NEWSYM scale2xFilter, db 0
NEWSYM st010difficulty,  db 0     ; place holder till we commit the other Seta 10 file
NEWSYM SRAMPath, times 1024 db 0
NEWSYM SnapPath, times 1024 db 0
NEWSYM SPCPath, times 1024 db 0
NEWSYM BSXPath, times 1024 db 0
NEWSYM STPath, times 1024 db 0
NEWSYM GNextPath, times 1024 db 0
NEWSYM SGPath, times 1024 db 0
NEWSYM FEOEZPath, times 1024 db 0
NEWSYM SJNSPath, times 1024 db 0
NEWSYM MDHPath, times 1024 db 0
NEWSYM SPL4Path, times 1024 db 0


GUIsave equ $-GUIRAdd

section .bss

NEWSYM ForceROMTiming, resb 1
NEWSYM ForceHiLoROM, resb 1

NEWSYM CombinDataGlob, resb 3300 ; 20-name, 42-combo, 2-key#, 1-P#, 1-ff
NEWSYM CombinDataLocl, resb 3300

section .data
NEWSYM CmdLineNetPlay, db 0
NEWSYM CmdLineTCPIPAddress, times 29 db 0
section .bss

GUIwinorder resb 18
GUIwinpos   resb 18
GUIwinactiv resb 18
DialNumber  resb 40
ViewBuffer  resb 50*32
NEWSYM ModemInitStat, resb 1
ModemProcess resb 1       ; Shows current dial/answer process
ModemPTimer  resd 1       ; Timer for modem process
ModemOKStat  resb 1       ; OK is detected on modem status

SECTION .data
;                LOAD STAT INPT OPT  VID  SND  CHT  NET  GMKEY GUIOP ABT  RSET SRC  STCN MOVE CMBO ADDO CHIP PATH
GUIwinposxo dd 0,5   ,60  ,30  ,55  ,50  ,35  ,5   ,30  ,10   ,10   ,50  ,65  ,20  ,70  ,50  ,3   ,50  ,50  ,5
GUIwinposyo dd 0,20  ,70  ,30  ,20  ,20  ,20  ,20  ,30  ,20   ,20   ,20  ,60  ,30  ,65  ,50  ,22  ,60  ,60  ,20
GUIwinsizex dd 0,244 ,126 ,189 ,167 ,180 ,188 ,244 ,8*16,235  ,240  ,190 ,9*16,8*16,9*16,140 ,250 ,160 ,160 ,244
GUIwinsizey dd 0,190 ,3*16,166 ,190 ,192 ,188 ,191 ,40  ,189  ,150  ,190 ,42  ,40  ,42  ,70  ,190 ,100 ,100 ,190
GUIwinptr   db 0

section .bss
GUItextcolor resb 5
GUIcmenupos  resb 1
GUIescpress  resb 1
GUIcwinpress resb 1
GUIpmenupos  resb 1
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
GUICHold     resd 1
GUICBHold    resd 1
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
NEWSYM NetPlayNoMore, resb 1
RestoreValues resb 1
NEWSYM NetChatFirst, resb 1
NEWSYM NetServer, resb 1
NEWSYM NetQuitAfter, resb 1
NEWSYM NetNewNick, resb 16
NEWSYM NetFilename, resb 512

NEWSYM CheatOn, resd 1
NEWSYM NumCheats, resd 1
NEWSYM cheatdataprev, resb 28 ; leave contents blank
NEWSYM cheatdata, resb 28*255+56 ; toggle, value, address, pvalue, name(22)

NEWSYM GUIcurrentdir, resb 131

curgsval resb 1

SECTION .data
NEWSYM numdrives, dd 26
SubPalTable times 256 db 1      ; Corresponding Gray Scale Color

SECTION .bss
WhichRemote resd 1                ; Modem = 1, IPX = 2, TCP/IP = 4
Connected   resd 1
IDCheckPos  resd 1

NEWSYM pl1neten,    resb 1
NEWSYM pl2neten,    resb 1
NEWSYM pl3neten,    resb 1
NEWSYM pl4neten,    resb 1
NEWSYM pl5neten,    resb 1
NEWSYM cnetplaybuf, resb 512
NEWSYM cnetptrhead, resd 1
NEWSYM cnetptrtail, resd 1
NEWSYM prevp1net,   resd 1
NEWSYM prevp2net,   resd 1
NEWSYM prevp3net,   resd 1
NEWSYM prevp4net,   resd 1
NEWSYM prevp5net,   resd 1
NEWSYM netdelayed,  resb 1
NEWSYM ChatProgress,resd 1
NEWSYM RecvProgress,resd 1
NEWSYM IPXInfoStr,  resw 1
NEWSYM IPXInfoStrR, resw 1
NEWSYM GUICMessage, resd 1
NEWSYM GUICTimer,   resd 1
NEWSYM GUIOn,       resb 1
NEWSYM GUIOn2,      resb 1
NEWSYM GUIReset,    resb 0
;GOSPort db 0
NEWSYM CurPalSelect, resb 1
NEWSYM MotionBlur, resb 1

NEWSYM StartLL, resd 1
NEWSYM StartLR, resd 1
NEWSYM LatencyVal, resb 32

NEWSYM NetLoadState, resb 1

NEWSYM TRVal, resw 1
NEWSYM TGVal, resw 1
NEWSYM TBVal, resw 1
NEWSYM TRVali, resw 1
NEWSYM TGVali, resw 1
NEWSYM TBVali, resw 1
NEWSYM TRVal2, resw 1
NEWSYM TGVal2, resw 1
NEWSYM TBVal2, resw 1

SECTION .text

%macro stim 0
%ifdef __MSDOS__
    sti
%endif
%endmacro

%macro clim 0
%ifdef __MSDOS__
    cli
%endif
%endmacro

clearsram:
    push eax
    push ecx
    mov eax,srama
    mov ecx,65536
.loop
    mov byte[eax],0FFh
    inc eax
    dec ecx
    jnz .loop
    mov eax,[sfxramdata]
    mov ecx,65536
.loop2
    mov byte[eax],0FFh
    inc eax
    dec ecx
    jnz .loop2

    cmp byte[SETAEnable],0
    je .nosetasram
    mov eax,[setaramdata]
    mov ecx,4096
.loop2seta
    mov byte[eax],0FFh
    inc eax
    dec ecx
    jnz .loop2seta
.nosetasram

    cmp byte[SA1Enable],1
    jne .nosa1
    mov eax,[SA1RAMArea]
    mov ecx,65536*2
.loop3
    mov byte[eax],0FFh
    inc eax
    dec ecx
    jnz .loop3
.nosa1
    pop ecx
    pop eax
    ret

GUIQuickLoadUpdate:
    cmp byte[prevlfreeze],0
    je .off
    mov byte[GUIPrevMenuData.onoff+15],'O'
    mov byte[GUIPrevMenuData.onoff+16],'N'
    mov byte[GUIPrevMenuData.onoff+17],' '
    jmp .on
.off
    mov byte[GUIPrevMenuData.onoff+15],'O'
    mov byte[GUIPrevMenuData.onoff+16],'F'
    mov byte[GUIPrevMenuData.onoff+17],'F'
.on
    mov esi,prevloadfnamel
%ifdef __MSDOS__
    mov esi,prevloadnames
%endif
    mov edi,GUIPrevMenuData+3
    mov edx,10
.mainloop
    mov ecx,25
%ifdef __MSDOS__
    mov ecx,16
%endif
    push edi
    push esi
    cmp byte[esi],32
    je near .fin2
.loop
    mov al,[esi]
    cmp al,0
    je .zero
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jnz .loop
    cmp byte[esi],0
    je .zero
    mov byte[edi],'.'
    mov byte[edi+1],'.'
    mov byte[edi+2],'.'
    jmp .fin
.zero
    add ecx,3
.loop2
    mov byte[edi],32
    inc edi
    dec ecx
    jnz .loop2
    jmp .fin
.fin2
    mov ecx,18
.loop3
    mov byte[edi],32
    inc edi
    dec ecx
    jnz .loop3
.fin
    pop esi
    pop edi
    add esi,512 ;16
%ifdef __MSDOS__
    sub esi,512-16
%endif
    add edi,32
    dec edx
    jnz near .mainloop
    ret



CalcChecksum:
    mov eax,GUIRAdd
    mov ecx,100
    xor edx,edx
    xor ebx,ebx
.loop
    mov bl,[eax]
    add edx,ebx
    inc eax
    dec ecx
    jnz .loop
    mov ebx,edx
    xor bx,1011001011101101b
    xor eax,eax
    test bh,08h
    jz .nb
    mov al,1
.nb
    and bh,0F7h
    test bl,10h
    jz .nb2
    or bh,08h
.nb2
    and bl,0EFh
    test al,1
    jz .nb3
    or bl,10h
.nb3
    xor bl,bh
    or bl,80h
    ret

NEWSYM GUIRestoreVars
    mov edx,GUIFName
    call Open_File
    jc .fail
    mov bx,ax
    mov edx,GUIRAdd
    mov ecx,GUIsave
    call Read_File
    call Close_File
.fail
    mov al,[GUIsmallscreenon]
    mov [smallscreenon],al
    mov al,[GUIScreenScale]
    mov [ScreenScale],al
    cmp byte[CmdLineNetPlay],0
    je .nocmdlinenetplay
    mov ecx,28/4
    mov esi,CmdLineTCPIPAddress
    mov edi,TCPIPAddress
.netplayloop
    mov eax,[esi]
    add esi,byte 4
    mov [edi],eax
    add edi,byte 4
    dec ecx
    jnz .netplayloop
    xor eax,eax
.nocmdlinenetplay
    call CalcChecksum
    cmp byte[TimeChecker],bl
    jne .nottimer
    mov byte[ShowTimer],1
    mov dword[NumSnow],200
    mov dword[SnowTimer],0
.nottimer
    cmp byte[ReCalib],0
    je .nocal
    mov byte[ReCalib],0
    mov dword[CalibXmin],0
    mov dword[CalibXmax],0
    mov dword[CalibYmin],0
    mov dword[CalibYmax],0
    mov dword[CalibXmin209],0
    mov dword[CalibXmax209],0
    mov dword[CalibYmin209],0
    mov dword[CalibYmax209],0
.nocal

    mov dword[NumComboGlob],0
    mov edx,GUICName
    call Open_File
    jc .failb
    mov bx,ax
    mov edx,ComboBlHeader
    mov ecx,23
    call Read_File
    mov al,byte[ComboBlHeader+22]
    or al,al
    jz .done
    mov [NumComboGlob],al
    mov ecx,[NumComboGlob]
    mov edx,ecx
    shl ecx,6
    add ecx,edx
    add ecx,edx
    mov edx,CombinDataGlob
    call Read_File
.done
    call Close_File
.failb
    ret

SECTION .data
NEWSYM ComboHeader, db 'Key Combination File',26,1,0
NEWSYM ComboBlHeader, times 23 db 0
SECTION .text

NEWSYM ExecGUISaveVars
    cmp byte[ShowTimer],1
    jne .nottimer
    call CalcChecksum
    mov byte[TimeChecker],bl
.nottimer
    cmp byte[cfgdontsave],1
    je .failed
    mov edx,GUIFName
    call Create_File
    jc .failed
    mov bx,ax
    mov edx,GUIRAdd
    mov ecx,GUIsave
    call Write_File
    call Close_File
.failed
    mov al,[NumComboGlob]
    or al,al
    jz .failb
    mov [ComboHeader+22],al
    mov edx,GUICName
    call Create_File
    jc .failb
    mov bx,ax
    mov edx,ComboHeader
    mov ecx,23
    call Write_File
    mov ecx,[NumComboGlob]
    mov edx,ecx
    shl ecx,6
    add ecx,edx
    add ecx,edx
    mov edx,CombinDataGlob
    call Write_File
    call Close_File
.failb
    ret

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

SECTION .bss
NEWSYM GUIoldhand9o, resd 1
NEWSYM GUIoldhand9s, resw 1
NEWSYM GUIoldhand8o, resd 1
NEWSYM GUIoldhand8s, resw 1
GUIt1cc resd 1
GUIt1ccSwap resb 1
GUIskipnextkey42 resb 1

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
    cmp dword[ModemPTimer],0
    je .nodec5
    dec dword[ModemPTimer]
.nodec5
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
    inc byte[GUINetTextm2+2]
    and byte[GUINetTextm2+2],0Fh
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
    cmp byte[resetposn],1
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
    mov bl,byte[GUIwinorder+eax]
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
    cmp byte[CheatBDoor],1
    je .nomodem
    cmp byte[CNetType],21
    je .modem
    cmp byte[CNetType],22
    je .modem
    cmp byte[CNetType],20
    jne .nomodem
.modem
    mov byte[GUICheatMenuData],2
    mov byte[GUICheatMenuData+14],2
    mov byte[GUICheatMenuData+14*2],2
.nomodem
    cmp byte[romloadskip],0
    je .noromloaded2
    mov byte[GUIGameMenuData+14],2
    mov byte[GUIGameMenuData+14*2],2
    mov byte[GUIGameMenuData+14*4],2
    mov byte[GUIGameMenuData+14*5],2
    mov byte[GUIGameMenuData+14*6],2
    mov byte[GUICheatMenuData],2
    mov byte[GUICheatMenuData+14],2
    mov byte[GUICheatMenuData+14*2],2
    mov byte[GUIMiscMenuData+14*2],2
.noromloaded2
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
NumSnow dd 0
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
    mov byte[esi+eax],bl
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
SECTION .data
cstempfname db 'tmpchtsr.___',0
SECTION .text


NEWSYM SaveSramData
    ; save sram
    cmp byte[sramsavedis],1
    je .nosram
    cmp dword[ramsize],0
    je .nosram
    clim
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    xor esi,esi
    xor edi,edi
    mov edx,fnames+1
    call Create_File
    jc .failed
    mov bx,ax
    mov ecx,[ramsize]
    mov edx,[sram]
    call Write_File
    call Close_File
.failed
    stim
.nosram
    call SaveCombFile
    ret

NEWSYM ProcRewind
    mov eax,KeyRewind
    add eax,4
    mov ebx,8
.loop
    cmp byte[eax],'a'
    jb .b
    cmp byte[eax],'z'
    ja .b
    sub byte[eax],'a'-'A'
.b
    inc eax
    dec ebx
    jnz .loop
    sub eax,8
    ; 90,83,75,78,73,71
    mov dword[.temp],44*65536*256+24*65536+72*256+40
    mov word[.temp+4],41*256+50
    add dword[.temp],34*65536*256+51*65536+11*256+50
    add word[.temp+4],30*256+23
    call .c
    ; 95,68,69,77,79,95
    mov dword[.temp],25*65536*256+29*65536+31*256+62
    mov word[.temp+4],43*256+18
    add dword[.temp],52*65536*256+40*65536+37*256+33
    add word[.temp+4],52*256+61
    call .c
    ; 80,72,65,82,79,83
    mov dword[.temp],11*65536*256+33*65536+24*256+35
    mov word[.temp+4],52*256+30
    add dword[.temp],71*65536*256+32*65536+48*256+45
    add word[.temp+4],31*256+49
    call .c
    ret
.c
    mov ebx,[.temp]
    cmp [eax],ebx
    jne .noteq
    mov bx,[.temp+4]
    cmp [eax+4],bx
    jne .noteq
    mov dword[eax],0
    mov dword[eax+4],0
    mov dword[eax+8],0
.noteq
    ret
section .bss
.temp resd 2
section .text

%macro ProcessOneDigit 1
    cmp dl,9
    jbe %%notover
    add dl,65-48-10
%%notover
    add dl,48
    mov byte[.message+%1],dl
    xor edx,edx
    div ebx
%endmacro

NEWSYM TestSent
    mov eax,[NetSent]
    xor edx,edx
    mov ebx,16
    div ebx
    ProcessOneDigit 3
    ProcessOneDigit 2
    ProcessOneDigit 1
    ProcessOneDigit 0

    mov eax,[valuea]
    xor edx,edx
    mov ebx,16
    div ebx
    ProcessOneDigit 8
    ProcessOneDigit 7
    ProcessOneDigit 6
    ProcessOneDigit 5

    mov dword[GUICMessage],.message
    mov dword[GUICTimer],100000
    ret
SECTION .data
.message db 0,0,0,0,' ',0,0,0,0,0,0,0
SECTION .text

NEWSYM StartGUI
    cmp byte[TripBufAvail],0
    jne .notexttb
    mov byte[Triplebufen],0
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
    je .no2xsaidis
    mov byte[Triplebufen],0
.no2xsaidis
    cmp byte[En2xSaI],0
    je .no2xsaien
    mov byte[hqFilter],0
    mov byte[scanlines],0
    mov byte[antienab],0
.no2xsaien
    cmp byte[hqFilter],0
    je .nohq
    mov byte[En2xSaI],0
    mov byte[scanlines],0
    mov byte[antienab],0
.nohq
    mov ecx,64
    mov eax,SpecialLine
.slloop
    mov dword[eax],0
    add eax,4
    dec ecx
    jnz .slloop
    cmp byte[OldWinPos],0
    jne .okayow
    xor esi,esi
    mov ecx,17
.nextow
    mov eax,[GUIwinposx2+esi*4]
    mov [GUIwinposx+esi*4],eax
    mov eax,[GUIwinposy2+esi*4]
    mov [GUIwinposy+esi*4],eax
    inc esi
    dec ecx
    jnz .nextow
    mov byte[OldWinPos],1
    mov eax,[pl1p209b]
    mov [pl1p209],eax
.okayow
%ifndef __MSDOS__
    mov dword[GUINetPlayMenuData+1],'INTE'
    mov dword[GUINetPlayMenuData+5],'RNET'
    mov dword[GUINetPlayMenuData+1+14],'----'
    mov dword[GUINetPlayMenuData+5+14],'----'
    mov byte[MenuDat5],0
    mov byte[MenuDat5+1],2
%endif
;.notwinport
    ; copy old quickfilename to new quickfilename
    cmp byte[prevloadl],0
    jne .noconvertlfqm
    mov byte[prevloadl],1
    mov ecx,10
    xor edx,edx
.convlfnlp
    mov eax,ecx
    dec eax
    mov edx,eax
    shl eax,9
    shl edx,4
    mov bl,16
.convlfnlp2
    mov bh,[prevloadfname+edx]
    mov [prevloadfnamel+eax],bh
    inc edx
    inc eax
    dec bl
    jnz .convlfnlp2
    mov eax,ecx
    dec eax
    mov edx,eax
    shl eax,9
    shl edx,7
    mov bl,128
.convlfnlp2b
    mov bh,[prevloaddname+edx]
    mov [prevloaddnamel+eax],bh
    inc edx
    inc eax
    dec bl
    jnz .convlfnlp2b
    dec ecx
    jnz .convlfnlp
.noconvertlfqm

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
    xor ecx,ecx
.joysloop
    cmp dword[pl1ULk+ecx*4],80h
    jbe .nojoystick
    mov dword[pl1ULk+ecx*4],0
.nojoystick
    inc ecx
    cmp ecx,16
    jne .joysloop

    mov dword[GUICTimer],0
    cmp byte[OldVolume],1
    jne .notold
    mov byte[OldVolume],0
    mov byte[MusicRelVol],100
    mov byte[cfgvolume],100
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
.notold
    mov byte[CheatSearchStatus],0
    cmp byte[newgfx16b],0
    je .nong
    mov ecx,255*144
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
    sub ebx,5
    cmp eax,ebx
    jbe .noof
    mov eax,ebx
.noof
    mov [GUIcurrentvideoviewloc],eax

    ; change to sram dir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir

    call SaveSramData

    clim
    mov edx,fnames+1
    call Create_File
    jc .nosfxramwrite
    mov bx,ax
    mov ecx,65536
    mov edx,[sfxramdata]
    call Write_File
    call Close_File
.nosfxramwrite
    stim

    cmp byte[SETAEnable],0
    je .nosetasram
    clim
    mov edx,fnames+1
    call Create_File
    jc .nosetaramwrite
    mov bx,ax
    mov ecx,4096
    mov edx,[setaramdata]
    call Write_File
    call Close_File
.nosetaramwrite
    stim
.nosetasram

    call GUIQuickLoadUpdate
    call LoadDetermine
    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir

    cmp byte[NetFilename],0
    je near .nofilenamenet
    cmp byte[NetChatFirst],0
    je near .filenamenetb
    mov ebx,NetFilename
    xor ecx,ecx
.fnetloop
    cmp byte[ebx],'\'
    jne .fnetloopb
    mov ecx,ebx
.fnetloopb
    inc ebx
    cmp byte[ebx],0
    jne .fnetloop
    or ecx,ecx
    jz near .nofilenamenet
    mov byte[ecx],0
    push ecx
    mov dl,[LoadDrive]
    cmp byte[NetFilename+1],':'
    jne .nodrivenetb
    mov dl,[NetFilename]
    sub dl,'A'
.nodrivenetb
    mov ebx,NetFilename
    call Change_Dir
    pop ecx
    mov ebx,NetFilename
    inc ecx
.nextnetl
    mov al,[ecx]
    mov [ebx],al
    inc ecx
    inc ebx
    or al,al
    jnz .nextnetl
    jmp .nofilenamenet
.filenamenetb
    mov dl,[LoadDrive]
    cmp byte[NetFilename+1],':'
    jne .nodrivenet
    mov dl,[NetFilename]
    sub dl,'A'
.nodrivenet
    mov ebx,NetFilename
    call Change_Dir
    mov byte[NetFilename],0
.nofilenamenet
    cmp byte[NetServer],0
    je .noserverclient
    mov byte[CNetType],15
    mov byte[ModemProcess],40
    cmp byte[NetServer],2
    jne .noclient
    mov byte[ModemProcess],41
.noclient
    mov byte[NetServer],0
    mov byte[WhichRemote],4
    mov byte[GUIcmenupos],0
    mov byte[GUIcrowpos],0
    call loadnetopen
.noserverclient
    cmp byte[NetNewNick],0
    je .nonewnick
    mov ebx,NetNewNick
    mov ecx,ChatNick
.nickloop
    mov al,[ebx]
    mov [ecx],al
    inc ebx
    inc ecx
    or al,al
    jnz .nickloop
    mov byte[NetNewNick],0
.nonewnick

;NEWSYM NetChatFirst, db 0
;NEWSYM NetServer, db 0
;NEWSYM NetQuitAfter, db 0
;NEWSYM NetNewNick, times 16 db 0
;NEWSYM NetFilename, times 512 db 0

    cmp byte[CNetType],20
    je near .noautostate
    cmp byte[AutoState],0
    je .noautostate
    cmp byte[romloadskip],0
    jne .noautostate
    call SaveSecondState
.noautostate

    GUIInitIRQs

    cmp byte[CNetType],20
    jne near .nostat20
    test byte[NetQuit],80h
    jnz near .nostat20
    mov byte[GUIcmenupos],0
    mov byte[GUIcrowpos],0
    call loadnetopen
%ifdef __MSDOS__
    cmp byte[WhichRemote],1
    jne .yesdcd
    call ModemCheckDCD
    cmp al,1
    jne near .nostat20
.yesdcd
%endif

    mov byte[RestoreValues],1
    pushad
    mov dword[CBackupPos],0
    call BackupCVFrame
    popad

    call DisableSUDPPacket
    call Wait1SecWin
    ; sync
    call PreparePacket
    mov al,254
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,254
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,254
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,254
    call RemoteSendChar
    call SendPacket
    mov dword[ModemPTimer],4*32
.nochar
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dword[ModemPTimer],0
    je near .nostat20
    cmp dh,0
    je .nochar
    cmp dl,254
    jne .nochar
    call PreparePacket
    mov al,253
    call RemoteSendChar
    call SendPacket
.nocharc
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dword[ModemPTimer],0
    je near .nostat20
    cmp dh,0
    je .nocharc
    cmp dl,253
    jne .nocharc
    call PreparePacket
    mov al,1
    call RemoteSendChar
    call SendPacket
    call ClearUDPStuff

    mov byte[RemoteCommand],1
    mov byte[HoldCommand],1
    cmp byte[NetLoadState],1
    jne .notreceive
    mov byte[CNetType],22
.noreceivestate
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dh,0
    je .noreceivestate
    cmp dl,14
    jne .noreceivestate
    call loadstaterecvinit
.notreceive
    cmp byte[NetLoadState],2
    jne .notsend
    call NetLoadStuff
.notsend
.nostat20

    cmp byte[GUIwinptr],0
    jne .nomenuopen
    cmp byte[lastcursres],1
    je .nomenuchange
    mov byte[GUIcmenupos],2
    mov byte[GUIcrowpos],0
    mov dword[GUICYLocPtr],MenuDat2
    cmp byte[lastcursres],0
    je .nomenuchange
.nomenuopen
    mov byte[GUIcmenupos],0
.nomenuchange
    cmp byte[GUIwinactiv+1],0
    je .noloadrefresh
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
    ; change to sram dir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir

    ; Load Cheat Search File
    mov edx,cstempfname
    call Open_File
    jc .csskipb
    mov bx,ax
    mov edx,[vidbuffer]
    add edx,129600
    mov ecx,65536*2+32768
    call Read_File
    call Close_File

.csskipb
    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
.csskip

    mov byte[GUIQuit],0
.nokey
    cmp byte[CNetType],21
    je .noquit
    cmp byte[CNetType],22
    jne .yesquit
.noquit
    mov byte[GUIQuit],0
.yesquit
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
    cmp byte[CNetType],20
    je .nowater
    cmp byte[CNetType],21
    je .nowater
    cmp byte[CNetType],22
    je .nowater
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
    jne .nosmoke
;    call DrawSmoke
    call DrawBurn
.nosmoke
;    call TestSent
    cmp byte[CNetType],20
    jne .noreceive

    cmp byte[NetChatFirst],0
    je .noloadbeforechat
    mov eax,NetFilename
    call GUIloadfilename.nocnettype
    mov byte[sramsavedis],1
    call transfersram
    mov byte[NetChatFirst],0
.noloadbeforechat

    cmp byte[GUIcmenupos],0
    jne .nomenuout2
    cmp byte[GUIwinptr],0
    jne .nomenuout2
    cmp byte[netlastloaded],1
    je .openmenu
    mov byte[GUIcmenupos],2
    mov byte[GUIcrowpos],0
    jmp .nomenuout2
.openmenu
    mov byte[netlastloaded],0
    call loadnetopen
.nomenuout2
    call RemoteGetChar
    cmp dh,0
    jne .received
    mov dl,1
.received
    call ProcessRemoteCommand
    jmp .noreceive2
.noreceive
    mov byte[HoldCommand],0
.noreceive2

    cmp byte[CNetType],21
    jne .noloadstatesend
    call loadstatesend
.noloadstatesend
    cmp byte[CNetType],22
    jne .noloadstaterecv
    call loadstaterecv
.noloadstaterecv

    cmp byte[CNetType],15
    je .modem
    cmp byte[CNetType],12
    je .modem
    cmp byte[CNetType],11
    je .modem
    cmp byte[CNetType],10
    jne near .nomodem
.modem
    call ProcessModem
%ifdef __MSDOS__
    cmp byte[Connected],1
    je near .nomodem
    call ModemGetChar
    cmp dh,0
    je .nomodem
    cmp byte[ModemOKStat],0
    jne .foundokay
    mov byte[ModemOKStat],1
    jmp .skipstat
.foundokay
    cmp byte[ModemOKStat],1
    jne .nostat0
    cmp dl,13
    jne .nostat0
    inc byte[ModemOKStat]
    jmp .skipstat
.nostat0
    cmp byte[ModemOKStat],2
    jne .nostat1
    cmp dl,'O'
    jne .nostat1
    inc byte[ModemOKStat]
    jmp .skipstat
.nostat1
    cmp byte[ModemOKStat],3
    jne .nostat2
    cmp dl,'K'
    jne .nostat2
    inc byte[ModemOKStat]
    jmp .skipstat
.nostat2
.skipstat
    mov dh,0
    call NetAddChar
%endif
.nomodem

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
    cmp dword[GUICTimer],0
    je .notimer
    GUIOuttext 21,211,[GUICMessage],50
    GUIOuttext 20,210,[GUICMessage],63
.notimer
    call vidpastecopyscr
    call GUIgetcurrentinput
    jmp .nokey

.exitgui
    cmp byte[CNetType],20
    jne near .nostat20b2
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
    call PreparePacket
    mov al,255
    call RemoteSendChar
    call SendPacket
.nostat20b2

    GUIDeInitIRQs

    mov ax,[PrevResoln]
    mov [resolutn],ax
    jmp endprog
.exit
    mov edi,[spcBuffera]
    mov ecx,65536
    xor eax,eax
    rep stosd
    mov edi,spcRamcmp
    mov ecx,65536/4
    xor eax,eax
    rep stosd
    GUIDeInitIRQs
    call ClearScreen
    cmp byte[cbitmode],0
    jne .nomakepal
    call makepal
.nomakepal
    mov word[t1cc],1

    mov byte[chaton],0
    mov dword[chatstrL],0
    mov dword[chatLpos],0
    mov dword[chatstrR],0
    mov dword[chatRTL],0

    cmp byte[CNetType],20
    jne near .nostat20b

    mov al,10
    sub al,[Latency]
    cmp byte[Latency],4
    jb .nolatency
    mov al,7
.nolatency
    mov [BackStateSize],al

    call ResetExecStuff

    mov byte[MultiTap],1
    cmp byte[pl3neten],0
    jne .mtap
    cmp byte[pl4neten],0
    jne .mtap
    cmp byte[pl5neten],0
    jne .mtap
.nomtap
    mov byte[MultiTap],0
.mtap

    cmp byte[RestoreValues],1
    jne .norestoreval
    pushad
    mov dword[PBackupPos],0
    call RestoreCVFrame
    popad
.norestoreval

    mov dword[nmiprevaddrl],0
    mov dword[nmiprevaddrh],0
    mov dword[nmirept],0
    mov dword[nmiprevline],224
    mov dword[nmistatus],0
    mov dword[spcnumread],0
 mov dword[spchalted],-1
    mov byte[NextLineCache],0
    mov byte[DSPMem+08h],0
    mov byte[DSPMem+18h],0
    mov byte[DSPMem+28h],0
    mov byte[DSPMem+38h],0
    mov byte[DSPMem+48h],0
    mov byte[DSPMem+58h],0
    mov byte[DSPMem+68h],0
    mov byte[DSPMem+78h],0

    mov byte[netdelayed],0
    mov dword[cnetptrhead],0
    mov dword[cnetptrtail],0
    mov dword[prevp1net],0
    mov dword[prevp2net],0
    mov dword[prevp3net],0
    mov dword[prevp4net],0
    mov dword[prevp5net],0
    mov byte[BackState],1
    mov dword[CBackupPos],0
    mov dword[PBackupPos],0
    mov dword[PPValue],0
    mov dword[DPValue],0
    mov byte[CurRecv],0
    mov dword[NetQuitter],0
    mov dword[LatencyV],0
    mov dword[LatencyV+4],0
    mov dword[LatencyV+8],0
    mov dword[LatencyV+12],0
    mov dword[LatencyRecvPtr],0
    mov dword[LatencySendPtr],0

    mov eax,cnetplaybuf
    mov ecx,512
.loop20
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .loop20
    mov al,[Latency]
    mov [LatencyLeft],al
    mov byte[NetSwap],0

    mov dword[CBackupPos],0
    mov dword[PBackupPos],0

    mov ebx,[romdata]
    mov ecx,[NumofBanks]
    shl ecx,15
    xor eax,eax
    or ecx,ecx
    jz .nocsumloop
.csumloop
    add al,[ebx]
    adc ah,0
    inc ebx
    dec ecx
    jnz .csumloop
.nocsumloop
    mov [CheckSumVal],eax

    mov ebx,eax
    ; sync with modem
    call PreparePacket
    mov al,30
    call RemoteSendChar
    mov al,230
    call RemoteSendChar
    mov al,[CheckSumVal]
    call RemoteSendChar
    mov al,[CheckSumVal+1]
    call RemoteSendChar
    call SendPacket
.nocharb
    pushad
    call JoyRead
    popad
;    cmp byte[pressed+1],1
;    je near .faileda

    call RemoteGetChar
    cmp dh,0
    je .nocharb
    cmp dl,230
    jne .nocharb
.nocharb2
    call RemoteGetChar
    cmp dh,0
    je .nocharb2
    cmp dl,[CheckSumVal]
    jne .wrongcs
.nocharb3
    call RemoteGetChar
    cmp dh,0
    je .nocharb3
    cmp dl,[CheckSumVal+1]
    je .okaychat
.wrongcs
    mov esi,WrongCheckSum
    call WritetochatBuffer
    jmp StartGUI
.okaychat
    call PreparePacket
    mov al,229
    call RemoteSendChar
    call SendPacket
.nocharb5
    pushad
    call JoyRead
    popad
;    cmp byte[pressed+1],1
;    je near .failedb

    call RemoteGetChar
    cmp dh,0
    je .nocharb5
    cmp dl,229
    jne .nocharb5
.nostat20b
    call EnableSUDPPacket

    mov byte[ChatProgress],0
    mov dword[RecvProgress],0

    ; get LoadDrive/LoadDir
    mov ebx,LoadDir
    mov edx,LoadDrive
    call Get_Dir

    ; change dir to InitDrive/InitDir
    mov dl,[InitDrive]
    mov ebx,InitDir
    ; save config
    call Change_Dir
    call createnewcfg
    call GUISaveVars

    ; change dir to SRAMDrive/SRAMDir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir

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
    mov eax,dword[vidbuffer]
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
    ; Save Cheat Search File
    mov edx,cstempfname
    call Create_File
    jc .csskip2
    mov bx,ax
    mov edx,[vidbuffer]
    add edx,129600
    mov ecx,65536*2+32768
    call Write_File
    call Close_File
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

    mov al,[cfgsoundon]
    mov [soundon],al
    mov al,[cfgStereoSound]
    mov [StereoSound],al
    mov al,[cfgSoundQuality]
    mov [SoundQuality],al
    call AdjustFrequency
    mov byte[GUIOn],0
    mov byte[GUIOn2],0
    mov byte[GUIReset],0
    mov dword[StartLL],0
    mov dword[StartLR],0
    mov byte[NetLoadState],0
    jmp continueprog

.faileda
    call WinErrorA
    jmp continueprog
.failedb
    call WinErrorB
    jmp continueprog

SECTION .bss
CheckSumVal resd 1
SECTION .data
WrongCheckSum db 10,13,'ROM Data Mismatch',10,13,10,13,0
SECTION .text


SRAMDirc:
    ; get LoadDrive/LoadDir
    mov ebx,LoadDir
    mov edx,LoadDrive
    call Get_Dir
    ; change to sram dir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
    ret

LOADDir:
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
    ret

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
    GUIOuttext 56,81,guiftimemsg1,220-15
    GUIOuttext 55,80,guiftimemsg1,220
    GUIOuttext 56,96,guiftimemsg2,220-15
    GUIOuttext 55,95,guiftimemsg2,220
    GUIOuttext 56,104,guiftimemsg3,220-15
    GUIOuttext 55,103,guiftimemsg3,220
    GUIOuttext 56,112,guiftimemsg4,220-15
    GUIOuttext 55,111,guiftimemsg4,220
    GUIOuttext 56,120,guiftimemsg5,220-15
    GUIOuttext 55,119,guiftimemsg5,220
    GUIOuttext 56,128,guiftimemsg6,220-15
    GUIOuttext 55,127,guiftimemsg6,220
    GUIOuttext 56,136,guiftimemsg7,220-15
    GUIOuttext 55,135,guiftimemsg7,220
    GUIOuttext 56,151,guiftimemsg8,220-15
    GUIOuttext 55,150,guiftimemsg8,220
    call vidpastecopyscr
    call GUIUnBuffer
    call DisplayBoxes
    call DisplayMenu
    call JoyRead
    cmp byte[pressed+2Ch],0
    jne .pressedokay
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
    ret

SECTION .data
guiftimemsg1 db 'ONE TIME USER REMINDER :',0
guiftimemsg2 db 'PLEASE BE SURE TO READ',0
guiftimemsg3 db 'GUINOTES.TXT FOR AN',0
guiftimemsg4 db 'IMPORTANT REMINDER.',0
guiftimemsg5 db 'ALSO, WHENEVER YOU HAVE',0
guiftimemsg6 db 'PROBLEMS, BE SURE TO READ',0
guiftimemsg7 db 'ZSNES.FAQ AND README.TXT',0
guiftimemsg8 db 'PRESS "Z" TO CONTINUE.',0
SECTION .text

guimustrestartmsg:
    xor ebx,ebx
    mov ecx,256
.a
    cmp byte[pressed+ebx],1
    jne .npr1
    mov byte[pressed+ebx],2
.npr1
    inc ebx
    dec ecx
    jnz .a
    mov byte[pressed+2Ch],0
.again
    GUIBox 43,87,213,151,160
    GUIBox 43,87,213,87,162
    GUIBox 43,87,43,151,161
    GUIBox 213,87,213,151,159
    GUIBox 43,151,213,151,158
    GUIOuttext 56,92,guiqtimemsg1,220-15
    GUIOuttext 55,91,guiqtimemsg1,220
    GUIOuttext 56,100,guiqtimemsg2,220-15
    GUIOuttext 55,99,guiqtimemsg2,220
    GUIOuttext 56,108,guiqtimemsg3,220-15
    GUIOuttext 55,107,guiqtimemsg3,220
    GUIOuttext 56,116,guiqtimemsg4,220-15
    GUIOuttext 55,115,guiqtimemsg4,220
    GUIOuttext 56,139,guiqtimemsg8,220-15
    GUIOuttext 55,138,guiqtimemsg8,220
    call vidpastecopyscr
    call GUIUnBuffer
    call DisplayBoxes
    call DisplayMenu
    call JoyRead

    mov byte[pressed+2Ch],0

    call JoyRead
    xor ebx,ebx
    mov ecx,256+128+64
.b
    cmp byte[pressed+ebx],1
    je .pressedokay
    inc ebx
    dec ecx
    jnz .b
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
    mov byte[GUIQuit],1
    ret

SECTION .data
guiqtimemsg1 db 'ZSNES MUST BE RESTARTED',0
guiqtimemsg2 db 'TO USE THIS OPTION.',0
guiqtimemsg3 db 'THIS PROGRAM WILL NOW',0
guiqtimemsg4 db 'EXIT.',0
guiqtimemsg8 db 'PRESS ANY KEY.',0
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
    jne .pressedokay
    inc ebx
    dec ecx
    jnz .b
    cmp byte[MouseDis],1
    je .mousedis
    call Get_MouseData
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
    ret

SECTION .data
guiprevidmsg1 db 'ZSNES WILL NOW ATTEMPT',0
guiprevidmsg2 db 'TO CHANGE YOUR VIDEO',0
guiprevidmsg3 db 'MODE.  IF THE CHANGE',0
guiprevidmsg4 db 'IS UNSUCCESSFUL, WAIT',0
guiprevidmsg5 db '10 SECONDS AND VIDEO',0
guiprevidmsg6 db 'MODE WILL BE RESET',0
guiprevidmsg7 db 'PRESS ANY KEY',0
SECTION .text

guipostvideo:
    mov ecx,255*144
    mov eax,[vidbufferofsb]
.loop
    mov dword[eax],0FFFFFFFFh
    add eax,4
    dec ecx
    jnz .loop

    xor ebx,ebx
    mov ecx,256
.a
    mov byte[pressed+ebx],0
    inc ebx
    dec ecx
    jnz .a
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

    call JoyRead
    xor ebx,ebx
    mov ecx,256+128+64
.b2
    cmp byte[pressed+ebx],0
    jne near .pressedfail
    inc ebx
    dec ecx
    jnz .b2
    cmp byte[MouseDis],1
    je .mousedis3
    call Get_MouseData
    test bx,01h
    jnz near .pressedfail
.mousedis3

.again
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
    test bx,01h
    jnz .pressedokay
.mousedis
    cmp dword[GUIkeydelay],0
    je .pressedokay
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
    mov byte[GUIpclicked],1
    ret

SECTION .data
guipostvidmsg1 db 'VIDEO MODE CHANGED.',0
guipostvidmsg2 db 'PRESS ANY KEY',0
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
%ifndef __LINUX__
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

GUILoadManualDir
    mov ebx,GUILoadTextA
    mov [ManualCPtr],ebx
    cmp byte[ebx],0
    je near .nofindfile
    xor eax,eax
.next
    cmp byte[ebx],':'
    jne .nocolon
    mov eax,ebx
.nocolon
    inc ebx
    cmp byte[ebx],0
    jne .next
    or eax,eax
    jz .nomorecolon
    cmp eax,GUILoadTextA
    je .invalidcolon
    mov bl,[eax-1]
    cmp bl,'a'
    jb .nolower
    cmp bl,'z'
    ja .nolower
    sub bl,'a'-'A'
.nolower
    cmp bl,'A'
    jb .invalidcolon
    cmp bl,'Z'
    ja .invalidcolon
    sub bl,'A'
    mov dl,bl
    push eax
    call Change_Drive
    pop eax
    mov byte[ManualStatus],1
.invalidcolon
    inc eax
    mov [ManualCPtr],eax
.nomorecolon
    mov ebx,[ManualCPtr]
    cmp byte[ebx],0
    je near .finish
    xor eax,eax
.next2
    cmp byte[ebx],'\'
    jne .nobackslash
    mov eax,ebx
.nobackslash
    inc ebx
    cmp byte[ebx],0
    jne .next2
    or eax,eax
    jz .finish
    inc eax
    mov cl,[eax]
    mov byte[eax],0
    push ecx
    push eax
    mov edx,[ManualCPtr]
    call Change_Single_Dir
    jc .nosuchdir
    mov byte[ManualStatus],1
.nosuchdir
    pop eax
    pop ecx
    mov [eax],cl
    mov [ManualCPtr],eax
.finish
    mov edx,[ManualCPtr]
    call Change_Single_Dir
    jc .notdir
    mov byte[ManualStatus],1
    jmp .nomoredir
.notdir
    call .nomoredir
    mov edx,[ManualCPtr]
    cmp byte[edx],0
    je .nofindfile
    ; otherwise set ManualStatus to 2
    mov byte[ManualStatus],2
    mov dword[GUIcurrentfilewin],0
.nofindfile
    ret
.nomoredir
    ; refresh dir if necessary
    cmp byte[ManualStatus],1
    jne .norefresh
    call GetLoadData.a
.norefresh
    ret

SECTION .bss
ManualCPtr resd 1
ManualStatus resb 1

NEWSYM MovieCounter, resd 1

SECTION .data
UnableMovie2 db 'MUST PLAY WITH SOUND OFF',0
UnableMovie3 db 'MUST PLAY WITH SOUND ON',0

SECTION .text

MoviePlay:
    cmp byte[CNetType],20
    je near .dontplay
    mov byte[GUICBHold],0
    mov dword[MovieCounter],0
    cmp byte[MovieProcessing],0
    jne near .dontplay
    mov byte[GUIQuit],2
    mov ebx,[statefileloc]
    mov eax,[fnamest+ebx-3]
    push eax
    mov dword[fnamest+ebx-3],'.zmv'
    mov al,[CMovieExt]
    mov byte[fnamest+ebx],al
    call ChangetoSRAMdir
    mov dword[Totalbyteloaded],0
    call loadstate2
    mov edx,fnamest+1
    call Open_File
    jc near .notexist
    mov bx,ax
    mov [MovieFileHand],bx
    mov cx,[Totalbyteloaded+2]
    mov dx,[Totalbyteloaded]
    call File_Seek
    mov edx,RecData
    mov ecx,16
    call Read_File
    cmp byte[RecData+2],1
    jne .noextra
    mov eax,[RecData+3]
    mov [timer2upd],eax
    mov eax,[RecData+7]
    mov [curexecstate],eax
    mov dword[nmiprevaddrl],0
    mov dword[nmiprevaddrh],0
    mov dword[nmirept],0
    mov dword[nmiprevline],224
    mov dword[nmistatus],0
    mov dword[spcnumread],0
	mov dword[spchalted],-1
    mov byte[NextLineCache],0
.noextra
    mov al,[RecData]
    cmp al,[soundon]
    jne near .soundisoff
    cmp dword[ramsize],0
    je .noram
    mov edx,[sram]
    mov ecx,[ramsize]
    call Read_File
.noram
    mov byte[MovieProcessing],1
.skip
    mov dword[PJoyAOrig],0
    mov dword[PJoyBOrig],0
    mov dword[PJoyCOrig],0
    mov dword[PJoyDOrig],0
    mov dword[PJoyEOrig],0
    mov byte[sramsavedis],1
    mov byte[UseRemoteSRAMData],0
    mov byte[DSPMem+08h],0
    mov byte[DSPMem+18h],0
    mov byte[DSPMem+28h],0
    mov byte[DSPMem+38h],0
    mov byte[DSPMem+48h],0
    mov byte[DSPMem+58h],0
    mov byte[DSPMem+68h],0
    mov byte[DSPMem+78h],0
.notexist
    call ChangetoLOADdir
    pop eax
    mov ebx,[statefileloc]
    mov [fnamest+ebx-3],eax
.dontplay
    ret
.soundisoff
    mov dword[Msgptr],UnableMovie2
    cmp byte[soundon],0
    jne .soundon
    mov dword[Msgptr],UnableMovie3
.soundon
    mov eax,[MsgCount]
    mov [MessageOn],eax
    call Close_File
    pop eax
    ret

SECTION .bss
NEWSYM Totalbyteloaded, resd 1
NEWSYM sramsavedis, resb 1

SECTION .data
DevicePtr dd pl1selk,pl2selk,pl3selk,pl4selk,pl5selk

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
    mov byte[GUIwinorder+eax],dl
    mov byte[GUIwinactiv+edx],1
    cmp byte[resetposn],1
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
    mov bl,byte[GUIwinorder+eax]
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
    mov esi,prevloaddnamel+%1*512
    cmp byte[esi+1],0
    je %%notvalid
    mov edi,prevloadfnamel+%1*512
    mov ebx,prevloadnames+%1*16
    mov ecx,%1
    call loadquickfname
%%notvalid
    ret
%%skip
%endmacro

GUITryMenuItem:
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
    mov byte[GUIPrevMenuData.onoff+15],'O'
    mov byte[GUIPrevMenuData.onoff+16],'N'
    mov byte[GUIPrevMenuData.onoff+17],' '
    jmp .on
.off
    mov byte[GUIPrevMenuData.onoff+15],'O'
    mov byte[GUIPrevMenuData.onoff+16],'F'
    mov byte[GUIPrevMenuData.onoff+17],'F'
.on
.skipswitch
    cmp byte[GUIcrowpos],12
    jne .skipclear
    cmp byte[prevlfreeze],0
    jne .skipclear
    mov edi,prevloadnames
    mov eax,20202020h
    mov ecx,4*10
    rep stosd
    mov edi,prevloaddnamel
    xor eax,eax
    mov ecx,128*10
    rep stosd
    mov edi,prevloadfnamel
    mov eax,0 ;20202020h
    mov ecx,128*10
    rep stosd
    call GUIQuickLoadUpdate
    ret
.skipclear
.noquickload
    cmp byte[GUIcmenupos],2
    jne near .nomain
    GUICheckMenuItem 1, 0               ; Load
    cmp byte[GUIcrowpos],0
    jne .noloadrefresh
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
    cmp byte[CNetType],21
    je near .noreset
    cmp byte[CNetType],22
    je near .noreset
    GUICheckMenuItem 12, 2              ; Reset
    cmp byte[GUIcrowpos],2
    jne .noreset
    mov byte[GUICResetPos],1
.noreset
;    cmp byte[OSPort],3
;    je .win32state
;    cmp byte[CNetType],20
;    je near .noromloaded
;.win32state
    cmp byte[CNetType],21
    je near .noromloaded
    cmp byte[CNetType],22
    je near .noromloaded
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
    GUICheckMenuItem 3, 0
    GUICheckMenuItem 3, 1
    GUICheckMenuItem 3, 2
    GUICheckMenuItem 3, 3
    GUICheckMenuItem 3, 4
    cmp byte[GUIcrowpos],0
    jne .noplay1
    mov byte[cplayernum],0
.noplay1
    cmp byte[GUIcrowpos],1
    jne .noplay2
    mov byte[cplayernum],1
.noplay2
    cmp byte[GUIcrowpos],2
    jne .noplay3
    mov byte[cplayernum],2
.noplay3
    cmp byte[GUIcrowpos],3
    jne .noplay4
    mov byte[cplayernum],3
.noplay4
    cmp byte[GUIcrowpos],4
    jne .noplay5
    mov byte[cplayernum],4
.noplay5
    ;The number on the left is the window to open
    ;the number on the right is where in the drop down box we are
    GUICheckMenuItem 17, 6
    GUICheckMenuItem 18, 7
    GUICheckMenuItem 4, 9
    cmp byte[GUIcrowpos],10
    jne near .novideo
    ; set Video cursor location
    xor eax,eax
    mov al,[cvidmode]
    mov [GUIcurrentvideocursloc],eax
    mov edx,[NumVideoModes]
    sub edx,5
    cmp eax,edx
    jbe .noof
    mov eax,edx
.noof
    mov [GUIcurrentvideoviewloc],eax
    mov edx,5
    call CheckMenuItemHelp
.novideo
    GUICheckMenuItem 6, 11
    GUICheckMenuItem 19, 12
.noconfig
    cmp byte[romloadskip],0
    jne near .nocheat
    cmp byte[CheatBDoor],1
    je .yescheat
    cmp byte[CNetType],20
    je near .nocheat
    cmp byte[CNetType],21
    je near .nocheat
    cmp byte[CNetType],22
    je near .nocheat
.yescheat
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
;    cmp byte[GOSPort],3
;    je near .win32
%ifdef __MSDOS__
    cmp byte[CNetType],10
    jae .nomod
    mov byte[CNetType],0
.nomod
    GUICheckMenuItem 8, 0
    GUICheckMenuItem 8, 1
    cmp byte[CNetType],10
    jae near .nonet
    cmp byte[GUIcrowpos],1
    jne .noipx
    mov byte[CNetType],7
.noipx
    cmp byte[GUIcrowpos],0
    jne near .nonet
    mov byte[CNetType],1
    jmp .nonet
%endif
;.win32
    GUICheckMenuItem 8, 0
    cmp byte[CNetType],10
    jae near .nonet
    cmp byte[GUIcrowpos],0
    jne near .nonet
    mov byte[CNetType],4
    call GetHostName
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
    GUICheckMenuItem 16, 3
    cmp byte[GUIcrowpos],4
    jne .nosavestuff

    ; change dir to InitDrive/InitDir
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir
    call createnewcfg
    call GUISaveVars

    call Makemode7Table
    mov dword[GUICMessage],.message1
    mov dword[GUICTimer],50
    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
.nosavestuff
    GUICheckMenuItem 11, 6
.nomisc
    ret

SECTION .data
.message1 db 'CONFIGURATION FILES SAVED.',0
SECTION .text

DisplayBoxes:
    xor esi,esi
    mov byte[cwindrawn],0
.next2
    mov al,[GUIwinorder+esi]
    cmp al,0
    je .done
    inc byte[cwindrawn]
    inc esi
    jmp .next2
.done
    dec byte[cwindrawn]
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
.finstuff
    pop esi
    inc esi
    dec byte[cwindrawn]
    jmp .next
.nomore
    ret



ChangetoSRAMdir:
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
    ret

ChangetoLOADdir:
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
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
    ; change dir to SRAMDrive/SRAMDir
    call ChangetoSRAMdir
    cmp byte[GUIStatesText5],1
    je .loadstate
    call statesaver
    jmp .changedir
.loadstate
    cmp byte[CNetType],20
    jne .notnet
    call NetLoadStuff
    jmp .changedir
.notnet
    call loadstate2
.changedir
    ; change dir to LoadDrive/LoadDir
    call ChangetoLOADdir
    ret

SaveSecondState:
    ; change dir to SRAMDrive/SRAMDir
    call ChangetoSRAMdir
    mov ebx,[statefileloc]
    mov al,[fnamest+ebx]
    mov byte[fnamest+ebx],'s'
    push eax
    call statesaver
    pop eax
    mov ebx,[statefileloc]
    mov [fnamest+ebx],al
    call ChangetoLOADdir
    ret

LoadSecondState:
    call ChangetoSRAMdir
    mov ebx,[statefileloc]
    mov al,[fnamest+ebx]
    mov byte[fnamest+ebx],'s'
    push eax
    call loadstate2
    pop eax
    mov ebx,[statefileloc]
    mov [fnamest+ebx],al
    call ChangetoLOADdir
    ret

GUIProcReset:
    cmp byte[GUICBHold],2
    jne .noreset
    mov byte[GUIReset],1
    call GUIDoReset
    cmp byte[CNetType],20
    jne .noreset
    call PreparePacket
    mov al,40
    call RemoteSendChar
    call SendPacket
    mov byte[GUIQuit],0
.noreset
    mov byte[GUICBHold],0
    xor eax,eax
    mov al,[GUIwinptr]
    dec eax
    mov byte[GUIwinactiv+12],0
    mov byte[GUIwinorder+eax],0
    dec byte[GUIwinptr]
    ret

SECTION .bss
LoadDuplicFound resb 1
SECTION .text

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

;    cmp byte[OSPort],3
;    jne near .notwinpressa
    %ifdef __LINUX__
    GUIShadow 238,9,247,20
    GUIShadow 249,9,257,20
    %endif
    %ifdef __WIN32__
    GUIShadow 238,9,247,14
    GUIShadow 238,16,247,20
    GUIShadow 249,9,257,20
    %endif
.notwinpressa

;    cmp byte[OSPort],3
;    jne near .notwinpressb

    %ifdef __LINUX__
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
    GUIDrawMenuM 52,16,9,13,GUIConfigMenuData,54,57,22,149,42 ;19+13*10
    mov dword[GUICYLocPtr],MenuDat3
.nomenu3
    cmp byte[GUIcmenupos],4
    jne near .nomenu4
    GUIDrawMenuM 99,16,11,3,GUICheatMenuData,101,104,22,49,36 ;19+3*10
    mov dword[GUICYLocPtr],MenuDat4
.nomenu4
    cmp byte[GUIcmenupos],5
    jne near .nomenu5
;    cmp byte[GOSPort],3
;    je near .menu5b
%ifdef __MSDOS__
    GUIDrawMenuM 140,16,10,2,GUINetPlayMenuData,142,145,22,39,48 ;19+2*10
    mov dword[GUICYLocPtr],MenuDat5
    jmp .nomenu5
%endif
.menu5b
    GUIDrawMenuM 140,16,10,1,GUINetPlayMenuData,142,145,22,29,48 ;19+2*10
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
    call ClearScreen
    call Clear2xSaIBuffer
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
    add ah,1
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

SECTION .bss ;ALIGN=32
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
    mov word[GUICPC+%1*2],ax
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
    add ah,1
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

    cmp byte[vesa2red10],1
    jne .nored10
    jmp .nored10
    mov esi,GUICPC
    mov ecx,256
.next2
    mov ax,[esi]
    mov bx,ax
    and bx,0000000000011111b
    and ax,1111111111000000b
    shr ax,1
    or ax,bx
    mov [esi],ax
    add esi,2
    dec ecx
    jnz .next2
.nored10
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
        db 17,18,18,19,20,20,21,22,22,23,24,24,25,26,26,27,28,28,29,30,30,
        db 31
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

SECTION .data
GUIMousePtr db 50+88,47+88,45+88,43+88,42+88,00,00,00
            db 53+88,52+88,46+88,42+88,00,00,00,00
            db 55+88,54+88,54+88,44+88,00,00,00,00
            db 57+88,57+88,56+88,52+88,45+88,00,00,00
            db 59+88,00,00,55+88,50+88,45+88,00,00
            db 00,00,00,00,55+88,50+88,45+88,00
            db 00,00,00,00,00,55+88,50+88,47+88
            db 00,00,00,00,00,00,52+88,00

            db 50,47,45,43,40,00,00,00
            db 53,52,46,42,00,00,00,00
            db 55,54,54,44,00,00,00,00
            db 57,57,56,52,45,00,00,00
            db 59,00,00,55,50,45,00,00
            db 00,00,00,00,55,50,45,00
            db 00,00,00,00,00,55,50,47
            db 00,00,00,00,00,00,52,00

NEWSYM GUIFontData
         db 0,0,0,0,0
         db 01110000b
         db 10011000b
         db 10101000b
         db 11001000b
         db 01110000b; 0
         db 00100000b
         db 01100000b
         db 00100000b
         db 00100000b
         db 01110000b; 1
         db 01110000b
         db 10001000b
         db 00110000b
         db 01000000b
         db 11111000b; 2
         db 01110000b
         db 10001000b
         db 00110000b
         db 10001000b
         db 01110000b; 3
         db 01010000b
         db 10010000b
         db 11111000b
         db 00010000b
         db 00010000b; 4
         db 11111000b
         db 10000000b
         db 11110000b
         db 00001000b
         db 11110000b; 5
         db 01110000b
         db 10000000b
         db 11110000b
         db 10001000b
         db 01110000b; 6
         db 11111000b
         db 00001000b
         db 00010000b
         db 00010000b
         db 00010000b; 7
         db 01110000b
         db 10001000b
         db 01110000b
         db 10001000b
         db 01110000b; 8
         db 01110000b
         db 10001000b
         db 01111000b
         db 00001000b
         db 01110000b; 9
         db 01110000b
         db 10001000b
         db 11111000b
         db 10001000b
         db 10001000b; A
         db 11110000b
         db 10001000b
         db 11110000b
         db 10001000b
         db 11110000b; B
         db 01110000b
         db 10001000b
         db 10000000b
         db 10001000b
         db 01110000b; C
         db 11110000b
         db 10001000b
         db 10001000b
         db 10001000b
         db 11110000b; D
         db 11111000b
         db 10000000b
         db 11110000b
         db 10000000b
         db 11111000b; E
         db 11111000b
         db 10000000b
         db 11110000b
         db 10000000b
         db 10000000b; F
         db 01111000b
         db 10000000b
         db 10011000b
         db 10001000b
         db 01110000b; G
         db 10001000b
         db 10001000b
         db 11111000b
         db 10001000b
         db 10001000b; H
         db 11111000b
         db 00100000b
         db 00100000b
         db 00100000b
         db 11111000b; I
         db 01111000b
         db 00010000b
         db 00010000b
         db 10010000b
         db 01100000b; J
         db 10010000b
         db 10100000b
         db 11100000b
         db 10010000b
         db 10001000b; K
         db 10000000b
         db 10000000b
         db 10000000b
         db 10000000b
         db 11111000b; L
         db 11011000b
         db 10101000b
         db 10101000b
         db 10101000b
         db 10001000b; M
         db 11001000b
         db 10101000b
         db 10101000b
         db 10101000b
         db 10011000b; N
         db 01110000b
         db 10001000b
         db 10001000b
         db 10001000b
         db 01110000b; O
         db 11110000b
         db 10001000b
         db 11110000b
         db 10000000b
         db 10000000b; P
         db 01110000b
         db 10001000b
         db 10101000b
         db 10010000b
         db 01101000b; Q
         db 11110000b
         db 10001000b
         db 11110000b
         db 10010000b
         db 10001000b; R
         db 01111000b
         db 10000000b
         db 01110000b
         db 00001000b
         db 11110000b; S
         db 11111000b
         db 00100000b
         db 00100000b
         db 00100000b
         db 00100000b; T
         db 10001000b
         db 10001000b
         db 10001000b
         db 10001000b
         db 01110000b; U
         db 10001000b
         db 10001000b
         db 01010000b
         db 01010000b
         db 00100000b; V
         db 10001000b
         db 10101000b
         db 10101000b
         db 10101000b
         db 01010000b; W
         db 10001000b
         db 01010000b
         db 00100000b
         db 01010000b
         db 10001000b; X
         db 10001000b
         db 01010000b
         db 00100000b
         db 00100000b
         db 00100000b; Y
         db 11111000b
         db 00010000b
         db 00100000b
         db 01000000b
         db 11111000b; Z
         db 00000000b
         db 00000000b
         db 11111000b
         db 00000000b
         db 00000000b; -
         db 00000000b
         db 00000000b
         db 00000000b
         db 00000000b
         db 11111000b; _
         db 01101000b
         db 10010000b
         db 00000000b
         db 00000000b
         db 00000000b; ~
         db 00000000b
         db 00000000b
         db 00000000b
         db 00000000b
         db 00100000b; .
         db 00001000b
         db 00010000b
         db 00100000b
         db 01000000b
         db 10000000b; /
         db 00010000b
         db 00100000b
         db 01000000b
         db 00100000b
         db 00010000b; <
         db 01000000b
         db 00100000b
         db 00010000b
         db 00100000b
         db 01000000b; >
         db 01110000b
         db 01000000b
         db 01000000b
         db 01000000b
         db 01110000b; [
         db 01110000b
         db 00010000b
         db 00010000b
         db 00010000b
         db 01110000b; ]
         db 00000000b
         db 00100000b
         db 00000000b
         db 00100000b
         db 00000000b; :
         db 01100000b
         db 10011000b
         db 01110000b
         db 10011000b
         db 01101000b; &
         db 00100000b
         db 00100000b
         db 10101000b
         db 01110000b
         db 00100000b; arrow
         db 01010000b
         db 11111000b
         db 01010000b
         db 11111000b
         db 01010000b; #
         db 00000000b
         db 11111000b
         db 00000000b
         db 11111000b
         db 00000000b; =
         db 01001000b
         db 10010000b
         db 00000000b
         db 00000000b
         db 00000000b; "
         db 10000000b
         db 01000000b
         db 00100000b
         db 00010000b
         db 00001000b; \ (Screw you nassm)
         db 10101000b
         db 01110000b
         db 11111000b
         db 01110000b
         db 10101000b; *
         db 01110000b
         db 10001000b
         db 00110000b
         db 00000000b
         db 00100000b; ?
         db 10001000b
         db 00010000b
         db 00100000b
         db 01000000b
         db 10001000b; %
         db 00100000b
         db 00100000b
         db 11111000b
         db 00100000b
         db 00100000b; +
         db 00000000b
         db 00000000b
         db 00000000b
         db 00100000b
         db 01000000b; ,
         db 00110000b
         db 01000000b
         db 01000000b
         db 01000000b
         db 00110000b; (
         db 01100000b
         db 00010000b
         db 00010000b
         db 00010000b
         db 01100000b; )
         db 01110000b
         db 10011000b
         db 10111000b
         db 10000000b
         db 01110000b; @
         db 00100000b
         db 01000000b
         db 00000000b
         db 00000000b
         db 00000000b; '
         db 00100000b
         db 00100000b
         db 00100000b
         db 00000000b
         db 00100000b; !
         db 01111000b
         db 10100000b
         db 01110000b
         db 00101000b
         db 11110000b; $
         db 00000000b
         db 00100000b
         db 00000000b
         db 00100000b
         db 01000000b; ;
         db 01000000b
         db 00100000b
         db 00000000b
         db 00000000b
         db 00000000b; `
         db 00100000b
         db 01010000b
         db 00000000b
         db 00000000b
         db 00000000b; ^
         db 00110000b
         db 01000000b
         db 11000000b
         db 01000000b
         db 00110000b; {
         db 01100000b
         db 00010000b
         db 00011000b
         db 00010000b
         db 01100000b; }
         db 00100000b
         db 00100000b
         db 01110000b
         db 01110000b
         db 11111000b; Up
         db 11111000b
         db 01110000b
         db 01110000b
         db 00100000b
         db 00100000b; Down
         db 00001000b
         db 00111000b
         db 11111000b
         db 00111000b
         db 00001000b; Left
         db 10000000b
         db 11100000b
         db 11111000b
         db 11100000b
         db 10000000b; Right
         db 00100000b
         db 01100000b
         db 11111000b
         db 01100000b
         db 00100000b; Arrow Left
         db 00111000b
         db 00100000b
         db 00110000b
         db 00001000b
         db 10110000b; .5
         db 11111100b
         db 10000100b
         db 11111100b
         db 00000000b
         db 00000000b; Maximize
         db 00000000b
         db 11111100b
         db 00000000b
         db 00000000b
         db 00000000b; Minimize
         db 11111000b
         db 10001000b
         db 10001000b
         db 10001000b
         db 11111000b; Maximize (Linux)

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

GUIIconDataCheckBoxX:
    db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,165,0  ,0  ,0  ,0  ,0  ,165,0  ,0
    db 0  ,220,165,218,217,216,165,0  ,0  ,0
    db 0  ,219,218,165,216,165,214,202,0  ,0
    db 0  ,218,217,216,165,214,213,202,0  ,0
    db 0  ,217,216,165,214,165,212,202,0  ,0
    db 0  ,216,165,214,213,212,165,202,0  ,0
    db 0  ,165,214,213,212,211,210,165,0  ,0
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

SECTION .text

NEWSYM GuiAsmEnd
