/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#ifdef __UNIXSDL__
#include "gblhdr.h"
#else
#include <io.h>
#include <stdio.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <direct.h>
#endif
#endif

#define StringA "FRAMESKIP"
#define StringB "AUTOFRAMESKIP"
#define StringC "PLAYER1DEVICE"
#define StringD "PLAYER2DEVICE"
#define StringE "SCANKEY1"
#define StringF "SCANKEY2"
#define StringG "SOUND"
#define StringH "SOUNDRATE"
#ifdef __WIN32__
#define StringI "VIDEOMODEWIN"
#endif
#ifdef __UNIXSDL__
#define StringI "VIDEOMODELIN"
#endif
#ifdef __MSDOS__
#define StringI "VIDEOMODEDOS"
#endif
#define StringJ "EXECUTE"
#define StringM "STEREO"
#define StringN "GUIDISABLE"
#define StringO "SCANLINES"
#define StringP "INTERPOLATION"
#define StringQ "ENTERSKIP"
#define StringR "FORCE8BIT"
#define StringS "SAVEDIRECTORY"
#define StringT "GAMEDIRECTORY"
#define StringU "JOYMAP1"
#define StringV "NEWGFX"
#define StringW "VSYNC"
#define StringX "VOLUME"
#define StringY "ECHODISABLE"
#define StringZ "REVERSESTEREO"
#define String1 "PL34TO12SHARE"
#define String2 "DONTSAVE"
#define String3 "REINITTIME"

#define ASCIIChar2Bool(x) (((x)-'0') ? 1 : 0)

extern char CMDLineStr[256];
extern char LoadDir[128];
extern char LoadDrive[2];

extern unsigned int pl2selk;
extern unsigned int pl2startk;
extern unsigned int pl2upk;
extern unsigned int pl2Ak;
extern unsigned int pl2Bk;
extern unsigned int pl2Lk;
extern unsigned int pl2Rk;
extern unsigned int pl2Xk;
extern unsigned int pl2Yk;
extern unsigned int pl2downk;
extern unsigned int pl2leftk;
extern unsigned int pl2rightk;
extern unsigned char pl2contrl;

extern unsigned int pl1selk;
extern unsigned int pl1startk;
extern unsigned int pl1upk;
extern unsigned int pl1Ak;
extern unsigned int pl1Bk;
extern unsigned int pl1Lk;
extern unsigned int pl1Rk;
extern unsigned int pl1Xk;
extern unsigned int pl1Yk;
extern unsigned int pl1downk;
extern unsigned int pl1leftk;
extern unsigned int pl1rightk;
extern unsigned char pl1contrl;
extern unsigned char JoyBC;
extern unsigned char JoyLC;
extern unsigned char JoyRC;
extern unsigned char JoySelec;
extern unsigned char JoyStart;
extern unsigned char JoyXC;
extern unsigned char JoyYC;
extern unsigned char JoyAC;
extern unsigned int per2exec;
extern unsigned int newengen;
extern char SRAMDrive[2];
extern unsigned char DontSavePath;
extern unsigned char guioff;
extern unsigned char frameskip;
extern unsigned char enterpress;
extern unsigned char cvidmode;
extern unsigned char antienab;
extern unsigned char StereoSound;
extern unsigned int SoundQuality;
extern unsigned char MusicRelVol;
extern unsigned char Force8b;
extern unsigned char scanlines;
extern unsigned char soundon;
extern unsigned char spcon;
extern unsigned char vsyncon;

extern unsigned char savecfgforce;

//extern void Open_File();
extern unsigned int ZFileRead();
extern unsigned int ZOpenMode;
extern unsigned int ZCloseFileHandle;
extern char *ZOpenFileName;
extern unsigned char *ZFileWriteBlock;
extern unsigned int ZFileWriteSize;
extern unsigned int ZFileWriteHandle;
extern unsigned char *ZFileReadBlock;
extern unsigned int ZFileReadHandle;
extern unsigned int ZFileReadSize;
extern unsigned int ZOpenFile(); //Create_File. Open_File
extern unsigned int ZFileWrite(); //Write_File();
extern unsigned int ZCloseFile(); //Close_File

#ifdef __UNIXSDL__
extern char zcfgdir[1024];
unsigned char cfgloadsdir = 1; //Set to yes, since savedir is always set and considered to be changed
#else
unsigned char cfgloadsdir = 0; //Only set to yes if modified in the paths window
#endif


char SRAMDir[1024];

char LoadDriveB[2];
char LoadDirB[128];

unsigned char cfgsoundon = 0;
unsigned char cfgSoundQuality = 5;
unsigned char cfgStereoSound = 1;
unsigned char cfgguioff = 0;
unsigned char cfgper2exec = 100;
unsigned char cfgcvidmode = 4;
unsigned char cfgscanline = 0;
unsigned char cfginterp = 0;
unsigned char cfgenterskip = 0;
unsigned char cfgforce8b = 0;
unsigned char cfgloadgdir = 0;
unsigned char cfgnewgfx = 0;
unsigned char cfgvsync = 0;
unsigned char cfgvolume = 75;
unsigned char cfgecho = 0;
unsigned char RevStereo = 0;
unsigned char JoyStatRead = 0;
unsigned char pl12s34 = 0;
unsigned char cfgdontsave = 0;
unsigned char cfgreinittime = 30;

#define ConvertJoyMapHelp(a,b,c)\
  if(b == c) \
  {\
    if(c!=0)\
    { \
      c+=0x81;\
      a=c;\
    }\
  }



