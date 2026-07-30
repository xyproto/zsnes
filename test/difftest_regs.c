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
u1 oamram[1024];
u2 cgram[256];
u4 oamaddr;
u2 cgaddr, latchx, latchy;
u1 winl1, winr1, winl2, winr2, winlogica, winlogicb;
u1 winenabm, winenabs, scaddset, scaddtype, INTEnab, multa;
u2 scrnon, diva;
u1 bgscrolPrev, vramread;
u2 bg1scrolx, bg2scrolx, bg3scrolx, bg4scrolx;
u2 bg1scroly, bg2scroly, bg3scroly, bg4scroly;
u2 bg1scrolx_m7, bg1scroly_m7;
u2 mode7C, mode7D, mode7X0, mode7Y0;
u1 dmadata[129], hdmarestart, nohdmaframe, hdmadelay, SPC7110Enable;
u2 resolutn, curypos;
static u1 dma_init[129];
/* The write table; the oracle's cpu/regsw.mac names it. */
void (*regptwa[0x3000])(void);
static u1 oam_init[1024];
static u2 cg_init[256];
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
DECL(reg2138r);
DECL(reg213Br);
DECL(reg213Cr);
DECL(reg213Dr);
DECL(reg2126w);
DECL(reg2127w);
DECL(reg2128w);
DECL(reg2129w);
DECL(reg212Aw);
DECL(reg212Bw);
DECL(reg212Cw);
DECL(reg212Ew);
DECL(reg212Fw);
DECL(reg2130w);
DECL(reg2131w);
DECL(reg2181w);
DECL(reg4200w);
DECL(reg4202w);
DECL(reg4204w);
DECL(reg210Dw);
DECL(reg210Ew);
DECL(reg210Fw);
DECL(reg2110w);
DECL(reg2111w);
DECL(reg2112w);
DECL(reg2113w);
DECL(reg2114w);
DECL(reg211Bw);
DECL(reg211Cw);
DECL(reg211Dw);
DECL(reg211Ew);
DECL(reg211Fw);
DECL(reg2120w);
DECL(reg2121w);
DECL(reg212Dw);
DECL(reg2180w);
DECL(reg2182w);
DECL(reg4205w);
DECL(reg43X0w);
DECL(reg43X1w);
DECL(reg43x2w);
DECL(reg43x3w);
DECL(reg43x4w);
DECL(reg43x5w);
DECL(reg43x6w);
DECL(reg43x7w);
DECL(reg43x8w);
DECL(reg43x9w);
DECL(reg43XAw);
DECL(reg43XBw);
DECL(regINVALIDw);
DECL(reg43XXr);
DECL(regINVALID);
#undef DECL

/* Call a handler with eax, ecx and edx set, and report what came back.
   The .text is not optional: top-level asm lands in whatever section the
   previous definition left current, and the array above puts that in .bss -
   emitting code there links as "file truncated". */
u4 regs_out[4];
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
        "movl %ebx, regs_out+12\n"
        "addl $4, %esp\n"
        "popl %ebp\n"
        "popl %edi\n"
        "popl %esi\n"
        "popl %ebx\n"
        "ret\n"
        ".text\n");
/* ebx too: UpdateScrollRegX shifts the whole 32-bit register, so drive its
   upper half to prove the caller's bits cannot leak into the result. */
void regs_call(void* fn, u4 eax, u4 ecx, u4 edx, u4 ebx);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* 1 = the handler reads cx and indexes dmadata with it, so the address has
       to stay inside $4300..$4380 or both sides walk off the table. */
    int dma;
} regcase;

