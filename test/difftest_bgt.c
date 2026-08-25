/* Differential test: the four background tile dispatchers in
 * video/newgfx16.asm (drawbg1tile16b .. drawbg4tile16b) against the C port in
 * video/c_ngbg.c.
 *
 * These end by tail-jumping into video/newg162.asm's renderers with the whole
 * register set live, so what has to match is the register state *at the jump*
 * plus everything the dispatcher wrote on the way - the same contract
 * difftest_m716t.c pins for the makev16t gates.
 *
 * The six renderers are recorded rather than run. The assembly reaches them by
 * jmp after a push, so its recorder pops before returning; the C reaches them
 * by a plain call and records from the register block. Both write the same
 * globals, so one comparison covers both.
 *
 * BuildWindow and Gendcolortable are recorded too: the assembly and the port
 * both call them cdecl, so a single C stand-in serves both sides.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int32_t s4;

enum { R_EDI, R_ESI, R_EBP, R_ESP, R_EBX, R_EDX, R_ECX, R_EAX };

/* --- the state the dispatchers read and write ---------------------------- */
extern u1 curmosaicsz, BGMA[], t16x161[], scadsng[], vidbright, prevbrightdc;
extern u1 winbg1enval[], winlogicaval[];
extern u2 BGPT1[], BGPT1X[], BGPT1Y[], BG1SXl[], BG1SYl[], BGOPT1[];
extern u2 bgtxad[];
extern u4 ng16bprval, ng16bbgval, bgtxadd, ngptrdat[], ngptrdat2;
extern u4 ngceax[], ngcedi[], mode0add, mode0ads, cpalval[];
extern u4 taddnfy16x16, taddfy16x16, ngwinen, nglogicval, ngwintable[];
extern u1 bgmode, intrlng[], mosenng[], mosszng[], osm2dis, xtravbuf[];
extern u1* pesimpng;
extern u2 BG3SXl[], BG3SYl[], BGPT3[], BGPT3X[];
extern u4 cfieldad, mosstart[], yposng, flipyposng, yposngom, flipyposngom;
extern u4 ofsmcptr, ofsmcptr2, ofsmady, ofsmadx, ofsmtptr, ofsmtptrs;
extern u4 ofsmmptr, ofsmcyps, ofshvaladd, ofsmval, ofsmvalh, bgtxadd2;
extern u4 CPalPtrng;

/* Not in endmem: the mode-to-depth table the dispatch reads, and the direct
   colour palette. Zero here would pin every layer to "draws nothing". */
/* Nine rows, not eight: the dispatch reads a dword at mode*4 + bg, so the last
   layer of the last mode reads three bytes past the table. The emulator's copy
   has whatever follows it in .data; here the slack row makes it defined. */
