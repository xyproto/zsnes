/*
Copyright (C) 2003 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )

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

#include <stdio.h>

extern int maxromspace;
extern int curromspace;
extern int NumofBytes;
extern int NumofBanks;
extern unsigned int *romdata;
extern unsigned char IPSPatched;

char *patchfile;

void PatchUsingIPS()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int location = 0, length = 0;
  
  FILE *fp = 0;
  fp = fopen(patchfile, "rb");
  if (!fp) { return; }
  
  IPSPatched = 0;

  //Yup, it's goto! :)
  //See 'IPSDone:' for explanation
  if (fgetc(fp) != 'P') { goto IPSDone; }
  if (fgetc(fp) != 'A') { goto IPSDone; }
  if (fgetc(fp) != 'T') { goto IPSDone; }
  if (fgetc(fp) != 'C') { goto IPSDone; }
  if (fgetc(fp) != 'H') { goto IPSDone; }

  while (!feof(fp))
  {
    //Location is a 3 byte value (max 16MB)
    int inloc = (fgetc(fp) << 16) | (fgetc(fp) << 8) | fgetc(fp);

    if (inloc == 0x454f46) //EOF
    { 
      break;
    }

    //We assume all IPS files are for ROMs with headers
    location = inloc - 512;

    //Length is a 2 byte value (max 64KB)
    length = (fgetc(fp) << 8) | fgetc(fp);

    if (length) // Not RLE 
    {
      int i;
      for (i = 0; i < length; i++, location++)
      {
        if (location >= 0)
        {
          if (location >= maxromspace) { goto IPSDone; }
          ROM[location] = (unsigned char)fgetc(fp);
        }
        else
        {
          fgetc(fp); //Need to skip the bytes that write to header
        }
      }
    }
    else //RLE
    {
      int i;
      unsigned char newVal;
      length = (fgetc(fp) << 8) | fgetc(fp); 
      newVal = (unsigned char)fgetc(fp);
      for (i = 0; i < length; i++, location++)
      {
        if (location >= 0)
        {
          if (location >= maxromspace) { goto IPSDone; }
          ROM[location] = newVal;
        }
      }
    }
  }
  
  //We use gotos to break out of the nested loops,
  //as well as a simple way to check for 'PATCH' in
  //some cases like this one, goto is the way to go.
  IPSDone:
  
  fclose(fp);
  IPSPatched = 1;
  
  //Adjust size values if the ROM was expanded
  if (location > curromspace  && location <= maxromspace)
  {
    curromspace = location;
    NumofBytes = location;
    NumofBanks = NumofBytes/32768;
  }

  /*
  //Write out patched ROM
  fp = fopen("zsnes.rom", "wb");
  if (!fp) { asm volatile("int $3"); }
  fwrite(ROM, 1, curromspace, fp);
  fclose(fp);
  */
}  


