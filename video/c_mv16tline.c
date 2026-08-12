/*
 * video/c_mv16tline.c - the scanline drivers of video/makev16t.asm:
 * drawline16t, NextDrawLine16bt, priority216t, Priority2NextDrawLine16bt,
 * processmode716t and processmode716t2.
 *
 * The top of the transparency renderer. One scanline is: set the palette,
 * work out the colour window, clear the back area into the transparency
 * buffer, then walk the four backgrounds and the four sprite priorities twice
 * - once into the transparency buffer (the "sub" pass) and once into the video
 * line (the "main" pass). Mode 7 has its own pair of drivers with the same
 * shape.
 *
 * Only drawline16t is reached from outside; the other five are fallen into or
 * jumped to. The assembly's push/pop of the callee-saved registers spans the
 * pair - drawline16t pushes and NextDrawLine16bt pops - which is why they read
 * as one function here.
 *
 * Everything this calls is already C behind a register seam, so the calls go
 * back out through those thunks. calldl16t (video/makev16t.asm) is the inverse
 * of a seam: it loads the register block, calls, and stores it back, so the
 * registers thread between the calls exactly as they did.
 */
#include <stdint.h>

#include "../types.h"
#include "makevid.h"

/* The register block calldl16t moves in and out, in eax..ebp order. */
u4 DLR[7];
void (*DLFN)(void);

extern void calldl16t(void);

/* The ported clusters, still reached through their seam thunks. */
extern void procwindowback16t(void);
extern void clearback16bts(void);
extern void clearback16t(void);
extern void drawbackgrndsub16t(void);
extern void drawbackgrndmain16t(void);
extern void procspritessub16t(void);
extern void procspritesmain16t(void);
extern void procmode716tsub(void);
extern void procmode716tsubextbg(void);
extern void procmode716tsubextbgb(void);
extern void procmode716tsubextbg2(void);
extern void procmode716tmain(void);
extern void procmode716tmainextbg(void);
extern void procmode716tmainextbgb(void);
extern void procmode716tmainextbg2(void);

/* cdecl already, so they need no seam of their own; the assembly reached them
   through ccallv, which preserves every register. */
extern void setpalette16b(void);
extern void makewindowsp(void);
extern void preparesprpr(void);
extern void procbackgrnd(u4 layer);

extern u1 bgmode, scaddset, winenabm, cwinenabm, sprprifix, bg3high2;
extern u1 interlval, extbgdone;
extern u2 scrnon, curypos;
extern u1 transpbuf[];
extern u1 sprleftpr[], sprlefttot[];
extern u1* spritetablea;
extern u1* vidbuffer;
extern u1* cursprloc;
extern SpriteInfo* currentobjptr;

static void call_asm(void (*fn)(void))
{
    DLFN = fn;
    calldl16t();
}

/* `mov ebp,N` before the call is the only register any of these reads. */
static void call_layer(void (*fn)(void), u4 const layer)
{
    DLR[6] = layer;
    call_asm(fn);
}

/* The sprite table for this scanline, and the priority list the sprite passes
   walk. Shared by the two sub-screen drivers. */
static void sprite_setup(void)
{
    DLR[1] = ((u1)curypos << 9) + (u4)(uintptr_t)spritetablea;
    currentobjptr = (SpriteInfo*)(uintptr_t)DLR[1];
    cursprloc = sprleftpr;
    if (sprprifix != 0) {
        cursprloc = sprlefttot;
        preparesprpr();
    }
}

/* Both main-screen drivers start the video line the same way. */
static void main_line(void)
{
    u4 const y = curypos;

    DLR[1] = y << 6;
    DLR[4] = (y << 9) + DLR[1] + 32u + (u4)(uintptr_t)vidbuffer;
    curvidoffset = (u1*)(uintptr_t)DLR[4];
}

/* NextDrawLine16bt: the main-screen pass for modes 0 and 1. */
static void next_draw_line(void)
{
    main_line();
    call_asm(clearback16t);
    curbgpr = 0x00;
    curbgnum = 0x08;
    call_layer(drawbackgrndmain16t, 3);
    curbgnum = 0x04;
    call_layer(drawbackgrndmain16t, 2);
    call_layer(procspritesmain16t, 0);
    curbgnum = 0x08;
    call_layer(drawbackgrndmain16t, 3);
    curbgpr = 0x20;
    if (bg3high2 != 1) {
        curbgnum = 0x04;
        call_layer(drawbackgrndmain16t, 2);
    }
    call_layer(procspritesmain16t, 1);
    curbgpr = 0x00;
    curbgnum = 0x02;
    call_layer(drawbackgrndmain16t, 1);
    curbgnum = 0x01;
    call_layer(drawbackgrndmain16t, 0);
    call_layer(procspritesmain16t, 2);
    curbgpr = 0x20;
    curbgnum = 0x02;
    call_layer(drawbackgrndmain16t, 1);
    curbgnum = 0x01;
    call_layer(drawbackgrndmain16t, 0);
    call_layer(procspritesmain16t, 3);
    if (bg3high2 == 1) {
        curbgpr = 0x20;
        curbgnum = 0x04;
        call_layer(drawbackgrndmain16t, 2);
    }
}

/* Priority2NextDrawLine16bt: the main-screen pass for modes 2 and up, which
   have no background 3 or 4. */