u1 colormodedef[9][4] = { { 1, 1, 1, 1 }, { 2, 2, 1, 0 }, { 2, 2, 0, 0 },
    { 3, 2, 0, 0 }, { 3, 1, 0, 0 }, { 2, 1, 0, 0 }, { 2, 0, 0, 0 },
    { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
u4 dcolortab[256];
/* Excluded from the stubs because ngtransp_extra.c has the first two; vram is
   the tile area the line dispatchers index, which this test does not drive. */
u1* vram;
/* The dispatcher takes the mosaic tail itself now; the renderers here are
   recorders, so it never fires. */
u4 ng2_mosaic;
u4 rec_mos_hits;
void c_domosaicng16b(void);
void c_domosaicng16b(void) { rec_mos_hits++; }

/* --- the recorders ------------------------------------------------------- */
u4 rec_regs[7]; /* eax ebx ecx edx esi edi ebp at the jump */
u4 rec_which; /* 1..6, which renderer, 0 if none */
u4 rec_bw_hits, rec_bw_a, rec_bw_b, rec_gdc_hits;

/* Two recorders, because the two sides reach it differently: the oracle is the
   pre-port assembly, where ProcessBuildWindow still called BuildWindow with
   eax and ebx, and the port calls the cdecl C one it became. mkoracle's
   --define renames the oracle's call to the register-ABI copy. Both record the
   same thing and seed the same run list. */
void BuildWindow(u4 a, u4 b);
void BuildWindow(u4 const a, u4 const b)
{
    rec_bw_hits++;
    rec_bw_a = a;
    rec_bw_b = b;
    ngwintable[0] = 40;
    ngwintable[1] = 60;
}

__asm__(".text\n.globl BuildWindow_reg\nBuildWindow_reg:\n"
        "  movl %eax, rec_bw_a\n  movl %ebx, rec_bw_b\n"
        "  incl rec_bw_hits\n"
        "  movl $40, ngwintable\n  movl $60, ngwintable+4\n"
        "  ret\n");

void Gendcolortable(void);
void Gendcolortable(void) { rec_gdc_hits++; }

/* The assembly side. Entered by jmp with one word pushed, so it pops. */
__asm__(".text\n"
        ".globl drawtileng2b16b\n drawtileng2b16b: movl $1, rec_which\n jmp rec_common\n"
        ".globl drawtileng4b16b\n drawtileng4b16b: movl $2, rec_which\n jmp rec_common\n"
        ".globl drawtileng8b16b\n drawtileng8b16b: movl $3, rec_which\n jmp rec_common\n"
        ".globl drawtileng16x162b16b\n drawtileng16x162b16b: movl $4, rec_which\n jmp rec_common\n"
        ".globl drawtileng16x164b16b\n drawtileng16x164b16b: movl $5, rec_which\n jmp rec_common\n"
        ".globl drawtileng16x168b16b\n drawtileng16x168b16b: movl $6, rec_which\n jmp rec_common\n"
        ".globl drawlineng2b16b\n drawlineng2b16b: movl $7, rec_which\n jmp rec_common\n"
        ".globl drawlineng4b16b\n drawlineng4b16b: movl $8, rec_which\n jmp rec_common\n"
        ".globl drawlineng8b16b\n drawlineng8b16b: movl $9, rec_which\n jmp rec_common\n"
        ".globl drawlineng16x162b16b\n drawlineng16x162b16b: movl $10, rec_which\n jmp rec_common\n"
        ".globl drawlineng16x164b16b\n drawlineng16x164b16b: movl $11, rec_which\n jmp rec_common\n"
        ".globl drawlineng16x168b16b\n drawlineng16x168b16b: movl $12, rec_which\n jmp rec_common\n"
        ".globl drawlineng16x82b16b\n drawlineng16x82b16b: movl $13, rec_which\n jmp rec_common\n"
        ".globl drawlineng16x84b16b\n drawlineng16x84b16b: movl $14, rec_which\n jmp rec_common\n"        ".globl drawlinengom2b16b\n drawlinengom2b16b: movl $15, rec_which\n jmp rec_common\n"        ".globl drawlinengom4b16b\n drawlinengom4b16b: movl $16, rec_which\n jmp rec_common\n"        ".globl drawlinengom8b16b\n drawlinengom8b16b: movl $17, rec_which\n jmp rec_common\n"        ".globl drawlinengom16x162b16b\n drawlinengom16x162b16b: movl $18, rec_which\n jmp rec_common\n"        ".globl drawlinengom16x164b16b\n drawlinengom16x164b16b: movl $19, rec_which\n jmp rec_common\n"        ".globl drawlinengom16x168b16b\n drawlinengom16x168b16b: movl $20, rec_which\n jmp rec_common\n"
        "rec_common:\n"
        "  movl %eax, rec_regs+0\n  movl %ebx, rec_regs+4\n"
        "  movl %ecx, rec_regs+8\n  movl %edx, rec_regs+12\n"
        "  movl %esi, rec_regs+16\n movl %edi, rec_regs+20\n"
        "  movl %ebp, rec_regs+24\n"
        "  popl %ebx\n  ret\n");

/* The C side records the same thing out of the register block. */
static void rec_block(u4 const* const r, u4 const which)
{
    rec_which = which;
    rec_regs[0] = r[R_EAX];
    rec_regs[1] = r[R_EBX];
    rec_regs[2] = r[R_ECX];
    rec_regs[3] = r[R_EDX];
    rec_regs[4] = r[R_ESI];
    rec_regs[5] = r[R_EDI];
    rec_regs[6] = r[R_EBP];
}

void c_ng_drawtileng2b16b(u4* r);
void c_ng_drawtileng4b16b(u4* r);
void c_ng_drawtileng8b16b(u4* r);
void c_ng_drawtileng16x162b16b(u4* r);
void c_ng_drawtileng16x164b16b(u4* r);
void c_ng_drawtileng16x168b16b(u4* r);
void c_ng_drawtileng2b16b(u4* r) { rec_block(r, 1); }
void c_ng_drawtileng4b16b(u4* r) { rec_block(r, 2); }
void c_ng_drawtileng8b16b(u4* r) { rec_block(r, 3); }
void c_ng_drawtileng16x162b16b(u4* r) { rec_block(r, 4); }
void c_ng_drawtileng16x164b16b(u4* r) { rec_block(r, 5); }
void c_ng_drawtileng16x168b16b(u4* r) { rec_block(r, 6); }

void c_ng_drawlineng2b16b(u4* r);
void c_ng_drawlineng4b16b(u4* r);
void c_ng_drawlineng8b16b(u4* r);
void c_ng_drawlineng16x162b16b(u4* r);
void c_ng_drawlineng16x164b16b(u4* r);
void c_ng_drawlineng16x168b16b(u4* r);
void c_ng_drawlineng16x82b16b(u4* r);
void c_ng_drawlineng16x84b16b(u4* r);
void c_ng_drawlinengom2b16b(u4* r);
void c_ng_drawlinengom4b16b(u4* r);
void c_ng_drawlinengom8b16b(u4* r);
void c_ng_drawlinengom16x162b16b(u4* r);
void c_ng_drawlinengom16x164b16b(u4* r);
void c_ng_drawlinengom16x168b16b(u4* r);
void c_ng_drawlineng2b16b(u4* r) { rec_block(r, 7); }
void c_ng_drawlineng4b16b(u4* r) { rec_block(r, 8); }
void c_ng_drawlineng8b16b(u4* r) { rec_block(r, 9); }
void c_ng_drawlineng16x162b16b(u4* r) { rec_block(r, 10); }
void c_ng_drawlineng16x164b16b(u4* r) { rec_block(r, 11); }
void c_ng_drawlineng16x168b16b(u4* r) { rec_block(r, 12); }
void c_ng_drawlineng16x82b16b(u4* r) { rec_block(r, 13); }
void c_ng_drawlineng16x84b16b(u4* r) { rec_block(r, 14); }
void c_ng_drawlinengom2b16b(u4* r) { rec_block(r, 15); }
void c_ng_drawlinengom4b16b(u4* r) { rec_block(r, 16); }
void c_ng_drawlinengom8b16b(u4* r) { rec_block(r, 17); }
void c_ng_drawlinengom16x162b16b(u4* r) { rec_block(r, 18); }
void c_ng_drawlinengom16x164b16b(u4* r) { rec_block(r, 19); }
void c_ng_drawlinengom16x168b16b(u4* r) { rec_block(r, 20); }

void asm_drawbg1tile16b(void), asm_drawbg2tile16b(void);
void asm_drawbg3tile16b(void), asm_drawbg4tile16b(void);
void asm_drawbg1tilepr116b(void), asm_drawbg2tilepr116b(void);
void asm_drawbg3tilepr116b(void), asm_drawbg4tilepr116b(void);
void asm_drawbg1line16b(void), asm_drawbg2line16b(void);
void asm_drawbg3line16b(void), asm_drawbg4line16b(void);
void asm_drawbg1linepr116b(void), asm_drawbg2linepr116b(void);
void asm_drawbg3linepr116b(void), asm_drawbg4linepr116b(void);
void c_drawbg1tile16b(u4* r), c_drawbg2tile16b(u4* r);
void c_drawbg3tile16b(u4* r), c_drawbg4tile16b(u4* r);
void c_drawbg1tilepr116b(u4* r), c_drawbg2tilepr116b(u4* r);
void c_drawbg3tilepr116b(u4* r), c_drawbg4tilepr116b(u4* r);
void c_drawbg1line16b(u4* r), c_drawbg2line16b(u4* r);
void c_drawbg3line16b(u4* r), c_drawbg4line16b(u4* r);
void c_drawbg1linepr116b(u4* r), c_drawbg2linepr116b(u4* r);
void c_drawbg3linepr116b(u4* r), c_drawbg4linepr116b(u4* r);

static u4 IN_EAX, IN_EBX, IN_ECX, IN_EDX, IN_ESI, IN_EDI, IN_EBP;

/* The emulator reaches these through calldl16t (video/makev16t.asm): it loads
   the register block, calls through DLFN, and stores the registers back. Use
   the same bridge rather than a hand-rolled one - it is what the dispatchers
   were written against, it saves the callee-saved set the ABI requires, and a
   bespoke caller here is one more thing that can be wrong. */
unsigned int DLR[7];
void (*DLFN)(void);
void calldl16t(void);
__asm__(".text\n.globl calldl16t\ncalldl16t:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl DLR+4, %ebx\n  movl DLR+8, %ecx\n  movl DLR+12, %edx\n"
        "  movl DLR+16, %esi\n movl DLR+20, %edi\n movl DLR+24, %ebp\n"
        "  movl DLR, %eax\n"
        "  call *DLFN\n"
        "  movl %eax, DLR\n   movl %ebx, DLR+4\n  movl %ecx, DLR+8\n"
        "  movl %edx, DLR+12\n movl %esi, DLR+16\n movl %edi, DLR+20\n"
        "  movl %ebp, DLR+24\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n  ret\n");

static void asm_call(void (*const fn)(void))
{
    DLR[0] = IN_EAX;
    DLR[1] = IN_EBX;
    DLR[2] = IN_ECX;
    DLR[3] = IN_EDX;
    DLR[4] = IN_ESI;
    DLR[5] = IN_EDI;
    DLR[6] = IN_EBP;
    DLFN = fn;
    calldl16t();
}

static void call_bg1(void) { asm_call(asm_drawbg1tile16b); }
static void call_bg2(void) { asm_call(asm_drawbg2tile16b); }
static void call_bg3(void) { asm_call(asm_drawbg3tile16b); }
static void call_bg4(void) { asm_call(asm_drawbg4tile16b); }
static void call_p1(void) { asm_call(asm_drawbg1tilepr116b); }
static void call_p2(void) { asm_call(asm_drawbg2tilepr116b); }
static void call_p3(void) { asm_call(asm_drawbg3tilepr116b); }
static void call_p4(void) { asm_call(asm_drawbg4tilepr116b); }
static void call_l1(void) { asm_call(asm_drawbg1line16b); }
static void call_l2(void) { asm_call(asm_drawbg2line16b); }
static void call_l3(void) { asm_call(asm_drawbg3line16b); }
static void call_l4(void) { asm_call(asm_drawbg4line16b); }
static void call_q1(void) { asm_call(asm_drawbg1linepr116b); }
static void call_q2(void) { asm_call(asm_drawbg2linepr116b); }
static void call_q3(void) { asm_call(asm_drawbg3linepr116b); }
static void call_q4(void) { asm_call(asm_drawbg4linepr116b); }

static u1 vidbuf[8192];

typedef struct {
    u4 regs[7], which, bw_hits, bw_a, bw_b, gdc_hits;
    u4 bgtxadd, ngptrdat2, mode0add, taddn, taddf;
    u4 yp, fyp, ypom, fypom, cpal, pesim;
    u4 om[11], btx2, xtsum;
    u4 ngbgval, ngprval, ngwinen, nglogic, wintab[4];
    u2 bgtxad;
    u4 ngptrdat, ngceax, ngcedi;
    u1 mosaic, prevbright;
} snap;

static void reset(void)
{
    memset(rec_regs, 0, sizeof rec_regs);
    rec_which = rec_bw_hits = rec_bw_a = rec_bw_b = rec_gdc_hits = 0;
    bgtxadd = ngptrdat2 = mode0add = 0;
    taddnfy16x16 = taddfy16x16 = 0;
    ng16bbgval = ng16bprval = ngwinen = nglogicval = 0;
    curmosaicsz = 0;
    memset(ngwintable, 0, 16);
    yposng = flipyposng = yposngom = flipyposngom = 0;
    ofsmcptr = ofsmcptr2 = ofsmady = ofsmadx = ofsmtptr = ofsmtptrs = 0;
    ofsmmptr = ofsmcyps = ofshvaladd = ofsmval = ofsmvalh = bgtxadd2 = 0;
    CPalPtrng = 0;
    pesimpng = 0;
    memset(xtravbuf, 0, 576);
    /* ngptrdat / ngceax / ngcedi / bgtxad are outputs of pass 0 and *inputs*
       to pass 1, so they are filled per iteration rather than cleared here. */
}

static void grab(snap* const s, u4 const i)
{
    memcpy(s->regs, rec_regs, sizeof rec_regs);
    s->which = rec_which;
    s->bw_hits = rec_bw_hits;
    s->bw_a = rec_bw_a;
    s->bw_b = rec_bw_b;
    s->gdc_hits = rec_gdc_hits;
    s->bgtxadd = bgtxadd;
    s->ngptrdat2 = ngptrdat2;
    s->mode0add = mode0add;
    s->taddn = taddnfy16x16;
    s->taddf = taddfy16x16;
    s->ngbgval = ng16bbgval;
    s->ngprval = ng16bprval;
    s->ngwinen = ngwinen;
    s->nglogic = nglogicval;
    memcpy(s->wintab, ngwintable, 16);
    s->bgtxad = bgtxad[i];
    s->ngptrdat = ngptrdat[i];
    s->ngceax = ngceax[i];
    s->ngcedi = ngcedi[i];
    s->mosaic = curmosaicsz;
    s->prevbright = prevbrightdc;
    s->yp = yposng;
    s->fyp = flipyposng;
    s->ypom = yposngom;
    s->fypom = flipyposngom;
    s->cpal = CPalPtrng;
    s->pesim = (u4)(uintptr_t)pesimpng;
    s->om[0] = ofsmcptr;
    s->om[1] = ofsmcptr2;
    s->om[2] = ofsmady;
    s->om[3] = ofsmadx;
    s->om[4] = ofsmtptr;
    s->om[5] = ofsmtptrs;
    s->om[6] = ofsmmptr;
    s->om[7] = ofsmcyps;
    s->om[8] = ofshvaladd;
    s->om[9] = ofsmval;
    s->om[10] = ofsmvalh;
    s->btx2 = bgtxadd2;
    {   /* the mosaic scratch line, as a checksum */
        u4 q, h = 0;
        for (q = 0; q < 544u; q++)
            h = h * 31u + xtravbuf[q];
        s->xtsum = h;
    }
}

int main(void)
{
    void (*const asmside[4][4])(void)
        = { { call_bg1, call_bg2, call_bg3, call_bg4 },
              { call_p1, call_p2, call_p3, call_p4 },
              { call_l1, call_l2, call_l3, call_l4 },
              { call_q1, call_q2, call_q3, call_q4 } };
    void (*const cside[4][4])(u4*)
        = { { c_drawbg1tile16b, c_drawbg2tile16b, c_drawbg3tile16b,
                c_drawbg4tile16b },
              { c_drawbg1tilepr116b, c_drawbg2tilepr116b, c_drawbg3tilepr116b,
                  c_drawbg4tilepr116b },
              { c_drawbg1line16b, c_drawbg2line16b, c_drawbg3line16b,
                  c_drawbg4line16b },
              { c_drawbg1linepr116b, c_drawbg2linepr116b,
                  c_drawbg3linepr116b, c_drawbg4linepr116b } };
    u4 passes[4] = { 0 };
    /* 0 = none, then the six tile renderers, the eight line ones and the
       six offset-per-tile ones - see rec_which. */
    u4 counts[21] = { 0 };
    u4 big = 0, win = 0, direct = 0;

    DT_MAIN(20260823, 40000)
    {
        u4 const bg = dt_mod(4);
        u4 const bx = dt_mod(224);
        /* Pass 1 reads back what pass 0 cached, so it needs those filled. */
        u4 const pass = dt_mod(4);
        u4 const i = bx + bg * 256u;
        snap a, b;

        dt_fill((u1*)BGPT1, 2048);
        dt_fill((u1*)BGPT1X, 2048);
        dt_fill((u1*)BGPT1Y, 2048);
        dt_fill((u1*)BG1SXl, 2048);
        dt_fill((u1*)BGOPT1, 2048);
        /* The scanline's BG mode, so 0..7. Random bytes land on the two
           offset-per-tile values twice in 256 and left those leaves at a
           handful of hits in 40000. */
        for (u4 q = 0; q < 256; q++)
            BGMA[q] = (u1)dt_mod(8);
        /* 0 or 1, not a random byte: the 16x16 path is taken on exactly 1,
           so random bytes reach it once in 256 and it was getting 68 of
           40000 runs. */
        for (u4 q = 0; q < 1024; q++)
            t16x161[q] = (u1)dt_mod(2);
        dt_fill(scadsng, 256);
        dt_fill(winbg1enval, 1024);
        dt_fill(winlogicaval, 512);
        dt_fill((u1*)cpalval, 1024);
        dt_fill((u1*)BG1SYl, 2048);
        dt_fill((u1*)ngceax, 4096);
        dt_fill((u1*)ngcedi, 4096);
        dt_fill((u1*)ngptrdat, 4096);
        dt_fill((u1*)bgtxad, 2048);
        vidbright = (u1)dt_mod(16);
        prevbrightdc = (u1)dt_mod(16);
        mode0ads = dt_u32();
        bgmode = (u1)dt_mod(8);
        osm2dis = (u1)dt_mod(2);
        cfieldad = dt_mod(64);
        dt_fill(intrlng, 256);
        dt_fill(mosenng, 256);
        dt_fill(mosszng, 256);
        dt_fill((u1*)mosstart, 16);
        dt_fill((u1*)BG3SXl, 512);
        dt_fill((u1*)BG3SYl, 512);
        dt_fill((u1*)BGPT3, 512);
        dt_fill((u1*)BGPT3X, 512);
        vram = vidbuf;

        IN_EAX = dt_u32();
        IN_EBX = bx;
        IN_ECX = dt_u32();
        IN_EDX = dt_u32();
        IN_ESI = (u4)(uintptr_t)vidbuf;
        IN_EDI = (u4)(uintptr_t)vidbuf + 64u;
        IN_EBP = dt_u32();

        {
            u1 const pb = prevbrightdc;
            reset();
            asmside[pass][bg]();
            grab(&a, i);

            prevbrightdc = pb;
            reset();
            {
                u4 r[8];
                r[R_EAX] = IN_EAX;
                r[R_EBX] = IN_EBX;
                r[R_ECX] = IN_ECX;
                r[R_EDX] = IN_EDX;
                r[R_ESI] = IN_ESI;
                r[R_EDI] = IN_EDI;
                r[R_EBP] = IN_EBP;
                r[R_ESP] = 0;
                cside[pass][bg](r);
            }
            grab(&b, i);
        }

        counts[a.which]++;
        passes[pass]++;
        if (a.which == 4 || a.which == 5 || a.which == 6 || a.which == 10
            || a.which == 11 || a.which == 12 || a.which >= 18)
            big++;
        if (a.bw_hits)
            win++;
        if (a.gdc_hits)
            direct++;

        if (a.which != b.which && DT_SHOW())
            printf("    bg=%u bx=%u pass=%u BGMA=%02x t16=%02x scadsng=%02x\n",
                bg, bx, pass, BGMA[bx], t16x161[i], scadsng[bx]);
        DT_EQ("renderer", a.which, b.which);
        {
            static char const* const rn[7]
                = { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp" };
            for (int q = 0; q < 7; q++)
                DT_EQ(rn[q], a.regs[q], b.regs[q]);
        }
        DT_EQ("BuildWindow hits", a.bw_hits, b.bw_hits);
        DT_EQ("BuildWindow line", a.bw_a, b.bw_a);
        DT_EQ("BuildWindow which", a.bw_b, b.bw_b);
        DT_EQ("Gendcolortable", a.gdc_hits, b.gdc_hits);
        DT_EQ("bgtxadd", a.bgtxadd, b.bgtxadd);
        DT_EQ("bgtxad", a.bgtxad, b.bgtxad);
        DT_EQ("ngptrdat", a.ngptrdat, b.ngptrdat);
        DT_EQ("ngptrdat2", a.ngptrdat2, b.ngptrdat2);
        DT_EQ("ngceax", a.ngceax, b.ngceax);
        DT_EQ("ngcedi", a.ngcedi, b.ngcedi);
        DT_EQ("mode0add", a.mode0add, b.mode0add);
        DT_EQ("taddnfy16x16", a.taddn, b.taddn);
        DT_EQ("taddfy16x16", a.taddf, b.taddf);
        DT_EQ("ng16bbgval", a.ngbgval, b.ngbgval);
        DT_EQ("ng16bprval", a.ngprval, b.ngprval);
        DT_EQ("ngwinen", a.ngwinen, b.ngwinen);
        DT_EQ("nglogicval", a.nglogic, b.nglogic);
        DT_EQ("curmosaicsz", a.mosaic, b.mosaic);
        DT_EQ("prevbrightdc", a.prevbright, b.prevbright);
        for (int q = 0; q < 4; q++)
            DT_EQ("ngwintable", a.wintab[q], b.wintab[q]);
        DT_EQ("yposng", a.yp, b.yp);
        DT_EQ("flipyposng", a.fyp, b.fyp);
        DT_EQ("yposngom", a.ypom, b.ypom);
        DT_EQ("flipyposngom", a.fypom, b.fypom);
        DT_EQ("CPalPtrng", a.cpal, b.cpal);
        DT_EQ("pesimpng", a.pesim, b.pesim);
        DT_EQ("bgtxadd2", a.btx2, b.btx2);
        DT_EQ("xtravbuf", a.xtsum, b.xtsum);
        {
            static char const* const on[11] = { "ofsmcptr", "ofsmcptr2",
                "ofsmady", "ofsmadx", "ofsmtptr", "ofsmtptrs", "ofsmmptr",
                "ofsmcyps", "ofshvaladd", "ofsmval", "ofsmvalh" };
            for (int q = 0; q < 11; q++)
                DT_EQ(on[q], a.om[q], b.om[q]);
        }
    }
    {
        static char const* const nm[21] = { "none", "t2b", "t4b", "t8b",
            "t16x2b", "t16x4b", "t16x8b", "l2b", "l4b", "l8b", "l16x2b",
            "l16x4b", "l16x8b", "l16x8-2b", "l16x8-4b", "om2b", "om4b",
            "om8b", "om16x2b", "om16x4b", "om16x8b" };
        printf("  renderer picked:");
        for (int q = 0; q < 21; q++)
            if (counts[q])
                printf(" %s=%u", nm[q], counts[q]);
        printf("\n");
    }
    printf("  16x16 %u, window built %u, direct colour %u; "
           "tile %u pr1 %u line %u linepr1 %u\n",
        big, win, direct, passes[0], passes[1], passes[2], passes[3]);
    DT_DONE("newgfx16 background tile dispatchers");
}