#define CASE(n) { #n, asm_##n, n, 0 }
#define CASE_DMA(n) { #n, asm_##n, n, 1 }
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
    CASE(reg2138r),
    CASE(reg213Br),
    CASE(reg213Cr),
    CASE(reg213Dr),
    CASE(reg2126w),
    CASE(reg2127w),
    CASE(reg2128w),
    CASE(reg2129w),
    CASE(reg212Aw),
    CASE(reg212Bw),
    CASE(reg212Cw),
    CASE(reg212Ew),
    CASE(reg212Fw),
    CASE(reg2130w),
    CASE(reg2131w),
    CASE(reg2181w),
    CASE(reg4200w),
    CASE(reg4202w),
    CASE(reg4204w),
    CASE(reg210Dw),
    CASE(reg210Ew),
    CASE(reg210Fw),
    CASE(reg2110w),
    CASE(reg2111w),
    CASE(reg2112w),
    CASE(reg2113w),
    CASE(reg2114w),
    CASE(reg211Bw),
    CASE(reg211Cw),
    CASE(reg211Dw),
    CASE(reg211Ew),
    CASE(reg211Fw),
    CASE(reg2120w),
    CASE(reg2121w),
    CASE(reg212Dw),
    CASE(reg2180w),
    CASE(reg2182w),
    CASE(reg4205w),
    CASE_DMA(reg43X0w),
    CASE_DMA(reg43X1w),
    CASE_DMA(reg43x2w),
    CASE_DMA(reg43x3w),
    CASE_DMA(reg43x4w),
    CASE_DMA(reg43x5w),
    CASE_DMA(reg43x6w),
    CASE_DMA(reg43x7w),
    CASE_DMA(reg43x8w),
    CASE_DMA(reg43x9w),
    CASE_DMA(reg43XAw),
    CASE_DMA(reg43XBw),
    CASE_DMA(reg43XXr),
    CASE(regINVALIDw),
    CASE_DMA(regINVALID),
};
#undef CASE
#undef CASE_DMA

typedef struct {
    u4 eax, ecx, edx;
    u1 mult[3], change;
    u1 lx, ly, mdr2, nmi, cur, irq;
    u4 wadr, oaddr;
    u2 caddr, scr, dv2;
    u1 wr[12], prev;
    u2 sc[12];
    u2 m7[6];
    u1 dma[129], hres, nohd, hdel;
    u1 wram[0x20000];
} snapshot;

typedef struct {
    u1 vb, fb, mc, rto, pal, pst, cf, ext, lx, ly, nmien, mdr, cur, irq;
    u1 iop, mdr2in;
    u2 dv, mr;
    u4 wadr, ja, jb, jc, jd, oaddr;
    u1 w0;
    u4 ebxin;
    u2 scr0, dv0, sc0, res0, cury0;
    u1 prev0, hres0, nohd0, hdel0, spc7110;
    u2 caddr, lx16, ly16;
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
    ppu2_mdr = in->mdr2in;
    wramrwadr = in->wadr;
    ioportval = in->iop;
    divres = in->dv;
    multres = in->mr;
    JoyARead = in->ja;
    JoyBRead = in->jb;
    JoyCRead2 = in->jc;
    JoyDRead = in->jd;
    memcpy(oamram, oam_init, sizeof oamram);
    memcpy(cgram, cg_init, sizeof cgram);
    oamaddr = in->oaddr;
    cgaddr = in->caddr;
    latchx = in->lx16;
    latchy = in->ly16;
    winl1 = in->w0;
    winr1 = in->w0;
    winl2 = in->w0;
    winr2 = in->w0;
    winlogica = in->w0;
    winlogicb = in->w0;
    winenabm = in->w0;
    winenabs = in->w0;
    scaddset = in->w0;
    scaddtype = in->w0;
    INTEnab = in->w0;
    multa = in->w0;
    scrnon = in->scr0;
    diva = in->dv0;
    bgscrolPrev = in->prev0;
    bg1scrolx = bg2scrolx = bg3scrolx = bg4scrolx = in->sc0;
    bg1scroly = bg2scroly = bg3scroly = bg4scroly = in->sc0;
    bg1scrolx_m7 = bg1scroly_m7 = in->sc0;
    mode7C = mode7D = mode7X0 = mode7Y0 = in->sc0;
    memcpy(dmadata, dma_init, sizeof dmadata);
    hdmarestart = in->hres0;
    nohdmaframe = in->nohd0;
    hdmadelay = in->hdel0;
    SPC7110Enable = in->spc7110;
    resolutn = in->res0;
    curypos = in->cury0;
    vidbright = vb;
    forceblnk = fb;
    multchange = mc;
    mode7A = m7a;
    mode7B = m7b;
    memcpy(compmult, mult, 3);

    regs_call((void*)fn, a, c, d, in->ebxin);

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
    out->oaddr = oamaddr;
    out->caddr = cgaddr;
    out->scr = scrnon;
    out->dv2 = diva;
    {
        u1 const w[12] = { winl1, winr1, winl2, winr2, winlogica, winlogicb,
            winenabm, winenabs, scaddset, scaddtype, INTEnab, multa };
        memcpy(out->wr, w, sizeof out->wr);
    }
    out->prev = bgscrolPrev;
    {
        u2 const sc[12] = { bg1scrolx, bg2scrolx, bg3scrolx, bg4scrolx,
            bg1scroly, bg2scroly, bg3scroly, bg4scroly, bg1scrolx_m7,
            bg1scroly_m7, 0, 0 };
        u2 const m7[6] = { mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0 };
        memcpy(out->sc, sc, sizeof out->sc);
        memcpy(out->m7, m7, sizeof out->m7);
    }
    memcpy(out->dma, dmadata, sizeof out->dma);
    out->hres = hdmarestart;
    out->nohd = nohdmaframe;
    out->hdel = hdmadelay;
    memcpy(out->wram, wram_store, sizeof out->wram);
}

