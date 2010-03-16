#ifndef REGS_H
#define REGS_H

#include "../types.h"

typedef enum Layer
{
	LAYER_BG1 = 0,
	LAYER_BG2 = 1,
	LAYER_BG3 = 2,
	LAYER_BG4 = 3,
	LAYER_OBJ = 4,
	LAYER_COL = 5
} Layer;

extern void reg2100r();
extern void reg2134r();
extern void reg2135r();
extern void reg2136r();
extern void reg2137r();
extern void reg2138r();
extern void reg2139r();
extern void reg213Ar();
extern void reg213Br();
extern void reg213Cr();
extern void reg213Dr();
extern void reg213Er();
extern void reg213Fr();
extern void reg2140r();
extern void reg2141r();
extern void reg2142r();
extern void reg2143r();
extern void reg2180r();
extern void reg21C2r();
extern void reg21C3r();
extern void reg4016r();
extern void reg4017r();
extern void reg4100r();
extern void reg420Ar();
extern void reg420Br();
extern void reg420Cr();
extern void reg420Dr();
extern void reg420Er();
extern void reg420Fr();
extern void reg4210r();
extern void reg4211r();
extern void reg4212r();
extern void reg4213r();
extern void reg4214r();
extern void reg4215r();
extern void reg4216r();
extern void reg4217r();
extern void reg4218r();
extern void reg4219r();
extern void reg421Ar();
extern void reg421Br();
extern void reg421Cr();
extern void reg421Dr();
extern void reg421Er();
extern void reg421Fr();
extern void reg43XXr();
extern void regINVALID();

extern u1 INTEnab;          // enables NMI(7)/VIRQ(5)/HIRQ(4)/JOY(0)
extern u1 MultiTap;
extern u1 Voice0Disable[8]; // Disable Voice
extern u1 bg3highst;        // is 1 if background 3 has the highest priority
extern u1 bgmode;           // graphics mode (0 .. 7)
extern u1 bgtilesz;         // 0 = 8x8, 1 = 16x16, bit 0=bg1,bit1=bg2,etc
extern u1 cfield;
extern u1 cgmod;            // if cgram is modified or not
extern u1 coladdb;          // blue value of color to add
extern u1 coladdg;          // green value of color to add
extern u1 coladdr;          // red value of color to add
extern u1 colnull;          // keep this 0 (when accessing colors by dword)
extern u1 dmadata[129];     // dma data (written from ports 43xx)
extern u1 doirqnext;
extern u1 extlatch;
extern u1 forceblnk;        // force blanking on/off ($80=on)
extern u1 frskipper;        // used to control frame skipping
extern u1 irqon;            // if IRQ has been called (80h) or not (0)
extern u1 mosaicon;         // mosaic on, bit 0=bg1,bit1=bg2, etc
extern u1 mosaicsz;         // mosaic size in pixels
extern u1 scaddset;         // screen/fixed color addition settings
extern u1 scaddtype;        // which screen to add/sub
extern u1 scrndis;          // which background is disabled
extern u1 sndrot;           // rotates to use A,X or Y for sound skip
extern u1 vidbright;        // screen brightness (0 .. 15)
extern u1 winen[6];         // Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG1/BG2/BG3/BG4/sprites/backarea
extern u1 winenabm;         // Window logic enable for main screen
extern u1 winenabs;         // Window logic enable for sub screen
extern u1 winlogica;        // Window logic type for BG1 to 4
extern u2 bg1objptr[4];     // pointer to tiles in background1/2/3/4
extern u2 bg1ptr[4];        // pointer to background1/2/3/4
extern u2 bg1scrolx[4];     // background 1/2/3/4 x position
extern u2 bg1scroly[4];     // background 1/2/3/4 y position
extern u2 cgram[256];       // CGRAM
extern u2 latchx;           // latched x value
extern u2 latchy;           // latched y value
extern u2 resolutn;         // screen resolution
extern u2 scrnon;           // main & sub screen on
extern u4 bg1ptrx[4];       // pointer to background1/2/3/4
extern u4 bg1ptry[4];       // pointer to background1/2/3/4
extern u4 winl1;            // window 1/2 left/right position

#endif
