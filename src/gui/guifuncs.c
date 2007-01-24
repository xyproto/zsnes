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
#define fnamecmp strcmp
#define fnamencmp strncmp
#else
#ifdef __WIN32__
#include "../win/lib.h"
#endif


#ifdef __MSDOS__
#include <fcntl.h>
#include "../dos/lib.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#define fnamencmp strncasecmp
#define fnamecmp strcasecmp
#endif

#ifndef _MSC_VER
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include "../zpath.h"
#include "../md.h"
#include "../cfg.h"
#include "../input.h"
#include "../asm_call.h"
#include "../zloader.h"

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#define BIT(X) (1 << (X))

extern unsigned char ComboHeader[23], ComboBlHeader[23], CombinDataGlob[3300];
extern unsigned char ShowTimer, savecfgforce;
extern unsigned int SnowTimer, NumSnow, NumComboGlob;
extern unsigned char GUIFontData1[705], GUIFontData[705];
enum vtype { UB, UW, UD, SB, SW, SD };

unsigned int ConvertBinaryToInt(char data[])
{
  int x;
  int num = 0;

  for(x = 0;x<8;x++) { if(data[x] == '1') { num |= BIT(7-x); } }

  return(num);
}

void InsertFontChar(char data[], int pos)
{
  GUIFontData1[pos] = ConvertBinaryToInt(data);
}