int main(void)
{
    dt_fill(wram_init, sizeof wram_init);
    dt_fill(oam_init, sizeof oam_init);
    dt_fill(cg_init, sizeof cg_init);
    dt_fill(dma_init, sizeof dma_init);
    DT_MAIN(20260730, 200000)
    {
        regcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot x, y;
        u4 const a = dt_u32(), d = dt_u32();
        u4 c = dt_u32();
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
        /* PPU2's bus latch is what the second counter read keeps 7 bits of. */
        in.mdr2in = (u1)dt_u32();
        in.dv = (u2)dt_u32();
        in.mr = (u2)dt_u32();
        in.ja = dt_u32();
        in.jb = dt_u32();
        in.jc = dt_u32();
        in.jd = dt_u32();
        /* OAM wraps one past 543, so sit on that edge; CGRAM's is 9 bits. */
        in.oaddr = dt_mod(2) ? (dt_u32() & ~0xFFFFu) | (542u + dt_mod(4))
                             : (dt_u32() & ~0xFFFFu) | dt_mod(544);
        in.caddr = (u2)(dt_mod(2) ? 0x1FEu + dt_mod(3) : dt_mod(0x200));
        in.lx16 = (u2)dt_u32();
        in.ly16 = (u2)dt_u32();
        in.w0 = (u1)dt_u32();
        in.ebxin = dt_u32();
        if (k->dma) {
            /* Inside the DMA window; regINVALID also wants the $2100 edge. */
            c = dt_mod(2) ? 0x4300u + dt_mod(129)
                          : (dt_u32() & 0xFFFF0000u) | (0x20FEu + dt_mod(4));
            if (k->c_fn != regINVALID) {
                c = 0x4300u + dt_mod(129);
            }
            /* The index is `mov bx,cx`, so the high half is dropped. Drive it
               dirty: anything that forgets the 16-bit truncation indexes far
               outside the 129-byte table. */
            c |= dt_u32() & 0xFFFF0000u;
        }
        /* The high halves must survive a byte store. */
        in.scr0 = (u2)dt_u32();
        in.dv0 = (u2)dt_u32();
        in.sc0 = (u2)dt_u32();
        in.prev0 = (u1)dt_u32();
        in.hres0 = (u1)dt_u32();
        in.nohd0 = (u1)dt_u32();
        in.hdel0 = (u1)dt_u32();
        in.spc7110 = (u1)(dt_mod(2) ? 0 : dt_u32());
        /* $43XA compares the current line against the last visible one, so
           straddle that boundary rather than sampling it by luck. */
        in.res0 = (u2)(dt_mod(2) ? 224 : dt_u32() & 0x1FF);
        in.cury0 = (u2)(dt_mod(2) ? in.res0 + dt_mod(3) - 1 : dt_u32() & 0x1FF);

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
        DT_EQ("oamaddr", x.oaddr, y.oaddr);
        DT_EQ("cgaddr", x.caddr, y.caddr);
        DT_EQ("scrnon", x.scr, y.scr);
        DT_EQ("diva", x.dv2, y.dv2);
        DT_MEM("byte registers", x.wr, y.wr, sizeof x.wr);
        DT_EQ("bgscrolPrev", x.prev, y.prev);
        DT_MEM("scroll registers", x.sc, y.sc, sizeof x.sc);
        DT_MEM("mode 7 matrix", x.m7, y.m7, sizeof x.m7);
        DT_MEM("dmadata", x.dma, y.dma, sizeof x.dma);
        DT_EQ("hdmarestart", x.hres, y.hres);
        DT_EQ("nohdmaframe", x.nohd, y.nohd);
        DT_EQ("hdmadelay", x.hdel, y.hdel);
        DT_MEM("wramdata", x.wram, y.wram, sizeof x.wram);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s eax=%x mult=%d A=%04x B=%04x\n", k->name, a, mc, m7a, m7b);
        }
    }
    DT_DONE("PPU register reads and writes");
}
