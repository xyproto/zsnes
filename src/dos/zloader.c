//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int pccmdline(void);

extern void zstart(void);
extern void DosExit(void);
extern void ConvertJoyMap1(void);
extern void ConvertJoyMap2(void);
extern void displayparams(void);
extern void makeextension(void);

extern unsigned char Palette0, SPC700sh, OffBy1Line, DSPDisable,
                     FPUCopy, Force8b, ForcePal, GUIClick, MouseDis,
                     MusicRelVol, ScreenScale, SoundCompD, SoundQuality,
                     StereoSound, V8Mode, antienab, cvidmode, debugdisble,
                     debugger, enterpress, finterleave, frameskip,
                     gammalevel, guioff, per2exec, pl1contrl, pl2contrl,
                     romtype, scanlines, showallext, smallscreenon, soundon,
                     spcon, vsyncon, DisplayS, fname, filefound, SnowOn;

void ccmdline(void);

char *ers[] =
{
  "Frame Skip must be a value of 0 to 9!\n",
  "Gamma Correction Level must be a value of 0 to 5!\n",
  "Sound Sampling Rate must be a value of 0 to 6!\n",
  "Invalid Video Mode!\n",
  "Percentage of instructions to execute must be a number from 50 to 150!\n",
  "Player Input must be a value from 0 to 6!\n",
  "Volume must be a number from 0 to 100!\n"

};


int argc;
char **argv;
int main(int margc, char **margv)
{
  argc=margc;
  argv=margv;
  zstart();
  return(0);
}


int my_atoi(char *nptr)
{
  int p,c;
  c=0;
  for(p=0;nptr[p];p++)
  {
    if( !isdigit(nptr[p]) ) c+=1;
  }
  if(c) return -1;
  else return atoi(nptr);

}


void ccmdline(void)
{
  int p=0;
  p=pccmdline();
  if(p == 0) return;
  
  if(p == 9)
  {
    displayparams();
  }
  if(p == 4)
  {
//    printf("Mangled command line, did you forget a parm?\n");
    printf("Invalid Commandline!\n");
    DosExit();
  }
  
  if((p > 9) && (p < 17))
  {
    printf(ers[p-10]);
    DosExit();
  }
  if(p == 2)
  {
    DosExit();
  }
  
  
  printf("cmdline returned %i\n",p);
  DosExit();

}

int pccmdline(void)
{
  int p;
  int gfnm=0;

  for(p=1;p<argc;p++)
  {
 /*
    printf("(%i/%i): %s\n",p,argc,argv[p]);
 */
          
    if(argv[p][0] == '-')
    {
      int hasroom=0;
      int pp=1;
      int cp=p;
      int nn='_';
      for(pp=1;argv[cp][pp];pp++)
      {
        if( (p+1) < argc) hasroom=1;
        nn=tolower(argv[cp][pp+1]);
        switch(tolower(argv[cp][pp]))
        {
        case '1': /* Player 1 Input */
        {
          if(!hasroom) return 4;
          pl1contrl=my_atoi(argv[p+1]);
          if(pl1contrl > 6) return 15;
          p++;
          ConvertJoyMap1();
          break;
        }
        case '2': /* Player 2 Input */
        {
          if(!hasroom) return 4;
          pl2contrl=my_atoi(argv[p+1]);
          if(pl2contrl > 6) return 15;
          p++;
          ConvertJoyMap2();
          break;
        }
        case 'f': 
        {
          if(!hasroom) return 4;
          frameskip=my_atoi(argv[p+1]);
          if(frameskip > 9) return 10;
          frameskip++;
          p++;
          break;
        }
        case 'g': 
        {
          if(!hasroom) return 4;
          gammalevel=my_atoi(argv[p+1]);
          if(gammalevel > 5) return 11;
          p++;
          break;
        }
        case 'p': 
        {
          if(!hasroom) return 4;
          per2exec=my_atoi(argv[p+1]);
          if(per2exec > 150) return 14;
          if(per2exec < 50) return 14;
          p++;
          break;
        }
        case 'r': 
        {
          if(!hasroom) return 4;
          SoundQuality=my_atoi(argv[p+1]);
          if(SoundQuality > 6) return 12;
          p++;
          break;
        }
        case 'v': 
        {
          if(nn == '8')
          {
            V8Mode=1;
            pp++;
          }
          else
          {
            if(!hasroom) return 4;
            cvidmode=my_atoi(argv[p+1]);
            if(cvidmode > 10) return 13;
            p++;
          }
          break;
        }
        case 'k': 
        {
          if(!hasroom) return 4;
          MusicRelVol=my_atoi(argv[p+1]);
          if(MusicRelVol > 100) return 16;
          p++;
          break;
        }
        case '8':
        {
          Force8b=1;
          break;
        }
        case '0': /* Palette 0 disable */
        {
          Palette0=1;
          break;
        }
        case '7': /* SPC700 speed hack disable */
        {
          SPC700sh=1;
          break;
        }
        case '9': /* Off by 1 line */
        {
          OffBy1Line=1;
          break;
        }
        case 'e':
        {
          enterpress=1;
          break;            
        }
        case 'h':
        {
          romtype=2;
          break;
        }
        case 'l':
        {
          romtype=1;
          break;
        }
        case 'm':
        {
          guioff=1; /* disables GUI */
          break;
        }
        case 'n':
        {
          scanlines=1;
          break;
        }
        case 's':
        {
          if(nn == 'p')
          {
            DisplayS=1;
            pp++;
          }
          else
          if(nn == 'a')
          {
            showallext=1;
            pp++;
          }
          else
          if(nn == 'n')
          {
            SnowOn=1;
            pp++;
          }
          else
          {
            spcon=1;
            soundon=1;
          }
          break;
        }
        case 't':
        {
          ForcePal=1;
          break;
        }
        case 'u':
        {
          ForcePal=2;
          break;
        }
        case 'w':
        {
          vsyncon=1;
          break;
        }
        case 'z':
        {
          StereoSound=1;
          break;
        }
        case 'd':
        {
          if(nn == 'd')
          {
            DSPDisable=1;
            pp++;
          }
          else
          {
            debugger=1;
            debugdisble=0;
          }
          break;
        }
        case 'b':
        {
          SoundCompD=1;
          break;
        }

        case 'c':
        {
          if(nn == 'c')
          {
            smallscreenon=1;
            pp++;
          }
          else
          {
            ScreenScale=1;
          }
          break;
        }

        case 'y':
        {
          antienab=1;
          break;
        }
        case 'o':
        {
          if(nn == 'm')
          {
            FPUCopy=2;
            pp++;
          }
          else
          {
            FPUCopy=0;
          }
          break;
        }
        case 'i':
        {
          finterleave=1;
          break;
        }
        case 'j':
        {
          GUIClick=0;
          MouseDis=1;
          break;
        }
        case '?':
        {
          return 9;
        }
        }
                                                    
      }
      
    }
    else
    {
      if(gfnm > 0)
      {
        printf("Limit yourself to one filename\n");
        return 2;
      }
      else
      {
        char *fvar;
        fvar=&fname;
        fvar[0] = strlen(argv[p]);
        strncpy(&fvar[1],argv[p],127);
        gfnm++;
      }
    }
  }
  if(gfnm == 1)
  {
    filefound=0;
    makeextension();
  }
  return 0;
}
