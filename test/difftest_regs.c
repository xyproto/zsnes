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

#include "asmdata.h"
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
/* cpu/c_regsppu.c indexes these per layer as bg1ptr[n] while the asm reaches
   each layer by its own name, so the harness owns storage that has to satisfy
   both. Loose scalars line up only by luck: gcc emits them in declaration
   order at -O0 but reverses them at -O2, where bg1scroly[2] lands on
   bg3scrolx. Pin the layout the way cpu/c_regsdata.c does. */
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptr) ".skip 2\n" ASM_GSYM(bg2ptr) ".skip 2\n" ASM_GSYM(bg3ptr) ".skip 2\n" ASM_GSYM(bg4ptr) ".skip 2\n" ASM_SEC_END);
extern u2 bg1ptr[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptrb) ".skip 2\n" ASM_GSYM(bg2ptrb) ".skip 2\n" ASM_GSYM(bg3ptrb) ".skip 2\n" ASM_GSYM(bg4ptrb) ".skip 2\n" ASM_SEC_END);
extern u2 bg1ptrb[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptrc) ".skip 2\n" ASM_GSYM(bg2ptrc) ".skip 2\n" ASM_GSYM(bg3ptrc) ".skip 2\n" ASM_GSYM(bg4ptrc) ".skip 2\n" ASM_SEC_END);
extern u2 bg1ptrc[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptrd) ".skip 2\n" ASM_GSYM(bg2ptrd) ".skip 2\n" ASM_GSYM(bg3ptrd) ".skip 2\n" ASM_GSYM(bg4ptrd) ".skip 2\n" ASM_SEC_END);
extern u2 bg1ptrd[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptrx) ".skip 4\n" ASM_GSYM(bg2ptrx) ".skip 4\n" ASM_GSYM(bg3ptrx) ".skip 4\n" ASM_GSYM(bg4ptrx) ".skip 4\n" ASM_SEC_END);
extern u4 bg1ptrx[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1ptry) ".skip 4\n" ASM_GSYM(bg2ptry) ".skip 4\n" ASM_GSYM(bg3ptry) ".skip 4\n" ASM_GSYM(bg4ptry) ".skip 4\n" ASM_SEC_END);
extern u4 bg1ptry[4];
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1objptr) ".skip 2\n" ASM_GSYM(bg2objptr) ".skip 2\n" ASM_GSYM(bg3objptr) ".skip 2\n" ASM_GSYM(bg4objptr) ".skip 2\n" ASM_SEC_END);
extern u2 bg1objptr[4];

/* bg1sx separates the two scroll runs in cpu/c_regsdata.c; keep it here. */
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bg1scrolx) ".skip 2\n" ASM_GSYM(bg2scrolx) ".skip 2\n" ASM_GSYM(bg3scrolx) ".skip 2\n" ASM_GSYM(bg4scrolx) ".skip 2\n" ASM_GSYM(bg1sx) ".skip 2\n" ASM_GSYM(bg1scroly) ".skip 2\n" ASM_GSYM(bg2scroly) ".skip 2\n" ASM_GSYM(bg3scroly) ".skip 2\n" ASM_GSYM(bg4scroly) ".skip 2\n" ASM_SEC_END);
extern u2 bg1scrolx[4], bg1scroly[4];
u2 bg1scrolx_m7, bg1scroly_m7;
u2 mode7C, mode7D, mode7X0, mode7Y0;
u1 dmadata[129], hdmarestart, nohdmaframe, hdmadelay, SPC7110Enable;
u2 resolutn, curypos;
u1 NextLineCache, prevoamptr, oamlow, nexthprior, nosprincr, objhipr;
u4 objptr, objptrn;
u1 objsize1, objsize2, objmovs1, objmovs2;
u2 objadds1, objadds2, oamaddrs, poamaddrs;
/* The sprite tables reg2101w indexes; same contents as cpu/c_regswdata.c. */
u1 reg2101w_objsize1[8] = { 1, 1, 1, 4, 4, 16, 8, 8 };
u1 reg2101w_objsize2[8] = { 4, 16, 64, 16, 64, 64, 32, 16 };
u1 reg2101w_objmovs1[8] = { 2, 2, 2, 2, 2, 4, 2, 2 };
u1 reg2101w_objmovs2[8] = { 2, 4, 8, 4, 8, 8, 4, 4 };
u2 reg2101w_objadds1[8] = { 14, 14, 14, 14, 14, 12, 14, 14 };
u2 reg2101w_objadds2[8] = { 14, 12, 8, 12, 8, 8, 12, 12 };
/* reg2105w clears all four tile-size flags with one `mov dword[BG116x16t],0`,
   so they have to be a single 4-byte run. Loose scalars are contiguous at -O0
   and reordered at -O2, where that store reaches bgtilesz instead. */
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(bgmode) ".skip 1\n" ASM_GSYM(bg3highst) ".skip 1\n" ASM_GSYM(bgtilesz) ".skip 1\n" ASM_GSYM(mosaicon) ".skip 1\n" ASM_GSYM(mosaicsz) ".skip 1\n" ASM_SEC_END);
__asm__(ASM_SEC_BSS(".bss")
        ASM_GSYM(BG116x16t) ".skip 1\n" ASM_GSYM(BG216x16t) ".skip 1\n" ASM_GSYM(BG316x16t) ".skip 1\n" ASM_GSYM(BG416x16t) ".skip 1\n" ASM_SEC_END);
