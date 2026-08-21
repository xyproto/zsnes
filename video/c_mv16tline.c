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
#include "c_procwin.h"
/* The clearback seams (video/c_mv16tclr.c, video/c_mv16bclr.c). Their
   trampolines used to spill the registers into these; the call sites below do
   it now, and the seams go when calldl16t does. */
extern u4 CBAX, CBBX, CBCX, CBDX, CBSI, CBDI, CBBP;
extern u4 CLBAX, CLBBX, CLBCX, CLBDX, CLBSI, CLBDI;
void c_clearback16t(void);
void c_clearback16bts(void);
#include "c_m716gate.h"
#include "c_mv16draw.h"

/* The renderers a mode 7 gate can pick, indexed by the tail id it returns.
   Still assembly, so they are reached the same way as before. */
extern void drawmode716t(void), drawmode716b(void), drawmode716tb(void);
extern void drawmode716extbg(void), drawmode716textbg(void);
extern void drawmode716extbg2(void), drawmode716textbg2(void);

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

static void (*const m7_renderer[])(void) = { 0, drawmode716t, drawmode716b,
    drawmode716tb, drawmode716extbg, drawmode716textbg, drawmode716extbg2,
    drawmode716textbg2 };

/* A gate used to tail-jump into its renderer, so the renderer returned to
   whoever called the gate. Calling it here is the same thing: the registers
   go in through DLR either way, and the gate leaves the caller's own ecx, esi
   and edi alone. */
/* Same idea for the sprite gates, whose tail picks between two renderers.
   Both are C now (video/c_m716gate.c), so they are called straight. */

/* The six renderers a background gate can call, indexed by its id. Still
   assembly, so still reached through calldl16t. */
/* Ids 2 and 4 are C (video/c_m716gate.c); the rest are still assembly and go
   through calldl16t, so the two kinds are dispatched apart. */
void domosaic16b(void); /* video/mode716b.c */
/* Only the 16x16 mosaic drawer is still assembly. */
extern void draw16x1616tms(void);

/* DLR is the register set calldl16t passes to what assembly is left; r is what
   the ported drawers use. These two are the only places the two meet. */
static void dlr_get(m7regs* const r)
{
    r->ax = DLR[0];
    r->bx = DLR[1];
    r->cx = DLR[2];
    r->dx = DLR[3];
    r->si = DLR[4];
    r->di = DLR[5];
    r->bp = DLR[6];
}

static void dlr_put(m7regs const* const r)
{
    DLR[0] = r->ax;
    DLR[1] = r->bx;
    DLR[2] = r->cx;
    DLR[3] = r->dx;
    DLR[4] = r->si;
    DLR[5] = r->di;
    DLR[6] = r->bp;
}

static void bggate(u4 (*const g)(m7regs*), u4 const layer)
{
    m7regs r;
    u4 tail, mosaic = 0;

    DLR[6] = layer;
    dlr_get(&r);

    tail = g(&r);
    if (tail == 0) {
        dlr_put(&r);
        return;
    }

    /* The renderer the gate picked. All but the 16x16 mosaic drawer are C, so
       they work on r directly; that one still goes through calldl16t. Ids past
       the table reach the bookkeeping with no renderer at all. */
    switch (tail) {
    case 1:
        mosaic = draw8x816t(&r);
        break;
    case 2:
        mosaic = draw16x1616t(&r);
        break;
    case 3:
        mosaic = draw8x816bt(&r);
        break;
    case 4:
        mosaic = draw16x1616bt(&r);
        break;
    case 5:
        mosaic = draw8x816tms(&r);
        break;
    case 6:
        dlr_put(&r);
        call_asm(draw16x1616tms);
        dlr_get(&r);
        break;
    default:
        break;
    }

    /* The mosaic tail was a jump, so it returns to the gate's caller. */
    if (mosaic != 0) {
        domosaic16b();
    }
    drawbackgrnd_mark(&r);
    dlr_put(&r);
}

