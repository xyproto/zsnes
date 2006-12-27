/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

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



/**********************************************************\
* ZSNES TCP/IP MODULE FOR NETWORK PLAY                     *
*                                                          *
* Coded by the ZSNES team                                  *
*   TCP/IP drivers coded by _Demo_, revised by Pharos      *
*   UDP drivers coded by _Demo_, revised by zsKnight       *
*   Gameplay netplay implementation by zsKnight            *
*   UDP Packet loss/out of order algorithm/implementation  *
*     by zsKnight, assistance on normal packets by Pharos  *
\**********************************************************/

// UDP Algorithm:
//
// UDP Header (1 byte): 1 = Normal Packet w/ reply req, 2 = Reply Packet,
//            3 = Gameplay Packet (single byte),
//            4 = Gameplay Packet (larger packet), 5 = Re-request gameplay
//            packet
//
// Normal Packets:
//   Note: The actual implementation turned out to be quite different
//         than the below descriptions.
//   First byte contains the packet counter, followed by packet contents.
//   Remote will send a Reply Packet (just contains packet counter)
//   Each packet buffer will have a timer counter which decreases after
//     every 1/60 seconds (value set at start is 60).  If this reaches 0
//     that packet will be re-sent and reset the timer value back to 60.
//   If the local side receives the reply packet, it will set the timer
//     counter to -1.
//
// Gameplay Packets:
//   Note: Gameplay counter is separate from normal packet counter.
//   Note2: When referring to TCP/IP, it refers to the Normal Packets above.
//   Each packet in TCP/IP will contain a byte counter when UDP is
//     enabled.
//   Each UDP packet will contain a byte counter, the number of packets,
//     then each packet will contain a byte size only if there are > 1
//     packets.  If the packet is just one byte long and contains a value<2,
//     it will follow by a byte containing info on how many packets its has
//     been like that for (it will not go beyond 32).  If the packet is
//     more than one byte long, it will repeat that packet as the extra
//     packets for the next 3 packets, with the first byte of those packets
//     as the byte counter of that packet, then the second as the size.
//     Also, the send data will be stored in a 256*32 byte buffer in case
//     of packet loss.
//   When receiving, since no UDP packets will exceed 32bytes in length,
//     there will be a 256*32 byte buffer and a 256 byte flag buffer.
//     The flag clearing pointer will move at an offset of 128 from the
//     actual point of the receive buffer.  When it receives data from
//     the UDP (or TCP), if the byte count of the data matches the
//     receive pointer, it will just send the data directly and increase the
//     receive pointer.  Else it will fill the buffer accordingly based on
//     the send data (for a maximum of 32 bytes).  Then if the bit on the
//     flag buffer is set for the current receive pointer, return the
//     appropriate buffer and increase receive pointer.
//   In case of packet loss, if no data has been received for every 500ms, the
//     local side would send a re-send package request.  What this would
//     do is let the remote side build up a package containing all the
//     data from the requested send point to the current receive point.
//     A resend request will start off with 0x00,0xFF, then the counter
//     number.  A resent packet will start off with 0x00,0xFE, the # of
//     packets, then the packet data (size of packet, data).  A resend will
//     only be done if the requested packet is within the past 64 packets.
//   In-game chat will be moved to a separate packet in TCP/IP

#ifdef __UNIXSDL__
#include "gblhdr.h"
#define closesocket(A) close(A)
#define CopyMemory(A,B,C) memcpy(A,B,C)
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#define UINT unsigned int
#define WORD unsigned short
#define SOCKET int
#define SOCKADDR_IN struct sockaddr_in
#define LPSOCKADDR struct sockaddr*
#define LPHOSTENT struct hostent*
#define HOSTENT struct hostent
#define LPINADDR struct in_addr*
#define LPIN_ADDR struct in_addr*
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define ioctlsocket ioctl
#define FD_SET_VAR fd_set
#else
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#define FD_SET_VAR FD_SET
#endif

int RecvPtr;
int RecvPtr2;
unsigned char RecvFlags[256];
unsigned char RecvBuffer[256*32];
int RecvBufferSize[256];

int SendPtr;
int SendPtr2;
unsigned char SendBuffer[256*32];
int SendBufferSize[256];

int SendRepeated;

int PrevSPacket[16];
int PrevSData[16*32];
int PrevSSize[16];
int PrevSPtr[16];

int tcperr;
unsigned short portval;
int UDPEnable = 1;
int UDPConfig = 1;
int UDPBackTrace = 6;
int blahblahblah = 0;
int CounterA = -1;
int CounterB = -1;
int UDPMode2 = 0;