static void priority2_next_draw_line(void)
{
    main_line();
    call_asm(clearback16t);
    curbgpr = 0x00;
    curbgnum = 0x02;
    call_layer(drawbackgrndmain16t, 1);
    call_layer(procspritesmain16t, 0);
    curbgnum = 0x01;
    call_layer(drawbackgrndmain16t, 0);
    call_layer(procspritesmain16t, 1);
    curbgpr = 0x20;
    curbgnum = 0x02;
    call_layer(drawbackgrndmain16t, 1);
    call_layer(procspritesmain16t, 2);
    curbgnum = 0x01;
    call_layer(drawbackgrndmain16t, 0);
    call_layer(procspritesmain16t, 3);
}

/* priority216t: the sub-screen pass for modes 2 and up. */
static void priority2(void)
{
    if (scaddset & 0x02u) {
        curbgpr = 0x00;
        curbgnum = 0x02;
        call_layer(drawbackgrndsub16t, 1);
        call_layer(procspritessub16t, 0);
        curbgnum = 0x01;
        call_layer(drawbackgrndsub16t, 0);
        call_layer(procspritessub16t, 1);
        curbgpr = 0x20;
        curbgnum = 0x02;
        call_layer(drawbackgrndsub16t, 1);
        call_layer(procspritessub16t, 2);
        curbgnum = 0x01;
        call_layer(drawbackgrndsub16t, 0);
        call_layer(procspritessub16t, 3);
    }
    cwinenabm = winenabm;
    priority2_next_draw_line();
}

/* processmode716t and processmode716t2. */
static void process_mode7(void)
{
    curvidoffset = transpbuf + 32;
    setpalette16b();
    call_asm(procwindowback16t);
    call_asm(clearback16bts);
    makewindowsp();
    DLR[0] = 0;
    DLR[2] = 0;
    sprite_setup();
    extbgdone = 0;
    if (scaddset & 0x02u) {
        if (interlval & 0x40u) {
            call_asm(procmode716tsubextbg);
        }
        call_layer(procspritessub16t, 0);
        if (!(interlval & 0x40u)) {
            call_asm(procmode716tsub);
        }
        call_layer(procspritessub16t, 1);
        if (interlval & 0x40u) {
            call_asm(procmode716tsubextbgb);
            call_asm(procmode716tsubextbg2);
        }
        call_layer(procspritessub16t, 2);
        call_layer(procspritessub16t, 3);
    }
    cwinenabm = winenabm;

    /* processmode716t2 */
    main_line();
    sprite_setup();
    call_asm(clearback16t);
    DLR[0] = 0;
    DLR[2] = 0;
    extbgdone = 0;
    if (interlval & 0x40u) {
        call_asm(procmode716tmainextbg);
    }
    call_layer(procspritesmain16t, 0);
    if (!(interlval & 0x40u)) {
        call_asm(procmode716tmain);
    }
    call_layer(procspritesmain16t, 1);
    if (interlval & 0x40u) {
        call_asm(procmode716tmainextbgb);
        call_asm(procmode716tmainextbg2);
    }
    call_layer(procspritesmain16t, 2);
    call_layer(procspritesmain16t, 3);
}

void drawline16t(void)
{
    if (bgmode == 7) {
        process_mode7();
        return;
    }
    /* A layer on both screens at once, with the back area off on one of them,
       is dropped from the main screen for this line. */
    if (((scrnon >> 8) & scrnon & 0xFFu) != 0 && !(scrnon & 0x10u)
        && ((scrnon >> 8) & 0x10u) != 0) {
        scrnon = (scrnon & 0xFF00u) | (scrnon & ~(scrnon >> 8) & 0xFFu);
    }
    curvidoffset = transpbuf + 32;
    setpalette16b();
    call_asm(procwindowback16t);
    call_asm(clearback16bts);
    makewindowsp();
    DLR[0] = 0;
    DLR[2] = 0;
    sprite_setup();

    curbgnum = 0x02;
    procbackgrnd(0x01);
    curbgnum = 0x01;
    procbackgrnd(0x00);
    curbgnum = 0x08;
    procbackgrnd(0x03);
    curbgnum = 0x04;
    procbackgrnd(0x02);

    if (bgmode > 1) {
        priority2();
        return;
    }
    if (scaddset & 0x02u) {
        curbgpr = 0x00;
        curbgnum = 0x08;
        call_layer(drawbackgrndsub16t, 3);
        curbgnum = 0x04;
        call_layer(drawbackgrndsub16t, 2);
        call_layer(procspritessub16t, 0);
        curbgnum = 0x08;
        call_layer(drawbackgrndsub16t, 3);
        curbgpr = 0x20;
        if (bg3high2 != 1) {
            curbgnum = 0x04;
            call_layer(drawbackgrndsub16t, 2);
        }
        call_layer(procspritessub16t, 1);
        curbgpr = 0x00;
        curbgnum = 0x02;
        call_layer(drawbackgrndsub16t, 1);
        curbgnum = 0x01;
        call_layer(drawbackgrndsub16t, 0);
        call_layer(procspritessub16t, 2);
        curbgpr = 0x20;
        curbgnum = 0x02;
        call_layer(drawbackgrndsub16t, 1);
        curbgnum = 0x01;
        call_layer(drawbackgrndsub16t, 0);
        call_layer(procspritessub16t, 3);
        if (bg3high2 == 1) {
            curbgnum = 0x04;
            call_layer(drawbackgrndsub16t, 2);
        }
    }
    cwinenabm = winenabm;
    next_draw_line();
}