static void sprgate(u4 (*const g)(m7regs*), u4 const layer)
{
    m7regs r;
    u4 tail;

    DLR[6] = layer;
    r.ax = DLR[0];
    r.bx = DLR[1];
    r.cx = DLR[2];
    r.dx = DLR[3];
    r.si = DLR[4];
    r.di = DLR[5];
    r.bp = DLR[6];

    tail = g(&r);

    DLR[0] = r.ax;
    DLR[1] = r.bx;
    DLR[2] = r.cx;
    DLR[3] = r.dx;
    DLR[4] = r.si;
    DLR[5] = r.di;
    DLR[6] = r.bp;
    if (tail == 1) {
        drawsprites16t(&r);
    } else if (tail != 0) {
        drawsprites16bt(&r);
    }
    if (tail != 0) {
        DLR[0] = r.ax;
        DLR[1] = r.bx;
        DLR[2] = r.cx;
        DLR[3] = r.dx;
        DLR[4] = r.si;
        DLR[5] = r.di;
        DLR[6] = r.bp;
    }
}

static void m7gate(u4 (*const g)(m7regs*))
{
    m7regs r = { DLR[0], DLR[1], DLR[2], DLR[3], DLR[4], DLR[5], DLR[6] };
    u4 const tail = g(&r);

    DLR[0] = r.ax;
    DLR[1] = r.bx;
    DLR[2] = r.cx;
    DLR[3] = r.dx;
    DLR[4] = r.si;
    DLR[5] = r.di;
    DLR[6] = r.bp;
    if (tail != 0) {
        call_asm(m7_renderer[tail]);
    }
}

static void clearback_t(void)
{
    CBAX = DLR[0];
    CBBX = DLR[1];
    CBCX = DLR[2];
    CBDX = DLR[3];
    CBSI = DLR[4];
    CBDI = DLR[5];
    CBBP = DLR[6];
    c_clearback16t();
    DLR[0] = CBAX;
    DLR[1] = CBBX;
    DLR[2] = CBCX;
    DLR[3] = CBDX;
    DLR[4] = CBSI;
    DLR[5] = CBDI;
    DLR[6] = CBBP;
}

/* The sub-screen one never took ebp. */
static void clearback_bts(void)
{
    CLBAX = DLR[0];
    CLBBX = DLR[1];
    CLBCX = DLR[2];
    CLBDX = DLR[3];
    CLBSI = DLR[4];
    CLBDI = DLR[5];
    c_clearback16bts();
    DLR[0] = CLBAX;
    DLR[1] = CLBBX;
    DLR[2] = CLBCX;
    DLR[3] = CLBDX;
    DLR[4] = CLBSI;
    DLR[5] = CLBDI;
}

/* clearback16bts runs straight after this and reads the registers it leaves,
   which is why the four come back through DLR rather than being dropped. */
static void procwindowback(void)
{
    pwregs pw = { DLR[0], DLR[1], DLR[2], DLR[4] };

    c_procwindowback16t(&pw);
    DLR[0] = pw.ax;
    DLR[1] = pw.bx;
    DLR[2] = pw.cx;
    DLR[4] = pw.si;
}

/* `mov ebp,N` before the call is the only register any of these reads. */

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
    clearback_t();
    curbgpr = 0x00;
    curbgnum = 0x08;
    bggate(drawbackgrndmain16t, 3);
    curbgnum = 0x04;
    bggate(drawbackgrndmain16t, 2);
    sprgate(procspritesmain16t, 0);
    curbgnum = 0x08;
    bggate(drawbackgrndmain16t, 3);
    curbgpr = 0x20;
    if (bg3high2 != 1) {
        curbgnum = 0x04;
        bggate(drawbackgrndmain16t, 2);
    }
    sprgate(procspritesmain16t, 1);
    curbgpr = 0x00;
    curbgnum = 0x02;
    bggate(drawbackgrndmain16t, 1);
    curbgnum = 0x01;
    bggate(drawbackgrndmain16t, 0);
    sprgate(procspritesmain16t, 2);
    curbgpr = 0x20;
    curbgnum = 0x02;
    bggate(drawbackgrndmain16t, 1);
    curbgnum = 0x01;
    bggate(drawbackgrndmain16t, 0);
    sprgate(procspritesmain16t, 3);
    if (bg3high2 == 1) {
        curbgpr = 0x20;
        curbgnum = 0x04;
        bggate(drawbackgrndmain16t, 2);
    }
}

/* Priority2NextDrawLine16bt: the main-screen pass for modes 2 and up, which
   have no background 3 or 4. */
