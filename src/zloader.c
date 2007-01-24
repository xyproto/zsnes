/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

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
#ifdef __LIBAO__
#include <ao/ao.h>
#endif
#else
#define _POSIX_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#ifdef __WIN32__
#include <windows.h>
#include <direct.h>
#undef _POSIX_
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#include "asm_call.h"
#include "cfg.h"
#include "input.h"
#include "zpath.h"
#include "zloader.h"

#ifdef __WIN32__
void ImportDirectX();
#endif


extern unsigned char romtype, MouseDis, ZMVZClose, ZMVRawDump, debugger, debugdisble;
extern unsigned char gammalevel, spcon, ForcePal, DSPDisable, V8Mode;
extern unsigned char autoloadstate, autoloadmovie, MovieForcedLengthEnabled;
extern char *STCart2;
extern unsigned int NumInputDevices, MovieForcedLength;
void zstart();
#ifdef __WIN32__
void InitDebugger();
#endif

#define put_line(x)                          \
  if (tty && (lines_out == 22))              \
  {                                          \
    puts("  -- Press Enter to Continue --"); \
    getchar();                               \
    lines_out = 0;                           \
  }                                          \
  puts(x);                                   \
  lines_out++;

static void display_help()
{
  size_t lines_out = 0;
  bool tty = isatty(fileno(stdout));
#ifdef __UNIXSDL__
#ifdef __LIBAO__
  int driver_count;
  ao_info **driver_info;
#endif
  char line[75];
#endif

  put_line("Usage : zsnes [-d,-f #, ... ] <filename.sfc>");
  put_line("   Eg : zsnes -s -r 2 game.sfc");
  put_line("");
#ifdef __MSDOS__
  put_line("  -0      Force black blackground in 8-bit modes");
#endif
  put_line("  -1 #/-2 #   Select Player 1/2 Input :");
#ifdef __MSDOS__
  put_line("                0 = None             1 = Keyboard         2 = 2 button pad");
  put_line("                3 = 4 button pad     4 = 6 button pad     5 = 8 button pad");
  put_line("                6 = Sidewinder #1    7 = Sidewinder #2    8 = Sidewinder #3");
  put_line("                9 = Sidewinder #4   10 = Gamepad Pro #0  11 = Gamepad Pro #1");
  put_line("               12 = LPT1 #1         13 = LPT1 #2         14 = LPT1 #3");
  put_line("               15 = LPT1 #4         16 = LPT1 #5");
#else
  put_line("                0 = None       1 = Keyboard/Gamepad");
#endif
#ifndef __UNIXSDL__
  put_line("  -3      Enable triple buffering (replaces vsync)");
#endif
#ifdef __WIN32__
  put_line("  -6 #    Force a user-specified refresh rate for fullscreen modes [50..180]");
#endif
#ifdef __UNIXSDL__
  put_line("  -ad <>  Select Audio Driver :");
  snprintf(line, sizeof(line), "%22s = Automatically select output", "auto");
  put_line(line);
#ifdef __LIBAO__
  driver_info = ao_driver_info_list(&driver_count);
  while (driver_count--)
  {
    if (driver_info[driver_count]->type == AO_TYPE_LIVE)
    {
      snprintf(line, sizeof(line), "%22s = %s", driver_info[driver_count]->short_name, driver_info[driver_count]->name);
      put_line(line);
    }
  }
#endif
  snprintf(line, sizeof(line), "%22s = Simple DirectMedia Layer output", "sdl");
  put_line(line);
#endif
#ifdef __MSDOS__
  put_line("  -8      Force 8-bit sound");
  put_line("  -c      Enable full/wide screen (when available)");
  put_line("  -cc     Enable small screen (when available)");
#endif
#ifndef NO_DEBUGGER
  put_line("  -d      Start with debugger enabled");
#endif
  put_line("  -dd     Disable sound SPC700/DSP emulation which also disables sound output");
  put_line("  -ds     Disable sound output");
  put_line("  -dh     Disable ROM-specific hacks");
  put_line("  -f #    Enable fixed frame rate [0..9]");
  put_line("  -g #    Specify gamma correction value [0..15]");
  put_line("  -h      Force HiROM");
  put_line("  -j      Disable mouse (Automatically turns off right mouse click)");
#ifndef __MSDOS__
  put_line("  -js #   Set joystick sensitivity [0..32767]");
#endif
  put_line("  -k #    Set volume level [0..100]");
#ifdef __WIN32__
  put_line("  -kp     Enable the KitchenSync for PAL only");
  put_line("  -ks     Enable the KitchenSync");
#endif
  put_line("  -l      Force LoROM");
  put_line("  -m      Disable GUI (Must specify ROM filename)");
  put_line("  -mc     Exit ZSNES when closing a movie (use with -zm)");
  put_line("  -md #   Dump Video (use with -zm)");
  put_line("             1 = Raw  2 = FFV1 3 = x264");
  put_line("             4 = XviD 5 = Custom");
  put_line("  -ml #   Define movie dump length in amount of frames (use with -md)");
  put_line("  -n #    Enable scanlines (when available)");
  put_line("             0 = None, 1 = Full, 2 = 25%, 3 = 50%");
  put_line("  -o      Disable MMX support");
  put_line("  -p #    Percentage of instructions to execute [50..150]");
  put_line("  -r #    Set sound sampling rate:");
  put_line("             0 = 8000Hz  1 = 11025Hz 2 = 22050Hz 3 = 44100Hz");
  put_line("             4 = 16000Hz 5 = 32000Hz 6 = 48000Hz");
  put_line("  -s      Enable sound output and enable SPC700/DSP emulation");
  put_line("  -sa     Show all files in GUI (*.*)");
#ifdef __MSDOS__
  put_line("  -sp     Display sound information");
#endif
  put_line("  -t      Force NTSC timing");
  put_line("  -u      Force PAL timing");
  put_line("  -v #    Select video mode :");
#ifdef __WIN32__
#define VIDEO_MODE_COUNT 42
  put_line("             0 = 256x224      R WIN     1 = 256x224      R FULL");
  put_line("             2 = 512x448      R WIN     3 = 512x448     DR WIN");
  put_line("             4 = 640x480      S WIN     5 = 640x480     DS WIN");
  put_line("             6 = 640x480     DR FULL    7 = 640x480     DS FULL");
  put_line("             8 = 640x480      S FULL    9 = 768x672      R WIN");
  put_line("            10 = 768x672     DR WIN    11 = 800x600      S WIN");
  put_line("            12 = 800x600     DS WIN    13 = 800x600      S FULL");
  put_line("            14 = 800x600     DR FULL   15 = 800x600     DS FULL");
  put_line("            16 = 1024x768     S WIN    17 = 1024x768    DS WIN");
  put_line("            18 = 1024x768     S FULL   19 = 1024x768    DR FULL");
  put_line("            20 = 1024x768    DS FULL   21 = 1024x896     R WIN");
  put_line("            22 = 1024x896    DR WIN    23 = 1280x960     S WIN");
  put_line("            24 = 1280x960    DS WIN    25 = 1280x960     S FULL");
  put_line("            26 = 1280x960    DR FULL   27 = 1280x960    DS FULL");
  put_line("            28 = 1280x1024    S WIN    29 = 1280x1024   DS WIN");
  put_line("            30 = 1280x1024    S FULL   31 = 1280x1024   DR FULL");
  put_line("            32 = 1280x1024   DS FULL   33 = 1600x1200    S WIN");
  put_line("            34 = 1600x1200   DS WIN    35 = 1600x1200   DR FULL");
  put_line("            36 = 1600x1200   DS FULL   37 = 1600x1200    S FULL");
  put_line("            38 = CUSTOM      D  WIN    39 = CUSTOM      DS FULL");
  put_line("            40 = CUSTOM         WIN    41 = CUSTOM       S FULL");
  put_line("            42 = CUSTOM      DR FULL");
#endif
#ifdef __UNIXSDL__
  put_line("             0 = 256x224      R WIN     1 = 256x224      R FULL");
  put_line("             2 = 512x448     DR WIN     3 = 512x448     DR FULL");
  put_line("             4 = 640x480     DR FULL");
#ifndef __OPENGL__
#define VIDEO_MODE_COUNT 4
#else
#define VIDEO_MODE_COUNT 22
  put_line("             5 = 256x224    O R WIN     6 = 512x448    ODR WIN");
  put_line("             7 = 640x480    ODS FULL    8 = 640x480    ODS WIN");
  put_line("             9 = 640x560    ODR WIN    10 = 768x672    ODR WIN");
  put_line("            11 = 800x600    ODS FULL   12 = 800x600    ODS WIN");
  put_line("            13 = 896x784    ODR WIN    14 = 1024x768   ODS FULL");
  put_line("            15 = 1024x768   ODS WIN    16 = 1024x896   ODR WIN");
  put_line("            17 = 1280x960   ODS FULL   18 = 1280x1024  ODS FULL");
  put_line("            19 = 1600x1200  ODS FULL   20 = VARIABLE   ODR WIN");
  put_line("            21 = VARIABLE   ODS WIN    22 = CUSTOM     OD  FULL");
#endif
#endif
#ifdef __MSDOS__
#define VIDEO_MODE_COUNT 18
  put_line("             0 = 256x224x8B   MODEQ     1 = 256x240x8B  MODEQ");
  put_line("             2 = 256x256x8B   MODEQ     3 = 320x224x8B  MODEX");
  put_line("             4 = 320x240x8B   MODEX     5 = 320x256x8B  MODEX");
  put_line("             6 = 640x480x16B  VESA1     7 = 320x240x8B  VESA2");
  put_line("             8 = 320x240x16B  VESA2     9 = 320x480x8B  VESA2");
  put_line("            10 = 320x480x16B  VESA2    11 = 512x384x8B  VESA2");
  put_line("            12 = 512x384x16B  VESA2    13 = 640x400x8B  VESA2");
  put_line("            14 = 640x400x16B  VESA2    15 = 640x480x8B  VESA2");
  put_line("            16 = 640x480x16B  VESA2    17 = 800x600x8B  VESA2");
  put_line("            18 = 800x600x16B  VESA2");
#endif
  put_line("  -v8     Grayscale mode");
#ifndef __UNIXSDL__
  put_line("  -w      Enable vsync (disables triple buffering)");
#endif
  put_line("  -y      Enable anti-aliasing (video interpolation)");
  put_line("  -z      Disable stereo sound");
  put_line("  -zm #   Auto load specified movie slot on startup [0..9]");
  put_line("  -zs #   Auto load specified save state slot on startup [0..99]");
  put_line("");
  put_line("  File Formats Supported by GUI : SMC,SFC,SWC,FIG,MGD,MGH,UFO,BIN,");
  put_line("                                  GD3,GD7,USA,EUR,JAP,AUS,ST,BS,");
  put_line("                                  DX2,048,058,078,1,A,GZ,ZIP,JMA");
  put_line("");
#ifndef __UNIXSDL__
  put_line("  Microsoft-style options (/option) are also accepted");
#endif
/*
#ifndef __MSDOS__
  put_line("               --Netplay Parameters--");
  put_line(" Commandline: /ABCDE <nickname> <fname> <IP Addy>");
  put_line("   nickname = user nickname");
  put_line("   fname = filename w/ full path (if L) or path name (if C)");
  put_line("   IP Addy = IP Address (Client Only)");
  put_line(" A = U (UDP - Recommended if works), T (TCP/IP)");
  put_line(" B = S (Server), C (Client)");
  put_line(" C = C (Chat first), L (load game first)");
  put_line(" D = N (Stay in ZSNES after disconnect), Q (Quit after disconnect)");
  put_line(" E = # of connections (Keep it 2 for now)");
#ifdef __WIN32__
  put_line("   eg: ZSNESW /UCCN2 nickname d:\\snesroms 202.36.124.28");
#else
  put_line("   eg: zsnes /UCCN2 nickname /home/zuser/snesroms 202.36.124.28");
#endif
#endif
*/

  exit(1);
}

