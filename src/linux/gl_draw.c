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

#include "gblhdr.h"

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

typedef enum
{ FALSE = 0, TRUE = !FALSE }
BOOL;

// VIDEO VARIABLES
extern unsigned char cvidmode;
extern SDL_Surface *surface;
extern int SurfaceX, SurfaceY;
extern int SurfaceLocking;
extern DWORD BitDepth;

// OPENGL VARIABLES
static unsigned short *glvidbuffer = 0;
static GLuint gltextures[4];
static int gltexture256, gltexture512;
static int glfilters = GL_NEAREST;
extern Uint8 En2xSaI, scanlines;
extern Uint8 BilinearFilter;
extern Uint8 FilteredGUI;
extern Uint8 GUIOn2;

extern unsigned int vidbuffer;
extern unsigned char curblank;

void gl_clearwin();
void UpdateVFrame(void);

int gl_start(int width, int height, int req_depth, int FullScreen)
{
	Uint32 flags =
		SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_OPENGL;
	GLubyte scanbuffer[256];
	int i;

	flags |= (cvidmode == 16 ? SDL_RESIZABLE : 0);
	flags |= (FullScreen ? SDL_FULLSCREEN : 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SurfaceX = width; SurfaceY = height;
	surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
	if (surface == NULL)
	{
		fprintf(stderr, "Could not set %dx%d-GL video mode.\n",
			SurfaceX, SurfaceY);
		return FALSE;
	}

	glvidbuffer = (unsigned short *) malloc(512 * 512 * sizeof(short));
	gl_clearwin();
	SDL_WarpMouse(SurfaceX / 4, SurfaceY / 4);

	// Grab mouse in fullscreen mode
	FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) :
		SDL_WM_GrabInput(SDL_GRAB_OFF);

	SDL_WM_SetCaption("ZSNES-GL Linux", "ZSNES");
	SDL_ShowCursor(0);

	/* Setup some GL stuff */

	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, SurfaceX, SurfaceY);

	/*
	 * gltextures[0]: 2D texture, 256x224
	 * gltextures[1]: 2D texture, Left half of 512x224
	 * gltextures[2]: 2D texture, Right half of 512x224
	 * gltextures[3]: 1D texture, 256 lines of alternating alpha
	 */
	glGenTextures(4, gltextures);

	/* Initialize the scanline texture (alternating opaque/transparent) */
	for (i = 0; i < 256; i += 2)
	{
		scanbuffer[i] = 0xff;
		scanbuffer[i + 1] = 0;
	}

	glBindTexture(GL_TEXTURE_1D, gltextures[3]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 256);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE_ALPHA, 256, 0, GL_ALPHA,
		     GL_UNSIGNED_BYTE, scanbuffer);

	return TRUE;
}

void gl_end()
{
	glDeleteTextures(4, gltextures);
	free(glvidbuffer);
}

extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char *WinVidMemStart;
extern unsigned char FPUCopy;
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern void copy640x480x16bwin(void);
extern unsigned char SpecialLine[224];	/* 0 if lo-res, > 0 if hi-res */

void gl_clearwin()
{
	glClear(GL_COLOR_BUFFER_BIT);
	if (En2xSaI)
		memset(glvidbuffer, 0, 512 * 448 * 2);
}

/* gl_drawspan:
 * Puts a quad on the screen for hires/lores portions, starting at line start,
 * and ending at line end..
 * Builds the 256x256/512x256 textures if gltexture256 or gltexture512 == 0
 */
static void gl_drawspan(int hires, int start, int end)
{
	int i, j;

	if (hires)
	{
		if (!gltexture512)
		{
			unsigned short *vbuf1 = &((unsigned short *) vidbuffer)[16];
			unsigned short *vbuf2 = &((unsigned short *) vidbuffer)[75036 * 2 + 16];
			unsigned short *vbuf = &glvidbuffer[0];

			for (j = 0; j < 224; j++)
			{
				for (i = 0; i < 256; i++)
				{
					*vbuf++ = *vbuf1++;
					*vbuf++ = *vbuf2++;
				}
				vbuf1 += 32;
				vbuf2 += 32;	// skip the two 16-pixel-wide columns
			}

			glBindTexture(GL_TEXTURE_2D, gltextures[1]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
				     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				     glvidbuffer + 512);

			glBindTexture(GL_TEXTURE_2D, gltextures[2]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 256);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
				     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				     glvidbuffer + 512);

			gltexture512 = 1;
		}

		glBindTexture(GL_TEXTURE_2D, gltextures[1]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(-1.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(0.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(0.0f, (112 - end) / 112.0);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(-1.0f, (112 - end) / 112.0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, gltextures[2]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(0.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(1.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(1.0f, (112 - end) / 112.0);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(0.0f, (112 - end) / 112.0);
		glEnd();
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, gltextures[0]);
		if (!gltexture256)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilters);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 16);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 288);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
				     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				     ((unsigned short *) vidbuffer) + 288);

			gltexture256 = 1;
		}

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(-1.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (start / 224.0));
			glVertex2f(1.0f, (112 - start) / 112.0);
			glTexCoord2f(1.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(1.0f, (112 - end) / 112.0);
			glTexCoord2f(0.0f, (224.0 / 256.0) * (end / 224.0));
			glVertex2f(-1.0f, (112 - end) / 112.0);
		glEnd();
	}
}

