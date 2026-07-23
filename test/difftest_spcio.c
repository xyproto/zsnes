/*
 * difftest_spcio.c - proves the SPC700 I/O register handlers in
 * cpu/spc_ioregs.h bit-identical to their assembly (the SPCRegF0..FF /
 * RSPCRegF0..FF routines in cpu/spc700.asm). Run with `make spcio`.
 * Transient: remove once the handler asm is deleted.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef uint32_t u4;

/* SPC RAM is 0xFFC0 of RAM followed by the 64-byte IPL ROM window; SPCRegF1
 * writes $FFC0..$FFFF, so give it a full 64 KiB (+ slack) here. */
u1 SPCRAM[0x10010];
u1 DSPMem[256];
u1 SPCROM[64];
u1 spcextraram[64];
u1 disablespcclr, SPCSkipXtraROM;
u1 reg1read, reg2read, reg3read, reg4read;
u4 spc700read;
u1 timeron, timincr0, timincr1, timincr2, timinl0, timinl1, timinl2;
u1 spcnumread;

/* Stand-in DSP write handler (register ABI: al=value, ebx=reg) that both the
 * asm SPCRegF3 (call [dspWptr+ebx*4]) and the C port call. It records the byte
 * and preserves al, so both sides agree on state and on the $F3 write-back. */
u1 dspstub[128];
extern void dspw_stub(void);
__asm__(
    ".text\n"
    ".globl dspw_stub\n"
    "dspw_stub:\n"
    "  movb %al, dspstub(%ebx)\n"
    "  ret\n");
void* dspWptr[128];

#include "../cpu/spc_ioregs.h"

extern void asm_SPCRegF0(void); /* the 16 write + 16 read handlers, renamed */
typedef void spcfn(void);
#define AW(n) extern void asm_SPCReg##n(void);
#define AR(n) extern void asm_RSPCReg##n(void);
AW(F0)
AW(F1)
AW(F2)
AW(F3)
AW(F4) AW(F5) AW(F6) AW(F7)
    AW(F8) AW(F9) AW(FA) AW(FB) AW(FC) AW(FD) AW(FE) AW(FF)
        AR(F0) AR(F1) AR(F2) AR(F3) AR(F4) AR(F5) AR(F6) AR(F7)
            AR(F8) AR(F9) AR(FA) AR(FB) AR(FC) AR(FD) AR(FE) AR(FF)

    /* Call an asm handler with the register ABI (al=value, ebx=reg); returns al. */
    static u1 call_asm(spcfn* fn, u4 reg, u1 al)
{
    u4 eax = al, ebx = reg;
    __asm__ volatile("call *%[fn]"
        : "+a"(eax), "+b"(ebx)
        : [fn] "r"(fn)
        : "ecx", "edx", "cc", "memory");
    return (u1)eax;
}

/* Everything a handler may touch, for save/restore and compare. */
struct state {
    u1 spcram[0x10010];
    u1 dspmem[256], dsp[128];
    u1 r1, r2, r3, r4, ton, ti0, ti1, ti2, tl0, tl1, tl2, nr;
    u4 s7r;
};
static void snapshot(struct state* s)
{
    memcpy(s->spcram, SPCRAM, sizeof s->spcram);
    memcpy(s->dspmem, DSPMem, sizeof s->dspmem);
    memcpy(s->dsp, dspstub, sizeof s->dsp);
    s->r1 = reg1read;
    s->r2 = reg2read;
    s->r3 = reg3read;
    s->r4 = reg4read;
    s->ton = timeron;
    s->ti0 = timincr0;
    s->ti1 = timincr1;
    s->ti2 = timincr2;
    s->tl0 = timinl0;
    s->tl1 = timinl1;
    s->tl2 = timinl2;
    s->nr = spcnumread;
    s->s7r = spc700read;
}
static void compare(const struct state* a, const struct state* c)
{
    DT_MEM("SPCRAM", a->spcram, c->spcram, sizeof a->spcram);
    DT_MEM("DSPMem", a->dspmem, c->dspmem, sizeof a->dspmem);
    DT_MEM("dspstub", a->dsp, c->dsp, sizeof a->dsp);
    DT_EQ("reg1read", a->r1, c->r1);
    DT_EQ("reg2read", a->r2, c->r2);
    DT_EQ("reg3read", a->r3, c->r3);
    DT_EQ("reg4read", a->r4, c->r4);
    DT_EQ("timeron", a->ton, c->ton);
    DT_EQ("timincr0", a->ti0, c->ti0);
    DT_EQ("timincr1", a->ti1, c->ti1);
    DT_EQ("timincr2", a->ti2, c->ti2);
    DT_EQ("timinl0", a->tl0, c->tl0);
    DT_EQ("timinl1", a->tl1, c->tl1);
    DT_EQ("timinl2", a->tl2, c->tl2);
    DT_EQ("spcnumread", a->nr, c->nr);
    DT_EQ("spc700read", a->s7r, c->s7r);
}