int packetnum,packetnumhead;
int packetrecvhead;
unsigned char packetdata[2048*16];
unsigned char packetrdata[2048*32];
int packetconfirm[256];
int packetreceived[256];
int packetreceivesize[256];
int packetsize[256];
unsigned char cpacketdata[2048+32];
UINT ConnectAddr;
int packettimeleft[256];
int packetresent[256];
int PacketCounter=0;
unsigned char CLatencyVal=0;


SOCKET   gamesocket;    /* tcp socket for the game */
SOCKET   serversocket;  /* tcp socket when the server is listening */

SOCKET   ugamesocket;    /* udp socket sending */
SOCKET   userversocket;  /* udp socket listening */

SOCKADDR_IN    serveraddress; /* address of the server */
SOCKADDR_IN    ugameaddress; /* address of the server */
SOCKADDR_IN    userveraddress; /* address of the server */

char blah[256];
char remotehost[256];
char hostname[50] = "IP N/A";

// Function Prototypes

int SendData(int dsize,unsigned char *dptr);
int GetData(int dsize,unsigned char *dptr);
int GetLeftUDP();

/**********************************************************\
* Initialize the zsnes tcpip module                        *
* - no parameters                                          *
* - return 0 on success other value on error               *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

int InitTCP()
{
#ifndef __UNIXSDL__
   WORD versionneeded = MAKEWORD(2,2);
   WSADATA wsadata;
#endif

   UDPEnable=0;

#ifndef __UNIXSDL__
   /* Startup winsock */
   WSAStartup(versionneeded, &wsadata);

   /* Verify version number and exit on wrong version */
   if (wsadata.wVersion != versionneeded)
   {
      return(-1);
   }
   serversocket=INVALID_SOCKET;
#endif
   return(0);
}


/**********************************************************\
* Deinitialize the zsnes tcpip module                      *
* - no parameters                                          *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

void DeInitTCP()
{
#ifndef __UNIXSDL__
   WSACleanup();
#endif
}

/**********************************************************\
* Gets UDP Status through sending data                     *
* - no parameters                                          *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

void GetUDPStatus() {
  int retval;

  UDPEnable=UDPConfig;

  if (!UDPEnable){
    blah[0]=0;
    retval = send(gamesocket,blah,1,0);
    gethostname(blah,255);
    retval = send(gamesocket,blah,strlen(blah),0);
  }
  else {
    blah[0]=1;
    retval = send(gamesocket,blah,1,0);
    gethostname(blah,255);
    retval = send(gamesocket,blah,strlen(&blah[1])+1,0);
  }

  retval = recv(gamesocket,blah,256,0);
  if (blah[0]==0) UDPEnable=0;
  retval = recv(gamesocket,blah,256,0);
}

/**********************************************************\
* Connect to game server                                   *
* - parameters                                             *
*     - pointer server name                                *
*     - server port                                        *
* - return 0 on success other value on error               *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

int isipval(char *name){
  int i=0;

  while(name[i]!=0){
    if (!((name[i]=='.') || ((name[i]>='0') && (name[i]<='9'))))
      return(0);
    i++;
  }
  return(1);
}

int ConnectServer(char *servername, unsigned int port)
{
   char blah[255];
   int retval,i;
   LPHOSTENT host1=NULL;
   int yesip;

   packetnum = 0;
   packetnumhead = 0;
   packetrecvhead = 0;
   RecvPtr = 0;
   SendPtr = 0;
   RecvPtr2 = 0;
   SendPtr2 = 0;

   ConnectAddr = 0;
   SendRepeated = 0;
   for (i=0;i<16;i++)
     PrevSPacket[i]=0;

   /* get host and verify if it is valid */
   yesip = isipval(servername);
   if (!yesip){
     host1 = gethostbyname(servername);
     if (host1 == NULL)
     {
        return(-1);
     }
   }

