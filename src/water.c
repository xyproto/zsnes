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
#include "gblhdr.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#endif

#include "fixsin.h"

extern char *vidbuffer;

#define SCRW 288
#define SCRH 224

static unsigned char vscr[SCRW*SCRH];


static int Height[2][SCRW*SCRH];


static void DrawWaterNoLight(int *ptr);
void DrawWaterWithLight(int *ptr,int light);
static void SineBlob(int x, int y, int radius, int height, int page);
static void CalcWater(int *nptr,int *optr,int density);

//static int x,y;
static int ox=80,oy=60;
static int xang,yang;
static int density=4;
static int Hpage=0;
static int mode=0x0001;
static int offset;
static int pheight=400;
static int radius=30;

void DrawWater(void)
{
	//		tslast=tscurrent;
//		tscurrent=time(NULL);
			
		DrawWaterNoLight(Height[Hpage]);
//		DrawWaterWithLight(Height[Hpage],1);	
		if(mode&2)	//  && (tscurrent-tslast))
		{
			int x,y;
         x=rand()%(SCRW-2)+1;
         y=rand()%(SCRH-2)+1;
         Height[Hpage][y*SCRW+x]=rand()%(pheight<<2);
		}
		
		/* the surfer */
		if(mode&1)
		{
			int x,y;
			x = (SCRW/2)
			+ ((((FSin( (xang* 65) >>8) >>8)
			* (FSin( (xang*349) >>8) >>8)) 
			* ((SCRW-8)/2)) >> 16);
			
			y = (SCRH/2)
			+ ((((FSin( (yang*377) >>8) >>8)
			*(FSin( (yang* 84) >>8) >>8))
			* ((SCRH-8)/2)) >> 16);
			xang += 13;
			yang += 12;
			
			if(mode & 0x4000)
			{
				offset = (oy+y)/2*SCRW + (ox+x)/2;
				Height[Hpage][offset] = pheight;
				Height[Hpage][offset + 1] =
				Height[Hpage][offset - 1] =
				Height[Hpage][offset + SCRW] =
				Height[Hpage][offset - SCRW] = pheight >> 1;

				offset = y*SCRW + x;
				Height[Hpage][offset] = pheight<<1;
				Height[Hpage][offset + 1] =
				Height[Hpage][offset - 1] =
				Height[Hpage][offset + SCRW] =
				Height[Hpage][offset - SCRW] = pheight;
			}
			else
			{
				SineBlob((ox+x)/2, (oy+y)/2, 3, -1200, Hpage);
				SineBlob(x, y, 4, -2000, Hpage);

			}

			ox = x;
			oy = y;			
			
		}
		
		if(mode&4)
		{
			if(rand()%20 == 0)
			{
/*
				if(mode & 0x4000)
//					HeightBlob(-1, -1, radius/2, pheight, Hpage);
				else
*/
					SineBlob(-1, -1, radius, -pheight*6, Hpage);
			}
		}	
		CalcWater(Height[Hpage^1], Height[Hpage], density);
		Hpage ^= 1; /* flip flop */	

}


void DrawWaterNoLight(int *ptr)
{
	int dx,dy;
	int x,y;
	int c;
	int p;
	
	int offset = SCRW+1;
	if(ptr == NULL)
	{
		return;
	}
	
	for(y=((SCRH-1)*SCRW); offset < y; offset+=2)
	{
		for(x = offset+SCRW-2;offset<x;offset++)
		{
			dx=ptr[offset]-ptr[offset+1];
			dy=ptr[offset]-ptr[offset+SCRW];
			p=offset+SCRW*(dy>>3)+(dx>>3);
/*			if(p>(SCRH*SCRW))
			{
				for(;p<(SCRH*SCRW);p-=SCRW);
			}
			if(p<0)
			{
				for(;p>0;p+=SCRW);
			}
*/
			if(p >= (SCRW*SCRH) )
			{
				p=(SCRW*SCRH)-1;
			}
			else
			{
				if(p < 0)
				{
					p=0;
				}
			}
			c=vidbuffer[p];
			(c<1) ? c=1 : (c > 31) ? c=31 : 0;
			vscr[offset]=c;
			offset++;
			dx=ptr[offset]-ptr[offset+1];
			dy=ptr[offset]-ptr[offset+SCRW];
			p=offset+SCRW*(dy>>3)+(dx>>3);
/*
			if(p>(SCRH*SCRW))
			{
				for(;p<(SCRH*SCRW);p-=SCRW);
			}
			if(p<0)
			{
				for(;p>=0;p+=SCRW);
			}
*/
			if(p >= (SCRW*SCRH) )
			{
				p=(SCRW*SCRH)-1;
			}
			else
			{
				if(p < 0)
				{
					p=0;
				}
			}

			c=vidbuffer[p];
			(c<1) ? c=1 : (c > 31) ? c=31 : 0;
			vscr[offset]=c;
		}
	}
	
	memcpy( vidbuffer,vscr,SCRW*SCRH);
//	frames++;
	
}