void LoadCustomFont()
{
  FILE *fp;
  char data[100];
  int x = 0;

  fp = fopen_dir(ZCfgPath, "zfont.txt", "r");
  if (fp)
  {
    while (fgets(data,100,fp) && strcmp(data,"EOF\n") && x < 705)
    {
      fgets(data,10,fp);        //get first line
      InsertFontChar(data,x++);

      fgets(data,10,fp);        //get second line
      InsertFontChar(data,x++);

      fgets(data,10,fp);        //get third line
      InsertFontChar(data,x++);

      fgets(data,10,fp);        //get fourth line
      InsertFontChar(data,x++);

      fgets(data,10,fp);        //get fifth line
      InsertFontChar(data,x++);
    }
  }
  else
  {
    memcpy(GUIFontData1,GUIFontData,705);
    fp = fopen_dir(ZCfgPath, "zfont.txt", "w");
    fputs("; empty space 0x00\n00000000\n00000000\n00000000\n00000000\n00000000\n",fp);
    fputs("; 0 0x01\n01110000\n10011000\n10101000\n11001000\n01110000\n",fp);
    fputs("; 1 0x02\n00100000\n01100000\n00100000\n00100000\n01110000\n",fp);
    fputs("; 2 0x03\n01110000\n10001000\n00110000\n01000000\n11111000\n",fp);
    fputs("; 3 0x04\n01110000\n10001000\n00110000\n10001000\n01110000\n",fp);
    fputs("; 4 0x05\n01010000\n10010000\n11111000\n00010000\n00010000\n",fp);
    fputs("; 5 0x06\n11111000\n10000000\n11110000\n00001000\n11110000\n",fp);
    fputs("; 6 0x07\n01110000\n10000000\n11110000\n10001000\n01110000\n",fp);
    fputs("; 7 0x08\n11111000\n00001000\n00010000\n00010000\n00010000\n",fp);
    fputs("; 8 0x09\n01110000\n10001000\n01110000\n10001000\n01110000\n",fp);
    fputs("; 9 0x0A\n01110000\n10001000\n01111000\n00001000\n01110000\n",fp);
    fputs("; A 0x0B\n01110000\n10001000\n11111000\n10001000\n10001000\n",fp);
    fputs("; B 0x0C\n11110000\n10001000\n11110000\n10001000\n11110000\n",fp);
    fputs("; C 0x0D\n01110000\n10001000\n10000000\n10001000\n01110000\n",fp);
    fputs("; D 0x0E\n11110000\n10001000\n10001000\n10001000\n11110000\n",fp);
    fputs("; E 0x0F\n11111000\n10000000\n11110000\n10000000\n11111000\n",fp);
    fputs("; F 0x10\n11111000\n10000000\n11110000\n10000000\n10000000\n",fp);
    fputs("; G 0x11\n01111000\n10000000\n10011000\n10001000\n01110000\n",fp);
    fputs("; H 0x12\n10001000\n10001000\n11111000\n10001000\n10001000\n",fp);
    fputs("; I 0x13\n11111000\n00100000\n00100000\n00100000\n11111000\n",fp);
    fputs("; J 0x14\n01111000\n00010000\n00010000\n10010000\n01100000\n",fp);
    fputs("; K 0x15\n10010000\n10100000\n11100000\n10010000\n10001000\n",fp);
    fputs("; L 0x16\n10000000\n10000000\n10000000\n10000000\n11111000\n",fp);
    fputs("; M 0x17\n11011000\n10101000\n10101000\n10101000\n10001000\n",fp);
    fputs("; N 0x18\n11001000\n10101000\n10101000\n10101000\n10011000\n",fp);
    fputs("; O 0x19\n01110000\n10001000\n10001000\n10001000\n01110000\n",fp);
    fputs("; P 0x1A\n11110000\n10001000\n11110000\n10000000\n10000000\n",fp);
    fputs("; Q 0x1B\n01110000\n10001000\n10101000\n10010000\n01101000\n",fp);
    fputs("; R 0x1C\n11110000\n10001000\n11110000\n10010000\n10001000\n",fp);
    fputs("; S 0x1D\n01111000\n10000000\n01110000\n00001000\n11110000\n",fp);
    fputs("; T 0x1E\n11111000\n00100000\n00100000\n00100000\n00100000\n",fp);
    fputs("; U 0x1F\n10001000\n10001000\n10001000\n10001000\n01110000\n",fp);
    fputs("; V 0x20\n10001000\n10001000\n01010000\n01010000\n00100000\n",fp);
    fputs("; W 0x21\n10001000\n10101000\n10101000\n10101000\n01010000\n",fp);
    fputs("; X 0x22\n10001000\n01010000\n00100000\n01010000\n10001000\n",fp);
    fputs("; Y 0x23\n10001000\n01010000\n00100000\n00100000\n00100000\n",fp);
    fputs("; Z 0x24\n11111000\n00010000\n00100000\n01000000\n11111000\n",fp);
    fputs("; - 0x25\n00000000\n00000000\n11111000\n00000000\n00000000\n",fp);
    fputs("; _ 0x26\n00000000\n00000000\n00000000\n00000000\n11111000\n",fp);
    fputs("; ~ 0x27\n01101000\n10010000\n00000000\n00000000\n00000000\n",fp);
    fputs("; . 0x28\n00000000\n00000000\n00000000\n00000000\n00100000\n",fp);
    fputs("; / 0x29\n00001000\n00010000\n00100000\n01000000\n10000000\n",fp);
    fputs("; < 0x2A\n00010000\n00100000\n01000000\n00100000\n00010000\n",fp);
    fputs("; > 0x2B\n01000000\n00100000\n00010000\n00100000\n01000000\n",fp);
    fputs("; [ 0x2C\n01110000\n01000000\n01000000\n01000000\n01110000\n",fp);
    fputs("; ] 0x2D\n01110000\n00010000\n00010000\n00010000\n01110000\n",fp);
    fputs("; : 0x2E\n00000000\n00100000\n00000000\n00100000\n00000000\n",fp);
    fputs("; & 0x2F\n01100000\n10011000\n01110000\n10011000\n01101000\n",fp);
    fputs("; arrow down 0x30\n00100000\n00100000\n10101000\n01110000\n00100000\n",fp);
    fputs("; # 0x31\n01010000\n11111000\n01010000\n11111000\n01010000\n",fp);
    fputs("; = 0x32\n00000000\n11111000\n00000000\n11111000\n00000000\n",fp);
    fputs("; \" 0x33\n01001000\n10010000\n00000000\n00000000\n00000000\n",fp);
    fputs("; \\ 0x34\n10000000\n01000000\n00100000\n00010000\n00001000\n",fp);
    fputs("; * 0x35\n10101000\n01110000\n11111000\n01110000\n10101000\n",fp);
    fputs("; ? 0x36\n01110000\n10001000\n00110000\n00000000\n00100000\n",fp);
    fputs("; % 0x37\n10001000\n00010000\n00100000\n01000000\n10001000\n",fp);
    fputs("; + 0x38\n00100000\n00100000\n11111000\n00100000\n00100000\n",fp);
    fputs("; , 0x39\n00000000\n00000000\n00000000\n00100000\n01000000\n",fp);
    fputs("; ( 0x3A\n00110000\n01000000\n01000000\n01000000\n00110000\n",fp);
    fputs("; ) 0x3B\n01100000\n00010000\n00010000\n00010000\n01100000\n",fp);
    fputs("; @ 0x3C\n01110000\n10011000\n10111000\n10000000\n01110000\n",fp);
    fputs("; \' 0x3D\n00100000\n01000000\n00000000\n00000000\n00000000\n",fp);
    fputs("; ! 0x3E\n00100000\n00100000\n00100000\n00000000\n00100000\n",fp);
    fputs("; $ 0x3F\n01111000\n10100000\n01110000\n00101000\n11110000\n",fp);
    fputs("; ; 0x40\n00000000\n00100000\n00000000\n00100000\n01000000\n",fp);
    fputs("; ` 0x41\n01000000\n00100000\n00000000\n00000000\n00000000\n",fp);
    fputs("; ^ 0x42\n00100000\n01010000\n00000000\n00000000\n00000000\n",fp);
    fputs("; { 0x43\n00110000\n01000000\n11000000\n01000000\n00110000\n",fp);
    fputs("; } 0x44\n01100000\n00010000\n00011000\n00010000\n01100000\n",fp);
    fputs("; up 0x45\n00100000\n00100000\n01110000\n01110000\n11111000\n",fp);
    fputs("; down 0x46\n11111000\n01110000\n01110000\n00100000\n00100000\n",fp);
    fputs("; left 0x47\n00001000\n00111000\n11111000\n00111000\n00001000\n",fp);
    fputs("; right 0x48\n10000000\n11100000\n11111000\n11100000\n10000000\n",fp);
    fputs("; arrow left 0x49\n00100000\n01100000\n11111000\n01100000\n00100000\n",fp);
    fputs("; .5 0x4A\n00111000\n00100000\n00110000\n00001000\n10110000\n",fp);
    fputs("; maximize (Win) 0x4B\n11111100\n10000100\n11111100\n00000000\n00000000\n",fp);
    fputs("; minimize (Win) 0x4C\n00000000\n11111100\n00000000\n00000000\n00000000\n",fp);
    fputs("; maximize (SDL) 0x4D\n11111000\n10001000\n10001000\n10001000\n11111000\n",fp);
    fputs("; shw fullstop 0x4E\n00000000\n00000000\n00100000\n01010000\n00100000\n",fp);
    fputs("; shw left bracket 0x4F\n01110000\n01000000\n01000000\n01000000\n00000000\n",fp);
    fputs("; shw right bracket 0x50\n00000000\n00010000\n00010000\n00010000\n01110000\n",fp);
    fputs("; shw comma 0x51\n00000000\n00000000\n00000000\n01000000\n00100000\n",fp);
    fputs("; shw mid-dot 0x52\n00000000\n00100000\n01110000\n00100000\n00000000\n",fp);
    fputs("; shw wo 0x53\n11111000\n00001000\n11110000\n00100000\n11000000\n",fp);
    fputs("; shw mini a 0x54\n00000000\n11111000\n01010000\n01100000\n01000000\n",fp);
    fputs("; shw mini i 0x55\n00000000\n00010000\n00100000\n11100000\n00100000\n",fp);
    fputs("; shw mini u 0x56\n00000000\n00100000\n11111000\n10001000\n00110000\n",fp);
    fputs("; shw mini e 0x57\n00000000\n00000000\n11111000\n00100000\n11111000\n",fp);
    fputs("; shw mini o 0x58\n00000000\n00010000\n11111000\n00110000\n11010000\n",fp);
    fputs("; shw mini ya 0x59\n00000000\n01000000\n11111000\n01010000\n01000000\n",fp);
    fputs("; shw mini yu 0x5A\n00000000\n00000000\n11110000\n00010000\n11111000\n",fp);
    fputs("; shw mini yo 0x5B\n00000000\n11111000\n00001000\n01111000\n11111000\n",fp);
    fputs("; shw mini tsu 0x5C\n00000000\n10101000\n10101000\n00010000\n01100000\n",fp);
    fputs("; shw prolong 0x5D\n00000000\n10000000\n01111000\n00000000\n00000000\n",fp);
    fputs("; shw a 0x5E\n11111000\n00101000\n00110000\n00100000\n11000000\n",fp);
    fputs("; shw i 0x5F\n00001000\n00110000\n11100000\n00100000\n00100000\n",fp);
    fputs("; shw u 0x60\n00100000\n11111000\n10001000\n00010000\n01100000\n",fp);
    fputs("; shw e 0x61\n11111000\n00100000\n00100000\n00100000\n11111000\n",fp);
    fputs("; shw o 0x62\n00010000\n11111000\n00110000\n01010000\n10010000\n",fp);
    fputs("; shw ka 0x63\n01000000\n11111000\n01001000\n01001000\n10011000\n",fp);
    fputs("; shw ki 0x64\n00100000\n11111000\n00100000\n11111000\n00100000\n",fp);
    fputs("; shw ku 0x65\n01000000\n01111000\n10001000\n00010000\n01100000\n",fp);
    fputs("; shw ke 0x66 ^^\n01000000\n01111000\n10010000\n00010000\n01100000\n",fp);
    fputs("; shw ko 0x67\n11111000\n00001000\n00001000\n00001000\n11111000\n",fp);
    fputs("; shw sa 0x68\n01010000\n11111000\n01010000\n00010000\n01100000\n",fp);
    fputs("; shw shi 0x69\n01000000\n10101000\n01001000\n00010000\n11100000\n",fp);
    fputs("; shw su 0x6A\n11111000\n00001000\n00010000\n00110000\n11001000\n",fp);
    fputs("; shw se 0x6B\n01000000\n11111000\n01010000\n01000000\n00111000\n",fp);
    fputs("; shw so 0x6C\n10001000\n01001000\n00001000\n00010000\n01100000\n",fp);
    fputs("; shw ta 0x6D\n01000000\n01111000\n11001000\n00110000\n01100000\n",fp);
    fputs("; shw chi 0x6E\n11111000\n00100000\n11111000\n00100000\n01000000\n",fp);
    fputs("; shw tsu 0x6F\n10101000\n10101000\n00001000\n00010000\n01100000\n",fp);
    fputs("; shw te 0x70\n11111000\n00000000\n11111000\n00100000\n11000000\n",fp);
    fputs("; shw to 0x71\n01000000\n01000000\n01100000\n01010000\n01000000\n",fp);
    fputs("; shw na 0x72\n00100000\n11111000\n00100000\n00100000\n01000000\n",fp);
    fputs("; shw ni 0x73\n11110000\n00000000\n00000000\n00000000\n11111000\n",fp);
    fputs("; shw nu 0x74\n11111000\n00001000\n00101000\n00010000\n01101000\n",fp);
    fputs("; shw ne 0x75\n00100000\n11111000\n00001000\n01110000\n10101000\n",fp);
    fputs("; shw no 0x76\n00001000\n00001000\n00001000\n00010000\n01100000\n",fp);
    fputs("; shw ha 0x77\n01010000\n01010000\n01010000\n10001000\n10001000\n",fp);
    fputs("; shw hi 0x78\n10000000\n10011000\n11100000\n10000000\n01111000\n",fp);
    fputs("; shw hu 0x79\n11111000\n00001000\n00001000\n00010000\n01100000\n",fp);
    fputs("; shw he 0x7A\n01000000\n10100000\n10010000\n00001000\n00000000\n",fp);
    fputs("; shw ho 0x7B\n00100000\n11111000\n01110000\n10101000\n00100000\n",fp);
    fputs("; shw ma 0x7C\n11111000\n00001000\n10010000\n01100000\n00100000\n",fp);
    fputs("; shw mi 0x7D\n11111000\n00000000\n11111000\n00000000\n11111000\n",fp);
    fputs("; shw mu 0x7E\n00100000\n01000000\n01000000\n10010000\n11111000\n",fp);
    fputs("; shw me 0x7F\n00001000\n01001000\n00110000\n00110000\n11001000\n",fp);
    fputs("; shw mo 0x80\n11111000\n00100000\n11111000\n00100000\n00111000\n",fp);
    fputs("; shw ya 0x81\n01000000\n11111100\n01001000\n00100000\n00100000\n",fp);
    fputs("; shw yu 0x82\n11110000\n00010000\n00010000\n00010000\n11111000\n",fp);
    fputs("; shw yo 0x83\n11111000\n00001000\n11111000\n00001000\n11111000\n",fp);
    fputs("; shw ra 0x84\n11111000\n00000000\n11111000\n00010000\n01100000\n",fp);
    fputs("; shw ri 0x85\n10001000\n10001000\n10001000\n00010000\n01100000\n",fp);
    fputs("; shw ru 0x86\n01100000\n01100000\n01101000\n01101000\n10110000\n",fp);
    fputs("; shw re 0x87\n10000000\n10000000\n10001000\n10001000\n11110000\n",fp);
    fputs("; shw ro 0x88\n11111000\n10001000\n10001000\n10001000\n11111000\n",fp);
    fputs("; shw wa 0x89\n11111000\n10001000\n00001000\n00010000\n01100000\n",fp);
    fputs("; shw n 0x8A\n10000000\n01001000\n00001000\n00010000\n11100000\n",fp);
    fputs("; shw voiced 0x8B\n10100000\n10100000\n00000000\n00000000\n00000000\n",fp);
    fputs("; shw halfvoiced 0x8C\n01000000\n10100000\n01000000\n00000000\n00000000\n",fp);
    fputs("EOF\n",fp);
  }

  fclose(fp);
}