//   return(-1);
   if (UDPConfig) UDPEnable = 1;

   if (UDPEnable)
   {
      PacketCounter=1;
      for (i=0;i<256;i++) {packettimeleft[i]=-1; packetconfirm[i]=1; packetreceived[i]=0; RecvFlags[i]=0;}

      userveraddress.sin_family = AF_INET;
      ugameaddress.sin_family = AF_INET;

      if (!yesip)
      {
         ugameaddress.sin_addr = *( (LPIN_ADDR) *host1->h_addr_list );
      }
      else
      {
         ugameaddress.sin_addr.s_addr = inet_addr(servername);
      }

      ConnectAddr = ugameaddress.sin_addr.s_addr;

      userveraddress.sin_addr.s_addr = INADDR_ANY;

//      port++;
      ugameaddress.sin_port = htons((unsigned short) port);
      userveraddress.sin_port = htons((unsigned short) port);
//      port--;

      userversocket = socket(AF_INET, SOCK_DGRAM,0);
      ugamesocket = socket(AF_INET, SOCK_DGRAM,0);

      if (ugamesocket == INVALID_SOCKET)
      {
#ifdef __UNIXSDL__
         STUB_FUNCTION;
#else
                 tcperr=WSAGetLastError();
                 sprintf(blah,"Could not initialize UDP(2) : %d",tcperr);
                 MessageBox(NULL,blah,"Error",MB_SYSTEMMODAL|MB_OK);
#endif
                 return(-2);
      }

      if (userversocket == INVALID_SOCKET)
      {
#ifdef __UNIXSDL__
         STUB_FUNCTION;
#else
                 tcperr=WSAGetLastError();
                 sprintf(blah,"Could not initialize UDP(2.5) : %d",tcperr);
                 MessageBox(NULL,blah,"Error",MB_SYSTEMMODAL|MB_OK);
#endif
       return(-2);
      }

      if (bind(userversocket,(struct sockaddr*)&userveraddress,sizeof(userveraddress))==
          SOCKET_ERROR)
      {
#ifdef __UNIXSDL__
         STUB_FUNCTION;
#else
         tcperr=WSAGetLastError();
         sprintf(blah,"Could not initialize UDP(16) : %d",tcperr);
         MessageBox(NULL,blah,"Error",MB_SYSTEMMODAL|MB_OK);
#endif
         return(-2);
      }


//      blah[0]=1;
//      retval = sendto(ugamesocket,blah,1,0,(struct sockaddr*)&ugameaddress,sizeof(struct sockaddr));
//      if (retval == SOCKET_ERROR) return(-1);

      blah[0]=1;
      SendData(1,blah);

//      retval = sendto(ugamesocket,blah,5,0,(struct sockaddr*)&ugameaddress,sizeof(struct sockaddr));
//      blah[0]=0;
//      i = sizeof(struct sockaddr);
//      retval = recvfrom(userversocket,blah,5,0,(struct sockaddr*)&userveraddress,&i);

//      MessageBox(NULL,blah,
//              "Error",
//              MB_SYSTEMMODAL|MB_OK);

      return(0);

//      retval = send(gamesocket,blah,1,0);
//      retval = recv(gamesocket,blah,1,0);
   }


   /* create the game socket and verify if it is valid */
   gamesocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (gamesocket == INVALID_SOCKET)
   {
      return(-2);
   }


   /* initialize server address */
   serveraddress.sin_family = AF_INET;
   if (!yesip)
     serveraddress.sin_addr = *( (LPIN_ADDR) *host1->h_addr_list );
   else
     serveraddress.sin_addr.s_addr = inet_addr(servername);

   serveraddress.sin_port = htons((unsigned short)port);


   /* try to connect to the server */
   retval = connect( gamesocket,
                     (LPSOCKADDR)&serveraddress,
                     sizeof(struct sockaddr));
   if (retval == SOCKET_ERROR)
   {
#ifdef __UNIXSDL__
      STUB_FUNCTION;
#else
      sprintf(blah,"Could not connect to other side");
      MessageBox(NULL,blah,
              "Error",
              MB_SYSTEMMODAL|MB_OK);
#endif

      closesocket(gamesocket);
      return(-3);
   }

//   GetUDPStatus();

   return(0);
}

int WaitForServer(){
  int i;

  if (UDPEnable){
    if ((i=GetData(1,blah))){
      if ((i==1) && (blah[0]==1))
        return(1);
    }
    return(0);
  }
  return(1);
}


/**********************************************************\
* Disconnect from game server                              *
* - no parameters                                          *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

void Disconnect()
{
   if (UDPEnable)
   {
     closesocket(ugamesocket);
     closesocket(userversocket);
     return;
   }
   PacketCounter=0;
   closesocket(gamesocket);
}


/**********************************************************\
* Start the game server                                    *
* - parameters                                             *
      - port number
* - return 0 on success other value on error               *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

int StartServerCycle(unsigned short port)
{
   int retval,i;

   portval = port;
   packetnum = 0;
   packetnumhead = 0;
   packetrecvhead = 0;
   ConnectAddr = 0;
   SendRepeated = 0;
   RecvPtr = 0;
   SendPtr = 0;
   RecvPtr2 = 0;
   SendPtr2 = 0;

   for (i=0;i<16;i++)
     PrevSPacket[i]=0;


   if (UDPConfig) UDPEnable = 1;

   if (UDPEnable)
   {
      /* get host and verify if it is valid */
      PacketCounter=1;
      for (i=0;i<256;i++) {packettimeleft[i]=-1; packetconfirm[i]=1; packetreceived[i]=0; RecvFlags[i]=0;}

      userveraddress.sin_family = AF_INET;
      ugameaddress.sin_family = AF_INET;

      userveraddress.sin_addr.s_addr = INADDR_ANY;
      ugameaddress.sin_addr.s_addr = INADDR_ANY;

