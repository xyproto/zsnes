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
extern DWORD BitDepth;

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
    Uint32 flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE;
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

    BitDepth = surface->format->BitsPerPixel;
    // Check hardware for 565/555
    GBitMask = surface->format->Gmask;

    if(BitDepth == 16 && GBitMask != 0x07E0) {
        converta = 1;
	//Init_2xSaI(555);
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
extern unsigned char FPUCopy;
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern void copy640x480x16bwin(void);

/* FIXME: Figure out how to make these locals */
static DWORD ScreenPtr;
static DWORD SurfBufD;
static DWORD *SURFDW;
static DWORD pitch;

void sw_clearwin()
{
    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;

    LockSurface();
    switch (BitDepth) {
	case 16:
	    __asm__ __volatile__ (
	"	pushw %%es\n"					\
	"	movw %%ds, %%ax\n"				\
	"	movw %%ax, %%es\n"				\
	"	xorl %%eax, %%eax\n"				\
	"	movl SurfBufD, %%edi\n"				\
	"	xorl %%ebx, %%ebx\n"				\
        "Blank2:\n"						\
	"	movl SurfaceX, %%ecx\n"				\
	"	rep\n"						\
	"	stosw\n"					\
	"	addl pitch, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	addl $1, %%ebx\n"				\
	"	cmpl SurfaceY, %%ebx\n"				\
	"	jne Blank2\n"					\
	"	popw %%es\n"					\
	: : : "cc", "memory", "eax", "ebx", "ecx", "edi");
	    break;
	case 32:
	    __asm__ __volatile__ (
	"	pushw %%es\n"					\
	"	movw %%ds, %%ax\n"				\
	"	movw %%ax, %%es\n"				\
	"	xorl %%eax, %%eax\n"				\
	"	movl SurfBufD, %%edi\n"				\
	"	xorl %%ebx, %%ebx\n"				\
	"Blank3:\n"						\
	"	movl SurfaceX, %%ecx\n"				\
	"	rep\n"						\
	"	stosl\n"					\
	"	addl pitch, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	subl SurfaceX, %%edi\n"				\
	"	addl $1, %%ebx\n"				\
	"	cmpl SurfaceY, %%ebx\n"				\
	"	jne Blank3\n"					\
	"	popw %%es\n"					\
	: : : "cc", "memory", "eax", "ebx", "ecx","edi");
	    break;
    }
    UnlockSurface();
}

void sw_drawwin()
{
    DWORD i,j,color32;

    NGNoTransp = 0;             // Set this value to 1 within the appropriate
                                // video mode if you want to add a custom
                                // transparency routine or hardware
                                // transparency.  This only works if
                                // the value of newengen is equal to 1.
                                // (see ProcessTransparencies in newgfx16.asm
                                //  for ZSNES' current transparency code)
	UpdateVFrame();
	if (curblank != 0)
		return;

    LockSurface();

    ScreenPtr = vidbuffer;
    ScreenPtr += 16*2+32*2+256*2;

    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;
    SURFDW = (DWORD *) surface->pixels;

    if (SurfBufD == 0) {
	UnlockSurface();
	return;
    }

    if (SurfaceX == 256 && SurfaceY == 224) {
	switch(BitDepth) {
	    case 16:
		if (FPUCopy){
		    __asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
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
		"	addl pitch, %%edi\n"				\
		"	subl $512, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying3\n"					\
		
		"	xorl %%eax, %%eax\n"				\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	popw %%es\n"					\
		"	emms\n"						\
		: : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
		} else {
		    // Doesn't seem to work - DDOI
		    __asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"Copying:\n"						\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	movsl\n"					\
		"	incl %%eax\n"					\
		"	addl pitch, %%edi\n"				\
		"	subl $512, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying\n"					\
		"	xorl %%eax, %%eax\n"				\
		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	popw %%es\n"					\
		: : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
		}
		break;

	    case 32:
		__asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	movl BitConv32Ptr, %%ebx\n"			\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"Copying32b:\n"						\
		"	movl $256, %%ecx\n"				\
		"	pushl %%eax\n"					\
		"	xorl %%eax, %%eax\n"				\
		"CopyLoop32b:\n"					\
		"	movw (%%esi), %%ax\n"				\
		"	addl $2, %%esi\n"				\
		"	movl (%%ebx, %%eax, 4), %%edx\n"		\
		"	movl %%edx, (%%edi)\n"				\
		"	addl $4, %%edi\n"				\
		"	loop CopyLoop32b\n"				\
		"	popl %%eax\n"					\
		"	incl %%eax\n"					\
		"	addl pitch, %%edi\n"				\
		"	subl $1024, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying32b\n"				\
		"	popw %%es\n"					\
		: : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
		SURFDW = (DWORD *) SurfBufD + 222*pitch;
		color32 = 0x7F000000;
	
		for(i=0; i<256; i++)
		    {
			SURFDW[i] = color32;
		    }
		
		SURFDW=(DWORD *) SurfBufD + 223*pitch;
		color32=0x7F000000;
		
		for(i=0; i<256; i++)
		    {
			SURFDW[i] = color32;
		    }
		break;

	    case 24:
		fprintf (stderr, "Sorry, this mode does not work in 24 bit color\n");
		LinuxExit();
		/*
		  cvidmode=3;
		  initwinvideo();
		  sleep(1);
		  drawscreenwin();
		*/
		break;
	    default:
		UnlockSurface();
		fprintf(stderr, "Mode only available in 16 and 32 bit color.\n");
		LinuxExit();
		/*
		  cvidmode=2;
		  initwinvideo();
		  sleep(1);
		  drawscreenwin();
		*/
		break;
	} // switch (BitDepth)
    } else if (SurfaceX == 320 && SurfaceY == 240) {
	switch(BitDepth) {
	    case 16:
		if (FPUCopy) {
		    __asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xor %%eax, %%eax\n"				\
		"	xor %%ebx, %%ebx\n"				\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"Blank1MMX:\n"						\
		"	mov $160, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	subl $160, %%edi\n"				\
		"	addl pitch, %%edi\n"				\
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
		"	addl pitch, %%edi\n"				\
		"	subl $640, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%ebx\n"				\
		"	jne Copying2MMX\n"				\

		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	pop %%es\n"					\
		"	emms\n"						\
		: : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

		} else {
		    __asm__ __volatile__ (
		"	push %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	xorl %%ebx, %%ebx\n"				\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"Blank1:\n"						\
		"	movl $160, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	subl $640, %%edi\n"				\
		"	addl pitch, %%edi\n"				\
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
		"	addl pitch, %%edi\n"				\
		"	subl $640, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%ebx\n"				\
		"	jne Copying2\n"					\

		"	movl $128, %%ecx\n"				\
		"	rep\n"						\
		"	stosl\n"					\
		"	pop %%es\n"					\
		: : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

		}
		break;

	    case 32:
		for(j=0; j<8; j++)
		    {
			SURFDW = (DWORD *) SurfBufD + j*pitch;
			color32 = 0x7F000000;
			
			for(i=0; i<320; i++)
			    {
				SURFDW[i] = color32;
			    }
		    }
		
		for(j=8; j<223+8; j++)
		    {
			color32 = 0x7F000000;
			for(i=0; i<32; i++)
			    {
				SURFDW[i]=color32;
			    }
			
			for(i=32; i<(256+32); i++)
			    {
				color32 = (((*(WORD *)(ScreenPtr))&0xF800)<<8)+
				    (((*(WORD *)(ScreenPtr))&0x07E0)<<5)+
				    (((*(WORD *)(ScreenPtr))&0x001F)<<3)+0x7F000000;
				SURFDW[i] = color32;
				ScreenPtr += 2;
			    }
			
			color32 = 0x7F000000;
			for(i=(256+32); i<320; i++)
			    {
				SURFDW[i] = color32;
			    }
			
			ScreenPtr = ScreenPtr+576-512;
			SURFDW=(DWORD *) SurfBufD + j*pitch;
		    }
		
		for(j=(223+8);j<240;j++)
		    {
			SURFDW=(DWORD *) SurfBufD + j*pitch;
			
			color32 = 0x7F000000;
			for(i=0; i<320; i++)
			    {
				SURFDW[i] = color32;
			    }
		    }
		break;
	    default:
		UnlockSurface();
		fprintf(stderr, "Mode only available in 16 and 32 bit color.\n");
		LinuxExit();
		/*
		  cvidmode=2;
		  initwinvideo();
		  sleep(1);
		  drawscreenwin();
		*/
		break;
	} // switch
    } else if(SurfaceX == 512 && SurfaceY == 448) {
	switch(BitDepth) {
	    case 16:
		AddEndBytes = pitch-1024;
		NumBytesPerLine = pitch;
		WinVidMemStart = (void*)SurfBufD;
		copy640x480x16bwin();
		break;

	    case 32:
		__asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	movl BitConv32Ptr, %%ebx\n"			\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"Copying32c:\n"						\
		"	movl $256, %%ecx\n"				\
		"	pushl %%eax\n"					\
		"	xorl %%eax, %%eax\n"				\
		"CopyLoop32c:\n"					\
		"	movw (%%esi), %%ax\n"				\
		"	addl $2, %%esi\n"				\
		"	movl (%%ebx, %%eax, 4), %%edx\n"		\
		"	movl %%edx, (%%edi)\n"				\
		"	movl %%edx, 4(%%edi)\n"				\
		"	addl $8, %%edi\n"				\
		"	loop CopyLoop32c\n"				\
		"	pushl %%esi\n"					\
		"	movl %%edi, %%esi\n"				\
		"	subl pitch, %%esi\n"				\
		"	movl $512, %%ecx\n"				\
		"	rep\n"						\
		"	movsl\n"					\
		"	popl %%esi\n"					\
		"	popl %%eax\n"					\
		"	incl %%eax\n"					\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying32c\n"				\
		"	popw %%es\n"					\
		: : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
		break;
		/*
		  addl pitch, %%edi
		  subl $2048, %%edi
		*/
	    default:
		UnlockSurface();
		fprintf(stderr, "Mode only available in 16 or 32 bit color.\n");
		LinuxExit();
		/*
		  cvidmode=2;
		  initwinvideo();
		  sleep(1);
		  drawscreenwin();
		*/
		break;
	} // switch
    } else if (SurfaceX == 640 && SurfaceY == 480) {
	switch(BitDepth)
	    {
	    case 16:
		AddEndBytes = pitch-1024;
		NumBytesPerLine = pitch;
		WinVidMemStart = (void*) (SurfBufD + 16*640*2 + 64*2);
		copy640x480x16bwin();
		break;

	    case 32:
		__asm__ __volatile__ (
		"	pushw %%es\n"					\
		"	movw %%ds, %%ax\n"				\
		"	movw %%ax, %%es\n"				\
		"	xorl %%eax, %%eax\n"				\
		"	movl BitConv32Ptr, %%ebx\n"			\
		"	movl ScreenPtr, %%esi\n"			\
		"	movl SurfBufD, %%edi\n"				\
		"	addl $20608, %%edi\n"				\
		"Copying32d:\n"						\
		"	movl $256, %%ecx\n"				\
		"	pushl %%eax\n"					\
		"	xorl %%eax, %%eax\n"				\
		"CopyLoop32d:\n"					\
		"	movw (%%esi), %%ax\n"				\
		"	addl $2, %%esi\n"				\
		"	movl (%%ebx, %%eax, 4), %%edx\n"		\
		"	movl %%edx, (%%edi)\n"				\
		"	movl %%edx, 4(%%edi)\n"				\
		"	addl $8, %%edi\n"				\
		"	loop CopyLoop32d\n"				\
		"	addl $512, %%edi\n"				\
		"	pushl %%esi\n"					\
		"	movl %%edi, %%esi\n"				\
		"	subl pitch, %%esi\n"				\
		"	movl $512, %%ecx\n"				\
		"	rep\n"						\
		"	movsl\n"					\
		"	popl %%esi\n"					\
		"	popl %%eax\n"					\
		"	incl %%eax\n"					\
		"	addl $512, %%edi\n"				\
		"	addl $64, %%esi\n"				\
		"	cmpl $223, %%eax\n"				\
		"	jne Copying32d\n"				\
		"	popw %%es\n"					\
		: : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
		break;
		/*
		  addl pitch, %%edi
		  subl $2048, %%edi
		*/

	    default:
		UnlockSurface();
		fprintf(stderr, "Mode only available in 16 or 32 bit color.\n");
		LinuxExit();
		/*
		  cvidmode=2;
		  initwinvideo();
		  sleep(1);
		  drawscreenwin();
		*/
		break;
	}
    }
    UnlockSurface();
}