void ConvertJoyMap1()
{
  unsigned int bl;
  // Convert if 2,4,6, or sidewinder
  if (pl1contrl == 2)
  {
    pl1Bk = 0x83;
    pl1Yk = 0x82;
    pl1upk = 0xCC;
    pl1downk = 0xCD;
    pl1leftk = 0xCE;
    pl1rightk = 0xCF;
  }

  if (pl1contrl == 3 || pl1contrl == 4)
  {
    bl = 4;
  }
  else
  {
    bl = 0;
  }

  if (pl1contrl == 5)
  {
    bl = 6;
  }
  if (bl != 0)
  {
    // Convert button data
    pl1upk = 0xCC;
    pl1downk = 0xCD;
    pl1leftk = 0xCE;
    pl1rightk = 0xCF;
    ConvertJoyMapHelp(JoyStart, bl, pl1startk);
    ConvertJoyMapHelp(JoySelec, bl, pl1selk);
    ConvertJoyMapHelp(JoyBC, bl, pl1Yk);
    ConvertJoyMapHelp(JoyYC, bl, pl1Xk);
    ConvertJoyMapHelp(JoyAC, bl, pl1Bk);
    ConvertJoyMapHelp(JoyXC, bl, pl1Ak);
    ConvertJoyMapHelp(JoyLC, bl, pl1Lk);
    ConvertJoyMapHelp(JoyRC, bl, pl1Rk);
  }
  if (pl1contrl == 6)
  {
    pl1upk = 0xD4;
    pl1downk = 0xD5;
    pl1leftk = 0xD6;
    pl1rightk = 0xD7;
    pl1startk = 0xC8;
    pl1selk = 0xC9;
    pl1Ak = 0x89;
    pl1Bk = 0x88;
    pl1Xk = 0x8C;
    pl1Yk = 0x8B;
    pl1Lk = 0x8E;
    pl1Rk = 0x8F;
  }
  return;
}

void ConvertJoyMap2()
{
  unsigned int bl;
  //  mov al,[pl2contrl]
  // Convert if 2,4,6, or sidewinder
  //If pl1contrl=2 and pl2contrl=2, then set pl2 buttons to 3 & 4
  if (pl2contrl == 2)
  {
    if (pl1contrl != 2)
    {
      pl2Bk = 0x83;
      pl2Yk = 0x82;
      pl2upk = 0xCC;
      pl2downk = 0xCD;
      pl2leftk = 0xCE;
      pl2rightk = 0xCF;
    }
    else
    {
      pl2Bk = 0x85;
      pl2Yk = 0x84;
      pl2upk = 0xE8;
      pl2downk = 0xE9;
      pl2leftk = 0xEA;
      pl2rightk = 0xEB;
    }
  }

  if (pl2contrl == 3 || pl2contrl == 4)
  {
    bl = 4;
  }
  else
  {
    bl = 0;
  }

  if (pl2contrl == 5)
  {
    bl = 6;
  }

  if (bl != 0)
  {
    //Convert button data
    pl2upk = 0xCC;
    pl2downk = 0xCD;
    pl2leftk = 0xCE;
    pl2rightk = 0xCF;
    ConvertJoyMapHelp(JoyStart, bl, pl2startk);
    ConvertJoyMapHelp(JoySelec, bl, pl2selk);
    ConvertJoyMapHelp(JoyBC, bl, pl2Yk);
    ConvertJoyMapHelp(JoyYC, bl, pl2Xk);
    ConvertJoyMapHelp(JoyAC, bl, pl2Bk);
    ConvertJoyMapHelp(JoyXC, bl, pl2Ak);
    ConvertJoyMapHelp(JoyLC, bl, pl2Lk);
    ConvertJoyMapHelp(JoyRC, bl, pl2Rk);
  }

  //If both sidewinder, set pl2 buttons to sw2
  if (pl2contrl == 6)
  {
    if (pl1contrl != 6)
    {
      pl2upk = 0xD4;
      pl2downk = 0xD5;
      pl2leftk = 0xD6;
      pl2rightk = 0xD7;
      pl2startk = 0xC8;
      pl2selk = 0xC9;
      pl2Ak = 0x89;
      pl2Bk = 0x88;
      pl2Xk = 0x8C;
      pl2Yk = 0x8B;
      pl2Lk = 0x8E;
      pl2Rk = 0x8F;
    }
    else
    {
      pl2upk = 0xDC;
      pl2downk = 0xDD;
      pl2leftk = 0xDE;
      pl2rightk = 0xDF;
      pl2startk = 0xD0;
      pl2selk = 0xD1;
      pl2Ak = 0x91;
      pl2Bk = 0x90;
      pl2Xk = 0x94;
      pl2Yk = 0x93;
      pl2Lk = 0x96;
      pl2Rk = 0x97;
      pl2contrl = 7;
    }
  }
  return;
}

void ConvertJoyMap()
{
  if (JoyStatRead == 1)
  {
    ConvertJoyMap1();
    ConvertJoyMap2();
  }
  return;
}

#define SAVE_LINE(a) fwrite(a, 1, strlen(a), fp)
#define WRITE_LINE(a) sprintf(buffer, a);\
  SAVE_LINE(buffer);