//      portval++;
      ugameaddress.sin_port = htons((unsigned short) portval);
      userveraddress.sin_port = htons((unsigned short) portval);
//      portval--;

      userversocket = socket(AF_INET, SOCK_DGRAM,0);
      ugamesocket = socket(AF_INET, SOCK_DGRAM,0);

      if (userversocket == INVALID_SOCKET)
      {
#ifdef __UNIXSDL__
         STUB_FUNCTION;
#else
         tcperr=WSAGetLastError();
                 sprintf(blah,"Could not initialize UDP(5) : %d",tcperr);
                 MessageBox(NULL,blah,"Error",MB_SYSTEMMODAL|MB_OK);
#endif
                 return(-2);
      }
      if (bind(userversocket,(struct sockaddr*)&userveraddress,sizeof(userveraddress))==
          SOCKET_ERROR)
      {
#ifdef __UNIXSDL__
         STUB_FUNCTION;
#else
         tcperr=WSAGetLastError();
         sprintf(blah,"Could not initialize UDP(6) : %d",tcperr);
         MessageBox(NULL,blah,"Error",MB_SYSTEMMODAL|MB_OK);
#endif
         return(-2);
      }


      blah[0]=2;
      blah[1]='C';
      blah[2]='B';
      blah[3]='A';
      blah[4]=0;


//      retval = recvfrom(userversocket,blah,5,0,
//                        (struct sockaddr *)&userveraddress,&socklen);

      ugameaddress.sin_addr.s_addr = userveraddress.sin_addr.s_addr;

      ugamesocket = socket(AF_INET, SOCK_DGRAM,0);

      return(0);

//      retval = send(gamesocket,blah,1,0);
//      retval = recv(gamesocket,blah,1,0);

   }

   /* Create the listen socket */
   serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (serversocket == INVALID_SOCKET)
   {
#ifndef __UNIXSDL__
     tcperr=WSAGetLastError();
#endif

      return(-1);
   }

   serveraddress.sin_family = AF_INET;
   serveraddress.sin_addr.s_addr = INADDR_ANY;
   serveraddress.sin_port = htons(port);

   /* bind name and socket */
   retval = bind(serversocket,
                 (LPSOCKADDR)&serveraddress,
                 sizeof(struct sockaddr));
   if (retval == SOCKET_ERROR)
   {
#ifndef __UNIXSDL__
     tcperr=WSAGetLastError();
#endif
      closesocket(serversocket);
      return(-2);
   }

   /* setup socket to listen */
   retval = listen(serversocket, SOMAXCONN);
   if (retval == SOCKET_ERROR)
   {
#ifndef __UNIXSDL__
      tcperr=WSAGetLastError();
#endif
      closesocket(serversocket);
      return(-3);
   }

   return 0;
}

int acceptzuser()
{
   if (UDPEnable)
   {
     return(0);
   }

   /* wait for connection */

   gamesocket = accept(serversocket, NULL, NULL);
   if (gamesocket == INVALID_SOCKET)
   {
#ifndef __UNIXSDL__
      tcperr=WSAGetLastError();
#endif
      closesocket(serversocket);
      serversocket=-1;
      return(-1);
   }

//   GetUDPStatus();

   return(0);
}

int ServerCheckNewClient()
{
   FD_SET_VAR zrf;
   struct timeval nto;
   int r;

        if (UDPEnable)
        {
          r=GetData(256,blah);
          if (r == -1) return(-1);
          if (r > 0){
            ugameaddress.sin_addr.s_addr=userveraddress.sin_addr.s_addr;
            ConnectAddr = ugameaddress.sin_addr.s_addr;
            blah[0]=1;
            r=SendData(1,blah);
            return(1);
          }
          return(0);
        }

   if(serversocket == INVALID_SOCKET)
   {
      return(-1);
   }
   nto.tv_sec=0;
   nto.tv_usec=0; /* return immediately */

        FD_ZERO(&zrf);
   FD_SET(serversocket,&zrf);
   r=select(serversocket+1,&zrf,0,0,&nto);

   if(r == -1)
   {
#ifndef __UNIXSDL__
                tcperr=WSAGetLastError();
#endif
                return(-2);
   }
   if(r == 0)
   {
      return(0);
   }
        if(FD_ISSET(serversocket,&zrf))
   {
      return 1;
   }
   return(0);

}


