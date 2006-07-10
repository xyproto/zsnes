/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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
#else
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#include "../zpath.h"
#include "ntsc.h"

extern unsigned char newengen;
extern unsigned int nggposng[];
extern unsigned short PrevPicture[64*56], *vidbuffer, *vidbufferofsb;

void CapturePicture()
{
  unsigned short work1, work2, filter;
  unsigned int i, j, offset, pppos=0;

  if (newengen && ((*nggposng & 0xFF) == 5))
  {
    filter = 0x7BDE;	// 0111 1011 1101 1110
  }
  else
  {
    filter = 0xF7DE;	// 1111 0111 1101 1110
  }

  for (j=0 ; j<56 ; j++)
  {
    offset = 288+16+j*288*4;

    for (i=0 ; i<64 ; i++)
    {
      work1 = ((vidbuffer[offset] & filter)>>1) + ((vidbuffer[offset+2] & filter)>>1);
      work2 = ((vidbuffer[offset+288] & filter)>>1) + ((vidbuffer[offset+288+2] & filter)>>1);
      PrevPicture[pppos] = ((work1 & filter)>>1) + ((work2 & filter)>>1);
      offset += 4;
      pppos++;
    }
  }

  if (newengen && ((*nggposng & 0xFF) == 5))
  {
    for (pppos=0 ; pppos<64*56 ; pppos++)
    {
      PrevPicture[pppos] = ((PrevPicture[pppos] & 0x7FE0)<<1)|(PrevPicture[pppos] & 0x001F);
    } // 0111 1111 1110 0000 and 0000 0000 0001 1111
  }
}

extern unsigned char MovieProcessing;
extern unsigned int cur_zst_size, old_zst_size;

void mzt_chdir_up();
void mzt_chdir_down();

void LoadPicture()
{
  const unsigned int pic_size = 64*56*2;
  FILE *fp;

  memset(PrevPicture, 0, pic_size);

  if (MovieProcessing) { mzt_chdir_up(); }
  if ((fp = fopen_dir(ZSramPath, ZStateName, "rb")))
  {
    unsigned int file_size;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    if ((file_size-pic_size == cur_zst_size) || (file_size-pic_size == old_zst_size))
    {
      fseek(fp, -((signed)pic_size), SEEK_END);
      fread(PrevPicture, 1, pic_size, fp);
    }

    fclose(fp);
  }
  if (MovieProcessing) { mzt_chdir_down(); }
}

void Clear2xSaIBuffer()
{
  memset(vidbufferofsb+288, 0xFF, 576*239);
}

// NTSC filter variables

unsigned char ntsc_phase = 0;
snes_ntsc_setup_t ntsc_setup;
snes_ntsc_t ntsc_snes;
extern unsigned char NTSCBlend;
extern signed char NTSCHue, NTSCSat, NTSCCont, NTSCBright, NTSCSharp, NTSCWarp;

// Init NTSC filter command, should be called whenever changes are made in the GUI related to the GUI
void NTSCFilterInit()
{
  // Set GUI options
  ntsc_setup.hue = (float)(NTSCHue / 100.0);
  ntsc_setup.saturation = (float)(NTSCSat / 100.0);
  ntsc_setup.contrast = (float)(NTSCCont / 100.0);
  ntsc_setup.brightness = (float)(NTSCBright / 100.0);
  ntsc_setup.sharpness = (float)(NTSCSharp / 100.0);
  ntsc_setup.hue_warping = (float)(NTSCWarp / 100.0);
  ntsc_setup.merge_fields = (int) NTSCBlend;
  snes_ntsc_init(&ntsc_snes, &ntsc_setup);
}

void NTSCFilterDraw(int SurfaceX, int SurfaceY, int pitch, unsigned char *buffer)
{
  snes_ntsc_blit(&ntsc_snes, vidbuffer+16, 576, ntsc_phase, SurfaceX, SurfaceY, buffer, pitch);

  // Change phase on alternating frames
  ntsc_phase ^= 1;
}

extern unsigned int statefileloc;

unsigned char newestfileloc;
time_t newestfiledate;

void DetermineNew()
{
  struct stat filestat;

  if (MovieProcessing) { mzt_chdir_up(); }
  if (!stat_dir(ZSramPath, ZStateName, &filestat) && filestat.st_mtime > newestfiledate)
  {
    newestfiledate = filestat.st_mtime;
    newestfileloc = ZStateName[statefileloc] == 't' ? 0 : ZStateName[statefileloc]-'0';
  }
  if (MovieProcessing) { mzt_chdir_down(); }
}

int StateExists()
{
  int ret;

  if (MovieProcessing) { mzt_chdir_up(); }
  ret = access_dir(ZSramPath, ZStateName, F_OK) ? 0 : 1;
  if (MovieProcessing) { mzt_chdir_down(); }

  return(ret);
}
