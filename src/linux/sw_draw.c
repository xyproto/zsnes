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

typedef enum { FALSE = 0, TRUE = !FALSE } BOOL;

// VIDEO VARIABLES
extern unsigned char cvidmode;
extern SDL_Surface *surface;
extern int SurfaceX, SurfaceY;
extern int SurfaceLocking;

extern void LinuxExit();

extern unsigned int vidbuffer;
extern DWORD converta;
extern unsigned int BitConv32Ptr;
extern unsigned char curblank;
void UpdateVFrame(void);

BOOL sw_start(int width, int height, int req_depth, int FullScreen)
{
    unsigned int color32, p;
    int i;
    Uint32 flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ANYFORMAT

;
    DWORD GBitMask;
    
    p = BitConv32Ptr;
    for(i=0; i<65536; i++) {
        color32 = ((i&0xF800)<<8) + ((i&0x07E0)<<5) + ((i&0x001F)<<3)+0xFF000000;
	    (*(unsigned int *)(p)) = color32;
	    p += 4;
    }
    
    flags |= (FullScreen ? SDL_FULLSCREEN : 0);
    
    SurfaceX = width; SurfaceY = height;
    surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
    if (surface == NULL) {
	fprintf (stderr, "Could not set %dx%d video mode.\n", SurfaceX, SurfaceY);
	return FALSE;
      
    }
    
    SurfaceLocking = SDL_MUSTLOCK(surface);
    SDL_WarpMouse(SurfaceX/4,SurfaceY/4);

    // Grab mouse in fullscreen mode
    FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_WM_SetCaption ("ZSNES Linux","ZSNES");
    SDL_ShowCursor(0);

    // Check hardware for 565/555
    GBitMask = surface->format->Gmask;
    if(GBitMask != 0x07E0) {
        converta = 1;
    } else {
        converta = 0;
    }

    return TRUE;
}

void sw_end() {
    // Do nothing
}

static void LockSurface(void)
{
    if (SurfaceLocking) SDL_LockSurface(surface);
}

static void UnlockSurface(void)
{
    if (SurfaceLocking) SDL_UnlockSurface(surface);
    SDL_Flip(surface);
}

extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char *WinVidMemStart;
extern unsigned char MMXSupport;
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern unsigned short resolutn;
extern void copy640x480x16bwin(void);

/* FIXME: Figure out how to make these locals */
static DWORD ScreenPtr;
static DWORD SurfBufD;
static DWORD pitch;

void sw_clearwin()
{
    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;

    LockSurface();
    __asm__ __volatile__ (
	"	xorl %%eax, %%eax\n"				\
	"	xorl %%ebx, %%ebx\n"				\
        "Blank2:\n"						\
	"	movl %1, %%ecx\n"				\
	"	rep\n"						\
	"	stosw\n"					\
	"	movl %1, %%edx\n"				\
	"	addl %0, %%edi\n"				\
	"	shll $1, %%edx\n"				\
	"	addl $1, %%ebx\n"				\
	"	subl %%edx, %%edi\n"				\
	"	cmpl %2, %%ebx\n"				\
	"	jne Blank2\n"					\
    : : "g" (pitch), "g" (SurfaceX), "g" (SurfaceY), "D" (SurfBufD)
    : "cc", "memory", "eax", "ebx", "edx", "ecx");
    UnlockSurface();
}