/**********************************************************\
* Stop the game server                                     *
* - no parameters                                          *
*                                                          *
* - no known side effects                                  *
\**********************************************************/

void StopServer()
{
   if (UDPEnable)
   {
     closesocket(ugamesocket);
     closesocket(userversocket);
     return;
   }
   PacketCounter=0;
   closesocket(gamesocket);
   closesocket(serversocket);
}


/**********************************************************\
* Send data                                                *
* - parameters :                                           *
*        - size of data                                    *
*        - pointer to data                                 *
* - return 0 on success other value on error               *
*                                                          *
* - side effects :                                         *
*        - close the socket on error                       *
\**********************************************************/

int PacketReceive()
{
   int dataleft,i,i2,i3,i4,i5,i6,i7,retval;

   dataleft=GetLeftUDP();
   if (dataleft<=0) return(dataleft);
   i = sizeof(userveraddress);
   retval = recvfrom(userversocket,cpacketdata,2048+32,0,(struct sockaddr *)&userveraddress,&i);
   if ((ConnectAddr!=0) && (ConnectAddr != userveraddress.sin_addr.s_addr)) return(0);
   if (retval == SOCKET_ERROR)
   {
      closesocket(ugamesocket);
      return(-1);
   }
   if ((cpacketdata[0]==1) && (retval>0)) {
     i=(unsigned char)cpacketdata[1];
     blah[0]=2;
     blah[1]=cpacketdata[1];
     sendto(ugamesocket,blah,2,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
     if (!packetreceived[i]){
       packetreceived[i]=1;
       packetreceivesize[i]=retval-2;
       CopyMemory(&(packetrdata[2048*(i & 0x0F)]),&(cpacketdata[2]),retval-2);
     }
   }
   if (cpacketdata[0]==2){
     packetconfirm[cpacketdata[1]]=1;
     while ((packetconfirm[packetnumhead]) && (packetnum!=packetnumhead))
       packetnumhead=(packetnumhead+1) & 0xFF;
   }

   if ((cpacketdata[0]==16) && (cpacketdata[1]!=SendPtr)){
      i=cpacketdata[1];
      cpacketdata[0]=17;
      cpacketdata[2]=SendPtr;
      i3=3;
      while (i!=SendPtr){
        cpacketdata[i3]=SendBufferSize[i];
        i3++;
        for (i4=0;i4<SendBufferSize[i];i4++){
          cpacketdata[i3]=SendBuffer[i4+(i << 5)];
          i3++;
        }
        i=(i+1) & 0xFF;
      }
      sendto(ugamesocket,cpacketdata,i3,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
      return(0);
   }

   if (cpacketdata[0]==17){
     i2=cpacketdata[1];
     i3=3;
     while (i2!=cpacketdata[2]){
       i4=cpacketdata[i3];
       i3++;
       RecvFlags[i2]=1;
       RecvBufferSize[i2]=i4;
       for (i5=0;i5<i4;i5++){
         RecvBuffer[(i2 << 5)+i5]=cpacketdata[i3];
         i3++;
       }
       i2=(i2+1) & 0xFF;
     }
   }

   i2=RecvPtr+(RecvPtr2 << 8);
   i3=(cpacketdata[2]+(cpacketdata[3] << 8))-i2;
   if (i3<0) i3+=65536;

   if ((((cpacketdata[0] & 0xF7)==4) || ((cpacketdata[0] & 0xF7)==5))
      && ((i3>=0) && (i3<=127))) {


      CLatencyVal=cpacketdata[1];
      i=cpacketdata[2];
      i3=0;

      if ((cpacketdata[0] & 0x07)==4){
        for (i2=0;i2<cpacketdata[4];i2++){
          RecvBuffer[((i-i2) & 0xFF) << 5] = 0;
          RecvBuffer[(((i-i2) & 0xFF) << 5)+1] = CLatencyVal;
          RecvFlags[((i-i2) & 0xFF)] = 1;
          RecvBufferSize[((i-i2) & 0xFF)] = 2;
        }
        i3+=5;
      } else {
        for (i2=0;i2<cpacketdata[4];i2++){
          RecvBuffer[(i << 5) + i2] = cpacketdata[i2+5];
        }
        RecvFlags[i] = 1;
        RecvBufferSize[i] = cpacketdata[4];
        i3+=cpacketdata[4]+5;
      }
      if (cpacketdata[0] & 0x08){
        retval=cpacketdata[i3];
        i3++;
        for (i2=0;i2<retval;i2++){
          i=cpacketdata[i3];
          i5=cpacketdata[i3+1];
          i3+=2;
          RecvFlags[i] = 1;
          RecvBufferSize[i] = i5;
          if ((cpacketdata[i3]==0) && (i5==3)){
            i7 = cpacketdata[i3+2];
            for (i6=0;i6<i7;i6++){
              RecvFlags[(i-i6) & 0xFF] = 1;
              RecvBufferSize[(i-i6) & 0xFF] = 2;
            }
            for (i4=0;i4<i5;i4++){
              for (i6=0;i6<i7;i6++)
                RecvBuffer[(((i-i6) & 0xFF) << 5)+i4]=cpacketdata[i3];
              i3++;
            }
          }
          else
          {
           for (i4=0;i4<i5;i4++){
              RecvBuffer[(i << 5)+i4]=cpacketdata[i3];
              i3++;
            }
          }
        }
      }
   }

   return(0);
}


void PacketResend()
{
   int i;
   for (i=0;i<256;i++) {
     if ((packettimeleft[i]==0) && (packetconfirm[i]==0)){
       packettimeleft[i]=180;
       if (packetresent[i]==1) packettimeleft[i]=60;
       if (packetresent[i]==2) packettimeleft[i]=90;
       if (packetsize[i]>512) packettimeleft[packetnum]=60*3;
       packetresent[i]++;
       CopyMemory(&(cpacketdata[2]),&(packetdata[2048*(i & 0x0F)]),packetsize[i]);
       cpacketdata[0]=1;
       cpacketdata[1]=(char)i;
       sendto(ugamesocket,cpacketdata,packetsize[i]+2,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
     }
   }
}

extern void UpdateVFrame(void);

int SendData(int dsize,unsigned char *dptr)
{
   int retval;

   if (UDPEnable){
/*      retval = sendto(ugamesocket,dptr,dsize,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
      if (retval == SOCKET_ERROR)
           {
         closesocket(gamesocket);
         return(-1);
           }
      return(0);  */

     if (((packetnum-packetnumhead) & 0xFF) >= 15){
//        sprintf(message1,"Packet Overflow.");
//        MessageBox (NULL, message1, "Init Error" , MB_ICONERROR );

        // wait for receive packet, call JoyRead while waiting
        while (((packetnum-packetnumhead) & 0xFF) >= 15){
           PacketResend();
           PacketReceive();
           UpdateVFrame();
           while ((packetconfirm[packetnumhead]) && (packetnum!=packetnumhead))
             packetnumhead=(packetnumhead+1) & 0xFF;
        }
     }
     CopyMemory(&(cpacketdata[2]),dptr,dsize);
     CopyMemory(&(packetdata[2048*(packetnum & 0x0F)]),dptr,dsize);
     packetsize[packetnum]=dsize;
     packetconfirm[packetnum]=0;
     cpacketdata[0]=1;
     cpacketdata[1]=(char)packetnum;
     retval = sendto(ugamesocket,cpacketdata,dsize+2,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
     packettimeleft[packetnum]=60;
     if (dsize>512) packettimeleft[packetnum]=90;
     packetresent[packetnum]=1;
     packetnum=(packetnum+1) & 0xFF;
     if (retval == SOCKET_ERROR)
          {
        closesocket(ugamesocket);
        return(-1);
          }
     return(0);
   }

   /* send data with the socket */
   retval = send(gamesocket,dptr,dsize,0);
   if (retval == SOCKET_ERROR)
   {
      closesocket(gamesocket);
      return(-1);
   }
   return(0);
}

extern int PacketSendSize;
extern unsigned char PacketSendArray[2048+256];

int SendDataNop()
{
   return (SendData(PacketSendSize,PacketSendArray));
}


/**********************************************************\
* Send data UDP                                            *
* - parameters :                                           *
*        - size of data                                    *
*        - pointer to data                                 *
* - return 0 on success other value on error               *
*                                                          *
* - side effects :                                         *
*        - close the socket on error                       *
\**********************************************************/

int AttachEnd(int psb){
   int i,i2,i3,ps;
//int PrevSPacket[4];
//int PrevSData[4*32];
//int PrevSSize[4];

   ps=psb;
   i2=0;
   for (i=0;i<(UDPBackTrace-1);i++){
     if (PrevSPacket[i]) i2++;
   }
//   if (PrevSPacket[0]) i2=0;
   if (i2){
      cpacketdata[0]+=8;
      cpacketdata[ps]=(char)i2;
      ps++;
      for (i=0;i<(UDPBackTrace-1);i++){
        if (PrevSPacket[i]){
          cpacketdata[ps]=PrevSPtr[i];
          cpacketdata[ps+1]=PrevSSize[i];
          ps+=2;
          for (i3=0;i3<PrevSSize[i];i3++){
            cpacketdata[ps]=PrevSData[i*32+i3];
            ps++;
          }
        }
      }
      for (i=0;i<(UDPBackTrace-2);i++){
        PrevSPacket[i]=PrevSPacket[i+1];
        PrevSSize[i]=PrevSSize[i+1];
        PrevSPtr[i]=PrevSPtr[i+1];
        CopyMemory(&(PrevSData[i*32]),&(PrevSData[i*32+32]),32);
      }
   }

   return ps;
}

int SendDataUDP(int dsize,unsigned char *dptr)
{
   int retval,i;
   int packetsize;



//   return (SendData(dsize,dptr));

   if (UDPEnable){

/*int SendPtr;
char SendBuffer[256*32];
char SendBufferSize[256];*/
      blahblahblah++;

      packetsize = 0;

      for (i=0;i<dsize;i++)
        SendBuffer[SendPtr*32+i]=dptr[i];
      SendBufferSize[SendPtr]=dsize;

      if ((dsize == 2) && (dptr[0]<=1)){
        if (SendRepeated < 32) SendRepeated++;
        cpacketdata[0]=4;
        cpacketdata[1]=dptr[1];
        cpacketdata[2]=(char)SendPtr;
        cpacketdata[3]=(char)SendPtr2;
        cpacketdata[4]=(char)SendRepeated;
        packetsize=5;
        packetsize=AttachEnd(packetsize);
        PrevSPacket[UDPBackTrace-2]=0;
        SendPtr=(SendPtr+1) & 0xFF;
        if (!SendPtr) SendPtr2=(SendPtr2+1) & 0xFF;
        retval = sendto(ugamesocket,cpacketdata,packetsize,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
        if (retval == SOCKET_ERROR)
           {
         closesocket(gamesocket);
         return(-1);
           }
      } else {
        if (SendRepeated){
          PrevSPacket[UDPBackTrace-2]=1;
          PrevSSize[UDPBackTrace-2]=3;
          PrevSData[(UDPBackTrace-2)*32]=0;
          PrevSData[(UDPBackTrace-2)*32+1]=dptr[1];
          PrevSData[(UDPBackTrace-2)*32+2]=SendRepeated;
          PrevSPtr[UDPBackTrace-2]=(SendPtr-1) & 0xFF;
        }
        SendRepeated=0;
        cpacketdata[0]=5;
        cpacketdata[1]=dptr[1];
        cpacketdata[2]=SendPtr;
        cpacketdata[3]=SendPtr2;
        cpacketdata[4]=dsize;
        packetsize=5;
        for (i=0;i<dsize;i++)
          cpacketdata[i+5]=dptr[i];
        packetsize+=dsize;
        packetsize=AttachEnd(packetsize);

        PrevSPacket[UDPBackTrace-2]=1;
        PrevSSize[UDPBackTrace-2]=dsize;
        for (i=0;i<dsize;i++)
          PrevSData[(UDPBackTrace-2)*32+i]=dptr[i];
        PrevSPtr[UDPBackTrace-2]=SendPtr;

        SendPtr=(SendPtr+1) & 0xFF;
        if (!SendPtr) SendPtr2=(SendPtr2+1) & 0xFF;
        retval = sendto(ugamesocket,cpacketdata,packetsize,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
        if (retval == SOCKET_ERROR)
           {
         closesocket(gamesocket);
         return(-1);
           }
      }
      return(0);
   }

   /* send data with the socket */
   retval = sendto(gamesocket,dptr,dsize,0,(struct sockaddr *) &ugameaddress,sizeof(struct sockaddr));
   if (retval == SOCKET_ERROR)
   {
      closesocket(gamesocket);
      return(-1);
   }
   return(0);
}

int SendDataUDPNop()
{
   return (SendDataUDP(PacketSendSize,PacketSendArray));
}

/**********************************************************\
* Get data left                                            *
* - return size left on success negative value on error    *
*                                                          *
* - side effects :                                         *
*        - close the socket on error                       *
\**********************************************************/

int GetLeft()
{
   int retval;
   int tempsize;
   retval = ioctlsocket(gamesocket,FIONREAD,&tempsize);
   if (retval == SOCKET_ERROR)
   {
      closesocket(gamesocket);
      return(-1);
   }
   return(tempsize);
}

int GetLeftUDP()
{
   FD_SET_VAR zrf;
   struct timeval nto;
   int r;

   nto.tv_sec=0;
   nto.tv_usec=0; /* return immediately */

   FD_ZERO(&zrf);
   FD_SET(userversocket,&zrf);
   r=select(userversocket+1,&zrf,0,0,&nto);

   if (r == SOCKET_ERROR)
   {
      closesocket(userversocket);
      return(-1);
   }
   return(r);
}

/**********************************************************\
* Receive data                                             *
* - parameters :                                           *
*        - size of data                                    *
*        - pointer to data                                 *
* - return size on success negative value on error         *
*                                                          *
* - side effects :                                         *
*        - close the socket on error                       *
\**********************************************************/

int GetData(int dsize,unsigned char *dptr)
{
   int retval,i;
   int dataleft;

   retval=0;

      // Temporary UDP routines
   if (UDPEnable) {

     PacketResend();
     PacketReceive();

     i=packetrecvhead;
     if (packetreceived[i]){
       CopyMemory(dptr,&(packetrdata[2048*(i & 0x0F)]),packetreceivesize[i]);
       retval = packetreceivesize[i];
       packetreceived[(i+128) & 0xFF]=0;
       packetrecvhead=(packetrecvhead+1) & 0xFF;
       return(retval);
     }

     i=RecvPtr;
     if ((RecvFlags[i]) && (UDPMode2)){
       CopyMemory(dptr,&(RecvBuffer[32*i]),RecvBufferSize[i]);
       retval = RecvBufferSize[i];
       RecvFlags[(i+128) & 0xFF]=0;
       RecvPtr=(RecvPtr+1) & 0xFF;
       if (!RecvPtr) RecvPtr2=(RecvPtr2+1) & 0xFF;
       CounterA=90;
       return(retval);
     }

     if ((CounterA==0) & (UDPMode2)){
       // Send 16+RecvPtr
       cpacketdata[0]=16;
       cpacketdata[1]=RecvPtr;
       sendto(ugamesocket,cpacketdata,2,0,(struct sockaddr *)&ugameaddress,sizeof(ugameaddress));
       CounterA=90;
       return(0);
     }

     return(0);
   }

   dataleft=GetLeft();
   if(dataleft==0) return(0);

   if(dataleft<dsize)
   {
      dsize=dataleft;
   }
   /* get data with the socket */
   retval = recv(gamesocket,dptr,dsize,0);
   if (retval == SOCKET_ERROR)
   {
      closesocket(gamesocket);
      return(-1);
   }
   return(retval);
}

extern unsigned char PacketRecvArray[2048+256];

int GetDataNop()
{
   return (GetData(2048,PacketRecvArray));
}

void GetHostName()
{
  HOSTENT* phe;

  if (!InitTCP()){

    strcpy(hostname,"YOUR IP: ");
    gethostname(blah,255);
    phe = gethostbyname(blah);
    strcpy(blah, inet_ntoa(*(struct in_addr*)phe->h_addr));
    strcat(hostname,blah);
  }
}

void UDPWait1Sec(){
  CounterB=60;
  while (CounterB>0)
    UpdateVFrame();
}

void UDPClearVars(){
  int i;
  CounterA=-1;
  RecvPtr = 0;
  SendPtr = 0;
  for (i=0;i<16;i++)
    PrevSPacket[i]=0;
  for (i=0;i<256;i++)
    RecvFlags[i]=0;
}

void UDPEnableMode(){
  UDPMode2=1;
}

void UDPDisableMode(){
  UDPMode2=0;
}

void WinErrorA2(void){
#ifdef __UNIXSDL__
    STUB_FUNCTION;
#else
    char message1[256];
    sprintf(message1,"Failed waiting for checksum.");
    MessageBox (NULL, message1, "Init Error" , MB_ICONERROR );
#endif
}

void WinErrorB2(void){
#ifdef __UNIXSDL__
    STUB_FUNCTION;
#else
    char message1[256];
    sprintf(message1,"Failed waiting for confirmation.");
    MessageBox (NULL, message1, "Init Error" , MB_ICONERROR );
#endif
}

void WinErrorC2(void){
#ifdef __UNIXSDL__
    STUB_FUNCTION;
#else
    char message1[256];
    sprintf(message1,"Failed waiting for confirmation(B).");
    MessageBox (NULL, message1, "Init Error" , MB_ICONERROR );
#endif
}