#define ConvertJoyMapHelp(a,b) if (b && (a == b)) { b += 0x81; }

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

  bl = (pl1contrl == 3 || pl1contrl == 4) ? 4 : 0;
  if (pl1contrl == 5) { bl = 6; }

  if (bl)
  {
    // Convert button data
    pl1upk = 0xCC;
    pl1downk = 0xCD;
    pl1leftk = 0xCE;
    pl1rightk = 0xCF;
    ConvertJoyMapHelp(bl, pl1startk);
    ConvertJoyMapHelp(bl, pl1selk);
    ConvertJoyMapHelp(bl, pl1Yk);
    ConvertJoyMapHelp(bl, pl1Xk);
    ConvertJoyMapHelp(bl, pl1Bk);
    ConvertJoyMapHelp(bl, pl1Ak);
    ConvertJoyMapHelp(bl, pl1Lk);
    ConvertJoyMapHelp(bl, pl1Rk);
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

  bl = (pl2contrl == 3 || pl2contrl == 4) ? 4 : 0;
  if (pl2contrl == 5) { bl = 6; }

  if (bl)
  {
    //Convert button data
    pl2upk = 0xCC;
    pl2downk = 0xCD;
    pl2leftk = 0xCE;
    pl2rightk = 0xCF;
    ConvertJoyMapHelp(bl, pl2startk);
    ConvertJoyMapHelp(bl, pl2selk);
    ConvertJoyMapHelp(bl, pl2Yk);
    ConvertJoyMapHelp(bl, pl2Xk);
    ConvertJoyMapHelp(bl, pl2Bk);
    ConvertJoyMapHelp(bl, pl2Ak);
    ConvertJoyMapHelp(bl, pl2Lk);
    ConvertJoyMapHelp(bl, pl2Rk);
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
      pl2contrl = 7;
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
    }
  }
  return;
}