extern u1 bgmode, bg3highst, bgtilesz, mosaicon, mosaicsz;
extern u1 BG116x16t, BG216x16t, BG316x16t, BG416x16t;
u1 bg1scsize, bg2scsize, bg3scsize, bg4scsize;
u1 cgmod, winbg1en, winbg2en, winbg3en, winbg4en, winobjen, wincolen;
u1 coladdr, coladdg, coladdb, interlval;
u1 iohvlatch, MultiTapStat, JoyCRead;
u4 JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;
u4 JoyANow, JoyBNow, JoyCNow, JoyDNow, JoyENow;
u1 cycpl, cycphb, xirqb;
u4 cycpblt; /* u4 in init.h: a u1 here is a 3-byte overrun */
u1 opexec268, opexec268cph, opexec358, opexec358cph, cycpb268, cycpb358;
u2 HIRQLoc, VIRQLoc, totlines;
u4 HIRQCycNext;
u1 HIRQNextExe;
u4 vramaddr;
u1 vramread2, mode7set;
/* vram points at vrama in the emulator (ui.c); the asm reaches VRAM by both
   routes, so the test has to keep them the same object. Two bytes of slack:
   the $2119 handlers store at offset + 1. */
u1 vrama[0x10002];
static u1 vram_init[0x10002];
u1* vram = vrama;
u1 vidmemch2[4096], vidmemch4[4096], vidmemch8[4096];
u1 vramincby8left, vramincby8totl, vraminctype, vramincby8on, vramincr;
u1 vramincby8rowl;
u1 nssdip1, nssdip2, nssdip3, nssdip4, nssdip5, nssdip6;
u2 RumbleData;
u1 MultiTap, device2, hblank;
u4 nmistatus;
u2 vramincby8var, vramincby8ptri, addrincr;
static u1 vmc_init[4096];
static u1 dma_init[129];
/* The write table; the oracle's cpu/regsw.mac names it. */
void (*regptwa[0x3000])(void);
static u1 oam_init[1024];
static u2 cg_init[256];
/* The $2180 port walks a 128K window, so the buffer has to cover the wrap. */
static u1 wram_store[0x20000], wram_init[0x20000];
u1* wramdata = wram_store;

/* The APU I/O ports ($2140-$2143) and the $2137 latch. */
u1 spcon, spcnumread, sndrot, sndrot2;
u1 reg1read, reg2read, reg3read, reg4read;
u1 SPCRAM[0x10000];
u4 SPC700read, SPC700write, h_dot_counter, xa;
u4 nmirept, cycpbl, curexecstate;
/* The opcode tables reenablespc reloads edi from; only its address matters,
   and regs_call discards edi exactly as the real callers do. */
void* tableadc[256];
/* The seam the ported handlers take their arguments through and leave their
   results in; the oracle is assembly and still uses the registers, so run()
   sets both up and reads each side back its own way. MemSeamS is esi, which
   the $2140-$2143 sound-skip hack wants as the program counter. */
u4 MemSeamA, MemSeamB, MemSeamC, MemSeamD, MemSeamS;
/* The instruction stream the sound-skip hack patches. Eight bytes is two more
   than the longest scan plus the pair it writes. */
static u1 pc_store[8], pc_init[8];
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
DECL(reg2100w);
DECL(reg2101w);
DECL(reg2102w);
DECL(reg2103w);
DECL(reg2104w);
DECL(reg2105w);
DECL(reg2106w);
DECL(reg2107w);
DECL(reg2108w);
DECL(reg2109w);
DECL(reg210Aw);
DECL(reg210Bw);
DECL(reg210Cw);
DECL(reg2122w);
DECL(reg2123w);
DECL(reg2124w);
DECL(reg2125w);
DECL(reg2132w);
DECL(reg2133w);
DECL(reg2183w);
DECL(reg4201w);
DECL(reg4203w);
DECL(reg4206w);
DECL(reg420Dw);
DECL(reg4207w);
DECL(reg4208w);
DECL(reg4209w);
DECL(reg420Aw);
DECL(reg2116w);
DECL(reg2117w);
DECL(reg211Aw);
DECL(reg4016r);
DECL(reg4017r);
DECL(reg4212r);
DECL(reg4100r);
DECL(reg2139r);
DECL(reg213Ar);
DECL(reg4016w);
DECL(reg2115w);
DECL(reg2118);
DECL(reg2118inc);
DECL(reg2118inc8);
DECL(reg2118inc8inc);
DECL(reg2119);
DECL(reg2119inc);
DECL(reg2119inc8);
DECL(reg2119inc8inc);
DECL(reg2137r);
DECL(reg2140r);
DECL(reg2141r);
DECL(reg2142r);
DECL(reg2143r);
DECL(reg2140w);
DECL(reg2141w);
DECL(reg2142w);
DECL(reg2143w);
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
        "movl 44(%esp), %esi\n"
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
void regs_call(void* fn, u4 eax, u4 ecx, u4 edx, u4 ebx, u4 esi);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* 1 = the handler reads cx and indexes dmadata with it, so the address has
       to stay inside $4300..$4380 or both sides walk off the table. */
    int dma;
    /* 1 = the assembly clobbers al, so eax is not comparable. */
    int noax;
} regcase;

#define CASE(n) { #n, asm_##n, n, 0, 0 }
#define CASE_DMA(n) { #n, asm_##n, n, 1, 0 }
/* The assembly leaves its loaded value in al. The write ABI does not
   promise al - the header lists AH, ECX, ESI, EDI, *S and DX - so the
   trampoline preserving it is a widening, not a divergence. */
