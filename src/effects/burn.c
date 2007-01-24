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

/*
   Fire effects implementation by Frank Jan Sorensen, Joachim Fenkes,
   Stefan Goehler, Jonas Quinn, et al.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#ifndef PI
#define M_PI 3.1415926535897932384626433832795
#else
#define M_PI PI
#endif
#endif

#define BUF_WIDTH 288
#define BUF_HEIGHT 224

const int rootrand = 20;	/* Max/Min decrease of the root of the flames */
const int decay = 5;		/* How far should the flames go up on the screen? This MUST be positive - JF */
const int miny = 1;		/* Starting line of the flame routine. (should be adjusted along with MinY above) */
const int smooth = 1;   /* How discrete can the flames be? */
const int minfire = 50; /* limit between the "starting to burn" and the "is burning" routines */
const int xstart = 0;  /* Starting position on the screen, should be divisible by 4 without remainder! */
const int xend = 287;  /* Guess! */
const int width = 1 + 287;
const int maxcolor = 110; /* Constant for the MakePal procedure */
const int fireincrease = 3; /* 3 = Wood, 90 = Gasoline */

typedef struct colorvalue
{
  unsigned char r, g, b;
} colorvalue;
typedef colorvalue vgapalettetype[256];

/* Converts (Hue, Saturation, Intensity) -> (RGB) */
void hsi2rgb(double h, double s, double i, struct colorvalue *c)
{
  double t;
  double rv, gv, bv;

  t = h;
  rv = 1 + s * sin(t - 2 * M_PI / 3);
  gv = 1 + s * sin(t);
  bv = 1 + s * sin(t + 2 * M_PI / 3);
  t = 255.999 * i / 2;
  c->r = (unsigned char) floor(rv * t);
  c->g = (unsigned char) floor(gv * t);
  c->b = (unsigned char) floor(bv * t);
}

void genpal()
{
  int i;
  vgapalettetype pal;

  memset(pal, 0, 3);
  for( i=1; i <= maxcolor; i ++)
  {
    hsi2rgb(4.6-1.5*i/maxcolor,(double)(i)/maxcolor,(double)(i)/maxcolor,&pal[i]);
  }
  for( i=maxcolor; i <= 255; i ++)
  {
    pal[i]=pal[i-1];
    {
      struct colorvalue *with = &pal[i];

      if (with->r<255) with->r += 1;
      if (with->r<255) with->r += 1;
      if ((~i & 1) && (with->g<215)) with->g += 1;
      if ((~i & 1) && (with->b<255)) with->b += 1;
    }
  }

}

int started = 0;

#define Randomize()
#define randint(a) (rand() % (a))
#define randreal() (((double)rand()) / ((double)RAND_MAX))
#define rand1(a) ((randint(a*2+1))-a)

unsigned char flamearray[BUF_WIDTH];
int morefire;
extern unsigned char *vidbuffer;

/* damn, this seems like such a waste */
static unsigned char pt[BUF_WIDTH * BUF_HEIGHT];

void DrawBurn()
{
  int i,j;
  int x,p;
  int v;

  if (!started)
  {
    started = 1;

    for( i=xstart; i <= xend; i++)
    {
      flamearray[i]=0;
    }

    Randomize();
    morefire=1;
    memset(pt, 0, BUF_HEIGHT * BUF_WIDTH);
  }

  /* Put the values from FlameArray on the bottom line of the screen */
  memcpy(pt+((BUF_HEIGHT-1)*BUF_WIDTH)+xstart,flamearray, width);

  /* This loop makes the actual flames */

  for( i=xstart; i <= xend; i++)
  {
    for( j=miny; j <= (BUF_HEIGHT-1); j ++)
    {
      v = pt[j*BUF_WIDTH + i];
      if ((v==0) ||
          (v<decay) ||
          (i<=xstart) ||
          (i>=xend))
        pt[(j-1)*BUF_WIDTH + i] = 0;
      else
        pt[((j-1)*BUF_WIDTH) + (i-(randint(3)-1))] = v - randint(decay);
    }
  }

  /* Match? */
  if (randint(150)==0)
  {
    memset(flamearray + xstart + randint(xend-xstart-5),255,5);
  }

  /* This loop controls the "root" of the
    flames, i.e. the values in FlameArray. */
  for( i=xstart; i <= xend; i++)
  {
    x=flamearray[i];

    if (x<minfire)    /* Increase by the "burnability" */
    {
      /* Starting to burn: */
      if (x>10)  x += randint(fireincrease);
    }
    else
    /* Otherwise randomize and increase by intensity (is burning) */
      x += rand1(rootrand)+morefire;
    if (x>255)  x=255;    /* X Too large? */
    flamearray[i]=x;
  }

  /* Smoothen the values of FrameArray to avoid "discrete" flames */
  p=0;
  for( i=xstart+smooth; i <= xend-smooth; i++)
  {
    x=0;
    for( j=-smooth; j <= smooth; j++) x += flamearray[i+j];
    flamearray[i] = x / ((smooth << 1) + 1);
  }

  for (x=0; x < BUF_WIDTH*BUF_HEIGHT; x++)
  {
    i = vidbuffer[x];
    j = pt[x] >> 3;

    if (j > i) { vidbuffer[x] = j; }
    else { vidbuffer[x] = ((i + j) >> 1) + 1; }
  }
}