void sw_drawwin()
{
    NGNoTransp = 0;             // Set this value to 1 within the appropriate
                                // video mode if you want to add a custom
                                // transparency routine or hardware
                                // transparency.  This only works if
                                // the value of newengen is equal to 1.
                                // (see ProcessTransparencies in newgfx16.asm
                                //  for ZSNES' current transparency code)
    UpdateVFrame();
    if (curblank != 0) return;

    LockSurface();

    ScreenPtr = vidbuffer;
    ScreenPtr += 16*2+32*2+256*2;

    if (resolutn == 239) ScreenPtr+=8*288*2;

    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;

    if (SurfBufD == 0) {
	UnlockSurface();
	return;
    }

    if (SurfaceX == 256 && SurfaceY == 224) {
	if (MMXSupport){
	    __asm__ __volatile__ (
		"	xorl %%eax, %%eax\n"				\
		"Copying3:\n"						\
		"	movl $32, %%ecx\n"				\
		"CopyLoop:\n"						\
		"	movq (%%esi), %%mm0\n"				\
		"	movq 8(%%esi), %%mm1\n"				\
		"	movq %%mm0, (%%edi)\n"				\
		"	movq %%mm1, 8(%%edi)\n"				\
		"	addl $16, %%esi\n"				\
		"	addl $16, %%edi\n"				\
		"	decl %%ecx\n"					\
		"	jnz CopyLoop\n"					\
		"	incl %%eax\n"					\
		"	addl %0, %%edi\n"				\
		"	subl $512, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying3\n"					\
		
		"	xorl %%eax, %%eax\n"				\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	emms\n"						\
	    : : "g" (pitch), "S" (ScreenPtr), "D" (SurfBufD) : "cc", "memory", "eax", "ecx");
	} else {
	    __asm__ __volatile__ (
		"	xorl %%eax, %%eax\n"				\
		"Copying:\n"						\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	movsl\n"					\
		"	incl %%eax\n"					\
		"	addl %0, %%edi\n"				\
		"	subl $512, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying\n"					\
		"	xorl %%eax, %%eax\n"				\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
	    : : "g" (pitch), "S" (ScreenPtr), "D" (SurfBufD)
	    : "cc", "memory", "eax", "ecx");
	}
    } else if (SurfaceX == 320 && SurfaceY == 240) {
	if (MMXSupport) {
	    __asm__ __volatile__ (
		"	xor %%eax, %%eax\n"				\
		"	xor %%ebx, %%ebx\n"				\
		"Blank1MMX:\n"						\
		"	mov $160, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	subl $160, %%edi\n"				\
		"	addl %0, %%edi\n"				\
		"	addl $1, %%ebx\n"				\
		"	cmpl $8, %%ebx\n"				\
		"	jne Blank1MMX\n"				\
		"	xor %%ebx, %%ebx\n"				\
		"	pxor %%mm0, %%mm0\n"				\
		"Copying2MMX:\n"					\
		"	mov $4, %%ecx\n"				\
		"MMXLoopA:\n"						\
		"	movq %%mm0, 0(%%edi)\n"				\
		"	movq %%mm0, 8(%%edi)\n"				\
		"	addl $16, %%edi\n"				\
		"	dec %%ecx\n"					\
		"	jnz MMXLoopA\n"					\
		"	mov $32, %%ecx\n"				\
		"MMXLoopB:\n"						\
		"	movq 0(%%esi), %%mm1\n"				\
		"	movq 8(%%esi), %%mm2\n"				\
		"	movq %%mm1, 0(%%edi)\n"				\
		"	movq %%mm2, 8(%%edi)\n"				\
		"	addl $16, %%esi\n"				\
		"	addl $16, %%edi\n"				\
		"	decl %%ecx\n"					\
		"	jnz MMXLoopB\n"					\
		"	mov $4, %%ecx\n"				\
		"MMXLoopC:\n"						\
		"	movq %%mm0, 0(%%edi)\n"				\
		"	movq %%mm0, 8(%%edi)\n"				\
		"	addl $16, %%edi\n"				\
		"	decl %%ecx\n"					\
		"	jnz MMXLoopC\n"					\
		"	incl %%ebx\n"					\
		"	addl %0, %%edi\n"				\
		"	subl $640, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%ebx\n"				\
		"	jne Copying2MMX\n"				\

		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	emms\n"						\
	    : : "g" (pitch), "S" (ScreenPtr), "D" (SurfBufD)
	    : "cc", "memory", "eax", "ebx", "ecx");
	} else {
	    __asm__ __volatile__ (
		"	xorl %%eax, %%eax\n"				\
		"	xorl %%ebx, %%ebx\n"				\
		"Blank1:\n"						\
		"	movl $160, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	subl $640, %%edi\n"				\
		"	addl %0, %%edi\n"				\
		"	addl $1, %%ebx\n"				\
		"	cmpl $8, %%ebx\n"				\
		"jne Blank1\n"						\
		"	xor %%ebx, %%ebx\n"				\
		"Copying2:\n"						\
		"	movl $16, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	movsl\n"					\
		"	movl $16, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	incl %%ebx\n"					\
		"	addl %0, %%edi\n"				\
		"	subl $640, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%ebx\n"				\
		"	jne Copying2\n"					\

		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
	    : : "g" (pitch), "S" (ScreenPtr), "D" (SurfBufD)
	    : "cc", "memory", "eax", "ebx", "ecx");
	}
    } else if(SurfaceX == 512 && SurfaceY == 448) {
	AddEndBytes = pitch-1024;
	NumBytesPerLine = pitch;
	WinVidMemStart = (void*)SurfBufD;
	copy640x480x16bwin();
    } else if (SurfaceX == 640 && SurfaceY == 480) {
	AddEndBytes = pitch-1024;
	NumBytesPerLine = pitch;
	WinVidMemStart = (void*) (SurfBufD + 16*640*2 + 64*2);
	copy640x480x16bwin();
    }
    UnlockSurface();
}