#define CASE_NOAX(n) { #n, asm_##n, n, 0, 1 }
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
    CASE(reg2100w),
    CASE(reg2101w),
    CASE(reg2102w),
    CASE(reg2103w),
    CASE(reg2104w),
    CASE(reg2105w),
    CASE(reg2106w),
    CASE(reg2107w),
    CASE(reg2108w),
    CASE(reg2109w),
    CASE(reg210Aw),
    CASE(reg210Bw),
    CASE(reg210Cw),
    CASE(reg2122w),
    CASE(reg2123w),
    CASE(reg2124w),
    CASE(reg2125w),
    CASE(reg2132w),
    CASE(reg2133w),
    CASE(reg2183w),
    CASE(reg4201w),
    CASE(reg4203w),
    CASE(reg4206w),
    CASE_NOAX(reg420Dw),
    CASE(reg4207w),
    CASE(reg4208w),
    CASE(reg4209w),
    CASE_NOAX(reg420Aw), /* `and al,01h` clobbers al, like reg420Dw */
    CASE(reg2116w),
    CASE(reg2117w),
    CASE(reg211Aw),
    CASE(reg4016r),
    CASE(reg4017r),
    CASE(reg4212r),
    CASE(reg4100r),
    CASE(reg2139r),
    CASE(reg213Ar),
    CASE(reg4016w),
    CASE(reg2115w),
    CASE(reg2118),
    CASE(reg2118inc),
    CASE(reg2118inc8),
    CASE(reg2118inc8inc),
    CASE(reg2119),
    CASE(reg2119inc),
    CASE(reg2119inc8),
    CASE(reg2119inc8inc),
    /* $2137 takes the cycle count in dh; the APU ports read the program
       counter out of esi, which regs_call points at pc_store. */
    CASE(reg2137r),
    CASE(reg2140r),
    CASE(reg2141r),
    CASE(reg2142r),
    CASE(reg2143r),
    CASE_NOAX(reg2140w),
    CASE_NOAX(reg2141w),
    CASE_NOAX(reg2142w),
    CASE_NOAX(reg2143w),
};
#undef CASE
#undef CASE_DMA
#undef CASE_NOAX

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
    u1 vbo, fbo;
    u1 cgm, win[7], cola[3], intl;
    u1 iohv, mtap, iop, spd[4];
    u1 jcr;
    u4 jnow[5];
    u2 hirql, virql;
    u4 vaddr;
    u1 vrd, vrd2, m7set;
    u1 vinct, vb8on, vincr, vb8l, vb8t, vb8r;
    u1 dip;
    u2 rumble;
    u1 hbl;
    u2 vb8v, vb8p, ainc;
    void (*tbl[2])(void);
    u1 vmc[3 * 4096];
    u1 vr[0x10002];
    u4 hirqc;
    u1 hirqx;
    u2 dvr, mr2;
    u2 res;
    u1 cgr[512];
    u1 nlc, poam, olow, nhp, nospr, ohipr;
    u4 optr[2];
    u1 objb[4];
    u2 obja[2];
    u2 oams[2];
    u1 modeb[9];
    u2 bgp[16], bgo[4];
    u4 bgxy[8];
    u1 bgsc[4];
    u1 oam[1024];
    u1 wram[0x20000];
    /* $2137 and the APU I/O ports. */
    u4 hdot, spcrd, spcwr, xa_, nmirept_, cycpbl_, cxs;
    u1 spcnr, srot, srot2, apu[4];
    u1 pc[8];
    /* The 16-bit H/V latches $2137 writes and $213C/$213D read out. */
    u2 lx16o, ly16o;
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
    u1 poam0, olow0, nhp0, nospr0, ohipr0, nlc0;
    u1 cgm0, win0, cola0, intl0;
    u1 iohv0, mtap0, spd0, cphb0, xirq0, cpblt0;
    u1 jcr0;
    u4 jorig[5], jnow0[5];
    u2 hirql0, virql0, totl0;
    u4 vaddr0;
    u1 vrd0, vrd20, m7set0;
    u1 vb8l, vb8t, vinct, vb8on, vincr, vb8r0;
    u1 dips[6];
    u2 rumble0;
    u1 mtap2, dev2;
    u4 nmist;
    u2 vb8v, vb8p, aincr;
    u4 hirqc0;
    u1 hirqx0;
    u1 ox268, ox268c, ox358, ox358c, cb268, cb358;
    u4 optr0;
    u2 oams0, poams0;
    u1 bgb0;
    u2 bgp0;
    u4 bgxy0;
    /* $2137 and the APU I/O ports. */
    u1 spcon0, spcnr0, srot0, srot20, apurd[4], apu0[4];
    u4 hdot0, spcrd0, spcwr0, xa0, nmirept0, cycpbl0, cxs0;
    u1 pc0[8];
} state;