void gl_drawwin()
{
	int i;

	NGNoTransp = 0;		// Set this value to 1 within the appropriate
				// video mode if you want to add a custom
				// transparency routine or hardware
				// transparency.  This only works if
				// the value of newengen is equal to 1.
				// (see ProcessTransparencies in newgfx16.asm
				//  for ZSNES' current transparency code)
	UpdateVFrame();
	if (curblank != 0)
		return;

	if (BilinearFilter)
	{
		glfilters = GL_LINEAR;
		if (GUIOn2 && !FilteredGUI)
			glfilters = GL_NEAREST;
	}
	else
	{
		glfilters = GL_NEAREST;
	}

	if (En2xSaI)
	{
		/* We have to use copy640x480x16bwin for 2xSaI */
		AddEndBytes = 0;
		NumBytesPerLine = 1024;
		WinVidMemStart = (void *) glvidbuffer;
		__asm__ __volatile__("call copy640x480x16bwin"
			::: "memory", "eax", "ebx", "ecx", "edx", "esi", "edi");

		/* Display 4 256x256 quads for the 512x448 buffer */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilters);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilters);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		/* Upper left quad */
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			     GL_RGB, GL_UNSIGNED_SHORT_5_6_5, glvidbuffer);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(0.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, (224.0 / 256.0));
			glVertex3f(0.0f, 0.0f, -1.0f);
			glTexCoord2f(0.0f, (224.0 / 256.0));
			glVertex3f(-1.0f, 0.0f, -1.0f);
		glEnd();

		/* Upper right quad */
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
			     glvidbuffer + 256);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(0.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, (224.0 / 256.0));
			glVertex3f(1.0f, 0.0f, -1.0f);
			glTexCoord2f(0.0f, (224.0 / 256.0));
			glVertex3f(0.0f, 0.0f, -1.0f);
		glEnd();

		/* Lower left quad */
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
			     glvidbuffer + 512 * 224);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-1.0f, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -1.0f);
			glTexCoord2f(1.0f, (224.0 / 256.0));
			glVertex3f(0.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, (224.0 / 256.0));
			glVertex3f(-1.0f, -1.0f, -1.0f);
		glEnd();

		/* Lower right quad */
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
			     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
			     glvidbuffer + 512 * 224 + 256);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(1.0f, 0.0f, -1.0f);
			glTexCoord2f(1.0f, (224.0 / 256.0));
			glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, (224.0 / 256.0));
			glVertex3f(0.0f, -1.0f, -1.0f);
		glEnd();
	}
	else
	{
		/*
		 * This code splits the hires/lores portions up, and draws
		 * them with gl_drawspan
		 */
		int lasthires, lasthires_line = 0;

		gltexture256 = gltexture512 = 0;

		lasthires = SpecialLine[1];
		for (i = 1; i < 222; i++)
		{
			if (SpecialLine[i + 1])
			{
				if (lasthires)
					continue;
				gl_drawspan(lasthires, lasthires_line, i);

				lasthires = SpecialLine[i + 1];
				lasthires_line = i;
			}
			else
			{
				if (!lasthires)
					continue;
				gl_drawspan(lasthires, lasthires_line, i);

				lasthires = SpecialLine[i + 1];
				lasthires_line = i;
			}
		}
		gl_drawspan(lasthires, lasthires_line, i);

		/*
		 * This is here rather than right outside this if because the
		 * GUI doesn't allow scanlines to be selected while filters are
		 * on.. There is no technical reason they can't be on while
		 * filters are on, however.  Feel free to change the GUI, and
		 * move this outside the if (En2xSaI) {}, if you do.
		 */
		if (scanlines)
		{
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindTexture(GL_TEXTURE_1D, gltextures[3]);
			glColor4f(1.0f, 1.0f, 1.0f,
				  scanlines == 1 ? 1.0f : (scanlines ==
							   2 ? 0.25f : 0.50f));
			for (i = 0; i < SurfaceY; i += 256)
			{
				glBegin(GL_QUADS);
				glTexCoord1f(0.0f);
				glVertex3f(-1.0f, (SurfaceY - i * 2.0) / SurfaceY, -1.0f);
				glTexCoord1f(0.0f);
				glVertex3f(1.0f, (SurfaceY - i * 2.0) / SurfaceY, -1.0f);
				glTexCoord1f(1.0f);
				glVertex3f(1.0f, (SurfaceY - (i + 256) * 2.0) / SurfaceY, -1.0f);
				glTexCoord1f(1.0f);
				glVertex3f(-1.0f, (SurfaceY - (i + 256) * 2.0) / SurfaceY, -1.0f);
				glEnd();
			}

			glDisable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);
		}
	}
	SDL_GL_SwapBuffers();
}