struct backup_cmdline_vars saved_cmdline_vars;

#ifdef __MSDOS__
#define BACKUP_HELP_DOS(func) \
  func(Palette0); \

#else
#define BACKUP_HELP_DOS(func)
#endif

#ifdef __WIN32__
#define BACKUP_HELP_WIN(func) \
  func(KitchenSync); \
  func(KitchenSyncPAL); \
  func(ForceRefreshRate); \
  func(SetRefreshRate); \
  func(joy_sensitivity); \

#else
#define BACKUP_HELP_WIN(func)
#endif

#ifdef __UNIXSDL__
#define BACKUP_HELP_SDL(func) \
func(joy_sensitivity); \

#else
#define BACKUP_HELP_SDL(func)
#endif


#define BACKUP_HELP(func) \
  func(guioff) \
  func(per2exec) \
  func(HacksDisable) \
  func(AllowMMX) \
  BACKUP_HELP_DOS(func) \
  BACKUP_HELP_WIN(func) \
  BACKUP_HELP_SDL(func)

#define BACKUP_VAR(var) saved_cmdline_vars._ ## var = var;
static void backup_all_vars()
{
  BACKUP_HELP(BACKUP_VAR)
}

#define SWAP_BACKUP_VAR(var) \
  saved_cmdline_vars._ ## var ^= var; \
  var ^= saved_cmdline_vars._ ## var; \
  saved_cmdline_vars._ ## var ^= var;
