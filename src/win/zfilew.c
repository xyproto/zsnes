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


// Enable use of network loaded games with zsnes:// file names
//#define CCBETA

// Enable debug of network loaded games
//#define CCDEBUG

#ifdef CCBETA
#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include "resource.h"
#endif

#include <stdio.h>
#include <time.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>


#ifdef CCDEBUG
FILE * ZFILELog = NULL;

void ZLog_Message (char *Message, ...)
{
    char Msg[400];
	va_list ap;

    va_start(ap,Message);
    vsprintf(Msg,Message,ap );
    va_end(ap);

    strcat(Msg,"\r\n\0");
    fwrite(Msg,strlen(Msg),1,ZFILELog);
    fflush(ZFILELog);
}


void ZStart_Log (void)
{
	char LogFileName[255];
//  [4/15/2001]	char *p;

    strcpy(LogFileName,"zfile.log\0");

    ZFILELog = fopen(LogFileName,"wb");
}

void ZStop_Log(void)
{
    if(ZFILELog)
    {
    fclose(ZFILELog);
    ZFILELog=NULL;
    }
}

#else
#define ZLog_Message()
#define ZStart_Log()
#define ZStop_Log()
#endif


#define DWORD unsigned int
#define BYTE unsigned char


FILE *FILEHANDLE[16];
DWORD CurrentHandle=0;

#ifdef CCBETA
int FILETYPE[16]; // 0 = normal file  1 = network file
int FILESIZE[16]; // 0 = normal file else network size
char * FILEDATA[16]; // 0 = normal file else network data
int FILECURPOS[16]; // network currrent position
#endif

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


#ifdef CCBETA

/* Additional code to handle remote roms */

SOCKET   gameServerSocket;    /* tcp socket for the game Server */
SOCKADDR_IN    gameServerAddress; /* address of the game server */

int InitTCPFile()
{
    char blah[255];
    WORD versionneeded = MAKEWORD(2,2);
    WSADATA wsadata;

    /* Startup winsock */
    WSAStartup(versionneeded, &wsadata);

    /* Verify version number and exit on wrong version */
    if (wsadata.wVersion != versionneeded)
	{
        return(-1);
	}
    gameServerSocket=INVALID_SOCKET;
    return(0);
}

int ConnectGameServer(char *servername, unsigned int port)
{
    char blah[255];
    int retval,i;
    LPHOSTENT host1;
    unsigned long addr1;
    int yesip;
    WSADATA wsadata;
    int timeout=10000;

    host1 = gethostbyname(servername);
    if (host1 == NULL)
    {
        return(-1);
    }

    gameServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(gameServerSocket == INVALID_SOCKET)
    {
        return(-2);
	}

    /* initialize server address */
    gameServerAddress.sin_family = AF_INET;
    gameServerAddress.sin_addr = *( (LPIN_ADDR) *host1->h_addr_list );
    gameServerAddress.sin_port = htons((unsigned short)port);

    retval = connect( gameServerSocket,
                     (LPSOCKADDR)&gameServerAddress,
                     sizeof(struct sockaddr));
    if (retval == SOCKET_ERROR)
	{
        sprintf(blah,"Could not connect to other side");
        MessageBox(NULL,blah,
                  "Error",
                   MB_SYSTEMMODAL|MB_OK);
        closesocket(gameServerSocket);
        return(-3);
	}

    setsockopt(gameServerSocket,SOL_SOCKET,SO_RCVTIMEO,
               &timeout,sizeof(timeout));
    return 0;
}

sendCommand(char *dptr, int dsize)
{
    int retval,i;
    retval = send(gameServerSocket,dptr,dsize,0);
    if (retval == SOCKET_ERROR)
    {
      closesocket(gameServerSocket);
      return(-1);
	}
    return(0);
}

receiveData(char *dptr, int dsize)
{
    int retval;
    /* get data with the socket */
    retval = recv(gameServerSocket,dptr,dsize,0);
    if (retval == SOCKET_ERROR)
	{
        closesocket(gameServerSocket);
        return(-1);
	}
   return(retval);
}


