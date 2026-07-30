/* Differential test: the PPU read handlers in cpu/regs.inc against the C port
 * in cpu/c_regs.c.
 *
 * Legacy ABI: no argument, value in al, and ecx, edx and the upper half of eax
 * must survive. Both sides are called through the same shim so the registers
 * on the way in are identical; ebx is deliberately not compared - neither side
 * promises it, and the only callers (cpu/mem_ops.h) restore their own.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;
typedef int8_t s1;

u1 vidbright, forceblnk, multchange, compmult[3];
u2 mode7A, mode7B;
u1 rtoflags, romispal, ppustatus, cfield, extlatch, ppu2_mdr;
u1 latchxr, latchyr, NMIEnab, cpu_mdr, curnmi, irqon;
u4 wramrwadr;
u1 ioportval;
u2 divres, multres;
u4 JoyARead, JoyBRead, JoyCRead2, JoyDRead;
/* The $2180 port walks a 128K window, so the buffer has to cover the wrap. */
static u1 wram_store[0x20000], wram_init[0x20000];
u1* wramdata = wram_store;
/* The table the handlers are installed in; unused here but the oracle's
   cpu/regs.mac references it. */
void (*regptra[0x3000])(void);

#define DECL(n)                \
    extern void asm_##n(void); \
    void n(void)
DECL(reg2100r);
DECL(reg2134r);
DECL(reg2135r);
DECL(reg2136r);
DECL(reg213Er);
DECL(reg213Fr);
DECL(reg2180r);
DECL(reg21C2r);
DECL(reg21C3r);
DECL(reg420Ar);
DECL(reg420Br);
DECL(reg420Fr);
extern void asm_reg21C3r(void), asm_reg420Br(void), asm_reg420Fr(void);
DECL(reg4210r);
DECL(reg4211r);
DECL(reg4213r);
DECL(reg4214r);
DECL(reg4215r);
DECL(reg4216r);
DECL(reg4217r);
DECL(reg4218r);
DECL(reg4219r);
DECL(reg421Ar);
DECL(reg421Br);
DECL(reg421Cr);
DECL(reg421Dr);
DECL(reg421Er);
DECL(reg421Fr);
#undef DECL

/* Call a handler with eax, ecx and edx set, and report what came back.
   The .text is not optional: top-level asm lands in whatever section the
   previous definition left current, and the array above puts that in .bss -
   emitting code there links as "file truncated". */
u4 regs_out[3];
__asm__(".text\n"
        ".globl regs_call\n"
        "regs_call:\n"
        "pushl %ebx\n"
        "pushl %esi\n"
        "pushl %edi\n"
        "pushl %ebp\n"
        "movl 20(%esp), %eax\n"
        "pushl %eax\n"
        "movl 28(%esp), %eax\n"
        "movl 32(%esp), %ecx\n"
        "movl 36(%esp), %edx\n"
        "call *(%esp)\n"
        "movl %eax, regs_out\n"
        "movl %ecx, regs_out+4\n"
        "movl %edx, regs_out+8\n"
        "addl $4, %esp\n"
        "popl %ebp\n"
        "popl %edi\n"
        "popl %esi\n"
        "popl %ebx\n"
        "ret\n"
        ".text\n");
void regs_call(void* fn, u4 eax, u4 ecx, u4 edx);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
} regcase;

#define CASE(n) { #n, asm_##n, n }
static regcase const cases[] = {
    CASE(reg2100r),
    CASE(reg2134r),
    CASE(reg2135r),
    CASE(reg2136r),
    CASE(reg213Er),
    CASE(reg213Fr),
    CASE(reg2180r),
    CASE(reg21C2r),
    CASE(reg420Ar),
    CASE(reg4210r),
    CASE(reg4211r),
    /* The aliases share one body in the assembly; each label is its own
       entry point, so drive every one of them. */
    CASE(reg21C3r),
    CASE(reg420Br),
    CASE(reg420Fr),
    CASE(reg4213r),
    CASE(reg4214r),
    CASE(reg4215r),
    CASE(reg4216r),
    CASE(reg4217r),
    CASE(reg4218r),
    CASE(reg4219r),
    CASE(reg421Ar),
    CASE(reg421Br),
    CASE(reg421Cr),
    CASE(reg421Dr),
    CASE(reg421Er),
    CASE(reg421Fr),
};
#undef CASE

typedef struct {
    u4 eax, ecx, edx;
    u1 mult[3], change;
    u1 lx, ly, mdr2, nmi, cur, irq;
    u4 wadr;
    u1 wram[0x20000];
} snapshot;

typedef struct {
    u1 vb, fb, mc, rto, pal, pst, cf, ext, lx, ly, nmien, mdr, cur, irq;
    u1 iop;
    u2 dv, mr;
    u4 wadr, ja, jb, jc, jd;
} state;

