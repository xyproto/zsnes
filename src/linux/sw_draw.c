#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL.h"

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
	    __asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl SurfBufD, %%edi
		xorl %%ebx, %%ebx
        Blank2:
		movl SurfaceX, %%ecx
		rep
		stosw
		addl pitch, %%edi
		subl SurfaceX, %%edi
		subl SurfaceX, %%edi
		addl $1, %%ebx
		cmpl SurfaceY, %%ebx
		jne Blank2
		popw %%es
	" : : : "cc", "memory", "eax", "ebx", "ecx", "edi");
	    break;
	case 32:
	    __asm__ __volatile__ ("
                pushw %%es
                movw %%ds, %%ax
                movw %%ax, %%es
                xorl %%eax, %%eax
                movl SurfBufD, %%edi
                xorl %%ebx, %%ebx
        Blank3:
                movl SurfaceX, %%ecx
                rep
                stosl
                addl pitch, %%edi
                subl SurfaceX, %%edi
                subl SurfaceX, %%edi
                subl SurfaceX, %%edi
                subl SurfaceX, %%edi
                addl $1, %%ebx
                cmpl SurfaceY, %%ebx
                jne Blank3
                popw %%es       
        " : : : "cc", "memory", "eax", "ebx", "ecx","edi");
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
		    __asm__ __volatile__ ("
		    pushw %%es
		    movw %%ds, %%ax
		    movw %%ax, %%es
		    xorl %%eax, %%eax
		    movl ScreenPtr, %%esi
		    movl SurfBufD, %%edi
	    Copying3:
		    movl $32, %%ecx
	    CopyLoop:
		    movq (%%esi), %%mm0
		    movq 8(%%esi), %%mm1
		    movq %%mm0, (%%edi)
		    movq %%mm1, 8(%%edi)
		    addl $16, %%esi
		    addl $16, %%edi
		    decl %%ecx
		    jnz CopyLoop
		    incl %%eax
		    addl pitch, %%edi
		    subl $512, %%edi
		    addl $64, %%esi
		    cmpl $223, %%eax
		    jne Copying3

		    xorl %%eax, %%eax
		    movl $128, %%ecx
		    rep
		    stosl
		    popw %%es
		    emms
	    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
		} else {
		    // Doesn't seem to work - DDOI
		    __asm__ __volatile__ ("
		    pushw %%es
		    movw %%ds, %%ax
		    movw %%ax, %%es
		    xorl %%eax, %%eax
		    movl ScreenPtr, %%esi
		    movl SurfBufD, %%edi
	    Copying:
		    movl $128, %%ecx
		    rep
		    movsl
		    incl %%eax
		    addl pitch, %%edi
		    subl $512, %%edi
		    addl $64, %%esi
		    cmpl $223, %%eax
		    jne Copying
		    xorl %%eax, %%eax
		    movl $128, %%ecx
		    rep
		    stosl
		    popw %%es
	    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
		}
		break;

	    case 32:
		__asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl BitConv32Ptr, %%ebx
		movl ScreenPtr, %%esi
		movl SurfBufD, %%edi
	Copying32b:
		movl $256, %%ecx
		pushl %%eax
		xorl %%eax, %%eax
	CopyLoop32b:
		movw (%%esi), %%ax
		addl $2, %%esi
		movl (%%ebx, %%eax, 4), %%edx
		movl %%edx, (%%edi)
		addl $4, %%edi
		loop CopyLoop32b
		popl %%eax
		incl %%eax
		addl pitch, %%edi
		subl $1024, %%edi
		addl $64, %%esi
		cmpl $223, %%eax
		jne Copying32b
		popw %%es
	" : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
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
		    __asm__ __volatile__ ("
		    pushw %%es
		    movw %%ds, %%ax
		    movw %%ax, %%es
		    xor %%eax, %%eax
		    xor %%ebx, %%ebx
		    movl ScreenPtr, %%esi
		    movl SurfBufD, %%edi
	    Blank1MMX:
		    mov $160, %%ecx
		    rep
		    stosl
		    subl $160, %%edi
		    addl pitch, %%edi
		    addl $1, %%ebx
		    cmpl $8, %%ebx
		    jne Blank1MMX
		    xor %%ebx, %%ebx
		    pxor %%mm0, %%mm0
	    Copying2MMX:
		    mov $4, %%ecx
	    MMXLoopA:
		    movq %%mm0, 0(%%edi)
		    movq %%mm0, 8(%%edi)
		    addl $16, %%edi
		    dec %%ecx
		    jnz MMXLoopA
		    mov $32, %%ecx
	    MMXLoopB:
		    movq 0(%%esi), %%mm1
		    movq 8(%%esi), %%mm2
		    movq %%mm1, 0(%%edi)
		    movq %%mm2, 8(%%edi)
		    addl $16, %%esi
		    addl $16, %%edi
		    decl %%ecx
		    jnz MMXLoopB
		    mov $4, %%ecx
	    MMXLoopC:
		    movq %%mm0, 0(%%edi)
		    movq %%mm0, 8(%%edi)
		    addl $16, %%edi
		    decl %%ecx
		    jnz MMXLoopC
		    incl %%ebx
		    addl pitch, %%edi
		    subl $640, %%edi
		    addl $64, %%esi
		    cmpl $223, %%ebx
		    jne Copying2MMX
		    
		    movl $128, %%ecx
		    rep
		    stosl
		    pop %%es
		    emms
	    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

		} else {
		    __asm__ __volatile__ ("
		    push %%es
		    movw %%ds, %%ax
		    movw %%ax, %%es
		    xorl %%eax, %%eax
		    xorl %%ebx, %%ebx
		    movl ScreenPtr, %%esi
		    movl SurfBufD, %%edi
	    Blank1:
		    movl $160, %%ecx
		    rep
		    stosl
		    subl $640, %%edi
		    addl pitch, %%edi
		    addl $1, %%ebx
		    cmpl $8, %%ebx 
		    jne Blank1
		    xor %%ebx, %%ebx
	    Copying2:
		    movl $16, %%ecx
		    rep
		    stosl
		    movl $128, %%ecx
		    rep
		    movsl
		    movl $16, %%ecx
		    rep
		    stosl
		    incl %%ebx
		    addl pitch, %%edi
		    subl $640, %%edi
		    addl $64, %%esi
		    cmpl $223, %%ebx
		    jne Copying2
		    
		    movl $128, %%ecx
		    rep
		    stosl
		    pop %%es
	    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

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
		__asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl BitConv32Ptr, %%ebx
		movl ScreenPtr, %%esi
		movl SurfBufD, %%edi
	Copying32c:
		movl $256, %%ecx
		pushl %%eax
		xorl %%eax, %%eax
	CopyLoop32c:
		movw (%%esi), %%ax
		addl $2, %%esi
		movl (%%ebx, %%eax, 4), %%edx
		movl %%edx, (%%edi)
		movl %%edx, 4(%%edi)
		addl $8, %%edi
		loop CopyLoop32c
		pushl %%esi
		movl %%edi, %%esi
		subl pitch, %%esi
		movl $512, %%ecx
		rep
		movsl
		popl %%esi
		popl %%eax
		incl %%eax
		addl $64, %%esi
		cmpl $223, %%eax
		jne Copying32c
		popw %%es
	" : : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
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
		__asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl BitConv32Ptr, %%ebx
		movl ScreenPtr, %%esi
		movl SurfBufD, %%edi
		addl $20608, %%edi
	Copying32d:
		movl $256, %%ecx
		pushl %%eax
		xorl %%eax, %%eax
	CopyLoop32d:
		movw (%%esi), %%ax
		addl $2, %%esi
		movl (%%ebx, %%eax, 4), %%edx
		movl %%edx, (%%edi)
		movl %%edx, 4(%%edi)
		addl $8, %%edi
		loop CopyLoop32d
		addl $512, %%edi
		pushl %%esi
		movl %%edi, %%esi
		subl pitch, %%esi
		movl $512, %%ecx
		rep
		movsl
		popl %%esi
		popl %%eax
		incl %%eax
		addl $512, %%edi
		addl $64, %%esi
		cmpl $223, %%eax
		jne Copying32d
		popw %%es
	" : : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
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