int checkconnection=0;

extern char fname[512];

void CCExit(void)
{
    char temp[512];
    if(checkconnection)
    {
            ZLog_Message("Disconnecting from server");
            checkconnection=0;
//            sprintf(temp,"cl");
//            sendCommand(temp,strlen(temp));
//                receiveData(temp,512);
            closesocket(gameServerSocket);
            return;
    }
}

extern char romloadskip;
extern unsigned int pressed;

// Function given by Jereth
LPTHREAD_START_ROUTINE MonitorThreadProc(SOCKET s)
{
    unsigned char * keys;

    unsigned int clockstart;
    unsigned int clocknow;
    char temp[512];
    char error[255];
    int destruct = 10;
    char sec[] = " seconds";
    fd_set socketTest;
    struct timeval timeOut = {0,0};
    FD_ZERO(&socketTest);
    FD_SET(s,&socketTest);
    ZLog_Message("Starting monitor thread");
    for(;;)
    {
        if(checkconnection)
        {
            if(!strstr(&fname[1],"zsnes://"))
            {
                ZLog_Message("Disconnecting from server");
                checkconnection=0;
//                sprintf(temp,"cl");
//                sendCommand(temp,strlen(temp));
//                receiveData(temp,512);
                closesocket(gameServerSocket);
                return 0;
            }
            sprintf(temp,"hi");
            sendCommand(temp,strlen(temp));
            receiveData(temp,512);
            if(!strstr(temp,"ok"))
            {
                HMODULE hModule = GetModuleHandle(NULL);
                HWND hwndNCD;
                if(destruct>0)
                    hwndNCD = CreateDialog(hModule, MAKEINTRESOURCE
                                       (IDD_NO_CONNECT), NULL, NULL);
                while(destruct > 0)
                {
                    if (destruct == 10)
                        clockstart = timeGetTime();

                    if (destruct == 1)
                    {
                        sec[7] = 0;
                    }

                    wsprintf(error, "This message will self-destruct in %d%s%s", destruct, sec, ".");
                    SetDlgItemText(hwndNCD, IDC_DESTRUCT,error);
                    clocknow = timeGetTime();
                    while(clocknow < (clockstart+1000))
                    {
                        clocknow = timeGetTime();
                    }
                    clockstart+=1000;
                    destruct--;
                }

                DestroyWindow(hwndNCD);

                keys = (unsigned char *)&pressed;
                keys[1]=1;
                romloadskip=1;
            }
            Sleep(1000);
        }
    }
    return 0;
}


DWORD ThreadId;
HANDLE theMonitor;


