/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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
#include "../gblhdr.h"
#include "../linux/sdllink.h"
#define fnamecmp strcmp
#define fnamencmp strncmp
#else
#ifdef __WIN32__
#include "../win/lib.h"
#endif

#ifdef __MSDOS__
#include "../dos/lib.h"
#include <fcntl.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#define fnamencmp strncasecmp
#define fnamecmp strcasecmp
#endif

#ifndef __MSDOS__
#include "../c_intrf.h"
#endif

#ifndef _MSC_VER
#include <stdint.h>
#include <unistd.h>
#endif

#include "../asm_call.h"
#include "../cfg.h"
#include "../initc.h"
#include "../input.h"
#include "../macros.h"
#include "../md.h"
#include "../ui.h"
#include "../zdir.h"
#include "../zloader.h"
#include "../zpath.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guicheat.h"
#include "guifuncs.h"
#include "guiwindp.h"

#define BIT(X) (1 << (X))

enum vtype { UB,
    UW,
    UD,
    SB,
    SW,
    SD };

unsigned int ConvertBinaryToInt(char data[])
{
    int x;
    int num = 0;

    for (x = 0; x < 8; x++) {
        if (data[x] == '1') {
            num |= BIT(7 - x);
        }
    }

    return (num);
}

static void InsertFontChar(char data[], u4 const glyph, u4 const y)
{
    GUIFontData1[glyph][y] = ConvertBinaryToInt(data);
}