static void CheckValueBounds(void *ptr, int min, int max, int val, enum vtype type)
{
  switch (type)
  {
    case SB:
      if (((*(char*)ptr) > (char)max) || ((*(char*)ptr) < (char)min))
      { *(char*)ptr = (char)val; }
      break;
    case UB:
      if (((*(unsigned char*)ptr) > (unsigned char)max) ||
          ((*(unsigned char*)ptr) < (unsigned char)min))
      { *(unsigned char*)ptr = (unsigned char)val; }
      break;

    case SW:
      if (((*(short*)ptr) > (short)max) || ((*(short*)ptr) < (short)min))
      { *(short*)ptr = (short)val; }
      break;
    case UW:
      if (((*(unsigned short*)ptr) > (unsigned short)max) ||
          ((*(unsigned short*)ptr) < (unsigned short)min))
      { *(unsigned short*)ptr = (unsigned short)val; }
      break;

    default:
    case SD:
      if (((*(int*)ptr) > max) || ((*(int*)ptr) < min))
      { *(int*)ptr = val; }
      break;
    case UD:
      if (((*(unsigned int*)ptr) > (unsigned int)max) ||
          ((*(unsigned int*)ptr) < (unsigned int)min))
      { *(unsigned int*)ptr = (unsigned int)val; }
  }
}

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
  int i;
  FILE *cfg_fp;

  psr_cfg_run(read_cfg_vars, ZCfgPath, ZCfgFile);
  psr_cfg_run(read_md_vars, ZCfgPath, "zmovie.cfg");
  psr_cfg_run(read_input_vars, ZCfgPath, "zinput.cfg");

#ifdef __MSDOS__
  CheckValueBounds(&pl1contrl, 0, 16, 1, UB);
  CheckValueBounds(&pl1p209, 0, 1, 0, UB);
  CheckValueBounds(&pl2contrl, 0, 16, 0, UB);
  CheckValueBounds(&pl2p209, 0, 1, 0, UB);
  CheckValueBounds(&pl3contrl, 0, 16, 0, UB);
  CheckValueBounds(&pl3p209, 0, 1, 0, UB);
  CheckValueBounds(&pl4contrl, 0, 16, 0, UB);
  CheckValueBounds(&pl4p209, 0, 1, 0, UB);
  CheckValueBounds(&pl5contrl, 0, 16, 0, UB);
  CheckValueBounds(&pl5p209, 0, 1, 0, UB);
#else
  CheckValueBounds(&pl1contrl, 0, 1, 1, UB);
  CheckValueBounds(&pl2contrl, 0, 1, 0, UB);
  CheckValueBounds(&pl3contrl, 0, 1, 0, UB);
  CheckValueBounds(&pl4contrl, 0, 1, 0, UB);
  CheckValueBounds(&pl5contrl, 0, 1, 0, UB);
#endif

#ifndef __MSDOS__
  CheckValueBounds(&joy_sensitivity, 0, 32767, 16384, UW);
#endif
#ifdef __WIN32__
  CheckValueBounds(&MouseSensitivity, 1, 255, 1, UB);
#endif
#ifdef __MSDOS__
  CheckValueBounds(&SidewinderFix, 0, 1, 0, UB);
#endif
  CheckValueBounds(&pl12s34, 0, 1, 0, UB);
  CheckValueBounds(&AllowUDLR, 0, 1, 0, UB);
  CheckValueBounds(&Turbo30hz, 0, 1, 1, UB);
  CheckValueBounds(&mouse1lh, 0, 1, 0, UB);
  CheckValueBounds(&mouse2lh, 0, 1, 0, UB);
  CheckValueBounds(&device1, 0, 1, 0, UB);
  CheckValueBounds(&device2, 0, 4, 0, UB);
  CheckValueBounds(&GUIComboGameSpec, 0, 1, 0, UB);
  CheckValueBounds(&GameSpecificInput, 0, 1, 0, UB);

  CheckValueBounds(&AllowMMX, 0, 1, 1, UB);
#ifdef __WIN32__
  CheckValueBounds(&PauseFocusChange, 0, 1, 0, UB);
  CheckValueBounds(&HighPriority, 0, 1, 0, UB);
  CheckValueBounds(&DisableScreenSaver, 0, 1, 1, UB);
#endif
  CheckValueBounds(&newengen, 0, 1, 1, UB);
  CheckValueBounds(&bgfixer, 0, 1, 0, UB);
#ifdef NO_PNG
  CheckValueBounds(&ScreenShotFormat, 0, 0, 0, UB);
#else
  CheckValueBounds(&ScreenShotFormat, 0, 1, 0, UB);
#endif
  CheckValueBounds(&AutoPatch, 0, 1, 1, UB);
  CheckValueBounds(&DisplayInfo, 0, 1, 1, UB);
  CheckValueBounds(&RomInfo, 0, 1, 1, UB);
  CheckValueBounds(&FPSAtStart, 0, 1, 0, UB);
  CheckValueBounds(&TimerEnable, 0, 1, 0, UB);
  CheckValueBounds(&TwelveHourClock, 0, 1, 0, UB);
  CheckValueBounds(&ClockBox, 0, 1, 1, UB);
  CheckValueBounds(&SmallMsgText, 0, 1, 0, UB);
  CheckValueBounds(&GUIEnableTransp, 0, 1, 0, UB);

#ifdef __MSDOS__
  CheckValueBounds(&Palette0, 0, 1, 1, UB);
#endif
#ifdef __WIN32__
  CheckValueBounds(&cvidmode, 0, 42, 2, UB);
  CheckValueBounds(&PrevWinMode, 0, 42, 2, UB);
  CheckValueBounds(&PrevFSMode, 0, 42, 6, UB);
#endif
#ifdef __UNIXSDL__
#ifdef __OPENGL__
  CheckValueBounds(&cvidmode, 0, 22, 2, UB);
  CheckValueBounds(&PrevWinMode, 0, 22, 2, UB);
  CheckValueBounds(&PrevFSMode, 0, 22, 3, UB);
#else
  CheckValueBounds(&cvidmode, 0, 4, 2, UB);
  CheckValueBounds(&PrevWinMode, 0, 4, 2, UB);
  CheckValueBounds(&PrevFSMode, 0, 4, 3, UB);
#endif
#endif
#ifdef __MSDOS__
  CheckValueBounds(&cvidmode, 0, 18, 4, UB);
#endif
#ifndef __MSDOS__
  CheckValueBounds(&CustomResX, 256, 2048, 640, UD);
  CheckValueBounds(&CustomResY, 224, 1536, 480, UD);
#endif

  CheckValueBounds(&antienab, 0, 1, 0, UB);
#ifdef __OPENGL__
  CheckValueBounds(&BilinearFilter, 0, 1, 0, UB);