OpenConnection(char *User, char *Password, char *FileName)
{
    HWND hwndNCD;
    char temp[513];
    int cur=0;
    HMODULE hModule = GetModuleHandle(NULL);
    int retval

    ZLog_Message("Need to open %s",FileName);

    ZLog_Message("Starting winsock");
    if(InitTCPFile()!=0)
    {
//        printf("Winsock version 2.2 is needed\n");
        return(0);
    }

    ZLog_Message("Connection to server");

    if(ConnectGameServer("www.consoleclassix.com",11001)<0)
    {
//        printf("Error connecting to the game server\n");
        return(0);
    }

    ZLog_Message("Sending user name");

    sprintf(temp,"%s",User);
    sendCommand(temp,strlen(temp));
    receiveData(temp,512);

    if(!strstr(temp,"ok")) return 0;

    ZLog_Message("Sending password");

    sprintf(temp,"%s",Password);
    sendCommand(temp,strlen(temp));
    receiveData(temp,512);

    if(!strstr(temp,"ok"))
    {
        ZLog_Message("Sending user name");

        sprintf(temp,"%s",User);
        sendCommand(temp,strlen(temp));
        receiveData(temp,512);

        if(!strstr(temp,"ok")) return 0;

        ZLog_Message("Sending password");

        sprintf(temp,"%s",Password);
        sendCommand(temp,strlen(temp));
        receiveData(temp,512);
        if(!strstr(temp,"ok")) return 0;
    }

    ZLog_Message("Sending IP");

    sprintf(temp,"255.255.255.255");
    sendCommand(temp,strlen(temp));
    receiveData(temp,512);

    ZLog_Message("Sending op");

    sprintf(temp,"op");
    sendCommand(temp,strlen(temp)+1);
    receiveData(temp,512);

    ZLog_Message("Sending file name: %s",FileName);

    sprintf(temp,"%s",FileName);
    sendCommand(temp,strlen(temp));
    receiveData(temp,512);

    ZLog_Message("Sending ok reply");

    sprintf(temp,"ok");
    sendCommand(temp,strlen(temp));
    receiveData(temp,512);

    sscanf(temp,"%d",&FILESIZE[CurrentHandle]);

    FILEDATA[CurrentHandle]=(char *) malloc(FILESIZE[CurrentHandle]);
    FILECURPOS[CurrentHandle]=0;
    sprintf(temp,"ok");
    sendCommand(temp,strlen(temp));

    ZLog_Message("Reading file");

    hwndNCD = CreateDialog(hModule, MAKEINTRESOURCE
                               (IDD_LOADING), NULL, NULL);


    while(cur<FILESIZE[CurrentHandle])
    {
        retval=receiveData(FILEDATA[CurrentHandle]+cur,4096);
        if(retval<0)
            return 0;
        cur+=retval;
        ZLog_Message("Reading file at %d",cur);
        wsprintf(temp, "%7d/%7d", cur, FILESIZE[CurrentHandle]);
        SetDlgItemText(hwndNCD, IDC_PARTDONE,temp);
        SendDlgItemMessage(hwndNCD, IDC_PROGRESS1, PBM_SETPOS, 100*cur/FILESIZE[CurrentHandle],
           0);
    }
    DestroyWindow(hwndNCD);

    atexit(CCExit);
    theMonitor = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                              (DWORD)0,
                              (LPTHREAD_START_ROUTINE)MonitorThreadProc,
                              (LPVOID)gameServerSocket,
                              (DWORD)0,
                              (LPDWORD)&ThreadId);
    ZLog_Message("Created thread %X",theMonitor);

    SetThreadPriority(theMonitor,THREAD_PRIORITY_LOWEST);
    checkconnection=1;

    return(1);
}


// Add this later to disconnect...
DWORD ZFileSystemDeInit()
{
}

int UPDialogDone;
char user[512];
char pass[512];

LRESULT CALLBACK UPDialogMain(HWND hDlg, UINT message, WPARAM wParam,
                              LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
            break;
        case WM_COMMAND:
            if(LOWORD(wParam)==IDOK)
            {
                if(!GetDlgItemText(hDlg, IDC_EDITUSER, user, 512)) *user=0;
                if(!GetDlgItemText(hDlg, IDC_EDITPASS, pass, 512)) *pass=0;

                EndDialog(hDlg, LOWORD(wParam));
                UPDialogDone=1;
                return TRUE;
            }
            break;
    }
    return FALSE;
}

#endif //CCBETA


