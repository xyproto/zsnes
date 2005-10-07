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

#ifdef __UNIXSDL__
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#endif
#include <errno.h>

FILE *FILEHANDLE[16];

unsigned int CurrentHandle=0;


// ZFileSystemInit
// return 0

// ZOpenFile info :
char * ZOpenFileName;
unsigned int ZOpenMode;
// Open modes :   0 read/write in
//                1 write (create file, overwrite)
// return file handle if success, 0xFFFFFFFF if error

// ZCloseFile info :
unsigned int ZCloseFileHandle;
// return 0

// ZFileSeek info :
unsigned int ZFileSeekHandle;
unsigned int ZFileSeekPos;
unsigned int ZFileSeekMode; // 0 start, 1 end
// return 0

// ZFileReadBlock info :
char * ZFileReadBlock;
unsigned int ZFileReadSize;
unsigned int ZFileReadHandle;
// return 0

// ZFileWriteBlock info :
char * ZFileWriteBlock;
unsigned int ZFileWriteSize;
unsigned int ZFileWriteHandle;
// return 0

// ZFileTell
unsigned int ZFileTellHandle;

// ZFileGetftime
char * ZFFTimeFName;
unsigned int ZFTimeHandle;
unsigned int ZFDate;
unsigned int ZFTime;

// MKDir/CHDir
char * MKPath;
char * CHPath;
char * RMPath;

//Indicate whether the file must be opened using
//zlib or not (used for gzip support)
char TextFile;

// GetDir
char * DirName;
unsigned int DriveNumber;

// ZFileDelete
char * ZFileDelFName;
// return current position

unsigned int ZFileSystemInit()
{
#ifdef __GZIP__
	TextFile = 0;
#else
	TextFile = 1;
#endif
	CurrentHandle=0;
	return(0);
}

unsigned int ZOpenFile()
{
	if(ZOpenMode==0)
	{
		if (TextFile)
			FILEHANDLE[CurrentHandle]=fopen(ZOpenFileName,"rb");
		else
			FILEHANDLE[CurrentHandle]=(FILE *)gzopen(ZOpenFileName,"rb");
		if(FILEHANDLE[CurrentHandle]!=NULL)
		{
			CurrentHandle+=1;
			return(CurrentHandle-1);
		}
		return(0xFFFFFFFF);
	}
	if(ZOpenMode==1)
	{
		if (TextFile)
			FILEHANDLE[CurrentHandle]=fopen(ZOpenFileName,"wb");
		else
			FILEHANDLE[CurrentHandle]=(FILE *)gzopen(ZOpenFileName,"wb");
		if(FILEHANDLE[CurrentHandle]!=NULL)
		{
			CurrentHandle+=1;
			return(CurrentHandle-1);
		}
		return(0xFFFFFFFF);
	}
	if(ZOpenMode==2)
	{
		if (TextFile)
			FILEHANDLE[CurrentHandle]=fopen(ZOpenFileName,"r+b");
		else
			FILEHANDLE[CurrentHandle]=gzopen(ZOpenFileName,"r+b");
		if(FILEHANDLE[CurrentHandle]!=NULL)
		{
			CurrentHandle+=1;
			return(CurrentHandle-1);
		}
		return(0xFFFFFFFF);
	}
	return(0xFFFFFFFF);
}

unsigned int ZCloseFile()
{
	if (TextFile)
		fclose(FILEHANDLE[ZCloseFileHandle]);
	else
		gzclose(FILEHANDLE[ZCloseFileHandle]);
	CurrentHandle-=1;
	return(0);
}

unsigned int ZFileSeek()
{
	int mode = 0;
	if (ZFileSeekMode==0)
		mode = SEEK_SET;
	else if (ZFileSeekMode==1) {
		mode = SEEK_END;
		if (TextFile==0)
			printf("Warning : gzseek(SEEK_END) not supported");
	} else return (0xFFFFFFFF);

	if (TextFile) {
		fseek(FILEHANDLE[ZFileSeekHandle], ZFileSeekPos, mode);
		return 0;
	} else {
		gzseek(FILEHANDLE[ZFileSeekHandle], ZFileSeekPos, mode);
		return 0;
	}
	return(0xFFFFFFFF);
}

