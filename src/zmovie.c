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
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define DIR_SLASH "\\"
#endif
#include "gblvars.h"
#include "asm_call.h"

#define bool unsigned char

struct
{
  FILE *fp;
  char linebuf[256];
  size_t frame_current;
  size_t message_start;
  size_t message_duration;
} MovieSub;

void MovieSub_Open(const char *filename)
{
  MovieSub.fp = fopen(filename, "r");
  MovieSub.frame_current = 0;
  MovieSub.message_start = 0;
  MovieSub.message_duration = 0;
}

void MovieSub_Close()
{
  fclose(MovieSub.fp);
  MovieSub.fp = 0;
}

char *MovieSub_GetData()
{
  if (MovieSub.fp)
  {
    char *i, *num;
    
    MovieSub.frame_current++;
        
    if (MovieSub.frame_current > MovieSub.message_start + MovieSub.message_duration)
    {
      MovieSub.message_duration = 0;
      fgets(MovieSub.linebuf, 256, MovieSub.fp);
      if (!(num = strtok(MovieSub.linebuf, ":"))) { return(0); }
      for (i = num; *i; i++)
      {
        if (!isascii(*i)) { return(0); }
      }
      MovieSub.message_start = atoi(num);
      if (!(num = strtok(0, ":"))) { return(0); }
      for (i = num; *i; i++)
      {
        if (!isascii(*i))
        { 
          MovieSub.message_start = 0;
          return(0);
        }
      }
      MovieSub.message_duration = atoi(num);
    }
  
    if (MovieSub.frame_current == MovieSub.message_start)
    {
      return(strtok(0, ":"));
    }
  }
  return(0);
}

size_t MovieSub_GetDuration()
{
  return(MovieSub.message_duration);
}


extern unsigned int PJoyAOrig, PJoyBOrig, PJoyCOrig, PJoyDOrig, PJoyEOrig;
extern unsigned int JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;
extern unsigned int MsgCount, MessageOn;
extern unsigned char MovieTemp, txtmovieended[15], MovieProcessing, *Msgptr;

static FILE *movfhandle;

void Replay()
{
  if (fread(&MovieTemp, 1, 1, movfhandle))
  {
    if (MovieTemp < 2) // 1 or 0 are correct values
    {
      char *sub;
      
      if (MovieTemp == 0) // 0 means the input has changed
      {
	fread(&PJoyAOrig, 1, 4, movfhandle);
	fread(&PJoyBOrig, 1, 4, movfhandle);
	fread(&PJoyCOrig, 1, 4, movfhandle);
	fread(&PJoyDOrig, 1, 4, movfhandle);
	fread(&PJoyEOrig, 1, 4, movfhandle);
      }

      JoyAOrig = PJoyAOrig;
      JoyBOrig = PJoyBOrig;
      JoyCOrig = PJoyCOrig;
      JoyDOrig = PJoyDOrig;
      JoyEOrig = PJoyEOrig;
    
      if ((sub = MovieSub_GetData()))
      {
        Msgptr = sub;
        MessageOn = MovieSub_GetDuration();
      }
    }
    else // anything else is bad - the file isn't a movie.
    {
      MovieProcessing = 0;

      fclose(movfhandle);
      MovieSub_Close();
    }
  }
  else
  {
    Msgptr = txtmovieended;
    MessageOn = MsgCount;
    MovieProcessing = 0;

    fclose(movfhandle);
    MovieSub_Close();
  }
}

extern unsigned int MovieBuffFrame, MovieBuffSize;
unsigned char MovieBuffer[21*60];

void IncFrameWriteBuffer()
{
  MovieBuffFrame++;

  if (MovieBuffFrame == 60)
  {
    fwrite(MovieBuffer, 1, MovieBuffSize, movfhandle);

    MovieBuffSize = 0;
    MovieBuffFrame = 0;
  }
}

void intsplitter (unsigned char *buffer, unsigned int offset, unsigned int value)
{
  unsigned char i;

  for (i=0 ; i<4 ; i++)
  {
    buffer[offset + i] = ((value >> i*8) & 0xFF);
  }
}

unsigned int bytemerger (unsigned char heaviest, unsigned char heavy, unsigned char light, unsigned char lightest)
{
  return ((heaviest << 24) | (heavy << 16) | (light << 8) | (lightest));
}

extern unsigned int CReadHead, ReadHead, CFWriteStart, CFWriteHead;
extern unsigned char BackState, CNetType, StoreBuffer[128*32];

