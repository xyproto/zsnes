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



#include <stdio.h>
#include <time.h>
#include <zlib.h>
#include <dos.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <unistd.h>


#define DWORD unsigned int
#define BYTE unsigned char

FILE *FILEHANDLE[16];
DWORD CurrentHandle=0;

//Indicate whether the file must be opened using
//zlib or not (used for gzip support)
BYTE TextFile;


// ZFileSystemInit
// return 0

// ZOpenFile info :
BYTE * ZOpenFileName;
DWORD ZOpenMode;
// Open modes :   0 read/write in
//                1 write (create file, overwrite)
// return file handle if success, 0xFFFFFFFF if error

// ZCloseFile info :
DWORD ZCloseFileHandle;
// return 0

// ZFileSeek info :
DWORD ZFileSeekHandle;
DWORD ZFileSeekPos;
DWORD ZFileSeekMode; // 0 start, 1 end
// return 0

// ZFileReadBlock info :
BYTE * ZFileReadBlock;
DWORD ZFileReadSize;
DWORD ZFileReadHandle;
// return 0

// ZFileWriteBlock info :
BYTE * ZFileWriteBlock;
DWORD ZFileWriteSize;
DWORD ZFileWriteHandle;
// return 0

// ZFileTell
DWORD ZFileTellHandle;

// ZFileGetftime
BYTE * ZFFTimeFName;
DWORD ZFTimeHandle;
DWORD ZFDate;
DWORD ZFTime;

// MKDir/CHDir
BYTE * MKPath;
BYTE * CHPath;
BYTE * RMPath;

// GetDir
BYTE * DirName;
DWORD DriveNumber;

// ZFileDelete
BYTE * ZFileDelFName;
// return current position


DWORD ZFileSystemInit()
{
#ifdef __GZIP__
	TextFile = 0;
#else
	TextFile = 1;
#endif
	CurrentHandle=0;
	return(0);
}


DWORD ZOpenFile()
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
			FILEHANDLE[CurrentHandle]=(FILE *)gzopen(ZOpenFileName,"r+b");
		if(FILEHANDLE[CurrentHandle]!=NULL)
		{
			CurrentHandle+=1;
			return(CurrentHandle-1);
		}
		return(0xFFFFFFFF);
	}
	return(0xFFFFFFFF);
}

DWORD ZCloseFile()
{
	if (TextFile)
		fclose(FILEHANDLE[ZCloseFileHandle]);
	else
		gzclose(FILEHANDLE[ZCloseFileHandle]);
	CurrentHandle-=1;
	return(0);
}

DWORD ZFileSeek()
{
    /*int res = 0;*/
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

DWORD ZFileRead()
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


DWORD ZFileWrite()
{
	int res=0;
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

DWORD ZFileTell()
{
	int res = 0;
	if (TextFile) {
		res = ftell(FILEHANDLE[ZFileTellHandle]);
		if (res == -1) fprintf(stderr, "Oups!! gzTell\n");
		return(res);
	} else return gztell(FILEHANDLE[ZFileTellHandle]);
}

DWORD ZFileDelete()
{
  return(remove(ZFileDelFName));
}


DWORD ZFileGetFTime()
{
  _dos_open(ZFFTimeFName, 0,&ZFTimeHandle);
  _dos_getftime(ZFTimeHandle,&ZFDate,&ZFTime);
  _dos_close(ZFTimeHandle);
  return(0);
}

DWORD ZFileMKDir()
{
  /*return(mkdir(MKPath));*/
  return (mkdir(MKPath, S_IWUSR));
}

DWORD ZFileCHDir()
{
  return(chdir(CHPath));
}

DWORD ZFileRMDir()
{
  return(rmdir(RMPath));
}

DWORD ZFileGetDir()
{
  /*return(getcwd(DirName,128));*/
  return(*getcwd(DirName,128));
}

BYTE * ZFileFindPATH;
DWORD ZFileFindATTRIB;
DWORD DTALocPos;

//struct _find_t {
//  char reserved[21] __attribute__((packed));
//  unsigned char attrib __attribute__((packed));
//  unsigned short wr_time __attribute__((packed));
//  unsigned short wr_date __attribute__((packed));
//  unsigned long size __attribute__((packed));
//  char name[256] __attribute__((packed));
//};

DWORD ZFileFindFirst()
{
   /*return(_dos_findfirst(ZFileFindPATH,ZFileFindATTRIB,DTALocPos));*/
   return(_dos_findfirst(ZFileFindPATH,ZFileFindATTRIB,((struct find_t *)DTALocPos)));

}

DWORD ZFileFindNext()
{
   /*return(_dos_findnext(DTALocPos));*/
   return(_dos_findnext(((struct find_t *) DTALocPos)));
}

DWORD ZFileFindEnd()  // for compatibility with windows later
{
   return(0);
}


//BYTE * DirName;
//DWORD DriveNumber;

//unsigned int   _dos_findfirst(char *_name, unsigned int _attr, struct _find_t *_result);
//unsigned int   _dos_findnext(struct _find_t *_result);


DWORD GetTime()
{
   DWORD value;
   struct tm *newtime;
   time_t long_time;

   time( &long_time );
   newtime = localtime( &long_time );

   value = ((newtime->tm_sec) % 10)+((newtime->tm_sec)/10)*16
          +((((newtime->tm_min) % 10)+((newtime->tm_min)/10)*16) << 8)
          +((((newtime->tm_hour) % 10)+((newtime->tm_hour)/10)*16) << 16);
   return(value);
}

DWORD GetDate()
{
   DWORD value;
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