static void run(void (*fn)(void), u4 a, u4 c, u4 d, state const* in,
    u2 m7a, u2 m7b, u1 const* mult, int seam, snapshot* out)
{
    u1 const vb = in->vb, fb = in->fb, mc = in->mc;

    memcpy(wram_store, wram_init, sizeof wram_store);
    memcpy(pc_store, in->pc0, sizeof pc_store);
    memcpy(pc_init, in->pc0, sizeof pc_init);
    MemSeamS = (u4)(uintptr_t)pc_store;
    spcon = in->spcon0;
    spcnumread = in->spcnr0;
    sndrot = in->srot0;
    sndrot2 = in->srot20;
    reg1read = in->apurd[0];
    reg2read = in->apurd[1];
    reg3read = in->apurd[2];
    reg4read = in->apurd[3];
    memcpy(SPCRAM + 0xF4, in->apu0, 4);
    h_dot_counter = in->hdot0;
    SPC700read = in->spcrd0;
    SPC700write = in->spcwr0;
    xa = in->xa0;
    nmirept = in->nmirept0;
    cycpbl = in->cycpbl0;
    curexecstate = in->cxs0;
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
    for (int i = 0; i < 4; i++) {
        bg1scrolx[i] = bg1scroly[i] = in->sc0;
    }
    bg1scrolx_m7 = bg1scroly_m7 = in->sc0;
    mode7C = mode7D = mode7X0 = mode7Y0 = in->sc0;
    memcpy(dmadata, dma_init, sizeof dmadata);
    hdmarestart = in->hres0;
    nohdmaframe = in->nohd0;
    hdmadelay = in->hdel0;
    SPC7110Enable = in->spc7110;
    resolutn = in->res0;
    curypos = in->cury0;
    memcpy(oamram, oam_init, sizeof oamram);
    cgmod = in->cgm0;
    iohvlatch = in->iohv0;
    memcpy(vrama, vram_init, sizeof vrama);
    vramaddr = in->vaddr0;
    memcpy(vidmemch2, vmc_init, 4096);
    memcpy(vidmemch4, vmc_init, 4096);
    memcpy(vidmemch8, vmc_init, 4096);
    vraminctype = in->vinct;
    vramincby8on = in->vb8on;
    vramincr = in->vincr;
    RumbleData = in->rumble0;
    MultiTap = in->mtap2;
    nmistatus = in->nmist;
    hblank = 0;
    device2 = in->dev2;
    nssdip1 = in->dips[0];
    nssdip2 = in->dips[1];
    nssdip3 = in->dips[2];
    nssdip4 = in->dips[3];
    nssdip5 = in->dips[4];
    nssdip6 = in->dips[5];
    vramincby8rowl = in->vb8r0;
    vramincby8left = in->vb8l;
    vramincby8totl = in->vb8t;
    vramincby8var = in->vb8v;
    vramincby8ptri = in->vb8p;
    addrincr = in->aincr;
    vramread = in->vrd0;
    vramread2 = in->vrd20;
    mode7set = in->m7set0;
    HIRQLoc = in->hirql0;
    VIRQLoc = in->virql0;
    HIRQCycNext = in->hirqc0;
    HIRQNextExe = in->hirqx0;
    totlines = in->totl0;
    MultiTapStat = in->mtap0;
    JoyCRead = in->jcr0;
    JoyAOrig = in->jorig[0];
    JoyBOrig = in->jorig[1];
    JoyCOrig = in->jorig[2];
    JoyDOrig = in->jorig[3];
    JoyEOrig = in->jorig[4];
    JoyANow = in->jnow0[0];
    JoyBNow = in->jnow0[1];
    JoyCNow = in->jnow0[2];
    JoyDNow = in->jnow0[3];
    JoyENow = in->jnow0[4];
    cycpl = in->spd0;
    cycphb = in->cphb0;
    xirqb = in->xirq0;
    cycpblt = in->cpblt0;
    opexec268 = in->ox268;
    opexec268cph = in->ox268c;
    opexec358 = in->ox358;
    opexec358cph = in->ox358c;
    cycpb268 = in->cb268;
    cycpb358 = in->cb358;
    winbg1en = winbg2en = winbg3en = winbg4en = in->win0;
    winobjen = wincolen = in->win0;
    coladdr = coladdg = coladdb = in->cola0;
    interlval = in->intl0;
    NextLineCache = in->nlc0;
    prevoamptr = in->poam0;
    oamlow = in->olow0;
    nexthprior = in->nhp0;
    nosprincr = in->nospr0;
    objhipr = in->ohipr0;
    objptr = objptrn = in->optr0;
    objsize1 = objsize2 = objmovs1 = objmovs2 = in->bgb0;
    objadds1 = objadds2 = in->bgp0;
    oamaddrs = in->oams0;
    poamaddrs = in->poams0;
    bgmode = bg3highst = bgtilesz = mosaicon = mosaicsz = in->bgb0;
    BG116x16t = BG216x16t = BG316x16t = BG416x16t = in->bgb0;
    for (int i = 0; i < 4; i++) {
        bg1ptr[i] = bg1ptrb[i] = bg1ptrc[i] = bg1ptrd[i] = in->bgp0;
        bg1ptrx[i] = bg1ptry[i] = in->bgxy0;
    }
    bg1scsize = bg2scsize = bg3scsize = bg4scsize = in->bgb0;
    for (int i = 0; i < 4; i++) {
        bg1objptr[i] = in->bgp0;
    }
    vidbright = vb;
    forceblnk = fb;
    multchange = mc;
    mode7A = m7a;
    mode7B = m7b;
    memcpy(compmult, mult, 3);

    MemSeamA = a;
    MemSeamB = in->ebxin;
    MemSeamC = c;
    MemSeamD = d;
    regs_call((void*)fn, a, c, d, in->ebxin, (u4)(uintptr_t)pc_store);

    /* A ported handler leaves everything in the seam; the oracle in the
       registers regs_call captured. */
    out->eax = seam ? MemSeamA : regs_out[0];
    out->ecx = seam ? MemSeamC : regs_out[1];
    out->edx = seam ? MemSeamD : regs_out[2];
    memcpy(out->mult, compmult, 3);
    out->change = multchange;
    out->lx = latchxr;
    out->ly = latchyr;
    out->lx16o = latchx;
    out->ly16o = latchy;
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
        u2 const sc[12] = { bg1scrolx[0], bg1scrolx[1], bg1scrolx[2],
            bg1scrolx[3], bg1scroly[0], bg1scroly[1], bg1scroly[2],
            bg1scroly[3], bg1scrolx_m7, bg1scroly_m7, 0, 0 };
        u2 const m7[6] = { mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0 };
        memcpy(out->sc, sc, sizeof out->sc);
        memcpy(out->m7, m7, sizeof out->m7);
    }
    memcpy(out->dma, dmadata, sizeof out->dma);
    out->hres = hdmarestart;
    out->nohd = nohdmaframe;
    out->hdel = hdmadelay;
    memcpy(out->wram, wram_store, sizeof out->wram);
    memcpy(out->pc, pc_store, sizeof out->pc);
    memcpy(out->apu, SPCRAM + 0xF4, 4);
    out->hdot = h_dot_counter;
    out->spcrd = SPC700read;
    out->spcwr = SPC700write;
    out->xa_ = xa;
    out->nmirept_ = nmirept;
    out->cycpbl_ = cycpbl;
    out->cxs = curexecstate;
    out->spcnr = spcnumread;
    out->srot = sndrot;
    out->srot2 = sndrot2;
    memcpy(out->oam, oamram, sizeof out->oam);
    out->vbo = vidbright;
    out->cgm = cgmod;
    out->iohv = iohvlatch;
    out->vaddr = vramaddr;
    out->vinct = vraminctype;
    out->vb8on = vramincby8on;
    out->vincr = vramincr;
    out->vb8r = vramincby8rowl;
    out->vb8l = vramincby8left;
    out->vb8t = vramincby8totl;
    out->vb8v = vramincby8var;
    out->vb8p = vramincby8ptri;
    out->ainc = addrincr;
    memcpy(out->tbl, regptwa + 0x118, sizeof out->tbl);
    memcpy(out->vmc, vidmemch2, 4096);
    memcpy(out->vmc + 4096, vidmemch4, 4096);
    memcpy(out->vmc + 8192, vidmemch8, 4096);
    memcpy(out->vr, vrama, sizeof out->vr);
    out->vrd = vramread;
    out->vrd2 = vramread2;
    out->m7set = mode7set;
    out->hirql = HIRQLoc;
    out->virql = VIRQLoc;
    out->hirqc = HIRQCycNext;
    out->hirqx = HIRQNextExe;
    out->iop = ioportval;
    out->mtap = MultiTapStat;
    out->jcr = JoyCRead;
    out->rumble = RumbleData;
    out->hbl = hblank;
    out->jnow[0] = JoyANow;
    out->jnow[1] = JoyBNow;
    out->jnow[2] = JoyCNow;
    out->jnow[3] = JoyDNow;
    out->jnow[4] = JoyENow;
    out->dvr = divres;
    out->mr2 = multres;
    {
        u1 const sp[4] = { cycpl, cycphb, xirqb, (u1)cycpblt };

        memcpy(out->spd, sp, sizeof out->spd);
    }
    out->intl = interlval;
    out->res = resolutn;
    memcpy(out->cgr, cgram, sizeof out->cgr);
    {
        u1 const w[7] = { winbg1en, winbg2en, winbg3en, winbg4en, winobjen,
            wincolen, 0 };
        u1 const c[3] = { coladdr, coladdg, coladdb };

        memcpy(out->win, w, sizeof out->win);
        memcpy(out->cola, c, sizeof out->cola);
    }
    out->fbo = forceblnk;
    out->nlc = NextLineCache;
    out->poam = prevoamptr;
    out->olow = oamlow;
    out->nhp = nexthprior;
    out->nospr = nosprincr;
    out->ohipr = objhipr;
    out->optr[0] = objptr;
    out->optr[1] = objptrn;
    {
        u1 const ob[4] = { objsize1, objsize2, objmovs1, objmovs2 };
        u2 const oa[2] = { objadds1, objadds2 };
        u2 const os[2] = { oamaddrs, poamaddrs };
        u1 const mb[9] = { bgmode, bg3highst, bgtilesz, mosaicon, mosaicsz,
            BG116x16t, BG216x16t, BG316x16t, BG416x16t };
        u2 const bp[16] = { bg1ptr[0], bg1ptr[1], bg1ptr[2], bg1ptr[3],
            bg1ptrb[0], bg1ptrb[1], bg1ptrb[2], bg1ptrb[3], bg1ptrc[0],
            bg1ptrc[1], bg1ptrc[2], bg1ptrc[3], bg1ptrd[0], bg1ptrd[1],
            bg1ptrd[2], bg1ptrd[3] };
        u4 const bxy[8] = { bg1ptrx[0], bg1ptrx[1], bg1ptrx[2], bg1ptrx[3],
            bg1ptry[0], bg1ptry[1], bg1ptry[2], bg1ptry[3] };
        u1 const bs[4] = { bg1scsize, bg2scsize, bg3scsize, bg4scsize };
        u2 const bo[4] = { bg1objptr[0], bg1objptr[1], bg1objptr[2],
            bg1objptr[3] };

        memcpy(out->objb, ob, sizeof out->objb);
        memcpy(out->obja, oa, sizeof out->obja);
        memcpy(out->oams, os, sizeof out->oams);
        memcpy(out->modeb, mb, sizeof out->modeb);
        memcpy(out->bgp, bp, sizeof out->bgp);
        memcpy(out->bgxy, bxy, sizeof out->bgxy);
        memcpy(out->bgsc, bs, sizeof out->bgsc);
        memcpy(out->bgo, bo, sizeof out->bgo);
    }
}