void DOScreatenewcfg()
{
  char buffer[4096];
  FILE *fp = 0;

  if (cfgdontsave && !savecfgforce)
  {
    return;
  }

#ifdef __UNIXSDL__
  chdir(zcfgdir);
#endif

  fp = fopen(CMDLineStr, "wb");
  if (!fp)
  {
    return;
  }

  WRITE_LINE("; ZSNES Configuration file\r\n\r\n");
  WRITE_LINE("; Frame Skip: 0 = Auto, 1-10 = Skip 0 .. 9\r\n\r\n");

  sprintf(buffer, "FrameSkip = %d\r\n\r\n", frameskip);
  SAVE_LINE(buffer);

//  WRITE_LINE("; Auto Frame Skip = 0 or 1 (1 = ON)\r\n\r\n");

//  sprintf(buffer, "AutoFrameSkip = %d\r\n\r\n", (frameskip == 0) ? 1 : 0);
//  SAVE_LINE(buffer);

  WRITE_LINE("; Player 1/2 Input Device.  Use the GUI to set these values\r\n");
  WRITE_LINE("; NOTE : Using this to select joysticks manually will NOT work!\r\n\r\n");

  sprintf(buffer, "Player1Device = %d\r\n", pl1contrl);
  SAVE_LINE(buffer);

  sprintf(buffer, "Player2Device = %d\r\n\r\n", pl2contrl);
  SAVE_LINE(buffer);

  WRITE_LINE("; Keyboard Scancodes/Joystick Mappings for Keyboard 1 & 2\r\n");
  WRITE_LINE("; In order of Right, Left, Down, Up, Start, Select, B, Y, A, X, L, R\r\n");
  WRITE_LINE("; Use the GUI to set these values\r\n\r\n");

  sprintf(buffer, "ScanKey1 = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", pl1rightk, pl1leftk,
          pl1downk, pl1upk, pl1startk, pl1selk, pl1Bk, pl1Yk, pl1Ak, pl1Xk, pl1Lk, pl1Rk);
  SAVE_LINE(buffer);

  sprintf(buffer, "ScanKey2 = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n\r\n", pl2rightk, pl2leftk,
          pl2downk, pl2upk, pl2startk, pl2selk, pl2Bk, pl2Yk, pl2Ak, pl2Xk, pl2Lk, pl2Rk);
  SAVE_LINE(buffer);

  WRITE_LINE("; Share Player 3 and 4 control inputs with Player 1 and 2 to allow\r\n");
  WRITE_LINE("; 2 devices to be shared on a single player.  This feature automatically\r\n");
  WRITE_LINE("; disables MultiTap (Multiplayer 5) support.  Set this to 1 to enable.\r\n\r\n");

  sprintf(buffer, "Pl34to12Share = %d\r\n\r\n", pl12s34);
  SAVE_LINE(buffer);

  WRITE_LINE("; Percent to Execute [50 .. 150]\r\n\r\n");

  sprintf(buffer, "Execute = %d\r\n\r\n", cfgper2exec);
  SAVE_LINE(buffer);

#ifdef __WIN32__
  WRITE_LINE("; Video Mode, 0 - 32\r\n");
  WRITE_LINE(";   0 = 256x224   R WIN       1 = 256x224   R FULL\r\n");
  WRITE_LINE(";   2 = 512x448   R WIN       3 = 512x448   DR WIN\r\n");
  WRITE_LINE(";   4 = 640x480   S WIN       5 = 640x480   DS WIN\r\n");
  WRITE_LINE(";   6 = 640x480   DR FULL     7 = 640x480   DS FULL\r\n");
  WRITE_LINE(";   8 = 640x480   S FULL      9 = 768x672   R WIN\r\n");
  WRITE_LINE(";  10 = 768x672   DR WIN     11 = 800x600   S WIN\r\n");
  WRITE_LINE(";  12 = 800x600   DS WIN     13 = 800x600   S FULL\r\n");
  WRITE_LINE(";  14 = 800x600   DR FULL    15 = 800x600   DS FULL\r\n");
  WRITE_LINE(";  16 = 1024x768  S WIN      17 = 1024x768  DS WIN\r\n");
  WRITE_LINE(";  18 = 1024x768  S FULL     19 = 1024x768  DR FULL\r\n");
  WRITE_LINE(";  20 = 1024x768  DS FULL    21 = 1024x896  R WIN\r\n");
  WRITE_LINE(";  22 = 1024x896  DR WIN     23 = 1280x960  S WIN\r\n");
  WRITE_LINE(";  24 = 1280x960  DS WIN     25 = 1280x960  S FULL\r\n");
  WRITE_LINE(";  26 = 1280x960  DR FULL    27 = 1280x960  DS FULL\r\n");
  WRITE_LINE(";  28 = 1280x1024 S WIN      29 = 1280x1024 DS WIN\r\n");
  WRITE_LINE(";  30 = 1280x1024 S FULL     31 = 1280x1024 DR FULL\r\n");
  WRITE_LINE(";  32 = 1280x1024 DS FULL    33 = 1600x1200 S WIN\r\n");
  WRITE_LINE(";  34 = 1600x1200 DS WIN     35 = 1600x1200 DR FULL\r\n");
  WRITE_LINE(";  36 = 1600x1200 DS FULL    37 = 1680x1050 DR FULL\r\n\r\n");
  sprintf(buffer, "VideoModeWin = %d\r\n\r\n", cfgcvidmode);
  SAVE_LINE(buffer);
#endif
#ifdef __UNIXSDL__
#ifdef __OPENGL__
  WRITE_LINE("; Video Mode, 0 - 21\r\n");
#else
  WRITE_LINE("; Video Mode, 0 - 5\r\n");
#endif
  WRITE_LINE(";   0 = 256x224   R WIN        1 = 256x224  R FULL\r\n");
  WRITE_LINE(";   2 = 512x448   DR WIN       3 = 512x448  DR FULL\r\n");
  WRITE_LINE(";   4 = 640x480   DR FULL      5 = 800x600  DR FULL\r\n");
#ifdef __OPENGL__
  WRITE_LINE(";   6 = 256x224   OR  WIN      7 = 512x448   ODR WIN\r\n");
  WRITE_LINE(";   8 = 640x480   ODS FULL     9 = 640x480   ODS WIN\r\n");
  WRITE_LINE(";  10 = 640x576   ODR WIN     11 = 768x672   ODR WIN\r\n");
  WRITE_LINE(";  12 = 800x600   ODS FULL    13 = 800x600   ODS WIN\r\n");
  WRITE_LINE(";  14 = 896x784   ODR WIN     15 = 1024x768  ODS FULL\r\n");
  WRITE_LINE(";  16 = 1024x768  ODS WIN     17 = 1024x896  ODR WIN\r\n");
  WRITE_LINE(";  18 = 1280x960  ODS FULL    19 = 1280x1024 ODS FULL\r\n");
  WRITE_LINE(";  20 = 1600x1200 ODR FULL    21 = VARIABLE  ODR WIN\r\n");
#endif
  sprintf(buffer, "\r\nVideoModeLin = %d\r\n\r\n", cfgcvidmode);
  SAVE_LINE(buffer);

#endif
#ifdef __MSDOS__
  WRITE_LINE("; Video Mode, 0 - 18\r\n");
  WRITE_LINE(";  0 = 256x224x8B  (MODEQ)  1 = 256x240x8B (MODEQ)\r\n");
  WRITE_LINE(";  2 = 256x256x8B  (MODEQ)  3 = 320x224x8B (MODEX)\r\n");
  WRITE_LINE(";  4 = 320x240x8B  (MODEX)  5 = 320x256x8B (MODEX)\r\n");
  WRITE_LINE(";  6 = 640x480x16B (VESA1)  7 = 320x240x8B (VESA2)\r\n");
  WRITE_LINE(";  8 = 320x240x16B (VESA2)  9 = 320x480x8B (VESA2)\r\n");
  WRITE_LINE("; 10 = 320x480x16B (VESA2) 11 = 512x384x8B (VESA2)\r\n");
  WRITE_LINE("; 12 = 512x384x16B (VESA2) 13 = 640x400x8B (VESA2)\r\n");
  WRITE_LINE("; 14 = 640x400x16B (VESA2) 15 = 640x480x8B (VESA2)\r\n");
  WRITE_LINE("; 16 = 640x480x16B (VESA2) 17 = 800x600x8B (VESA2)\r\n");
  WRITE_LINE("; 18 = 800x600x16B (VESA2)\r\n\r\n");

  sprintf(buffer, "VideoModeDos = %d\r\n\r\n", cfgcvidmode);
  SAVE_LINE(buffer);
#endif
  WRITE_LINE("; Sound Emulation = 0 or 1 (1 = ON)\r\n\r\n");

  sprintf(buffer, "Sound = %d\r\n\r\n", cfgsoundon);
  SAVE_LINE(buffer);

  WRITE_LINE("; Sound Sampling Rate\r\n");
  WRITE_LINE(";   0 =  8,000 Hz, 1 = 11,025 Hz, 2 = 22,050 Hz\r\n");
  WRITE_LINE(";   3 = 44,100 Hz, 4 = 16,000 Hz, 5 = 32,000 Hz\r\n");
  WRITE_LINE(";   6 = 48,000 Hz\r\n\r\n");

  sprintf(buffer, "SoundRate = %d\r\n\r\n", cfgSoundQuality);
  SAVE_LINE(buffer);

  WRITE_LINE("; Stereo (0 = off, 1 = on)\r\n\r\n");
  sprintf(buffer, "Stereo = %d\r\n\r\n", cfgStereoSound);
  SAVE_LINE(buffer);

  WRITE_LINE("; Stereo Reversed.  Swaps left channel with right. (0 = off, 1 = L <-> R)\r\n\r\n");
  sprintf(buffer, "ReverseStereo = %d\r\n\r\n", RevStereo);
  SAVE_LINE(buffer);

  WRITE_LINE("; GUI Disable (1 = Disable GUI, 0 = Enable GUI)\r\n\r\n");
  sprintf(buffer, "GUIDisable = %d\r\n\r\n", cfgguioff);
  SAVE_LINE(buffer);

  WRITE_LINE("; New Graphics Engine (1 = Enable, 0 = Disable)\r\n");
#ifdef __MSDOS__
  WRITE_LINE("; All 256 color modes and 320x240x65536 supported\r\n");
#endif
  sprintf(buffer, "\r\nNewGfx = %d\r\n\r\n", cfgnewgfx);
  SAVE_LINE(buffer);

  WRITE_LINE("; Scanlines (0 = Disable, 1 = Full, 2 = 25%%, 3 = 50%%)\r\n");
#ifdef __MSDOS__
  WRITE_LINE("; 256x256x256 or 640x480 modes only (25%% and 50%% in 640x480x65536 mode only)\r\n");
#endif
  sprintf(buffer, "\r\nScanlines = %d\r\n", cfgscanline);
  SAVE_LINE(buffer);
#ifdef __MSDOS__
  WRITE_LINE("\r\n; Interpolation (1 = Enable, 0 = Disable) - 640x480x65536 mode only\r\n");
  WRITE_LINE("; This option also Enables EAGLE          - 640x480x256 mode only\r\n");
#else
  WRITE_LINE("\r\n; Interpolation (1 = Enable, 0 = Disable)\r\n");
#endif
  sprintf(buffer, "\r\nInterpolation = %d\r\n\r\n", cfginterp);
  SAVE_LINE(buffer);

#ifndef __UNIXSDL__
  WRITE_LINE("; VSync (1 = Enable, 0 = Disable) - Wait for Vertical Sync (Fast cpu reqd)\r\n\r\n");
  sprintf(buffer, "VSync = %d\r\n\r\n", cfgvsync);
  SAVE_LINE(buffer);

#endif
#ifdef __MSDOS__
  WRITE_LINE("; Skip Enter Press at Beginning  (1 = Yes, 0 = No)\r\n\r\n");
  sprintf(buffer, "EnterSkip = %d\r\n\r\n", cfgenterskip);
  SAVE_LINE(buffer);

  WRITE_LINE("; Force 8-bit sound on  (1 = Yes, 0 = No)\r\n\r\n");
  sprintf(buffer, "Force8bit = %d\r\n\r\n", cfgforce8b);
  SAVE_LINE(buffer);

#endif
  WRITE_LINE("; Disable Echo  (1 = Yes, 0 = No)\r\n\r\n");
  sprintf(buffer, "EchoDisable = %d\r\n\r\n", cfgecho);
  SAVE_LINE(buffer);

  WRITE_LINE("; Sound Volume Level (0 .. 100)\r\n");
  WRITE_LINE("; Note : Setting this too high can cause sound overflow which degrades quality\r\n\r\n");
  sprintf(buffer, "Volume = %d\r\n\r\n", cfgvolume);
  SAVE_LINE(buffer);

  WRITE_LINE("; Set this to 1 if you do not want ZSNES to save the configuration files.\r\n\r\n");
  sprintf(buffer, "DontSave = %d\r\n\r\n", cfgdontsave);
  SAVE_LINE(buffer);

  WRITE_LINE("; Savefile directory.  Leave it blank if you want the save files to be in the\r\n");
  WRITE_LINE("; same directory as the games.  It should be in a format like : C:\\dir\\dir\r\n\r\n");

  if (cfgloadsdir)
  {
    sprintf(buffer, "SaveDirectory = %s\r\n\r\n", SRAMDir);
  }
  else
  {
    sprintf(buffer, "SaveDirectory = \r\n\r\n");
  }
  SAVE_LINE(buffer);

  WRITE_LINE("; Game directory.  This is the directory where the GUI starts at.\r\n");
  WRITE_LINE("; ZSNES automatically writes the current directory here upon exit.\r\n\r\n");

  if (DontSavePath != 1)
  {
#ifdef __UNIXSDL__
    sprintf(buffer, "GameDirectory = %s\r\n\r\n", LoadDir);
#else
    sprintf(buffer, "GameDirectory = %c:\\%s\r\n", (char) (*LoadDrive + 65), LoadDir);
#endif
  }
  else
  {
#ifdef __UNIXSDL__
    sprintf(buffer, "GameDirectory = %s\r\n\r\n", LoadDirB);
#else
    sprintf(buffer, "GameDirectory = %c:\\%s\r\n", (char) (*LoadDriveB + 65), LoadDirB);
#endif
  }
  SAVE_LINE(buffer);
  fclose(fp);
}

