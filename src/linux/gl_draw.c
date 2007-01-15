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


#include "../cfg.h"
#include "../gblhdr.h"
#include "../asm_call.h"

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

// FUNCTIONS
void hq2x_16b(void);

// VIDEO VARIABLES
extern SDL_Surface *surface;
extern int SurfaceX, SurfaceY;
extern int SurfaceLocking;
extern DWORD BitDepth;

// OPENGL VARIABLES
static unsigned short *glvidbuffer = 0;
static GLuint gltextures[4];
static int gltexture256, gltexture512;
static int glfilters = GL_NEAREST;
static int glscanready = 0;
extern Uint8 GUIOn2;

extern unsigned int vidbuffer;
extern unsigned char curblank;
extern BYTE GUIRESIZE[];

void gl_clearwin();
void UpdateVFrame();

void gl_scanlines();

bool OGLModeCheck();

int gl_start(int width, int height, int req_depth, int FullScreen)
{
  Uint32 flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_OPENGL;
  int i;

  flags |= (GUIRESIZE[cvidmode] ? SDL_RESIZABLE : 0);
  flags |= (FullScreen ? SDL_FULLSCREEN : 0);


  SurfaceX = width; SurfaceY = height;
  surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
  if (surface == NULL)
  {
    fprintf(stderr, "Could not set %dx%d-GL video mode.\n",SurfaceX, SurfaceY);
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if (SDL_MAJOR_VERSION > 1) || ((SDL_MINOR_VERSION > 2) || ((SDL_MINOR_VERSION == 2) && (SDL_PATCHLEVEL >= 10)))
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif

  if (!glvidbuffer)
  {
    glvidbuffer = (unsigned short *) malloc(512 * 512 * sizeof(short));
  }
  gl_clearwin();
  SDL_WarpMouse(SurfaceX / 4, SurfaceY / 4);

  // Grab mouse in fullscreen mode
  FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) :
               SDL_WM_GrabInput(SDL_GRAB_OFF);

  SDL_WM_SetCaption("ZSNES", "ZSNES");
  SDL_ShowCursor(0);

  /* Setup some GL stuff */

  glEnable(GL_TEXTURE_1D);
  glEnable(GL_TEXTURE_2D);

  glViewport(0, 0, SurfaceX, SurfaceY);

  /*
   * gltextures[0]: 2D texture, 256x224
   * gltextures[1]: 2D texture, 512x224
   * gltextures[3]: 1D texture, 256 lines of alternating alpha
   */
  glGenTextures(4, gltextures);
  for (i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, gltextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilters);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilters);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  if (scanlines) gl_scanlines();

  return true;
}

void gl_end()
{
  if (glvidbuffer)
  {
    glDeleteTextures(4, gltextures);
    free(glvidbuffer);
    glvidbuffer = 0;
  }
}

extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char *WinVidMemStart;
extern unsigned char NGNoTransp;
void copy640x480x16bwin(void);
extern unsigned char SpecialLine[224];  /* 0 if lo-res, > 0 if hi-res */