static spcfn* const WRITE[16] = {
    asm_SPCRegF0, asm_SPCRegF1, asm_SPCRegF2, asm_SPCRegF3, asm_SPCRegF4,
    asm_SPCRegF5, asm_SPCRegF6, asm_SPCRegF7, asm_SPCRegF8, asm_SPCRegF9,
    asm_SPCRegFA, asm_SPCRegFB, asm_SPCRegFC, asm_SPCRegFD, asm_SPCRegFE,
    asm_SPCRegFF
};
static spcfn* const READ[16] = {
    asm_RSPCRegF0, asm_RSPCRegF1, asm_RSPCRegF2, asm_RSPCRegF3, asm_RSPCRegF4,
    asm_RSPCRegF5, asm_RSPCRegF6, asm_RSPCRegF7, asm_RSPCRegF8, asm_RSPCRegF9,
    asm_RSPCRegFA, asm_RSPCRegFB, asm_RSPCRegFC, asm_RSPCRegFD, asm_RSPCRegFE,
    asm_RSPCRegFF
};

/* Randomize all handler-visible state; keep an identical copy to replay. */
static void randomize(struct state* saved)
{
    dt_fill(SPCRAM, sizeof(u1[0x10010]));
    dt_fill(DSPMem, 256);
    dt_fill(SPCROM, 64);
    dt_fill(spcextraram, 64);
    memset(dspstub, 0, sizeof dspstub);
    disablespcclr = (u1)(rand() & 1);
    SPCSkipXtraROM = (u1)(rand() & 1);
    reg1read = (u1)rand();
    reg2read = (u1)rand();
    reg3read = (u1)rand();
    reg4read = (u1)rand();
    spc700read = dt_u32();
    timeron = (u1)rand();
    timincr0 = (u1)rand();
    timincr1 = (u1)rand();
    timincr2 = (u1)rand();
    timinl0 = (u1)rand();
    timinl1 = (u1)rand();
    timinl2 = (u1)rand();
    spcnumread = (u1)rand();
    for (u4 i = 0; i < 128; i++)
        dspWptr[i] = (void*)dspw_stub;
    snapshot(saved);
}
static void restore(const struct state* s)
{
    memcpy(SPCRAM, s->spcram, sizeof s->spcram);
    memcpy(DSPMem, s->dspmem, sizeof s->dspmem);
    memcpy(dspstub, s->dsp, sizeof s->dsp);
    reg1read = s->r1;
    reg2read = s->r2;
    reg3read = s->r3;
    reg4read = s->r4;
    timeron = s->ton;
    timincr0 = s->ti0;
    timincr1 = s->ti1;
    timincr2 = s->ti2;
    timinl0 = s->tl0;
    timinl1 = s->tl1;
    timinl2 = s->tl2;
    spcnumread = s->nr;
    spc700read = s->s7r;
}

int main(void)
{
    int total = 0;

    /* writes */
    for (u4 r = 0; r < 16; r++) {
        DT_MAIN(700u + r, 40000)
        {
            struct state saved, A, C;
            randomize(&saved);
            u1 val = (u1)rand();
            u4 reg = 0xF0 + r;

            call_asm(WRITE[r], reg, val);
            snapshot(&A);

            restore(&saved);
            spc_write_reg(reg, val);
            snapshot(&C);

            compare(&A, &C);
        }
        printf("  W $%02lX  %s (%d/%ld)\n", (unsigned long)(0xF0 + r),
            dt_fails ? "FAIL" : "PASS", dt_fails, dt_iters);
        total += dt_fails;
    }

    /* reads */
    for (u4 r = 0; r < 16; r++) {
        DT_MAIN(900u + r, 40000)
        {
            struct state saved, A, C;
            randomize(&saved);
            u4 reg = 0xF0 + r;

            u1 a_al = call_asm(READ[r], reg, 0);
            snapshot(&A);

            restore(&saved);
            u1 c_al = spc_read_reg(reg);
            snapshot(&C);

            DT_EQ("al", a_al, c_al);
            compare(&A, &C);
        }
        printf("  R $%02lX  %s (%d/%ld)\n", (unsigned long)(0xF0 + r),
            dt_fails ? "FAIL" : "PASS", dt_fails, dt_iters);
        total += dt_fails;
    }

    printf(total ? "spcio: FAIL\n" : "spcio: PASS (all 32 handlers bit-identical to asm)\n");
    return total ? 1 : 0;
}