static void priority2_next_draw_line(void)
{
    main_line();
    clearback_t();
    curbgpr = 0x00;
    curbgnum = 0x02;
    bggate(drawbackgrndmain16t, 1);
    sprgate(procspritesmain16t, 0);
    curbgnum = 0x01;
    bggate(drawbackgrndmain16t, 0);
    sprgate(procspritesmain16t, 1);
    curbgpr = 0x20;
    curbgnum = 0x02;
    bggate(drawbackgrndmain16t, 1);
    sprgate(procspritesmain16t, 2);
    curbgnum = 0x01;
    bggate(drawbackgrndmain16t, 0);
    sprgate(procspritesmain16t, 3);
}

/* priority216t: the sub-screen pass for modes 2 and up. */
static void priority2(void)
{
    if (scaddset & 0x02u) {
        curbgpr = 0x00;
        curbgnum = 0x02;
        bggate(drawbackgrndsub16t, 1);
        sprgate(procspritessub16t, 0);
        curbgnum = 0x01;
        bggate(drawbackgrndsub16t, 0);
        sprgate(procspritessub16t, 1);
        curbgpr = 0x20;
        curbgnum = 0x02;
        bggate(drawbackgrndsub16t, 1);
        sprgate(procspritessub16t, 2);
        curbgnum = 0x01;
        bggate(drawbackgrndsub16t, 0);
        sprgate(procspritessub16t, 3);
    }
    cwinenabm = winenabm;
    priority2_next_draw_line();
}

/* processmode716t and processmode716t2. */
static void process_mode7(void)
{
    curvidoffset = transpbuf + 32;
    setpalette16b();
    procwindowback();
    clearback_bts();
    makewindowsp();
    DLR[0] = 0;
    DLR[2] = 0;
    sprite_setup();
    extbgdone = 0;
    if (scaddset & 0x02u) {
        if (interlval & 0x40u) {
            m7gate(procmode716tsubextbg);
        }
        sprgate(procspritessub16t, 0);
        if (!(interlval & 0x40u)) {
            m7gate(procmode716tsub);
        }
        sprgate(procspritessub16t, 1);
        if (interlval & 0x40u) {
            m7gate(procmode716tsubextbgb);
            m7gate(procmode716tsubextbg2);
        }
        sprgate(procspritessub16t, 2);
        sprgate(procspritessub16t, 3);
    }
    cwinenabm = winenabm;

    /* processmode716t2 */
    main_line();
    sprite_setup();
    clearback_t();
    DLR[0] = 0;
    DLR[2] = 0;
    extbgdone = 0;
    if (interlval & 0x40u) {
        m7gate(procmode716tmainextbg);
    }
    sprgate(procspritesmain16t, 0);
    if (!(interlval & 0x40u)) {
        m7gate(procmode716tmain);
    }
    sprgate(procspritesmain16t, 1);
    if (interlval & 0x40u) {
        m7gate(procmode716tmainextbgb);
        m7gate(procmode716tmainextbg2);
    }
    sprgate(procspritesmain16t, 2);
    sprgate(procspritesmain16t, 3);
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
    procwindowback();
    clearback_bts();
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
        bggate(drawbackgrndsub16t, 3);
        curbgnum = 0x04;
        bggate(drawbackgrndsub16t, 2);
        sprgate(procspritessub16t, 0);
        curbgnum = 0x08;
        bggate(drawbackgrndsub16t, 3);
        curbgpr = 0x20;
        if (bg3high2 != 1) {
            curbgnum = 0x04;
            bggate(drawbackgrndsub16t, 2);
        }
        sprgate(procspritessub16t, 1);
        curbgpr = 0x00;
        curbgnum = 0x02;
        bggate(drawbackgrndsub16t, 1);
        curbgnum = 0x01;
        bggate(drawbackgrndsub16t, 0);
        sprgate(procspritessub16t, 2);
        curbgpr = 0x20;
        curbgnum = 0x02;
        bggate(drawbackgrndsub16t, 1);
        curbgnum = 0x01;
        bggate(drawbackgrndsub16t, 0);
        sprgate(procspritessub16t, 3);
        if (bg3high2 == 1) {
            curbgnum = 0x04;
            bggate(drawbackgrndsub16t, 2);
        }
    }
    cwinenabm = winenabm;
    next_draw_line();
}