void LoadCustomFont()
{
    FILE* fp;
    char data[100];
    int x = 0;

    fp = fopen_dir(ZCfgPath, "zfont.txt", "r");
    if (fp) {
        while (fgets(data, 100, fp) && strcmp(data, "EOF\n") && x < 141) {
            fgets(data, 10, fp); // get first line
            InsertFontChar(data, x, 0);

            fgets(data, 10, fp); // get second line
            InsertFontChar(data, x, 1);

            fgets(data, 10, fp); // get third line
            InsertFontChar(data, x, 2);

            fgets(data, 10, fp); // get fourth line
            InsertFontChar(data, x, 3);

            fgets(data, 10, fp); // get fifth line
            InsertFontChar(data, x, 4);
        }
    } else {
        memcpy(GUIFontData1, GUIFontData, sizeof(GUIFontData1));
        fp = fopen_dir(ZCfgPath, "zfont.txt", "w");
        fputs("; empty space 0x00\n00000000\n00000000\n00000000\n00000000\n00000000\n", fp);
        fputs("; 0 0x01\n01110000\n10011000\n10101000\n11001000\n01110000\n", fp);
        fputs("; 1 0x02\n00100000\n01100000\n00100000\n00100000\n01110000\n", fp);
        fputs("; 2 0x03\n01110000\n10001000\n00110000\n01000000\n11111000\n", fp);
        fputs("; 3 0x04\n01110000\n10001000\n00110000\n10001000\n01110000\n", fp);
        fputs("; 4 0x05\n01010000\n10010000\n11111000\n00010000\n00010000\n", fp);
        fputs("; 5 0x06\n11111000\n10000000\n11110000\n00001000\n11110000\n", fp);
        fputs("; 6 0x07\n01110000\n10000000\n11110000\n10001000\n01110000\n", fp);
        fputs("; 7 0x08\n11111000\n00001000\n00010000\n00010000\n00010000\n", fp);
        fputs("; 8 0x09\n01110000\n10001000\n01110000\n10001000\n01110000\n", fp);
        fputs("; 9 0x0A\n01110000\n10001000\n01111000\n00001000\n01110000\n", fp);
        fputs("; A 0x0B\n01110000\n10001000\n11111000\n10001000\n10001000\n", fp);
        fputs("; B 0x0C\n11110000\n10001000\n11110000\n10001000\n11110000\n", fp);
        fputs("; C 0x0D\n01110000\n10001000\n10000000\n10001000\n01110000\n", fp);
        fputs("; D 0x0E\n11110000\n10001000\n10001000\n10001000\n11110000\n", fp);
        fputs("; E 0x0F\n11111000\n10000000\n11110000\n10000000\n11111000\n", fp);
        fputs("; F 0x10\n11111000\n10000000\n11110000\n10000000\n10000000\n", fp);
        fputs("; G 0x11\n01111000\n10000000\n10011000\n10001000\n01110000\n", fp);
        fputs("; H 0x12\n10001000\n10001000\n11111000\n10001000\n10001000\n", fp);
        fputs("; I 0x13\n11111000\n00100000\n00100000\n00100000\n11111000\n", fp);
        fputs("; J 0x14\n01111000\n00010000\n00010000\n10010000\n01100000\n", fp);
        fputs("; K 0x15\n10010000\n10100000\n11100000\n10010000\n10001000\n", fp);
        fputs("; L 0x16\n10000000\n10000000\n10000000\n10000000\n11111000\n", fp);
        fputs("; M 0x17\n11011000\n10101000\n10101000\n10101000\n10001000\n", fp);
        fputs("; N 0x18\n11001000\n10101000\n10101000\n10101000\n10011000\n", fp);
        fputs("; O 0x19\n01110000\n10001000\n10001000\n10001000\n01110000\n", fp);
        fputs("; P 0x1A\n11110000\n10001000\n11110000\n10000000\n10000000\n", fp);
        fputs("; Q 0x1B\n01110000\n10001000\n10101000\n10010000\n01101000\n", fp);
        fputs("; R 0x1C\n11110000\n10001000\n11110000\n10010000\n10001000\n", fp);
        fputs("; S 0x1D\n01111000\n10000000\n01110000\n00001000\n11110000\n", fp);
        fputs("; T 0x1E\n11111000\n00100000\n00100000\n00100000\n00100000\n", fp);
        fputs("; U 0x1F\n10001000\n10001000\n10001000\n10001000\n01110000\n", fp);
        fputs("; V 0x20\n10001000\n10001000\n01010000\n01010000\n00100000\n", fp);
        fputs("; W 0x21\n10001000\n10101000\n10101000\n10101000\n01010000\n", fp);
        fputs("; X 0x22\n10001000\n01010000\n00100000\n01010000\n10001000\n", fp);
        fputs("; Y 0x23\n10001000\n01010000\n00100000\n00100000\n00100000\n", fp);
        fputs("; Z 0x24\n11111000\n00010000\n00100000\n01000000\n11111000\n", fp);
        fputs("; - 0x25\n00000000\n00000000\n11111000\n00000000\n00000000\n", fp);
        fputs("; _ 0x26\n00000000\n00000000\n00000000\n00000000\n11111000\n", fp);
        fputs("; ~ 0x27\n01101000\n10010000\n00000000\n00000000\n00000000\n", fp);
        fputs("; . 0x28\n00000000\n00000000\n00000000\n00000000\n00100000\n", fp);
        fputs("; / 0x29\n00001000\n00010000\n00100000\n01000000\n10000000\n", fp);
        fputs("; < 0x2A\n00010000\n00100000\n01000000\n00100000\n00010000\n", fp);
        fputs("; > 0x2B\n01000000\n00100000\n00010000\n00100000\n01000000\n", fp);
        fputs("; [ 0x2C\n01110000\n01000000\n01000000\n01000000\n01110000\n", fp);
        fputs("; ] 0x2D\n01110000\n00010000\n00010000\n00010000\n01110000\n", fp);
        fputs("; : 0x2E\n00000000\n00100000\n00000000\n00100000\n00000000\n", fp);
        fputs("; & 0x2F\n01100000\n10011000\n01110000\n10011000\n01101000\n", fp);
        fputs("; arrow down 0x30\n00100000\n00100000\n10101000\n01110000\n00100000\n", fp);
        fputs("; # 0x31\n01010000\n11111000\n01010000\n11111000\n01010000\n", fp);
        fputs("; = 0x32\n00000000\n11111000\n00000000\n11111000\n00000000\n", fp);
        fputs("; \" 0x33\n01001000\n10010000\n00000000\n00000000\n00000000\n", fp);
        fputs("; \\ 0x34\n10000000\n01000000\n00100000\n00010000\n00001000\n", fp);
        fputs("; * 0x35\n10101000\n01110000\n11111000\n01110000\n10101000\n", fp);
        fputs("; ? 0x36\n01110000\n10001000\n00110000\n00000000\n00100000\n", fp);
        fputs("; % 0x37\n10001000\n00010000\n00100000\n01000000\n10001000\n", fp);
        fputs("; + 0x38\n00100000\n00100000\n11111000\n00100000\n00100000\n", fp);
        fputs("; , 0x39\n00000000\n00000000\n00000000\n00100000\n01000000\n", fp);
        fputs("; ( 0x3A\n00110000\n01000000\n01000000\n01000000\n00110000\n", fp);
        fputs("; ) 0x3B\n01100000\n00010000\n00010000\n00010000\n01100000\n", fp);
        fputs("; @ 0x3C\n01110000\n10011000\n10111000\n10000000\n01110000\n", fp);
        fputs("; \' 0x3D\n00100000\n01000000\n00000000\n00000000\n00000000\n", fp);
        fputs("; ! 0x3E\n00100000\n00100000\n00100000\n00000000\n00100000\n", fp);
        fputs("; $ 0x3F\n01111000\n10100000\n01110000\n00101000\n11110000\n", fp);
        fputs("; ; 0x40\n00000000\n00100000\n00000000\n00100000\n01000000\n", fp);
        fputs("; ` 0x41\n01000000\n00100000\n00000000\n00000000\n00000000\n", fp);
        fputs("; ^ 0x42\n00100000\n01010000\n00000000\n00000000\n00000000\n", fp);
        fputs("; { 0x43\n00110000\n01000000\n11000000\n01000000\n00110000\n", fp);
        fputs("; } 0x44\n01100000\n00010000\n00011000\n00010000\n01100000\n", fp);
        fputs("; up 0x45\n00100000\n00100000\n01110000\n01110000\n11111000\n", fp);
        fputs("; down 0x46\n11111000\n01110000\n01110000\n00100000\n00100000\n", fp);
        fputs("; left 0x47\n00001000\n00111000\n11111000\n00111000\n00001000\n", fp);
        fputs("; right 0x48\n10000000\n11100000\n11111000\n11100000\n10000000\n", fp);
        fputs("; arrow left 0x49\n00100000\n01100000\n11111000\n01100000\n00100000\n", fp);
        fputs("; .5 0x4A\n00111000\n00100000\n00110000\n00001000\n10110000\n", fp);
        fputs("; maximize (Win) 0x4B\n11111100\n10000100\n11111100\n00000000\n00000000\n", fp);
        fputs("; minimize (Win) 0x4C\n00000000\n11111100\n00000000\n00000000\n00000000\n", fp);
        fputs("; maximize (SDL) 0x4D\n11111000\n10001000\n10001000\n10001000\n11111000\n", fp);
        fputs("; shw fullstop 0x4E\n00000000\n00000000\n00100000\n01010000\n00100000\n", fp);
        fputs("; shw left bracket 0x4F\n01110000\n01000000\n01000000\n01000000\n00000000\n", fp);
        fputs("; shw right bracket 0x50\n00000000\n00010000\n00010000\n00010000\n01110000\n", fp);
        fputs("; shw comma 0x51\n00000000\n00000000\n00000000\n01000000\n00100000\n", fp);
        fputs("; shw mid-dot 0x52\n00000000\n00100000\n01110000\n00100000\n00000000\n", fp);
        fputs("; shw wo 0x53\n11111000\n00001000\n11110000\n00100000\n11000000\n", fp);
        fputs("; shw mini a 0x54\n00000000\n11111000\n01010000\n01100000\n01000000\n", fp);
        fputs("; shw mini i 0x55\n00000000\n00010000\n00100000\n11100000\n00100000\n", fp);
        fputs("; shw mini u 0x56\n00000000\n00100000\n11111000\n10001000\n00110000\n", fp);
        fputs("; shw mini e 0x57\n00000000\n00000000\n11111000\n00100000\n11111000\n", fp);
        fputs("; shw mini o 0x58\n00000000\n00010000\n11111000\n00110000\n11010000\n", fp);
        fputs("; shw mini ya 0x59\n00000000\n01000000\n11111000\n01010000\n01000000\n", fp);
        fputs("; shw mini yu 0x5A\n00000000\n00000000\n11110000\n00010000\n11111000\n", fp);
        fputs("; shw mini yo 0x5B\n00000000\n11111000\n00001000\n01111000\n11111000\n", fp);
        fputs("; shw mini tsu 0x5C\n00000000\n10101000\n10101000\n00010000\n01100000\n", fp);
        fputs("; shw prolong 0x5D\n00000000\n10000000\n01111000\n00000000\n00000000\n", fp);
        fputs("; shw a 0x5E\n11111000\n00101000\n00110000\n00100000\n11000000\n", fp);
        fputs("; shw i 0x5F\n00001000\n00110000\n11100000\n00100000\n00100000\n", fp);
        fputs("; shw u 0x60\n00100000\n11111000\n10001000\n00010000\n01100000\n", fp);
        fputs("; shw e 0x61\n11111000\n00100000\n00100000\n00100000\n11111000\n", fp);
        fputs("; shw o 0x62\n00010000\n11111000\n00110000\n01010000\n10010000\n", fp);
        fputs("; shw ka 0x63\n01000000\n11111000\n01001000\n01001000\n10011000\n", fp);
        fputs("; shw ki 0x64\n00100000\n11111000\n00100000\n11111000\n00100000\n", fp);
        fputs("; shw ku 0x65\n01000000\n01111000\n10001000\n00010000\n01100000\n", fp);
        fputs("; shw ke 0x66 ^^\n01000000\n01111000\n10010000\n00010000\n01100000\n", fp);
        fputs("; shw ko 0x67\n11111000\n00001000\n00001000\n00001000\n11111000\n", fp);
        fputs("; shw sa 0x68\n01010000\n11111000\n01010000\n00010000\n01100000\n", fp);
        fputs("; shw shi 0x69\n01000000\n10101000\n01001000\n00010000\n11100000\n", fp);
        fputs("; shw su 0x6A\n11111000\n00001000\n00010000\n00110000\n11001000\n", fp);
        fputs("; shw se 0x6B\n01000000\n11111000\n01010000\n01000000\n00111000\n", fp);
        fputs("; shw so 0x6C\n10001000\n01001000\n00001000\n00010000\n01100000\n", fp);
        fputs("; shw ta 0x6D\n01000000\n01111000\n11001000\n00110000\n01100000\n", fp);
        fputs("; shw chi 0x6E\n11111000\n00100000\n11111000\n00100000\n01000000\n", fp);
        fputs("; shw tsu 0x6F\n10101000\n10101000\n00001000\n00010000\n01100000\n", fp);
        fputs("; shw te 0x70\n11111000\n00000000\n11111000\n00100000\n11000000\n", fp);
        fputs("; shw to 0x71\n01000000\n01000000\n01100000\n01010000\n01000000\n", fp);
        fputs("; shw na 0x72\n00100000\n11111000\n00100000\n00100000\n01000000\n", fp);
        fputs("; shw ni 0x73\n11110000\n00000000\n00000000\n00000000\n11111000\n", fp);
        fputs("; shw nu 0x74\n11111000\n00001000\n00101000\n00010000\n01101000\n", fp);
        fputs("; shw ne 0x75\n00100000\n11111000\n00001000\n01110000\n10101000\n", fp);
        fputs("; shw no 0x76\n00001000\n00001000\n00001000\n00010000\n01100000\n", fp);
        fputs("; shw ha 0x77\n01010000\n01010000\n01010000\n10001000\n10001000\n", fp);
        fputs("; shw hi 0x78\n10000000\n10011000\n11100000\n10000000\n01111000\n", fp);
        fputs("; shw hu 0x79\n11111000\n00001000\n00001000\n00010000\n01100000\n", fp);
        fputs("; shw he 0x7A\n01000000\n10100000\n10010000\n00001000\n00000000\n", fp);
        fputs("; shw ho 0x7B\n00100000\n11111000\n01110000\n10101000\n00100000\n", fp);
        fputs("; shw ma 0x7C\n11111000\n00001000\n10010000\n01100000\n00100000\n", fp);
        fputs("; shw mi 0x7D\n11111000\n00000000\n11111000\n00000000\n11111000\n", fp);
        fputs("; shw mu 0x7E\n00100000\n01000000\n01000000\n10010000\n11111000\n", fp);
        fputs("; shw me 0x7F\n00001000\n01001000\n00110000\n00110000\n11001000\n", fp);
        fputs("; shw mo 0x80\n11111000\n00100000\n11111000\n00100000\n00111000\n", fp);
        fputs("; shw ya 0x81\n01000000\n11111100\n01001000\n00100000\n00100000\n", fp);
        fputs("; shw yu 0x82\n11110000\n00010000\n00010000\n00010000\n11111000\n", fp);
        fputs("; shw yo 0x83\n11111000\n00001000\n11111000\n00001000\n11111000\n", fp);
        fputs("; shw ra 0x84\n11111000\n00000000\n11111000\n00010000\n01100000\n", fp);
        fputs("; shw ri 0x85\n10001000\n10001000\n10001000\n00010000\n01100000\n", fp);
        fputs("; shw ru 0x86\n01100000\n01100000\n01101000\n01101000\n10110000\n", fp);
        fputs("; shw re 0x87\n10000000\n10000000\n10001000\n10001000\n11110000\n", fp);
        fputs("; shw ro 0x88\n11111000\n10001000\n10001000\n10001000\n11111000\n", fp);
        fputs("; shw wa 0x89\n11111000\n10001000\n00001000\n00010000\n01100000\n", fp);
        fputs("; shw n 0x8A\n10000000\n01001000\n00001000\n00010000\n11100000\n", fp);
        fputs("; shw voiced 0x8B\n10100000\n10100000\n00000000\n00000000\n00000000\n", fp);
        fputs("; shw halfvoiced 0x8C\n01000000\n10100000\n01000000\n00000000\n00000000\n", fp);
        fputs("EOF\n", fp);
    }

    fclose(fp);
}

