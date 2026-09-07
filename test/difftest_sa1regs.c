/*
 * difftest_sa1regs.c - SA-1 register handlers (chips/sa1regs.c) against the
 * assembly they were ported from.
 *
 * Both sides act on the same state block, which chips/sa1regs.c owns, so an
 * iteration is: randomise the block, run the oracle, snapshot, restore, run the
 * C handler, snapshot, compare.
 *
 * EDX is compared, not just the state. The two defects this file was written
 * for were both register side effects the C calling convention dropped:
 * IRamWrite2's `xor dh,dh` at $3000 and $2302's `add al,dh`. A state-only
 * comparison passes straight through either.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmdata.h"
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* Owned by chips/sa1regs.c (its .data.sa1state block). */
extern u4 SA1Mode, SA1Control, SA1BankPtr, SA1ResetV, SA1NMIV, SA1IRQV;
extern u4 SA1IRQEn, SA1Message, SA1IRQExec, SA1IRQEnable, SA1DoIRQ;
extern u4 SA1ARC, SA1AR1, SA1AR2, SA1ARR1, SA1ARR2, SNSNMIV, SNSIRQV;
extern u4 SA1DMACount, SA1DMAInfo, SA1DMAChar, SA1DMASource, SA1DMADest;
extern u4 BWShift, BWAndAddr, BWAnd, BWRAnd, SA1_in_cc1_dma, SA1_CC2_line;
extern u4 SA1xpb, SA1xs, SA1RegPCS, SA1BWPtr, SA1Ptr, SA1Overflow;
extern u4 VarLenAddr, VarLenAddrB, VarLenBarrel;
extern u4 SA1TimerVal, SA1TimerSet, SA1TimerCount, SA1IRQData, SNSBWPtr;
extern u4 CurBWPtr, PHnum2writesa1reg;
extern u1 IRAM[2049];
extern u1 SA1_BRF[16];
extern u1* SA1RAMArea;

/* Referenced by the handlers but living elsewhere in the emulator. */
u1 CurrentExecSA1, SA1Status, BWUsed2, debstop3;
u2 curypos, irqv, irqv2, nmiv, nmiv2;
u1* romdata;
u4 NumofBanks;
u1 AddrNoIncr, SDD1BankA[4], debuggeron;
u1* snesmmap[256];
u1* snesmap2[256];
void* memtabler8[256];
uintptr_t MemSeamA, MemSeamB, MemSeamC, MemSeamD;
void GetTime(void) { }
void GetDate(void) { }

/* The handlers reach these; both sides call the same body, so a shared stub
   keeps the comparison honest. */
static long stub_calls;
void SA1_DMA_CC2(void) { stub_calls++; }
void sa1dmairam(void) { stub_calls++; }
void sa1dmabwram(void) { stub_calls++; }
void UpdateArithStuff(void) { stub_calls++; }
void memaccessbankr8sdd1(void) { stub_calls++; }

/* Backing store: BW-RAM and a ROM image the bank handlers index. */
static u1 bw_store[0x40000];
static u1 rom_store[0x400000];

void asm_IRamRead(void);
void asm_IRamWrite(void);
void asm_IRamWrite2(void);
#define D(n) void asm_##n(void);
#include "_sa1regs_decls.h"
#undef D

/* Same trampoline shape as difftest_regs.c: load the registers the assembly
   expects, call, and capture what comes back. */
u4 sregs_out[4];
__asm__(".text\n"
        ".globl sregs_call\n"
        "sregs_call:\n"
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
        "movl %eax, sregs_out\n"
        "movl %ecx, sregs_out+4\n"
        "movl %edx, sregs_out+8\n"
        "movl %ebx, sregs_out+12\n"
        "addl $4, %esp\n"
        "popl %ebp\n"
        "popl %edi\n"
        "popl %esi\n"
        "popl %ebx\n"
        "ret\n"
        ".text\n");
void sregs_call(void* fn, u4 eax, u4 ecx, u4 edx);

/* The C entry points are the REGABI thunks, which take their operands from the
   seam rather than registers. */