#endif
  CheckValueBounds(&NTSCFilter, 0, 1, 0, UB);
  CheckValueBounds(&NTSCBlend, 0, 1, 0, UB);
  CheckValueBounds(&NTSCRef, 0, 1, 0, UB);
  CheckValueBounds(&NTSCHue, -100, 100, 0, SB);
  CheckValueBounds(&NTSCSat, -100, 100, 0, SB);
  CheckValueBounds(&NTSCCont, -100, 100, 0, SB);
  CheckValueBounds(&NTSCBright, -100, 100, 0, SB);
  CheckValueBounds(&NTSCSharp, -100, 100, 0, SB);
  CheckValueBounds(&NTSCGamma, -100, 100, 0, SB);
  CheckValueBounds(&NTSCRes, -100, 100, 0, SB);
  CheckValueBounds(&NTSCArt, -100, 100, 0, SB);
  CheckValueBounds(&NTSCFringe, -100, 100, 0, SB);
  CheckValueBounds(&NTSCBleed, -100, 100, 0, SB);
  CheckValueBounds(&NTSCWarp, -100, 100, 0, SB);
  CheckValueBounds(&En2xSaI, 0, 3, 0, UB);
#ifndef __MSDOS__
  CheckValueBounds(&hqFilter, 0, 1, 0, UB);
  CheckValueBounds(&hqFilterlevel, 2, 4, 2, UB);
#endif
  CheckValueBounds(&scanlines, 0, 3, 0, UB);
  CheckValueBounds(&GrayscaleMode, 0, 1, 0, UB);
  CheckValueBounds(&Mode7HiRes16b, 0, 1, 0, UD);
#ifndef __UNIXSDL__
  CheckValueBounds(&vsyncon, 0, 1, 0, UB);
#endif
#ifdef __WIN32__
  CheckValueBounds(&TripleBufferWin, 0, 1, 0, UB);
#endif
#ifdef __MSDOS__
  CheckValueBounds(&Triplebufen, 0, 1, 0, UB);
#endif
#ifdef __WIN32__
  CheckValueBounds(&ForceRefreshRate, 0, 1, 0, UB);
  CheckValueBounds(&SetRefreshRate, 50, 180, 60, UB);
  CheckValueBounds(&KitchenSync, 0, 1, 0, UB);
  CheckValueBounds(&KitchenSyncPAL, 0, 1, 0, UB);
#endif
#ifndef __MSDOS__
  CheckValueBounds(&Keep4_3Ratio, 0, 1, 1, UB);
#else
  CheckValueBounds(&smallscreenon, 0, 1, 0, UD);
  CheckValueBounds(&ScreenScale, 0, 1, 0, UB);
#endif
  CheckValueBounds(&gammalevel, 0, 15, 0, UB);

  CheckValueBounds(&SPCDisable, 0, 1, 0, UB);
  CheckValueBounds(&soundon, 0, 1, 1, UB);
  CheckValueBounds(&StereoSound, 0, 1, 1, UB);
  CheckValueBounds(&RevStereo, 0, 1, 0, UB);
  CheckValueBounds(&Surround, 0, 1, 0, UB);
#ifdef __WIN32__
  CheckValueBounds(&PrimaryBuffer, 0, 1, 0, UB);
#endif
#ifdef __MSDOS__
  CheckValueBounds(&Force8b, 0, 1, 0, UB);
#endif
  CheckValueBounds(&SoundQuality, 0, 6, 5, UD);
  CheckValueBounds(&MusicRelVol, 0, 100, 100, UB);
  CheckValueBounds(&SoundInterpType, 0, 3, 1, UB);
  CheckValueBounds(&LowPassFilterType, 0, 3, 0, UB);
#ifdef __MSDOS__
  CheckValueBounds(&DisplayS, 0, 1, 0, UB);
#endif
  CheckValueBounds(&EchoDis, 0, 1, 0, UB);

  CheckValueBounds(&RewindStates, 0, 99, 8, UB);
  CheckValueBounds(&RewindFrames, 1, 99, 15, UB);
  CheckValueBounds(&nosaveSRAM, 0, 1, 0, UB);
  CheckValueBounds(&SRAMSave5Sec, 0, 1, 0, UB);
  CheckValueBounds(&SRAMState, 0, 1, 1, UB);
  CheckValueBounds(&LatestSave, 0, 1, 0, UB);
  CheckValueBounds(&AutoIncSaveSlot, 0, 1, 0, UB);
  CheckValueBounds(&AutoIncSaveSlotBlock, 0, 1, 0, UB);
  CheckValueBounds(&AutoState, 0, 1, 0, UB);
  CheckValueBounds(&PauseLoad, 0, 1, 0, UB);
  CheckValueBounds(&PauseRewind, 0, 1, 0, UB);

  CheckValueBounds(&per2exec, 50, 150, 100, UD);
  CheckValueBounds(&HacksDisable, 0, 1, 0, UB);
  CheckValueBounds(&frameskip, 0, 10, 0, UB);
  CheckValueBounds(&maxskip, 0, 9, 9, UB);
  CheckValueBounds(&FastFwdToggle, 0, 1, 0, UB);
  CheckValueBounds(&FFRatio, 0, 28, 8, UB);
  CheckValueBounds(&SDRatio, 0, 28, 0, UB);
  CheckValueBounds(&EmuSpeed, 0, 58, 29, UB);

  CheckValueBounds(&guioff, 0, 1, 0, UB);
  CheckValueBounds(&showallext, 0, 1, 0, UB);
#ifdef __MSDOS__
  CheckValueBounds(&GUIloadfntype, 0, 2, 2, UB);
#else
  CheckValueBounds(&GUIloadfntype, 0, 1, 0, UB);
#endif
  CheckValueBounds(&prevlfreeze, 0, 1, 0, UB);
  CheckValueBounds(&GUIRClick, 0, 1, 0, UB);
  CheckValueBounds(&lhguimouse, 0, 1, 0, UB);
  CheckValueBounds(&mouseshad, 0, 1, 1, UB);
  CheckValueBounds(&mousewrap, 0, 1, 0, UB);
#ifdef __WIN32__
  CheckValueBounds(&TrapMouseCursor, 0, 1, 0, UB);
  CheckValueBounds(&MouseWheel, 0, 1, 1, UB);
#endif
  CheckValueBounds(&esctomenu, 0, 1, 1, UB);
  CheckValueBounds(&JoyPad1Move, 0, 1, 0, UB);
  CheckValueBounds(&FilteredGUI, 0, 1, 1, UB);
  CheckValueBounds(&newfont, 0, 1, 0, UB);
  CheckValueBounds(&savewinpos, 0, 1, 0, UB);
  for (i=1 ; i<22 ; i++)
  {
    CheckValueBounds(GUIwinposx+i, -233, 254, 10, SD);
    CheckValueBounds(GUIwinposy+i, 8, 221, 20, SD);
  }
  CheckValueBounds(&GUIEffect, 0, 5, 0, UB);
  CheckValueBounds(&GUIRAdd, 0, 31, 15, UB);
  CheckValueBounds(&GUIGAdd, 0, 31, 10, UB);
  CheckValueBounds(&GUIBAdd, 0, 31, 31, UB);
  CheckValueBounds(&GUITRAdd, 0, 31, 0, UB);
  CheckValueBounds(&GUITGAdd, 0, 31, 10, UB);
  CheckValueBounds(&GUITBAdd, 0, 31, 31, UB);
  CheckValueBounds(&GUIWRAdd, 0, 31, 8, UB);
  CheckValueBounds(&GUIWGAdd, 0, 31, 8, UB);
  CheckValueBounds(&GUIWBAdd, 0, 31, 25, UB);
#ifdef __WIN32__
  CheckValueBounds(&AlwaysOnTop, 0, 1, 0, UB);
  CheckValueBounds(&SaveMainWindowPos, 0, 1, 1, UB);
  CheckValueBounds(&AllowMultipleInst, 0, 1, 1, UB);
#endif

  CheckValueBounds(&AutoLoadCht, 0, 1, 0, UB);
  CheckValueBounds(&CheatSrcByteSize, 0, 3, 0, UB);
  CheckValueBounds(&CheatSrcByteBase, 0, 1, 0, UB);
  CheckValueBounds(&CheatSrcSearchType, 0, 1, 0, UB);
  CheckValueBounds(&CheatUpperByteOnly, 0, 1, 0, UB);

  CheckValueBounds(&MovieDisplayFrame, 0, 1, 0, UB);
  CheckValueBounds(&MovieStartMethod, 0, 3, 0, UB);
  CheckValueBounds(&MZTForceRTR, 0, 2, 0, UB);
  CheckValueBounds(&MovieVideoMode, 0, 5, 4, UB);
  CheckValueBounds(&MovieAudio, 0, 1, 1, UB);
  CheckValueBounds(&MovieAudioCompress, 0, 1, 1, UB);
  CheckValueBounds(&MovieVideoAudio, 0, 1, 1, UB);

  CheckValueBounds(&FirstTimeData, 0, 1, 1, UB);