unsigned int ZFileRead()
{
	if (TextFile)
		return(fread(ZFileReadBlock,
			     1,
			     ZFileReadSize,
			     FILEHANDLE[ZFileReadHandle]));
	else
		return(gzread(FILEHANDLE[ZFileReadHandle],
			      ZFileReadBlock,
			      ZFileReadSize));
}


unsigned int ZFileWrite()
{
	unsigned int res=0;
	if (TextFile)
		res = fwrite(ZFileWriteBlock,
			     1,
			     ZFileWriteSize,
			     FILEHANDLE[ZFileWriteHandle]);
	else
		res = gzwrite(FILEHANDLE[ZFileWriteHandle],
			      ZFileWriteBlock,
			      ZFileWriteSize);

	if (res!=ZFileWriteSize)
		return(0xFFFFFFFF);

	return(0);
}

unsigned int ZFileTell()
{
	int res = 0;
	if (TextFile) {
		res = ftell(FILEHANDLE[ZFileTellHandle]);
		if (res == -1) fprintf(stderr, "Oups!! gzTell\n");
		return(res);
	} else return gztell(FILEHANDLE[ZFileTellHandle]);
}

unsigned int ZFileDelete()
{
  return(remove(ZFileDelFName));
}

unsigned int ZFileGetFTime()
{
  struct stat filestat;

  ZFTime=0;

  if (stat(ZFFTimeFName, &filestat) < 0) ZFDate=0;
     else ZFDate = filestat.st_mtime;

  return(0);
}

unsigned int ZFileMKDir()
{
#ifdef __UNIXSDL__
  return(mkdir(MKPath, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)));
#else
  return(mkdir(MKPath));
#endif
}


unsigned int ZFileCHDir()
{
  return(chdir(CHPath));
}

unsigned int ZFileRMDir()
{
  return(rmdir(RMPath));
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
	//STUB_FUNCTION;
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

extern char SRAMDir[1024];
extern char LoadDir[512];

#ifdef __UNIXSDL__

char zcfgdir[1024];
#define ZCFG_DIR "/.zsnes"
#define ZCFG_DIR_LEN (1023-strlen(ZCFG_DIR))

void obtaindir()
{
  struct passwd *userinfo;
  DIR *tmp;

  if ((userinfo = getpwuid(getuid())))
  {
    strcpy(zcfgdir, userinfo->pw_dir);
  }
  else
  {
    getcwd(zcfgdir, ZCFG_DIR_LEN);
  }
  strcat(zcfgdir, ZCFG_DIR);
  tmp = opendir(zcfgdir);
  if (tmp == NULL)
  {
    MKPath = zcfgdir;
    ZFileMKDir();
  }
  else
  {
    closedir(tmp);
  }
  if (*SRAMDir == 0)
  {
    strcpy(SRAMDir, zcfgdir);
  }
  if (*LoadDir == 0)
  {
    getcwd(LoadDir, 512);
  }
}

void GetFilename()
{
  extern char fnamest;
  extern int statefileloc;
  char *tmp = &fnamest;
  char size;

  *tmp = '/';
  while (*tmp!=0) tmp++;
  while (*tmp!='/') tmp--;
  size = (strlen(tmp)-1) & 0xFF;
  memmove(&fnamest, tmp, strlen(tmp));
  fnamest = size;
  statefileloc-=(tmp-&fnamest);
}

char *olddir = NULL;


void pushdir()
{
	olddir = (char *)malloc(128);
	getcwd(olddir, 128);
}

void popdir()
{
	CHPath = olddir;
	ZFileCHDir();
	free(olddir);
	olddir = NULL;
}
#endif