void c_reg2104w(u1 al);

/* The behaviour $2104 no longer shares with the assembly, checked directly:
   see the note in cpu/c_regsppu.c. */
static int check_oam_mirroring(void)
{
    int bad = 0;

    /* The high table repeats every 32 bytes: 0x220 folds to 0x200 and 0x230
       to 0x210 - the second distinguishes a 0x1f mask from a narrower one. */
    memset(oamram, 0, sizeof oamram);
    nosprincr = 0;
    oamaddr = 0x220u; /* one past the high table */
    c_reg2104w(0x5Au);
    if (oamram[0x200] != 0x5A) {
        printf("  oam mirroring: 0x220 wrote %02X at 0x200, wanted 5A\n",
            oamram[0x200]);
        bad = 1;
    }
    if (oamaddr != 0x221u) {
        printf("  oam mirroring: 0x220 left oamaddr %04X, wanted 0221\n",
            (unsigned)oamaddr);
        bad = 1;
    }
    oamaddr = 0x230u;
    c_reg2104w(0xA5u);
    if (oamram[0x210] != 0xA5) {
        printf("  oam mirroring: 0x230 wrote %02X at 0x210, wanted A5\n",
            oamram[0x210]);
        bad = 1;
    }
    /* And it really wraps rather than running on into 0x220+. */
    oamaddr = 0x3F0u;
    c_reg2104w(0x3Cu);
    if (oamram[0x210] != 0x3C || oamram[0x3F0] != 0) {
        printf("  oam mirroring: 0x3F0 did not fold back into the table\n");
        bad = 1;
    }

    /* Ten-bit wrap, not a reset to 1. */
    oamaddr = 0x3FFu;
    c_reg2104w(0x11u);
    if (oamaddr != 0) {
        printf("  oam mirroring: 0x3FF left oamaddr %04X, wanted 0000\n",
            (unsigned)oamaddr);
        bad = 1;
    }

    /* An even address loads the latch whichever table it selects. */
    oamlow = 0;
    oamaddr = 0x210u;
    c_reg2104w(0x77u);
    if (oamlow != 0x77u) {
        printf("  oam mirroring: even high write left oamlow %02X, wanted 77\n",
            oamlow);
        bad = 1;
    }
    return bad;
}

