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



#include <stdio.h>
#include <time.h>
#include <zlib.h>
#include <dos.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <unistd.h>

// MKDir/CHDir
char * CHPath;

// GetDir
char * DirName;
unsigned int DriveNumber;

unsigned int ZFileCHDir()
{
  return(chdir(CHPath));
}

unsigned int ZFileGetDir()
{
  /*return(getcwd(DirName,128));*/
  return(*getcwd(DirName,128));
}

char * ZFileFindPATH;
unsigned int ZFileFindATTRIB;
unsigned int DTALocPos;

//struct _find_t {
//  char reserved[21] __attribute__((packed));
//  unsigned char attrib __attribute__((packed));
//  unsigned short wr_time __attribute__((packed));
//  unsigned short wr_date __attribute__((packed));
//  unsigned long size __attribute__((packed));
//  char name[256] __attribute__((packed));
//};

unsigned int ZFileFindFirst()
{
   /*return(_dos_findfirst(ZFileFindPATH,ZFileFindATTRIB,DTALocPos));*/
   return(_dos_findfirst(ZFileFindPATH,ZFileFindATTRIB,((struct find_t *)DTALocPos)));

}

unsigned int ZFileFindNext()
{
   /*return(_dos_findnext(DTALocPos));*/
   return(_dos_findnext(((struct find_t *) DTALocPos)));
}

unsigned int ZFileFindEnd()  // for compatibility with windows later
{
   return(0);
}


//unsigned char * DirName;
//unsigned int DriveNumber;

//unsigned int   _dos_findfirst(char *_name, unsigned int _attr, struct _find_t *_result);
//unsigned int   _dos_findnext(struct _find_t *_result);


unsigned int GetTime()
{
   unsigned int value;
   struct tm *newtime;
   time_t long_time;

   time( &long_time );
   newtime = localtime( &long_time );

   value = ((newtime->tm_sec) % 10)+((newtime->tm_sec)/10)*16
          +((((newtime->tm_min) % 10)+((newtime->tm_min)/10)*16) << 8)
          +((((newtime->tm_hour) % 10)+((newtime->tm_hour)/10)*16) << 16);
   return(value);
}

unsigned int GetDate()
{
   unsigned int value;
   struct tm *newtime;
   time_t long_time;

   time( &long_time );
   newtime = localtime( &long_time );
   value = ((newtime->tm_mday) % 10)+((newtime->tm_mday)/10)*16
          +(((newtime->tm_mon)+1) << 8)
          +((((newtime->tm_year) % 10)+((newtime->tm_year)/10)*16) << 16)
          +((newtime->tm_wday) << 28);

   return(value);
}
