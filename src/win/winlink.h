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

#ifndef WINLINK_H
#define WINLINK_H
extern "C"
{
  BYTE changeRes = 1;
  extern DWORD converta;
  extern unsigned int BitConv32Ptr;
  extern unsigned int RGBtoYUVPtr;
  extern unsigned short resolutn;
  extern BYTE GUIWFVID[];
  extern BYTE GUIDSIZE[];
  extern BYTE GUISMODE[];
  extern BYTE GUIDSMODE[];
  extern BYTE GUIHQ2X[];
  extern BYTE GUIHQ3X[];
  extern BYTE GUIHQ4X[];
  extern BYTE GUINTVID[];
  extern BYTE hqFilterlevel;
}

DWORD FirstVid = 1;
DWORD FirstFull = 1;
DWORD DMode = 0;
DWORD SMode = 0;
DWORD DSMode = 0;
DWORD NTSCMode = 0;
DWORD prevHQMode = ~0;
DWORD prevNTSCMode = 0;
DWORD prevScanlines = ~0;
WORD Refresh = 0;

#endif