static void CheckValueBounds(void* ptr, int min, int max, int val, enum vtype type)
{
    switch (type) {
    case SB:
        if (((*(char*)ptr) > (char)max) || ((*(char*)ptr) < (char)min)) {
            *(char*)ptr = (char)val;
        }
        break;
    case UB:
        if (((*(unsigned char*)ptr) > (unsigned char)max) || ((*(unsigned char*)ptr) < (unsigned char)min)) {
            *(unsigned char*)ptr = (unsigned char)val;
        }
        break;

    case SW:
        if (((*(short*)ptr) > (short)max) || ((*(short*)ptr) < (short)min)) {
            *(short*)ptr = (short)val;
        }
        break;
    case UW:
        if (((*(unsigned short*)ptr) > (unsigned short)max) || ((*(unsigned short*)ptr) < (unsigned short)min)) {
            *(unsigned short*)ptr = (unsigned short)val;
        }
        break;

    default:
    case SD:
        if (((*(int*)ptr) > max) || ((*(int*)ptr) < min)) {
            *(int*)ptr = val;
        }
        break;
    case UD:
        if (((*(unsigned int*)ptr) > (unsigned int)max) || ((*(unsigned int*)ptr) < (unsigned int)min)) {
            *(unsigned int*)ptr = (unsigned int)val;
        }
    }
}

unsigned char CalcCfgChecksum()
{
    unsigned char *ptr = &GUIRAdd, i = 0;
    unsigned short chksum = 0;

    for (; i < 100; i++, ptr++) {
        chksum += *ptr;
    }

    chksum ^= 0xB2ED; // xor bx,1011001011101101b
    i = (chksum & 0x800) >> 8;
    chksum &= 0xF7FF; // and bh,0F7h

    if (chksum & 0x10) {
        chksum |= 0x800;
    }
    chksum &= 0xFFEF; // and bl,0EFh
    if (i) {
        chksum |= 0x10;
    }

    i = (chksum >> 8);

    return (((chksum & 0xFF) ^ i) | 0x80);
}