#ifndef NO_DEBUGGER
  CheckValueBounds(&debuggeron, 0, 1, 0, UB);
#endif
  CheckValueBounds(&cfgdontsave, 0, 1, 0, UB);

  //if (TimeChecker == CalcCfgChecksum()) //What does this do?
  {
    ShowTimer = 1;
    NumSnow = 200;
    SnowTimer = 0;
  }

  NumComboGlob = 0;

  if ((cfg_fp = fopen_dir(ZCfgPath, "data.cmb", "rb")))
  {
    fread(ComboBlHeader, 1, 23, cfg_fp);

    if (ComboBlHeader[22])
    {
      NumComboGlob = ComboBlHeader[22];
      fread(CombinDataGlob, 1, 66*NumComboGlob, cfg_fp);
    }

    fclose(cfg_fp);
  }

  LoadCustomFont();
}

void GUISaveVars()
{
  FILE *cfg_fp;

  if (ShowTimer == 1) { TimeChecker = CalcCfgChecksum(); }

  if (!cfgdontsave || savecfgforce)
  {
    swap_backup_vars();
    psr_cfg_run(write_cfg_vars, ZCfgPath, ZCfgFile);
    if (!GameSpecificInput)
    {
      psr_cfg_run(write_input_vars, ZCfgPath, "zinput.cfg");
    }
    swap_backup_vars();
  }

  if (NumComboGlob && (cfg_fp = fopen_dir(ZCfgPath, "data.cmb", "wb")))
  {
    ComboHeader[22] = NumComboGlob;
    fwrite(ComboHeader, 1, 23, cfg_fp);
    fwrite(CombinDataGlob, 1, 66*NumComboGlob, cfg_fp);
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

                                {{0x7E206E49, 0x64203138, 0x20737961, 0x6F732061, 0x0072616C, 0x00727500, 0x0000002E, 0xB7F261E0},
                                 {0x65776F70, 0x20646572, 0x454E535A, 0x69772053, 0x62206C6C, 0x74612065, 0x00000000, 0xB7F261E0},
                                 {0x73277469, 0x61657020, 0x4500216B, 0x69772053, 0x62206C6C, 0x74612065, 0x00000000, 0xB7F261E0},
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

                                {{0x6E6E6957, 0x20737265, 0x276E6F64, 0x73752074, 0x72642065, 0x2E736775, 0x00007300, 0xB7F1F1E0},
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

extern unsigned int GUICBHold, NumCheats;
extern unsigned char cheatdata[28*255+56];

void CheatCodeSave()
{
  FILE *fp = 0;

  GUICBHold=0;

  if (NumCheats)
  {
    cheatdata[6]=254;
    cheatdata[7]=252;

    setextension(ZSaveName, "cht");

    if ((fp = fopen_dir(ZSramPath,ZSaveName,"wb")))
    {
      fwrite(cheatdata, 1, 28*NumCheats, fp);
      fclose(fp);
    }
  }
}

extern unsigned char CheatOn;
void DisableCheatsOnLoad(), EnableCheatsOnLoad();
extern unsigned int GUIcurrentcheatcursloc;

void CheatCodeLoad()
{
  FILE *fp = 0;
  unsigned int cheat_file_size, i, j, k;

  setextension(ZSaveName, "cht");
  GUICBHold = 0;

  if ((fp = fopen_dir(ZSramPath,ZSaveName,"rb")))
  {
    asm_call(DisableCheatsOnLoad);

    cheat_file_size = fread(cheatdata, 1, 255*28, fp);
    fclose(fp);

    if(cheatdata[6]==254 && cheatdata[7]==252)
      NumCheats = cheat_file_size / 28;
    else
    {
      NumCheats = cheat_file_size / 18;
      i = 28 * NumCheats;
      j = cheat_file_size - (cheat_file_size % 18);

      do
      {
        i-=28;
        j-=18;

        for (k=6;k>0;k--) cheatdata[i+k-1]=cheatdata[j+k-1];
        for (k=12;k>0;k--) cheatdata[i+k+7]=cheatdata[j+k+5];
        for (k=8;k>0;k--) cheatdata[i+k+19] = 0;
      } while(i>0);
    }

    asm_call(EnableCheatsOnLoad);

    if (NumCheats <= GUIcurrentcheatcursloc) GUIcurrentcheatcursloc=NumCheats-1;
    if (NumCheats) CheatOn=1;
    else GUIcurrentcheatcursloc=CheatOn=0;
  }
}

extern unsigned char *vidbuffer;

void SaveCheatSearchFile()
{
  FILE *fp = 0;

  if ((fp = fopen_dir(ZCfgPath,"tmpchtsr.___","wb")))
  {
    fwrite(vidbuffer+129600, 1, 65536*2+32768, fp);
    fclose(fp);
  }
}

void LoadCheatSearchFile()
{
  FILE *fp = 0;

  if ((fp = fopen_dir(ZCfgPath,"tmpchtsr.___","rb")))
  {
    fread(vidbuffer+129600, 1, 65536*2+32768, fp);
    fclose(fp);
  }
}

extern unsigned char *spcBuffera;

void dumpsound()
{
  FILE *fp = fopen_dir(ZSpcPath, "sounddmp.raw", "wb");
  if (fp)
  {
    fwrite(spcBuffera, 1, 65536*4+4096, fp);
    fclose(fp);
  }
}

static bool snes_extension_match(const char *filename)
{
  char *dot = strrchr(filename, '.');
  if (dot)
  {
    dot++;
    if (!strcasecmp(dot, "sfc") ||
        !strcasecmp(dot, "jma") ||
        !strcasecmp(dot, "zip") ||
        !strcasecmp(dot, "gz") ||
        !strcasecmp(dot, "st") ||
        !strcasecmp(dot, "bs") ||
        !strcasecmp(dot, "smc") ||
        !strcasecmp(dot, "swc") ||
        !strcasecmp(dot, "fig") ||
        !strcasecmp(dot, "dx2") ||
        !strcasecmp(dot, "ufo") ||
        !strcasecmp(dot, "gd3") ||
        !strcasecmp(dot, "gd7") ||
        !strcasecmp(dot, "mgd") ||
        !strcasecmp(dot, "mgh") ||
        !strcasecmp(dot, "048") ||
        !strcasecmp(dot, "058") ||
        !strcasecmp(dot, "078") ||
        !strcasecmp(dot, "bin") ||
        !strcasecmp(dot, "usa") ||
        !strcasecmp(dot, "eur") ||
        !strcasecmp(dot, "jap") ||
        !strcasecmp(dot, "aus") ||
        !strcasecmp(dot, "1") ||
        !strcasecmp(dot, "a"))
    {
      return(true);
    }
  }
  return(false);
}

#define HEADER_SIZE 512
#define INFO_LEN (0xFF - 0xC0)
#define INAME_LEN 21

static const char *get_rom_name(const char *filename, char *namebuffer, struct stat *filestats)
{
  int InfoScore(char *);
  unsigned int sum(unsigned char *array, unsigned int size);

  char *last_dot = strrchr(filename, '.');
  if (!last_dot || (strcasecmp(last_dot, ".zip") && strcasecmp(last_dot, ".gz") && strcasecmp(last_dot, ".jma")))
  {
    if ((filestats->st_size >= 0x8000) && (filestats->st_size <= 0x600000+HEADER_SIZE))
    {
      FILE *fp = fopen_dir(ZRomPath, filename, "rb");
      if (fp)
      {
        unsigned char HeaderBuffer[HEADER_SIZE];
        int HeaderSize = 0, HasHeadScore = 0, NoHeadScore = 0, HeadRemain = filestats->st_size & 0x7FFF;
        bool EHi = false;

        switch(HeadRemain)
        {
          case 0:
            NoHeadScore += 3;
            break;

          case HEADER_SIZE:
            HasHeadScore += 2;
            break;
        }

        fread(HeaderBuffer, 1, HEADER_SIZE, fp);

        if (sum(HeaderBuffer, HEADER_SIZE) < 2500) { HasHeadScore += 2; }

        //SMC/SWC Header
        if (HeaderBuffer[8] == 0xAA && HeaderBuffer[9] == 0xBB && HeaderBuffer[10]== 4)
        {
          HasHeadScore += 3;
        }
        //FIG Header
        else if ((HeaderBuffer[4] == 0x77 && HeaderBuffer[5] == 0x83) ||
                 (HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 0x82) ||
                 (HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 2) ||
                 (HeaderBuffer[4] == 0xF7 && HeaderBuffer[5] == 0x83) ||
                 (HeaderBuffer[4] == 0xFD && HeaderBuffer[5] == 0x82) ||
                 (HeaderBuffer[4] == 0x00 && HeaderBuffer[5] == 0x80) ||
                 (HeaderBuffer[4] == 0x47 && HeaderBuffer[5] == 0x83) ||
                 (HeaderBuffer[4] == 0x11 && HeaderBuffer[5] == 2))
        {
          HasHeadScore += 2;
        }
        else if (!strncmp("GAME DOCTOR SF 3", (char *)HeaderBuffer, 16))
        {
          HasHeadScore += 5;
        }

        HeaderSize = HasHeadScore > NoHeadScore ? HEADER_SIZE : 0;

        if (filestats->st_size - HeaderSize >= 0x500000)
        {
          fseek(fp, 0x40FFC0 + HeaderSize, SEEK_SET);
          fread(HeaderBuffer, 1, INFO_LEN, fp);
          if (InfoScore((char *)HeaderBuffer) > 1)
          {
            EHi = true;
            strncpy(namebuffer, (char *)HeaderBuffer, INAME_LEN);
          }
        }

        if (!EHi)
        {
          if (filestats->st_size - HeaderSize >= 0x10000)
          {
            char LoHead[INFO_LEN], HiHead[INFO_LEN];
            int LoScore, HiScore;

            fseek(fp, 0x7FC0 + HeaderSize, SEEK_SET);
            fread(LoHead, 1, INFO_LEN, fp);
            LoScore = InfoScore(LoHead);

            fseek(fp, 0xFFC0 + HeaderSize, SEEK_SET);
            fread(HiHead, 1, INFO_LEN, fp);
            HiScore = InfoScore(HiHead);

            strncpy(namebuffer, LoScore > HiScore ? LoHead : HiHead, INAME_LEN);

            if (filestats->st_size - HeaderSize >= 0x20000)
            {
              int IntLScore;
              fseek(fp, (filestats->st_size - HeaderSize) / 2 + 0x7FC0 + HeaderSize, SEEK_SET);
              fread(LoHead, 1, INFO_LEN, fp);
              IntLScore = InfoScore(LoHead) / 2;

              if (IntLScore > LoScore && IntLScore > HiScore)
              {
                strncpy(namebuffer, LoHead, INAME_LEN);
              }
            }
          }
          else //ROM only has one block
          {
            fseek(fp, 0x7FC0 + HeaderSize, SEEK_SET);
            fread(namebuffer, INAME_LEN, 1, fp);
          }
        }
        fclose(fp);
      }
      else //Couldn't open file
      {
        strcpy(namebuffer, "** READ FAILURE **");
      }
    }
    else //Smaller than a block, or Larger than 6MB
    {
      strcpy(namebuffer, "** INVALID FILE **");
    }
  }
  else //Compressed archive
  {
    return(filename);
  }
  namebuffer[21] = 0;
  return(namebuffer);
}

char **lf_names = 0; //Long File Names
char **et_names = 0; //Eight Three Names
char **i_names = 0; //Internal Names
char **d_names = 0; //Directory Names

char **selected_names = 0; //Used to point to requested one

#define LIST_LFN BIT(0)
#define LIST_ETN BIT(1)
#define LIST_IN  BIT(2)
#define LIST_DN  BIT(3)

#ifdef __MSDOS__
#define main_names et_names
#define LIST_MAIN LIST_ETN
#else
#define main_names lf_names
#define LIST_MAIN LIST_LFN
#endif

#ifndef _USE_LFN
#define _USE_LFN 1
#endif

#define swapper(array) if (array) { hold = array[x]; array[x] = array[y]; array[y] = hold; }

static void swapfiles(size_t x, size_t y)
{
  char *hold;
  swapper(lf_names);
  swapper(et_names);
  swapper(i_names);
}

static void swapdirs(size_t x, size_t y)
{
  char *hold = d_names[x];
  d_names[x] = d_names[y];
  d_names[y] = hold;
}

static void sort(intptr_t *array, int begin, int end, void (*swapfunc)(size_t, size_t))
{
  if (end > begin)
  {
    intptr_t *pivot = array + begin;
    int l = begin + 1;
    int r = end;
    while (l < r)
    {
      if (strcasecmp((const char *)*(array+l), (const char *)*pivot) <= 0)
      {
        l++;
      }
      else
      {
        r--;
        swapfunc(l, r);
      }
    }
    l--;
    swapfunc(begin, l);
    sort(array, begin, l, swapfunc);
    sort(array, r, end, swapfunc);
  }
}


void free_list(char ***list)
{
  char **p = *list;
  if (p)
  {
    p += 2;
    while (*p)
    {
      free(*p++);
    }
    free(*list);
    *list = 0;
  }
}

//A possible problem here would be if one of the list arrays got enlarged but a corosponding one ran out of memory
static void add_list(char ***reallist, const char *p)
{
  char **list = *reallist;
  if (!list)
  {
    if (!(list = malloc(1003*sizeof(void *)))) { return; }
    list[0] = (char *)2;
    list[1] = (char *)1002;
    list[2] = 0;
  }

  if (list[0] == list[1]-1)
  {
    char **p = realloc(list, ((size_t)list[1]+1000)*sizeof(void *));
    if (p)
    {
      list = p;
      list[1] += 1000;
    }
    else
    {
      return;
    }
  }

  if ((list[(size_t)*list] = malloc(strlen(p)+1)))
  {
    strcpy(list[(size_t)*list], p);
    list[0]++;
    list[(size_t)*list] = 0;
  }
  *reallist = list;
}

//Make sure ZRomPath contains a full absolute directory name before calling
void populate_lists(unsigned int lists, bool snes_ext_match)
{
  DIR *dir;

  if ((lists&LIST_DN) && (strlen(ZRomPath) > ROOT_LEN))
  {
    add_list(&d_names, "..");
  }

  if ((dir = opendir(ZRomPath)))
  {
    struct stat stat_buffer;
    struct dirent *entry;

    while ((entry = readdir(dir)))
    {
      if ((*entry->d_name != '.') && !stat_dir(ZRomPath, entry->d_name, &stat_buffer))
      {
        if (S_ISDIR(stat_buffer.st_mode))
        {
          if (lists&LIST_DN)
          {
            add_list(&d_names, entry->d_name);
          }
        }
        else if (!snes_ext_match || snes_extension_match(entry->d_name))
        {
          if (_USE_LFN && (lists&LIST_LFN))
          {
            add_list(&lf_names, entry->d_name);
          }

          if (lists&LIST_IN)
          {
            char namebuffer[22];
            add_list(&i_names, get_rom_name(entry->d_name, namebuffer, &stat_buffer));
          }

#ifdef __MSDOS__
          if (lists&LIST_ETN)
          {
            if (!_USE_LFN) //_USE_LFN won't be true when running under pure DOS
            {
              add_list(&et_names, entry->d_name);
            }
            else
            {
              char *sfn = realpath_sfn_dir(ZRomPath, entry->d_name, 0);
              if (sfn)
              {
                add_list(&et_names, basename(sfn));
                free(sfn);
              }
              else
              {
                char sfn[13];
                _lfn_gen_short_fname(entry->d_name, sfn);
                add_list(&et_names, sfn);
              }
            }
          }
#endif
        }
      }
    }
    closedir(dir);
  }

  if (lists&LIST_DN)
  {
#ifndef __UNIXSDL__
    unsigned int drives = GetLogicalDrives(), i = 0;
#endif

    if (d_names)
    {
      unsigned int offset = (d_names[2][0] == '.') ? 3 : 2;
      sort((intptr_t *)d_names, offset, (size_t)(*d_names), swapdirs);
    }

#ifndef __UNIXSDL__
    while (i < 26)
    {
      if (drives&BIT(i))
      {
        char drive[] = { '[', 'A', ':', ']', 0 };
        drive[1] = 'A'+i;
        add_list(&d_names, drive);
      }
      i++;
    }
#endif
  }

  if ((lists&LIST_IN) && i_names)
  {
    sort((intptr_t *)i_names, 2, (size_t)(*i_names), swapfiles);
  }
  else if ((lists&LIST_LFN) && lf_names)
  {
    sort((intptr_t *)lf_names, 2, (size_t)(*lf_names), swapfiles);
  }
  else if ((lists&LIST_ETN) && et_names)
  {
    sort((intptr_t *)et_names, 2, (size_t)(*et_names), swapfiles);
  }
}

static void memswap(void *p1, void *p2, size_t p2len)
{
  char *ptr1 = (char *)p1;
  char *ptr2 = (char *)p2;

  const size_t p1len = ptr2 - ptr1;
  unsigned char byte;
  while (p2len--)
  {
    byte = *ptr2++;
    memmove(ptr1+1, ptr1, p1len);
    *ptr1++ = byte;
  }
}

extern unsigned char GUIwinptr, GUIcmenupos, GUIpmenupos;
extern unsigned char GUIwinorder[], GUIwinactiv[], pressed[];

extern char GUIPrevMenuData[];

void powercycle(bool, bool);

void GUIloadfilename(char *filename)
{
  char *p = strdupcat(ZRomPath, filename);
  if (p)
  {
    if (init_rom_path(p)) { powercycle(false, true); }
    free(p);
  }
  if (GUIwinptr) { GUIcmenupos = GUIpmenupos; }
}

void loadquickfname(const unsigned char slot)
{
  if (prevloaddnamel[1+slot*512]) // replace with better test
  {
    strcpy(ZRomPath, (char *)prevloaddnamel+1+slot*512);
    strcatslash(ZRomPath);
    strcpy(ZCartName, (char *)prevloadfnamel+slot*512);

    if (!access_dir(ZRomPath, ZCartName, R_OK))
    {
      if (slot || !prevlfreeze)
      {
        // move menuitem to top
        memswap(prevloadiname,prevloadiname+slot*28,28);
        memswap(prevloadfnamel,prevloadfnamel+slot*512,512);
        memswap(prevloaddnamel,prevloaddnamel+slot*512,512);
      }

      GUIloadfilename(ZCartName);
    }
  }
}

void GUIQuickLoadUpdate()
{
  size_t entry_size, copy_num, i = 10;
  char *src;

  memcpy(GUIPrevMenuData+347, (prevlfreeze) ? " ON " : " OFF", 4);

  src = (char *)prevloadiname;
  entry_size = 28;
  copy_num = 28; //full window width

  while (i--)
  {
    char *p_src = src + i*entry_size;
    char *p_dest = GUIPrevMenuData+3 + i*32;
    size_t srclen = strlen(p_src);

    if (srclen >= copy_num)
    {
      strncpy(p_dest, p_src, copy_num);
      if (srclen > copy_num) { memset(p_dest+25, '.', 3); }
    }
    else
    {
      strncpy(p_dest, p_src, srclen);
      memset(p_dest+srclen, ' ', 28-srclen);
    }
  }
}


int GUIcurrentviewloc; //current file position
int GUIcurrentcursloc; //current cursor position (GUI)
int GUIcurrentdirviewloc; //current directory position
int GUIcurrentdircursloc; //current dir position (GUI)
int GUIdirentries;
int GUIfileentries;

void free_all_file_lists()
{
  free_list(&d_names);
  free_list(&i_names);
  free_list(&lf_names);
  free_list(&et_names);
}

void GetLoadData()
{
  GUIcurrentviewloc = GUIcurrentcursloc = GUIcurrentdirviewloc = GUIcurrentdircursloc = 0;

  free_all_file_lists();

  switch (GUIloadfntype)
  {
    case 0: //LFN
      populate_lists(LIST_DN|LIST_ETN|LIST_LFN, !showallext);
      selected_names = lf_names ? lf_names : et_names;
      break;
    case 1: //IN
      populate_lists(LIST_DN|LIST_MAIN|LIST_IN, !showallext);
      selected_names = i_names;
      break;
    default:
      populate_lists(LIST_DN|LIST_MAIN, !showallext);
      selected_names = main_names;
      break;
  }
  selected_names += 2;
  GUIfileentries = main_names ? ((unsigned int)(*main_names))-2 : 0;
  GUIdirentries = d_names ? ((unsigned int)(*d_names))-2 : 0;
}

unsigned int GUIcurrentfilewin;
unsigned int GUIdirStartLoc;

void GUILoadData()
{
  char *nameptr;

  GUICBHold = 0;
  if (GUIcurrentfilewin) // directories
  {
    nameptr = d_names[GUIcurrentdircursloc+2];

    strcatslash(ZRomPath);
    #ifndef __UNIXSDL__
    if ((strlen(nameptr) == 4) && (nameptr[2] == ':')) // MS drives are stored as '[?:]',
    { // so we can't use quick string catenation to browse through
      strncpy(ZRomPath, nameptr+1, 2);
      ZRomPath[2] = '\\';
      ZRomPath[3] = 0;
    }
    else
    #endif
    {
      if (!strcmp(nameptr, ".."))
      {
        strdirname(ZRomPath);
      }
      else
      {
        strcat(ZRomPath, nameptr);
      }
      strcatslash(ZRomPath);
    }

    GetLoadData();
  }
  else // files
  {
    nameptr = main_names[GUIcurrentcursloc+2];

    strcpy(ZCartName, nameptr);

    if (!prevlfreeze)
    {
      int i = 0;
      bool dupfound = false;
      bool modheader = true;

      while (!dupfound && i<10)
      {
        dupfound = (!fnamencmp(nameptr, (char *)prevloadfnamel+i*512, 512) && (!fnamencmp(ZRomPath, (char *)prevloaddnamel+i*512+1, 512)));
        if(dupfound && modheader)
        {
          strncpy((char *)prevloadiname+i*28, selected_names[GUIcurrentcursloc], 28);
          prevloadiname[i*28+27] = 0;
          modheader = false;
        }
        i++;
      }
      i--;

      if (!dupfound)
      {
        strncpy((char *)prevloadiname+9*28, selected_names[GUIcurrentcursloc], 28);
        prevloadiname[9*28+27] = 0;
        strcpy((char *)prevloaddnamel+9*512+1, ZRomPath);
        strcpy((char *)prevloadfnamel+9*512, ZCartName);
      }

      loadquickfname(i);
    }
    else
    {
      GUIloadfilename(ZCartName);
    }

    GUIwinactiv[1] = 0; // close load dialog
    GUIwinorder[--GUIwinptr] = 0;
  }
}

extern char GUILoadTextA[];
extern unsigned char GUILoadPos;

void GUILoadManualDir()
{

  if (*GUILoadTextA)
  {
    char path_buff[PATH_SIZE];
    bool realpath_success;

    if ((GUILoadPos > ROOT_LEN) && (GUILoadTextA[GUILoadPos-1] == DIR_SLASH_C))
    {
      GUILoadTextA[GUILoadPos-1] = 0;
    }

    realpath_success = (int)realpath_dir(ZRomPath, GUILoadTextA, path_buff);
    if (realpath_success)
    {
      struct stat stat_buffer;
      if (!stat(path_buff, &stat_buffer))
      {
        if (S_ISDIR(stat_buffer.st_mode))
        {
          strcpy(ZRomPath, path_buff);
          strcatslash(ZRomPath);
          GetLoadData();
        }
        else
        {
          if (init_rom_path(path_buff)) { powercycle(false, true); }
        }
        return;
      }
    }
  }

  GUILoadData();
}


unsigned char gui_key;
unsigned char gui_key_extended;
int GUILoadKeysNavigate()
{
#ifdef __UNIXSDL__
  extern unsigned int numlockptr;
#endif

  int *currentviewloc, *currentcursloc, *entries;
  if (GUIcurrentfilewin == 1)
  {
    currentviewloc = &GUIcurrentdirviewloc;
    currentcursloc = &GUIcurrentdircursloc;
    entries = &GUIdirentries;
  }
  else
  {
    currentviewloc = &GUIcurrentviewloc;
    currentcursloc = &GUIcurrentcursloc;
    entries = &GUIfileentries;
  }

  //Handle left and right
  if(GUIfileentries && GUIdirentries)
  {
    #ifdef __UNIXSDL__
    if ((gui_key_extended == 92) || ((numlockptr != 1) && (gui_key_extended == 75)))
    #else
    if (gui_key_extended == 75)
    #endif
    {
      GUILoadPos = 0;
      GUIcurrentfilewin ^= 1;
      return(1);
    }

    #ifdef __UNIXSDL__
    if ((gui_key_extended == 94) || ((numlockptr != 1) && (gui_key_extended == 77)))
    #else
    if(gui_key_extended == 77)
    #endif
    {
      GUILoadPos = 0;
      GUIcurrentfilewin ^= 1;
      return(1);
    }
  }

  //Enter press
  if (gui_key_extended == 13)
  {
    GUILoadPos = 0;
    GUILoadManualDir();
    return(1);
  }

  //Home key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 89)||((numlockptr != 1) && (gui_key_extended == 71)))
  #else
  if (gui_key_extended == 71)
  #endif
  {
    GUILoadPos = 0;
    *currentcursloc = 0;
    *currentviewloc = 0;
    return(1);
  }

  //End key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 95)||((numlockptr != 1) && (gui_key_extended == 79)))
  #else
  if (gui_key_extended == 79)
  #endif
  {
    GUILoadPos = 0;
    *currentcursloc = (*entries)-1;
    *currentviewloc = (*entries)-15;
    if (*currentviewloc < 0)
    {
      *currentviewloc = 0;
    }
    return(1);
  }

  //Up arrow key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 90)||((numlockptr != 1) && (gui_key_extended == 72)))
  #else
  if (gui_key_extended == 72)
  #endif
  {
    GUILoadPos = 0;
    if (*currentcursloc)
    {
      if (*currentviewloc == *currentcursloc)
      {
        (*currentviewloc)--;
      }
      (*currentcursloc)--;
    }
    return(1);
  }

  //Down arrow key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 96)||((numlockptr != 1) && (gui_key_extended == 80)))
  #else
  if (gui_key_extended == 80)
  #endif
  {
    GUILoadPos = 0;
    if ((*currentcursloc)+1 != *entries)
    {
      (*currentcursloc)++;
      if ((*currentcursloc)-15 == *currentviewloc)
      {
        (*currentviewloc)++;
      }
    }
    return(1);
  }

  //Page up key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 91)||((numlockptr != 1) && (gui_key_extended == 73)))
  #else
  if (gui_key_extended == 73)
  #endif
  {
    GUILoadPos = 0;
    *currentviewloc -= 15;
    *currentcursloc -= 15;
    if (*currentviewloc < 0)
    {
      *currentviewloc = 0;
    }
    if (*currentcursloc < 0)
    {
      *currentcursloc = 0;
    }
    return(1);
  }

  //Page down key
  #ifdef __UNIXSDL__
  if ((gui_key_extended == 97)||((numlockptr != 1) && (gui_key_extended == 81)))
  #else
  if (gui_key_extended == 81)
  #endif
  {
    GUILoadPos = 0;
    *currentviewloc += 15;
    *currentcursloc += 15;
    if (*currentcursloc >= (*entries)-1)
    {
      *currentcursloc = (*entries)-1;
    }
    if (*currentviewloc >= (*entries)-15)
    {
      *currentviewloc = ((*entries)-15)>0 ? (*entries)-15 : 0;
    }
   return(1);
  }

  return(0);
}

#ifdef __UNIXSDL__
#define DriveCount() 0
#else
static unsigned int DriveCount()
{
  unsigned int drives = GetLogicalDrives(), count = 0, i = 0;
  while (i < 26)
  {
    if (drives&BIT(i))
    {
      count++;
    }
    i++;
  }
  return(count);
}
#endif

int *GUIJT_currentviewloc, *GUIJT_currentcursloc, GUIJT_entries, GUIJT_offset, GUIJT_viewable;

void GUIGenericJumpTo()
{
  int mid = GUIJT_viewable>>1;
  *GUIJT_currentviewloc = (GUIJT_offset < GUIJT_entries-mid) ? GUIJT_offset-mid : GUIJT_entries-GUIJT_viewable;
  if (*GUIJT_currentviewloc < 0) { *GUIJT_currentviewloc = 0; }
  *GUIJT_currentcursloc = GUIJT_offset;
}

void GUILoadKeysJumpTo()
{
  char **base;
  int start, end;

  GUILoadTextA[GUILoadPos] = 0;

  if (GUIcurrentfilewin == 1)
  {
    GUIJT_currentviewloc = &GUIcurrentdirviewloc;
    GUIJT_currentcursloc = &GUIcurrentdircursloc;
    GUIJT_entries = GUIdirentries;
    base = d_names+2;
    if (!strcmp(*base, ".."))
    {
      base++;
      GUIJT_entries--;
    }

    GUIJT_entries -= DriveCount();
  }
  else
  {
    GUIJT_currentviewloc = &GUIcurrentviewloc;
    GUIJT_currentcursloc = &GUIcurrentcursloc;
    GUIJT_entries = GUIfileentries;
    base = selected_names;
  }

  start = 0;
  end = GUIJT_entries-1;
  GUIJT_offset = GUIJT_entries;
  if (!strcmp(GUILoadTextA, " ")) //Exactly a space picks a game randomely
  {
    GUIJT_offset = rand()%GUIJT_entries;
  }
  else
  {
    while (start <= end)
    {
      int mid = (start+end)>>1;
      int pos = strncasecmp(base[mid], GUILoadTextA, GUILoadPos);
      if (!pos)
      {
        do
        {
          GUIJT_offset = mid--;
        } while ((mid >= 0) && !strncasecmp(base[mid], GUILoadTextA, GUILoadPos));
        break;
      }
      if (pos > 0)
      {
        end = mid-1;
      }
      else
      {
        start = mid+1;
      }
    }
  }

  if (GUIJT_offset < GUIJT_entries)
  {
    if (GUIcurrentfilewin == 1)
    {
      GUIJT_entries += DriveCount();
      if (base > d_names+2)
      {
        GUIJT_offset++;
        GUIJT_entries++;
      }
    }

    GUIJT_viewable = 15;
    GUIGenericJumpTo();
  }
}

//Not entirely accurate pow, but good for most needs and very fast
static unsigned int npow(register unsigned int base, register unsigned int exponent)
{
  register unsigned int total = 1;
  if (exponent)
  {
    register unsigned int i;
    for (i = 2, total = base; i < exponent; i += i)
    {
      total *= total;
    }
    for (i >>= 1; i < exponent; i++)
    {
      total *= base;
    }
  }
  return(total);
}

static void int_to_str(char *dest, unsigned int len, unsigned int num)
{
  *dest = 0;
  if (len && (num < npow(10, len)))
  {
    int i;
    for (i = 1; num; i++)
    {
      memmove(dest+1, dest, i);
      *dest = (num%10)+'0';
      num /= 10;
    }
  }
}

#ifndef __MSDOS__

char GUICustomX[5], GUICustomY[5];
void GetCustomXY()
{
  static bool first_time = true;
  if (first_time)
  {
    int_to_str(GUICustomX, 4, CustomResX);
    int_to_str(GUICustomY, 4, CustomResY);
    first_time = false;
  }
}

void SetCustomXY()
{
  if(!((atoi(GUICustomX) < 256) || (atoi(GUICustomX) > 2048) || (atoi(GUICustomY) < 224) || (atoi(GUICustomY) > 1536)))
  {
    CustomResX = atoi(GUICustomX);
    CustomResY = atoi(GUICustomY);
    if(CustomResX < 298)
      Keep4_3Ratio = 0;
  }
}

extern char ShowKeep43;

void Keep43Check()
{
  if((CustomResX*3) == (CustomResY*4))
    ShowKeep43 = 0;
  else
    ShowKeep43 = 1;
}

#endif

extern unsigned int MovieForcedLength;
char GUIMovieForcedText[11];

void GetMovieForcedLength()
{
  static bool first_time = true;
  if (first_time)
  {
    int_to_str(GUIMovieForcedText, 10, MovieForcedLength);
    first_time = false;
  }
}

void SetMovieForcedLength()
{
  MovieForcedLength = atoi(GUIMovieForcedText);
}
