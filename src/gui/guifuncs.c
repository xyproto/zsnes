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



#ifdef __UNIXSDL__
#include "gblhdr.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#define DIR_SLASH "\\"
#endif

extern unsigned char ComboHeader[23], ComboBlHeader[23], GUIRAdd;
extern unsigned char GUIsmallscreenon, ScreenScale, TimeChecker;
extern unsigned char GUIScreenScale, ShowTimer, ReCalib, cfgdontsave;
extern unsigned char CombinDataGlob[3300], savecfgforce;
extern unsigned int PHnumGUIsave, smallscreenon, SnowTimer, NumSnow;
extern unsigned int CalibXmin, CalibXmax, CalibYmin, CalibYmax, NumComboGlob;
extern unsigned int CalibXmin209, CalibXmax209, CalibYmin209, CalibYmax209;
extern char GUICName[256], GUIFName[256];

unsigned char CalcCfgChecksum()
{
  unsigned char *ptr = &GUIRAdd, i = 0;
  unsigned short chksum = 0;

  for (; i < 100 ; i++, ptr++)  { chksum += *ptr; }

  chksum ^= 0xB2ED; // xor bx,1011001011101101b
  i = (chksum & 0x800) >> 8;
  chksum &= 0xF7FF; // and bh,0F7h

  if (chksum & 0x10) { chksum |= 0x800; }
  chksum &= 0xFFEF; // and bl,0EFh
  if (i) { chksum |= 0x10; }

  i = (chksum >> 8);

  return (((chksum & 0xFF) ^ i) | 0x80);
}

void GUIRestoreVars()
{
  unsigned char read_cfg_vars(const char *);
  FILE *cfg_fp;

  read_cfg_vars(GUIFName);

  smallscreenon = (unsigned int)GUIsmallscreenon;
  ScreenScale = GUIScreenScale;

  if (TimeChecker == CalcCfgChecksum())
  {
    ShowTimer = 1;
    NumSnow = 200;
    SnowTimer = 0;
  }

  if (ReCalib)
  {
    ReCalib = 0;
    CalibXmin = CalibXmax = CalibYmin = CalibYmax = 0;
    CalibXmin209 = CalibXmax209 = CalibYmin209 = CalibYmax209 = 0;
  }

  NumComboGlob = 0;

  if ((cfg_fp = fopen(GUICName, "rb")))
  {
    fread(ComboBlHeader, 1, 23, cfg_fp);

    if (ComboBlHeader[22])
    {
      NumComboGlob = ComboBlHeader[22];
      fread(CombinDataGlob, 1, (NumComboGlob << 6)+2*NumComboGlob, cfg_fp);
    }

    fclose(cfg_fp);
  }
}

void ExecGUISaveVars()
{
  unsigned char write_cfg_vars(const char *);
  FILE *cfg_fp;

  if (ShowTimer == 1) { TimeChecker = CalcCfgChecksum(); }

  if (!cfgdontsave || savecfgforce)
  {
    write_cfg_vars(GUIFName);
  }

  if (NumComboGlob && (cfg_fp = fopen(GUICName, "wb")))
  {
    ComboHeader[22] = NumComboGlob;
    fwrite(ComboHeader, 1, 23, cfg_fp);
    fwrite(CombinDataGlob, 1, (NumComboGlob << 6)+2*NumComboGlob, cfg_fp);
    fclose(cfg_fp);
  }
}
