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
extern u1 RTCData[16];
extern u4 RTCPtr, RTCPtr2, RTCRest;
extern u4 Sdd1Mode, Sdd1Bank, Sdd1Addr, Sdd1NewAddr;
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
/* Every bank needs 64K of addressable space plus the 4 bytes $230C/$230D read
   at the end of it, or a random VarLenAddr walks off the map. */
#define MAPSLOT 0x10004u
static u1 map_store[MAPSLOT * 8];

static void map_init(void)
{
    for (int i = 0; i < 256; i++) {
        snesmmap[i] = map_store + MAPSLOT * (unsigned)(i & 7);
        snesmap2[i] = map_store + MAPSLOT * (unsigned)((i + 3) & 7);
    }
}

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
    CASE_IRAM(IRamRead),
    CASE_IRAM(IRamWrite),
    CASE_IRAM(IRamWrite2),
#define D(n) CASE(n),
#include "_sa1regs_decls.h"
#undef D
};

/* The state the handlers may touch. Listing the fields rather than memcpy-ing
   the whole .data.sa1state block keeps this inside one object per copy (the
   block is contiguous only by the assembly's construction, not by C) and lets a
   mismatch name the register. */
#define STATE_FIELDS(F)                                                                                 \
    F(SA1Mode)                                                                                          \
    F(SA1Control)                                                                                       \
    F(SA1BankPtr)                                                                                       \
    F(SA1ResetV)                                                                                        \
    F(SA1NMIV)                                                                                          \
    F(SA1IRQV)                                                                                          \
    F(SA1IRQEn) F(SA1Message) F(SA1IRQExec) F(SA1IRQEnable)                                             \
        F(SA1DoIRQ) F(SA1ARC) F(SA1AR1) F(SA1AR2) F(SA1ARR1) F(SA1ARR2)                                 \
            F(SNSNMIV) F(SNSIRQV) F(SA1DMACount) F(SA1DMAInfo) F(SA1DMAChar)                            \
                F(SA1DMASource) F(SA1DMADest) F(BWShift) F(BWAndAddr) F(BWAnd)                          \
                    F(BWRAnd) F(SA1_in_cc1_dma) F(SA1_CC2_line) F(SA1xpb) F(SA1xs)                      \
                        F(SA1RegPCS) F(SA1BWPtr) F(SA1Ptr) F(SA1Overflow) F(VarLenAddr)                 \
                            F(VarLenAddrB) F(VarLenBarrel) F(SA1TimerVal) F(SA1TimerSet)                \
                                F(SA1TimerCount) F(SA1IRQData) F(SNSBWPtr) F(CurBWPtr)                  \
                                    F(RTCPtr) F(RTCPtr2) F(RTCRest) F(Sdd1Mode) F(Sdd1Bank) F(Sdd1Addr) \
                                        F(Sdd1NewAddr)

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
    u1* mmap[256];
    u1* map2[256];
    u1 rtc[16];
    u1 sdd1[4];
    void* mt8[256];
    long stubs;
} snapshot;

static u4 fld_save[NFIELDS];
static u1 iram_save[2049];
static u1 brf_save[16];
static u1 bw_save[0x40000];
static u2 irqv_save, nmiv_save;
static u1* mmap_save[256];
static u1* map2_save[256];
static u1 rtc_save[16], sdd1_save[4];
static void* mt8_save[256];

static void state_save(void)
{
    for (size_t i = 0; i < NFIELDS; i++)
        fld_save[i] = *fields[i].p;
    memcpy(iram_save, IRAM, sizeof iram_save);
    memcpy(brf_save, SA1_BRF, sizeof brf_save);
    memcpy(bw_save, bw_store, sizeof bw_save);
    irqv_save = irqv;
    nmiv_save = nmiv;
    memcpy(mmap_save, snesmmap, sizeof mmap_save);
    memcpy(map2_save, snesmap2, sizeof map2_save);
    memcpy(rtc_save, RTCData, sizeof rtc_save);
    memcpy(sdd1_save, SDD1BankA, sizeof sdd1_save);
    memcpy(mt8_save, memtabler8, sizeof mt8_save);
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
    memcpy(snesmmap, mmap_save, sizeof mmap_save);
    memcpy(snesmap2, map2_save, sizeof map2_save);
    memcpy(RTCData, rtc_save, sizeof rtc_save);
    memcpy(SDD1BankA, sdd1_save, sizeof sdd1_save);
    memcpy(memtabler8, mt8_save, sizeof mt8_save);
}

