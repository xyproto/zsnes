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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#endif
#include "zip/zunzip.h"

#ifndef __GNUC__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

#define BUFFER_SIZE 2048

extern int maxromspace;
extern int curromspace;
extern int NumofBytes;
extern int NumofBanks;
extern unsigned int *romdata;
extern bool IPSPatched;
extern unsigned char Header512;
extern bool AutoPatch;
void *doMemAlloc(int);
char *patchfile;

struct
{
  unsigned int file_size;
  unsigned char *data;
  unsigned char *current;
  unsigned int buffer_total;
  unsigned int proccessed;

  unzFile zipfile;
  FILE *fp;
} IPSPatch;


bool reloadBuffer()
{
  if (IPSPatch.proccessed == IPSPatch.file_size) { return(false); }

  IPSPatch.buffer_total = IPSPatch.fp ?
  /* Regular Files */     fread(IPSPatch.data, 1, BUFFER_SIZE, IPSPatch.fp) :
  /* Zip Files     */     unzReadCurrentFile(IPSPatch.zipfile, IPSPatch.data, BUFFER_SIZE);

  IPSPatch.current = IPSPatch.data;
  if (IPSPatch.buffer_total && (IPSPatch.buffer_total <= BUFFER_SIZE))
  {
    return(true);
  }

  IPSPatch.buffer_total = 0;
  return(false);
}

int IPSget()
{
  int retVal;
  if (IPSPatch.current == IPSPatch.data + IPSPatch.buffer_total)
  {
    if (!reloadBuffer()) { return(-1); }
  }
  IPSPatch.proccessed++;
  retVal = *IPSPatch.current;
  IPSPatch.current++;
  return(retVal);
}

bool initPatch()
{
  struct stat stat_results;
  stat(patchfile, &stat_results);

  IPSPatch.file_size = (unsigned int)stat_results.st_size;
  IPSPatch.data = (unsigned char *)doMemAlloc(BUFFER_SIZE);
  IPSPatch.proccessed = 0;

  IPSPatch.zipfile = 0;

  IPSPatch.fp = 0;
  IPSPatch.fp = fopen(patchfile, "rb");
  if (!IPSPatch.fp) { return(false); }

  return(reloadBuffer());
}

void deinitPatch()
{
  if (IPSPatch.data)
  {
    free(IPSPatch.data);
    IPSPatch.data = 0;
  }

  if (IPSPatch.fp)
  {
    fclose(IPSPatch.fp);
    IPSPatch.fp = 0;
  }

  if (IPSPatch.zipfile)
  {
    unzCloseCurrentFile(IPSPatch.zipfile);
    unzClose(IPSPatch.zipfile);
    IPSPatch.zipfile = 0;
  }
}


void PatchUsingIPS()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int location = 0, length = 0, last = 0;
  int sub = Header512 ? 512 : 0;

  IPSPatched = false;

  if (!AutoPatch)
  {
    deinitPatch(); //Needed if the call to this function was done from findZipIPS()
    return;
  }

  if (patchfile) //Regular file, not Zip
  {
    if (!initPatch())
    {
      deinitPatch(); //Needed because if it didn't fully init, some things could have
      return;
    }
  }

  //Yup, it's goto! :)
  //See 'IPSDone:' for explanation
  if (IPSget() != 'P') { goto IPSDone; }
  if (IPSget() != 'A') { goto IPSDone; }
  if (IPSget() != 'T') { goto IPSDone; }
  if (IPSget() != 'C') { goto IPSDone; }
  if (IPSget() != 'H') { goto IPSDone; }

  while (IPSPatch.proccessed != IPSPatch.file_size)
  {
    //Location is a 3 byte value (max 16MB)
    int inloc = (IPSget() << 16) | (IPSget() << 8) | IPSget();

    if (inloc == 0x454f46) //EOF
    {
      break;
    }

    //Offset by size of ROM header
    location = inloc - sub;

    //Length is a 2 byte value (max 64KB)
    length = (IPSget() << 8) | IPSget();

    if (length) // Not RLE
    {
      int i;
      for (i = 0; i < length; i++, location++)
      {
        if (location >= 0)
        {
          if (location >= maxromspace) { goto IPSDone; }
          ROM[location] = (unsigned char)IPSget();
          if (location > last) { last = location; }
        }
        else
        {
          IPSget(); //Need to skip the bytes that write to header
        }
      }
    }
    else //RLE
    {
      int i;
      unsigned char newVal;
      length = (IPSget() << 8) | IPSget();
      newVal = (unsigned char)IPSget();
      for (i = 0; i < length; i++, location++)
      {
        if (location >= 0)
        {
          if (location >= maxromspace) { goto IPSDone; }
          ROM[location] = newVal;
          if (location > last) { last = location; }
        }
      }
    }
  }

  //We use gotos to break out of the nested loops,
  //as well as a simple way to check for 'PATCH' in
  //some cases like this one, goto is the way to go.
  IPSDone:

  deinitPatch();

  IPSPatched = true;

  //Adjust size values if the ROM was expanded
  if (last > curromspace)
  {
    curromspace = last;
    NumofBytes = last;
    NumofBanks = NumofBytes/32768;
  }

  /*
  //Write out patched ROM
  {
    FILE *fp = 0;
    fp = fopen("zsnes.rom", "wb");
    if (!fp) { perror(0); asm volatile("int $3"); }
    fwrite(ROM, 1, curromspace, fp);
    fclose(fp);
  }
  */
}

void findZipIPS(char *compressedfile)
{
  bool FoundIPS = false;
  unz_file_info cFileInfo; //Create variable to hold info for a compressed file
  int cFile;

  IPSPatch.zipfile = unzOpen(compressedfile); //Open zip file
  cFile = unzGoToFirstFile(IPSPatch.zipfile); //Set cFile to first compressed file

  while(cFile == UNZ_OK) //While not at end of compressed file list
  {
    //Temporary char array for file name
    char cFileName[256];

    //Gets info on current file, and places it in cFileInfo
    unzGetCurrentFileInfo(IPSPatch.zipfile, &cFileInfo, cFileName, 256, NULL, 0, NULL, 0);

    //Find IPS file
    if (strlen(cFileName) >= 5) //Char + ".IPS"
    {
      char *ext = cFileName+strlen(cFileName)-4;
      if (!strncasecmp(ext, ".IPS", 4))
      {
        FoundIPS = true;
        break;
      }
    }

    //Go to next file in zip file
    cFile = unzGoToNextFile(IPSPatch.zipfile);
  }

  if (!FoundIPS)
  {
    unzClose(IPSPatch.zipfile);
    IPSPatch.zipfile = 0;
    return;
  }

  //Open file
  unzOpenCurrentFile(IPSPatch.zipfile);

  patchfile = 0;
  IPSPatch.file_size = (unsigned int)cFileInfo.uncompressed_size;
  IPSPatch.data = (unsigned char *)doMemAlloc(BUFFER_SIZE);
  IPSPatch.proccessed = 0;
  IPSPatch.fp = 0;
  reloadBuffer();
  PatchUsingIPS();
}



