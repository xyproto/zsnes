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


#ifdef __LINUX__
#include "../gblhdr.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#endif

#ifdef __MSDOS__
#include <utime.h>
#include <sys/stat.h>
#endif

#include "zunzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)


unsigned int ZipError=0;
// 1 : Cannot open file
// 2 : Could not create directory
// 3 : Zip error
// 4 : Memory error
// 5 : Error opening file
// 6 : Error Writing file

#ifndef __LINUX__
#ifndef __MSDOS__
struct utimbuf {
  time_t actime;
  time_t modtime;
};
#endif
#endif

void change_file_date(const char *filename,uLong dosdate,tm_unz tmu_date)
{
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
}


int mymkdir(const char *dirname)
{
#ifdef __LINUX__
  return(mkdir(dirname, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)));
#else
#ifdef __MSDOS__
  return(mkdir(dirname, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)));
#else
  return(mkdir(dirname));
#endif
#endif
}

int makedir (char *newdir)
{
  char *buffer ;
  char *p;
  int  len = strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          ZipError=2;
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}




int do_extract_currentfile(unzFile uf,
                           const int* popt_extract_without_path,
                           int *popt_overwrite)
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

	unz_file_info file_info;
	err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

	if (err!=UNZ_OK)
	{
      ZipError=3;
		return err;
	}

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        ZipError=4;
        return UNZ_INTERNALERROR;
    }

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0')
	{
		if (((*p)=='/') || ((*p)=='\\'))
			filename_withoutpath = p+1;
		p++;
	}

	if ((*filename_withoutpath)=='\0')
	{
		if ((*popt_extract_without_path)==0)
		{
			mymkdir(filename_inzip);
		}
	}
	else
	{
                char* write_filename;
		int skip=0;

		if ((*popt_extract_without_path)==0)
			write_filename = filename_inzip;
		else
			write_filename = filename_withoutpath;

		err = unzOpenCurrentFile(uf);
		if (err!=UNZ_OK)
		{
         ZipError=4;
		}

		if (((*popt_overwrite)==0) && (err==UNZ_OK))
		{
			char rep='A';
			FILE* ftestexist;
			ftestexist = fopen(write_filename,"rb");
			if (ftestexist!=NULL)
			{
				fclose(ftestexist);
            rep='N';
			}

			if (rep == 'N')
				skip = 1;

			if (rep == 'A')
				*popt_overwrite=1;
		}

		if ((skip==0) && (err==UNZ_OK))
		{
			fout=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }

			if (fout==NULL)
			{
            ZipError=5;
			}
		}

		if (fout!=NULL)
		{
			do
			{
				err = unzReadCurrentFile(uf,buf,size_buf);
				if (err<0)
				{
               ZipError=4;
					break;
				}
				if (err>0)
					if (fwrite(buf,err,1,fout)!=1)
					{
                  ZipError=6;
                        err=UNZ_ERRNO;
						break;
					}
			}
			while (err>0);
			fclose(fout);
			if (err==0)
				change_file_date(write_filename,file_info.dosDate,
					             file_info.tmu_date);
		}

        if (err==UNZ_OK)
        {
		    err = unzCloseCurrentFile (uf);
		    if (err!=UNZ_OK)
		    {
             ZipError=4;
		    }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
	}

    free(buf);
    return err;
}


int do_extract(unzFile uf,int opt_extract_without_path,int opt_overwrite)
{
	uLong i;
	unz_global_info gi;
	int err;

	err = unzGetGlobalInfo (uf,&gi);
	if (err!=UNZ_OK)
      ZipError=4;

	for (i=0;i<gi.number_entry;i++)
	{
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite) != UNZ_OK)
            break;

		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err!=UNZ_OK)
			{
            ZipError=4;
				break;
			}
		}
	}

	return 0;
}


void extractzip(char *FileToExtract)
{
	unzFile uf=NULL;
// I really don't like hardcoding these sizes...
	char oldpath[128];

#ifdef __LINUX__
	extern char InitDir;

	getcwd(oldpath, 128);
	chdir(&InitDir);
#endif
	uf = unzOpen(FileToExtract);
#ifdef __LINUX__
	chdir(oldpath);
#endif

	if (uf==NULL)
	{
		ZipError=1;
		return;
	}
	ZipError=0;
	do_extract(uf,0,0);
}