static void snap(snapshot* s)
{
    for (size_t i = 0; i < NFIELDS; i++)
        s->v[i] = *fields[i].p;
    memcpy(s->iram, IRAM, sizeof s->iram);
    memcpy(s->brf, SA1_BRF, sizeof s->brf);
    s->irqv = irqv;
    s->nmiv = nmiv;
    memcpy(s->mmap, snesmmap, sizeof s->mmap);
    memcpy(s->map2, snesmap2, sizeof s->map2);
    memcpy(s->rtc, RTCData, sizeof s->rtc);
    memcpy(s->sdd1, SDD1BankA, sizeof s->sdd1);
    memcpy(s->mt8, memtabler8, sizeof s->mt8);
    s->stubs = stub_calls;
}

int main(void)
{
    size_t const ncases = sizeof cases / sizeof cases[0];

    SA1RAMArea = bw_store;
    romdata = rom_store;
    dt_fill(rom_store, sizeof rom_store);
    map_init();

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
        dt_fill(RTCData, sizeof rtc_save);
        dt_fill(SDD1BankA, sizeof sdd1_save);
        RTCPtr = dt_mod(20u);
        RTCPtr2 = dt_mod(20u);
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
        map_init();
        dt_fill(map_store, sizeof map_store);
        NumofBanks = (rand() & 1) ? 64u : 128u;
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
        if (getenv("DT_TRACE")) {
            printf("it=%ld %s\n", dt_it, k->name);
            fflush(stdout);
        }
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

        /* $2236 is the one handler whose assembly leaves through a bare
           `jnz near sa1chconv` rather than the register-preserving ccall
           bridge, so the C callee's clobber of eax/ecx/edx lands in the
           oracle's registers. That is the tail-jump's doing, not state the
           port should reproduce; its writes are still compared below. */
        if (strcmp(k->name, "sa12236w") != 0) {
            /* Reads return in al; the rest of eax is the caller's. */
            DT_EQ(k->name, x.eax & 0xFFu, y.eax & 0xFFu);
            /* DH is the scanline cycle count. This is the comparison that
               catches a handler which forgot to charge or clear it. */
            DT_EQ("edx", x.edx, y.edx);
        }
        DT_EQ("stub calls", x.stubs, y.stubs);
        for (size_t i = 0; i < NFIELDS; i++)
            DT_EQ(fields[i].name, x.v[i], y.v[i]);
        DT_EQ("irqv", x.irqv, y.irqv);
        DT_EQ("nmiv", x.nmiv, y.nmiv);
        for (int i = 0; i < 256; i++) {
            if (x.mmap[i] != y.mmap[i] || x.map2[i] != y.map2[i]) {
                char lbl[64];

                snprintf(lbl, sizeof lbl, "%s snesmmap[%d]", k->name, i);
                DT_EQ(lbl, (uintptr_t)x.mmap[i], (uintptr_t)y.mmap[i]);
                snprintf(lbl, sizeof lbl, "%s snesmap2[%d]", k->name, i);
                DT_EQ(lbl, (uintptr_t)x.map2[i], (uintptr_t)y.map2[i]);
                break;
            }
        }
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
        DT_EQ("RTCData", memcmp(x.rtc, y.rtc, sizeof x.rtc), 0);
        DT_EQ("SDD1BankA", memcmp(x.sdd1, y.sdd1, sizeof x.sdd1), 0);
        DT_EQ("memtabler8", memcmp(x.mt8, y.mt8, sizeof x.mt8), 0);
    }
    DT_DONE("sa1regs");
}
