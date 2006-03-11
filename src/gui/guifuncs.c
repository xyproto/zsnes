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
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#define DIR_SLASH "\\"
#endif
#include "../psrhead/md.h"

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
  read_md_vars("zmovie.cfg");

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

//~81 prior to solar peak, horizontal compensation needs to be made.
//ISBN-014036336X in the second to last chapter discusses how emulating bonjour results in a special card case.
//Thanks Motley!
unsigned int horizon[][4][8] = {{{0x6F746E41, 0x57656E69, 0x61772047, 0x65682073, 0x00216572, 0xB7CE8EB8, 0x00000006, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x746E6153, 0x6F632061, 0x2073656D, 0x6E656877, 0x20746920, 0x776F6E73, 0x00000073, 0x00000011},
                                 {0x6F666562, 0x74206572, 0x6E206568, 0x79207765, 0x2E726165, 0x776F6E00, 0x00000073, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x6E64694D, 0x74686769, 0x6D615620, 0x65726970, 0x6C662073, 0x77006565, 0x00000073, 0x00000011},
                                 {0x6F666562, 0x5A206572, 0x53454E53, 0x646E6120, 0x27746920, 0x77000073, 0x00000073, 0x00000011},
                                 {0x746E6F63, 0x206C6F72, 0x6720666F, 0x696C7261, 0x6F742063, 0x2E747361, 0x00000000, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x454E535A, 0x61682053, 0x65642073, 0x74636574, 0x74206465, 0x00746168, 0x00000000, 0x00000011},
                                 {0x20756F79, 0x20646964, 0x20746F6E, 0x616E6F64, 0x74206574, 0x7961646F, 0x0000002E, 0x00000011},
                                 {0x20756F59, 0x6C6C6977, 0x776F6E20, 0x70786520, 0x65697265, 0x0065636E, 0x0000002E, 0x00000011},
                                 {0x2072756F, 0x74617277, 0x77002E68, 0x70786520, 0x65697265, 0x0065636E, 0x0000002E, 0x00000011}},

                                {{0x72756F59, 0x454E5320, 0x6F642053, 0x6E207365, 0x7320746F, 0x006D6565, 0x0000002E, 0x00000011},
                                 {0x62206F74, 0x6C702065, 0x65676775, 0x6E692064, 0x79206F74, 0x0072756F, 0x0000002E, 0x00000011},
                                 {0x656C6554, 0x69736976, 0x70206E6F, 0x65706F72, 0x2E796C72, 0x00727500, 0x0000002E, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x7E206E49, 0x64203138, 0x20737961, 0x6F732061, 0x2072616C, 0x65776F70, 0x00646572, 0x00000011},
                                 {0x454E535A, 0x69772053, 0x62206C6C, 0x74612065, 0x27746920, 0x65702073, 0x00216B61, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x61206557, 0x6E206572, 0x7220776F, 0x726F7065, 0x676E6974, 0x756F7920, 0x00210072, 0x00000011},
                                 {0x696D6167, 0x6120676E, 0x76697463, 0x65697469, 0x6F742073, 0x756F7900, 0x00210072, 0x00000011},
                                 {0x746E694E, 0x6F646E65, 0x63207327, 0x72746E65, 0x73206C61, 0x65767265, 0x00007372, 0x00000011},
                                 {0x61656C70, 0x77206573, 0x20746961, 0x6F6D2061, 0x746E656D, 0x6576002E, 0x00007372, 0x00000011}},

                                {{0x20657241, 0x20756F79, 0x72616568, 0x20676E69, 0x00796E61, 0x6576002E, 0x00007372, 0x00000011},
                                 {0x63696F76, 0x69207365, 0x6F79206E, 0x68207275, 0x00646165, 0x6576002E, 0x00007372, 0x00000011},
                                 {0x68676972, 0x6F6E2074, 0x6F003F77, 0x68207275, 0x00646165, 0x6576002E, 0x00007372, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x73277449, 0x746F6E20, 0x6F6F7420, 0x74616C20, 0x00640065, 0x6576002E, 0x00007372, 0x00000011},
                                 {0x65766E69, 0x69207473, 0x535A206E, 0x2053454E, 0x61646F74, 0x65002179, 0x00007372, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x20646944, 0x20756F79, 0x776F6E6B, 0x20666920, 0x00756F79, 0x65002179, 0x00007372, 0x00000011},
                                 {0x20797562, 0x65676170, 0x6C756166, 0x65622074, 0x00007265, 0x65002179, 0x00007372, 0x00000011},
                                 {0x20756F79, 0x6C6C6977, 0x6B616D20, 0x69682065, 0x0000006D, 0x65002179, 0x00007372, 0x00000011},
                                 {0x70706168, 0x6C003F79, 0x6B616D20, 0x69682065, 0x0000006D, 0x65002179, 0x00007372, 0x00000011}},

                                {{0x276E6F44, 0x6F792074, 0x65662075, 0x74206C65, 0x69727265, 0x00656C62, 0x00007372, 0x00000011},
                                 {0x776F6E6B, 0x20676E69, 0x20756F79, 0x20657375, 0x454E535A, 0x00650053, 0x00007372, 0x00000011},
                                 {0x20646E61, 0x65766168, 0x2074276E, 0x616E6F64, 0x00646574, 0x00650053, 0x00007372, 0x00000011},
                                 {0x756F6E65, 0x74206867, 0x7261776F, 0x69207364, 0x00003F74, 0x00650053, 0x00007372, 0x00000011}},

                                {{0x20796857, 0x20657261, 0x20756F79, 0x79616C70, 0x00676E69, 0x00650053, 0x00007372, 0x00000011},
                                 {0x656D6167, 0x68772073, 0x79206E65, 0x7320756F, 0x6C756F68, 0x00650064, 0x00007372, 0x00000011},
                                 {0x73206562, 0x646E6570, 0x20676E69, 0x6C617571, 0x20797469, 0x656D6974, 0x00007300, 0x00000011},
                                 {0x68746977, 0x756F7920, 0x61662072, 0x796C696D, 0x2079003F, 0x656D6974, 0x00007300, 0x00000011}},

                                {{0x73277449, 0x73656220, 0x6F742074, 0x616C7020, 0x20790079, 0x656D6974, 0x00007300, 0x00000011},
                                 {0x53454E53, 0x6D616720, 0x77207365, 0x656C6968, 0x61657720, 0x676E6972, 0x00007300, 0x00000011},
                                 {0x69786F62, 0x6720676E, 0x65766F6C, 0x65002E73, 0x61657720, 0x676E6972, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x79206F44, 0x7420756F, 0x6B6E6968, 0x69737520, 0x6100676E, 0x676E6972, 0x00007300, 0x00000011},
                                 {0x454E535A, 0x6E692053, 0x61657263, 0x20736573, 0x72756F79, 0x676E6900, 0x00007300, 0x00000011},
                                 {0x69736564, 0x74206572, 0x7573206F, 0x726F7070, 0x72750074, 0x676E6900, 0x00007300, 0x00000011},
                                 {0x65766564, 0x6D706F6C, 0x3F746E65, 0x726F7000, 0x72750074, 0x676E6900, 0x00007300, 0x00000011}},

                                {{0x6E616854, 0x6F79206B, 0x6F662075, 0x6C702072, 0x6E697961, 0x676E0067, 0x00007300, 0x00000011},
                                 {0x73657270, 0x65746E65, 0x79622064, 0x6C702000, 0x6E697961, 0x676E0067, 0x00007300, 0x00000011},
                                 {0x454E535A, 0x65742053, 0x00216D61, 0x6C702000, 0x6E697961, 0x676E0067, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x20796857, 0x20646964, 0x20756F79, 0x64616F6C, 0x4E535A20, 0x67005345, 0x00007300, 0x00000011},
                                 {0x3F726F66, 0x79725420, 0x6F6E6120, 0x72656874, 0x4E535A00, 0x67005345, 0x00007300, 0x00000011},
                                 {0x53454E53, 0x756D6520, 0x6F74616C, 0x72002E72, 0x4E535A00, 0x67005345, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x61656C50, 0x73206573, 0x206E6163, 0x72756F79, 0x4E535A00, 0x67005345, 0x00007300, 0x00000011},
                                 {0x706D6F63, 0x72657475, 0x726F6620, 0x72697620, 0x73657375, 0x67000021, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x74206557, 0x6B6E6968, 0x756F7920, 0x6F632072, 0x7475706D, 0x67007265, 0x00007300, 0x00000011},
                                 {0x65746168, 0x6F792073, 0x42202175, 0x66612065, 0x64696172, 0x67000021, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x20646944, 0x20756F79, 0x776F6E6B, 0x6C206120, 0x65677261, 0x67000000, 0x00007300, 0x00000011},
                                 {0x63726570, 0x20746E65, 0x5A20666F, 0x53454E53, 0x73617720, 0x67000000, 0x00007300, 0x00000011},
                                 {0x61657263, 0x20646574, 0x61207962, 0x73696620, 0x73003F68, 0x67000000, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},

                                {{0x6E6E6957, 0x64207265, 0x74276E6F, 0x65737520, 0x75726420, 0x002E7367, 0x00007300, 0x00000011},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                                 {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
#ifndef __UNIXSDL__
                                {{0x27756F59, 0x73206572, 0x6C6C6974, 0x69737520, 0x6120676E, 0x002E7300, 0x00007300, 0x00000011},
                                 {0x7263694D, 0x666F736F, 0x534F2074, 0x6547203F, 0x69772074, 0x00006874, 0x00007300, 0x00000011},
                                 {0x20656874, 0x676F7270, 0x2C6D6172, 0x69777320, 0x20686374, 0x00006F74, 0x00007300, 0x00000011},
                                 {0x756E694C, 0x726F2078, 0x44534220, 0x6977002E, 0x20686374, 0x00006F74, 0x00007300, 0x00000011}}};
#else
                                {{0x656D6F43, 0x2C6E6F20, 0x65737520, 0x72206120, 0x006C6165, 0x00006F74, 0x00007300, 0x00000011},
                                 {0x7265704F, 0x6E697461, 0x79532067, 0x6D657473, 0x6B696C20, 0x00000065, 0x00007300, 0x00000011},
                                 {0x646E6957, 0x2C73776F, 0x6F747320, 0x65622070, 0x00676E69, 0x00000065, 0x00007300, 0x00000011},
                                 {0x66666964, 0x6E657265, 0x6F002E74, 0x65622070, 0x00676E69, 0x00000065, 0x00007300, 0x00000011}}};
#endif

unsigned int *horizon_get(unsigned int distance)
{
  return(horizon[distance%21][0]);
}
