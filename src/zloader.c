/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#ifdef __LINUX__
#include "gblhdr.h"
#define DIR_SLASH '/'
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#define DIR_SLASH '\\'
#ifdef __WIN32__
#include <windows.h>
#endif
#endif

#ifndef __MSDOS__
extern unsigned char NetChatFirst, NetServer, NetNewNick, NetFilename[512], CmdLineTCPIPAddress,
                     NetQuitAfter, UDPConfig, CmdLineNetPlay;
#endif

#ifdef __WIN32__
void ImportDirectX();

extern unsigned char KitchenSync, Force60hz;

#endif

extern unsigned char Palette0, pl1contrl, pl2contrl, MMXSupport, Force8b, ForcePal, GUIClick,
                     MouseDis, MusicRelVol, ScreenScale, SoundQuality, StereoSound, V8Mode,
                     antienab, cvidmode, debugdisble, debugger, enterpress, vsyncon, DisplayS,
                     fname, SnowOn, Triplebufen, SPC700sh, OffBy1Line, DSPDisable, frameskip,
                     gammalevel, guioff, romtype, per2exec, scanlines, soundon, spcon,
                     showallext, autoloadstate, smallscreenon;

void ConvertJoyMap1(), ConvertJoyMap2(), zstart(), makeextension();

