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

#include <GL/gl.h>

// Some code taken from DGen source

// GL Textures
static GLuint texture[2];

// Video Buffers (one for each texture 256x256 and 64x256 = 320x256)
static unsigned char oglbuffer1[256][256][4];
static unsigned char oglbuffer2[256][64][4];

static void maketex(int num, int size)
{
   glGenTextures(num,&texture[num-1]);
   glBindTexture(GL_TEXTURE_2D,texture[num-1]);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,size,256,0,GL_RGBA, GL_UNSIGNED_BYTE, oglbuffer);

}

void list()
{
  int i;

  dlist=glGenLists(1);
  glNewList(dlist,GL_COMPILE);

  glEnable(GL_TEXTURE_2D);

  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // 256x256
  glBindTexture(GL_TEXTURE_2D, texture[0]);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0); // upper left
    glTexCoord2f(0.0,0.0); glVertex2f(-1.0,1.0); // lower left
    glTexCoord2f(1.0,0.0); glVertex2f(tex_end,1.0); // lower right
    glTexCoord2f(1.0,1.0); glVertex2f(tex_end,-1.0); // upper right
  glEnd();

  // 64x256
  glBindTexture(GL_TEXTURE_2D, texture[1]);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex2f(tex_end, -1.0); // upper left
    glTexCoord2f(0.0, 0.0); glVertex2f(tex_end, 1.0); // lower left
    glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 1.0); // lower right
    glTexCoord2f(1.0, 1.0); glVertex2f(1.0, -1.0); // upper right
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glEndList();
}

void update() {
  int i,x,y;
  int c,x2;

  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 15, 256, 224, GL_RGBA, GL_UNSIGNED_BYTE, oglbuffer1);

  glBindTexture(GL_TEXTURE_2D, texture[1]);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 15, 64, 224, GL_RGBA, GL_UNSIGNED_BYTE, oglbuffer2);

  display();
}