void GUIRestoreVars()
{
    int i;
    FILE* cfg_fp;

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
#endif
    CheckValueBounds(&DisableScreenSaver, 0, 1, 1, UB);
    CheckValueBounds(&newengen, 0, 1, 1, UB);
    CheckValueBounds(&bgfixer, 0, 1, 0, UB);
#ifndef NO_PNG
#ifndef __MSDOS__
    CheckValueBounds(&ScreenShotFormat, 0, 1, 1, UB);
#else
    CheckValueBounds(&ScreenShotFormat, 0, 1, 0, UB);
#endif
#else
    CheckValueBounds(&ScreenShotFormat, 0, 0, 0, UB);
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
    CheckValueBounds(&cvidmode, 0, 59, 2, UB);
    CheckValueBounds(&PrevWinMode, 0, 59, 2, UB);
    CheckValueBounds(&PrevFSMode, 0, 59, 6, UB);
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
    CheckValueBounds(&Mode7HiRes16b, 0, 1, 0, UB);
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

    CheckValueBounds(&RelPathBase, 0, 1, 0, UB);

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
    for (i = 1; i < 22; i++) {
        CheckValueBounds(GUIwinposx + i, -233, 254, 10, SD);
        CheckValueBounds(GUIwinposy + i, 8, 221, 20, SD);
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

    // if (TimeChecker == CalcCfgChecksum()) //What does this do?
    {
        ShowTimer = 1;
        NumSnow = 200;
        SnowTimer = 0;
    }

    NumComboGlob = 0;

    if ((cfg_fp = fopen_dir(ZCfgPath, "data.cmb", "rb"))) {
        u1 ComboBlHeader[23];
        fread(ComboBlHeader, 1, 23, cfg_fp);

        if (ComboBlHeader[22]) {
            NumComboGlob = ComboBlHeader[22];
            fread(CombinDataGlob, sizeof(*CombinDataGlob), NumComboGlob, cfg_fp);
        }

        fclose(cfg_fp);
    }

    LoadCustomFont();
}

void GUISaveVars(void)
{
    FILE* cfg_fp;

    if (ShowTimer == 1) {
        TimeChecker = CalcCfgChecksum();
    }

    if (!cfgdontsave || savecfgforce) {
        swap_backup_vars();
        psr_cfg_run(write_cfg_vars, ZCfgPath, ZCfgFile);
        if (!GameSpecificInput) {
            psr_cfg_run(write_input_vars, ZCfgPath, "zinput.cfg");
        }
        swap_backup_vars();
    }

    if (NumComboGlob && (cfg_fp = fopen_dir(ZCfgPath, "data.cmb", "wb"))) {
        ComboHeader[22] = NumComboGlob;
        fwrite(ComboHeader, 1, 23, cfg_fp);
        fwrite(CombinDataGlob, sizeof(*CombinDataGlob), NumComboGlob, cfg_fp);
        fclose(cfg_fp);
    }
}

/* ~81 days prior to solar peak, horizontal compensation needs to be made.
 * ISBN-014036336X in the second to last chapter discusses how emulating bonjour
 * results in a special card case.  Thanks Motley! */
static char const* const horizon[][4] = {
    { "AntoineWG was here!", "", "", "" },
    { "Santa comes when it snows", "before the new year.", "", "" },
    { "Midnight vampires flee", "before ZSNES and its", "control of garlic toast.", "" },
    { "ZSNES has detected that", "you did not donate today.", "You will now experience", "our wrath." },
    { "Your SNES does not seem", "to be plugged into your", "television properly.", "" },
    { "In ~81 days a solar", "powered ZSNES will be at", "its peak!", "" },
    { "We are now reporting your", "gaming activities to", "Nintendo's central servers.", "Please wait a moment." },
    { "Are you hearing any", "voices in your head", "right now?", "" },
    { "It's not too late", "invest in ZSNES today!", "", "" },
    { "Did you know if you", "buy pagefault beer", "you will make him", "happy?" },
    { "Don't you feel terrible", "knowing you use ZSNES", "and haven't donated", "enough towards it?" },
    { "Why are you playing", "games when you should", "be spending quality time", "with your family?" },
    { "It's best to play", "SNES games while wearing", "boxing gloves.", "" },
    { "Do you think using", "ZSNES increases your", "desire to support", "development?" },
    { "Thank you for playing.", "Presented by", "the ZSNES team!", "" },
    { "What did you load ZSNES", "for? Try another", "SNES emulator.", "" },
    { "Please scan your", "computer for viruses!", "", "" },
    { "We think your computer", "hates you! Be afraid!", "", "" },
    { "Did you know that a large", "percentage of ZSNES was", "created by a fish?", "" },
    { "Winners don't use drugs.", "", "", "" },
#ifndef __UNIXSDL__
    { "You're still using a", "Microsoft OS? Get with", "the program, switch to", "Linux or BSD." }
#else
    { "Come on, use a real", "Operating System like", "Windows, stop being", "different." }
#endif
};

char const* const* horizon_get(u4 const distance)
{
    return horizon[distance % lengthof(horizon)];
}

void CheatCodeSave(void)
{
    FILE* fp = 0;

    GUICBHold = 0;

    if (NumCheats) {
        cheatdata[6] = 254;
        cheatdata[7] = 252;

        setextension(ZSaveName, "cht");

        if ((fp = fopen_dir(ZChtPath, ZSaveName, "wb"))) {
            fwrite(cheatdata, 1, 28 * NumCheats, fp);
            fclose(fp);
        }
    }
}

void CheatCodeLoad(void)
{
    FILE* fp = 0;
    unsigned int cheat_file_size, i, j;

    setextension(ZSaveName, "cht");
    GUICBHold = 0;

    if ((fp = fopen_dir(ZChtPath, ZSaveName, "rb"))) {
        DisableCheatsOnLoad();

        cheat_file_size = fread(cheatdata, 1, 255 * 28, fp);
        fclose(fp);

        if (cheatdata[6] == 254 && cheatdata[7] == 252)
            NumCheats = cheat_file_size / 28;
        else {
            NumCheats = cheat_file_size / 18;
            i = 28 * NumCheats;
            j = cheat_file_size - (cheat_file_size % 18);

            do {
                i -= 28;
                j -= 18;

                memset(&cheatdata[i + 20], 0, 8);
                memmove(&cheatdata[i + 8], &cheatdata[j + 6], 12);
                memmove(&cheatdata[i], &cheatdata[j], 6);
            } while (i > 0);
        }

        EnableCheatsOnLoad();

        if (NumCheats <= GUIcurrentcheatcursloc)
            GUIcurrentcheatcursloc = NumCheats - 1;
        if (NumCheats)
            CheatOn = 1;
        else
            GUIcurrentcheatcursloc = CheatOn = 0;
    }
}

void SaveCheatSearchFile(void)
{
    FILE* fp = 0;

    if ((fp = fopen_dir(ZCfgPath, "tmpchtsr.___", "wb"))) {
        fwrite(vidbuffer + 129600, 1, 65536 * 2 + 32768, fp);
        fclose(fp);
    }
}

void LoadCheatSearchFile(void)
{
    FILE* fp = 0;

    if ((fp = fopen_dir(ZCfgPath, "tmpchtsr.___", "rb"))) {
        fread(vidbuffer + 129600, 1, 65536 * 2 + 32768, fp);
        fclose(fp);
    }
}

void dumpsound(void)
{
    FILE* fp = fopen_dir(ZSpcPath, "sounddmp.raw", "wb");
    if (fp) {
        fwrite(spcBuffera, 1, 65536 * 4 + 4096, fp);
        fclose(fp);
    }
}

static bool snes_extension_match(const char* filename)
{
    char* dot = strrchr(filename, '.');
    if (dot) {
        dot++;
        if (!strcasecmp(dot, "sfc") ||
#ifndef NO_JMA
            !strcasecmp(dot, "jma") ||
#endif
            !strcasecmp(dot, "zip") || !strcasecmp(dot, "gz") || !strcasecmp(dot, "st") || !strcasecmp(dot, "bs") || !strcasecmp(dot, "smc") || !strcasecmp(dot, "swc") || !strcasecmp(dot, "fig") || !strcasecmp(dot, "dx2") || !strcasecmp(dot, "ufo") || !strcasecmp(dot, "gd3") || !strcasecmp(dot, "gd7") || !strcasecmp(dot, "mgd") || !strcasecmp(dot, "mgh") || !strcasecmp(dot, "048") || !strcasecmp(dot, "058") || !strcasecmp(dot, "078") || !strcasecmp(dot, "bin") || !strcasecmp(dot, "usa") || !strcasecmp(dot, "eur") || !strcasecmp(dot, "jap") || !strcasecmp(dot, "aus") || !strcasecmp(dot, "1") || !strcasecmp(dot, "a")) {
            return (true);
        }
    }
    return (false);
}

#define HEADER_SIZE 512
#define INFO_LEN (0xFF - 0xC0)
#define INAME_LEN 21

static const char* get_rom_name(struct dirent_info* entry, char* namebuffer)
{
    int InfoScore(char*);
    unsigned int sum(unsigned char* array, unsigned int size);

    char* last_dot = strrchr(entry->name, '.');
    if (!last_dot || (strcasecmp(last_dot, ".zip") && strcasecmp(last_dot, ".gz") && strcasecmp(last_dot, ".jma"))) {
        if ((entry->size >= 0x8000) && (entry->size <= 0x600000 + HEADER_SIZE)) {
            FILE* fp = fopen_dir(ZRomPath, entry->name, "rb");
            if (fp) {
                unsigned char HeaderBuffer[HEADER_SIZE];
                int HeaderSize = 0, HasHeadScore = 0, NoHeadScore = 0, HeadRemain = entry->size & 0x7FFF;
                bool EHi = false;

                switch (HeadRemain) {
                case 0:
                    NoHeadScore += 3;
                    break;

                case HEADER_SIZE:
                    HasHeadScore += 2;
                    break;
                }

                fread(HeaderBuffer, 1, HEADER_SIZE, fp);

                if (sum(HeaderBuffer, HEADER_SIZE) < 2500) {
                    HasHeadScore += 2;
                }

                // SMC/SWC Header
                if (HeaderBuffer[8] == 0xAA && HeaderBuffer[9] == 0xBB && HeaderBuffer[10] == 4) {
                    HasHeadScore += 3;
                }
                // FIG Header
                else if ((HeaderBuffer[4] == 0x77 && HeaderBuffer[5] == 0x83) || (HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 0x82) || (HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 2) || (HeaderBuffer[4] == 0xF7 && HeaderBuffer[5] == 0x83) || (HeaderBuffer[4] == 0xFD && HeaderBuffer[5] == 0x82) || (HeaderBuffer[4] == 0x00 && HeaderBuffer[5] == 0x80) || (HeaderBuffer[4] == 0x47 && HeaderBuffer[5] == 0x83) || (HeaderBuffer[4] == 0x11 && HeaderBuffer[5] == 2)) {
                    HasHeadScore += 2;
                } else if (!strncmp("GAME DOCTOR SF 3", (char*)HeaderBuffer, 16)) {
                    HasHeadScore += 5;
                }

                HeaderSize = HasHeadScore > NoHeadScore ? HEADER_SIZE : 0;

                if (entry->size - HeaderSize >= 0x500000) {
                    fseek(fp, 0x40FFC0 + HeaderSize, SEEK_SET);
                    fread(HeaderBuffer, 1, INFO_LEN, fp);
                    if (InfoScore((char*)HeaderBuffer) > 1) {
                        EHi = true;
                        strncpy(namebuffer, (char*)HeaderBuffer, INAME_LEN);
                    }
                }

                if (!EHi) {
                    if (entry->size - HeaderSize >= 0x10000) {
                        char LoHead[INFO_LEN], HiHead[INFO_LEN];
                        int LoScore, HiScore;

                        fseek(fp, 0x7FC0 + HeaderSize, SEEK_SET);
                        fread(LoHead, 1, INFO_LEN, fp);
                        LoScore = InfoScore(LoHead);

                        fseek(fp, 0xFFC0 + HeaderSize, SEEK_SET);
                        fread(HiHead, 1, INFO_LEN, fp);
                        HiScore = InfoScore(HiHead);

                        strncpy(namebuffer, LoScore > HiScore ? LoHead : HiHead, INAME_LEN);

                        if (entry->size - HeaderSize >= 0x20000) {
                            int IntLScore;
                            fseek(fp, (entry->size - HeaderSize) / 2 + 0x7FC0 + HeaderSize, SEEK_SET);
                            fread(LoHead, 1, INFO_LEN, fp);
                            IntLScore = InfoScore(LoHead) / 2;

                            if (IntLScore > LoScore && IntLScore > HiScore) {
                                strncpy(namebuffer, LoHead, INAME_LEN);
                            }
                        }
                    } else // ROM only has one block
                    {
                        fseek(fp, 0x7FC0 + HeaderSize, SEEK_SET);
                        fread(namebuffer, INAME_LEN, 1, fp);
                    }
                }
                fclose(fp);
            } else // Couldn't open file
            {
                strcpy(namebuffer, "** READ FAILURE **");
            }
        } else // Smaller than a block, or Larger than 6MB
        {
            strcpy(namebuffer, "** INVALID FILE **");
        }
    } else // Compressed archive
    {
        return (entry->name);
    }
    namebuffer[21] = 0;
    return (namebuffer);
}

char** lf_names = 0; // Long File Names
char** et_names = 0; // Eight Three Names
char** i_names = 0; // Internal Names
char** d_names = 0;

char** selected_names = 0;

#define LIST_LFN BIT(0)
#define LIST_ETN BIT(1)
#define LIST_IN BIT(2)
#define LIST_DN BIT(3)

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

#define swapper(array)       \
    if (array) {             \
        hold = array[x];     \
        array[x] = array[y]; \
        array[y] = hold;     \
    }

static void swapfiles(size_t x, size_t y)
{
    char* hold;
    swapper(lf_names);
    swapper(et_names);
    swapper(i_names);
}

static void swapdirs(size_t x, size_t y)
{
    char* hold = d_names[x];
    d_names[x] = d_names[y];
    d_names[y] = hold;
}

static void sort(intptr_t* array, int begin, int end, void (*swapfunc)(size_t, size_t))
{
    if (end > begin) {
        intptr_t* pivot = array + begin;
        int l = begin + 1;
        int r = end;
        while (l < r) {
            if (strcasecmp((const char*)*(array + l), (const char*)*pivot) <= 0) {
                l++;
            } else {
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

void free_list(char*** list)
{
    char** p = *list;
    if (p) {
        p += 2;
        while (*p) {
            free(*p++);
        }
        free(*list);
        *list = 0;
    }
}

// A possible problem here would be if one of the list arrays got enlarged but a corosponding one ran out of memory
static void add_list(char*** reallist, const char* p)
{
    char** list = *reallist;
    if (!list) {
        if (!(list = malloc(1003 * sizeof(void*)))) {
            return;
        }
        list[0] = (char*)2;
        list[1] = (char*)1002;
        list[2] = 0;
    }

    if (list[0] == list[1] - 1) {
        char** p = realloc(list, ((size_t)list[1] + 1000) * sizeof(void*));
        if (p) {
            list = p;
            list[1] += 1000;
        } else {
            return;
        }
    }

    if ((list[(size_t)*list] = malloc(strlen(p) + 1))) {
        strcpy(list[(size_t)*list], p);
        list[0]++;
        list[(size_t)*list] = 0;
    }
    *reallist = list;
}

// Make sure ZRomPath contains a full absolute directory name before calling
void populate_lists(unsigned int lists, bool snes_ext_match)
{
    DIR* dir;

    if ((lists & LIST_DN) && (strlen(ZRomPath) > ROOT_LEN)) {
        add_list(&d_names, "..");
    }

    if ((dir = opendir(ZRomPath))) {
        struct dirent_info* entry;

        while ((entry = readdir_info(dir))) {
            if (*entry->name != '.') {
                if (S_ISDIR(entry->mode)) {
                    if (lists & LIST_DN) {
                        add_list(&d_names, entry->name);
                    }
                } else if (!snes_ext_match || snes_extension_match(entry->name)) {
                    if (_USE_LFN && (lists & LIST_LFN)) {
                        add_list(&lf_names, entry->name);
                    }

                    if (lists & LIST_IN) {
                        char namebuffer[22];
                        add_list(&i_names, get_rom_name(entry, namebuffer));
                    }

#ifdef __MSDOS__
                    if (lists & LIST_ETN) {
                        if (!_USE_LFN) //_USE_LFN won't be true when running under pure DOS
                        {
                            add_list(&et_names, entry->name);
                        } else {
                            char* sfn = realpath_sfn_dir(ZRomPath, entry->name, 0);
                            if (sfn) {
                                add_list(&et_names, basename(sfn));
                                free(sfn);
                            } else {
                                char sfn[13];
                                _lfn_gen_short_fname(entry->name, sfn);
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

    if (lists & LIST_DN) {
#ifndef __UNIXSDL__
        unsigned int drives = GetLogicalDrives(), i = 0;
#endif

        if (d_names) {
            unsigned int offset = (d_names[2][0] == '.') ? 3 : 2;
            sort((intptr_t*)d_names, offset, (size_t)(*d_names), swapdirs);
        }

#ifndef __UNIXSDL__
        while (i < 26) {
            if (drives & BIT(i)) {
                char drive[] = { '[', 'A', ':', ']', 0 };
                drive[1] = 'A' + i;
                add_list(&d_names, drive);
            }
            i++;
        }
#endif
    }

    if ((lists & LIST_IN) && i_names) {
        sort((intptr_t*)i_names, 2, (size_t)(*i_names), swapfiles);
    } else if ((lists & LIST_LFN) && lf_names) {
        sort((intptr_t*)lf_names, 2, (size_t)(*lf_names), swapfiles);
    } else if ((lists & LIST_ETN) && et_names) {
        sort((intptr_t*)et_names, 2, (size_t)(*et_names), swapfiles);
    }
}

static void memswap(void* p1, void* p2, size_t p2len)
{
    char* ptr1 = (char*)p1;
    char* ptr2 = (char*)p2;

    const size_t p1len = ptr2 - ptr1;
    unsigned char byte;
    while (p2len--) {
        byte = *ptr2++;
        memmove(ptr1 + 1, ptr1, p1len);
        *ptr1++ = byte;
    }
}

void powercycle(bool, bool);

void GUIloadfilename(char* filename)
{
    char* p = strdupcat(ZRomPath, filename);
    if (p) {
        if (init_rom_path(p)) {
            powercycle(false, true);
        }
        free(p);
    }
    if (GUIwinptr) {
        GUIcmenupos = GUIpmenupos;
    }
}

void loadquickfname(u1 const slot)
{
    if (prevloaddnamel[1 + slot * 512]) // replace with better test
    {
        strcpy(ZRomPath, (char*)prevloaddnamel + 1 + slot * 512);
        strcatslash(ZRomPath);
        strcpy(ZCartName, (char*)prevloadfnamel + slot * 512);

        if (!access_dir(ZRomPath, ZCartName, R_OK)) {
            if (slot || !prevlfreeze) {
                // move menuitem to top
                memswap(prevloadiname, prevloadiname + slot * 28, 28);
                memswap(prevloadfnamel, prevloadfnamel + slot * 512, 512);
                memswap(prevloaddnamel, prevloaddnamel + slot * 512, 512);
            }

            GUIloadfilename(ZCartName);
        }
    }
}

void GUIQuickLoadUpdate(void)
{
    size_t entry_size, copy_num, i = 10;
    char* src;

    memcpy(GUIPrevMenuData + 347, (prevlfreeze) ? " ON " : " OFF", 4);

    src = (char*)prevloadiname;
    entry_size = 28;
    copy_num = 28; // full window width

    while (i--) {
        char* p_src = src + i * entry_size;
        char* p_dest = GUIPrevMenuData + 3 + i * 32;
        size_t srclen = strlen(p_src);

        if (srclen >= copy_num) {
            strncpy(p_dest, p_src, copy_num);
            if (srclen > copy_num) {
                memset(p_dest + 25, '.', 3);
            }
        } else {
            strncpy(p_dest, p_src, srclen);
            memset(p_dest + srclen, ' ', 28 - srclen);
        }
    }
}

s4 GUIcurrentviewloc;
s4 GUIcurrentcursloc;
s4 GUIcurrentdirviewloc;
s4 GUIcurrentdircursloc;
s4 GUIdirentries;
s4 GUIfileentries;

void free_all_file_lists()
{
    free_list(&d_names);
    free_list(&i_names);
    free_list(&lf_names);
    free_list(&et_names);
}

void GetLoadData(void)
{
    GUIcurrentviewloc = GUIcurrentcursloc = GUIcurrentdirviewloc = GUIcurrentdircursloc = 0;

    free_all_file_lists();

    switch (GUIloadfntype) {
    case 0: // LFN
        populate_lists(LIST_DN | LIST_ETN | LIST_LFN, !showallext);
        selected_names = lf_names ? lf_names : et_names;
        break;
    case 1: // IN
        populate_lists(LIST_DN | LIST_MAIN | LIST_IN, !showallext);
        selected_names = i_names;
        break;
    default:
        populate_lists(LIST_DN | LIST_MAIN, !showallext);
        selected_names = main_names;
        break;
    }
    selected_names += 2;
    GUIfileentries = main_names ? ((unsigned int)(uintptr_t)(*main_names)) - 2 : 0;
    GUIdirentries = d_names ? ((unsigned int)(uintptr_t)(*d_names)) - 2 : 0;
}

u4 GUIcurrentfilewin;

void GUILoadData(void)
{
    char* nameptr;

    GUICBHold = 0;
    if (GUIcurrentfilewin) // directories
    {
        nameptr = d_names[GUIcurrentdircursloc + 2];

        strcatslash(ZRomPath);
#ifndef __UNIXSDL__
        if ((strlen(nameptr) == 4) && (nameptr[2] == ':')) // MS drives are stored as '[?:]',
        { // so we can't use quick string catenation to browse through
            strncpy(ZRomPath, nameptr + 1, 2);
            ZRomPath[2] = '\\';
            ZRomPath[3] = 0;
        } else
#endif
        {
            if (!strcmp(nameptr, "..")) {
                strdirname(ZRomPath);
            } else {
                strcat(ZRomPath, nameptr);
            }
            strcatslash(ZRomPath);
        }

        GetLoadData();
    } else // files
    {
        nameptr = main_names[GUIcurrentcursloc + 2];

        strcpy(ZCartName, nameptr);

        if (!prevlfreeze) {
            int i = 0;
            bool dupfound = false;
            bool modheader = true;

            while (!dupfound && i < 10) {
                dupfound = (!fnamencmp(nameptr, (char*)prevloadfnamel + i * 512, 512) && (!fnamencmp(ZRomPath, (char*)prevloaddnamel + i * 512 + 1, 512)));
                if (dupfound && modheader) {
                    strncpy((char*)prevloadiname + i * 28, selected_names[GUIcurrentcursloc], 28);
                    prevloadiname[i * 28 + 27] = 0;
                    modheader = false;
                }
                i++;
            }
            i--;

            if (!dupfound) {
                strncpy((char*)prevloadiname + 9 * 28, selected_names[GUIcurrentcursloc], 28);
                prevloadiname[9 * 28 + 27] = 0;
                strcpy((char*)prevloaddnamel + 9 * 512 + 1, ZRomPath);
                strcpy((char*)prevloadfnamel + 9 * 512, ZCartName);
            }

            loadquickfname(i);
        } else {
            GUIloadfilename(ZCartName);
        }

        GUIwinactiv[1] = 0; // close load dialog
        GUIwinorder[--GUIwinptr] = 0;
    }
}

void GUILoadManualDir()
{

    if (*GUILoadTextA) {
        char path_buff[PATH_SIZE];
        bool realpath_success;

        if ((GUILoadPos > ROOT_LEN) && (GUILoadTextA[GUILoadPos - 1] == DIR_SLASH_C)) {
            GUILoadTextA[GUILoadPos - 1] = 0;
        }

        realpath_success = (intptr_t)realpath_dir(ZRomPath, GUILoadTextA, path_buff);
        if (realpath_success) {
            struct stat stat_buffer;
            if (!stat(path_buff, &stat_buffer)) {
                if (S_ISDIR(stat_buffer.st_mode)) {
                    strcpy(ZRomPath, path_buff);
                    strcatslash(ZRomPath);
                    GetLoadData();
                } else {
                    if (init_rom_path(path_buff)) {
                        powercycle(false, true);
                    }
                }
                return;
            }
        }
    }

    GUILoadData();
}

u4 GUILoadKeysNavigate(u1 const gui_key_extended)
{
    int *currentviewloc, *currentcursloc, *entries;
    if (GUIcurrentfilewin == 1) {
        currentviewloc = &GUIcurrentdirviewloc;
        currentcursloc = &GUIcurrentdircursloc;
        entries = &GUIdirentries;
    } else {
        currentviewloc = &GUIcurrentviewloc;
        currentcursloc = &GUIcurrentcursloc;
        entries = &GUIfileentries;
    }

    // Handle left and right
    if (GUIfileentries && GUIdirentries) {
#ifdef __UNIXSDL__
        if ((gui_key_extended == 92) || ((numlockptr != 1) && (gui_key_extended == 75)))
#else
        if (gui_key_extended == 75)
#endif
        {
            GUILoadPos = 0;
            GUIcurrentfilewin ^= 1;
            return (1);
        }

#ifdef __UNIXSDL__
        if ((gui_key_extended == 94) || ((numlockptr != 1) && (gui_key_extended == 77)))
#else
        if (gui_key_extended == 77)
#endif
        {
            GUILoadPos = 0;
            GUIcurrentfilewin ^= 1;
            return (1);
        }
    }

    // Enter press
    if (gui_key_extended == 13) {
        GUILoadPos = 0;
        GUILoadManualDir();
        return (1);
    }

// Home key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 89) || ((numlockptr != 1) && (gui_key_extended == 71)))
#else
    if (gui_key_extended == 71)
#endif
    {
        GUILoadPos = 0;
        *currentcursloc = 0;
        *currentviewloc = 0;
        return (1);
    }

// End key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 95) || ((numlockptr != 1) && (gui_key_extended == 79)))
#else
    if (gui_key_extended == 79)
#endif
    {
        GUILoadPos = 0;
        *currentcursloc = (*entries) - 1;
        *currentviewloc = (*entries) - 15;
        if (*currentviewloc < 0) {
            *currentviewloc = 0;
        }
        return (1);
    }

// Up arrow key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 90) || ((numlockptr != 1) && (gui_key_extended == 72)))
#else
    if (gui_key_extended == 72)
#endif
    {
        GUILoadPos = 0;
        if (*currentcursloc) {
            if (*currentviewloc == *currentcursloc) {
                (*currentviewloc)--;
            }
            (*currentcursloc)--;
        }
        return (1);
    }

// Down arrow key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 96) || ((numlockptr != 1) && (gui_key_extended == 80)))
#else
    if (gui_key_extended == 80)
#endif
    {
        GUILoadPos = 0;
        if ((*currentcursloc) + 1 != *entries) {
            (*currentcursloc)++;
            if ((*currentcursloc) - 15 == *currentviewloc) {
                (*currentviewloc)++;
            }
        }
        return (1);
    }

// Page up key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 91) || ((numlockptr != 1) && (gui_key_extended == 73)))
#else
    if (gui_key_extended == 73)
#endif
    {
        GUILoadPos = 0;
        *currentviewloc -= 15;
        *currentcursloc -= 15;
        if (*currentviewloc < 0) {
            *currentviewloc = 0;
        }
        if (*currentcursloc < 0) {
            *currentcursloc = 0;
        }
        return (1);
    }

// Page down key
#ifdef __UNIXSDL__
    if ((gui_key_extended == 97) || ((numlockptr != 1) && (gui_key_extended == 81)))
#else
    if (gui_key_extended == 81)
#endif
    {
        GUILoadPos = 0;
        *currentviewloc += 15;
        *currentcursloc += 15;
        if (*currentcursloc >= (*entries) - 1) {
            *currentcursloc = (*entries) - 1;
        }
        if (*currentviewloc >= (*entries) - 15) {
            *currentviewloc = ((*entries) - 15) > 0 ? (*entries) - 15 : 0;
        }
        return (1);
    }

    return (0);
}

#ifdef __UNIXSDL__
#define DriveCount() 0
#else
static unsigned int DriveCount()
{
    unsigned int drives = GetLogicalDrives(), count = 0, i = 0;
    while (i < 26) {
        if (drives & BIT(i)) {
            count++;
        }
        i++;
    }
    return (count);
}
#endif

s4 GUIJT_entries;
s4 GUIJT_offset;
s4 GUIJT_viewable;
s4* GUIJT_currentcursloc;
s4* GUIJT_currentviewloc;

void GUIGenericJumpTo(void)
{
    int mid = GUIJT_viewable >> 1;
    *GUIJT_currentviewloc = (GUIJT_offset < GUIJT_entries - mid) ? GUIJT_offset - mid : GUIJT_entries - GUIJT_viewable;
    if (*GUIJT_currentviewloc < 0) {
        *GUIJT_currentviewloc = 0;
    }
    *GUIJT_currentcursloc = GUIJT_offset;
}

void GUILoadKeysJumpTo(void)
{
    char** base;
    int start, end;

    GUILoadTextA[GUILoadPos] = 0;

    if (GUIcurrentfilewin == 1) {
        GUIJT_currentviewloc = &GUIcurrentdirviewloc;
        GUIJT_currentcursloc = &GUIcurrentdircursloc;
        GUIJT_entries = GUIdirentries;
        base = d_names + 2;
        if (!strcmp(*base, "..")) {
            base++;
            GUIJT_entries--;
        }

        GUIJT_entries -= DriveCount();
    } else {
        GUIJT_currentviewloc = &GUIcurrentviewloc;
        GUIJT_currentcursloc = &GUIcurrentcursloc;
        GUIJT_entries = GUIfileentries;
        base = selected_names;
    }

    start = 0;
    end = GUIJT_entries - 1;
    GUIJT_offset = GUIJT_entries;
    if (!strcmp(GUILoadTextA, " ")) // Exactly a space picks a game randomely
    {
        GUIJT_offset = rand() % GUIJT_entries;
    } else {
        while (start <= end) {
            int mid = (start + end) >> 1;
            int pos = strncasecmp(base[mid], GUILoadTextA, GUILoadPos);
            if (!pos) {
                do {
                    GUIJT_offset = mid--;
                } while ((mid >= 0) && !strncasecmp(base[mid], GUILoadTextA, GUILoadPos));
                break;
            }
            if (pos > 0) {
                end = mid - 1;
            } else {
                start = mid + 1;
            }
        }
    }

    if (GUIJT_offset < GUIJT_entries) {
        if (GUIcurrentfilewin == 1) {
            GUIJT_entries += DriveCount();
            if (base > d_names + 2) {
                GUIJT_offset++;
                GUIJT_entries++;
            }
        }

        GUIJT_viewable = 15;
        GUIGenericJumpTo();
    }
}

// Not entirely accurate pow, but good for most needs and very fast
static unsigned int npow(register unsigned int base, register unsigned int exponent)
{
    register unsigned int total = 1;
    if (exponent) {
        register unsigned int i;
        for (i = 2, total = base; i < exponent; i += i) {
            total *= total;
        }
        for (i >>= 1; i < exponent; i++) {
            total *= base;
        }
    }
    return (total);
}

static void int_to_str(char* dest, unsigned int len, unsigned int num)
{
    *dest = 0;
    if (len && (num < npow(10, len))) {
        int i;
        for (i = 1; num; i++) {
            memmove(dest + 1, dest, i);
            *dest = (num % 10) + '0';
            num /= 10;
        }
    }
}

#ifndef __MSDOS__

char GUICustomX[5], GUICustomY[5];
void GetCustomXY(void)
{
    static bool first_time = true;
    if (first_time) {
        int_to_str(GUICustomX, 4, CustomResX);
        int_to_str(GUICustomY, 4, CustomResY);
        first_time = false;
    }
}

void SetCustomXY(void)
{
    if (!((atoi(GUICustomX) < 256) || (atoi(GUICustomX) > 2048) || (atoi(GUICustomY) < 224) || (atoi(GUICustomY) > 1536))) {
        CustomResX = atoi(GUICustomX);
        CustomResY = atoi(GUICustomY);
        if (CustomResX < 298)
            Keep4_3Ratio = 0;
    }
}

bool Keep43Check(void)
{
    return CustomResX * 3 != CustomResY * 4;
}

char CheckOGLMode()
{
    return (GUIBIFIL[cvidmode]);
}

#endif

extern unsigned int MovieForcedLength;
char GUIMovieForcedText[11];

void GetMovieForcedLength(void)
{
    static bool first_time = true;
    if (first_time) {
        int_to_str(GUIMovieForcedText, 10, MovieForcedLength);
        first_time = false;
    }
}

void SetMovieForcedLength(void)
{
    MovieForcedLength = atoi(GUIMovieForcedText);
}