void Record()
{
  unsigned int offst, PJoyATemp, PJoyBTemp, PJoyCTemp, PJoyDTemp, PJoyETemp;

  if ((BackState == 1) && (CNetType >= 20))
  {
    if (CReadHead == ReadHead)
    {
      CFWriteStart++;
      CFWriteStart &= 0x7F;

      if (CFWriteStart == CFWriteHead)
      {
	offst = (CFWriteHead << 5);

	offst++;
	PJoyATemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyBTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyCTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyDTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyETemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;

//	if (StoreBuffer[offst]) - commented out in the ASM.
	if ((PJoyAOrig == PJoyATemp) && (PJoyBOrig == PJoyBTemp) && (PJoyCOrig == PJoyCTemp) && (PJoyDOrig == PJoyDTemp) && (PJoyEOrig == PJoyETemp))
	{
	  MovieBuffer[MovieBuffSize] = 1;
	  MovieBuffSize++;
	}
	else
	{
	  PJoyAOrig = PJoyATemp;
	  PJoyBOrig = PJoyBTemp;
	  PJoyCOrig = PJoyCTemp;
	  PJoyDOrig = PJoyDTemp;
	  PJoyEOrig = PJoyETemp;

	  MovieBuffer[MovieBuffSize] = 0;
	  MovieBuffSize++;

	  intsplitter (MovieBuffer, MovieBuffSize, PJoyAOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyBOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyCOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyDOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyEOrig);
	  MovieBuffSize += 4;
	}

	IncFrameWriteBuffer();

	CFWriteHead++;
	CFWriteHead &= 0x7F;
      }

      CReadHead++;
      CReadHead &= 0x7F;
    }

    offst = (ReadHead << 5);
    StoreBuffer[offst] = 0;
    offst++;

    intsplitter (StoreBuffer, offst, JoyAOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyBOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyCOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyDOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyEOrig);
    offst +=4;

    ReadHead++;
    ReadHead &= 0x7F;
  }
  else
  {
    if ((PJoyAOrig != JoyAOrig) || (PJoyBOrig != JoyBOrig) || (PJoyCOrig != JoyCOrig) || (PJoyDOrig != JoyDOrig) || (PJoyEOrig != JoyEOrig))
    {
      PJoyAOrig = JoyAOrig;
      PJoyBOrig = JoyBOrig;
      PJoyCOrig = JoyCOrig;
      PJoyDOrig = JoyDOrig;
      PJoyEOrig = JoyEOrig;
      MovieTemp = 0;
      MovieBuffer[MovieBuffSize] = 0;
      MovieBuffSize++;

      intsplitter (MovieBuffer, MovieBuffSize, JoyAOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyBOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyCOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyDOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyEOrig);
      MovieBuffSize += 4;
    }
    else
    {
      MovieTemp = 1;
      MovieBuffer[MovieBuffSize] = 1;
      MovieBuffSize++;
    }

    IncFrameWriteBuffer();
  }
}

void ProcessMovies()
{
  if (MovieProcessing == 2)	{ Record(); }
  else	{ Replay(); }
}

// The following will maybe end up in guic.c once we get it started.
// It came from guiwindp.inc and gui.asm, after all
extern unsigned char MovieRecordWinVal;
extern unsigned int GUICBHold;

void SkipMovie() 
{
  MovieRecordWinVal = 0;
  GUICBHold &= 0xFFFFFF00;
}

void MovieStop()
{
  if (MovieProcessing)
  {
    fclose(movfhandle);
    MovieProcessing = 0;
  }
}

extern unsigned int MovieCounter, statefileloc, Totalbyteloaded, curexecstate;
extern unsigned int nmiprevaddrl, nmiprevaddrh, nmirept, nmiprevline, nmistatus;
extern unsigned char GUIQuit, fnamest[512], CMovieExt, RecData[16], soundon;
extern unsigned char NextLineCache, sramsavedis, UseRemoteSRAMData;
extern unsigned char UnableMovie2[24], UnableMovie3[23];

void SRAMChdir();
void loadstate2();
void ChangetoLOADdir();