void swap_backup_vars()
{
  BACKUP_HELP(SWAP_BACKUP_VAR)
}

static size_t zatoi(const char *str)
{
  const char *orig_str = str;
  if (str)
  {
    while (*str)
    {
      if (!isdigit(*str++)) { return(~0); }
    }
    return((size_t)atoi(orig_str));
  }
  return(~0);
}

static void handle_params(int argc, char *argv[])
{
  int i;

  backup_all_vars();

  #ifndef __MSDOS__

  /*
  if (argc >= 5 && argv[1][0] == '/' && strlen(argv[1]) == 6)
  {
    size_t i = 0, j = 0;
    char *strp;

    if (toupper(argv[1][1]) == 'T') UDPConfig=0;

    //Next should be # of connections

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

  }
  */
  #endif

  for (i = 1; i < argc; i++)
  {
    #ifndef __UNIXSDL__
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
          #ifdef __MSDOS__
          case '0': //Palette color 0 disable
            Palette0 = 1;
            break;
          #endif

          case '1': //Player 1 Input
            i++;

            if ((pl1contrl = zatoi(argv[i])) >= NumInputDevices)
            {
              printf("Player 1 Input must be a value from 0 to %u!\n", NumInputDevices);
              exit(1);
            }

            ConvertJoyMap1();
            break;

          case '2': //Player 2 Input
            i++;

            if ((pl2contrl = zatoi(argv[i])) > NumInputDevices)
            {
              printf("Player 2 Input must be a value from 0 to %u!\n", NumInputDevices);
              exit(1);
            }

            ConvertJoyMap2();
            break;

          #ifndef __UNIXSDL__
          case '3': //Enable triple buffering for DOS/Windows
            vsyncon = 0;
          #ifdef __MSDOS__
            Triplebufen = 1;
          #elif __WIN32__
            TripleBufferWin = 1;
          #endif
            break;
          #endif

          #ifdef __WIN32__
          case '6': //Force Refresh Rate
            i++;
            SetRefreshRate = zatoi(argv[i]);
            if((SetRefreshRate < 50) || (SetRefreshRate > 180))
            {
              ForceRefreshRate = 0;
              puts("Refresh Rate must be a value 50 to 180!");
              exit(1);
            }
            else
            {
              ForceRefreshRate = 1;
            }
            break;
          #endif

          #ifdef __MSDOS__
          case '8': //Force 8-bit sound
            Force8b = 1;
            break;

          case 'c': //Enable full screen (when available)
            ScreenScale = 1;
            break;
          #endif

          #ifndef NO_DEBUGGER
          case 'd': //Start with debugger enabled
            debugger = 1;
            debugdisble = 0;
          #ifdef __WIN32__
            InitDebugger();
          #endif
            break;
          #endif

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

          case 'j': //Disable mouse
            MouseDis = 1;
            break;

          case 'k': //Set volume level
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

          case 'm': //Disable GUI
            guioff = 1;
            break;

          case 'n': //Enable scanlines (when available)
            i++;
            if ((scanlines = zatoi(argv[i])) > 3)
            {
              puts("Scanlines must be a value 0 to 3!");
              exit(1);
            }
            break;

          case 'o': //Disable MMX support
            AllowMMX = 0;
            break;

          case 'p': //Percentage of instructions to execute
            i++;
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

          case 's': //Enable sound output, and SPC700/DSP emulation
            spcon = 1;
            soundon = 1;
            break;

          case 't': //Force NTSC
            ForcePal = 1;
            break;

          case 'u': //Force PAL
            ForcePal = 2;
            break;

          case 'v': //Select video mode
            i++;
            if ((cvidmode = zatoi(argv[i])) > VIDEO_MODE_COUNT)
            {
              puts("Invalid Video Mode!");
              exit(1);
            }
            break;

          #ifndef __UNIXSDL__
          case 'w': //Enable vsync for DOS/Windows
            vsyncon = 1;
          #ifdef __MSDOS__
            Triplebufen = 0;
          #elif __WIN32__
            TripleBufferWin = 0;
          #endif
            break;
          #endif

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
      else if (!argv[i][3]) //- followed by two letters
      {
        if (tolower(argv[i][1]) == 'd' && tolower(argv[i][2]) == 'd') //Disable sound DSP emulation
        {
          DSPDisable = 1;
        }

        #ifdef __UNIXSDL__
        else if (tolower(argv[i][1]) == 'a' && tolower(argv[i][2]) == 'd') //Disable sound DSP emulation
        {
          i++;
          if (!argv[i])
          {
            display_help();
          }

          #ifdef __LIBAO__
          if (!strcmp(argv[i], "auto") || !strcmp(argv[i], "sdl") || (ao_driver_id(argv[i]) >= 0))
          #else
          if (!strcmp(argv[i], "auto") || !strcmp(argv[i], "sdl"))
          #endif
          {
            strcpy(libAoDriver, argv[i]);
          }
          else
          {
            puts("Audio driver selection invalid.");
            exit(1);
          }
        }
        #endif

        else if (tolower(argv[i][1]) == 'd' && tolower(argv[i][2]) == 's') //Disable sound output
        {
          soundon = 0;
        }

        #ifdef __MSDOS__
        else if (tolower(argv[i][1]) == 'c' && tolower(argv[i][2]) == 'c') //Enable small screen (when available)
        {
          smallscreenon = 1;
        }
        #endif

        else if (tolower(argv[i][1]) == 'd' && tolower(argv[i][2]) == 'h') //Disable hacks
        {
          HacksDisable = 1;
        }

        #ifndef __MSDOS__
        else if (tolower(argv[i][1]) == 'j' && tolower(argv[i][2]) == 's') //Set joystick sensitivity
        {
          i++;
          if ((joy_sensitivity = zatoi(argv[i])+1) > 32767)
          {
            puts("Joystick sensitivity must be a value of 0 to 32767!");
            exit(1);
          }
        }
        #endif

        #ifdef __WIN32__
        else if (tolower(argv[i][1]) == 'k' && tolower(argv[i][2]) == 's') //Enable KitchenSync
        {
          KitchenSync = 1;
        }
        #endif

        #ifdef __WIN32__
        else if (tolower(argv[i][1]) == 'k' && tolower(argv[i][2]) == 'p') //Enable KitchenSync for PAL only
        {
          KitchenSyncPAL = 1;
          KitchenSync = 0;
        }
        #endif

        else if (tolower(argv[i][1]) == 'm' && tolower(argv[i][2]) == 'c') //Close ZSNES when ZMV closes
        {
          ZMVZClose = 1;
        }

        else if (tolower(argv[i][1]) == 'm' && tolower(argv[i][2]) == 'd') //Dump raw vid with ZMV
        {
          i++;
          if ((ZMVRawDump = zatoi(argv[i])) > 5)
          {
            puts("Movie mode must be a number 1 to 5");
            exit(1);
          }
        }

        else if (tolower(argv[i][1]) == 'm' && tolower(argv[i][2]) == 'l') //Force ZMV length
        {
          i++;
          MovieForcedLengthEnabled = true;
          MovieForcedLength = zatoi(argv[i]);
        }

        #ifdef __MSDOS__
        else if (tolower(argv[i][1]) == 's' && tolower(argv[i][2]) == 'p') //Display sound information
        {
          DisplayS = 1;
        }
        #endif

        else if (tolower(argv[i][1]) == 's' && tolower(argv[i][2]) == 'a') //Show all extensions in GUI
        {
          showallext = 1;
        }

        else if (tolower(argv[i][1]) == 'v' && argv[i][2] == '8') //V8 Mode
        {
          V8Mode = 1;
        }

        else if (tolower(argv[i][1]) == 'z' && tolower(argv[i][2]) == 's') //Autoload save state
        {
          i++;
          if ((autoloadstate = zatoi(argv[i])+1) > 100)
          {
            puts("State load position must be a value of 0 to 99!");
            exit(1);
          }
        }

        else if (tolower(argv[i][1]) == 'z' && tolower(argv[i][2]) == 'm') //Autoload movie
        {
          i++;
          if ((autoloadmovie = zatoi(argv[i])+1) > 10)
          {
            puts("Movie load position must be a value of 0 to 9!");
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
      if (argv[i] && !init_rom_path(argv[i]))
      {
        printf("Could not load: %s\n", argv[i]);
      }

      if ((STCart2 = argv[i+1])) //Sufami Turbo second cart
      {
        char *p;

        natify_slashes(STCart2);
        p = strrchr(STCart2, DIR_SLASH_C);
        if (!p) { p = STCart2; }
        else { p++; }
        strcpy(ZSaveST2Name, p);
        setextension(ZSaveST2Name, "srm");
      }
      break;
    }
  }
}

static void ZCleanup(void)
{
  void deinit_paths();
  void deallocmem();
  void DeallocRewindBuffer();
  void DeallocPauseFrame();
  void DeallocSystemVars();
  void free_all_file_lists();
#ifdef __UNIXSDL__
  void UnloadSDL();
#endif

  deinit_paths();
  deallocmem();
  DeallocRewindBuffer();
  DeallocPauseFrame();
  DeallocSystemVars();
  free_all_file_lists();
#ifdef __UNIXSDL__
  UnloadSDL();
#endif

}

void zmain(int zargc, char *zargv[])
{
  if (init_paths(*zargv))
  {
    #ifdef __LIBAO__
    ao_initialize();
    atexit(ao_shutdown);
    #endif
    handle_params(zargc, zargv);

    atexit(ZCleanup);
    srand(time(0));
    zstart();
  }
}

#ifdef __WIN32__
extern HINSTANCE hInst;
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
  hInst=hInstance;
  ImportDirectX();
  zmain(__argc, __argv);
  return(0);
}
#else
int main(int zargc, char *zargv[])
{
  zmain(zargc, zargv);
  return(0);
}
#endif