void IRamRead(void);
void IRamWrite(void);
void IRamWrite2(void);
#define D(n) void n(void);
#include "_sa1regs_decls.h"
#undef D

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* 1 = reads ecx as the $3000..$37FF address of an I-RAM access. */
    int iram;
} sa1case;

#define CASE(n) { #n, asm_##n, n, 0 }
#define CASE_IRAM(n) { #n, asm_##n, n, 1 }
static sa1case const cases[] = {
    CASE_IRAM(IRamRead), CASE_IRAM(IRamWrite), CASE_IRAM(IRamWrite2),
#define D(n) CASE(n),
#include "_sa1regs_decls.h"
#undef D
};

/* The state the handlers may touch. Listing the fields rather than memcpy-ing
   the whole .data.sa1state block keeps this inside one object per copy (the
   block is contiguous only by the assembly's construction, not by C) and lets a
   mismatch name the register. */
#define STATE_FIELDS(F)                                                   \
    F(SA1Mode) F(SA1Control) F(SA1BankPtr) F(SA1ResetV) F(SA1NMIV)        \
    F(SA1IRQV) F(SA1IRQEn) F(SA1Message) F(SA1IRQExec) F(SA1IRQEnable)    \
    F(SA1DoIRQ) F(SA1ARC) F(SA1AR1) F(SA1AR2) F(SA1ARR1) F(SA1ARR2)       \
    F(SNSNMIV) F(SNSIRQV) F(SA1DMACount) F(SA1DMAInfo) F(SA1DMAChar)      \
    F(SA1DMASource) F(SA1DMADest) F(BWShift) F(BWAndAddr) F(BWAnd)        \
    F(BWRAnd) F(SA1_in_cc1_dma) F(SA1_CC2_line) F(SA1xpb) F(SA1xs)        \
    F(SA1RegPCS) F(SA1BWPtr) F(SA1Ptr) F(SA1Overflow) F(VarLenAddr)       \
    F(VarLenAddrB) F(VarLenBarrel) F(SA1TimerVal) F(SA1TimerSet)          \
    F(SA1TimerCount) F(SA1IRQData) F(SNSBWPtr) F(CurBWPtr)

static struct {
    char const* name;
    u4* p;
} const fields[] = {
#define F(n) { #n, &n },
    STATE_FIELDS(F)
#undef F
};
#define NFIELDS (sizeof fields / sizeof fields[0])

typedef struct {
    u4 eax, ecx, edx;
    u4 v[NFIELDS];
    u1 iram[2049];
    u1 brf[16];
    u2 irqv, nmiv;
    long stubs;
} snapshot;

static u4 fld_save[NFIELDS];
static u1 iram_save[2049];
static u1 brf_save[16];
static u1 bw_save[0x40000];
static u2 irqv_save, nmiv_save;

static void state_save(void)
{
    for (size_t i = 0; i < NFIELDS; i++)
        fld_save[i] = *fields[i].p;
    memcpy(iram_save, IRAM, sizeof iram_save);
    memcpy(brf_save, SA1_BRF, sizeof brf_save);
    memcpy(bw_save, bw_store, sizeof bw_save);
    irqv_save = irqv;
    nmiv_save = nmiv;
}

static void state_restore(void)
{
    for (size_t i = 0; i < NFIELDS; i++)
        *fields[i].p = fld_save[i];
    memcpy(IRAM, iram_save, sizeof iram_save);
    memcpy(SA1_BRF, brf_save, sizeof brf_save);
    memcpy(bw_store, bw_save, sizeof bw_save);
    irqv = irqv_save;
    nmiv = nmiv_save;
}

static void snap(snapshot* s)
{
    for (size_t i = 0; i < NFIELDS; i++)
        s->v[i] = *fields[i].p;
    memcpy(s->iram, IRAM, sizeof s->iram);
    memcpy(s->brf, SA1_BRF, sizeof s->brf);
    s->irqv = irqv;
    s->nmiv = nmiv;
    s->stubs = stub_calls;
}