static void display_help()
{
  puts("Usage : zsnes [-d,-f #, ... ] <filename.sfc>");
  puts("   Eg : zsnes -s -r 2 game.sfc");
  puts("");
  puts("  -0      Disable Color 0 modification in 8-bit modes");
  puts("  -1 #/-2 #   Select Player 1/2 Input :");
  puts("                0 = None       1 = Keyboard   2 = Joystick   3 = Gamepad");
  puts("                4 = 4Button    5 = 6Button    6 = Sidewinder   ");
  puts("  -3      Enable triple buffering (disables vsync)");
#ifdef __WIN32__
  puts("  -6      Force 60Hz refresh rate");
#endif
  puts("  -7      Disable SPC700 speedhack");
  puts("  -8      Force 8-bit sound");
  puts("  -9      Off by 1 line fix");
  puts("  -c      Enable full/wide screen (when available)");
  puts("  -cc     Enable small screen (when available)");
#ifdef __MSDOS__
  puts("  -d      Start with debugger enabled");
#endif
  puts("  -dd     Disable sound DSP emulation");
#ifdef __MSDOS__
  puts("  -e      Skip enter key press at the beginning");
#endif
  puts("  -f #    Enable fixed frame rate [0...9]");
  getchar();
#ifdef __MSDOS__
  puts("  -g #    Specify gamma correction value [0...15]");
  puts("          (Only works properly in 8-bit modes)");
#endif
  puts("  -h      Force HiROM");
  puts("  -j      Disable Mouse (Automatically turns off right mouse click)");
  puts("  -k #    Set Volume Level (0 .. 100)");
#ifdef __WIN32__
  puts("  -ks     Enable the KitchenSync");
#endif
  puts("  -l      Force LoROM");
  puts("  -m      Disable GUI (Must specify ROM filename)");
  puts("  -n #    Enable scanlines (when available)");
  puts("          Where # is: 1 = full, 2 = 25%, 3 = 50%");
  puts("  -om     Enable MMX support (when available)");
  puts("  -p #    Percentage of instructions to execute [50..120]");
  puts("  -r #    Set Sampling Sound Blaster Sampling Rate & Bit :");
  puts("             0 = 8000Hz  1 = 11025Hz 2 = 22050Hz 3 = 44100Hz");
  puts("             4 = 16000Hz 5 = 32000Hz 6 = 48000Hz");
  puts("  -s      Enable SPC700/DSP emulation (Sound)");
  puts("  -sa     Show all extensions in GUI (*.*)");
  puts("  -sn     Enable Snowy GUI Background");
  puts("  -t      Force NTSC timing");
  puts("  -u      Force PAL timing");
  getchar();
  puts("  -v #    Select Video Mode :");
#ifdef __WIN32__
#define VIDEO_MODE_COUNT 32
  puts("          0 = 256x224   R WIN       1 = 256x224   R FULL");
  puts("          2 = 512x448   R WIN       3 = 512x448   DR WIN");
  puts("          4 = 640x480   S WIN       5 = 640x480   DS WIN");
  puts("          6 = 640x480   DR FULL     7 = 640x480   DS FULL");
  puts("          8 = 640x480   S FULL      9 = 768x672   R WIN");
  puts("         10 = 768x672   DR WIN     11 = 800x600   S WIN");
  puts("         12 = 800x600   DS WIN     13 = 800x600   S FULL");
  puts("         14 = 800x600   DR FULL    15 = 800x600   DS FULL");
  puts("         16 = 1024x768  S WIN      17 = 1024x768  DS WIN");
  puts("         18 = 1024x768  S FULL     19 = 1024x768  DR FULL");
  puts("         20 = 1024x768  DS FULL    21 = 1024x896  R WIN");
  puts("         22 = 1024x896  DR WIN     23 = 1280x960  S WIN");
  puts("         24 = 1280x960  DS WIN     25 = 1280x960  S FULL");
  puts("         26 = 1280x960  DR FULL    27 = 1280x960  DS FULL");
  puts("         28 = 1280x1024 S WIN      29 = 1280x1024 DS WIN");
  puts("         30 = 1280x1024 S FULL     31 = 1280x1024 DR FULL");
  puts("         32 = 1280x1024 DS FULL");
#endif
#ifdef __LINUX__
  puts("          0 = 256x224   R WIN        1 = 256x224  R FULL");
  puts("          2 = 512x448   DR WIN       3 = 640x480  DS FULL");
#ifndef __OPENGL__
#define VIDEO_MODE_COUNT 3
#else
#define VIDEO_MODE_COUNT 18
  puts("          4 = 256x224   OR  WIN      5 = 512x448   ODR WIN");
  puts("          6 = 640x480   ODS FULL     7 = 640x480   ODS WIN");
  puts("          8 = 640x576   ODR WIN      9 = 768x672   ODR WIN");
  puts("         10 = 800x600   ODS FULL    11 = 800x600   ODS WIN");
  puts("         12 = 896x784   ODR WIN     13 = 1024x768  ODS FULL");
  puts("         14 = 1024x768  ODS WIN     15 = 1024x896  ODR WIN");
  puts("         16 = 1280x1024 ODS FULL    17 = 1600x1200 ODR FULL");
  puts("         18 = VARIABLE  ODS WIN");
#endif
#endif
#ifdef __MSDOS__
#define VIDEO_MODE_COUNT 18
  puts("          0 = 256x224x8B  (MODEQ)  1 = 256x240x8B (MODEQ)");
  puts("          2 = 256x256x8B  (MODEQ)  3 = 320x224x8B (MODEX)");
  puts("          4 = 320x240x8B  (MODEX)  5 = 320x256x8B (MODEX)");
  puts("          6 = 640x480x16B (VESA1)  7 = 320x240x8B (VESA2)");
  puts("          8 = 320x240x16B (VESA2)  9 = 320x480x8B (VESA2)");
  puts("         10 = 320x480x16B (VESA2) 11 = 512x384x8B (VESA2)");
  puts("         12 = 512x384x16B (VESA2) 13 = 640x400x8B (VESA2)");
  puts("         14 = 640x400x16B (VESA2) 15 = 640x480x8B (VESA2)");
  puts("         16 = 640x480x16B (VESA2) 17 = 800x600x8B (VESA2)");
  puts("         18 = 800x600x16B (VESA2)");
#endif
  getchar();
  puts("  -w      Enable vsync (disables triple buffering)");
  puts("  -y      Enable Anti-Aliasing");
  puts("  -z      Disable Stereo Sound");
  puts("  -zs #   Auto load specified save state slot on startup ");
  puts("");
  puts("  File Formats Supported by GUI : SMC,SFC,SWC,FIG,MGD,UFO,BIN,");
  puts("                                  058,078,1,USA,EUR,JAP,ZIP,JMA");
  puts("");
#ifndef __LINUX__
  puts("  Microsoft-style options (/option) are also accepted");
#endif
  exit(1);
}

static size_t zatoi(const char *str)
{
  const char *orig_str = str;
  while (*str)
  {
    if (!isdigit(*str++)) { return(~0); }
  }
  return((size_t)atoi(orig_str));
}