void MoviePlay()
{
  unsigned char FileExt[4];

  GUICBHold &= 0xFFFFFF00;

  if (CNetType != 20)
  {
    MovieCounter= 0;

    if (!MovieProcessing)
    {
      GUIQuit = 2;
      memcpy (FileExt, &fnamest[statefileloc-3], 4);
      memcpy (&fnamest[statefileloc-3], ".zmv", 4);
      fnamest[statefileloc] = CMovieExt;

      SRAMChdir();

      loadstate2();

      if ((movfhandle = fopen(fnamest+1,"rb")) != NULL)
      {
	memcpy(&fnamest[statefileloc-3], ".sub", 4);
        if (isdigit(CMovieExt)) { fnamest[statefileloc] = CMovieExt; }
	MovieSub_Open(fnamest+1);
	
	fseek(movfhandle, Totalbyteloaded, SEEK_SET);
	fread(RecData, 1, 16, movfhandle);
	printf("Movie made with version: %d\n", RecData[1]);

	if (RecData[2] == 1)
	{
	  timer2upd = bytemerger(RecData[6], RecData[5], RecData[4], RecData[3]);
	  curexecstate = bytemerger(RecData[10], RecData[9], RecData[9], RecData[7]);
	  nmiprevaddrl = 0;
	  nmiprevaddrh = 0;
	  nmirept = 0;
	  nmiprevline = 224;
	  nmistatus = 0;
	  spcnumread = 0;
	  spchalted = 0xFFFFFFFF;
	  NextLineCache = 0;
	}

	if (soundon == RecData[0])
	{
	  if (ramsize)	{ fread(sram, 1, ramsize, movfhandle); }

	  MovieProcessing = 1;
	  PJoyAOrig = 0;
	  PJoyBOrig = 0;
	  PJoyCOrig = 0;
	  PJoyDOrig = 0;
	  PJoyEOrig = 0;
	  sramsavedis = 1;
	  UseRemoteSRAMData = 0;
	  DSPMem[0x08] = 0;
	  DSPMem[0x18] = 0;
	  DSPMem[0x28] = 0;
	  DSPMem[0x38] = 0;
	  DSPMem[0x48] = 0;
	  DSPMem[0x58] = 0;
	  DSPMem[0x68] = 0;
	  DSPMem[0x78] = 0;

	  memcpy (&fnamest[statefileloc-3], FileExt, 4);
	}
	else
	{
	  Msgptr = (!soundon) ? UnableMovie3 : UnableMovie2;
	  MessageOn = MsgCount;
	  fclose(movfhandle);
	}
      }
      else
      {

	memcpy (&fnamest[statefileloc-3], FileExt, 4);
      }

      asm_call(ChangetoLOADdir);
    }
  }
}

extern unsigned char NoPictureSave;
extern unsigned int versionNumber;

void statesaver();

void MovieRecord()
{
  unsigned char FileExt[4];

  FILE *tempfhandle;

  GUICBHold &= 0xFFFFFF00;

  if (!MovieProcessing)
  {
    MovieCounter = 0;
    memcpy (FileExt, &fnamest[statefileloc-3], 4);
    memcpy (&fnamest[statefileloc-3], ".zmv", 4);
    fnamest[statefileloc] = CMovieExt;
    tempfhandle = fopen(fnamest+1,"rb");

	// check if file exists
    if ((MovieRecordWinVal == 1) || (tempfhandle == NULL))
    {
      if (!MovieProcessing)
      {
	CFWriteHead = 0;
	CReadHead = 0;
	ReadHead = 0;
	CFWriteStart = 64;
      }

      MovieRecordWinVal = 0;

      SRAMChdir();

      NoPictureSave = 1;
	// saves the statedata as first chunk of movie
      if (!MovieProcessing)	{ statesaver(); }

      NoPictureSave = 0;
	// it shouldn't fail, but paranoids can add an 'if'...
      movfhandle = fopen(fnamest+1,"r+b");
      fseek(movfhandle, 0, SEEK_END);

      if (!MovieProcessing)
      {
	RecData[0] = soundon;
	RecData[1] = (versionNumber & 0xFF); // valid for versions under 2.56
	RecData[2] = 1;
	intsplitter (RecData, 3, timer2upd);
	intsplitter (RecData, 7, curexecstate);
	fwrite(RecData, 1, 16, movfhandle);

	if (ramsize)	{ fwrite(sram, 1, ramsize, movfhandle); }

	MovieBuffSize = 0;
	MovieBuffFrame = 0;

	if ((CNetType != 20) && (CNetType != 21))
	{
	  nmiprevaddrl = 0;
	  nmiprevaddrh = 0;
	  nmirept = 0;
	  nmiprevline = 224;
	  nmistatus = 0;
	  spcnumread = 0;
	  spchalted = 0xFFFFFFFF;
	  NextLineCache = 0;
	  PJoyAOrig = 0;
	  PJoyBOrig = 0;
	  PJoyCOrig = 0;
	  PJoyDOrig = 0;
	  PJoyEOrig = 0;
	  DSPMem[0x08] = 0;
	  DSPMem[0x18] = 0;
	  DSPMem[0x28] = 0;
	  DSPMem[0x38] = 0;
	  DSPMem[0x48] = 0;
	  DSPMem[0x58] = 0;
	  DSPMem[0x68] = 0;
	  DSPMem[0x78] = 0;
	}
      }

      MovieProcessing = 2;

      asm_call(ChangetoLOADdir);
    }
    else
    {
      fclose(tempfhandle);
      MovieRecordWinVal = 1;
    }
    memcpy (&fnamest[statefileloc-3], FileExt, 4);
  }
}