DWORD ZFileSystemInit()
{
    ZStart_Log();
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
#ifdef CCBETA
    char site[512];
    char rom[512];
    int i;
    char * startstr;
    char * endstr;
    ZLog_Message("ZOpenFile %d %s",CurrentHandle, ZOpenFileName);

    if((startstr=strstr(ZOpenFileName,"zsnes://"))&&strstr(ZOpenFileName,"smc"))
    {
        FILETYPE[CurrentHandle]=1;
        startstr+=8;
        if(!strstr(startstr,"@"))
        {
            HMODULE hModule = GetModuleHandle(NULL);
            HWND hwndNCD;

            ZLog_Message("Creating user/pass window");

            UPDialogDone=0;
            hwndNCD = DialogBox(hModule, MAKEINTRESOURCE
                                   (IDD_USERPASS), NULL, UPDialogMain);

            while(!UPDialogDone)
            {
                Sleep(1000);
            }

            DestroyWindow(hwndNCD);

            ZLog_Message("User : %s Pass : %s",user,pass);
        }
        else
        {
            if((endstr=strstr(startstr,":"))==NULL) return 0xFFFFFFFF;
            if((endstr-startstr)>512) return 0xFFFFFFFF;
            for(i=0;i<endstr-startstr;i++)
                user[i]=startstr[i];
            user[endstr-startstr]=0;

            startstr=endstr+1;
            if((endstr=strstr(startstr,"@"))==NULL) return 0xFFFFFFFF;
            if((endstr-startstr)>512) return 0xFFFFFFFF;
            for(i=0;i<endstr-startstr;i++)
                pass[i]=startstr[i];
            pass[endstr-startstr]=0;
            ZLog_Message("User : %s Pass : %s",user,pass);
            startstr=endstr+1;
        }

        if((endstr=strstr(startstr,"/"))==NULL) return 0xFFFFFFFF;
        if((endstr-startstr)>512) return 0xFFFFFFFF;
        for(i=0;i<endstr-startstr;i++)
            site[i]=startstr[i];
        site[endstr-startstr]=0;

        startstr=endstr+1;
        endstr=startstr+strlen(startstr);
        if((endstr-startstr)>512) return 0xFFFFFFFF;
        for(i=0;i<endstr-startstr;i++)
            rom[i]=startstr[i];
        rom[endstr-startstr]=0;

        ZLog_Message("Site : %s Rom : %s",site,rom);

        if(!OpenConnection(user,pass,rom))
        {
            MessageBox(NULL,
                      "Couldn't connect to server. Please check your user information and verify your internet connection","Fatal Error",
                       MB_SYSTEMMODAL|MB_OK);
            exit(0);
        }

        CurrentHandle+=1;
        return(CurrentHandle-1);
    }
    else
    {
        if(startstr=strstr(ZOpenFileName,"zsnes://"))
        {
            ZOpenFileName=strstr(startstr+8,"/")+1;
        }
        ZLog_Message("Replaced file name with %s",ZOpenFileName);

        FILETYPE[CurrentHandle]=0;
    }

#endif
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

DWORD ZCloseFile()
{
    #ifdef CCBETA
    ZLog_Message("ZCloseFile %d",ZCloseFileHandle);

    if(FILETYPE[ZCloseFileHandle]==1)
    {

    }
    else
    {

    #endif
        if (TextFile)
            fclose(FILEHANDLE[ZCloseFileHandle]);
        else
            gzclose(FILEHANDLE[ZCloseFileHandle]);
    #ifdef CCBETA
    }
    #endif

	CurrentHandle-=1;
	return(0);
}

DWORD ZFileSeek()
{
#ifdef CCBETA
    ZLog_Message("ZFileSeek %d",ZFileSeekHandle);

    if(FILETYPE[ZFileSeekHandle]==1)
    {

    }
    else
    {
#endif
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
#ifdef CCBETA
    }
#endif
	return(0xFFFFFFFF);
}

DWORD ZFileRead()
{
#ifdef CCBETA
    ZLog_Message("ZFileRead %d len:%d curpos:%d",ZFileReadHandle,ZFileReadSize,FILECURPOS[ZFileReadHandle]);
    if(FILETYPE[ZFileReadHandle]==1)
    {
        if(FILECURPOS[ZFileReadHandle]>=FILESIZE[ZFileReadHandle]) return 0;
        if((FILECURPOS[ZFileReadHandle]+ZFileReadSize)>FILESIZE[ZFileReadHandle])
        {
            ZFileReadSize=FILESIZE[ZFileReadHandle]-FILECURPOS[ZFileReadHandle];
        }
        ZLog_Message("Copying len:%d",ZFileReadSize);

        memcpy(ZFileReadBlock,FILEDATA[ZFileReadHandle]+FILECURPOS[ZFileReadHandle],
               ZFileReadSize);
        FILECURPOS[ZFileReadHandle]+=ZFileReadSize;

        ZLog_Message("File read done");

        return ZFileReadSize;
    }
    else
    {
#endif
        if (TextFile)
            return(fread(ZFileReadBlock,
                     1,
                     ZFileReadSize,
                     FILEHANDLE[ZFileReadHandle]));
        else
            return(gzread(FILEHANDLE[ZFileReadHandle],
                      ZFileReadBlock,
                      ZFileReadSize));
#ifdef CCBETA
    }
#endif
}


DWORD ZFileWrite()
{
#ifdef CCBETA
    ZLog_Message("ZFileWrite %d",ZFileWriteHandle);
    if(FILETYPE[ZFileWriteHandle]==1)
    {

    }
    else
    {
#endif

		//MK: this will fail if we write 2GB files
		//so here's hoping we never need to.
        int res=0;
        if (TextFile)
            res = (int) fwrite(ZFileWriteBlock,
                     1,
                     ZFileWriteSize,
                     FILEHANDLE[ZFileWriteHandle]);
        else
            res = gzwrite(FILEHANDLE[ZFileWriteHandle],
                      ZFileWriteBlock,
                      ZFileWriteSize);

        if (res!=(int)ZFileWriteSize)
            return(0xFFFFFFFF);
#ifdef CCBETA
    }
#endif
	return(0);
}

DWORD ZFileTell()
{
#ifdef CCBETA
    ZLog_Message("ZFileTell %d",ZFileTellHandle);

    if(FILETYPE[ZFileTellHandle]==1)
    {
        return(FILECURPOS[ZFileTellHandle]);
    }
    else
    {
#endif
        int res = 0;
        if (TextFile) {
            res = ftell(FILEHANDLE[ZFileTellHandle]);
            if (res == -1) fprintf(stderr, "Oups!! gzTell\n");
            return(res);
        } else return gztell(FILEHANDLE[ZFileTellHandle]);
#ifdef CCBETA
    }
#endif
}


DWORD ZFileDelete()
{
  return(remove(ZFileDelFName));
}

DWORD ZFileGetFTime()
{
  struct _stat filestat;

  ZFTime=0;

  if (_stat(ZFFTimeFName, &filestat) < 0) ZFDate=0;
     else ZFDate = filestat.st_mtime;

  return 0;
}

DWORD ZFileMKDir()
{
  return(mkdir(MKPath));
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
  return (DWORD) (getcwd(DirName,128));
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


int FindFirstHandle;
int TempFind;
struct _finddata_t FindDataStruct;

DWORD ZFileFindNext()
{
   TempFind=_findnext(FindFirstHandle,&FindDataStruct);
   if(TempFind==-1) return(-1);

   *(char *)(DTALocPos+0x15)=0;

   if(ZFileFindATTRIB&0x10 && (FindDataStruct.attrib&0x10)==0) return(ZFileFindNext());
   if(((ZFileFindATTRIB&0x10)==0) && FindDataStruct.attrib&0x10) return(ZFileFindNext());

   if(FindDataStruct.attrib&_A_SUBDIR)  *(char *)(DTALocPos+0x15)=0x10;
   strcpy((char *)DTALocPos+0x1E,FindDataStruct.name);
   if(TempFind==-1) return(-1);
   return(0);
}

DWORD ZFileFindFirst()
{
   FindFirstHandle=_findfirst(ZFileFindPATH,&FindDataStruct);
   *(char *)(DTALocPos+0x15)=0;
   TempFind=0;
   if(FindFirstHandle==-1) return(-1);
   if(ZFileFindATTRIB&0x10 && (FindDataStruct.attrib&0x10)==0) return(ZFileFindNext());
   if(((ZFileFindATTRIB&0x10)==0) && FindDataStruct.attrib&0x10) return(ZFileFindNext());

   if(FindDataStruct.attrib&_A_SUBDIR)  *(char *)(DTALocPos+0x15)=0x10;
   strcpy((char *) DTALocPos+0x1E,FindDataStruct.name);
   if(FindFirstHandle==-1) return(-1);
   return(0);
}


DWORD ZFileFindEnd()  // for compatibility with windows later
{
   _findclose(FindFirstHandle);
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


