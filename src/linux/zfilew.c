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
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#ifdef __UNIXSDL__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <pwd.h>
#else
#include <time.h>
#include <io.h>
#endif

FILE *FILEHANDLE[16];

unsigned int CurrentHandle=0;



// MKDir/CHDir
char * CHPath;

// GetDir
char * DirName;
unsigned int DriveNumber;

unsigned int ZFileCHDir()
{
  return(chdir(CHPath));
}

char *ZFileGetDir()
{
  return(getcwd(DirName,128));
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


int TempFind;
#ifndef __UNIXSDL__
int FindFirstHandle;
struct _finddata_t FindDataStruct;
#else
int ignoredirs=0;
glob_t globbuf;
int globcur;
#endif

unsigned int ZFileFindNext()
{
#ifdef __UNIXSDL__
   struct stat filetype;

   if (globcur == -1)
   	return -1;

   globcur++;
   if (globcur > globbuf.gl_pathc) /* >= */
   	return -1;

   if (globcur == globbuf.gl_pathc) {
   	/* this is the end, so just add it ourselves */
   	*(char *)(DTALocPos + 0x15) = 0x10;
   	strcpy((char *)DTALocPos + 0x1E, "..");
   	return 0;
   }

   *(char *)(DTALocPos + 0x15) = 0;

   stat ( globbuf.gl_pathv[globcur], &filetype );

   if(ZFileFindATTRIB&0x10 && !S_ISDIR ( filetype.st_mode )) return(ZFileFindNext());
   if(((ZFileFindATTRIB&0x10)==0) && S_ISDIR ( filetype.st_mode )) return(ZFileFindNext());

   if ( S_ISDIR ( filetype.st_mode ))
     *(char *)(DTALocPos + 0x15) = 0x10;

   strcpy((char *)DTALocPos + 0x1E, globbuf.gl_pathv[globcur]);

#else
   TempFind=_findnext(FindFirstHandle,&FindDataStruct);
   if(TempFind==-1) return(-1);

   *(char *)(DTALocPos+0x15)=0;

   if(ZFileFindATTRIB&0x10 && (FindDataStruct.attrib&0x10)==0) return(ZFileFindNext());
   if((ZFileFindATTRIB&0x10==0) && FindDataStruct.attrib&0x10) return(ZFileFindNext());

   if(FindDataStruct.attrib&_A_SUBDIR)  *(char *)(DTALocPos+0x15)=0x10;
   strcpy((char *)DTALocPos+0x1E,FindDataStruct.name);
   if(TempFind==-1) return(-1);
#endif
   return(0);
}

unsigned int ZFileFindFirst()
{
#ifdef __UNIXSDL__
	//STUB_FUNCTION;
   struct stat filetype;

   if (globcur != -1) {
   	globfree(&globbuf);
   	globcur = -1;
   }

   if (glob(ZFileFindPATH, 0, NULL, &globbuf))
   	return -1;
   globcur = 0;

   *(char *)(DTALocPos + 0x15) = 0;

#ifdef __BSDSDL__
   if (globbuf.gl_matchc == 0)
	return -1;
#endif

   stat ( globbuf.gl_pathv[globcur], &filetype );

   if(ZFileFindATTRIB&0x10 && !S_ISDIR ( filetype.st_mode )) return(ZFileFindNext());
   if(((ZFileFindATTRIB&0x10)==0) && S_ISDIR ( filetype.st_mode )) return(ZFileFindNext());

   if ( S_ISDIR ( filetype.st_mode ))
     *(char *)(DTALocPos + 0x15) = 0x10;

   strcpy((char *)DTALocPos + 0x1E, globbuf.gl_pathv[globcur]);

#else
   FindFirstHandle=_findfirst(ZFileFindPATH,&FindDataStruct);
   *(char *)(DTALocPos+0x15)=0;
   TempFind=0;
   if(FindFirstHandle==-1) return(-1);
   if(ZFileFindATTRIB&0x10 && (FindDataStruct.attrib&0x10)==0) return(ZFileFindNext());
   if((ZFileFindATTRIB&0x10==0) && FindDataStruct.attrib&0x10) return(ZFileFindNext());

   if(FindDataStruct.attrib&_A_SUBDIR)  *(char *)(DTALocPos+0x15)=0x10;
   strcpy((char *) DTALocPos+0x1E,FindDataStruct.name);
   if(FindFirstHandle==-1) return(-1);
#endif
   return(0);
}


unsigned int ZFileFindEnd()  // for compatibility with windows later
{
#ifdef __UNIXSDL__
	//STUB_FUNCTION;
	if (globcur != -1) {
		globfree(&globbuf);
		globcur = -1;
	}
#else
   _findclose(FindFirstHandle);
#endif
   return(0);
}


//char * DirName;
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