int main(void)
{
    dt_fill(wram_init, sizeof wram_init);
    dt_fill(oam_init, sizeof oam_init);
    dt_fill(cg_init, sizeof cg_init);
    dt_fill(dma_init, sizeof dma_init);
    dt_fill(vram_init, sizeof vram_init);
    dt_fill(vmc_init, sizeof vmc_init);
    DT_MAIN(20260730, 200000)
    {
        regcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot x, y;
        u4 a = dt_u32();
        u4 d = dt_u32();
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
        /* $4206 divides diva by al; the by-zero path needs hitting properly. */
        in.iop = (u1)(dt_mod(2) ? 0x80u | dt_u32() : dt_u32());
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
        if (k->c_fn == reg4206w && dt_mod(4) == 0) {
            a &= ~0xFFu; /* the divide-by-zero branch */
        }
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
        in.cgm0 = (u1)dt_u32();
        /* $4201 arms the counter latch on a bit-7 edge, so both the old latch
           state and the old port value have to straddle 1 and 0x80. */
        in.iohv0 = (u1)(dt_mod(2) ? 1 : dt_u32());
        /* $4207/$4208 only act when the value changes, and $4209/$420A only
           when the beam has left the V-IRQ line - so curypos and VIRQLoc have
           to collide often, and HIRQLoc has to match al often. */
        /* The high half of vramaddr is not the address and must survive; the
           low half wraps, so sit on the top of the range too. */
        in.vaddr0 = (dt_u32() & ~0xFFFFu)
            | (dt_mod(2) ? 0xFFFCu + dt_mod(4) : dt_u32() & 0xFFFFu);
        in.vrd0 = (u1)dt_u32();
        /* The inc8 address is (addr & left) << 3 plus two masked terms; the
           asm has no bound check, so keep the sum inside VRAM the way the
           emulator's own field split does. */
        in.vinct = (u1)dt_u32();
        /* Exactly 1 enables the remap path; uniform bytes reach it 1 in
           256, which is not enough to exercise the row wrap. */
        /* 0x72 in the high byte is the sentry that freezes the rotate. */
        /* The multitap path needs device2 == 0 and MultiTap == 1. */
        /* $4212 gates vblank on nmistatus == 2 at exactly resolutn. */
        in.nmist = dt_mod(2) ? 2u : dt_u32();
        in.mtap2 = (u1)(dt_mod(2) ? 1 : dt_u32());
        in.dev2 = (u1)(dt_mod(2) ? 0 : dt_u32());
        in.rumble0 = (u2)(dt_mod(2) ? 0x7200u | (dt_u32() & 0xFFu) : dt_u32());
        for (int j = 0; j < 6; j++) {
            in.dips[j] = (u1)(dt_mod(2) ? 1 : dt_u32());
        }
        in.vb8on = (u1)(dt_mod(2) ? 1 : dt_u32());
        in.vincr = (u1)dt_u32();
        /* Both counters gate a branch each time they hit zero, so keep
           them small enough to actually get there. */
        in.vb8r0 = (u1)(dt_mod(2) ? 1 : dt_u32());
        in.vb8l = (u1)(dt_mod(2) ? 1 : dt_u32() & 0x1Fu);
        in.vb8t = (u1)dt_u32();
        in.vb8v = (u2)(dt_u32() & 0x1FFFu);
        in.vb8p = (u2)(dt_u32() & 0x1FFFu);
        in.aincr = (u2)(dt_mod(2) ? 2 : dt_u32());
        in.vrd20 = (u1)dt_u32();
        in.m7set0 = (u1)dt_u32();
        in.hirql0 = (u2)(dt_mod(2) ? (dt_u32() & 0xFF00u) | (a & 0xFFu)
                                   : dt_u32());
        in.virql0 = (u2)dt_u32(); /* re-pointed at curypos below */
        in.hirqc0 = dt_u32();
        in.hirqx0 = (u1)(dt_mod(2) ? 1 : dt_u32());
        /* $420A parks VIRQLoc out of range at totlines - 1; straddle it. */
        in.totl0 = (u2)(dt_mod(2) ? 262 : dt_u32());
        in.mtap0 = (u1)(dt_mod(2) ? (u1)(dt_mod(2) ? 0x80u : 0u) : dt_u32());
        /* $4016 re-latches only when JoyCRead reaches 3, so bias it onto
           the 1/2 bits rather than leaving it uniform. */
        in.jcr0 = (u1)(dt_mod(2) ? dt_mod(4) : dt_u32());
        for (int j = 0; j < 5; j++) {
            in.jorig[j] = dt_u32();
            in.jnow0[j] = dt_u32();
        }
        /* Four separate values: driving them from one made cycpl and cycphb
           indistinguishable, and $4207's position maths reads only cycpl. */
        in.spd0 = (u1)dt_u32();
        in.cphb0 = (u1)dt_u32();
        in.xirq0 = (u1)dt_u32();
        in.cpblt0 = (u1)dt_u32();
        /* determine_hirq_exec branches on `dh > cycpl - pos`, and both sides
           are otherwise uniform bytes - a 1-in-256 collision the run would
           mostly miss. HIRQLoc of 0 makes pos 0, so left is cycpl; the compare
           is against dh *after* HIRQCycNext has been added to it, so back that
           out to land exactly on the boundary. */
        if (dt_mod(4) == 0) {
            u1 const dh = (u1)(in.spd0 - (u1)in.hirqc0);

            in.hirql0 = 0;
            d = (d & ~0xFF00u) | ((u4)dh << 8);
        }
        in.ox268 = (u1)dt_u32();
        in.ox268c = (u1)dt_u32();
        in.ox358 = (u1)dt_u32();
        in.ox358c = (u1)dt_u32();
        in.cb268 = (u1)dt_u32();
        in.cb358 = (u1)dt_u32();
        in.win0 = (u1)dt_u32();
        in.cola0 = (u1)dt_u32();
        in.intl0 = (u1)dt_u32();
        in.nlc0 = (u1)dt_u32();
        in.olow0 = (u1)dt_u32();
        in.ohipr0 = (u1)dt_u32();
        /* $2101 skips a repeated write unless the previous value was 0xFF. */
        in.poam0 = (u1)(dt_mod(3) == 0 ? 0xFFu : dt_mod(2) ? a
                                                           : dt_u32());
        /* Both of these gate a whole branch, so hit the live value often. */
        in.nhp0 = (u1)(dt_mod(2) ? 1 : dt_u32());
        in.nospr0 = (u1)(dt_mod(2) ? 1 : dt_u32());
        in.optr0 = dt_u32();
        /* $2103 keeps the previous address when the new one lands on 0x200 and
           the old one was past the sprite table. Reaching 0x200 needs
           oamaddrs & 0x1FE clear, which random values almost never are. */
        in.oams0 = (u2)(dt_mod(2) ? (dt_mod(8) << 9) | dt_mod(2) : dt_u32());
        in.poams0 = (u2)(dt_mod(2) ? 0x1FFu + dt_mod(4) : dt_u32());
        /* The APU ports split on spcon, and both halves matter: keep SPC
           emulation on half the time and off the other half. */
        in.spcon0 = (u1)(dt_mod(2) ? 0 : 1);
        in.spcnr0 = (u1)dt_u32();
        in.srot0 = (u1)dt_u32();
        /* sndrot2 wraps at 3, so bias it onto the values that reach the wrap. */
        in.srot20 = (u1)(dt_mod(2) ? dt_mod(4) : dt_u32());
        for (int j = 0; j < 4; j++) {
            in.apurd[j] = (u1)dt_u32();
            in.apu0[j] = (u1)dt_u32();
        }
        in.hdot0 = dt_u32();
        in.spcrd0 = dt_u32();
        in.spcwr0 = dt_u32();
        in.xa0 = dt_u32();
        in.nmirept0 = dt_u32();
        /* reenablespc only fires past 0x1000000; straddle the edge. */
        in.cycpbl0 = dt_mod(2) ? 0x1000000u + dt_mod(4) - 2u : dt_u32();
        in.cxs0 = dt_mod(2) ? (dt_u32() & ~0x02u) : dt_u32();
        /* The sound-skip hack looks for a BNE in the next few bytes, and
           $2140 first checks for the BPL that starts the other wait loop.
           Uniform bytes would hit either about once in fifty. */
        for (int j = 0; j < 8; j++) {
            in.pc0[j] = (u1)(dt_mod(3) == 0 ? 0xD0u : dt_u32());
        }
        if (dt_mod(3) == 0) {
            in.pc0[0] = 0x10;
            in.pc0[1] = 0xFB;
        }
        in.bgb0 = (u1)dt_u32();
        in.bgp0 = (u2)dt_u32();
        in.bgxy0 = dt_u32();
        /* $2104 indexes oamram with the whole dword, so the high half has to
           be clean here or both sides walk off the array.
           Known divergence: the port follows snes9x and bsnes above the low
           table - the high table repeats every 32 bytes, the address wraps at
           ten bits, and the write latch loads on any even address - where the
           assembly reset the address to 1 and dropped the byte once past 544.
           The two agree exactly below 0x200, so the comparison stays there and
           check_oam_mirroring() covers the rest. */
        if (k->c_fn == reg2104w) {
            in.oaddr &= 0x1FFu;
        }
        /* Same for the VRAM data ports: a dirty high half in vramaddr walks
           the inc8 offset straight off the end of the buffer. */
        if (k->c_fn == reg2118 || k->c_fn == reg2118inc
            || k->c_fn == reg2118inc8 || k->c_fn == reg2118inc8inc
            || k->c_fn == reg2119 || k->c_fn == reg2119inc
            || k->c_fn == reg2119inc8 || k->c_fn == reg2119inc8inc) {
            in.vaddr0 &= 0xFFFEu;
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
        /* $4207/$4208 only recompute while the beam sits on the V-IRQ line, and
           $4209/$420A only cancel once it has left. curypos is not known until
           here, so pair them up now rather than earlier with a stale value. */
        if (dt_mod(2)) {
            in.virql0 = in.cury0;
        }
        /* $420A parks VIRQLoc when it reaches totlines - 1. Both sides are
           otherwise independent, so pin totlines to the value that puts the
           write exactly on that edge - VIRQLoc keeps its low byte and takes
           bit 8 from al. */
        if (dt_mod(4) == 0) {
            in.totl0 = (u2)(((in.virql0 & 0x00FFu) | ((a & 1u) << 8)) + 1u);
        }

        run(k->asm_fn, a, c, d, &in, m7a, m7b, mult, 0, &x);
        run(k->c_fn, a, c, d, &in, m7a, m7b, mult, 1, &y);

        if (!k->noax) {
            DT_EQ(k->name, x.eax, y.eax);
        }
        DT_EQ("ecx", x.ecx, y.ecx);
        DT_EQ("edx", x.edx, y.edx);
        DT_MEM("compmult", x.mult, y.mult, sizeof x.mult);
        DT_EQ("multchange", x.change, y.change);
        DT_EQ("latchxr", x.lx, y.lx);
        DT_EQ("latchyr", x.ly, y.ly);
        DT_EQ("latchx", x.lx16o, y.lx16o);
        DT_EQ("latchy", x.ly16o, y.ly16o);
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
        DT_MEM("instruction stream", x.pc, y.pc, sizeof x.pc);
        DT_MEM("SPCRAM F4-F7", x.apu, y.apu, sizeof x.apu);
        DT_EQ("h_dot_counter", x.hdot, y.hdot);
        DT_EQ("SPC700read", x.spcrd, y.spcrd);
        DT_EQ("SPC700write", x.spcwr, y.spcwr);
        DT_EQ("xa", x.xa_, y.xa_);
        DT_EQ("nmirept", x.nmirept_, y.nmirept_);
        DT_EQ("cycpbl", x.cycpbl_, y.cycpbl_);
        DT_EQ("curexecstate", x.cxs, y.cxs);
        DT_EQ("spcnumread", x.spcnr, y.spcnr);
        DT_EQ("sndrot", x.srot, y.srot);
        DT_EQ("sndrot2", x.srot2, y.srot2);
        DT_MEM("oamram", x.oam, y.oam, sizeof x.oam);
        DT_EQ("vidbright", x.vbo, y.vbo);
        DT_EQ("cgmod", x.cgm, y.cgm);
        DT_EQ("iohvlatch", x.iohv, y.iohv);
        DT_EQ("vramaddr", x.vaddr, y.vaddr);
        DT_EQ("vraminctype", x.vinct, y.vinct);
        DT_EQ("vramincby8on", x.vb8on, y.vb8on);
        DT_EQ("vramincr", x.vincr, y.vincr);
        DT_EQ("vramincby8left", x.vb8l, y.vb8l);
        DT_EQ("vramincby8rowl", x.vb8r, y.vb8r);
        DT_EQ("vramincby8totl", x.vb8t, y.vb8t);
        DT_EQ("vramincby8var", x.vb8v, y.vb8v);
        DT_EQ("vramincby8ptri", x.vb8p, y.vb8p);
        DT_EQ("addrincr", x.ainc, y.ainc);
        DT_MEM("regptw 2118/2119", x.tbl, y.tbl, sizeof x.tbl);
        DT_MEM("vidmemch", x.vmc, y.vmc, sizeof x.vmc);
        DT_MEM("vrama", x.vr, y.vr, sizeof x.vr);
        DT_EQ("vramread", x.vrd, y.vrd);
        DT_EQ("vramread2", x.vrd2, y.vrd2);
        DT_EQ("mode7set", x.m7set, y.m7set);
        DT_EQ("HIRQLoc", x.hirql, y.hirql);
        DT_EQ("VIRQLoc", x.virql, y.virql);
        DT_EQ("HIRQCycNext", x.hirqc, y.hirqc);
        DT_EQ("HIRQNextExe", x.hirqx, y.hirqx);
        DT_EQ("ioportval", x.iop, y.iop);
        DT_EQ("MultiTapStat", x.mtap, y.mtap);
        DT_EQ("JoyCRead", x.jcr, y.jcr);
        DT_EQ("RumbleData", x.rumble, y.rumble);
        DT_EQ("hblank", x.hbl, y.hbl);
        DT_MEM("Joy?Now", x.jnow, y.jnow, sizeof x.jnow);
        DT_EQ("divres", x.dvr, y.dvr);
        DT_EQ("multres", x.mr2, y.mr2);
        DT_MEM("cycle speed", x.spd, y.spd, sizeof x.spd);
        DT_MEM("cgram", x.cgr, y.cgr, sizeof x.cgr);
        DT_MEM("window selects", x.win, y.win, sizeof x.win);
        DT_MEM("fixed colour", x.cola, y.cola, sizeof x.cola);
        DT_EQ("interlval", x.intl, y.intl);
        DT_EQ("resolutn", x.res, y.res);
        DT_EQ("forceblnk", x.fbo, y.fbo);
        DT_EQ("NextLineCache", x.nlc, y.nlc);
        DT_EQ("prevoamptr", x.poam, y.poam);
        DT_EQ("oamlow", x.olow, y.olow);
        DT_EQ("nexthprior", x.nhp, y.nhp);
        DT_EQ("nosprincr", x.nospr, y.nospr);
        DT_EQ("objhipr", x.ohipr, y.ohipr);
        DT_MEM("objptr/objptrn", x.optr, y.optr, sizeof x.optr);
        DT_MEM("sprite sizes", x.objb, y.objb, sizeof x.objb);
        DT_MEM("sprite adds", x.obja, y.obja, sizeof x.obja);
        DT_MEM("oamaddrs", x.oams, y.oams, sizeof x.oams);
        DT_MEM("mode/mosaic", x.modeb, y.modeb, sizeof x.modeb);
        DT_MEM("BG tilemap ptrs", x.bgp, y.bgp, sizeof x.bgp);
        DT_MEM("BG tilemap offsets", x.bgxy, y.bgxy, sizeof x.bgxy);
        DT_MEM("BG screen sizes", x.bgsc, y.bgsc, sizeof x.bgsc);
        DT_MEM("BG char ptrs", x.bgo, y.bgo, sizeof x.bgo);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s eax=%x mult=%d A=%04x B=%04x\n", k->name, a, mc, m7a, m7b);
        }
    }
    if (check_oam_mirroring()) {
        printf("PPU register reads and writes: FAIL ($2104 OAM mirroring)\n");
        return 1;
    }
    DT_DONE("PPU register reads and writes");
}
