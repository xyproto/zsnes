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


#ifndef GL_DRAW_h
#define GL_DRAW_h 1

int gl_start(int width, int height, int req_depth, int FullScreen);
void gl_end();
void gl_clearwin();
void gl_drawwin();

// FUNCTIONS
extern void hq2x_16b(void);

// VIDEO VARIABLES
extern uint8_t cvidmode;
extern int32_t SurfaceX, SurfaceY;
extern int32_t SurfaceLocking;
extern uint32_t BitDepth;

extern HWND hMainWindow;
extern HDC hDC;
extern HGLRC hRC;

// OPENGL VARIABLES
static unsigned short *glvidbuffer = 0;
static GLuint gltextures[4];
static int32_t gltexture256, gltexture512;
static int32_t glfilters = GL_NEAREST;
static int32_t glscanready = 0;
extern uint8_t En2xSaI;
extern uint8_t sl_intensity;
extern uint8_t FilteredGUI;
extern uint8_t GUIOn2;
extern uint32_t vidbuffer;
extern uint8_t curblank;
extern uint8_t GUIRESIZE[];

void gl_clearwin();
void UpdateVFrame(void);
void gl_scanlines(void);


#endif