void DrawWaterWithLight(int *ptr,int light)
{
	int dx,dy;
	int x,y;
	int c;
	int p;
	
	int offset = SCRW+1;
	if(ptr == NULL)
	{
		return;
	}

	for(y=((SCRH-1)*SCRW); offset < y; offset+=2)
	{
		for(x = offset+SCRW-2;offset<x;offset++)
		{
			dx=ptr[offset]-ptr[offset+1];
			dy=ptr[offset]-ptr[offset+SCRW];

			p=offset+SCRW*(dy>>3)+(dx>>3);
			if(p>(SCRH*SCRW))
			{
            for(;p<(SCRH*SCRW);p-=SCRW);
			}
			if(p<0)
			{
				for(;p>=0;p+=SCRW);
			}
			c=vidbuffer[p];
			c-=(dx>>light);
			(c<0) ? c=0 : (c > 255) ? c=255 : 0;
			vscr[offset]=c;
			offset++;
			dx=ptr[offset]-ptr[offset+1];
			dy=ptr[offset]-ptr[offset+SCRW];
			p=offset+SCRW*(dy>>3)+(dx>>3);
			if(p>(SCRH*SCRW))
			{
            for(;p<(SCRH*SCRW);p-=SCRW);
			}
			if(p<0)
			{
				for(;p>=0;p+=SCRW);
			}
			c=vidbuffer[p];
			
			c-=(dx>>light);
			(c<0) ? c=0 : (c > 255) ? c=255 : 0;
			vscr[offset]=c;
		}
	}

	memcpy( vidbuffer,vscr,SCRW*SCRH);	
//	memcpy( VGLDisplay->Bitmap,vscr,SCRW*SCRH);
//	frames++;


}


void CalcWater(int *nptr,int *optr,int density)
{
	int newh;
	int count = SCRW+1;
	int x,y;
	
	for(y = (SCRH-1) * SCRW;count<y;count+=2)
	{
		for(x = count+SCRW-2;count<x;count++)
		{
			newh = ((optr[count+SCRW]
				+optr[count-SCRW]
				+optr[count+1]
				+optr[count-1]
				+optr[count-SCRW-1]
				+optr[count-SCRW+1]
				+optr[count+SCRW-1]
				+optr[count+SCRW+1]
				) >> 2)
				- nptr[count];
				
			nptr[count] = newh - (newh >> density);
		}
	}
	
}

void SineBlob(int x, int y, int radius, int height, int page)
{
  int cx, cy;
  int left,top,right,bottom;
  int square, dist;
  int radsquare = radius * radius;
  float length = (1024.0/(float)radius)*(1024.0/(float)radius);

  if(x<0) x = 1+radius+ rand()%(SCRW-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(SCRH-2*radius-1);


//  radsquare = (radius*radius) << 8;
  radsquare = (radius*radius);

//  height /= 8;

  left=-radius; right = radius;
  top=-radius; bottom = radius;


  // Perform edge clipping...
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > SCRW-1) right -= (x+radius-SCRW+1);
  if(y + radius > SCRH-1) bottom-= (y+radius-SCRH+1);

  for(cy = top; cy < bottom; cy++)
  {
    for(cx = left; cx < right; cx++)
    {
      square = cy*cy + cx*cx;
      if(square < radsquare)
      {
        dist = sqrt(square*length);
        Height[page][SCRW*(cy+y) + cx+x]
          += (int)((FCos(dist)+0xffff)*(height)) >> 19;
      }
    }
  }
}
