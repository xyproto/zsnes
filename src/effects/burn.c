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

/*

Hi guys, try this, use it in your code, but please credit

Frank Jan Sorensen Alias:Frank Patxi (fjs@lab.jt.dk) for the
fireroutine.

*/

/*

Hi again, guys!

If you use this code, please also credit me, Joachim Fenkes, 'cause I added
the following speedups:

  -Replaced one tiny loop with a faster Move(...) (not much speedup)
  -Wrote the main display loop in 100% assembler, including a faster random
   number generator (the RNG is only a more or less optimized version of
   Borland's generator (see MEGARAND.ASM), but with the advantage of the
   ultimate crash if you call it normally :-)
  -Changed version number into 1.10 (this isn't a speedup, but necessary :-)

*/

/*
 Bcoz of the knowledge that reading from videocards is much slower than
 writing to them, I changed some things to write and read from/to a pointer
 and put the result with 32-Bit moves to the screen

 Also I added now a much more faster randommer.

 The result of this change is more than 3 times fast than before
  Stefan Goehler
 Please credit me!
 ...
 to JF: your bug is fixed!
*/

/*
 Oops, silly me, I removed all of the assembly code, so I can let the compiler
 have at it. Also makes it more portable... even though I am doing this to add
 to a project that is very non-portable.
*/

#define BUF_WIDTH 288
#define BUF_HEIGHT 224

const int rootrand = 20;	/* Max/Min decrease of the root of the flames */
const int decay = 5;		/* How far should the flames go up on the screen? */
				/* This MUST be positive - JF */
const int miny = 0;		/* Startingline of the flame routine.
				(should be adjusted along with MinY above) */
const int smooth = 1;		/* How descrete can the flames be?*/
const int minfire = 50;		/* limit between the "starting to burn" and
					the "is burning" routines */
const int xstart = 0;		/* Startingpos on the screen, should be divideable by 4 without remain!*/
const int xend = 287;		/* Guess! */
const int width = 1 + 287;   /* +xend-xstart; Well- */
const int maxcolor = 110;	/* Constant for the MakePal procedure */
const int fireincrease = 3;	/*3 = Wood, 90 = Gazolin*/

typedef struct colorvalue
{
	unsigned char r, g, b;
} colorvalue;
typedef colorvalue vgapalettetype[256];

void hsi2rgb(double h, double s, double i, struct colorvalue *c)
/*Convert (Hue, Saturation, Intensity) -> (RGB)*/
{
  double t;
  double rv, gv, bv;

  t = h;
  rv = 1 + s * sin(t - 2 * M_PI / 3);
  gv = 1 + s * sin(t);
  bv = 1 + s * sin(t + 2 * M_PI / 3);
  t = 255.999 * i / 2;
  {
    c->r = (unsigned char) floor(rv * t);
    c->g = (unsigned char) floor(gv * t);
    c->b = (unsigned char) floor(bv * t);
  }
}

void genpal()
{
  int i;
  vgapalettetype pal;

  memset(pal, 0, 3);
  for( i=1; i <= maxcolor; i ++)
    hsi2rgb(4.6-1.5*i/maxcolor,(double)(i)/maxcolor,(double)(i)/maxcolor,&pal[i]);
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

#if 0 // emulating Turbo Pascal

unsigned int randseed;
const unsigned modulus = 2147483647;
const unsigned factor = 397204094;

void Randomize()
{
    randseed = time(NULL);
}

unsigned int randint(unsigned range)
{
    randseed = randseed * factor % modulus;
    return range ? randseed % range : 0;
}

double randreal()
{
    randseed = randseed * factor % modulus;
    return (double)randseed / modulus;
}

int rand1(int r)         /* Return a random number between -R And R*/
{
  int result;
  result=randint(r*2+1)-r;
  return result;
}
#else
#define Randomize()
#define randint(a) (rand() % (a))
#define randreal() (((double)rand()) / ((double)RAND_MAX))
#define rand1(a) ((randint(a*2+1))-a)
#endif

unsigned char flamearray[BUF_WIDTH];
int morefire;
extern unsigned char *vidbuffer;

/* damn, this seems like such a waste */
static unsigned char pt[BUF_WIDTH * BUF_HEIGHT];

#if 0

int burn_init()
{
  int i;

  if (flamearray) return 1;

  flamearray = (unsigned char *) malloc(BUF_WIDTH);

  for( i=xstart; i <= xend; i++)
    flamearray[i]=0;

  Randomize();

  morefire=1;
  genpal();

  return 0;
}

void burn_shutdown()
{
  if (flamearray)
  {
    free(flamearray);
    flamearray = NULL;
  }
}

#endif

void DrawBurn()
{
int i,j;
int x,p;
int v;

	if (!started)
	{
		started = 1;

		for( i=xstart; i <= xend; i++)
			flamearray[i]=0;

		Randomize();

		morefire=1;

		memset(pt, 0, BUF_HEIGHT * BUF_WIDTH);
		/* genpal(); */
    }

/*
	for (x=0; x < BUF_WIDTH*BUF_HEIGHT; x++)
	{
		i = pt[x];
		j = vidbuffer[x] << 2;

		if (i > j) pt[x] = i;
		else pt[x] = (i + j) >> 1;
	}
*/

    /* Put the values from FlameArray on the bottom line of the screen */
    memcpy(pt+((BUF_HEIGHT-1)*BUF_WIDTH)+xstart,flamearray, width);

    /* This loop makes the actual flames */

    for( i=xstart; i <= xend; i++)
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

    /*Match?*/
    if (randint(150)==0)
      memset(flamearray + xstart + randint(xend-xstart-5),255,5);

    /*This loop controls the "root" of the
     flames ie. the values in FlameArray.*/
    for( i=xstart; i <= xend; i++)
    {
      x=flamearray[i];

      if (x<minfire)    /* Increase by the "burnability"*/
      {
        /*Starting to burn:*/
        if (x>10)  x += randint(fireincrease);
      }
      else
      /* Otherwise randomize and increase by intensity (is burning)*/
        x += rand1(rootrand)+morefire;
      if (x>255)  x=255;    /* X Too large ?*/
      flamearray[i]=x;
    }


    /* Pour a little water on both sides of
      the fire to make it look nice on the sides*/
/*
    for( i=1; i <= width / 8; i ++)
    {
      x=floor(sqrt(randreal())*width/8);
      flamearray[xstart+x]=0;
      flamearray[xend-x]=0;
    }
*/

    /*Smoothen the values of FrameArray to avoid "descrete" flames*/
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

		if (j > i) vidbuffer[x] = j;
		else vidbuffer[x] = ((i + j) >> 1) + 1;
	}
}