int main(void)
{
    size_t const ncases = sizeof cases / sizeof cases[0];

    SA1RAMArea = bw_store;
    romdata = rom_store;
    dt_fill(rom_store, sizeof rom_store);

    DT_MAIN(20260907u, 20000)
    {
        sa1case const* k = &cases[dt_mod((u4)ncases)];
        snapshot x, y;
        u4 eax, ecx, edx;

        /* Randomise the whole block, then pin the fields that have to stay
           sane or both sides walk off a table rather than diverging. */
        for (size_t i = 0; i < NFIELDS; i++)
            *fields[i].p = dt_u32();
        dt_fill(IRAM, sizeof iram_save);
        dt_fill(SA1_BRF, sizeof brf_save);
        dt_fill(bw_store, sizeof bw_store);
        SA1RAMArea = bw_store;
        romdata = rom_store;
        SA1BWPtr = (u4)(uintptr_t)bw_store;
        SNSBWPtr = (u4)(uintptr_t)bw_store;
        CurBWPtr = (u4)(uintptr_t)bw_store;
        SA1Ptr = (u4)(uintptr_t)rom_store;
        SA1RegPCS = (u4)(uintptr_t)rom_store;
        SA1BankPtr = (u4)(uintptr_t)rom_store;
        /* The varlen readers index snesmmap/snesmap2 by bank; keep them out of
           this test rather than fake a memory map. */
        VarLenAddr &= 0xFFFFu;
        VarLenAddrB &= 0xFFFFu;
        CurrentExecSA1 = (u1)rand();
        SA1Status = (u1)(rand() & 1);
        BWUsed2 = (u1)rand();
        curypos = (u2)rand();
        irqv2 = (u2)rand();
        nmiv2 = (u2)rand();

        eax = dt_u32();
        edx = dt_u32();
        /* The dispatcher passes the register address in ecx, and the handlers
           that share one body index off it (the whole $2240..$224E bitmap file
           does). The name encodes the address, so drive it exactly as the real
           caller would; I-RAM takes an address inside $3000..$37FF. */
        if (k->iram) {
            ecx = 0x3000u + dt_mod(0x800u);
        } else {
            unsigned addr;

            ecx = (sscanf(k->name, "sa1%4x", &addr) == 1) ? addr : dt_u32();
        }

        state_save();
        stub_calls = 0;
        if (getenv("DT_TRACE")) { printf("it=%ld %s\n", dt_it, k->name); fflush(stdout); }
        sregs_call((void*)k->asm_fn, eax, ecx, edx);
        x.eax = sregs_out[0];
        x.ecx = sregs_out[1];
        x.edx = sregs_out[2];
        snap(&x);

        state_restore();
        stub_calls = 0;
        MemSeamA = eax;
        MemSeamC = ecx;
        MemSeamD = edx;
        MemSeamB = 0;
        k->c_fn();
        y.eax = (u4)MemSeamA;
        y.ecx = (u4)MemSeamC;
        y.edx = (u4)MemSeamD;
        snap(&y);

        /* Reads return in al; the rest of eax is the caller's and not promised. */
        DT_EQ(k->name, x.eax & 0xFFu, y.eax & 0xFFu);
        /* DH is the scanline cycle count. This is the comparison that catches
           a handler which forgot to charge or clear it. */
        DT_EQ("edx", x.edx, y.edx);
        DT_EQ("stub calls", x.stubs, y.stubs);
        for (size_t i = 0; i < NFIELDS; i++)
            DT_EQ(fields[i].name, x.v[i], y.v[i]);
        DT_EQ("irqv", x.irqv, y.irqv);
        DT_EQ("nmiv", x.nmiv, y.nmiv);
        if (memcmp(x.iram, y.iram, sizeof x.iram)) {
            for (size_t i = 0; i < sizeof x.iram; i++)
                if (x.iram[i] != y.iram[i]) {
                    char lbl[64];
                    snprintf(lbl, sizeof lbl, "%s IRAM[%zu]", k->name, i);
                    DT_EQ(lbl, x.iram[i], y.iram[i]);
                    break;
                }
        }
        DT_EQ("BRF", memcmp(x.brf, y.brf, sizeof x.brf), 0);
    }
    DT_DONE("sa1regs");
}
