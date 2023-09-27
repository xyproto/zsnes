/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#include "../ver.h"
#include "../cfg.h"
#include "../gblhdr.h"
#include "../link.h"
#include <stdint.h>

#ifdef _WIN32
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif

// VIDEO VARIABLES
extern SDL_Surface *surface;
extern uint64_t BitDepth;

// OPENGL VARIABLES
static unsigned short *glvidbuffer = 0;
static GLuint gltextures[4];
static int gltexture256, gltexture512;
extern Uint8 GUIOn2;

extern unsigned short *vidbuffer;
extern unsigned char curblank;

void SetGLAttributes() {
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if SDL_VERSION_ATLEAST(1, 2, 10)
	int const value = vsyncon ? 1 : 0;
#if SDL_VERSION_ATLEAST(1, 3, 0)
	SDL_GL_SetSwapInterval(value);
#else
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, value);
#endif
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif
}

int gl_start(int width, int height, int req_depth, int FullScreen) {
	uint32_t flags = SDL_OPENGL | (FullScreen ? SDL_FULLSCREEN : SDL_RESIZABLE);
	int i;

	SetGLAttributes();
	surface = SDL_SetVideoMode(width, height, req_depth, flags);
	if (surface == NULL) {
		fprintf(stderr, "Could not set %dx%d-GL video mode.\n", width, height);
		return false;
	}

	if (!glvidbuffer) {
		glvidbuffer = (unsigned short *)malloc(512 * 512 * sizeof(short));
	}

	// Grab mouse in fullscreen mode
	FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

	SDL_WM_SetCaption("ZSNES " ZVER "-experimental", "ZSNES " ZVER "-experimental");
	SDL_ShowCursor(0);

	/* Setup some GL stuff */

	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, width, height);

	/*
	 * gltextures[0]: 2D texture, 256x224
	 * gltextures[1]: 2D texture, 512x224
	 */
	glGenTextures(2, gltextures);
	for (i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, gltextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	return true;
}

void gl_end() {
	if (glvidbuffer) {
		glDeleteTextures(2, gltextures);
		free(glvidbuffer);
		glvidbuffer = 0;
	}
}

extern unsigned char NGNoTransp;
extern unsigned char SpecialLine[224]; /* 0 if lo-res, > 0 if hi-res */

/* gl_drawspan:
 * Puts a quad on the screen for hires/lores portions, starting at line start,
 * and ending at line end..
 * Builds the 256x256/512x256 textures if gltexture256 or gltexture512 == 0
 */
static void gl_drawspan(int hires, int start, int end) {
	int i, j;

	switch (hires) {
	case 0:
		break;
	case 3:
	case 7:
		hires = 2;
		break;
	default:
		hires = 1;
		break;
	}

	if (hires) {
		if (hires != gltexture512) {
			unsigned short *vbuf1 = vidbuffer + 16;
			unsigned short *vbuf2 = vidbuffer + (75036 * 2 + 16);
			unsigned short *vbuf = glvidbuffer;

			if (hires > 1) // mode 7
			{
				for (j = 224; j--;) {
					for (i = 256; i--;) {
						*vbuf++ = *vbuf1++;
					}
					for (i = 256; i--;) {
						*vbuf++ = *vbuf2++;
					}
					vbuf1 += 32;
					vbuf2 += 32;
				}
				glBindTexture(GL_TEXTURE_2D, gltextures[1]);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 512, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, glvidbuffer);

				gltexture512 = 2;
			} else {
				for (j = 224; j--;) {
					for (i = 256; i--;) {
						*vbuf++ = *vbuf1++;
						*vbuf++ = *vbuf2++;
					}
					vbuf1 += 32;
					vbuf2 += 32; // skip the two 16-pixel-wide columns
				}

				glBindTexture(GL_TEXTURE_2D, gltextures[1]);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, glvidbuffer);

				gltexture512 = 1;
			}
		}

		glBindTexture(GL_TEXTURE_2D, gltextures[1]);
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
	} else {
		glBindTexture(GL_TEXTURE_2D, gltextures[0]);
		if (!gltexture256) {
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 16);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 288);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, vidbuffer + 288);

			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

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

void gl_drawwin() {
	int i;
	NGNoTransp = 0; // Set this value to 1 within the appropriate
	// video mode if you want to add a custom
	// transparency routine or hardware
	// transparency.  This only works if
	// the value of newengen is equal to 1.
	// (see ProcessTransparencies in newgfx16.asm
	//  for ZSNES' current transparency code)
	UpdateVFrame();
	if (curblank) {
		return;
	}

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	 * This code splits the hires/lores portions up, and draws
	 * them with gl_drawspan
	 */
	int lasthires, lasthires_line = 0;

	gltexture256 = gltexture512 = 0;

	lasthires = SpecialLine[1];
	for (i = 0; i < 224; i++) {
		if (SpecialLine[i + 1]) {
			if (lasthires) {
				continue;
			}
			gl_drawspan(lasthires, lasthires_line, i);

			lasthires = SpecialLine[i + 1];
			lasthires_line = i;
		} else {
			if (!lasthires) {
				continue;
			}
			gl_drawspan(lasthires, lasthires_line, i);

			lasthires = SpecialLine[i + 1];
			lasthires_line = i;
		}
	}

	if (i - lasthires_line > 1) {
		gl_drawspan(lasthires, lasthires_line, i);
	}
	SDL_GL_SwapBuffers();
}