static void run(void (*fn)(void), u4 a, u4 c, u4 d, state const* in,
    u2 m7a, u2 m7b, u1 const* mult, snapshot* out)
{
    u1 const vb = in->vb, fb = in->fb, mc = in->mc;

    memcpy(wram_store, wram_init, sizeof wram_store);
    rtoflags = in->rto;
    romispal = in->pal;
    ppustatus = in->pst;
    cfield = in->cf;
    extlatch = in->ext;
    latchxr = in->lx;
    latchyr = in->ly;
    NMIEnab = in->nmien;
    cpu_mdr = in->mdr;
    curnmi = in->cur;
    irqon = in->irq;
    ppu2_mdr = 0;
    wramrwadr = in->wadr;
    ioportval = in->iop;
    divres = in->dv;
    multres = in->mr;
    JoyARead = in->ja;
    JoyBRead = in->jb;
    JoyCRead2 = in->jc;
    JoyDRead = in->jd;
    vidbright = vb;
    forceblnk = fb;
    multchange = mc;
    mode7A = m7a;
    mode7B = m7b;
    memcpy(compmult, mult, 3);

    regs_call((void*)fn, a, c, d);

    out->eax = regs_out[0];
    out->ecx = regs_out[1];
    out->edx = regs_out[2];
    memcpy(out->mult, compmult, 3);
    out->change = multchange;
    out->lx = latchxr;
    out->ly = latchyr;
    out->mdr2 = ppu2_mdr;
    out->nmi = NMIEnab;
    out->cur = curnmi;
    out->irq = irqon;
    out->wadr = wramrwadr;
    memcpy(out->wram, wram_store, sizeof out->wram);
}

int main(void)
{
    dt_fill(wram_init, sizeof wram_init);
    DT_MAIN(20260730, 200000)
    {
        regcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot x, y;
        u4 const a = dt_u32(), c = dt_u32(), d = dt_u32();
        state in;
        /* The deferred multiply only runs when the flag is set, so hit both;
           the operands are signed 16 x 8, so drive the sign bits. */
        u1 const mc = (u1)(dt_mod(2) ? dt_mod(256) : 0);
        u2 const m7a = (u2)(dt_mod(2) ? 0x8000u | dt_u32() : dt_u32());
        u2 const m7b = (u2)(dt_mod(2) ? 0x8000u | dt_u32() : dt_u32());
        u1 mult[3];

        mult[0] = (u1)dt_u32();
        mult[1] = (u1)dt_u32();
        mult[2] = (u1)dt_u32();

        in.vb = (u1)dt_u32();
        in.fb = (u1)dt_u32();
        in.mc = mc;
        in.rto = (u1)dt_u32();
        /* Only 0 and 1 are real, but the shift is 8-bit so drive it wider. */
        in.pal = dt_mod(2) ? (u1)dt_u32() : (u1)dt_mod(2);
        in.pst = (u1)dt_u32();
        in.cf = (u1)dt_u32();
        in.ext = (u1)dt_u32();
        in.lx = (u1)dt_u32();
        in.ly = (u1)dt_u32();
        in.nmien = (u1)dt_u32();
        in.mdr = (u1)dt_u32();
        /* Whether we are inside the NMI handler decides if $4210 re-arms. */
        in.cur = (u1)(dt_mod(2) ? 0 : dt_u32());
        in.irq = (u1)dt_u32();
        /* Sit on the 128K wrap half the time. */
        in.wadr = dt_mod(2) ? 0x1FFFFu - dt_mod(2) : dt_u32() & 0x1FFFFu;
        in.iop = (u1)dt_u32();
        in.dv = (u2)dt_u32();
        in.mr = (u2)dt_u32();
        in.ja = dt_u32();
        in.jb = dt_u32();
        in.jc = dt_u32();
        in.jd = dt_u32();

        run(k->asm_fn, a, c, d, &in, m7a, m7b, mult, &x);
        run(k->c_fn, a, c, d, &in, m7a, m7b, mult, &y);

        DT_EQ(k->name, x.eax, y.eax);
        DT_EQ("ecx", x.ecx, y.ecx);
        DT_EQ("edx", x.edx, y.edx);
        DT_MEM("compmult", x.mult, y.mult, sizeof x.mult);
        DT_EQ("multchange", x.change, y.change);
        DT_EQ("latchxr", x.lx, y.lx);
        DT_EQ("latchyr", x.ly, y.ly);
        DT_EQ("ppu2_mdr", x.mdr2, y.mdr2);
        DT_EQ("NMIEnab", x.nmi, y.nmi);
        DT_EQ("curnmi", x.cur, y.cur);
        DT_EQ("irqon", x.irq, y.irq);
        DT_EQ("wramrwadr", x.wadr, y.wadr);
        DT_MEM("wramdata", x.wram, y.wram, sizeof x.wram);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s eax=%x mult=%d A=%04x B=%04x\n", k->name, a, mc, m7a, m7b);
        }
    }
    DT_DONE("PPU register reads");
}