static void handle_params(int argc, char *argv[])
{
  int i;

  #ifndef __MSDOS__
  NetServer=0;
  NetChatFirst=0;
  NetQuitAfter=0;

  if (argc >= 5 && argv[1][0] == '/' && strlen(argv[1]) == 6)
  {
    size_t i = 0, j = 0;
    char *strp, *p;

    if (toupper(argv[1][1]) == 'T') UDPConfig=0;
    if (toupper(argv[1][2]) == 'S') NetServer=1;
    if (toupper(argv[1][2]) == 'C') NetServer=2;
    if (toupper(argv[1][3]) == 'C') NetChatFirst=0;
    if (toupper(argv[1][3]) == 'L') NetChatFirst=1;
    if (toupper(argv[1][4]) == 'N') NetQuitAfter=0;
    if (toupper(argv[1][4]) == 'Q') NetQuitAfter=1;
    //Next should be # of connections


    strp = &NetNewNick;

    while (argv[2][i]!=0)
    {
      switch (argv[2][i])
      {
        case '_':
        case '-':
        case '^':
        case '=':
        case '+':
        case '[':
        case ']':
        if ( j < 10)
        {
          strp[j] = argv[2][i];
          j++;
        }
        break;

        default:
          if (((toupper(argv[2][i]) >= 'A') && (toupper(argv[2][i]) <= 'Z')) ||
              ((argv[2][i] >= '0') && (argv[2][i] <= '9')))
          {
            if (j < 10)
            {
              strp[j] = argv[2][i];
              j++;
            }
          }
          break;
      }
      i++;
    }
    strp[j] = 0;

    strncpy(NetFilename, argv[3], 512);
    NetFilename[511] = 0;
    if (NetServer == 2)
    {
      if (argc < 5)
      {
        NetServer = 0;
      }
      else
      {
        CmdLineNetPlay = 1;
        strp = &CmdLineTCPIPAddress;
        strncpy(strp, argv[4], 28);
      }
    }

    for (p = NetFilename+strlen(NetFilename)-1; (p > (char *)NetFilename) && (*p != DIR_SLASH); p--);
    *p = 0;
    chdir(NetFilename);
    *p = DIR_SLASH;
  }
  #endif

  for (i = 1; i < argc; i++)
  {
    #ifndef __LINUX__
    if (argv[i][0] == '-' || argv[i][0] == '/')
    #else
    if (argv[i][0] == '-')
    #endif
    {
      if (!argv[i][1]) //Nothing but a - or /
      {
        display_help();
      }
      else if (!argv[i][2]) //- followed by a single letter
      {
        switch (tolower(argv[i][1]))
        {
          case '0': //Palette 0 disable
            Palette0 = 1;
            break;

          case '1': //Player 1 Input
            i++;
            if ((pl1contrl = zatoi(argv[i])) > 6)
            {
              puts("Player Input must be a value from 0 to 6!");
              exit(1);
            }
            ConvertJoyMap1();
            break;

          case '2': //Player 2 Input
            i++;
            if ((pl2contrl = zatoi(argv[i])) > 6)
            {
              puts("Player Input must be a value from 0 to 6!");
              exit(1);
            }
            ConvertJoyMap2();
            break;

          case '3': //Enable triple buffering
            vsyncon = 0;
            Triplebufen = 1;
            break;

          #ifdef __WIN32__
          case '6': //Force 60Hz
            Force60hz = 1;
            break;
          #endif

          case '7': //SPC700 speed hack disable
            SPC700sh = 1;
            break;

          case '8': //Force 8-bit sound
            Force8b = 1;
            break;

          case '9': //Off by 1 line
            OffBy1Line = 1;
            break;

          case 'c': //Enable full screen (when available)
            ScreenScale = 1;
            break;

          case 'd': //Start with debugger enabled
            debugger = 1;
            debugdisble = 0;
            break;

          case 'e': //Skip enter key press at the beginning
            enterpress = 1;
            break;

          case 'f': //Enable fixed frame rate
            i++;
            if ((frameskip = zatoi(argv[i])+1) > 10)
            {
              puts("Frame Skip must be a value of 0 to 9!");
              exit(1);
            }
            break;

          case 'g': //Specify gamma correction value
            i++;
            if ((gammalevel = zatoi(argv[i])) > 15)
            {
              puts("Gamma Correction Level must be a value of 0 to 15!");
              exit(1);
            }
            break;

          case 'h': //Force HiROM
            romtype = 2;
            break;

          case 'j': //Disable Mouse
            GUIClick = 0;
            MouseDis = 1;
            break;

          case 'k': //Set Volume Level
            i++;
            if ((MusicRelVol = zatoi(argv[i])) > 100)
            {
              puts("Volume must be a value from 0 to 100!");
              exit(1);
            }
            break;

          case 'l': //Force LoROM
            romtype = 1;
            break;

          case 'm': //Disables GUI
            guioff = 1;
            break;

          case 'n': //Enable scanlines (when available)
            i++;
            if ((scanlines = zatoi(argv[i])) > 3)
            {
              puts("Scanlines must be a value 1 to 3!");
              exit(1);
            }
            break;

          case 'o': //Enable MMX support
            MMXSupport = 0;
            break;

          case 'p': //Percentage of instructions to execute
            per2exec = zatoi(argv[i]);
            if (per2exec > 150 || per2exec < 50)
            {
              puts("Percentage of instructions to execute must be a value from 50 to 150!");
              exit(1);
            }
            break;

          case 'r': //Set sampling rate
            i++;
            if ((SoundQuality = zatoi(argv[i])) > 6)
            {
              puts("Sound Sampling Rate must be a value of 0 to 6!");
              exit(1);
            }
            break;

          case 's': //Enable SPC700/DSP emulation
            spcon = 1;
            soundon = 1;
            break;

          case 't': //Force NTSC
            ForcePal = 1;
            break;

          case 'u': //Force Pal
            ForcePal = 2;
            break;

          case 'v': //Select Video Mode
            i++;
            if ((cvidmode = zatoi(argv[i])) > VIDEO_MODE_COUNT)
            {
              puts("Invalid Video Mode!");
              exit(1);
            }
            break;

          case 'w': //Enable vsync
            Triplebufen = 0;
            vsyncon = 1;
            break;

          case 'y': //Enable anti-aliasing
            antienab = 1;
            break;

          case 'z': //Disable stereo sound
            StereoSound = 0;
            break;

          default:
            display_help();
            break;
        }
      }
      else if (!argv[i][3]) //- followed by a two letters
      {
        if (tolower(argv[i][1]) == 'c' && tolower(argv[i][2]) == 'c') //Enable small screen
        {
          smallscreenon = 1;
        }

        else if (tolower(argv[i][1]) == 'd' && tolower(argv[i][2]) == 'd') //Disable sound DSP emulation
        {
          DSPDisable = 1;
        }

        #ifdef __WIN32__
        else if (tolower(argv[i][1]) == 'k' && tolower(argv[i][2]) == 's') //Enable KitchenSync
        {
          KitchenSync = 1;
        }
        #endif

        else if (tolower(argv[i][1]) == 'o' && tolower(argv[i][2]) == 'm') //Enable MMX support
        {
          MMXSupport = 1;
        }

        else if (tolower(argv[i][1]) == 's' && tolower(argv[i][2]) == 'p') //Display sound information
        {
          DisplayS = 1;
        }

        else if (tolower(argv[i][1]) == 's' && tolower(argv[i][2]) == 'a') //Show all extensions in GUI
        {
          showallext = 1;
        }

        else if (tolower(argv[i][1]) == 's' && tolower(argv[i][2]) == 'n') //Enable Snowy GUI Background
        {
          SnowOn = 1;
        }

        else if (tolower(argv[i][1]) == 'v' && argv[i][2] == '8') //V8 Mode
        {
          V8Mode = 1;
        }

        else if (tolower(argv[i][1]) == 'z' && argv[i][2] == 's') //Autoload save state
        {
          i++;
          if ((autoloadstate = zatoi(argv[i])+1) > 10)
          {
            puts("State load position must be a value of 0 to 9!");
            exit(1);
          }
        }

        else
        {
          display_help();
          break;
        }
      }
      else //- followed by more than 2 letters
      {
        display_help();
      }
    }
    else //Param with no - or / prefix
    {
      char *fvar = &fname;
      fvar[0] = strlen(argv[i]);
      strncpy(&fvar[1],argv[i],127);
      makeextension();
      break;
    }
  }
}

int argc;
char **argv;

void ccmdline()
{
  handle_params(argc, argv);
}

#ifdef __WIN32__
extern HINSTANCE hInst;
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
  argc = __argc;
  argv = __argv;

  hInst=hInstance;
  ImportDirectX();

  zstart();
  return(0);
}

#else

int main(int zargc, char *zargv[])
{
  argc = zargc;
  argv = zargv;

  #ifdef __LINUX__
  handle_params(zargc, zargv);
  #endif

  zstart();
  return(0);
}
#endif