void gl_clearwin()
{
  glClear(GL_COLOR_BUFFER_BIT);
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

  switch (hires)
  {
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

  if (hires)
  {
    if (hires != gltexture512)
    {
      unsigned short *vbuf1 = &((unsigned short *) vidbuffer)[16];
      unsigned short *vbuf2 = &((unsigned short *) vidbuffer)[75036 * 2 + 16];
      unsigned short *vbuf = &glvidbuffer[0];

      if (hires>1) // mode 7
      {
        for (j = 224; j--;)
        {
          for (i = 256; i--;)
            *vbuf++ = *vbuf1++;
          for (i = 256; i--;)
            *vbuf++ = *vbuf2++;
          vbuf1 += 32;
          vbuf2 += 32;
        }
        glBindTexture(GL_TEXTURE_2D, gltextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 512, 0,
               GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
               glvidbuffer);

        gltexture512 = 2;
      }
      else
      {
        for (j = 224; j--;)
        {
          for (i = 256; i--;)
          {
            *vbuf++ = *vbuf1++;
            *vbuf++ = *vbuf2++;
          }
          vbuf1 += 32;
          vbuf2 += 32;  // skip the two 16-pixel-wide columns
        }

        glBindTexture(GL_TEXTURE_2D, gltextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 256, 0,
               GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
               glvidbuffer);

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
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, gltextures[0]);
    if (!gltexture256)
    {
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 16);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 288);

      glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
             GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
             ((unsigned short *) vidbuffer) + 288);

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

void gl_drawwin()
{
  int i;

  NGNoTransp = 0;    // Set this value to 1 within the appropriate
        // video mode if you want to add a custom
        // transparency routine or hardware
        // transparency.  This only works if
        // the value of newengen is equal to 1.
        // (see ProcessTransparencies in newgfx16.asm
        //  for ZSNES' current transparency code)
  UpdateVFrame();
  if (curblank || !OGLModeCheck())
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

  if (SurfaceX >= 512 && (hqFilter || En2xSaI))
  {
    AddEndBytes = 0;
    NumBytesPerLine = 1024;
    WinVidMemStart = (void *) glvidbuffer;

    if (hqFilter) hq2x_16b();
    else asm_call(copy640x480x16bwin);

    /* Display 1 512x448 quad for the 512x448 buffer */
    glBindTexture(GL_TEXTURE_2D, gltextures[1]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0,
                 GL_RGB, GL_UNSIGNED_SHORT_5_6_5, glvidbuffer);

    glDisable (GL_DEPTH_TEST);
    glDisable (GL_LIGHTING);
    glDisable (GL_BLEND);

    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, 1.0f, -1.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(1.0f, 1.0f, -1.0f);
      glTexCoord2f(1.0f, 448.0f / 512.0f);
      glVertex3f(1.0f, -1.0f, -1.0f);
      glTexCoord2f(0.0f, 448.0f / 512.0f);
      glVertex3f(-1.0f, -1.0f, -1.0f);
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
    for (i = 0; i < 224; i++)
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

    if (i - lasthires_line > 1)
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

      if (scanlines != glscanready) gl_scanlines();

      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      glBindTexture(GL_TEXTURE_1D, gltextures[3]);
      glBegin(GL_QUADS);
      for (i = 0; i < SurfaceY; i += 256)
      {
        glTexCoord1f(0.0f);
        glVertex3f(-1.0f, (SurfaceY - i * 2.0) / SurfaceY, -1.0f);
        glTexCoord1f(0.0f);
        glVertex3f(1.0f, (SurfaceY - i * 2.0) / SurfaceY, -1.0f);
        glTexCoord1f(1.0f);
        glVertex3f(1.0f, (SurfaceY - (i + 256) * 2.0) / SurfaceY, -1.0f);
        glTexCoord1f(1.0f);
        glVertex3f(-1.0f, (SurfaceY - (i + 256) * 2.0) / SurfaceY, -1.0f);
      }
      glEnd();

      glDisable(GL_BLEND);
      glEnable(GL_TEXTURE_2D);
    }
  }
  SDL_GL_SwapBuffers();
}

void gl_scanlines(void)
{
  GLubyte scanbuffer[256][4];
  int i, j = scanlines==1 ? 0 : (scanlines==2 ? 192 : 128);

  for (i = 0; i < 256; i += 2)
  {
    scanbuffer[i][0] = scanbuffer[i][1] = scanbuffer[i][2] = j;
    scanbuffer[i][3] = 0xFF;

    scanbuffer[i+1][0] = scanbuffer[i+1][1] = scanbuffer[i+1][2] = 0xFF;
    scanbuffer[i+1][3] = 0xFF;
  }

  glBindTexture(GL_TEXTURE_1D, gltextures[3]);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA,
         GL_UNSIGNED_BYTE, scanbuffer);

  glscanready = scanlines;
}