unsigned char _per2exec;
unsigned char _volume;
unsigned short _fileloc;
unsigned char _eofile;
unsigned char _ignore;
unsigned int _strlen;
unsigned int _stralen; // actual string length
unsigned int _strlena;
unsigned int _strlenb;
unsigned char _cchar;
unsigned char _forceauto;
char _string[128];  // full string
char _stringa[128];
char _stringb[128];
unsigned char _usespace;

void getcfg()
{
  unsigned char button_select = 0;
  int i = 0, j = 0;
  char temp;
  unsigned char no_save = 0;
  _forceauto = 0;

  //open file
  ZOpenFileName = CMDLineStr;
  ZOpenMode = 0;
  i = ZOpenFile();
  if (-1 != i)
  {
    _fileloc = i;


    //mov [.fileloc],ax //handled above
    _eofile = 0;
    ZFileReadHandle = _fileloc;
    ZFileReadSize = 1;
    ZFileReadBlock = &_cchar;

    do
    {
      _strlen = 0;
      _stralen = 0;
      _ignore = 0;

      while (0 != (i = ZFileRead()))
      {
        if (_cchar == ';')
        {
          _strlen++;
          _ignore = 1;
          continue;
        }
        else if (_cchar == 13)
        {
        }
        else if (_cchar == 10)
          break;
        else
        {
          if (_strlen <= 127)
          {
            if (_ignore != 1)
            {
              _string[_stralen] = _cchar;
              _stralen++;
            }
          }
          _strlen++;
        }
      }

      if (i == 0)
        _eofile = 1;

      if (_stralen > 1)
      {
        _string[_stralen] = '\0';
        //; search for ='s
        i = 0;
        j = 0;
        do
        {
          if (_string[i] == '=')
            j++;
          i++;
        }
        while ((unsigned)i < _stralen);

        if (j == 1)
        {
          i = 0;
          j = 0;
          _strlena = 0;
          _strlenb = 0;
          while ((unsigned)i < _stralen)
          {
            if (_string[i] == '=')
            {
              i++;
              if ((unsigned)i == _stralen)
                continue;
              j = 0;
              _usespace = 0;
              while ((unsigned)i < _stralen)
              {
                temp = _string[i];
                if (_usespace || temp != ' ')
                {
#ifndef __UNIXSDL__
                  if (temp >= 'a' && temp <= 'z')
                    temp -= ('a' - 'A');
#endif
                  _usespace = 1;
                  _stringb[j] = temp;
                  _strlenb++;
                  j++;
                }
                i++;
              }
              _stringb[_strlenb] = '\0';
            }
            else
            {
              if (_string[i] != ' ')
              {
                temp = _string[i];
                if (_string[i] >= 'a' && _string[i] <= 'z')
                {
                  temp -= ('a' - 'A');
                }
                _stringa[j] = temp;
                _strlena++;
                j++;
              }

              i++;
            }
          }
          _stringa[_strlena] = '\0';
          no_save = 0;

          if (_strlena != 0 && _strlenb != 0)
          {
            if (_strlena == strlen(StringA))
            {
              if (!strcmp(StringA, _stringa))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb >= '0' && *_stringb <= '9')
                  {
                    if (_forceauto != 1)
                    {
                      frameskip = *_stringb - 48;//shouldn't this be 48??
                    }
                  }
                }
              }
            }

            if (_strlena == strlen(StringB))
            {
              if (!strcmp(StringB, _stringa))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    _forceauto = 1;
                    frameskip = 0;
                  }
                }
              }
            }

            if (_strlena == strlen(StringC))
            {
              if (!strcmp(_stringa, StringC))
              {
                if (_strlenb == 2)
                {
                  if (*_stringb == '1')
                  {
                    if (_stringb[1] >= '0' && _stringb[1] <= '8')
                    {
                      pl1contrl = _stringb[1] - 38;
                    }
                  }
                }
                else if (_strlenb == 1)
                {
                  if (*_stringb >= '0' && *_stringb <= '9')
                  {
                    pl1contrl = *_stringb - 48;
                  }
                }
              }
            }

            if (_strlena == strlen(StringD))
            {
              if (!strcmp(_stringa, StringD))
              {
                if (_strlenb == 2)
                {
                  if (*_stringb == '1')
                  {
                    if (_stringb[1] >= '0' && _stringb[1] <= '8')
                    {
                      pl2contrl = _stringb[1] - 38;
                    }
                  }
                }
                if (_strlenb == 1)
                {
                  if (*_stringb >= '0' && *_stringb <= '9')
                  {
                    pl2contrl = *_stringb - 48;
                  }
                }
              }
            }

            if (_strlena == strlen(StringE))
            {
              if (!strcmp(_stringa, StringE))
              {
                i = 0;
                j = 0;
                button_select = 0;
                while (_strlenb > 0)
                {
                  if (_stringb[i] != ',')
                  {
                    if (_stringb[i] < '0' || _stringb[i] > '9')
                    {
                      i++;
                      _strlenb--;
                      continue;
                    }
                    j *= 10;
                    j += ((_stringb[i] - 48) & 0xFF);
                  }
                  if (_stringb[i] == ',')
                  {
                    // In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
                    if (button_select == 0)
                      pl1rightk = j;

                    if (button_select == 1)
                      pl1leftk = j;

                    if (button_select == 2)
                      pl1downk = j;

                    if (button_select == 3)
                      pl1upk = j;

                    if (button_select == 4)
                      pl1startk = j;

                    if (button_select == 5)
                      pl1selk = j;

                    if (button_select == 6)
                      pl1Bk = j;

                    if (button_select == 7)
                      pl1Yk = j;

                    if (button_select == 8)
                      pl1Ak = j;

                    if (button_select == 9)
                      pl1Xk = j;

                    if (button_select == 10)
                      pl1Lk = j;

                    if (button_select == 11)
                      pl1Rk = j;

                    j = 0;
                    button_select++;
                  }
                  i++;
                  _strlenb--;
                }
                if (_strlenb == 0)
                {
                  // In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
                  if (button_select == 0)
                    pl1rightk = j;

                  if (button_select == 1)
                    pl1leftk = j;

                  if (button_select == 2)
                    pl1downk = j;

                  if (button_select == 3)
                    pl1upk = j;

                  if (button_select == 4)
                    pl1startk = j;

                  if (button_select == 5)
                    pl1selk = j;

                  if (button_select == 6)
                    pl1Bk = j;

                  if (button_select == 7)
                    pl1Yk = j;

                  if (button_select == 8)
                    pl1Ak = j;

                  if (button_select == 9)
                    pl1Xk = j;

                  if (button_select == 10)
                    pl1Lk = j;

                  if (button_select == 11)
                    pl1Rk = j;
                }
              }
            }

            if (_strlena == strlen(StringF))
            {
              if (!strcmp(_stringa, StringF))
              {
                i = 0;
                j = 0;
                button_select = 0;
                while (_strlenb != 0)
                {
                  if (_stringb[i] != ',')
                  {
                    if (_stringb[i] < '0' || _stringb[i] > '9')
                    {
                      i++;
                      _strlenb--;
                      continue;
                    }
                    j *= 10;
                    j += ((_stringb[i] - 48) & 0xFF);
                  }
                  if (_stringb[i] == ',')
                  {
                    // In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
                    if (button_select == 0)
                      pl2rightk = j;

                    if (button_select == 1)
                      pl2leftk = j;

                    if (button_select == 2)
                      pl2downk = j;

                    if (button_select == 3)
                      pl2upk = j;

                    if (button_select == 4)
                      pl2startk = j;

                    if (button_select == 5)
                      pl2selk = j;

                    if (button_select == 6)
                      pl2Bk = j;

                    if (button_select == 7)
                      pl2Yk = j;

                    if (button_select == 8)
                      pl2Ak = j;

                    if (button_select == 9)
                      pl2Xk = j;

                    if (button_select == 10)
                      pl2Lk = j;

                    if (button_select == 11)
                      pl2Rk = j;

                    j = 0;
                    button_select++;
                  }
                  i++;
                  _strlenb--;
                }
                if (_strlenb == 0)
                {
                  // In order of Right, Left, Down, Up, Start, Select, A, B, X, Y, L, R
                  if (button_select == 0)
                    pl2rightk = j;

                  if (button_select == 1)
                    pl2leftk = j;

                  if (button_select == 2)
                    pl2downk = j;

                  if (button_select == 3)
                    pl2upk = j;

                  if (button_select == 4)
                    pl2startk = j;

                  if (button_select == 5)
                    pl2selk = j;

                  if (button_select == 6)
                    pl2Bk = j;

                  if (button_select == 7)
                    pl2Yk = j;

                  if (button_select == 8)
                    pl2Ak = j;

                  if (button_select == 9)
                    pl2Xk = j;

                  if (button_select == 10)
                    pl2Lk = j;

                  if (button_select == 11)
                    pl2Rk = j;

                  j = 0;
                  button_select++;
                }
              }
            }

            if (_strlena == strlen(StringG))
            {
              if (!strcmp(_stringa, StringG))
              {
                if (_strlenb == 1)
                {
                  spcon = ASCIIChar2Bool(*_stringb); // SPC Enabled?
                  soundon = ASCIIChar2Bool(*_stringb);
                  cfgsoundon = ASCIIChar2Bool(*_stringb);
                }
              }
            }

            if (_strlena == strlen(StringH))
            {
              if (!strcmp(_stringa, StringH))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb >= '0' && *_stringb <= '6')
                  {
                    SoundQuality = *_stringb - 48;
                    cfgSoundQuality = *_stringb - 48;
                  }
                }
              }
            }

            if (_strlena == strlen(StringI))
            {
              if (!strcmp(_stringa, StringI))
              {
                if (_strlenb == 2)
                {
                  if (*_stringb == '1')
                  {
                    cvidmode = _stringb[1] - 38;
                    cfgcvidmode = _stringb[1] - 38;
                  }
                  if (*_stringb == '2')
                  {
                    cvidmode = _stringb[1] - 28;
                    cfgcvidmode = _stringb[1] - 28;
                  }
                  if (*_stringb == '3')
                  {
                    cvidmode = _stringb[1] - 18;
                    cfgcvidmode = _stringb[1] - 18;
                  }
                }
                else if (_strlenb == 1)
                {
                  if (*_stringb >= '0' && *_stringb <= '9')
                  {
                    cvidmode = *_stringb - 48;
                    cfgcvidmode = *_stringb - 48;
                  }
                }
              }
            }

            if (_strlena == strlen(StringJ))
            {
              if (!strcmp(_stringa, StringJ))
              {
                if (_strlenb != 0)
                {
                  i = 0;
                  _per2exec = 0;
                  do
                  {
                    if (_per2exec >= 100)
                    {
                      no_save = 1;
                      break; //needs to go pasr setting the vars.
                    }
                    _per2exec *= 10;
                    if (_stringb[i] >= '0' && _stringb[i] <= '9')
                    {
                      _per2exec += (_stringb[i] - 48);
                    }
                    else
                    {
                      no_save = 1;
                      break; //needs to go pasr setting the vars.
                    }
                    i++;
                  }
                  while ((unsigned)i < _strlenb);
                  if ((_per2exec<150 && _per2exec>50) && !no_save)
                  {
                    per2exec = _per2exec;
                    cfgper2exec = _per2exec;
                  }
                }
              }
            }
            if (_strlena == strlen(StringM))
            {
              if (!strcmp(StringM, _stringa))
              {
                if (_strlenb == 1)
                {
                  StereoSound = ASCIIChar2Bool(*_stringb);
                  cfgStereoSound = ASCIIChar2Bool(*_stringb);
                }
              }
            }

            if (_strlena == strlen(StringN))
            {
              if (!strcmp(StringN, _stringa))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    guioff = 1;
                    cfgguioff = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringO))
            {
              if (!strcmp(StringO, _stringa))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    scanlines = 1;
                    cfgscanline = 1;
                  }
                  if (*_stringb == '2')
                  {
                    scanlines = 2;
                    cfgscanline = 2;
                  }
                  if (*_stringb == '3')
                  {
                    scanlines = 3;
                    cfgscanline = 3;
                  }
                }
              }
            }

            if (_strlena == strlen(StringP))
            {
              if (!strcmp(_stringa, StringP))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    antienab = 1;
                    cfginterp = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringQ))
            {
              if (!strcmp(_stringa, StringQ))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    enterpress = 1;
                    cfgenterskip = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringR))
            {
              if (!strcmp(_stringa, StringR))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    Force8b = 1;
                    cfgforce8b = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringS))
            {
              if (!strcmp(_stringa, StringS))
              {
                if (_strlenb >= 1)
                {
                  cfgloadsdir = 1;
                  strncpy(SRAMDir, _stringb, _strlenb);
                  SRAMDir[_strlenb]='\0';
                }
              }
            }

            if (_strlena == strlen(StringT))
            {
              if (!strcmp(_stringa, StringT))
              {
                if (_strlenb >= 3)
                {
#ifndef __UNIXSDL__
                  if (_stringb[1] == ':' && _stringb[2] == '\\')
                  {
                    cfgloadgdir = 1;
                    *LoadDrive = (char) (_stringb[0] - 65);
                    LoadDriveB[0] = _stringb[0] - 65;
                    strncpy(LoadDir, _stringb + 3, _strlenb - 3);
                    *(LoadDir + (_strlenb - 3)) = '\0';

#else
                    cfgloadgdir = 1;
                    strncpy(LoadDir, _stringb, _strlenb);
                    LoadDir[_strlenb] = '\0';

#endif
                    strcpy(LoadDirB, LoadDir);
#ifndef __UNIXSDL__
                  }
#endif
                }
              }
            }

            if (_strlena == strlen(StringU))
            {
              if (!strcmp(_stringa, StringU))
              {
                JoyStatRead = 1;
                i = 0;
                button_select = 0;
                j = 0;
                while (_strlenb != 0)
                {
                  if (_stringb[i] != ',')
                  {
                    if (_stringb[i] < '0' || _stringb[i] > '9')
                    {
                      i++;
                      _strlenb--;
                      continue;
                    }
                    j *= 10;
                    j += (_stringb[i] - 48);
                  }
                  if (_strlenb == 0 || _stringb[i] == ',')
                  {
                    // In order of Start, Select, B, Y, A, X, L, R
                    if (button_select == 0)
                      JoyStart = j;

                    if (button_select == 1)
                      JoySelec = j;

                    if (button_select == 2)
                      JoyBC = j;

                    if (button_select == 3)
                      JoyYC = j;

                    if (button_select == 4)
                      JoyAC = j;

                    if (button_select == 5)
                      JoyXC = j;

                    if (button_select == 6)
                      JoyLC = j;

                    if (button_select == 7)
                      JoyRC = j;

                    j = 0;
                    button_select++;
                  }
                  i++;
                  _strlenb--;
                }
              }
            }

            if (_strlena == strlen(StringV))
            {
              if (!strcmp(_stringa, StringV))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    newengen = 1;
                    cfgnewgfx = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringW))
            {
              if (!strcmp(_stringa, StringW))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                  {
                    vsyncon = 1;
                    cfgvsync = 1;
                  }
                }
              }
            }

            if (_strlena == strlen(StringX))
            {
              if (!strcmp(_stringa, StringX))
              {
                if (_strlenb != 0)
                {
                  i = 0;
                  _volume = 0;
                  do
                  {
                    if (_volume >= 100)
                    {
                      no_save = 1;
                      break;  //needs to go pasr setting the vars.
                    }
                    _volume *= 10;
                    if (_stringb[i] >= '0' && _stringb[i] <= '9')
                      _volume += (_stringb[i] - 48);
                    else
                    {
                      no_save = 1;
                      break; //needs to go pasr setting the vars.
                    }
                    i++;
                  }
                  while ((unsigned)i < _strlenb);
                  if (_volume <= 100 && !no_save)
                  {
                    MusicRelVol = _volume;
                    cfgvolume = _volume;
                  }
                }
              }
            }

            if (_strlena == strlen(StringY))
            {
              if (!strcmp(_stringa, StringY))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                    cfgecho = 1;
                }
              }
            }

            if (_strlena == strlen(StringZ))
            {
              if (!strcmp(_stringa, StringZ))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                    RevStereo = 1;
                }
              }
            }

            if (_strlena == strlen(String1))
            {
              if (!strcmp(String1, _stringa))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                    pl12s34 = 1;
                }
              }
            }

            if (_strlena == strlen(String2))
            {
              if (!strcmp(_stringa, String2))
              {
                if (_strlenb == 1)
                {
                  if (*_stringb == '1')
                    cfgdontsave = 1;
                }
              }
            }

            if (_strlena == strlen(String3))
            {
              if (!strcmp(_stringa, String3))
              {
                if (_strlenb == 0)
                {
                  i = 0;
                  _per2exec = 0;
                  do
                  {
                    if (_per2exec >= 100)
                    {
                      no_save = 1;
                      break; //should go past the setting of cfgreinit
                    }
                    _per2exec *= 10;
                    if (_stringb[i] >= '0' && _stringb[i] <= '9')
                    {
                      _per2exec += (_stringb[i] - 48);
                    }
                    else
                    {
                      no_save = 1;
                      break;
                    }
                    i++;
                  }
                  while ((unsigned)i < _strlenb);
                  if ((_per2exec<150 && _per2exec>5) && !no_save)//shouldn't this be 50?
                    cfgreinittime = _per2exec;
                }
              }
            }
          }
        }
      }
    }
    while (_eofile == 0);
    ZCloseFileHandle = _fileloc;
    ZCloseFile();
  }
  else
  {
    DOScreatenewcfg();
  }
}

unsigned char SRAMChdirFail = 0;

void SRAMChdir()
{
  if (!chdir(SRAMDir))
  {
    SRAMChdirFail = 0;
  }
  else
  {
    SRAMChdirFail = 1;
  }
}

void SRAMDirCurDir()
{
  getcwd(SRAMDir,1024);
}

void UpChdir()
{
  chdir("..");
}
