/*
 * Differential test: every SuperFX opcode handler ported into chips/fx_ops.h -
 * all 92 from chips/fxemu2b.asm (branches, TO rN / FROM rN in the b and c
 * table groups, ALT1/ALT2/ALT3) and the 523-strong group from
 * chips/fxemu2.asm (ADD/ADC/SUB/SBC/CMP/AND/BIC/OR/XOR/INC/DEC/MULT/UMULT,
 * register
 * and immediate).
 *
 * Run `make fxops` in this directory. Not part of `all`: the oracle is the
 * original assembly, pulled out of git by mkfxops.sh.
 *
 * Both sides run against the same random flags, registers, program counter and
 * instruction stream, and both dispatch the next opcode into the same recording
 * stub. The comparison covers everything a handler can touch: the resulting
 * program counter, the ALT-mode/opcode word, the source/destination pointers,
 * the whole SuperFX register file, the flag words, and what the nested dispatch
 * saw.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef int8_t s1;
typedef uint32_t u4;
typedef int32_t s4;
typedef uint16_t u2;
typedef int16_t s2;

/* Shared SuperFX state the handlers read (normally chips/c_fxdata.c). */
u4 SfxCarry, SfxSignZero, SfxOverflow;
u4 SfxB, SfxCPB, SfxCROM, SfxRomBuffer, withr15sk;
u4 SfxRAMMem, SfxLastRamAdr;
u4 SfxCBR, SfxPBR, SfxCacheActive;
u4 SfxRAMBR, SfxROMBR, SfxnRamBanks;
u4 SfxCOLR, SfxPOR, SfxSCMR;
u4 fxbit01[256], fxbit23[256], fxbit45[256], fxbit67[256];
u4 fxbit01pcal, fxbit23pcal, fxbit45pcal, fxbit67pcal;
u4 sfxclineloc, sfx128lineloc, sfx160lineloc, sfx192lineloc, sfxobjlineloc;
u4 SfxSFR, SfxCFGR, SfxPIPE, SfxSCBR, SCBRrel;
u4 fxxand[256];
u4 flagnz;

/* PLOT and RPIX index a line table with a packed (x, y) and then address tile
 * data through SCBRrel / sfxramdata. Tile numbers in the table are kept small
 * so the plot buffer can stay a size the test can copy every iteration. */
#define FXLINE_ENTRIES 0x10000
#define FXPLOT_SIZE 0x2000
static u4 fxlines[FXLINE_ENTRIES];
static u1 fxplot[FXPLOT_SIZE];
static u1 fxplot_init[FXPLOT_SIZE];
u4 NumberOfOpcodes, ChangeOps, SFXProc;
u4 PLOTJmpa[64], PLOTJmpb[64];
u4 FxTabled[1024];

u1* sfxramdata;

/* A stand-in for cartridge ROM. The GETB family dereferences SfxRomBuffer, and
 * the assembly reads a dword there, so leave room past the end. */
#define FXROM_SIZE 0x100
static u1 fxrom[FXROM_SIZE + 4];
u4 SfxMemTable[256];

/* chips/fxemu2.asm's FlushCache is a stub; the CACHE opcode only has to reach
 * it, so a stub here matches. */
void FlushCache(void) { }
u4 SfxR0[16];
u4 FxTable[1024];

/* The assembly names R14 and R15 individually; they are just the last two
 * slots of the contiguous register file. */
__asm__(".globl SfxR1\n.set SfxR1, SfxR0+4\n"
        ".globl SfxR2\n.set SfxR2, SfxR0+8\n"
        ".globl SfxR4\n.set SfxR4, SfxR0+16\n"
        ".globl SfxR7\n.set SfxR7, SfxR0+28\n"
        ".globl SfxR8\n.set SfxR8, SfxR0+32\n"
        ".globl SfxR6\n.set SfxR6, SfxR0+24\n"
        ".globl SfxR11\n.set SfxR11, SfxR0+44\n"
        ".globl SfxR12\n.set SfxR12, SfxR0+48\n"
        ".globl SfxR13\n.set SfxR13, SfxR0+52\n"
        ".globl SfxR14\n.set SfxR14, SfxR0+56\n"
        ".globl SfxR15\n.set SfxR15, SfxR0+60\n");

/* ecx indexes (ALT mode << 8) | opcode, so the b and c tables need all four
 * ALT sub-tables' worth of entries laid out adjacently, as in endmem.c. */
u4 FxTableb[1024];
u4 FxTablec[1024];

/* The seam block (normally chips/c_fxops.c). */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;
u4 FxLoopDone;
u4 SfxSREG, SfxDREG;

/* What the nested dispatch saw. StubTable identifies which of the three tables
 * it came through, so choosing the wrong one is a visible mismatch. */
u4 StubPC, StubCX, StubSrc, StubDst, StubHits, StubTable, StubB;
/* Which slot of that table the dispatch actually indexed. */
u4 StubIdx;
/* What the stub writes into withr15sk, standing in for a nested opcode that
 * set the program counter itself, and the R15 it leaves behind. */
u4 StubR15sk, StubR15, StubWrR15sk;
/* Set when a d-table handler ran the loop epilogue instead of chaining. */
u4 StubEndLoop;
/* Which PLOTJmp stub CMODE selected, and whether one ran. */
u4 StubPlotIdx, StubPlotHits;
extern u1 plotstubs[]; /* _fxops.o */

void asm_fxcall(void* fn); /* _fxops.o */
void fxstub(void), fxstubb(void), fxstubc(void); /* _fxops.o */

#include "../chips/fx_ops.h"

/* The oracle's stubs read the register ABI, so they only work under
   asm_fxcall. The ported side is entered as a plain C call and reads the seam,
   so it needs its own stubs; run() swaps the right set into the tables. The
   two must record exactly the same things or a real difference hides here.
   _fxops.o reaches these through the per-slot trampolines, hence non-static. */
#define FXSTUB_C(name, tab)                 \
    void name(void)                         \
    {                                       \
        StubPC = (u4)(uintptr_t)FxSeamPC;   \
        StubCX = FxSeamCX;                  \
        StubSrc = (u4)(uintptr_t)FxSeamSrc; \
        StubDst = (u4)(uintptr_t)FxSeamDst; \
        StubTable = (tab);                  \
        StubB = SfxB;                       \
        if (StubWrR15sk) {                  \
            withr15sk = StubR15sk;          \
        }                                   \
        SfxR0[15] = StubR15;                \
        StubHits++;                         \
    }

FXSTUB_C(fxstub_c, 1)
FXSTUB_C(fxstubb_c, 2)
FXSTUB_C(fxstubc_c, 3)
FXSTUB_C(fxstubd_c, 4)

/* 1024 trampolines per table, one per slot, each 16 bytes and each recording
   its own index before falling into the stub above. Without them every slot in
   a table is the same address and a dispatch that indexes wrong still lands on
   something that looks right. */
#define IDXSTUB 16
extern u1 idxa_asm[], idxb_asm[], idxc_asm[], idxd_asm[]; /* _fxops.o */
extern u1 idxa_c[], idxb_c[], idxc_c[], idxd_c[]; /* _fxops.o */

/* One copy of each dispatch table per side. The PLOT stubs are shared: they
   only write memory, so they are callable either way. */
static u4 tab_asm[4][1024], tab_c[4][1024];

extern void asm_FxOpb05(void), asm_FxOpb06(void), asm_FxOpb07(void), asm_FxOpb08(void);
extern void asm_FxOpb09(void), asm_FxOpb0A(void), asm_FxOpb0B(void), asm_FxOpb0C(void);
extern void asm_FxOpb0D(void), asm_FxOpb0E(void), asm_FxOpb0F(void), asm_FxOpb10(void);
extern void asm_FxOpb11(void), asm_FxOpb12(void), asm_FxOpb13(void), asm_FxOpb14(void);
extern void asm_FxOpb15(void), asm_FxOpb16(void), asm_FxOpb17(void), asm_FxOpb18(void);
extern void asm_FxOpb19(void), asm_FxOpb1A(void), asm_FxOpb1B(void), asm_FxOpb1C(void);
extern void asm_FxOpb1D(void), asm_FxOpb1E(void), asm_FxOpb1F(void), asm_FxOpb3D(void);
extern void asm_FxOpb3E(void), asm_FxOpb3F(void), asm_FxOpbB0(void), asm_FxOpbB1(void);
extern void asm_FxOpbB2(void), asm_FxOpbB3(void), asm_FxOpbB4(void), asm_FxOpbB5(void);
extern void asm_FxOpbB6(void), asm_FxOpbB7(void), asm_FxOpbB8(void), asm_FxOpbB9(void);
extern void asm_FxOpbBA(void), asm_FxOpbBB(void), asm_FxOpbBC(void), asm_FxOpbBD(void);
extern void asm_FxOpbBE(void), asm_FxOpbBF(void), asm_FxOpc05(void), asm_FxOpc06(void);
extern void asm_FxOpc07(void), asm_FxOpc08(void), asm_FxOpc09(void), asm_FxOpc0A(void);
extern void asm_FxOpc0B(void), asm_FxOpc0C(void), asm_FxOpc0D(void), asm_FxOpc0E(void);
extern void asm_FxOpc0F(void), asm_FxOpc10(void), asm_FxOpc11(void), asm_FxOpc12(void);
extern void asm_FxOpc13(void), asm_FxOpc14(void), asm_FxOpc15(void), asm_FxOpc16(void);
extern void asm_FxOpc17(void), asm_FxOpc18(void), asm_FxOpc19(void), asm_FxOpc1A(void);
extern void asm_FxOpc1B(void), asm_FxOpc1C(void), asm_FxOpc1D(void), asm_FxOpc1E(void);
extern void asm_FxOpc1F(void), asm_FxOpc3D(void), asm_FxOpc3E(void), asm_FxOpc3F(void);
extern void asm_FxOpcB0(void), asm_FxOpcB1(void), asm_FxOpcB2(void), asm_FxOpcB3(void);
extern void asm_FxOpcB4(void), asm_FxOpcB5(void), asm_FxOpcB6(void), asm_FxOpcB7(void);
extern void asm_FxOpcB8(void), asm_FxOpcB9(void), asm_FxOpcBA(void), asm_FxOpcBB(void);
extern void asm_FxOpcBC(void), asm_FxOpcBD(void), asm_FxOpcBE(void), asm_FxOpcBF(void);
extern void asm_FxOp50(void), asm_FxOp51(void), asm_FxOp52(void), asm_FxOp53(void);
extern void asm_FxOp54(void), asm_FxOp55(void), asm_FxOp56(void), asm_FxOp57(void);
extern void asm_FxOp58(void), asm_FxOp59(void), asm_FxOp5A(void), asm_FxOp5B(void);
extern void asm_FxOp5C(void), asm_FxOp5D(void), asm_FxOp5E(void), asm_FxOp50A1(void);
extern void asm_FxOp51A1(void), asm_FxOp52A1(void), asm_FxOp53A1(void), asm_FxOp54A1(void);
extern void asm_FxOp55A1(void), asm_FxOp56A1(void), asm_FxOp57A1(void), asm_FxOp58A1(void);
extern void asm_FxOp59A1(void), asm_FxOp5AA1(void), asm_FxOp5BA1(void), asm_FxOp5CA1(void);
extern void asm_FxOp5DA1(void), asm_FxOp5EA1(void), asm_FxOp50A2(void), asm_FxOp51A2(void);
extern void asm_FxOp52A2(void), asm_FxOp53A2(void), asm_FxOp54A2(void), asm_FxOp55A2(void);
extern void asm_FxOp56A2(void), asm_FxOp57A2(void), asm_FxOp58A2(void), asm_FxOp59A2(void);
extern void asm_FxOp5AA2(void), asm_FxOp5BA2(void), asm_FxOp5CA2(void), asm_FxOp5DA2(void);
extern void asm_FxOp5EA2(void), asm_FxOp5FA2(void), asm_FxOp50A3(void), asm_FxOp51A3(void);
extern void asm_FxOp52A3(void), asm_FxOp53A3(void), asm_FxOp54A3(void), asm_FxOp55A3(void);
extern void asm_FxOp56A3(void), asm_FxOp57A3(void), asm_FxOp58A3(void), asm_FxOp59A3(void);
extern void asm_FxOp5AA3(void), asm_FxOp5BA3(void), asm_FxOp5CA3(void), asm_FxOp5DA3(void);
extern void asm_FxOp5EA3(void), asm_FxOp5FA3(void), asm_FxOp60(void), asm_FxOp61(void);
extern void asm_FxOp62(void), asm_FxOp63(void), asm_FxOp64(void), asm_FxOp65(void);
extern void asm_FxOp66(void), asm_FxOp67(void), asm_FxOp68(void), asm_FxOp69(void);
extern void asm_FxOp6A(void), asm_FxOp6B(void), asm_FxOp6C(void), asm_FxOp6D(void);
extern void asm_FxOp6E(void), asm_FxOp60A1(void), asm_FxOp61A1(void), asm_FxOp62A1(void);
extern void asm_FxOp63A1(void), asm_FxOp64A1(void), asm_FxOp65A1(void), asm_FxOp66A1(void);
extern void asm_FxOp67A1(void), asm_FxOp68A1(void), asm_FxOp69A1(void), asm_FxOp6AA1(void);
extern void asm_FxOp6BA1(void), asm_FxOp6CA1(void), asm_FxOp6DA1(void), asm_FxOp6EA1(void);
extern void asm_FxOp60A2(void), asm_FxOp61A2(void), asm_FxOp62A2(void), asm_FxOp63A2(void);
extern void asm_FxOp64A2(void), asm_FxOp65A2(void), asm_FxOp66A2(void), asm_FxOp67A2(void);
extern void asm_FxOp68A2(void), asm_FxOp69A2(void), asm_FxOp6AA2(void), asm_FxOp6BA2(void);
extern void asm_FxOp6CA2(void), asm_FxOp6DA2(void), asm_FxOp6EA2(void), asm_FxOp6FA2(void);
extern void asm_FxOp60A3(void), asm_FxOp61A3(void), asm_FxOp62A3(void), asm_FxOp63A3(void);
extern void asm_FxOp64A3(void), asm_FxOp65A3(void), asm_FxOp66A3(void), asm_FxOp67A3(void);
extern void asm_FxOp68A3(void), asm_FxOp69A3(void), asm_FxOp6AA3(void), asm_FxOp6BA3(void);
extern void asm_FxOp6CA3(void), asm_FxOp6DA3(void), asm_FxOp6EA3(void), asm_FxOp71(void);
extern void asm_FxOp72(void), asm_FxOp73(void), asm_FxOp74(void), asm_FxOp75(void);
extern void asm_FxOp76(void), asm_FxOp77(void), asm_FxOp78(void), asm_FxOp79(void);
extern void asm_FxOp7A(void), asm_FxOp7B(void), asm_FxOp7C(void), asm_FxOp7D(void);
extern void asm_FxOp7E(void), asm_FxOp71A1(void), asm_FxOp72A1(void), asm_FxOp73A1(void);
extern void asm_FxOp74A1(void), asm_FxOp75A1(void), asm_FxOp76A1(void), asm_FxOp77A1(void);
extern void asm_FxOp78A1(void), asm_FxOp79A1(void), asm_FxOp7AA1(void), asm_FxOp7BA1(void);
extern void asm_FxOp7CA1(void), asm_FxOp7DA1(void), asm_FxOp7EA1(void), asm_FxOp71A2(void);
extern void asm_FxOp72A2(void), asm_FxOp73A2(void), asm_FxOp74A2(void), asm_FxOp75A2(void);
extern void asm_FxOp76A2(void), asm_FxOp77A2(void), asm_FxOp78A2(void), asm_FxOp79A2(void);
extern void asm_FxOp7AA2(void), asm_FxOp7BA2(void), asm_FxOp7CA2(void), asm_FxOp7DA2(void);
extern void asm_FxOp7EA2(void), asm_FxOp7FA2(void), asm_FxOp71A3(void), asm_FxOp72A3(void);
extern void asm_FxOp73A3(void), asm_FxOp74A3(void), asm_FxOp75A3(void), asm_FxOp76A3(void);
extern void asm_FxOp77A3(void), asm_FxOp78A3(void), asm_FxOp79A3(void), asm_FxOp7AA3(void);
extern void asm_FxOp7BA3(void), asm_FxOp7CA3(void), asm_FxOp7DA3(void), asm_FxOp7EA3(void);
extern void asm_FxOp7FA3(void), asm_FxOpC1(void), asm_FxOpC2(void), asm_FxOpC3(void);
extern void asm_FxOpC4(void), asm_FxOpC5(void), asm_FxOpC6(void), asm_FxOpC7(void);
extern void asm_FxOpC8(void), asm_FxOpC9(void), asm_FxOpCA(void), asm_FxOpCB(void);
extern void asm_FxOpCC(void), asm_FxOpCD(void), asm_FxOpCE(void), asm_FxOpC1A1(void);
extern void asm_FxOpC2A1(void), asm_FxOpC3A1(void), asm_FxOpC4A1(void), asm_FxOpC5A1(void);
extern void asm_FxOpC6A1(void), asm_FxOpC7A1(void), asm_FxOpC8A1(void), asm_FxOpC9A1(void);
extern void asm_FxOpCAA1(void), asm_FxOpCBA1(void), asm_FxOpCCA1(void), asm_FxOpCDA1(void);
extern void asm_FxOpCEA1(void), asm_FxOpC1A2(void), asm_FxOpC2A2(void), asm_FxOpC3A2(void);
extern void asm_FxOpC4A2(void), asm_FxOpC5A2(void), asm_FxOpC6A2(void), asm_FxOpC7A2(void);
extern void asm_FxOpC8A2(void), asm_FxOpC9A2(void), asm_FxOpCAA2(void), asm_FxOpCBA2(void);
extern void asm_FxOpCCA2(void), asm_FxOpCDA2(void), asm_FxOpCEA2(void), asm_FxOpCFA2(void);
extern void asm_FxOpC1A3(void), asm_FxOpC2A3(void), asm_FxOpC3A3(void), asm_FxOpC4A3(void);
extern void asm_FxOpC5A3(void), asm_FxOpC6A3(void), asm_FxOpC7A3(void), asm_FxOpC8A3(void);
extern void asm_FxOpC9A3(void), asm_FxOpCAA3(void), asm_FxOpCBA3(void), asm_FxOpCCA3(void);
extern void asm_FxOpCDA3(void), asm_FxOpCEA3(void), asm_FxOpCFA3(void), asm_FxOpD0(void);
extern void asm_FxOpD1(void), asm_FxOpD2(void), asm_FxOpD3(void), asm_FxOpD4(void);
extern void asm_FxOpD5(void), asm_FxOpD6(void), asm_FxOpD7(void), asm_FxOpD8(void);
extern void asm_FxOpD9(void), asm_FxOpDA(void), asm_FxOpDB(void), asm_FxOpDC(void);
extern void asm_FxOpDD(void), asm_FxOpE0(void), asm_FxOpE1(void), asm_FxOpE2(void);
extern void asm_FxOpE3(void), asm_FxOpE4(void), asm_FxOpE5(void), asm_FxOpE6(void);
extern void asm_FxOpE7(void), asm_FxOpE8(void), asm_FxOpE9(void), asm_FxOpEA(void);
extern void asm_FxOpEB(void), asm_FxOpEC(void), asm_FxOpED(void), asm_FxOp80(void);
extern void asm_FxOp81(void), asm_FxOp82(void), asm_FxOp83(void), asm_FxOp84(void);
extern void asm_FxOp85(void), asm_FxOp86(void), asm_FxOp87(void), asm_FxOp88(void);
extern void asm_FxOp89(void), asm_FxOp8A(void), asm_FxOp8B(void), asm_FxOp8C(void);
extern void asm_FxOp8D(void), asm_FxOp8E(void), asm_FxOp80A1(void), asm_FxOp81A1(void);
extern void asm_FxOp82A1(void), asm_FxOp83A1(void), asm_FxOp84A1(void), asm_FxOp85A1(void);
extern void asm_FxOp86A1(void), asm_FxOp87A1(void), asm_FxOp88A1(void), asm_FxOp89A1(void);
extern void asm_FxOp8AA1(void), asm_FxOp8BA1(void), asm_FxOp8CA1(void), asm_FxOp8DA1(void);
extern void asm_FxOp8EA1(void), asm_FxOp80A2(void), asm_FxOp81A2(void), asm_FxOp82A2(void);
extern void asm_FxOp83A2(void), asm_FxOp84A2(void), asm_FxOp85A2(void), asm_FxOp86A2(void);
extern void asm_FxOp87A2(void), asm_FxOp88A2(void), asm_FxOp89A2(void), asm_FxOp8AA2(void);
extern void asm_FxOp8BA2(void), asm_FxOp8CA2(void), asm_FxOp8DA2(void), asm_FxOp8EA2(void);
extern void asm_FxOp8FA2(void), asm_FxOp80A3(void), asm_FxOp81A3(void), asm_FxOp82A3(void);
extern void asm_FxOp83A3(void), asm_FxOp84A3(void), asm_FxOp85A3(void), asm_FxOp86A3(void);
extern void asm_FxOp87A3(void), asm_FxOp88A3(void), asm_FxOp89A3(void), asm_FxOp8AA3(void);
extern void asm_FxOp8BA3(void), asm_FxOp8CA3(void), asm_FxOp8DA3(void), asm_FxOp8EA3(void);
extern void asm_FxOp8FA3(void), asm_FxOp10(void), asm_FxOp11(void), asm_FxOp12(void);
extern void asm_FxOp13(void), asm_FxOp14(void), asm_FxOp15(void), asm_FxOp16(void);
extern void asm_FxOp17(void), asm_FxOp18(void), asm_FxOp19(void), asm_FxOp1A(void);
extern void asm_FxOp1B(void), asm_FxOp1C(void), asm_FxOp1D(void), asm_FxOp20(void);
extern void asm_FxOp21(void), asm_FxOp22(void), asm_FxOp23(void), asm_FxOp24(void);
extern void asm_FxOp25(void), asm_FxOp26(void), asm_FxOp27(void), asm_FxOp28(void);
extern void asm_FxOp29(void), asm_FxOp2A(void), asm_FxOp2B(void), asm_FxOp2C(void);
extern void asm_FxOp2D(void), asm_FxOpB0(void), asm_FxOpB1(void), asm_FxOpB2(void);
extern void asm_FxOpB3(void), asm_FxOpB4(void), asm_FxOpB5(void), asm_FxOpB6(void);
extern void asm_FxOpB7(void), asm_FxOpB8(void), asm_FxOpB9(void), asm_FxOpBA(void);
extern void asm_FxOpBB(void), asm_FxOpBC(void), asm_FxOpBD(void), asm_FxOpBE(void);
extern void asm_FxOp30(void), asm_FxOp31(void), asm_FxOp32(void), asm_FxOp33(void);
extern void asm_FxOp34(void), asm_FxOp35(void), asm_FxOp36(void), asm_FxOp37(void);
extern void asm_FxOp38(void), asm_FxOp39(void), asm_FxOp3A(void), asm_FxOp3B(void);
extern void asm_FxOp30A1(void), asm_FxOp31A1(void), asm_FxOp32A1(void), asm_FxOp33A1(void);
extern void asm_FxOp34A1(void), asm_FxOp35A1(void), asm_FxOp36A1(void), asm_FxOp37A1(void);
extern void asm_FxOp38A1(void), asm_FxOp39A1(void), asm_FxOp3AA1(void), asm_FxOp3BA1(void);
extern void asm_FxOp40(void), asm_FxOp41(void), asm_FxOp42(void), asm_FxOp43(void);
extern void asm_FxOp44(void), asm_FxOp45(void), asm_FxOp46(void), asm_FxOp47(void);
extern void asm_FxOp48(void), asm_FxOp49(void), asm_FxOp4A(void), asm_FxOp4B(void);
extern void asm_FxOp40A1(void), asm_FxOp41A1(void), asm_FxOp42A1(void), asm_FxOp43A1(void);
extern void asm_FxOp44A1(void), asm_FxOp45A1(void), asm_FxOp46A1(void), asm_FxOp47A1(void);
extern void asm_FxOp48A1(void), asm_FxOp49A1(void), asm_FxOp4AA1(void), asm_FxOp4BA1(void);
extern void asm_FxOpA0(void), asm_FxOpA1(void), asm_FxOpA2(void), asm_FxOpA3(void);
extern void asm_FxOpA4(void), asm_FxOpA5(void), asm_FxOpA6(void), asm_FxOpA7(void);
extern void asm_FxOpA8(void), asm_FxOpA9(void), asm_FxOpAA(void), asm_FxOpAB(void);
extern void asm_FxOpAC(void), asm_FxOpAD(void), asm_FxOpA0A1(void), asm_FxOpA1A1(void);
extern void asm_FxOpA2A1(void), asm_FxOpA3A1(void), asm_FxOpA4A1(void), asm_FxOpA5A1(void);
extern void asm_FxOpA6A1(void), asm_FxOpA7A1(void), asm_FxOpA8A1(void), asm_FxOpA9A1(void);
extern void asm_FxOpAAA1(void), asm_FxOpABA1(void), asm_FxOpACA1(void), asm_FxOpADA1(void);
extern void asm_FxOpA0A2(void), asm_FxOpA1A2(void), asm_FxOpA2A2(void), asm_FxOpA3A2(void);
extern void asm_FxOpA4A2(void), asm_FxOpA5A2(void), asm_FxOpA6A2(void), asm_FxOpA7A2(void);
extern void asm_FxOpA8A2(void), asm_FxOpA9A2(void), asm_FxOpAAA2(void), asm_FxOpABA2(void);
extern void asm_FxOpACA2(void), asm_FxOpADA2(void), asm_FxOpAEA2(void), asm_FxOpF0(void);
extern void asm_FxOpF1(void), asm_FxOpF2(void), asm_FxOpF3(void), asm_FxOpF4(void);
extern void asm_FxOpF5(void), asm_FxOpF6(void), asm_FxOpF7(void), asm_FxOpF8(void);
extern void asm_FxOpF9(void), asm_FxOpFA(void), asm_FxOpFB(void), asm_FxOpFC(void);
extern void asm_FxOpFD(void), asm_FxOpF0A1(void), asm_FxOpF1A1(void), asm_FxOpF2A1(void);
extern void asm_FxOpF3A1(void), asm_FxOpF4A1(void), asm_FxOpF5A1(void), asm_FxOpF6A1(void);
extern void asm_FxOpF7A1(void), asm_FxOpF8A1(void), asm_FxOpF9A1(void), asm_FxOpFAA1(void);
extern void asm_FxOpFBA1(void), asm_FxOpFCA1(void), asm_FxOpFDA1(void), asm_FxOpF0A2(void);
extern void asm_FxOpF1A2(void), asm_FxOpF2A2(void), asm_FxOpF3A2(void), asm_FxOpF4A2(void);
extern void asm_FxOpF5A2(void), asm_FxOpF6A2(void), asm_FxOpF7A2(void), asm_FxOpF8A2(void);
extern void asm_FxOpF9A2(void), asm_FxOpFAA2(void), asm_FxOpFBA2(void), asm_FxOpFCA2(void);
extern void asm_FxOpFDA2(void), asm_FxOpFEA2(void), asm_FxOp91(void), asm_FxOp92(void);
extern void asm_FxOp93(void), asm_FxOp94(void), asm_FxOp98(void), asm_FxOp99(void);
extern void asm_FxOp9A(void), asm_FxOp9B(void), asm_FxOp9C(void), asm_FxOp9D(void);
extern void asm_FxOp98A1(void), asm_FxOp99A1(void), asm_FxOp9AA1(void), asm_FxOp9BA1(void);
extern void asm_FxOp9CA1(void), asm_FxOp9DA1(void), asm_FxOp02(void);
extern void asm_FxOp01(void), asm_FxOp4D(void), asm_FxOp4F(void), asm_FxOp95(void), asm_FxOp96(void), asm_FxOp96A1(void), asm_FxOp97(void), asm_FxOp9E(void), asm_FxOpC0(void);
extern void asm_FxOp03(void), asm_FxOp04(void), asm_FxOp3C(void), asm_FxOp9F(void), asm_FxOp9FA1(void), asm_FxOpAE(void), asm_FxOpAF(void), asm_FxOpDE(void), asm_FxOpEE(void);
extern void asm_FxOp05(void), asm_FxOp06(void), asm_FxOp07(void), asm_FxOp08(void);
extern void asm_FxOp09(void), asm_FxOp0A(void), asm_FxOp0B(void), asm_FxOp0C(void);
extern void asm_FxOp0D(void), asm_FxOp0E(void), asm_FxOp0F(void), asm_FxOp1E(void);
extern void asm_FxOp1F(void), asm_FxOp2E(void), asm_FxOp2F(void), asm_FxOp3D(void);
extern void asm_FxOp3E(void), asm_FxOp3F(void), asm_FxOpBF(void);
extern void asm_FxOp5F(void), asm_FxOp5FA1(void), asm_FxOp6F(void), asm_FxOp6FA1(void);
extern void asm_FxOp6FA3(void), asm_FxOp7F(void), asm_FxOp7FA1(void), asm_FxOp8F(void);
extern void asm_FxOp8FA1(void), asm_FxOp90(void);
extern void asm_FxOpEF(void), asm_FxOpEFA1(void), asm_FxOpEFA2(void), asm_FxOpEFA3(void);
extern void asm_FxOpDFA2(void), asm_FxOpDFA3(void), asm_FxOpCF(void), asm_FxOpCFA1(void);
extern void asm_FxOpAEA1(void), asm_FxOpAFA1(void), asm_FxOpAFA2(void), asm_FxOpFE(void);
extern void asm_FxOpFF(void);
extern void asm_FxOp4E(void), asm_FxOpDF(void), asm_FxOp4EA1(void), asm_FxOp70(void);
extern void asm_FxOpFEA1(void), asm_FxOpFFA1(void), asm_FxOpFFA2(void);
extern void asm_FxOp00(void);
extern void asm_FxOp4C1284b(void), asm_FxOp4C1284bz(void), asm_FxOp4C1284bd(void), asm_FxOp4C1284bzd(void);
extern void asm_FxOp4C1282b(void), asm_FxOp4C1282bz(void), asm_FxOp4C1282bd(void), asm_FxOp4C1282bzd(void);
extern void asm_FxOp4C1288b(void), asm_FxOp4C1288bz(void), asm_FxOp4C1288bd(void), asm_FxOp4C1288bzd(void);
extern void asm_FxOp4C1288bl(void), asm_FxOp4C1288bzl(void), asm_FxOp4C1288bdl(void), asm_FxOp4C1288bzdl(void);
extern void asm_FxOp4C(void), asm_FxOp4CA1(void);
/* The d table: same bodies as the base table, reached through the threaded
 * FXReturn tail. Both sides go through a real seam thunk. */
extern void asm_FxOpd00(void), asm_FxOpd01(void), asm_FxOpd02(void), asm_FxOpd03(void);
extern void asm_FxOpd04(void), asm_FxOpd05(void), asm_FxOpd06(void), asm_FxOpd07(void);
extern void asm_FxOpd08(void), asm_FxOpd09(void), asm_FxOpd0A(void), asm_FxOpd0B(void);
extern void asm_FxOpd0C(void), asm_FxOpd0D(void), asm_FxOpd0E(void), asm_FxOpd0F(void);
extern void asm_FxOpd10(void), asm_FxOpd11(void), asm_FxOpd12(void), asm_FxOpd13(void);
extern void asm_FxOpd14(void), asm_FxOpd15(void), asm_FxOpd16(void), asm_FxOpd17(void);
extern void asm_FxOpd18(void), asm_FxOpd19(void), asm_FxOpd1A(void), asm_FxOpd1B(void);
extern void asm_FxOpd1C(void), asm_FxOpd1D(void), asm_FxOpd1E(void), asm_FxOpd1F(void);
extern void asm_FxOpd20(void), asm_FxOpd21(void), asm_FxOpd22(void), asm_FxOpd23(void);
extern void asm_FxOpd24(void), asm_FxOpd25(void), asm_FxOpd26(void), asm_FxOpd27(void);
extern void asm_FxOpd28(void), asm_FxOpd29(void), asm_FxOpd2A(void), asm_FxOpd2B(void);
extern void asm_FxOpd2C(void), asm_FxOpd2D(void), asm_FxOpd2E(void), asm_FxOpd2F(void);
extern void asm_FxOpd30(void), asm_FxOpd30A1(void), asm_FxOpd31(void), asm_FxOpd31A1(void);
extern void asm_FxOpd32(void), asm_FxOpd32A1(void), asm_FxOpd33(void), asm_FxOpd33A1(void);
extern void asm_FxOpd34(void), asm_FxOpd34A1(void), asm_FxOpd35(void), asm_FxOpd35A1(void);
extern void asm_FxOpd36(void), asm_FxOpd36A1(void), asm_FxOpd37(void), asm_FxOpd37A1(void);
extern void asm_FxOpd38(void), asm_FxOpd38A1(void), asm_FxOpd39(void), asm_FxOpd39A1(void);
extern void asm_FxOpd3A(void), asm_FxOpd3AA1(void), asm_FxOpd3B(void), asm_FxOpd3BA1(void);
extern void asm_FxOpd3C(void), asm_FxOpd3D(void), asm_FxOpd3E(void), asm_FxOpd3F(void);
extern void asm_FxOpd40(void), asm_FxOpd40A1(void), asm_FxOpd41(void), asm_FxOpd41A1(void);
extern void asm_FxOpd42(void), asm_FxOpd42A1(void), asm_FxOpd43(void), asm_FxOpd43A1(void);
extern void asm_FxOpd44(void), asm_FxOpd44A1(void), asm_FxOpd45(void), asm_FxOpd45A1(void);
extern void asm_FxOpd46(void), asm_FxOpd46A1(void), asm_FxOpd47(void), asm_FxOpd47A1(void);
extern void asm_FxOpd48(void), asm_FxOpd48A1(void), asm_FxOpd49(void), asm_FxOpd49A1(void);
extern void asm_FxOpd4A(void), asm_FxOpd4AA1(void), asm_FxOpd4B(void), asm_FxOpd4BA1(void);
extern void asm_FxOpd4C(void), asm_FxOpd4C1282b(void), asm_FxOpd4C1282bd(void), asm_FxOpd4C1282bz(void);
extern void asm_FxOpd4C1282bzd(void), asm_FxOpd4C1284b(void), asm_FxOpd4C1284bd(void), asm_FxOpd4C1284bz(void);
extern void asm_FxOpd4C1284bzd(void), asm_FxOpd4C1288b(void), asm_FxOpd4C1288bd(void), asm_FxOpd4C1288bdl(void);
extern void asm_FxOpd4C1288bl(void), asm_FxOpd4C1288bz(void), asm_FxOpd4C1288bzd(void), asm_FxOpd4C1288bzdl(void);
extern void asm_FxOpd4C1288bzl(void), asm_FxOpd4CA1(void), asm_FxOpd4D(void), asm_FxOpd4E(void);
extern void asm_FxOpd4EA1(void), asm_FxOpd4F(void), asm_FxOpd50(void), asm_FxOpd50A1(void);
extern void asm_FxOpd50A2(void), asm_FxOpd50A3(void), asm_FxOpd51(void), asm_FxOpd51A1(void);
extern void asm_FxOpd51A2(void), asm_FxOpd51A3(void), asm_FxOpd52(void), asm_FxOpd52A1(void);
extern void asm_FxOpd52A2(void), asm_FxOpd52A3(void), asm_FxOpd53(void), asm_FxOpd53A1(void);
extern void asm_FxOpd53A2(void), asm_FxOpd53A3(void), asm_FxOpd54(void), asm_FxOpd54A1(void);
extern void asm_FxOpd54A2(void), asm_FxOpd54A3(void), asm_FxOpd55(void), asm_FxOpd55A1(void);
extern void asm_FxOpd55A2(void), asm_FxOpd55A3(void), asm_FxOpd56(void), asm_FxOpd56A1(void);
extern void asm_FxOpd56A2(void), asm_FxOpd56A3(void), asm_FxOpd57(void), asm_FxOpd57A1(void);
extern void asm_FxOpd57A2(void), asm_FxOpd57A3(void), asm_FxOpd58(void), asm_FxOpd58A1(void);
extern void asm_FxOpd58A2(void), asm_FxOpd58A3(void), asm_FxOpd59(void), asm_FxOpd59A1(void);
extern void asm_FxOpd59A2(void), asm_FxOpd59A3(void), asm_FxOpd5A(void), asm_FxOpd5AA1(void);
extern void asm_FxOpd5AA2(void), asm_FxOpd5AA3(void), asm_FxOpd5B(void), asm_FxOpd5BA1(void);
extern void asm_FxOpd5BA2(void), asm_FxOpd5BA3(void), asm_FxOpd5C(void), asm_FxOpd5CA1(void);
extern void asm_FxOpd5CA2(void), asm_FxOpd5CA3(void), asm_FxOpd5D(void), asm_FxOpd5DA1(void);
extern void asm_FxOpd5DA2(void), asm_FxOpd5DA3(void), asm_FxOpd5E(void), asm_FxOpd5EA1(void);
extern void asm_FxOpd5EA2(void), asm_FxOpd5EA3(void), asm_FxOpd5F(void), asm_FxOpd5FA1(void);
extern void asm_FxOpd5FA2(void), asm_FxOpd5FA3(void), asm_FxOpd60(void), asm_FxOpd60A1(void);
extern void asm_FxOpd60A2(void), asm_FxOpd60A3(void), asm_FxOpd61(void), asm_FxOpd61A1(void);
extern void asm_FxOpd61A2(void), asm_FxOpd61A3(void), asm_FxOpd62(void), asm_FxOpd62A1(void);
extern void asm_FxOpd62A2(void), asm_FxOpd62A3(void), asm_FxOpd63(void), asm_FxOpd63A1(void);
extern void asm_FxOpd63A2(void), asm_FxOpd63A3(void), asm_FxOpd64(void), asm_FxOpd64A1(void);
extern void asm_FxOpd64A2(void), asm_FxOpd64A3(void), asm_FxOpd65(void), asm_FxOpd65A1(void);
extern void asm_FxOpd65A2(void), asm_FxOpd65A3(void), asm_FxOpd66(void), asm_FxOpd66A1(void);
extern void asm_FxOpd66A2(void), asm_FxOpd66A3(void), asm_FxOpd67(void), asm_FxOpd67A1(void);
extern void asm_FxOpd67A2(void), asm_FxOpd67A3(void), asm_FxOpd68(void), asm_FxOpd68A1(void);
extern void asm_FxOpd68A2(void), asm_FxOpd68A3(void), asm_FxOpd69(void), asm_FxOpd69A1(void);
extern void asm_FxOpd69A2(void), asm_FxOpd69A3(void), asm_FxOpd6A(void), asm_FxOpd6AA1(void);
extern void asm_FxOpd6AA2(void), asm_FxOpd6AA3(void), asm_FxOpd6B(void), asm_FxOpd6BA1(void);
extern void asm_FxOpd6BA2(void), asm_FxOpd6BA3(void), asm_FxOpd6C(void), asm_FxOpd6CA1(void);
extern void asm_FxOpd6CA2(void), asm_FxOpd6CA3(void), asm_FxOpd6D(void), asm_FxOpd6DA1(void);
extern void asm_FxOpd6DA2(void), asm_FxOpd6DA3(void), asm_FxOpd6E(void), asm_FxOpd6EA1(void);
extern void asm_FxOpd6EA2(void), asm_FxOpd6EA3(void), asm_FxOpd6F(void), asm_FxOpd6FA1(void);
extern void asm_FxOpd6FA2(void), asm_FxOpd6FA3(void), asm_FxOpd70(void), asm_FxOpd71(void);
extern void asm_FxOpd71A1(void), asm_FxOpd71A2(void), asm_FxOpd71A3(void), asm_FxOpd72(void);
extern void asm_FxOpd72A1(void), asm_FxOpd72A2(void), asm_FxOpd72A3(void), asm_FxOpd73(void);
extern void asm_FxOpd73A1(void), asm_FxOpd73A2(void), asm_FxOpd73A3(void), asm_FxOpd74(void);
extern void asm_FxOpd74A1(void), asm_FxOpd74A2(void), asm_FxOpd74A3(void), asm_FxOpd75(void);
extern void asm_FxOpd75A1(void), asm_FxOpd75A2(void), asm_FxOpd75A3(void), asm_FxOpd76(void);
extern void asm_FxOpd76A1(void), asm_FxOpd76A2(void), asm_FxOpd76A3(void), asm_FxOpd77(void);
extern void asm_FxOpd77A1(void), asm_FxOpd77A2(void), asm_FxOpd77A3(void), asm_FxOpd78(void);
extern void asm_FxOpd78A1(void), asm_FxOpd78A2(void), asm_FxOpd78A3(void), asm_FxOpd79(void);
extern void asm_FxOpd79A1(void), asm_FxOpd79A2(void), asm_FxOpd79A3(void), asm_FxOpd7A(void);
extern void asm_FxOpd7AA1(void), asm_FxOpd7AA2(void), asm_FxOpd7AA3(void), asm_FxOpd7B(void);
extern void asm_FxOpd7BA1(void), asm_FxOpd7BA2(void), asm_FxOpd7BA3(void), asm_FxOpd7C(void);
extern void asm_FxOpd7CA1(void), asm_FxOpd7CA2(void), asm_FxOpd7CA3(void), asm_FxOpd7D(void);
extern void asm_FxOpd7DA1(void), asm_FxOpd7DA2(void), asm_FxOpd7DA3(void), asm_FxOpd7E(void);
extern void asm_FxOpd7EA1(void), asm_FxOpd7EA2(void), asm_FxOpd7EA3(void), asm_FxOpd7F(void);
extern void asm_FxOpd7FA1(void), asm_FxOpd7FA2(void), asm_FxOpd7FA3(void), asm_FxOpd80(void);
extern void asm_FxOpd80A1(void), asm_FxOpd80A2(void), asm_FxOpd80A3(void), asm_FxOpd81(void);
extern void asm_FxOpd81A1(void), asm_FxOpd81A2(void), asm_FxOpd81A3(void), asm_FxOpd82(void);
extern void asm_FxOpd82A1(void), asm_FxOpd82A2(void), asm_FxOpd82A3(void), asm_FxOpd83(void);
extern void asm_FxOpd83A1(void), asm_FxOpd83A2(void), asm_FxOpd83A3(void), asm_FxOpd84(void);
extern void asm_FxOpd84A1(void), asm_FxOpd84A2(void), asm_FxOpd84A3(void), asm_FxOpd85(void);
extern void asm_FxOpd85A1(void), asm_FxOpd85A2(void), asm_FxOpd85A3(void), asm_FxOpd86(void);
extern void asm_FxOpd86A1(void), asm_FxOpd86A2(void), asm_FxOpd86A3(void), asm_FxOpd87(void);
extern void asm_FxOpd87A1(void), asm_FxOpd87A2(void), asm_FxOpd87A3(void), asm_FxOpd88(void);
extern void asm_FxOpd88A1(void), asm_FxOpd88A2(void), asm_FxOpd88A3(void), asm_FxOpd89(void);
extern void asm_FxOpd89A1(void), asm_FxOpd89A2(void), asm_FxOpd89A3(void), asm_FxOpd8A(void);
extern void asm_FxOpd8AA1(void), asm_FxOpd8AA2(void), asm_FxOpd8AA3(void), asm_FxOpd8B(void);
extern void asm_FxOpd8BA1(void), asm_FxOpd8BA2(void), asm_FxOpd8BA3(void), asm_FxOpd8C(void);
extern void asm_FxOpd8CA1(void), asm_FxOpd8CA2(void), asm_FxOpd8CA3(void), asm_FxOpd8D(void);
extern void asm_FxOpd8DA1(void), asm_FxOpd8DA2(void), asm_FxOpd8DA3(void), asm_FxOpd8E(void);
extern void asm_FxOpd8EA1(void), asm_FxOpd8EA2(void), asm_FxOpd8EA3(void), asm_FxOpd8F(void);
extern void asm_FxOpd8FA1(void), asm_FxOpd8FA2(void), asm_FxOpd8FA3(void), asm_FxOpd90(void);
extern void asm_FxOpd91(void), asm_FxOpd92(void), asm_FxOpd93(void), asm_FxOpd94(void);
extern void asm_FxOpd95(void), asm_FxOpd96(void), asm_FxOpd96A1(void), asm_FxOpd97(void);
extern void asm_FxOpd98(void), asm_FxOpd98A1(void), asm_FxOpd99(void), asm_FxOpd99A1(void);
extern void asm_FxOpd9A(void), asm_FxOpd9AA1(void), asm_FxOpd9B(void), asm_FxOpd9BA1(void);
extern void asm_FxOpd9C(void), asm_FxOpd9CA1(void), asm_FxOpd9D(void), asm_FxOpd9DA1(void);
extern void asm_FxOpd9E(void), asm_FxOpd9F(void), asm_FxOpd9FA1(void), asm_FxOpdA0(void);
extern void asm_FxOpdA0A1(void), asm_FxOpdA0A2(void), asm_FxOpdA1(void), asm_FxOpdA1A1(void);
extern void asm_FxOpdA1A2(void), asm_FxOpdA2(void), asm_FxOpdA2A1(void), asm_FxOpdA2A2(void);
extern void asm_FxOpdA3(void), asm_FxOpdA3A1(void), asm_FxOpdA3A2(void), asm_FxOpdA4(void);
extern void asm_FxOpdA4A1(void), asm_FxOpdA4A2(void), asm_FxOpdA5(void), asm_FxOpdA5A1(void);
extern void asm_FxOpdA5A2(void), asm_FxOpdA6(void), asm_FxOpdA6A1(void), asm_FxOpdA6A2(void);
extern void asm_FxOpdA7(void), asm_FxOpdA7A1(void), asm_FxOpdA7A2(void), asm_FxOpdA8(void);
extern void asm_FxOpdA8A1(void), asm_FxOpdA8A2(void), asm_FxOpdA9(void), asm_FxOpdA9A1(void);
extern void asm_FxOpdA9A2(void), asm_FxOpdAA(void), asm_FxOpdAAA1(void), asm_FxOpdAAA2(void);
extern void asm_FxOpdAB(void), asm_FxOpdABA1(void), asm_FxOpdABA2(void), asm_FxOpdAC(void);
extern void asm_FxOpdACA1(void), asm_FxOpdACA2(void), asm_FxOpdAD(void), asm_FxOpdADA1(void);
extern void asm_FxOpdADA2(void), asm_FxOpdAE(void), asm_FxOpdAEA1(void), asm_FxOpdAEA2(void);
extern void asm_FxOpdAF(void), asm_FxOpdAFA1(void), asm_FxOpdAFA2(void), asm_FxOpdB0(void);
extern void asm_FxOpdB1(void), asm_FxOpdB2(void), asm_FxOpdB3(void), asm_FxOpdB4(void);
extern void asm_FxOpdB5(void), asm_FxOpdB6(void), asm_FxOpdB7(void), asm_FxOpdB8(void);
extern void asm_FxOpdB9(void), asm_FxOpdBA(void), asm_FxOpdBB(void), asm_FxOpdBC(void);
extern void asm_FxOpdBD(void), asm_FxOpdBE(void), asm_FxOpdBF(void), asm_FxOpdC0(void);
extern void asm_FxOpdC1(void), asm_FxOpdC1A1(void), asm_FxOpdC1A2(void), asm_FxOpdC1A3(void);
extern void asm_FxOpdC2(void), asm_FxOpdC2A1(void), asm_FxOpdC2A2(void), asm_FxOpdC2A3(void);
extern void asm_FxOpdC3(void), asm_FxOpdC3A1(void), asm_FxOpdC3A2(void), asm_FxOpdC3A3(void);
extern void asm_FxOpdC4(void), asm_FxOpdC4A1(void), asm_FxOpdC4A2(void), asm_FxOpdC4A3(void);
extern void asm_FxOpdC5(void), asm_FxOpdC5A1(void), asm_FxOpdC5A2(void), asm_FxOpdC5A3(void);
extern void asm_FxOpdC6(void), asm_FxOpdC6A1(void), asm_FxOpdC6A2(void), asm_FxOpdC6A3(void);
extern void asm_FxOpdC7(void), asm_FxOpdC7A1(void), asm_FxOpdC7A2(void), asm_FxOpdC7A3(void);
extern void asm_FxOpdC8(void), asm_FxOpdC8A1(void), asm_FxOpdC8A2(void), asm_FxOpdC8A3(void);
extern void asm_FxOpdC9(void), asm_FxOpdC9A1(void), asm_FxOpdC9A2(void), asm_FxOpdC9A3(void);
extern void asm_FxOpdCA(void), asm_FxOpdCAA1(void), asm_FxOpdCAA2(void), asm_FxOpdCAA3(void);
extern void asm_FxOpdCB(void), asm_FxOpdCBA1(void), asm_FxOpdCBA2(void), asm_FxOpdCBA3(void);
extern void asm_FxOpdCC(void), asm_FxOpdCCA1(void), asm_FxOpdCCA2(void), asm_FxOpdCCA3(void);
extern void asm_FxOpdCD(void), asm_FxOpdCDA1(void), asm_FxOpdCDA2(void), asm_FxOpdCDA3(void);
extern void asm_FxOpdCE(void), asm_FxOpdCEA1(void), asm_FxOpdCEA2(void), asm_FxOpdCEA3(void);
extern void asm_FxOpdCF(void), asm_FxOpdCFA1(void), asm_FxOpdCFA2(void), asm_FxOpdCFA3(void);
extern void asm_FxOpdD0(void), asm_FxOpdD1(void), asm_FxOpdD2(void), asm_FxOpdD3(void);
extern void asm_FxOpdD4(void), asm_FxOpdD5(void), asm_FxOpdD6(void), asm_FxOpdD7(void);
extern void asm_FxOpdD8(void), asm_FxOpdD9(void), asm_FxOpdDA(void), asm_FxOpdDB(void);
extern void asm_FxOpdDC(void), asm_FxOpdDD(void), asm_FxOpdDE(void), asm_FxOpdDF(void);
extern void asm_FxOpdDFA2(void), asm_FxOpdDFA3(void), asm_FxOpdE0(void), asm_FxOpdE1(void);
extern void asm_FxOpdE2(void), asm_FxOpdE3(void), asm_FxOpdE4(void), asm_FxOpdE5(void);
extern void asm_FxOpdE6(void), asm_FxOpdE7(void), asm_FxOpdE8(void), asm_FxOpdE9(void);
extern void asm_FxOpdEA(void), asm_FxOpdEB(void), asm_FxOpdEC(void), asm_FxOpdED(void);
extern void asm_FxOpdEE(void), asm_FxOpdEF(void), asm_FxOpdEFA1(void), asm_FxOpdEFA2(void);
extern void asm_FxOpdEFA3(void), asm_FxOpdF0(void), asm_FxOpdF0A1(void), asm_FxOpdF0A2(void);
extern void asm_FxOpdF1(void), asm_FxOpdF1A1(void), asm_FxOpdF1A2(void), asm_FxOpdF2(void);
extern void asm_FxOpdF2A1(void), asm_FxOpdF2A2(void), asm_FxOpdF3(void), asm_FxOpdF3A1(void);
extern void asm_FxOpdF3A2(void), asm_FxOpdF4(void), asm_FxOpdF4A1(void), asm_FxOpdF4A2(void);
extern void asm_FxOpdF5(void), asm_FxOpdF5A1(void), asm_FxOpdF5A2(void), asm_FxOpdF6(void);
extern void asm_FxOpdF6A1(void), asm_FxOpdF6A2(void), asm_FxOpdF7(void), asm_FxOpdF7A1(void);
extern void asm_FxOpdF7A2(void), asm_FxOpdF8(void), asm_FxOpdF8A1(void), asm_FxOpdF8A2(void);
extern void asm_FxOpdF9(void), asm_FxOpdF9A1(void), asm_FxOpdF9A2(void), asm_FxOpdFA(void);
extern void asm_FxOpdFAA1(void), asm_FxOpdFAA2(void), asm_FxOpdFB(void), asm_FxOpdFBA1(void);
extern void asm_FxOpdFBA2(void), asm_FxOpdFC(void), asm_FxOpdFCA1(void), asm_FxOpdFCA2(void);
extern void asm_FxOpdFD(void), asm_FxOpdFDA1(void), asm_FxOpdFDA2(void), asm_FxOpdFE(void);
extern void asm_FxOpdFEA1(void), asm_FxOpdFEA2(void), asm_FxOpdFF(void), asm_FxOpdFFA1(void);
extern void asm_FxOpdFFA2(void);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* Where the handler gets an address, so the difftest can keep it in
       bounds: 0 = neither, 1 = RAM via register addr_reg, 2 = RAM via a 16-bit
       immediate at the program counter, 3 = RAM via an 8-bit immediate scaled
       by two (always in range), 4 = new program counter from addr_reg,
       5 = new program counter from the source register via SfxMemTable,
       6 = RAM via SfxLastRamAdr, which the setup already keeps in range. */
    int mem;
    u4 addr_reg;
    /* Set for the d table: the ported side is the bare body, so the caller has
       to add the FXReturn tail the oracle's thunk still carries. */
    int c_is_d;
} fxcase;

static fxcase const cases[] = {
    { "FxOpb05 BRA", asm_FxOpb05, c_FxOpb05, 0, 0, 0 },
    { "FxOpb06 BGE", asm_FxOpb06, c_FxOpb06, 0, 0, 0 },
    { "FxOpb07 BLT", asm_FxOpb07, c_FxOpb07, 0, 0, 0 },
    { "FxOpb08 BNE", asm_FxOpb08, c_FxOpb08, 0, 0, 0 },
    { "FxOpb09 BEQ", asm_FxOpb09, c_FxOpb09, 0, 0, 0 },
    { "FxOpb0A BPL", asm_FxOpb0A, c_FxOpb0A, 0, 0, 0 },
    { "FxOpb0B BMI", asm_FxOpb0B, c_FxOpb0B, 0, 0, 0 },
    { "FxOpb0C BCC", asm_FxOpb0C, c_FxOpb0C, 0, 0, 0 },
    { "FxOpb0D BCS", asm_FxOpb0D, c_FxOpb0D, 0, 0, 0 },
    { "FxOpb0E BVC", asm_FxOpb0E, c_FxOpb0E, 0, 0, 0 },
    { "FxOpb0F BVS", asm_FxOpb0F, c_FxOpb0F, 0, 0, 0 },
    { "FxOpb10 TO r0", asm_FxOpb10, c_FxOpb10, 0, 0, 0 },
    { "FxOpb11 TO r1", asm_FxOpb11, c_FxOpb11, 0, 0, 0 },
    { "FxOpb12 TO r2", asm_FxOpb12, c_FxOpb12, 0, 0, 0 },
    { "FxOpb13 TO r3", asm_FxOpb13, c_FxOpb13, 0, 0, 0 },
    { "FxOpb14 TO r4", asm_FxOpb14, c_FxOpb14, 0, 0, 0 },
    { "FxOpb15 TO r5", asm_FxOpb15, c_FxOpb15, 0, 0, 0 },
    { "FxOpb16 TO r6", asm_FxOpb16, c_FxOpb16, 0, 0, 0 },
    { "FxOpb17 TO r7", asm_FxOpb17, c_FxOpb17, 0, 0, 0 },
    { "FxOpb18 TO r8", asm_FxOpb18, c_FxOpb18, 0, 0, 0 },
    { "FxOpb19 TO r9", asm_FxOpb19, c_FxOpb19, 0, 0, 0 },
    { "FxOpb1A TO r10", asm_FxOpb1A, c_FxOpb1A, 0, 0, 0 },
    { "FxOpb1B TO r11", asm_FxOpb1B, c_FxOpb1B, 0, 0, 0 },
    { "FxOpb1C TO r12", asm_FxOpb1C, c_FxOpb1C, 0, 0, 0 },
    { "FxOpb1D TO r13", asm_FxOpb1D, c_FxOpb1D, 0, 0, 0 },
    { "FxOpb1E TO r14", asm_FxOpb1E, c_FxOpb1E, 0, 0, 0 },
    { "FxOpb1F TO r15", asm_FxOpb1F, c_FxOpb1F, 0, 0, 0 },
    { "FxOpb3D ALT1", asm_FxOpb3D, c_FxOpb3D, 0, 0, 0 },
    { "FxOpb3E ALT2", asm_FxOpb3E, c_FxOpb3E, 0, 0, 0 },
    { "FxOpb3F ALT3", asm_FxOpb3F, c_FxOpb3F, 0, 0, 0 },
    { "FxOpbB0 FROM r0", asm_FxOpbB0, c_FxOpbB0, 0, 0, 0 },
    { "FxOpbB1 FROM r1", asm_FxOpbB1, c_FxOpbB1, 0, 0, 0 },
    { "FxOpbB2 FROM r2", asm_FxOpbB2, c_FxOpbB2, 0, 0, 0 },
    { "FxOpbB3 FROM r3", asm_FxOpbB3, c_FxOpbB3, 0, 0, 0 },
    { "FxOpbB4 FROM r4", asm_FxOpbB4, c_FxOpbB4, 0, 0, 0 },
    { "FxOpbB5 FROM r5", asm_FxOpbB5, c_FxOpbB5, 0, 0, 0 },
    { "FxOpbB6 FROM r6", asm_FxOpbB6, c_FxOpbB6, 0, 0, 0 },
    { "FxOpbB7 FROM r7", asm_FxOpbB7, c_FxOpbB7, 0, 0, 0 },
    { "FxOpbB8 FROM r8", asm_FxOpbB8, c_FxOpbB8, 0, 0, 0 },
    { "FxOpbB9 FROM r9", asm_FxOpbB9, c_FxOpbB9, 0, 0, 0 },
    { "FxOpbBA FROM r10", asm_FxOpbBA, c_FxOpbBA, 0, 0, 0 },
    { "FxOpbBB FROM r11", asm_FxOpbBB, c_FxOpbBB, 0, 0, 0 },
    { "FxOpbBC FROM r12", asm_FxOpbBC, c_FxOpbBC, 0, 0, 0 },
    { "FxOpbBD FROM r13", asm_FxOpbBD, c_FxOpbBD, 0, 0, 0 },
    { "FxOpbBE FROM r14", asm_FxOpbBE, c_FxOpbBE, 0, 0, 0 },
    { "FxOpbBF FROM r15", asm_FxOpbBF, c_FxOpbBF, 0, 0, 0 },
    { "FxOpc05 BRA", asm_FxOpc05, c_FxOpc05, 0, 0, 0 },
    { "FxOpc06 BGE", asm_FxOpc06, c_FxOpc06, 0, 0, 0 },
    { "FxOpc07 BLT", asm_FxOpc07, c_FxOpc07, 0, 0, 0 },
    { "FxOpc08 BNE", asm_FxOpc08, c_FxOpc08, 0, 0, 0 },
    { "FxOpc09 BEQ", asm_FxOpc09, c_FxOpc09, 0, 0, 0 },
    { "FxOpc0A BPL", asm_FxOpc0A, c_FxOpc0A, 0, 0, 0 },
    { "FxOpc0B BMI", asm_FxOpc0B, c_FxOpc0B, 0, 0, 0 },
    { "FxOpc0C BCC", asm_FxOpc0C, c_FxOpc0C, 0, 0, 0 },
    { "FxOpc0D BCS", asm_FxOpc0D, c_FxOpc0D, 0, 0, 0 },
    { "FxOpc0E BVC", asm_FxOpc0E, c_FxOpc0E, 0, 0, 0 },
    { "FxOpc0F BVS", asm_FxOpc0F, c_FxOpc0F, 0, 0, 0 },
    { "FxOpc10 TO r0", asm_FxOpc10, c_FxOpc10, 0, 0, 0 },
    { "FxOpc11 TO r1", asm_FxOpc11, c_FxOpc11, 0, 0, 0 },
    { "FxOpc12 TO r2", asm_FxOpc12, c_FxOpc12, 0, 0, 0 },
    { "FxOpc13 TO r3", asm_FxOpc13, c_FxOpc13, 0, 0, 0 },
    { "FxOpc14 TO r4", asm_FxOpc14, c_FxOpc14, 0, 0, 0 },
    { "FxOpc15 TO r5", asm_FxOpc15, c_FxOpc15, 0, 0, 0 },
    { "FxOpc16 TO r6", asm_FxOpc16, c_FxOpc16, 0, 0, 0 },
    { "FxOpc17 TO r7", asm_FxOpc17, c_FxOpc17, 0, 0, 0 },
    { "FxOpc18 TO r8", asm_FxOpc18, c_FxOpc18, 0, 0, 0 },
    { "FxOpc19 TO r9", asm_FxOpc19, c_FxOpc19, 0, 0, 0 },
    { "FxOpc1A TO r10", asm_FxOpc1A, c_FxOpc1A, 0, 0, 0 },
    { "FxOpc1B TO r11", asm_FxOpc1B, c_FxOpc1B, 0, 0, 0 },
    { "FxOpc1C TO r12", asm_FxOpc1C, c_FxOpc1C, 0, 0, 0 },
    { "FxOpc1D TO r13", asm_FxOpc1D, c_FxOpc1D, 0, 0, 0 },
    { "FxOpc1E TO r14", asm_FxOpc1E, c_FxOpc1E, 0, 0, 0 },
    { "FxOpc1F TO r15", asm_FxOpc1F, c_FxOpc1F, 0, 0, 0 },
    { "FxOpc3D ALT1", asm_FxOpc3D, c_FxOpc3D, 0, 0, 0 },
    { "FxOpc3E ALT2", asm_FxOpc3E, c_FxOpc3E, 0, 0, 0 },
    { "FxOpc3F ALT3", asm_FxOpc3F, c_FxOpc3F, 0, 0, 0 },
    { "FxOpcB0 FROM r0", asm_FxOpcB0, c_FxOpcB0, 0, 0, 0 },
    { "FxOpcB1 FROM r1", asm_FxOpcB1, c_FxOpcB1, 0, 0, 0 },
    { "FxOpcB2 FROM r2", asm_FxOpcB2, c_FxOpcB2, 0, 0, 0 },
    { "FxOpcB3 FROM r3", asm_FxOpcB3, c_FxOpcB3, 0, 0, 0 },
    { "FxOpcB4 FROM r4", asm_FxOpcB4, c_FxOpcB4, 0, 0, 0 },
    { "FxOpcB5 FROM r5", asm_FxOpcB5, c_FxOpcB5, 0, 0, 0 },
    { "FxOpcB6 FROM r6", asm_FxOpcB6, c_FxOpcB6, 0, 0, 0 },
    { "FxOpcB7 FROM r7", asm_FxOpcB7, c_FxOpcB7, 0, 0, 0 },
    { "FxOpcB8 FROM r8", asm_FxOpcB8, c_FxOpcB8, 0, 0, 0 },
    { "FxOpcB9 FROM r9", asm_FxOpcB9, c_FxOpcB9, 0, 0, 0 },
    { "FxOpcBA FROM r10", asm_FxOpcBA, c_FxOpcBA, 0, 0, 0 },
    { "FxOpcBB FROM r11", asm_FxOpcBB, c_FxOpcBB, 0, 0, 0 },
    { "FxOpcBC FROM r12", asm_FxOpcBC, c_FxOpcBC, 0, 0, 0 },
    { "FxOpcBD FROM r13", asm_FxOpcBD, c_FxOpcBD, 0, 0, 0 },
    { "FxOpcBE FROM r14", asm_FxOpcBE, c_FxOpcBE, 0, 0, 0 },
    { "FxOpcBF FROM r15", asm_FxOpcBF, c_FxOpcBF, 0, 0, 0 },
    { "FxOp50 ADD r0", asm_FxOp50, c_FxOp50, 0, 0, 0 },
    { "FxOp51 ADD r1", asm_FxOp51, c_FxOp51, 0, 0, 0 },
    { "FxOp52 ADD r2", asm_FxOp52, c_FxOp52, 0, 0, 0 },
    { "FxOp53 ADD r3", asm_FxOp53, c_FxOp53, 0, 0, 0 },
    { "FxOp54 ADD r4", asm_FxOp54, c_FxOp54, 0, 0, 0 },
    { "FxOp55 ADD r5", asm_FxOp55, c_FxOp55, 0, 0, 0 },
    { "FxOp56 ADD r6", asm_FxOp56, c_FxOp56, 0, 0, 0 },
    { "FxOp57 ADD r7", asm_FxOp57, c_FxOp57, 0, 0, 0 },
    { "FxOp58 ADD r8", asm_FxOp58, c_FxOp58, 0, 0, 0 },
    { "FxOp59 ADD r9", asm_FxOp59, c_FxOp59, 0, 0, 0 },
    { "FxOp5A ADD r10", asm_FxOp5A, c_FxOp5A, 0, 0, 0 },
    { "FxOp5B ADD r11", asm_FxOp5B, c_FxOp5B, 0, 0, 0 },
    { "FxOp5C ADD r12", asm_FxOp5C, c_FxOp5C, 0, 0, 0 },
    { "FxOp5D ADD r13", asm_FxOp5D, c_FxOp5D, 0, 0, 0 },
    { "FxOp5E ADD r14", asm_FxOp5E, c_FxOp5E, 0, 0, 0 },
    { "FxOp50A1 ADC r0", asm_FxOp50A1, c_FxOp50A1, 0, 0, 0 },
    { "FxOp51A1 ADC r1", asm_FxOp51A1, c_FxOp51A1, 0, 0, 0 },
    { "FxOp52A1 ADC r2", asm_FxOp52A1, c_FxOp52A1, 0, 0, 0 },
    { "FxOp53A1 ADC r3", asm_FxOp53A1, c_FxOp53A1, 0, 0, 0 },
    { "FxOp54A1 ADC r4", asm_FxOp54A1, c_FxOp54A1, 0, 0, 0 },
    { "FxOp55A1 ADC r5", asm_FxOp55A1, c_FxOp55A1, 0, 0, 0 },
    { "FxOp56A1 ADC r6", asm_FxOp56A1, c_FxOp56A1, 0, 0, 0 },
    { "FxOp57A1 ADC r7", asm_FxOp57A1, c_FxOp57A1, 0, 0, 0 },
    { "FxOp58A1 ADC r8", asm_FxOp58A1, c_FxOp58A1, 0, 0, 0 },
    { "FxOp59A1 ADC r9", asm_FxOp59A1, c_FxOp59A1, 0, 0, 0 },
    { "FxOp5AA1 ADC r10", asm_FxOp5AA1, c_FxOp5AA1, 0, 0, 0 },
    { "FxOp5BA1 ADC r11", asm_FxOp5BA1, c_FxOp5BA1, 0, 0, 0 },
    { "FxOp5CA1 ADC r12", asm_FxOp5CA1, c_FxOp5CA1, 0, 0, 0 },
    { "FxOp5DA1 ADC r13", asm_FxOp5DA1, c_FxOp5DA1, 0, 0, 0 },
    { "FxOp5EA1 ADC r14", asm_FxOp5EA1, c_FxOp5EA1, 0, 0, 0 },
    { "FxOp50A2 ADD #0", asm_FxOp50A2, c_FxOp50A2, 0, 0, 0 },
    { "FxOp51A2 ADD #1", asm_FxOp51A2, c_FxOp51A2, 0, 0, 0 },
    { "FxOp52A2 ADD #2", asm_FxOp52A2, c_FxOp52A2, 0, 0, 0 },
    { "FxOp53A2 ADD #3", asm_FxOp53A2, c_FxOp53A2, 0, 0, 0 },
    { "FxOp54A2 ADD #4", asm_FxOp54A2, c_FxOp54A2, 0, 0, 0 },
    { "FxOp55A2 ADD #5", asm_FxOp55A2, c_FxOp55A2, 0, 0, 0 },
    { "FxOp56A2 ADD #6", asm_FxOp56A2, c_FxOp56A2, 0, 0, 0 },
    { "FxOp57A2 ADD #7", asm_FxOp57A2, c_FxOp57A2, 0, 0, 0 },
    { "FxOp58A2 ADD #8", asm_FxOp58A2, c_FxOp58A2, 0, 0, 0 },
    { "FxOp59A2 ADD #9", asm_FxOp59A2, c_FxOp59A2, 0, 0, 0 },
    { "FxOp5AA2 ADD #10", asm_FxOp5AA2, c_FxOp5AA2, 0, 0, 0 },
    { "FxOp5BA2 ADD #11", asm_FxOp5BA2, c_FxOp5BA2, 0, 0, 0 },
    { "FxOp5CA2 ADD #12", asm_FxOp5CA2, c_FxOp5CA2, 0, 0, 0 },
    { "FxOp5DA2 ADD #13", asm_FxOp5DA2, c_FxOp5DA2, 0, 0, 0 },
    { "FxOp5EA2 ADD #14", asm_FxOp5EA2, c_FxOp5EA2, 0, 0, 0 },
    { "FxOp5FA2 ADD #15", asm_FxOp5FA2, c_FxOp5FA2, 0, 0, 0 },
    { "FxOp50A3 ADC #0", asm_FxOp50A3, c_FxOp50A3, 0, 0, 0 },
    { "FxOp51A3 ADC #1", asm_FxOp51A3, c_FxOp51A3, 0, 0, 0 },
    { "FxOp52A3 ADC #2", asm_FxOp52A3, c_FxOp52A3, 0, 0, 0 },
    { "FxOp53A3 ADC #3", asm_FxOp53A3, c_FxOp53A3, 0, 0, 0 },
    { "FxOp54A3 ADC #4", asm_FxOp54A3, c_FxOp54A3, 0, 0, 0 },
    { "FxOp55A3 ADC #5", asm_FxOp55A3, c_FxOp55A3, 0, 0, 0 },
    { "FxOp56A3 ADC #6", asm_FxOp56A3, c_FxOp56A3, 0, 0, 0 },
    { "FxOp57A3 ADC #7", asm_FxOp57A3, c_FxOp57A3, 0, 0, 0 },
    { "FxOp58A3 ADC #8", asm_FxOp58A3, c_FxOp58A3, 0, 0, 0 },
    { "FxOp59A3 ADC #9", asm_FxOp59A3, c_FxOp59A3, 0, 0, 0 },
    { "FxOp5AA3 ADC #10", asm_FxOp5AA3, c_FxOp5AA3, 0, 0, 0 },
    { "FxOp5BA3 ADC #11", asm_FxOp5BA3, c_FxOp5BA3, 0, 0, 0 },
    { "FxOp5CA3 ADC #12", asm_FxOp5CA3, c_FxOp5CA3, 0, 0, 0 },
    { "FxOp5DA3 ADC #13", asm_FxOp5DA3, c_FxOp5DA3, 0, 0, 0 },
    { "FxOp5EA3 ADC #14", asm_FxOp5EA3, c_FxOp5EA3, 0, 0, 0 },
    { "FxOp5FA3 ADC #15", asm_FxOp5FA3, c_FxOp5FA3, 0, 0, 0 },
    { "FxOp60 SUB r0", asm_FxOp60, c_FxOp60, 0, 0, 0 },
    { "FxOp61 SUB r1", asm_FxOp61, c_FxOp61, 0, 0, 0 },
    { "FxOp62 SUB r2", asm_FxOp62, c_FxOp62, 0, 0, 0 },
    { "FxOp63 SUB r3", asm_FxOp63, c_FxOp63, 0, 0, 0 },
    { "FxOp64 SUB r4", asm_FxOp64, c_FxOp64, 0, 0, 0 },
    { "FxOp65 SUB r5", asm_FxOp65, c_FxOp65, 0, 0, 0 },
    { "FxOp66 SUB r6", asm_FxOp66, c_FxOp66, 0, 0, 0 },
    { "FxOp67 SUB r7", asm_FxOp67, c_FxOp67, 0, 0, 0 },
    { "FxOp68 SUB r8", asm_FxOp68, c_FxOp68, 0, 0, 0 },
    { "FxOp69 SUB r9", asm_FxOp69, c_FxOp69, 0, 0, 0 },
    { "FxOp6A SUB r10", asm_FxOp6A, c_FxOp6A, 0, 0, 0 },
    { "FxOp6B SUB r11", asm_FxOp6B, c_FxOp6B, 0, 0, 0 },
    { "FxOp6C SUB r12", asm_FxOp6C, c_FxOp6C, 0, 0, 0 },
    { "FxOp6D SUB r13", asm_FxOp6D, c_FxOp6D, 0, 0, 0 },
    { "FxOp6E SUB r14", asm_FxOp6E, c_FxOp6E, 0, 0, 0 },
    { "FxOp60A1 SBC r0", asm_FxOp60A1, c_FxOp60A1, 0, 0, 0 },
    { "FxOp61A1 SBC r1", asm_FxOp61A1, c_FxOp61A1, 0, 0, 0 },
    { "FxOp62A1 SBC r2", asm_FxOp62A1, c_FxOp62A1, 0, 0, 0 },
    { "FxOp63A1 SBC r3", asm_FxOp63A1, c_FxOp63A1, 0, 0, 0 },
    { "FxOp64A1 SBC r4", asm_FxOp64A1, c_FxOp64A1, 0, 0, 0 },
    { "FxOp65A1 SBC r5", asm_FxOp65A1, c_FxOp65A1, 0, 0, 0 },
    { "FxOp66A1 SBC r6", asm_FxOp66A1, c_FxOp66A1, 0, 0, 0 },
    { "FxOp67A1 SBC r7", asm_FxOp67A1, c_FxOp67A1, 0, 0, 0 },
    { "FxOp68A1 SBC r8", asm_FxOp68A1, c_FxOp68A1, 0, 0, 0 },
    { "FxOp69A1 SBC r9", asm_FxOp69A1, c_FxOp69A1, 0, 0, 0 },
    { "FxOp6AA1 SBC r10", asm_FxOp6AA1, c_FxOp6AA1, 0, 0, 0 },
    { "FxOp6BA1 SBC r11", asm_FxOp6BA1, c_FxOp6BA1, 0, 0, 0 },
    { "FxOp6CA1 SBC r12", asm_FxOp6CA1, c_FxOp6CA1, 0, 0, 0 },
    { "FxOp6DA1 SBC r13", asm_FxOp6DA1, c_FxOp6DA1, 0, 0, 0 },
    { "FxOp6EA1 SBC r14", asm_FxOp6EA1, c_FxOp6EA1, 0, 0, 0 },
    { "FxOp60A2 SUB #0", asm_FxOp60A2, c_FxOp60A2, 0, 0, 0 },
    { "FxOp61A2 SUB #1", asm_FxOp61A2, c_FxOp61A2, 0, 0, 0 },
    { "FxOp62A2 SUB #2", asm_FxOp62A2, c_FxOp62A2, 0, 0, 0 },
    { "FxOp63A2 SUB #3", asm_FxOp63A2, c_FxOp63A2, 0, 0, 0 },
    { "FxOp64A2 SUB #4", asm_FxOp64A2, c_FxOp64A2, 0, 0, 0 },
    { "FxOp65A2 SUB #5", asm_FxOp65A2, c_FxOp65A2, 0, 0, 0 },
    { "FxOp66A2 SUB #6", asm_FxOp66A2, c_FxOp66A2, 0, 0, 0 },
    { "FxOp67A2 SUB #7", asm_FxOp67A2, c_FxOp67A2, 0, 0, 0 },
    { "FxOp68A2 SUB #8", asm_FxOp68A2, c_FxOp68A2, 0, 0, 0 },
    { "FxOp69A2 SUB #9", asm_FxOp69A2, c_FxOp69A2, 0, 0, 0 },
    { "FxOp6AA2 SUB #10", asm_FxOp6AA2, c_FxOp6AA2, 0, 0, 0 },
    { "FxOp6BA2 SUB #11", asm_FxOp6BA2, c_FxOp6BA2, 0, 0, 0 },
    { "FxOp6CA2 SUB #12", asm_FxOp6CA2, c_FxOp6CA2, 0, 0, 0 },
    { "FxOp6DA2 SUB #13", asm_FxOp6DA2, c_FxOp6DA2, 0, 0, 0 },
    { "FxOp6EA2 SUB #14", asm_FxOp6EA2, c_FxOp6EA2, 0, 0, 0 },
    { "FxOp6FA2 SUB #15", asm_FxOp6FA2, c_FxOp6FA2, 0, 0, 0 },
    { "FxOp60A3 CMP r0", asm_FxOp60A3, c_FxOp60A3, 0, 0, 0 },
    { "FxOp61A3 CMP r1", asm_FxOp61A3, c_FxOp61A3, 0, 0, 0 },
    { "FxOp62A3 CMP r2", asm_FxOp62A3, c_FxOp62A3, 0, 0, 0 },
    { "FxOp63A3 CMP r3", asm_FxOp63A3, c_FxOp63A3, 0, 0, 0 },
    { "FxOp64A3 CMP r4", asm_FxOp64A3, c_FxOp64A3, 0, 0, 0 },
    { "FxOp65A3 CMP r5", asm_FxOp65A3, c_FxOp65A3, 0, 0, 0 },
    { "FxOp66A3 CMP r6", asm_FxOp66A3, c_FxOp66A3, 0, 0, 0 },
    { "FxOp67A3 CMP r7", asm_FxOp67A3, c_FxOp67A3, 0, 0, 0 },
    { "FxOp68A3 CMP r8", asm_FxOp68A3, c_FxOp68A3, 0, 0, 0 },
    { "FxOp69A3 CMP r9", asm_FxOp69A3, c_FxOp69A3, 0, 0, 0 },
    { "FxOp6AA3 CMP r10", asm_FxOp6AA3, c_FxOp6AA3, 0, 0, 0 },
    { "FxOp6BA3 CMP r11", asm_FxOp6BA3, c_FxOp6BA3, 0, 0, 0 },
    { "FxOp6CA3 CMP r12", asm_FxOp6CA3, c_FxOp6CA3, 0, 0, 0 },
    { "FxOp6DA3 CMP r13", asm_FxOp6DA3, c_FxOp6DA3, 0, 0, 0 },
    { "FxOp6EA3 CMP r14", asm_FxOp6EA3, c_FxOp6EA3, 0, 0, 0 },
    { "FxOp71 AND r1", asm_FxOp71, c_FxOp71, 0, 0, 0 },
    { "FxOp72 AND r2", asm_FxOp72, c_FxOp72, 0, 0, 0 },
    { "FxOp73 AND r3", asm_FxOp73, c_FxOp73, 0, 0, 0 },
    { "FxOp74 AND r4", asm_FxOp74, c_FxOp74, 0, 0, 0 },
    { "FxOp75 AND r5", asm_FxOp75, c_FxOp75, 0, 0, 0 },
    { "FxOp76 AND r6", asm_FxOp76, c_FxOp76, 0, 0, 0 },
    { "FxOp77 AND r7", asm_FxOp77, c_FxOp77, 0, 0, 0 },
    { "FxOp78 AND r8", asm_FxOp78, c_FxOp78, 0, 0, 0 },
    { "FxOp79 AND r9", asm_FxOp79, c_FxOp79, 0, 0, 0 },
    { "FxOp7A AND r10", asm_FxOp7A, c_FxOp7A, 0, 0, 0 },
    { "FxOp7B AND r11", asm_FxOp7B, c_FxOp7B, 0, 0, 0 },
    { "FxOp7C AND r12", asm_FxOp7C, c_FxOp7C, 0, 0, 0 },
    { "FxOp7D AND r13", asm_FxOp7D, c_FxOp7D, 0, 0, 0 },
    { "FxOp7E AND r14", asm_FxOp7E, c_FxOp7E, 0, 0, 0 },
    { "FxOp71A1 BIC r1", asm_FxOp71A1, c_FxOp71A1, 0, 0, 0 },
    { "FxOp72A1 BIC r2", asm_FxOp72A1, c_FxOp72A1, 0, 0, 0 },
    { "FxOp73A1 BIC r3", asm_FxOp73A1, c_FxOp73A1, 0, 0, 0 },
    { "FxOp74A1 BIC r4", asm_FxOp74A1, c_FxOp74A1, 0, 0, 0 },
    { "FxOp75A1 BIC r5", asm_FxOp75A1, c_FxOp75A1, 0, 0, 0 },
    { "FxOp76A1 BIC r6", asm_FxOp76A1, c_FxOp76A1, 0, 0, 0 },
    { "FxOp77A1 BIC r7", asm_FxOp77A1, c_FxOp77A1, 0, 0, 0 },
    { "FxOp78A1 BIC r8", asm_FxOp78A1, c_FxOp78A1, 0, 0, 0 },
    { "FxOp79A1 BIC r9", asm_FxOp79A1, c_FxOp79A1, 0, 0, 0 },
    { "FxOp7AA1 BIC r10", asm_FxOp7AA1, c_FxOp7AA1, 0, 0, 0 },
    { "FxOp7BA1 BIC r11", asm_FxOp7BA1, c_FxOp7BA1, 0, 0, 0 },
    { "FxOp7CA1 BIC r12", asm_FxOp7CA1, c_FxOp7CA1, 0, 0, 0 },
    { "FxOp7DA1 BIC r13", asm_FxOp7DA1, c_FxOp7DA1, 0, 0, 0 },
    { "FxOp7EA1 BIC r14", asm_FxOp7EA1, c_FxOp7EA1, 0, 0, 0 },
    { "FxOp71A2 AND #1", asm_FxOp71A2, c_FxOp71A2, 0, 0, 0 },
    { "FxOp72A2 AND #2", asm_FxOp72A2, c_FxOp72A2, 0, 0, 0 },
    { "FxOp73A2 AND #3", asm_FxOp73A2, c_FxOp73A2, 0, 0, 0 },
    { "FxOp74A2 AND #4", asm_FxOp74A2, c_FxOp74A2, 0, 0, 0 },
    { "FxOp75A2 AND #5", asm_FxOp75A2, c_FxOp75A2, 0, 0, 0 },
    { "FxOp76A2 AND #6", asm_FxOp76A2, c_FxOp76A2, 0, 0, 0 },
    { "FxOp77A2 AND #7", asm_FxOp77A2, c_FxOp77A2, 0, 0, 0 },
    { "FxOp78A2 AND #8", asm_FxOp78A2, c_FxOp78A2, 0, 0, 0 },
    { "FxOp79A2 AND #9", asm_FxOp79A2, c_FxOp79A2, 0, 0, 0 },
    { "FxOp7AA2 AND #10", asm_FxOp7AA2, c_FxOp7AA2, 0, 0, 0 },
    { "FxOp7BA2 AND #11", asm_FxOp7BA2, c_FxOp7BA2, 0, 0, 0 },
    { "FxOp7CA2 AND #12", asm_FxOp7CA2, c_FxOp7CA2, 0, 0, 0 },
    { "FxOp7DA2 AND #13", asm_FxOp7DA2, c_FxOp7DA2, 0, 0, 0 },
    { "FxOp7EA2 AND #14", asm_FxOp7EA2, c_FxOp7EA2, 0, 0, 0 },
    { "FxOp7FA2 AND #15", asm_FxOp7FA2, c_FxOp7FA2, 0, 0, 0 },
    { "FxOp71A3 BIC #1", asm_FxOp71A3, c_FxOp71A3, 0, 0, 0 },
    { "FxOp72A3 BIC #2", asm_FxOp72A3, c_FxOp72A3, 0, 0, 0 },
    { "FxOp73A3 BIC #3", asm_FxOp73A3, c_FxOp73A3, 0, 0, 0 },
    { "FxOp74A3 BIC #4", asm_FxOp74A3, c_FxOp74A3, 0, 0, 0 },
    { "FxOp75A3 BIC #5", asm_FxOp75A3, c_FxOp75A3, 0, 0, 0 },
    { "FxOp76A3 BIC #6", asm_FxOp76A3, c_FxOp76A3, 0, 0, 0 },
    { "FxOp77A3 BIC #7", asm_FxOp77A3, c_FxOp77A3, 0, 0, 0 },
    { "FxOp78A3 BIC #8", asm_FxOp78A3, c_FxOp78A3, 0, 0, 0 },
    { "FxOp79A3 BIC #9", asm_FxOp79A3, c_FxOp79A3, 0, 0, 0 },
    { "FxOp7AA3 BIC #10", asm_FxOp7AA3, c_FxOp7AA3, 0, 0, 0 },
    { "FxOp7BA3 BIC #11", asm_FxOp7BA3, c_FxOp7BA3, 0, 0, 0 },
    { "FxOp7CA3 BIC #12", asm_FxOp7CA3, c_FxOp7CA3, 0, 0, 0 },
    { "FxOp7DA3 BIC #13", asm_FxOp7DA3, c_FxOp7DA3, 0, 0, 0 },
    { "FxOp7EA3 BIC #14", asm_FxOp7EA3, c_FxOp7EA3, 0, 0, 0 },
    { "FxOp7FA3 BIC #15", asm_FxOp7FA3, c_FxOp7FA3, 0, 0, 0 },
    { "FxOpC1 OR r1", asm_FxOpC1, c_FxOpC1, 0, 0, 0 },
    { "FxOpC2 OR r2", asm_FxOpC2, c_FxOpC2, 0, 0, 0 },
    { "FxOpC3 OR r3", asm_FxOpC3, c_FxOpC3, 0, 0, 0 },
    { "FxOpC4 OR r4", asm_FxOpC4, c_FxOpC4, 0, 0, 0 },
    { "FxOpC5 OR r5", asm_FxOpC5, c_FxOpC5, 0, 0, 0 },
    { "FxOpC6 OR r6", asm_FxOpC6, c_FxOpC6, 0, 0, 0 },
    { "FxOpC7 OR r7", asm_FxOpC7, c_FxOpC7, 0, 0, 0 },
    { "FxOpC8 OR r8", asm_FxOpC8, c_FxOpC8, 0, 0, 0 },
    { "FxOpC9 OR r9", asm_FxOpC9, c_FxOpC9, 0, 0, 0 },
    { "FxOpCA OR r10", asm_FxOpCA, c_FxOpCA, 0, 0, 0 },
    { "FxOpCB OR r11", asm_FxOpCB, c_FxOpCB, 0, 0, 0 },
    { "FxOpCC OR r12", asm_FxOpCC, c_FxOpCC, 0, 0, 0 },
    { "FxOpCD OR r13", asm_FxOpCD, c_FxOpCD, 0, 0, 0 },
    { "FxOpCE OR r14", asm_FxOpCE, c_FxOpCE, 0, 0, 0 },
    { "FxOpC1A1 XOR r1", asm_FxOpC1A1, c_FxOpC1A1, 0, 0, 0 },
    { "FxOpC2A1 XOR r2", asm_FxOpC2A1, c_FxOpC2A1, 0, 0, 0 },
    { "FxOpC3A1 XOR r3", asm_FxOpC3A1, c_FxOpC3A1, 0, 0, 0 },
    { "FxOpC4A1 XOR r4", asm_FxOpC4A1, c_FxOpC4A1, 0, 0, 0 },
    { "FxOpC5A1 XOR r5", asm_FxOpC5A1, c_FxOpC5A1, 0, 0, 0 },
    { "FxOpC6A1 XOR r6", asm_FxOpC6A1, c_FxOpC6A1, 0, 0, 0 },
    { "FxOpC7A1 XOR r7", asm_FxOpC7A1, c_FxOpC7A1, 0, 0, 0 },
    { "FxOpC8A1 XOR r8", asm_FxOpC8A1, c_FxOpC8A1, 0, 0, 0 },
    { "FxOpC9A1 XOR r9", asm_FxOpC9A1, c_FxOpC9A1, 0, 0, 0 },
    { "FxOpCAA1 XOR r10", asm_FxOpCAA1, c_FxOpCAA1, 0, 0, 0 },
    { "FxOpCBA1 XOR r11", asm_FxOpCBA1, c_FxOpCBA1, 0, 0, 0 },
    { "FxOpCCA1 XOR r12", asm_FxOpCCA1, c_FxOpCCA1, 0, 0, 0 },
    { "FxOpCDA1 XOR r13", asm_FxOpCDA1, c_FxOpCDA1, 0, 0, 0 },
    { "FxOpCEA1 XOR r14", asm_FxOpCEA1, c_FxOpCEA1, 0, 0, 0 },
    { "FxOpC1A2 OR #1", asm_FxOpC1A2, c_FxOpC1A2, 0, 0, 0 },
    { "FxOpC2A2 OR #2", asm_FxOpC2A2, c_FxOpC2A2, 0, 0, 0 },
    { "FxOpC3A2 OR #3", asm_FxOpC3A2, c_FxOpC3A2, 0, 0, 0 },
    { "FxOpC4A2 OR #4", asm_FxOpC4A2, c_FxOpC4A2, 0, 0, 0 },
    { "FxOpC5A2 OR #5", asm_FxOpC5A2, c_FxOpC5A2, 0, 0, 0 },
    { "FxOpC6A2 OR #6", asm_FxOpC6A2, c_FxOpC6A2, 0, 0, 0 },
    { "FxOpC7A2 OR #7", asm_FxOpC7A2, c_FxOpC7A2, 0, 0, 0 },
    { "FxOpC8A2 OR #8", asm_FxOpC8A2, c_FxOpC8A2, 0, 0, 0 },
    { "FxOpC9A2 OR #9", asm_FxOpC9A2, c_FxOpC9A2, 0, 0, 0 },
    { "FxOpCAA2 OR #10", asm_FxOpCAA2, c_FxOpCAA2, 0, 0, 0 },
    { "FxOpCBA2 OR #11", asm_FxOpCBA2, c_FxOpCBA2, 0, 0, 0 },
    { "FxOpCCA2 OR #12", asm_FxOpCCA2, c_FxOpCCA2, 0, 0, 0 },
    { "FxOpCDA2 OR #13", asm_FxOpCDA2, c_FxOpCDA2, 0, 0, 0 },
    { "FxOpCEA2 OR #14", asm_FxOpCEA2, c_FxOpCEA2, 0, 0, 0 },
    { "FxOpCFA2 OR #15", asm_FxOpCFA2, c_FxOpCFA2, 0, 0, 0 },
    { "FxOpC1A3 XOR #1", asm_FxOpC1A3, c_FxOpC1A3, 0, 0, 0 },
    { "FxOpC2A3 XOR #2", asm_FxOpC2A3, c_FxOpC2A3, 0, 0, 0 },
    { "FxOpC3A3 XOR #3", asm_FxOpC3A3, c_FxOpC3A3, 0, 0, 0 },
    { "FxOpC4A3 XOR #4", asm_FxOpC4A3, c_FxOpC4A3, 0, 0, 0 },
    { "FxOpC5A3 XOR #5", asm_FxOpC5A3, c_FxOpC5A3, 0, 0, 0 },
    { "FxOpC6A3 XOR #6", asm_FxOpC6A3, c_FxOpC6A3, 0, 0, 0 },
    { "FxOpC7A3 XOR #7", asm_FxOpC7A3, c_FxOpC7A3, 0, 0, 0 },
    { "FxOpC8A3 XOR #8", asm_FxOpC8A3, c_FxOpC8A3, 0, 0, 0 },
    { "FxOpC9A3 XOR #9", asm_FxOpC9A3, c_FxOpC9A3, 0, 0, 0 },
    { "FxOpCAA3 XOR #10", asm_FxOpCAA3, c_FxOpCAA3, 0, 0, 0 },
    { "FxOpCBA3 XOR #11", asm_FxOpCBA3, c_FxOpCBA3, 0, 0, 0 },
    { "FxOpCCA3 XOR #12", asm_FxOpCCA3, c_FxOpCCA3, 0, 0, 0 },
    { "FxOpCDA3 XOR #13", asm_FxOpCDA3, c_FxOpCDA3, 0, 0, 0 },
    { "FxOpCEA3 XOR #14", asm_FxOpCEA3, c_FxOpCEA3, 0, 0, 0 },
    { "FxOpCFA3 XOR #15", asm_FxOpCFA3, c_FxOpCFA3, 0, 0, 0 },
    { "FxOpD0 INC r0", asm_FxOpD0, c_FxOpD0, 0, 0, 0 },
    { "FxOpD1 INC r1", asm_FxOpD1, c_FxOpD1, 0, 0, 0 },
    { "FxOpD2 INC r2", asm_FxOpD2, c_FxOpD2, 0, 0, 0 },
    { "FxOpD3 INC r3", asm_FxOpD3, c_FxOpD3, 0, 0, 0 },
    { "FxOpD4 INC r4", asm_FxOpD4, c_FxOpD4, 0, 0, 0 },
    { "FxOpD5 INC r5", asm_FxOpD5, c_FxOpD5, 0, 0, 0 },
    { "FxOpD6 INC r6", asm_FxOpD6, c_FxOpD6, 0, 0, 0 },
    { "FxOpD7 INC r7", asm_FxOpD7, c_FxOpD7, 0, 0, 0 },
    { "FxOpD8 INC r8", asm_FxOpD8, c_FxOpD8, 0, 0, 0 },
    { "FxOpD9 INC r9", asm_FxOpD9, c_FxOpD9, 0, 0, 0 },
    { "FxOpDA INC r10", asm_FxOpDA, c_FxOpDA, 0, 0, 0 },
    { "FxOpDB INC r11", asm_FxOpDB, c_FxOpDB, 0, 0, 0 },
    { "FxOpDC INC r12", asm_FxOpDC, c_FxOpDC, 0, 0, 0 },
    { "FxOpDD INC r13", asm_FxOpDD, c_FxOpDD, 0, 0, 0 },
    { "FxOpE0 DEC r0", asm_FxOpE0, c_FxOpE0, 0, 0, 0 },
    { "FxOpE1 DEC r1", asm_FxOpE1, c_FxOpE1, 0, 0, 0 },
    { "FxOpE2 DEC r2", asm_FxOpE2, c_FxOpE2, 0, 0, 0 },
    { "FxOpE3 DEC r3", asm_FxOpE3, c_FxOpE3, 0, 0, 0 },
    { "FxOpE4 DEC r4", asm_FxOpE4, c_FxOpE4, 0, 0, 0 },
    { "FxOpE5 DEC r5", asm_FxOpE5, c_FxOpE5, 0, 0, 0 },
    { "FxOpE6 DEC r6", asm_FxOpE6, c_FxOpE6, 0, 0, 0 },
    { "FxOpE7 DEC r7", asm_FxOpE7, c_FxOpE7, 0, 0, 0 },
    { "FxOpE8 DEC r8", asm_FxOpE8, c_FxOpE8, 0, 0, 0 },
    { "FxOpE9 DEC r9", asm_FxOpE9, c_FxOpE9, 0, 0, 0 },
    { "FxOpEA DEC r10", asm_FxOpEA, c_FxOpEA, 0, 0, 0 },
    { "FxOpEB DEC r11", asm_FxOpEB, c_FxOpEB, 0, 0, 0 },
    { "FxOpEC DEC r12", asm_FxOpEC, c_FxOpEC, 0, 0, 0 },
    { "FxOpED DEC r13", asm_FxOpED, c_FxOpED, 0, 0, 0 },
    { "FxOp80 MULT r0", asm_FxOp80, c_FxOp80, 0, 0, 0 },
    { "FxOp81 MULT r1", asm_FxOp81, c_FxOp81, 0, 0, 0 },
    { "FxOp82 MULT r2", asm_FxOp82, c_FxOp82, 0, 0, 0 },
    { "FxOp83 MULT r3", asm_FxOp83, c_FxOp83, 0, 0, 0 },
    { "FxOp84 MULT r4", asm_FxOp84, c_FxOp84, 0, 0, 0 },
    { "FxOp85 MULT r5", asm_FxOp85, c_FxOp85, 0, 0, 0 },
    { "FxOp86 MULT r6", asm_FxOp86, c_FxOp86, 0, 0, 0 },
    { "FxOp87 MULT r7", asm_FxOp87, c_FxOp87, 0, 0, 0 },
    { "FxOp88 MULT r8", asm_FxOp88, c_FxOp88, 0, 0, 0 },
    { "FxOp89 MULT r9", asm_FxOp89, c_FxOp89, 0, 0, 0 },
    { "FxOp8A MULT r10", asm_FxOp8A, c_FxOp8A, 0, 0, 0 },
    { "FxOp8B MULT r11", asm_FxOp8B, c_FxOp8B, 0, 0, 0 },
    { "FxOp8C MULT r12", asm_FxOp8C, c_FxOp8C, 0, 0, 0 },
    { "FxOp8D MULT r13", asm_FxOp8D, c_FxOp8D, 0, 0, 0 },
    { "FxOp8E MULT r14", asm_FxOp8E, c_FxOp8E, 0, 0, 0 },
    { "FxOp80A1 UMULT r0", asm_FxOp80A1, c_FxOp80A1, 0, 0, 0 },
    { "FxOp81A1 UMULT r1", asm_FxOp81A1, c_FxOp81A1, 0, 0, 0 },
    { "FxOp82A1 UMULT r2", asm_FxOp82A1, c_FxOp82A1, 0, 0, 0 },
    { "FxOp83A1 UMULT r3", asm_FxOp83A1, c_FxOp83A1, 0, 0, 0 },
    { "FxOp84A1 UMULT r4", asm_FxOp84A1, c_FxOp84A1, 0, 0, 0 },
    { "FxOp85A1 UMULT r5", asm_FxOp85A1, c_FxOp85A1, 0, 0, 0 },
    { "FxOp86A1 UMULT r6", asm_FxOp86A1, c_FxOp86A1, 0, 0, 0 },
    { "FxOp87A1 UMULT r7", asm_FxOp87A1, c_FxOp87A1, 0, 0, 0 },
    { "FxOp88A1 UMULT r8", asm_FxOp88A1, c_FxOp88A1, 0, 0, 0 },
    { "FxOp89A1 UMULT r9", asm_FxOp89A1, c_FxOp89A1, 0, 0, 0 },
    { "FxOp8AA1 UMULT r10", asm_FxOp8AA1, c_FxOp8AA1, 0, 0, 0 },
    { "FxOp8BA1 UMULT r11", asm_FxOp8BA1, c_FxOp8BA1, 0, 0, 0 },
    { "FxOp8CA1 UMULT r12", asm_FxOp8CA1, c_FxOp8CA1, 0, 0, 0 },
    { "FxOp8DA1 UMULT r13", asm_FxOp8DA1, c_FxOp8DA1, 0, 0, 0 },
    { "FxOp8EA1 UMULT r14", asm_FxOp8EA1, c_FxOp8EA1, 0, 0, 0 },
    { "FxOp80A2 MULT #0", asm_FxOp80A2, c_FxOp80A2, 0, 0, 0 },
    { "FxOp81A2 MULT #1", asm_FxOp81A2, c_FxOp81A2, 0, 0, 0 },
    { "FxOp82A2 MULT #2", asm_FxOp82A2, c_FxOp82A2, 0, 0, 0 },
    { "FxOp83A2 MULT #3", asm_FxOp83A2, c_FxOp83A2, 0, 0, 0 },
    { "FxOp84A2 MULT #4", asm_FxOp84A2, c_FxOp84A2, 0, 0, 0 },
    { "FxOp85A2 MULT #5", asm_FxOp85A2, c_FxOp85A2, 0, 0, 0 },
    { "FxOp86A2 MULT #6", asm_FxOp86A2, c_FxOp86A2, 0, 0, 0 },
    { "FxOp87A2 MULT #7", asm_FxOp87A2, c_FxOp87A2, 0, 0, 0 },
    { "FxOp88A2 MULT #8", asm_FxOp88A2, c_FxOp88A2, 0, 0, 0 },
    { "FxOp89A2 MULT #9", asm_FxOp89A2, c_FxOp89A2, 0, 0, 0 },
    { "FxOp8AA2 MULT #10", asm_FxOp8AA2, c_FxOp8AA2, 0, 0, 0 },
    { "FxOp8BA2 MULT #11", asm_FxOp8BA2, c_FxOp8BA2, 0, 0, 0 },
    { "FxOp8CA2 MULT #12", asm_FxOp8CA2, c_FxOp8CA2, 0, 0, 0 },
    { "FxOp8DA2 MULT #13", asm_FxOp8DA2, c_FxOp8DA2, 0, 0, 0 },
    { "FxOp8EA2 MULT #14", asm_FxOp8EA2, c_FxOp8EA2, 0, 0, 0 },
    { "FxOp8FA2 MULT #15", asm_FxOp8FA2, c_FxOp8FA2, 0, 0, 0 },
    { "FxOp80A3 UMULT #0", asm_FxOp80A3, c_FxOp80A3, 0, 0, 0 },
    { "FxOp81A3 UMULT #1", asm_FxOp81A3, c_FxOp81A3, 0, 0, 0 },
    { "FxOp82A3 UMULT #2", asm_FxOp82A3, c_FxOp82A3, 0, 0, 0 },
    { "FxOp83A3 UMULT #3", asm_FxOp83A3, c_FxOp83A3, 0, 0, 0 },
    { "FxOp84A3 UMULT #4", asm_FxOp84A3, c_FxOp84A3, 0, 0, 0 },
    { "FxOp85A3 UMULT #5", asm_FxOp85A3, c_FxOp85A3, 0, 0, 0 },
    { "FxOp86A3 UMULT #6", asm_FxOp86A3, c_FxOp86A3, 0, 0, 0 },
    { "FxOp87A3 UMULT #7", asm_FxOp87A3, c_FxOp87A3, 0, 0, 0 },
    { "FxOp88A3 UMULT #8", asm_FxOp88A3, c_FxOp88A3, 0, 0, 0 },
    { "FxOp89A3 UMULT #9", asm_FxOp89A3, c_FxOp89A3, 0, 0, 0 },
    { "FxOp8AA3 UMULT #10", asm_FxOp8AA3, c_FxOp8AA3, 0, 0, 0 },
    { "FxOp8BA3 UMULT #11", asm_FxOp8BA3, c_FxOp8BA3, 0, 0, 0 },
    { "FxOp8CA3 UMULT #12", asm_FxOp8CA3, c_FxOp8CA3, 0, 0, 0 },
    { "FxOp8DA3 UMULT #13", asm_FxOp8DA3, c_FxOp8DA3, 0, 0, 0 },
    { "FxOp8EA3 UMULT #14", asm_FxOp8EA3, c_FxOp8EA3, 0, 0, 0 },
    { "FxOp8FA3 UMULT #15", asm_FxOp8FA3, c_FxOp8FA3, 0, 0, 0 },
    { "FxOp10 TO r0", asm_FxOp10, c_FxOp10, 0, 0, 0 },
    { "FxOp11 TO r1", asm_FxOp11, c_FxOp11, 0, 0, 0 },
    { "FxOp12 TO r2", asm_FxOp12, c_FxOp12, 0, 0, 0 },
    { "FxOp13 TO r3", asm_FxOp13, c_FxOp13, 0, 0, 0 },
    { "FxOp14 TO r4", asm_FxOp14, c_FxOp14, 0, 0, 0 },
    { "FxOp15 TO r5", asm_FxOp15, c_FxOp15, 0, 0, 0 },
    { "FxOp16 TO r6", asm_FxOp16, c_FxOp16, 0, 0, 0 },
    { "FxOp17 TO r7", asm_FxOp17, c_FxOp17, 0, 0, 0 },
    { "FxOp18 TO r8", asm_FxOp18, c_FxOp18, 0, 0, 0 },
    { "FxOp19 TO r9", asm_FxOp19, c_FxOp19, 0, 0, 0 },
    { "FxOp1A TO r10", asm_FxOp1A, c_FxOp1A, 0, 0, 0 },
    { "FxOp1B TO r11", asm_FxOp1B, c_FxOp1B, 0, 0, 0 },
    { "FxOp1C TO r12", asm_FxOp1C, c_FxOp1C, 0, 0, 0 },
    { "FxOp1D TO r13", asm_FxOp1D, c_FxOp1D, 0, 0, 0 },
    { "FxOp20 WITH r0", asm_FxOp20, c_FxOp20, 0, 0, 0 },
    { "FxOp21 WITH r1", asm_FxOp21, c_FxOp21, 0, 0, 0 },
    { "FxOp22 WITH r2", asm_FxOp22, c_FxOp22, 0, 0, 0 },
    { "FxOp23 WITH r3", asm_FxOp23, c_FxOp23, 0, 0, 0 },
    { "FxOp24 WITH r4", asm_FxOp24, c_FxOp24, 0, 0, 0 },
    { "FxOp25 WITH r5", asm_FxOp25, c_FxOp25, 0, 0, 0 },
    { "FxOp26 WITH r6", asm_FxOp26, c_FxOp26, 0, 0, 0 },
    { "FxOp27 WITH r7", asm_FxOp27, c_FxOp27, 0, 0, 0 },
    { "FxOp28 WITH r8", asm_FxOp28, c_FxOp28, 0, 0, 0 },
    { "FxOp29 WITH r9", asm_FxOp29, c_FxOp29, 0, 0, 0 },
    { "FxOp2A WITH r10", asm_FxOp2A, c_FxOp2A, 0, 0, 0 },
    { "FxOp2B WITH r11", asm_FxOp2B, c_FxOp2B, 0, 0, 0 },
    { "FxOp2C WITH r12", asm_FxOp2C, c_FxOp2C, 0, 0, 0 },
    { "FxOp2D WITH r13", asm_FxOp2D, c_FxOp2D, 0, 0, 0 },
    { "FxOpB0 FROM r0", asm_FxOpB0, c_FxOpB0, 0, 0, 0 },
    { "FxOpB1 FROM r1", asm_FxOpB1, c_FxOpB1, 0, 0, 0 },
    { "FxOpB2 FROM r2", asm_FxOpB2, c_FxOpB2, 0, 0, 0 },
    { "FxOpB3 FROM r3", asm_FxOpB3, c_FxOpB3, 0, 0, 0 },
    { "FxOpB4 FROM r4", asm_FxOpB4, c_FxOpB4, 0, 0, 0 },
    { "FxOpB5 FROM r5", asm_FxOpB5, c_FxOpB5, 0, 0, 0 },
    { "FxOpB6 FROM r6", asm_FxOpB6, c_FxOpB6, 0, 0, 0 },
    { "FxOpB7 FROM r7", asm_FxOpB7, c_FxOpB7, 0, 0, 0 },
    { "FxOpB8 FROM r8", asm_FxOpB8, c_FxOpB8, 0, 0, 0 },
    { "FxOpB9 FROM r9", asm_FxOpB9, c_FxOpB9, 0, 0, 0 },
    { "FxOpBA FROM r10", asm_FxOpBA, c_FxOpBA, 0, 0, 0 },
    { "FxOpBB FROM r11", asm_FxOpBB, c_FxOpBB, 0, 0, 0 },
    { "FxOpBC FROM r12", asm_FxOpBC, c_FxOpBC, 0, 0, 0 },
    { "FxOpBD FROM r13", asm_FxOpBD, c_FxOpBD, 0, 0, 0 },
    { "FxOpBE FROM r14", asm_FxOpBE, c_FxOpBE, 0, 0, 0 },
    { "FxOp30 STW r0", asm_FxOp30, c_FxOp30, 1, 0, 0 },
    { "FxOp31 STW r1", asm_FxOp31, c_FxOp31, 1, 1, 0 },
    { "FxOp32 STW r2", asm_FxOp32, c_FxOp32, 1, 2, 0 },
    { "FxOp33 STW r3", asm_FxOp33, c_FxOp33, 1, 3, 0 },
    { "FxOp34 STW r4", asm_FxOp34, c_FxOp34, 1, 4, 0 },
    { "FxOp35 STW r5", asm_FxOp35, c_FxOp35, 1, 5, 0 },
    { "FxOp36 STW r6", asm_FxOp36, c_FxOp36, 1, 6, 0 },
    { "FxOp37 STW r7", asm_FxOp37, c_FxOp37, 1, 7, 0 },
    { "FxOp38 STW r8", asm_FxOp38, c_FxOp38, 1, 8, 0 },
    { "FxOp39 STW r9", asm_FxOp39, c_FxOp39, 1, 9, 0 },
    { "FxOp3A STW r10", asm_FxOp3A, c_FxOp3A, 1, 10, 0 },
    { "FxOp3B STW r11", asm_FxOp3B, c_FxOp3B, 1, 11, 0 },
    { "FxOp30A1 STB r0", asm_FxOp30A1, c_FxOp30A1, 1, 0, 0 },
    { "FxOp31A1 STB r1", asm_FxOp31A1, c_FxOp31A1, 1, 1, 0 },
    { "FxOp32A1 STB r2", asm_FxOp32A1, c_FxOp32A1, 1, 2, 0 },
    { "FxOp33A1 STB r3", asm_FxOp33A1, c_FxOp33A1, 1, 3, 0 },
    { "FxOp34A1 STB r4", asm_FxOp34A1, c_FxOp34A1, 1, 4, 0 },
    { "FxOp35A1 STB r5", asm_FxOp35A1, c_FxOp35A1, 1, 5, 0 },
    { "FxOp36A1 STB r6", asm_FxOp36A1, c_FxOp36A1, 1, 6, 0 },
    { "FxOp37A1 STB r7", asm_FxOp37A1, c_FxOp37A1, 1, 7, 0 },
    { "FxOp38A1 STB r8", asm_FxOp38A1, c_FxOp38A1, 1, 8, 0 },
    { "FxOp39A1 STB r9", asm_FxOp39A1, c_FxOp39A1, 1, 9, 0 },
    { "FxOp3AA1 STB r10", asm_FxOp3AA1, c_FxOp3AA1, 1, 10, 0 },
    { "FxOp3BA1 STB r11", asm_FxOp3BA1, c_FxOp3BA1, 1, 11, 0 },
    { "FxOp40 LDW r0", asm_FxOp40, c_FxOp40, 1, 0, 0 },
    { "FxOp41 LDW r1", asm_FxOp41, c_FxOp41, 1, 1, 0 },
    { "FxOp42 LDW r2", asm_FxOp42, c_FxOp42, 1, 2, 0 },
    { "FxOp43 LDW r3", asm_FxOp43, c_FxOp43, 1, 3, 0 },
    { "FxOp44 LDW r4", asm_FxOp44, c_FxOp44, 1, 4, 0 },
    { "FxOp45 LDW r5", asm_FxOp45, c_FxOp45, 1, 5, 0 },
    { "FxOp46 LDW r6", asm_FxOp46, c_FxOp46, 1, 6, 0 },
    { "FxOp47 LDW r7", asm_FxOp47, c_FxOp47, 1, 7, 0 },
    { "FxOp48 LDW r8", asm_FxOp48, c_FxOp48, 1, 8, 0 },
    { "FxOp49 LDW r9", asm_FxOp49, c_FxOp49, 1, 9, 0 },
    { "FxOp4A LDW r10", asm_FxOp4A, c_FxOp4A, 1, 10, 0 },
    { "FxOp4B LDW r11", asm_FxOp4B, c_FxOp4B, 1, 11, 0 },
    { "FxOp40A1 LDB r0", asm_FxOp40A1, c_FxOp40A1, 1, 0, 0 },
    { "FxOp41A1 LDB r1", asm_FxOp41A1, c_FxOp41A1, 1, 1, 0 },
    { "FxOp42A1 LDB r2", asm_FxOp42A1, c_FxOp42A1, 1, 2, 0 },
    { "FxOp43A1 LDB r3", asm_FxOp43A1, c_FxOp43A1, 1, 3, 0 },
    { "FxOp44A1 LDB r4", asm_FxOp44A1, c_FxOp44A1, 1, 4, 0 },
    { "FxOp45A1 LDB r5", asm_FxOp45A1, c_FxOp45A1, 1, 5, 0 },
    { "FxOp46A1 LDB r6", asm_FxOp46A1, c_FxOp46A1, 1, 6, 0 },
    { "FxOp47A1 LDB r7", asm_FxOp47A1, c_FxOp47A1, 1, 7, 0 },
    { "FxOp48A1 LDB r8", asm_FxOp48A1, c_FxOp48A1, 1, 8, 0 },
    { "FxOp49A1 LDB r9", asm_FxOp49A1, c_FxOp49A1, 1, 9, 0 },
    { "FxOp4AA1 LDB r10", asm_FxOp4AA1, c_FxOp4AA1, 1, 10, 0 },
    { "FxOp4BA1 LDB r11", asm_FxOp4BA1, c_FxOp4BA1, 1, 11, 0 },
    { "FxOpA0 IBT r0", asm_FxOpA0, c_FxOpA0, 0, 0, 0 },
    { "FxOpA1 IBT r1", asm_FxOpA1, c_FxOpA1, 0, 0, 0 },
    { "FxOpA2 IBT r2", asm_FxOpA2, c_FxOpA2, 0, 0, 0 },
    { "FxOpA3 IBT r3", asm_FxOpA3, c_FxOpA3, 0, 0, 0 },
    { "FxOpA4 IBT r4", asm_FxOpA4, c_FxOpA4, 0, 0, 0 },
    { "FxOpA5 IBT r5", asm_FxOpA5, c_FxOpA5, 0, 0, 0 },
    { "FxOpA6 IBT r6", asm_FxOpA6, c_FxOpA6, 0, 0, 0 },
    { "FxOpA7 IBT r7", asm_FxOpA7, c_FxOpA7, 0, 0, 0 },
    { "FxOpA8 IBT r8", asm_FxOpA8, c_FxOpA8, 0, 0, 0 },
    { "FxOpA9 IBT r9", asm_FxOpA9, c_FxOpA9, 0, 0, 0 },
    { "FxOpAA IBT r10", asm_FxOpAA, c_FxOpAA, 0, 0, 0 },
    { "FxOpAB IBT r11", asm_FxOpAB, c_FxOpAB, 0, 0, 0 },
    { "FxOpAC IBT r12", asm_FxOpAC, c_FxOpAC, 0, 0, 0 },
    { "FxOpAD IBT r13", asm_FxOpAD, c_FxOpAD, 0, 0, 0 },
    { "FxOpA0A1 LMS r0", asm_FxOpA0A1, c_FxOpA0A1, 3, 0, 0 },
    { "FxOpA1A1 LMS r1", asm_FxOpA1A1, c_FxOpA1A1, 3, 0, 0 },
    { "FxOpA2A1 LMS r2", asm_FxOpA2A1, c_FxOpA2A1, 3, 0, 0 },
    { "FxOpA3A1 LMS r3", asm_FxOpA3A1, c_FxOpA3A1, 3, 0, 0 },
    { "FxOpA4A1 LMS r4", asm_FxOpA4A1, c_FxOpA4A1, 3, 0, 0 },
    { "FxOpA5A1 LMS r5", asm_FxOpA5A1, c_FxOpA5A1, 3, 0, 0 },
    { "FxOpA6A1 LMS r6", asm_FxOpA6A1, c_FxOpA6A1, 3, 0, 0 },
    { "FxOpA7A1 LMS r7", asm_FxOpA7A1, c_FxOpA7A1, 3, 0, 0 },
    { "FxOpA8A1 LMS r8", asm_FxOpA8A1, c_FxOpA8A1, 3, 0, 0 },
    { "FxOpA9A1 LMS r9", asm_FxOpA9A1, c_FxOpA9A1, 3, 0, 0 },
    { "FxOpAAA1 LMS r10", asm_FxOpAAA1, c_FxOpAAA1, 3, 0, 0 },
    { "FxOpABA1 LMS r11", asm_FxOpABA1, c_FxOpABA1, 3, 0, 0 },
    { "FxOpACA1 LMS r12", asm_FxOpACA1, c_FxOpACA1, 3, 0, 0 },
    { "FxOpADA1 LMS r13", asm_FxOpADA1, c_FxOpADA1, 3, 0, 0 },
    { "FxOpA0A2 SMS r0", asm_FxOpA0A2, c_FxOpA0A2, 3, 0, 0 },
    { "FxOpA1A2 SMS r1", asm_FxOpA1A2, c_FxOpA1A2, 3, 0, 0 },
    { "FxOpA2A2 SMS r2", asm_FxOpA2A2, c_FxOpA2A2, 3, 0, 0 },
    { "FxOpA3A2 SMS r3", asm_FxOpA3A2, c_FxOpA3A2, 3, 0, 0 },
    { "FxOpA4A2 SMS r4", asm_FxOpA4A2, c_FxOpA4A2, 3, 0, 0 },
    { "FxOpA5A2 SMS r5", asm_FxOpA5A2, c_FxOpA5A2, 3, 0, 0 },
    { "FxOpA6A2 SMS r6", asm_FxOpA6A2, c_FxOpA6A2, 3, 0, 0 },
    { "FxOpA7A2 SMS r7", asm_FxOpA7A2, c_FxOpA7A2, 3, 0, 0 },
    { "FxOpA8A2 SMS r8", asm_FxOpA8A2, c_FxOpA8A2, 3, 0, 0 },
    { "FxOpA9A2 SMS r9", asm_FxOpA9A2, c_FxOpA9A2, 3, 0, 0 },
    { "FxOpAAA2 SMS r10", asm_FxOpAAA2, c_FxOpAAA2, 3, 0, 0 },
    { "FxOpABA2 SMS r11", asm_FxOpABA2, c_FxOpABA2, 3, 0, 0 },
    { "FxOpACA2 SMS r12", asm_FxOpACA2, c_FxOpACA2, 3, 0, 0 },
    { "FxOpADA2 SMS r13", asm_FxOpADA2, c_FxOpADA2, 3, 0, 0 },
    { "FxOpAEA2 SMS r14", asm_FxOpAEA2, c_FxOpAEA2, 3, 0, 0 },
    { "FxOpF0 IWT r0", asm_FxOpF0, c_FxOpF0, 0, 0, 0 },
    { "FxOpF1 IWT r1", asm_FxOpF1, c_FxOpF1, 0, 0, 0 },
    { "FxOpF2 IWT r2", asm_FxOpF2, c_FxOpF2, 0, 0, 0 },
    { "FxOpF3 IWT r3", asm_FxOpF3, c_FxOpF3, 0, 0, 0 },
    { "FxOpF4 IWT r4", asm_FxOpF4, c_FxOpF4, 0, 0, 0 },
    { "FxOpF5 IWT r5", asm_FxOpF5, c_FxOpF5, 0, 0, 0 },
    { "FxOpF6 IWT r6", asm_FxOpF6, c_FxOpF6, 0, 0, 0 },
    { "FxOpF7 IWT r7", asm_FxOpF7, c_FxOpF7, 0, 0, 0 },
    { "FxOpF8 IWT r8", asm_FxOpF8, c_FxOpF8, 0, 0, 0 },
    { "FxOpF9 IWT r9", asm_FxOpF9, c_FxOpF9, 0, 0, 0 },
    { "FxOpFA IWT r10", asm_FxOpFA, c_FxOpFA, 0, 0, 0 },
    { "FxOpFB IWT r11", asm_FxOpFB, c_FxOpFB, 0, 0, 0 },
    { "FxOpFC IWT r12", asm_FxOpFC, c_FxOpFC, 0, 0, 0 },
    { "FxOpFD IWT r13", asm_FxOpFD, c_FxOpFD, 0, 0, 0 },
    { "FxOpF0A1 LM r0", asm_FxOpF0A1, c_FxOpF0A1, 2, 0, 0 },
    { "FxOpF1A1 LM r1", asm_FxOpF1A1, c_FxOpF1A1, 2, 0, 0 },
    { "FxOpF2A1 LM r2", asm_FxOpF2A1, c_FxOpF2A1, 2, 0, 0 },
    { "FxOpF3A1 LM r3", asm_FxOpF3A1, c_FxOpF3A1, 2, 0, 0 },
    { "FxOpF4A1 LM r4", asm_FxOpF4A1, c_FxOpF4A1, 2, 0, 0 },
    { "FxOpF5A1 LM r5", asm_FxOpF5A1, c_FxOpF5A1, 2, 0, 0 },
    { "FxOpF6A1 LM r6", asm_FxOpF6A1, c_FxOpF6A1, 2, 0, 0 },
    { "FxOpF7A1 LM r7", asm_FxOpF7A1, c_FxOpF7A1, 2, 0, 0 },
    { "FxOpF8A1 LM r8", asm_FxOpF8A1, c_FxOpF8A1, 2, 0, 0 },
    { "FxOpF9A1 LM r9", asm_FxOpF9A1, c_FxOpF9A1, 2, 0, 0 },
    { "FxOpFAA1 LM r10", asm_FxOpFAA1, c_FxOpFAA1, 2, 0, 0 },
    { "FxOpFBA1 LM r11", asm_FxOpFBA1, c_FxOpFBA1, 2, 0, 0 },
    { "FxOpFCA1 LM r12", asm_FxOpFCA1, c_FxOpFCA1, 2, 0, 0 },
    { "FxOpFDA1 LM r13", asm_FxOpFDA1, c_FxOpFDA1, 2, 0, 0 },
    { "FxOpF0A2 SM r0", asm_FxOpF0A2, c_FxOpF0A2, 2, 0, 0 },
    { "FxOpF1A2 SM r1", asm_FxOpF1A2, c_FxOpF1A2, 2, 0, 0 },
    { "FxOpF2A2 SM r2", asm_FxOpF2A2, c_FxOpF2A2, 2, 0, 0 },
    { "FxOpF3A2 SM r3", asm_FxOpF3A2, c_FxOpF3A2, 2, 0, 0 },
    { "FxOpF4A2 SM r4", asm_FxOpF4A2, c_FxOpF4A2, 2, 0, 0 },
    { "FxOpF5A2 SM r5", asm_FxOpF5A2, c_FxOpF5A2, 2, 0, 0 },
    { "FxOpF6A2 SM r6", asm_FxOpF6A2, c_FxOpF6A2, 2, 0, 0 },
    { "FxOpF7A2 SM r7", asm_FxOpF7A2, c_FxOpF7A2, 2, 0, 0 },
    { "FxOpF8A2 SM r8", asm_FxOpF8A2, c_FxOpF8A2, 2, 0, 0 },
    { "FxOpF9A2 SM r9", asm_FxOpF9A2, c_FxOpF9A2, 2, 0, 0 },
    { "FxOpFAA2 SM r10", asm_FxOpFAA2, c_FxOpFAA2, 2, 0, 0 },
    { "FxOpFBA2 SM r11", asm_FxOpFBA2, c_FxOpFBA2, 2, 0, 0 },
    { "FxOpFCA2 SM r12", asm_FxOpFCA2, c_FxOpFCA2, 2, 0, 0 },
    { "FxOpFDA2 SM r13", asm_FxOpFDA2, c_FxOpFDA2, 2, 0, 0 },
    { "FxOpFEA2 SM r14", asm_FxOpFEA2, c_FxOpFEA2, 2, 0, 0 },
    { "FxOp91 LINK #1", asm_FxOp91, c_FxOp91, 0, 1, 0 },
    { "FxOp92 LINK #2", asm_FxOp92, c_FxOp92, 0, 2, 0 },
    { "FxOp93 LINK #3", asm_FxOp93, c_FxOp93, 0, 3, 0 },
    { "FxOp94 LINK #4", asm_FxOp94, c_FxOp94, 0, 4, 0 },
    { "FxOp98 JMP r8", asm_FxOp98, c_FxOp98, 4, 8, 0 },
    { "FxOp99 JMP r9", asm_FxOp99, c_FxOp99, 4, 9, 0 },
    { "FxOp9A JMP r10", asm_FxOp9A, c_FxOp9A, 4, 10, 0 },
    { "FxOp9B JMP r11", asm_FxOp9B, c_FxOp9B, 4, 11, 0 },
    { "FxOp9C JMP r12", asm_FxOp9C, c_FxOp9C, 4, 12, 0 },
    { "FxOp9D JMP r13", asm_FxOp9D, c_FxOp9D, 4, 13, 0 },
    { "FxOp98A1 LJMP r8", asm_FxOp98A1, c_FxOp98A1, 5, 8, 0 },
    { "FxOp99A1 LJMP r9", asm_FxOp99A1, c_FxOp99A1, 5, 9, 0 },
    { "FxOp9AA1 LJMP r10", asm_FxOp9AA1, c_FxOp9AA1, 5, 10, 0 },
    { "FxOp9BA1 LJMP r11", asm_FxOp9BA1, c_FxOp9BA1, 5, 11, 0 },
    { "FxOp9CA1 LJMP r12", asm_FxOp9CA1, c_FxOp9CA1, 5, 12, 0 },
    { "FxOp9DA1 LJMP r13", asm_FxOp9DA1, c_FxOp9DA1, 5, 13, 0 },
    { "FxOp02 CACHE", asm_FxOp02, c_FxOp02, 0, 0, 0 },
    { "FxOp01 NOP", asm_FxOp01, c_FxOp01, 0, 0, 0 },
    { "FxOp4D SWAP", asm_FxOp4D, c_FxOp4D, 0, 0, 0 },
    { "FxOp4F NOT", asm_FxOp4F, c_FxOp4F, 0, 0, 0 },
    { "FxOp95 SEX", asm_FxOp95, c_FxOp95, 0, 0, 0 },
    { "FxOp96 ASR", asm_FxOp96, c_FxOp96, 0, 0, 0 },
    { "FxOp96A1 DIV2", asm_FxOp96A1, c_FxOp96A1, 0, 0, 0 },
    { "FxOp97 ROR", asm_FxOp97, c_FxOp97, 0, 0, 0 },
    { "FxOp9E LOB", asm_FxOp9E, c_FxOp9E, 0, 0, 0 },
    { "FxOpC0 HIB", asm_FxOpC0, c_FxOpC0, 0, 0, 0 },
    { "FxOp03 LSR", asm_FxOp03, c_FxOp03, 0, 0, 0 },
    { "FxOp04 ROL", asm_FxOp04, c_FxOp04, 0, 0, 0 },
    { "FxOp3C LOOP", asm_FxOp3C, c_FxOp3C, 4, 13, 0 }, /* branches to R13 */
    { "FxOp9F FMULT", asm_FxOp9F, c_FxOp9F, 0, 0, 0 },
    { "FxOp9FA1 LMULT", asm_FxOp9FA1, c_FxOp9FA1, 0, 0, 0 },
    { "FxOpAE IBT R14", asm_FxOpAE, c_FxOpAE, 0, 0, 0 },
    { "FxOpAF JMP #d", asm_FxOpAF, c_FxOpAF, 0, 0, 0 },
    { "FxOpDE INC R14", asm_FxOpDE, c_FxOpDE, 0, 0, 0 },
    { "FxOpEE DEC R14", asm_FxOpEE, c_FxOpEE, 0, 0, 0 },
    { "FxOp05 BRA", asm_FxOp05, c_FxOp05, 0, 0, 0 },
    { "FxOp06 BGE", asm_FxOp06, c_FxOp06, 0, 0, 0 },
    { "FxOp07 BLT", asm_FxOp07, c_FxOp07, 0, 0, 0 },
    { "FxOp08 BNE", asm_FxOp08, c_FxOp08, 0, 0, 0 },
    { "FxOp09 BEQ", asm_FxOp09, c_FxOp09, 0, 0, 0 },
    { "FxOp0A BPL", asm_FxOp0A, c_FxOp0A, 0, 0, 0 },
    { "FxOp0B BMI", asm_FxOp0B, c_FxOp0B, 0, 0, 0 },
    { "FxOp0C BCC", asm_FxOp0C, c_FxOp0C, 0, 0, 0 },
    { "FxOp0D BCS", asm_FxOp0D, c_FxOp0D, 0, 0, 0 },
    { "FxOp0E BVC", asm_FxOp0E, c_FxOp0E, 0, 0, 0 },
    { "FxOp0F BVS", asm_FxOp0F, c_FxOp0F, 0, 0, 0 },
    { "FxOp1E TO R14", asm_FxOp1E, c_FxOp1E, 0, 0, 0 },
    { "FxOp1F TO R15", asm_FxOp1F, c_FxOp1F, 0, 0, 0 },
    { "FxOp2E WITH R14", asm_FxOp2E, c_FxOp2E, 0, 0, 0 },
    { "FxOp2F WITH R15", asm_FxOp2F, c_FxOp2F, 0, 0, 0 },
    { "FxOp3D ALT1", asm_FxOp3D, c_FxOp3D, 0, 0, 0 },
    { "FxOp3E ALT2", asm_FxOp3E, c_FxOp3E, 0, 0, 0 },
    { "FxOp3F ALT3", asm_FxOp3F, c_FxOp3F, 0, 0, 0 },
    { "FxOpBF FROM R15", asm_FxOpBF, c_FxOpBF, 0, 0, 0 },
    { "FxOp5F ADD R15", asm_FxOp5F, c_FxOp5F, 0, 0, 0 },
    { "FxOp5FA1 ADC R15", asm_FxOp5FA1, c_FxOp5FA1, 0, 0, 0 },
    { "FxOp6F SUB R15", asm_FxOp6F, c_FxOp6F, 0, 0, 0 },
    { "FxOp6FA1 SBC R15", asm_FxOp6FA1, c_FxOp6FA1, 0, 0, 0 },
    { "FxOp6FA3 CMP R15", asm_FxOp6FA3, c_FxOp6FA3, 0, 0, 0 },
    { "FxOp7F AND R15", asm_FxOp7F, c_FxOp7F, 0, 0, 0 },
    { "FxOp7FA1 BIC R15", asm_FxOp7FA1, c_FxOp7FA1, 0, 0, 0 },
    { "FxOp8F MULT R15", asm_FxOp8F, c_FxOp8F, 0, 0, 0 },
    { "FxOp8FA1 UMULT R15", asm_FxOp8FA1, c_FxOp8FA1, 0, 0, 0 },
    { "FxOp90 SBK", asm_FxOp90, c_FxOp90, 6, 0, 0 },
    { "FxOpEF GETB", asm_FxOpEF, c_FxOpEF, 0, 0, 0 },
    { "FxOpEFA1 GETBH", asm_FxOpEFA1, c_FxOpEFA1, 0, 0, 0 },
    { "FxOpEFA2 GETBL", asm_FxOpEFA2, c_FxOpEFA2, 0, 0, 0 },
    { "FxOpEFA3 GETBS", asm_FxOpEFA3, c_FxOpEFA3, 0, 0, 0 },
    { "FxOpDFA2 RAMB", asm_FxOpDFA2, c_FxOpDFA2, 0, 0, 0 },
    { "FxOpDFA3 ROMB", asm_FxOpDFA3, c_FxOpDFA3, 0, 0, 0 },
    { "FxOpCF OR R15", asm_FxOpCF, c_FxOpCF, 0, 0, 0 },
    { "FxOpCFA1 XOR R15", asm_FxOpCFA1, c_FxOpCFA1, 0, 0, 0 },
    { "FxOpAEA1 LMS R14", asm_FxOpAEA1, c_FxOpAEA1, 3, 0, 0 },
    { "FxOpAFA1 LMS R15", asm_FxOpAFA1, c_FxOpAFA1, 3, 0, 0 },
    { "FxOpAFA2 SMS R15", asm_FxOpAFA2, c_FxOpAFA2, 3, 0, 0 },
    { "FxOpFE IWT R14", asm_FxOpFE, c_FxOpFE, 0, 0, 0 },
    { "FxOpFF IWT R15", asm_FxOpFF, c_FxOpFF, 0, 0, 0 },
    { "FxOp4E COLOR", asm_FxOp4E, c_FxOp4E, 0, 0, 0 },
    { "FxOpDF GETC", asm_FxOpDF, c_FxOpDF, 0, 0, 0 },
    { "FxOp4EA1 CMODE", asm_FxOp4EA1, c_FxOp4EA1, 0, 0, 0 },
    { "FxOp70 MERGE", asm_FxOp70, c_FxOp70, 0, 0, 0 },
    { "FxOpFEA1 LM R14", asm_FxOpFEA1, c_FxOpFEA1, 2, 0, 0 },
    { "FxOpFFA1 LM R15", asm_FxOpFFA1, c_FxOpFFA1, 2, 0, 0 },
    { "FxOpFFA2 SM R15", asm_FxOpFFA2, c_FxOpFFA2, 2, 0, 0 },
    { "FxOp00 STOP", asm_FxOp00, c_FxOp00, 0, 0, 0 },
    { "FxOp4C1284b PLOT 4bpp", asm_FxOp4C1284b, c_FxOp4C1284b, 7, 0, 0 },
    { "FxOp4C1284bz PLOT 4bpp z", asm_FxOp4C1284bz, c_FxOp4C1284bz, 7, 0, 0 },
    { "FxOp4C1284bd PLOT 4bpp d", asm_FxOp4C1284bd, c_FxOp4C1284bd, 7, 0, 0 },
    { "FxOp4C1284bzd PLOT 4bpp zd", asm_FxOp4C1284bzd, c_FxOp4C1284bzd, 7, 0, 0 },
    { "FxOp4C1282b PLOT 2bpp", asm_FxOp4C1282b, c_FxOp4C1282b, 7, 0, 0 },
    { "FxOp4C1282bz PLOT 2bpp z", asm_FxOp4C1282bz, c_FxOp4C1282bz, 7, 0, 0 },
    { "FxOp4C1282bd PLOT 2bpp d", asm_FxOp4C1282bd, c_FxOp4C1282bd, 7, 0, 0 },
    { "FxOp4C1282bzd PLOT 2bpp zd", asm_FxOp4C1282bzd, c_FxOp4C1282bzd, 7, 0, 0 },
    { "FxOp4C1288b PLOT 8bpp", asm_FxOp4C1288b, c_FxOp4C1288b, 7, 0, 0 },
    { "FxOp4C1288bz PLOT 8bpp z", asm_FxOp4C1288bz, c_FxOp4C1288bz, 7, 0, 0 },
    { "FxOp4C1288bd PLOT 8bpp d", asm_FxOp4C1288bd, c_FxOp4C1288bd, 7, 0, 0 },
    { "FxOp4C1288bzd PLOT 8bpp zd", asm_FxOp4C1288bzd, c_FxOp4C1288bzd, 7, 0, 0 },
    { "FxOp4C1288bl PLOT 8bpp l", asm_FxOp4C1288bl, c_FxOp4C1288bl, 7, 0, 0 },
    { "FxOp4C1288bzl PLOT 8bpp zl", asm_FxOp4C1288bzl, c_FxOp4C1288bzl, 7, 0, 0 },
    { "FxOp4C1288bdl PLOT 8bpp dl", asm_FxOp4C1288bdl, c_FxOp4C1288bdl, 7, 0, 0 },
    { "FxOp4C1288bzdl PLOT 8bpp zdl", asm_FxOp4C1288bzdl, c_FxOp4C1288bzdl, 7, 0, 0 },
    { "FxOp4C PLOT", asm_FxOp4C, c_FxOp4C, 7, 0, 0 },
    { "FxOp4CA1 RPIX", asm_FxOp4CA1, c_FxOp4CA1, 7, 0, 0 },
    { "FxOpd00", asm_FxOpd00, c_FxOpd00, 0, 0, 1 },
    { "FxOpd01", asm_FxOpd01, c_FxOp01, 0, 0, 1 },
    { "FxOpd02", asm_FxOpd02, c_FxOp02, 0, 0, 1 },
    { "FxOpd03", asm_FxOpd03, c_FxOp03, 0, 0, 1 },
    { "FxOpd04", asm_FxOpd04, c_FxOp04, 0, 0, 1 },
    { "FxOpd05", asm_FxOpd05, c_FxOp05, 0, 0, 1 },
    { "FxOpd06", asm_FxOpd06, c_FxOp06, 0, 0, 1 },
    { "FxOpd07", asm_FxOpd07, c_FxOp07, 0, 0, 1 },
    { "FxOpd08", asm_FxOpd08, c_FxOp08, 0, 0, 1 },
    { "FxOpd09", asm_FxOpd09, c_FxOp09, 0, 0, 1 },
    { "FxOpd0A", asm_FxOpd0A, c_FxOp0A, 0, 0, 1 },
    { "FxOpd0B", asm_FxOpd0B, c_FxOp0B, 0, 0, 1 },
    { "FxOpd0C", asm_FxOpd0C, c_FxOp0C, 0, 0, 1 },
    { "FxOpd0D", asm_FxOpd0D, c_FxOp0D, 0, 0, 1 },
    { "FxOpd0E", asm_FxOpd0E, c_FxOp0E, 0, 0, 1 },
    { "FxOpd0F", asm_FxOpd0F, c_FxOp0F, 0, 0, 1 },
    { "FxOpd10", asm_FxOpd10, c_FxOp10, 0, 0, 1 },
    { "FxOpd11", asm_FxOpd11, c_FxOp11, 0, 0, 1 },
    { "FxOpd12", asm_FxOpd12, c_FxOp12, 0, 0, 1 },
    { "FxOpd13", asm_FxOpd13, c_FxOp13, 0, 0, 1 },
    { "FxOpd14", asm_FxOpd14, c_FxOp14, 0, 0, 1 },
    { "FxOpd15", asm_FxOpd15, c_FxOp15, 0, 0, 1 },
    { "FxOpd16", asm_FxOpd16, c_FxOp16, 0, 0, 1 },
    { "FxOpd17", asm_FxOpd17, c_FxOp17, 0, 0, 1 },
    { "FxOpd18", asm_FxOpd18, c_FxOp18, 0, 0, 1 },
    { "FxOpd19", asm_FxOpd19, c_FxOp19, 0, 0, 1 },
    { "FxOpd1A", asm_FxOpd1A, c_FxOp1A, 0, 0, 1 },
    { "FxOpd1B", asm_FxOpd1B, c_FxOp1B, 0, 0, 1 },
    { "FxOpd1C", asm_FxOpd1C, c_FxOp1C, 0, 0, 1 },
    { "FxOpd1D", asm_FxOpd1D, c_FxOp1D, 0, 0, 1 },
    { "FxOpd1E", asm_FxOpd1E, c_FxOp1E, 0, 0, 1 },
    { "FxOpd1F", asm_FxOpd1F, c_FxOp1F, 0, 0, 1 },
    { "FxOpd20", asm_FxOpd20, c_FxOp20, 0, 0, 1 },
    { "FxOpd21", asm_FxOpd21, c_FxOp21, 0, 0, 1 },
    { "FxOpd22", asm_FxOpd22, c_FxOp22, 0, 0, 1 },
    { "FxOpd23", asm_FxOpd23, c_FxOp23, 0, 0, 1 },
    { "FxOpd24", asm_FxOpd24, c_FxOp24, 0, 0, 1 },
    { "FxOpd25", asm_FxOpd25, c_FxOp25, 0, 0, 1 },
    { "FxOpd26", asm_FxOpd26, c_FxOp26, 0, 0, 1 },
    { "FxOpd27", asm_FxOpd27, c_FxOp27, 0, 0, 1 },
    { "FxOpd28", asm_FxOpd28, c_FxOp28, 0, 0, 1 },
    { "FxOpd29", asm_FxOpd29, c_FxOp29, 0, 0, 1 },
    { "FxOpd2A", asm_FxOpd2A, c_FxOp2A, 0, 0, 1 },
    { "FxOpd2B", asm_FxOpd2B, c_FxOp2B, 0, 0, 1 },
    { "FxOpd2C", asm_FxOpd2C, c_FxOp2C, 0, 0, 1 },
    { "FxOpd2D", asm_FxOpd2D, c_FxOp2D, 0, 0, 1 },
    { "FxOpd2E", asm_FxOpd2E, c_FxOp2E, 0, 0, 1 },
    { "FxOpd2F", asm_FxOpd2F, c_FxOp2F, 0, 0, 1 },
    { "FxOpd30", asm_FxOpd30, c_FxOp30, 1, 0, 1 },
    { "FxOpd30A1", asm_FxOpd30A1, c_FxOp30A1, 1, 0, 1 },
    { "FxOpd31", asm_FxOpd31, c_FxOp31, 1, 1, 1 },
    { "FxOpd31A1", asm_FxOpd31A1, c_FxOp31A1, 1, 1, 1 },
    { "FxOpd32", asm_FxOpd32, c_FxOp32, 1, 2, 1 },
    { "FxOpd32A1", asm_FxOpd32A1, c_FxOp32A1, 1, 2, 1 },
    { "FxOpd33", asm_FxOpd33, c_FxOp33, 1, 3, 1 },
    { "FxOpd33A1", asm_FxOpd33A1, c_FxOp33A1, 1, 3, 1 },
    { "FxOpd34", asm_FxOpd34, c_FxOp34, 1, 4, 1 },
    { "FxOpd34A1", asm_FxOpd34A1, c_FxOp34A1, 1, 4, 1 },
    { "FxOpd35", asm_FxOpd35, c_FxOp35, 1, 5, 1 },
    { "FxOpd35A1", asm_FxOpd35A1, c_FxOp35A1, 1, 5, 1 },
    { "FxOpd36", asm_FxOpd36, c_FxOp36, 1, 6, 1 },
    { "FxOpd36A1", asm_FxOpd36A1, c_FxOp36A1, 1, 6, 1 },
    { "FxOpd37", asm_FxOpd37, c_FxOp37, 1, 7, 1 },
    { "FxOpd37A1", asm_FxOpd37A1, c_FxOp37A1, 1, 7, 1 },
    { "FxOpd38", asm_FxOpd38, c_FxOp38, 1, 8, 1 },
    { "FxOpd38A1", asm_FxOpd38A1, c_FxOp38A1, 1, 8, 1 },
    { "FxOpd39", asm_FxOpd39, c_FxOp39, 1, 9, 1 },
    { "FxOpd39A1", asm_FxOpd39A1, c_FxOp39A1, 1, 9, 1 },
    { "FxOpd3A", asm_FxOpd3A, c_FxOp3A, 1, 10, 1 },
    { "FxOpd3AA1", asm_FxOpd3AA1, c_FxOp3AA1, 1, 10, 1 },
    { "FxOpd3B", asm_FxOpd3B, c_FxOp3B, 1, 11, 1 },
    { "FxOpd3BA1", asm_FxOpd3BA1, c_FxOp3BA1, 1, 11, 1 },
    { "FxOpd3C", asm_FxOpd3C, c_FxOp3C, 4, 13, 1 },
    { "FxOpd3D", asm_FxOpd3D, c_FxOp3D, 0, 0, 1 },
    { "FxOpd3E", asm_FxOpd3E, c_FxOp3E, 0, 0, 1 },
    { "FxOpd3F", asm_FxOpd3F, c_FxOp3F, 0, 0, 1 },
    { "FxOpd40", asm_FxOpd40, c_FxOp40, 1, 0, 1 },
    { "FxOpd40A1", asm_FxOpd40A1, c_FxOp40A1, 1, 0, 1 },
    { "FxOpd41", asm_FxOpd41, c_FxOp41, 1, 1, 1 },
    { "FxOpd41A1", asm_FxOpd41A1, c_FxOp41A1, 1, 1, 1 },
    { "FxOpd42", asm_FxOpd42, c_FxOp42, 1, 2, 1 },
    { "FxOpd42A1", asm_FxOpd42A1, c_FxOp42A1, 1, 2, 1 },
    { "FxOpd43", asm_FxOpd43, c_FxOp43, 1, 3, 1 },
    { "FxOpd43A1", asm_FxOpd43A1, c_FxOp43A1, 1, 3, 1 },
    { "FxOpd44", asm_FxOpd44, c_FxOp44, 1, 4, 1 },
    { "FxOpd44A1", asm_FxOpd44A1, c_FxOp44A1, 1, 4, 1 },
    { "FxOpd45", asm_FxOpd45, c_FxOp45, 1, 5, 1 },
    { "FxOpd45A1", asm_FxOpd45A1, c_FxOp45A1, 1, 5, 1 },
    { "FxOpd46", asm_FxOpd46, c_FxOp46, 1, 6, 1 },
    { "FxOpd46A1", asm_FxOpd46A1, c_FxOp46A1, 1, 6, 1 },
    { "FxOpd47", asm_FxOpd47, c_FxOp47, 1, 7, 1 },
    { "FxOpd47A1", asm_FxOpd47A1, c_FxOp47A1, 1, 7, 1 },
    { "FxOpd48", asm_FxOpd48, c_FxOp48, 1, 8, 1 },
    { "FxOpd48A1", asm_FxOpd48A1, c_FxOp48A1, 1, 8, 1 },
    { "FxOpd49", asm_FxOpd49, c_FxOp49, 1, 9, 1 },
    { "FxOpd49A1", asm_FxOpd49A1, c_FxOp49A1, 1, 9, 1 },
    { "FxOpd4A", asm_FxOpd4A, c_FxOp4A, 1, 10, 1 },
    { "FxOpd4AA1", asm_FxOpd4AA1, c_FxOp4AA1, 1, 10, 1 },
    { "FxOpd4B", asm_FxOpd4B, c_FxOp4B, 1, 11, 1 },
    { "FxOpd4BA1", asm_FxOpd4BA1, c_FxOp4BA1, 1, 11, 1 },
    { "FxOpd4C", asm_FxOpd4C, c_FxOp4C1284b, 7, 0, 1 },
    { "FxOpd4C1282b", asm_FxOpd4C1282b, c_FxOp4C1282b, 7, 0, 1 },
    { "FxOpd4C1282bd", asm_FxOpd4C1282bd, c_FxOp4C1282bd, 7, 0, 1 },
    { "FxOpd4C1282bz", asm_FxOpd4C1282bz, c_FxOp4C1282bz, 7, 0, 1 },
    { "FxOpd4C1282bzd", asm_FxOpd4C1282bzd, c_FxOp4C1282bzd, 7, 0, 1 },
    { "FxOpd4C1284b", asm_FxOpd4C1284b, c_FxOp4C1284b, 7, 0, 1 },
    { "FxOpd4C1284bd", asm_FxOpd4C1284bd, c_FxOp4C1284bd, 7, 0, 1 },
    { "FxOpd4C1284bz", asm_FxOpd4C1284bz, c_FxOp4C1284bz, 7, 0, 1 },
    { "FxOpd4C1284bzd", asm_FxOpd4C1284bzd, c_FxOp4C1284bzd, 7, 0, 1 },
    { "FxOpd4C1288b", asm_FxOpd4C1288b, c_FxOp4C1288b, 7, 0, 1 },
    { "FxOpd4C1288bd", asm_FxOpd4C1288bd, c_FxOp4C1288b, 7, 0, 1 },
    { "FxOpd4C1288bdl", asm_FxOpd4C1288bdl, c_FxOp4C1288bl, 7, 0, 1 },
    { "FxOpd4C1288bl", asm_FxOpd4C1288bl, c_FxOp4C1288bl, 7, 0, 1 },
    { "FxOpd4C1288bz", asm_FxOpd4C1288bz, c_FxOp4C1288bz, 7, 0, 1 },
    { "FxOpd4C1288bzd", asm_FxOpd4C1288bzd, c_FxOp4C1288bz, 7, 0, 1 },
    { "FxOpd4C1288bzdl", asm_FxOpd4C1288bzdl, c_FxOp4C1288bzl, 7, 0, 1 },
    { "FxOpd4C1288bzl", asm_FxOpd4C1288bzl, c_FxOp4C1288bzl, 7, 0, 1 },
    { "FxOpd4CA1", asm_FxOpd4CA1, c_FxOp4CA1, 7, 0, 1 },
    { "FxOpd4D", asm_FxOpd4D, c_FxOp4D, 0, 0, 1 },
    { "FxOpd4E", asm_FxOpd4E, c_FxOp4E, 0, 0, 1 },
    { "FxOpd4EA1", asm_FxOpd4EA1, c_FxOp4EA1, 0, 0, 1 },
    { "FxOpd4F", asm_FxOpd4F, c_FxOp4F, 0, 0, 1 },
    { "FxOpd50", asm_FxOpd50, c_FxOp50, 0, 0, 1 },
    { "FxOpd50A1", asm_FxOpd50A1, c_FxOp50A1, 0, 0, 1 },
    { "FxOpd50A2", asm_FxOpd50A2, c_FxOp50A2, 0, 0, 1 },
    { "FxOpd50A3", asm_FxOpd50A3, c_FxOp50A3, 0, 0, 1 },
    { "FxOpd51", asm_FxOpd51, c_FxOp51, 0, 0, 1 },
    { "FxOpd51A1", asm_FxOpd51A1, c_FxOp51A1, 0, 0, 1 },
    { "FxOpd51A2", asm_FxOpd51A2, c_FxOp51A2, 0, 0, 1 },
    { "FxOpd51A3", asm_FxOpd51A3, c_FxOp51A3, 0, 0, 1 },
    { "FxOpd52", asm_FxOpd52, c_FxOp52, 0, 0, 1 },
    { "FxOpd52A1", asm_FxOpd52A1, c_FxOp52A1, 0, 0, 1 },
    { "FxOpd52A2", asm_FxOpd52A2, c_FxOp52A2, 0, 0, 1 },
    { "FxOpd52A3", asm_FxOpd52A3, c_FxOp52A3, 0, 0, 1 },
    { "FxOpd53", asm_FxOpd53, c_FxOp53, 0, 0, 1 },
    { "FxOpd53A1", asm_FxOpd53A1, c_FxOp53A1, 0, 0, 1 },
    { "FxOpd53A2", asm_FxOpd53A2, c_FxOp53A2, 0, 0, 1 },
    { "FxOpd53A3", asm_FxOpd53A3, c_FxOp53A3, 0, 0, 1 },
    { "FxOpd54", asm_FxOpd54, c_FxOp54, 0, 0, 1 },
    { "FxOpd54A1", asm_FxOpd54A1, c_FxOp54A1, 0, 0, 1 },
    { "FxOpd54A2", asm_FxOpd54A2, c_FxOp54A2, 0, 0, 1 },
    { "FxOpd54A3", asm_FxOpd54A3, c_FxOp54A3, 0, 0, 1 },
    { "FxOpd55", asm_FxOpd55, c_FxOp55, 0, 0, 1 },
    { "FxOpd55A1", asm_FxOpd55A1, c_FxOp55A1, 0, 0, 1 },
    { "FxOpd55A2", asm_FxOpd55A2, c_FxOp55A2, 0, 0, 1 },
    { "FxOpd55A3", asm_FxOpd55A3, c_FxOp55A3, 0, 0, 1 },
    { "FxOpd56", asm_FxOpd56, c_FxOp56, 0, 0, 1 },
    { "FxOpd56A1", asm_FxOpd56A1, c_FxOp56A1, 0, 0, 1 },
    { "FxOpd56A2", asm_FxOpd56A2, c_FxOp56A2, 0, 0, 1 },
    { "FxOpd56A3", asm_FxOpd56A3, c_FxOp56A3, 0, 0, 1 },
    { "FxOpd57", asm_FxOpd57, c_FxOp57, 0, 0, 1 },
    { "FxOpd57A1", asm_FxOpd57A1, c_FxOp57A1, 0, 0, 1 },
    { "FxOpd57A2", asm_FxOpd57A2, c_FxOp57A2, 0, 0, 1 },
    { "FxOpd57A3", asm_FxOpd57A3, c_FxOp57A3, 0, 0, 1 },
    { "FxOpd58", asm_FxOpd58, c_FxOp58, 0, 0, 1 },
    { "FxOpd58A1", asm_FxOpd58A1, c_FxOp58A1, 0, 0, 1 },
    { "FxOpd58A2", asm_FxOpd58A2, c_FxOp58A2, 0, 0, 1 },
    { "FxOpd58A3", asm_FxOpd58A3, c_FxOp58A3, 0, 0, 1 },
    { "FxOpd59", asm_FxOpd59, c_FxOp59, 0, 0, 1 },
    { "FxOpd59A1", asm_FxOpd59A1, c_FxOp59A1, 0, 0, 1 },
    { "FxOpd59A2", asm_FxOpd59A2, c_FxOp59A2, 0, 0, 1 },
    { "FxOpd59A3", asm_FxOpd59A3, c_FxOp59A3, 0, 0, 1 },
    { "FxOpd5A", asm_FxOpd5A, c_FxOp5A, 0, 0, 1 },
    { "FxOpd5AA1", asm_FxOpd5AA1, c_FxOp5AA1, 0, 0, 1 },
    { "FxOpd5AA2", asm_FxOpd5AA2, c_FxOp5AA2, 0, 0, 1 },
    { "FxOpd5AA3", asm_FxOpd5AA3, c_FxOp5AA3, 0, 0, 1 },
    { "FxOpd5B", asm_FxOpd5B, c_FxOp5B, 0, 0, 1 },
    { "FxOpd5BA1", asm_FxOpd5BA1, c_FxOp5BA1, 0, 0, 1 },
    { "FxOpd5BA2", asm_FxOpd5BA2, c_FxOp5BA2, 0, 0, 1 },
    { "FxOpd5BA3", asm_FxOpd5BA3, c_FxOp5BA3, 0, 0, 1 },
    { "FxOpd5C", asm_FxOpd5C, c_FxOp5C, 0, 0, 1 },
    { "FxOpd5CA1", asm_FxOpd5CA1, c_FxOp5CA1, 0, 0, 1 },
    { "FxOpd5CA2", asm_FxOpd5CA2, c_FxOp5CA2, 0, 0, 1 },
    { "FxOpd5CA3", asm_FxOpd5CA3, c_FxOp5CA3, 0, 0, 1 },
    { "FxOpd5D", asm_FxOpd5D, c_FxOp5D, 0, 0, 1 },
    { "FxOpd5DA1", asm_FxOpd5DA1, c_FxOp5DA1, 0, 0, 1 },
    { "FxOpd5DA2", asm_FxOpd5DA2, c_FxOp5DA2, 0, 0, 1 },
    { "FxOpd5DA3", asm_FxOpd5DA3, c_FxOp5DA3, 0, 0, 1 },
    { "FxOpd5E", asm_FxOpd5E, c_FxOp5E, 0, 0, 1 },
    { "FxOpd5EA1", asm_FxOpd5EA1, c_FxOp5EA1, 0, 0, 1 },
    { "FxOpd5EA2", asm_FxOpd5EA2, c_FxOp5EA2, 0, 0, 1 },
    { "FxOpd5EA3", asm_FxOpd5EA3, c_FxOp5EA3, 0, 0, 1 },
    { "FxOpd5F", asm_FxOpd5F, c_FxOp5F, 0, 0, 1 },
    { "FxOpd5FA1", asm_FxOpd5FA1, c_FxOp5FA1, 0, 0, 1 },
    { "FxOpd5FA2", asm_FxOpd5FA2, c_FxOp5FA2, 0, 0, 1 },
    { "FxOpd5FA3", asm_FxOpd5FA3, c_FxOp5FA3, 0, 0, 1 },
    { "FxOpd60", asm_FxOpd60, c_FxOp60, 0, 0, 1 },
    { "FxOpd60A1", asm_FxOpd60A1, c_FxOp60A1, 0, 0, 1 },
    { "FxOpd60A2", asm_FxOpd60A2, c_FxOp60A2, 0, 0, 1 },
    { "FxOpd60A3", asm_FxOpd60A3, c_FxOp60A3, 0, 0, 1 },
    { "FxOpd61", asm_FxOpd61, c_FxOp61, 0, 0, 1 },
    { "FxOpd61A1", asm_FxOpd61A1, c_FxOp61A1, 0, 0, 1 },
    { "FxOpd61A2", asm_FxOpd61A2, c_FxOp61A2, 0, 0, 1 },
    { "FxOpd61A3", asm_FxOpd61A3, c_FxOp61A3, 0, 0, 1 },
    { "FxOpd62", asm_FxOpd62, c_FxOp62, 0, 0, 1 },
    { "FxOpd62A1", asm_FxOpd62A1, c_FxOp62A1, 0, 0, 1 },
    { "FxOpd62A2", asm_FxOpd62A2, c_FxOp62A2, 0, 0, 1 },
    { "FxOpd62A3", asm_FxOpd62A3, c_FxOp62A3, 0, 0, 1 },
    { "FxOpd63", asm_FxOpd63, c_FxOp63, 0, 0, 1 },
    { "FxOpd63A1", asm_FxOpd63A1, c_FxOp63A1, 0, 0, 1 },
    { "FxOpd63A2", asm_FxOpd63A2, c_FxOp63A2, 0, 0, 1 },
    { "FxOpd63A3", asm_FxOpd63A3, c_FxOp63A3, 0, 0, 1 },
    { "FxOpd64", asm_FxOpd64, c_FxOp64, 0, 0, 1 },
    { "FxOpd64A1", asm_FxOpd64A1, c_FxOp64A1, 0, 0, 1 },
    { "FxOpd64A2", asm_FxOpd64A2, c_FxOp64A2, 0, 0, 1 },
    { "FxOpd64A3", asm_FxOpd64A3, c_FxOp64A3, 0, 0, 1 },
    { "FxOpd65", asm_FxOpd65, c_FxOp65, 0, 0, 1 },
    { "FxOpd65A1", asm_FxOpd65A1, c_FxOp65A1, 0, 0, 1 },
    { "FxOpd65A2", asm_FxOpd65A2, c_FxOp65A2, 0, 0, 1 },
    { "FxOpd65A3", asm_FxOpd65A3, c_FxOp65A3, 0, 0, 1 },
    { "FxOpd66", asm_FxOpd66, c_FxOp66, 0, 0, 1 },
    { "FxOpd66A1", asm_FxOpd66A1, c_FxOp66A1, 0, 0, 1 },
    { "FxOpd66A2", asm_FxOpd66A2, c_FxOp66A2, 0, 0, 1 },
    { "FxOpd66A3", asm_FxOpd66A3, c_FxOp66A3, 0, 0, 1 },
    { "FxOpd67", asm_FxOpd67, c_FxOp67, 0, 0, 1 },
    { "FxOpd67A1", asm_FxOpd67A1, c_FxOp67A1, 0, 0, 1 },
    { "FxOpd67A2", asm_FxOpd67A2, c_FxOp67A2, 0, 0, 1 },
    { "FxOpd67A3", asm_FxOpd67A3, c_FxOp67A3, 0, 0, 1 },
    { "FxOpd68", asm_FxOpd68, c_FxOp68, 0, 0, 1 },
    { "FxOpd68A1", asm_FxOpd68A1, c_FxOp68A1, 0, 0, 1 },
    { "FxOpd68A2", asm_FxOpd68A2, c_FxOp68A2, 0, 0, 1 },
    { "FxOpd68A3", asm_FxOpd68A3, c_FxOp68A3, 0, 0, 1 },
    { "FxOpd69", asm_FxOpd69, c_FxOp69, 0, 0, 1 },
    { "FxOpd69A1", asm_FxOpd69A1, c_FxOp69A1, 0, 0, 1 },
    { "FxOpd69A2", asm_FxOpd69A2, c_FxOp69A2, 0, 0, 1 },
    { "FxOpd69A3", asm_FxOpd69A3, c_FxOp69A3, 0, 0, 1 },
    { "FxOpd6A", asm_FxOpd6A, c_FxOp6A, 0, 0, 1 },
    { "FxOpd6AA1", asm_FxOpd6AA1, c_FxOp6AA1, 0, 0, 1 },
    { "FxOpd6AA2", asm_FxOpd6AA2, c_FxOp6AA2, 0, 0, 1 },
    { "FxOpd6AA3", asm_FxOpd6AA3, c_FxOp6AA3, 0, 0, 1 },
    { "FxOpd6B", asm_FxOpd6B, c_FxOp6B, 0, 0, 1 },
    { "FxOpd6BA1", asm_FxOpd6BA1, c_FxOp6BA1, 0, 0, 1 },
    { "FxOpd6BA2", asm_FxOpd6BA2, c_FxOp6BA2, 0, 0, 1 },
    { "FxOpd6BA3", asm_FxOpd6BA3, c_FxOp6BA3, 0, 0, 1 },
    { "FxOpd6C", asm_FxOpd6C, c_FxOp6C, 0, 0, 1 },
    { "FxOpd6CA1", asm_FxOpd6CA1, c_FxOp6CA1, 0, 0, 1 },
    { "FxOpd6CA2", asm_FxOpd6CA2, c_FxOp6CA2, 0, 0, 1 },
    { "FxOpd6CA3", asm_FxOpd6CA3, c_FxOp6CA3, 0, 0, 1 },
    { "FxOpd6D", asm_FxOpd6D, c_FxOp6D, 0, 0, 1 },
    { "FxOpd6DA1", asm_FxOpd6DA1, c_FxOp6DA1, 0, 0, 1 },
    { "FxOpd6DA2", asm_FxOpd6DA2, c_FxOp6DA2, 0, 0, 1 },
    { "FxOpd6DA3", asm_FxOpd6DA3, c_FxOp6DA3, 0, 0, 1 },
    { "FxOpd6E", asm_FxOpd6E, c_FxOp6E, 0, 0, 1 },
    { "FxOpd6EA1", asm_FxOpd6EA1, c_FxOp6EA1, 0, 0, 1 },
    { "FxOpd6EA2", asm_FxOpd6EA2, c_FxOp6EA2, 0, 0, 1 },
    { "FxOpd6EA3", asm_FxOpd6EA3, c_FxOp6EA3, 0, 0, 1 },
    { "FxOpd6F", asm_FxOpd6F, c_FxOp6F, 0, 0, 1 },
    { "FxOpd6FA1", asm_FxOpd6FA1, c_FxOp6FA1, 0, 0, 1 },
    { "FxOpd6FA2", asm_FxOpd6FA2, c_FxOp6FA2, 0, 0, 1 },
    { "FxOpd6FA3", asm_FxOpd6FA3, c_FxOp6FA3, 0, 0, 1 },
    { "FxOpd70", asm_FxOpd70, c_FxOp70, 0, 0, 1 },
    { "FxOpd71", asm_FxOpd71, c_FxOp71, 0, 0, 1 },
    { "FxOpd71A1", asm_FxOpd71A1, c_FxOp71A1, 0, 0, 1 },
    { "FxOpd71A2", asm_FxOpd71A2, c_FxOp71A2, 0, 0, 1 },
    { "FxOpd71A3", asm_FxOpd71A3, c_FxOp71A3, 0, 0, 1 },
    { "FxOpd72", asm_FxOpd72, c_FxOp72, 0, 0, 1 },
    { "FxOpd72A1", asm_FxOpd72A1, c_FxOp72A1, 0, 0, 1 },
    { "FxOpd72A2", asm_FxOpd72A2, c_FxOp72A2, 0, 0, 1 },
    { "FxOpd72A3", asm_FxOpd72A3, c_FxOp72A3, 0, 0, 1 },
    { "FxOpd73", asm_FxOpd73, c_FxOp73, 0, 0, 1 },
    { "FxOpd73A1", asm_FxOpd73A1, c_FxOp73A1, 0, 0, 1 },
    { "FxOpd73A2", asm_FxOpd73A2, c_FxOp73A2, 0, 0, 1 },
    { "FxOpd73A3", asm_FxOpd73A3, c_FxOp73A3, 0, 0, 1 },
    { "FxOpd74", asm_FxOpd74, c_FxOp74, 0, 0, 1 },
    { "FxOpd74A1", asm_FxOpd74A1, c_FxOp74A1, 0, 0, 1 },
    { "FxOpd74A2", asm_FxOpd74A2, c_FxOp74A2, 0, 0, 1 },
    { "FxOpd74A3", asm_FxOpd74A3, c_FxOp74A3, 0, 0, 1 },
    { "FxOpd75", asm_FxOpd75, c_FxOp75, 0, 0, 1 },
    { "FxOpd75A1", asm_FxOpd75A1, c_FxOp75A1, 0, 0, 1 },
    { "FxOpd75A2", asm_FxOpd75A2, c_FxOp75A2, 0, 0, 1 },
    { "FxOpd75A3", asm_FxOpd75A3, c_FxOp75A3, 0, 0, 1 },
    { "FxOpd76", asm_FxOpd76, c_FxOp76, 0, 0, 1 },
    { "FxOpd76A1", asm_FxOpd76A1, c_FxOp76A1, 0, 0, 1 },
    { "FxOpd76A2", asm_FxOpd76A2, c_FxOp76A2, 0, 0, 1 },
    { "FxOpd76A3", asm_FxOpd76A3, c_FxOp76A3, 0, 0, 1 },
    { "FxOpd77", asm_FxOpd77, c_FxOp77, 0, 0, 1 },
    { "FxOpd77A1", asm_FxOpd77A1, c_FxOp77A1, 0, 0, 1 },
    { "FxOpd77A2", asm_FxOpd77A2, c_FxOp77A2, 0, 0, 1 },
    { "FxOpd77A3", asm_FxOpd77A3, c_FxOp77A3, 0, 0, 1 },
    { "FxOpd78", asm_FxOpd78, c_FxOp78, 0, 0, 1 },
    { "FxOpd78A1", asm_FxOpd78A1, c_FxOp78A1, 0, 0, 1 },
    { "FxOpd78A2", asm_FxOpd78A2, c_FxOp78A2, 0, 0, 1 },
    { "FxOpd78A3", asm_FxOpd78A3, c_FxOp78A3, 0, 0, 1 },
    { "FxOpd79", asm_FxOpd79, c_FxOp79, 0, 0, 1 },
    { "FxOpd79A1", asm_FxOpd79A1, c_FxOp79A1, 0, 0, 1 },
    { "FxOpd79A2", asm_FxOpd79A2, c_FxOp79A2, 0, 0, 1 },
    { "FxOpd79A3", asm_FxOpd79A3, c_FxOp79A3, 0, 0, 1 },
    { "FxOpd7A", asm_FxOpd7A, c_FxOp7A, 0, 0, 1 },
    { "FxOpd7AA1", asm_FxOpd7AA1, c_FxOp7AA1, 0, 0, 1 },
    { "FxOpd7AA2", asm_FxOpd7AA2, c_FxOp7AA2, 0, 0, 1 },
    { "FxOpd7AA3", asm_FxOpd7AA3, c_FxOp7AA3, 0, 0, 1 },
    { "FxOpd7B", asm_FxOpd7B, c_FxOp7B, 0, 0, 1 },
    { "FxOpd7BA1", asm_FxOpd7BA1, c_FxOp7BA1, 0, 0, 1 },
    { "FxOpd7BA2", asm_FxOpd7BA2, c_FxOp7BA2, 0, 0, 1 },
    { "FxOpd7BA3", asm_FxOpd7BA3, c_FxOp7BA3, 0, 0, 1 },
    { "FxOpd7C", asm_FxOpd7C, c_FxOp7C, 0, 0, 1 },
    { "FxOpd7CA1", asm_FxOpd7CA1, c_FxOp7CA1, 0, 0, 1 },
    { "FxOpd7CA2", asm_FxOpd7CA2, c_FxOp7CA2, 0, 0, 1 },
    { "FxOpd7CA3", asm_FxOpd7CA3, c_FxOp7CA3, 0, 0, 1 },
    { "FxOpd7D", asm_FxOpd7D, c_FxOp7D, 0, 0, 1 },
    { "FxOpd7DA1", asm_FxOpd7DA1, c_FxOp7DA1, 0, 0, 1 },
    { "FxOpd7DA2", asm_FxOpd7DA2, c_FxOp7DA2, 0, 0, 1 },
    { "FxOpd7DA3", asm_FxOpd7DA3, c_FxOp7DA3, 0, 0, 1 },
    { "FxOpd7E", asm_FxOpd7E, c_FxOp7E, 0, 0, 1 },
    { "FxOpd7EA1", asm_FxOpd7EA1, c_FxOp7EA1, 0, 0, 1 },
    { "FxOpd7EA2", asm_FxOpd7EA2, c_FxOp7EA2, 0, 0, 1 },
    { "FxOpd7EA3", asm_FxOpd7EA3, c_FxOp7EA3, 0, 0, 1 },
    { "FxOpd7F", asm_FxOpd7F, c_FxOp7F, 0, 0, 1 },
    { "FxOpd7FA1", asm_FxOpd7FA1, c_FxOp7FA1, 0, 0, 1 },
    { "FxOpd7FA2", asm_FxOpd7FA2, c_FxOp7FA2, 0, 0, 1 },
    { "FxOpd7FA3", asm_FxOpd7FA3, c_FxOp7FA3, 0, 0, 1 },
    { "FxOpd80", asm_FxOpd80, c_FxOp80, 0, 0, 1 },
    { "FxOpd80A1", asm_FxOpd80A1, c_FxOp80A1, 0, 0, 1 },
    { "FxOpd80A2", asm_FxOpd80A2, c_FxOp80A2, 0, 0, 1 },
    { "FxOpd80A3", asm_FxOpd80A3, c_FxOp80A3, 0, 0, 1 },
    { "FxOpd81", asm_FxOpd81, c_FxOp81, 0, 0, 1 },
    { "FxOpd81A1", asm_FxOpd81A1, c_FxOp81A1, 0, 0, 1 },
    { "FxOpd81A2", asm_FxOpd81A2, c_FxOp81A2, 0, 0, 1 },
    { "FxOpd81A3", asm_FxOpd81A3, c_FxOp81A3, 0, 0, 1 },
    { "FxOpd82", asm_FxOpd82, c_FxOp82, 0, 0, 1 },
    { "FxOpd82A1", asm_FxOpd82A1, c_FxOp82A1, 0, 0, 1 },
    { "FxOpd82A2", asm_FxOpd82A2, c_FxOp82A2, 0, 0, 1 },
    { "FxOpd82A3", asm_FxOpd82A3, c_FxOp82A3, 0, 0, 1 },
    { "FxOpd83", asm_FxOpd83, c_FxOp83, 0, 0, 1 },
    { "FxOpd83A1", asm_FxOpd83A1, c_FxOp83A1, 0, 0, 1 },
    { "FxOpd83A2", asm_FxOpd83A2, c_FxOp83A2, 0, 0, 1 },
    { "FxOpd83A3", asm_FxOpd83A3, c_FxOp83A3, 0, 0, 1 },
    { "FxOpd84", asm_FxOpd84, c_FxOp84, 0, 0, 1 },
    { "FxOpd84A1", asm_FxOpd84A1, c_FxOp84A1, 0, 0, 1 },
    { "FxOpd84A2", asm_FxOpd84A2, c_FxOp84A2, 0, 0, 1 },
    { "FxOpd84A3", asm_FxOpd84A3, c_FxOp84A3, 0, 0, 1 },
    { "FxOpd85", asm_FxOpd85, c_FxOp85, 0, 0, 1 },
    { "FxOpd85A1", asm_FxOpd85A1, c_FxOp85A1, 0, 0, 1 },
    { "FxOpd85A2", asm_FxOpd85A2, c_FxOp85A2, 0, 0, 1 },
    { "FxOpd85A3", asm_FxOpd85A3, c_FxOp85A3, 0, 0, 1 },
    { "FxOpd86", asm_FxOpd86, c_FxOp86, 0, 0, 1 },
    { "FxOpd86A1", asm_FxOpd86A1, c_FxOp86A1, 0, 0, 1 },
    { "FxOpd86A2", asm_FxOpd86A2, c_FxOp86A2, 0, 0, 1 },
    { "FxOpd86A3", asm_FxOpd86A3, c_FxOp86A3, 0, 0, 1 },
    { "FxOpd87", asm_FxOpd87, c_FxOp87, 0, 0, 1 },
    { "FxOpd87A1", asm_FxOpd87A1, c_FxOp87A1, 0, 0, 1 },
    { "FxOpd87A2", asm_FxOpd87A2, c_FxOp87A2, 0, 0, 1 },
    { "FxOpd87A3", asm_FxOpd87A3, c_FxOp87A3, 0, 0, 1 },
    { "FxOpd88", asm_FxOpd88, c_FxOp88, 0, 0, 1 },
    { "FxOpd88A1", asm_FxOpd88A1, c_FxOp88A1, 0, 0, 1 },
    { "FxOpd88A2", asm_FxOpd88A2, c_FxOp88A2, 0, 0, 1 },
    { "FxOpd88A3", asm_FxOpd88A3, c_FxOp88A3, 0, 0, 1 },
    { "FxOpd89", asm_FxOpd89, c_FxOp89, 0, 0, 1 },
    { "FxOpd89A1", asm_FxOpd89A1, c_FxOp89A1, 0, 0, 1 },
    { "FxOpd89A2", asm_FxOpd89A2, c_FxOp89A2, 0, 0, 1 },
    { "FxOpd89A3", asm_FxOpd89A3, c_FxOp89A3, 0, 0, 1 },
    { "FxOpd8A", asm_FxOpd8A, c_FxOp8A, 0, 0, 1 },
    { "FxOpd8AA1", asm_FxOpd8AA1, c_FxOp8AA1, 0, 0, 1 },
    { "FxOpd8AA2", asm_FxOpd8AA2, c_FxOp8AA2, 0, 0, 1 },
    { "FxOpd8AA3", asm_FxOpd8AA3, c_FxOp8AA3, 0, 0, 1 },
    { "FxOpd8B", asm_FxOpd8B, c_FxOp8B, 0, 0, 1 },
    { "FxOpd8BA1", asm_FxOpd8BA1, c_FxOp8BA1, 0, 0, 1 },
    { "FxOpd8BA2", asm_FxOpd8BA2, c_FxOp8BA2, 0, 0, 1 },
    { "FxOpd8BA3", asm_FxOpd8BA3, c_FxOp8BA3, 0, 0, 1 },
    { "FxOpd8C", asm_FxOpd8C, c_FxOp8C, 0, 0, 1 },
    { "FxOpd8CA1", asm_FxOpd8CA1, c_FxOp8CA1, 0, 0, 1 },
    { "FxOpd8CA2", asm_FxOpd8CA2, c_FxOp8CA2, 0, 0, 1 },
    { "FxOpd8CA3", asm_FxOpd8CA3, c_FxOp8CA3, 0, 0, 1 },
    { "FxOpd8D", asm_FxOpd8D, c_FxOp8D, 0, 0, 1 },
    { "FxOpd8DA1", asm_FxOpd8DA1, c_FxOp8DA1, 0, 0, 1 },
    { "FxOpd8DA2", asm_FxOpd8DA2, c_FxOp8DA2, 0, 0, 1 },
    { "FxOpd8DA3", asm_FxOpd8DA3, c_FxOp8DA3, 0, 0, 1 },
    { "FxOpd8E", asm_FxOpd8E, c_FxOp8E, 0, 0, 1 },
    { "FxOpd8EA1", asm_FxOpd8EA1, c_FxOp8EA1, 0, 0, 1 },
    { "FxOpd8EA2", asm_FxOpd8EA2, c_FxOp8EA2, 0, 0, 1 },
    { "FxOpd8EA3", asm_FxOpd8EA3, c_FxOp8EA3, 0, 0, 1 },
    { "FxOpd8F", asm_FxOpd8F, c_FxOp8F, 0, 0, 1 },
    { "FxOpd8FA1", asm_FxOpd8FA1, c_FxOp8FA1, 0, 0, 1 },
    { "FxOpd8FA2", asm_FxOpd8FA2, c_FxOp8FA2, 0, 0, 1 },
    { "FxOpd8FA3", asm_FxOpd8FA3, c_FxOp8FA3, 0, 0, 1 },
    { "FxOpd90", asm_FxOpd90, c_FxOp90, 6, 0, 1 },
    { "FxOpd91", asm_FxOpd91, c_FxOp91, 0, 1, 1 },
    { "FxOpd92", asm_FxOpd92, c_FxOp92, 0, 2, 1 },
    { "FxOpd93", asm_FxOpd93, c_FxOp93, 0, 3, 1 },
    { "FxOpd94", asm_FxOpd94, c_FxOp94, 0, 4, 1 },
    { "FxOpd95", asm_FxOpd95, c_FxOp95, 0, 0, 1 },
    { "FxOpd96", asm_FxOpd96, c_FxOp96, 0, 0, 1 },
    { "FxOpd96A1", asm_FxOpd96A1, c_FxOp96A1, 0, 0, 1 },
    { "FxOpd97", asm_FxOpd97, c_FxOp97, 0, 0, 1 },
    { "FxOpd98", asm_FxOpd98, c_FxOp98, 4, 8, 1 },
    { "FxOpd98A1", asm_FxOpd98A1, c_FxOp98A1, 5, 8, 1 },
    { "FxOpd99", asm_FxOpd99, c_FxOp99, 4, 9, 1 },
    { "FxOpd99A1", asm_FxOpd99A1, c_FxOp99A1, 5, 9, 1 },
    { "FxOpd9A", asm_FxOpd9A, c_FxOp9A, 4, 10, 1 },
    { "FxOpd9AA1", asm_FxOpd9AA1, c_FxOp9AA1, 5, 10, 1 },
    { "FxOpd9B", asm_FxOpd9B, c_FxOp9B, 4, 11, 1 },
    { "FxOpd9BA1", asm_FxOpd9BA1, c_FxOp9BA1, 5, 11, 1 },
    { "FxOpd9C", asm_FxOpd9C, c_FxOp9C, 4, 12, 1 },
    { "FxOpd9CA1", asm_FxOpd9CA1, c_FxOp9CA1, 5, 12, 1 },
    { "FxOpd9D", asm_FxOpd9D, c_FxOp9D, 4, 13, 1 },
    { "FxOpd9DA1", asm_FxOpd9DA1, c_FxOp9DA1, 5, 13, 1 },
    { "FxOpd9E", asm_FxOpd9E, c_FxOp9E, 0, 0, 1 },
    { "FxOpd9F", asm_FxOpd9F, c_FxOp9F, 0, 0, 1 },
    { "FxOpd9FA1", asm_FxOpd9FA1, c_FxOp9FA1, 0, 0, 1 },
    { "FxOpdA0", asm_FxOpdA0, c_FxOpA0, 0, 0, 1 },
    { "FxOpdA0A1", asm_FxOpdA0A1, c_FxOpA0A1, 3, 0, 1 },
    { "FxOpdA0A2", asm_FxOpdA0A2, c_FxOpA0A2, 3, 0, 1 },
    { "FxOpdA1", asm_FxOpdA1, c_FxOpA1, 0, 0, 1 },
    { "FxOpdA1A1", asm_FxOpdA1A1, c_FxOpA1A1, 3, 0, 1 },
    { "FxOpdA1A2", asm_FxOpdA1A2, c_FxOpA1A2, 3, 0, 1 },
    { "FxOpdA2", asm_FxOpdA2, c_FxOpA2, 0, 0, 1 },
    { "FxOpdA2A1", asm_FxOpdA2A1, c_FxOpA2A1, 3, 0, 1 },
    { "FxOpdA2A2", asm_FxOpdA2A2, c_FxOpA2A2, 3, 0, 1 },
    { "FxOpdA3", asm_FxOpdA3, c_FxOpA3, 0, 0, 1 },
    { "FxOpdA3A1", asm_FxOpdA3A1, c_FxOpA3A1, 3, 0, 1 },
    { "FxOpdA3A2", asm_FxOpdA3A2, c_FxOpA3A2, 3, 0, 1 },
    { "FxOpdA4", asm_FxOpdA4, c_FxOpA4, 0, 0, 1 },
    { "FxOpdA4A1", asm_FxOpdA4A1, c_FxOpA4A1, 3, 0, 1 },
    { "FxOpdA4A2", asm_FxOpdA4A2, c_FxOpA4A2, 3, 0, 1 },
    { "FxOpdA5", asm_FxOpdA5, c_FxOpA5, 0, 0, 1 },
    { "FxOpdA5A1", asm_FxOpdA5A1, c_FxOpA5A1, 3, 0, 1 },
    { "FxOpdA5A2", asm_FxOpdA5A2, c_FxOpA5A2, 3, 0, 1 },
    { "FxOpdA6", asm_FxOpdA6, c_FxOpA6, 0, 0, 1 },
    { "FxOpdA6A1", asm_FxOpdA6A1, c_FxOpA6A1, 3, 0, 1 },
    { "FxOpdA6A2", asm_FxOpdA6A2, c_FxOpA6A2, 3, 0, 1 },
    { "FxOpdA7", asm_FxOpdA7, c_FxOpA7, 0, 0, 1 },
    { "FxOpdA7A1", asm_FxOpdA7A1, c_FxOpA7A1, 3, 0, 1 },
    { "FxOpdA7A2", asm_FxOpdA7A2, c_FxOpA7A2, 3, 0, 1 },
    { "FxOpdA8", asm_FxOpdA8, c_FxOpA8, 0, 0, 1 },
    { "FxOpdA8A1", asm_FxOpdA8A1, c_FxOpA8A1, 3, 0, 1 },
    { "FxOpdA8A2", asm_FxOpdA8A2, c_FxOpA8A2, 3, 0, 1 },
    { "FxOpdA9", asm_FxOpdA9, c_FxOpA9, 0, 0, 1 },
    { "FxOpdA9A1", asm_FxOpdA9A1, c_FxOpA9A1, 3, 0, 1 },
    { "FxOpdA9A2", asm_FxOpdA9A2, c_FxOpA9A2, 3, 0, 1 },
    { "FxOpdAA", asm_FxOpdAA, c_FxOpAA, 0, 0, 1 },
    { "FxOpdAAA1", asm_FxOpdAAA1, c_FxOpAAA1, 3, 0, 1 },
    { "FxOpdAAA2", asm_FxOpdAAA2, c_FxOpAAA2, 3, 0, 1 },
    { "FxOpdAB", asm_FxOpdAB, c_FxOpAB, 0, 0, 1 },
    { "FxOpdABA1", asm_FxOpdABA1, c_FxOpABA1, 3, 0, 1 },
    { "FxOpdABA2", asm_FxOpdABA2, c_FxOpABA2, 3, 0, 1 },
    { "FxOpdAC", asm_FxOpdAC, c_FxOpAC, 0, 0, 1 },
    { "FxOpdACA1", asm_FxOpdACA1, c_FxOpACA1, 3, 0, 1 },
    { "FxOpdACA2", asm_FxOpdACA2, c_FxOpACA2, 3, 0, 1 },
    { "FxOpdAD", asm_FxOpdAD, c_FxOpAD, 0, 0, 1 },
    { "FxOpdADA1", asm_FxOpdADA1, c_FxOpADA1, 3, 0, 1 },
    { "FxOpdADA2", asm_FxOpdADA2, c_FxOpADA2, 3, 0, 1 },
    { "FxOpdAE", asm_FxOpdAE, c_FxOpAE, 0, 0, 1 },
    { "FxOpdAEA1", asm_FxOpdAEA1, c_FxOpAEA1, 3, 0, 1 },
    { "FxOpdAEA2", asm_FxOpdAEA2, c_FxOpAEA2, 3, 0, 1 },
    { "FxOpdAF", asm_FxOpdAF, c_FxOpAF, 0, 0, 1 },
    { "FxOpdAFA1", asm_FxOpdAFA1, c_FxOpAFA1, 3, 0, 1 },
    { "FxOpdAFA2", asm_FxOpdAFA2, c_FxOpAFA2, 3, 0, 1 },
    { "FxOpdB0", asm_FxOpdB0, c_FxOpB0, 0, 0, 1 },
    { "FxOpdB1", asm_FxOpdB1, c_FxOpB1, 0, 0, 1 },
    { "FxOpdB2", asm_FxOpdB2, c_FxOpB2, 0, 0, 1 },
    { "FxOpdB3", asm_FxOpdB3, c_FxOpB3, 0, 0, 1 },
    { "FxOpdB4", asm_FxOpdB4, c_FxOpB4, 0, 0, 1 },
    { "FxOpdB5", asm_FxOpdB5, c_FxOpB5, 0, 0, 1 },
    { "FxOpdB6", asm_FxOpdB6, c_FxOpB6, 0, 0, 1 },
    { "FxOpdB7", asm_FxOpdB7, c_FxOpB7, 0, 0, 1 },
    { "FxOpdB8", asm_FxOpdB8, c_FxOpB8, 0, 0, 1 },
    { "FxOpdB9", asm_FxOpdB9, c_FxOpB9, 0, 0, 1 },
    { "FxOpdBA", asm_FxOpdBA, c_FxOpBA, 0, 0, 1 },
    { "FxOpdBB", asm_FxOpdBB, c_FxOpBB, 0, 0, 1 },
    { "FxOpdBC", asm_FxOpdBC, c_FxOpBC, 0, 0, 1 },
    { "FxOpdBD", asm_FxOpdBD, c_FxOpBD, 0, 0, 1 },
    { "FxOpdBE", asm_FxOpdBE, c_FxOpBE, 0, 0, 1 },
    { "FxOpdBF", asm_FxOpdBF, c_FxOpBF, 0, 0, 1 },
    { "FxOpdC0", asm_FxOpdC0, c_FxOpC0, 0, 0, 1 },
    { "FxOpdC1", asm_FxOpdC1, c_FxOpC1, 0, 0, 1 },
    { "FxOpdC1A1", asm_FxOpdC1A1, c_FxOpC1A1, 0, 0, 1 },
    { "FxOpdC1A2", asm_FxOpdC1A2, c_FxOpC1A2, 0, 0, 1 },
    { "FxOpdC1A3", asm_FxOpdC1A3, c_FxOpC1A3, 0, 0, 1 },
    { "FxOpdC2", asm_FxOpdC2, c_FxOpC2, 0, 0, 1 },
    { "FxOpdC2A1", asm_FxOpdC2A1, c_FxOpC2A1, 0, 0, 1 },
    { "FxOpdC2A2", asm_FxOpdC2A2, c_FxOpC2A2, 0, 0, 1 },
    { "FxOpdC2A3", asm_FxOpdC2A3, c_FxOpC2A3, 0, 0, 1 },
    { "FxOpdC3", asm_FxOpdC3, c_FxOpC3, 0, 0, 1 },
    { "FxOpdC3A1", asm_FxOpdC3A1, c_FxOpC3A1, 0, 0, 1 },
    { "FxOpdC3A2", asm_FxOpdC3A2, c_FxOpC3A2, 0, 0, 1 },
    { "FxOpdC3A3", asm_FxOpdC3A3, c_FxOpC3A3, 0, 0, 1 },
    { "FxOpdC4", asm_FxOpdC4, c_FxOpC4, 0, 0, 1 },
    { "FxOpdC4A1", asm_FxOpdC4A1, c_FxOpC4A1, 0, 0, 1 },
    { "FxOpdC4A2", asm_FxOpdC4A2, c_FxOpC4A2, 0, 0, 1 },
    { "FxOpdC4A3", asm_FxOpdC4A3, c_FxOpC4A3, 0, 0, 1 },
    { "FxOpdC5", asm_FxOpdC5, c_FxOpC5, 0, 0, 1 },
    { "FxOpdC5A1", asm_FxOpdC5A1, c_FxOpC5A1, 0, 0, 1 },
    { "FxOpdC5A2", asm_FxOpdC5A2, c_FxOpC5A2, 0, 0, 1 },
    { "FxOpdC5A3", asm_FxOpdC5A3, c_FxOpC5A3, 0, 0, 1 },
    { "FxOpdC6", asm_FxOpdC6, c_FxOpC6, 0, 0, 1 },
    { "FxOpdC6A1", asm_FxOpdC6A1, c_FxOpC6A1, 0, 0, 1 },
    { "FxOpdC6A2", asm_FxOpdC6A2, c_FxOpC6A2, 0, 0, 1 },
    { "FxOpdC6A3", asm_FxOpdC6A3, c_FxOpC6A3, 0, 0, 1 },
    { "FxOpdC7", asm_FxOpdC7, c_FxOpC7, 0, 0, 1 },
    { "FxOpdC7A1", asm_FxOpdC7A1, c_FxOpC7A1, 0, 0, 1 },
    { "FxOpdC7A2", asm_FxOpdC7A2, c_FxOpC7A2, 0, 0, 1 },
    { "FxOpdC7A3", asm_FxOpdC7A3, c_FxOpC7A3, 0, 0, 1 },
    { "FxOpdC8", asm_FxOpdC8, c_FxOpC8, 0, 0, 1 },
    { "FxOpdC8A1", asm_FxOpdC8A1, c_FxOpC8A1, 0, 0, 1 },
    { "FxOpdC8A2", asm_FxOpdC8A2, c_FxOpC8A2, 0, 0, 1 },
    { "FxOpdC8A3", asm_FxOpdC8A3, c_FxOpC8A3, 0, 0, 1 },
    { "FxOpdC9", asm_FxOpdC9, c_FxOpC9, 0, 0, 1 },
    { "FxOpdC9A1", asm_FxOpdC9A1, c_FxOpC9A1, 0, 0, 1 },
    { "FxOpdC9A2", asm_FxOpdC9A2, c_FxOpC9A2, 0, 0, 1 },
    { "FxOpdC9A3", asm_FxOpdC9A3, c_FxOpC9A3, 0, 0, 1 },
    { "FxOpdCA", asm_FxOpdCA, c_FxOpCA, 0, 0, 1 },
    { "FxOpdCAA1", asm_FxOpdCAA1, c_FxOpCAA1, 0, 0, 1 },
    { "FxOpdCAA2", asm_FxOpdCAA2, c_FxOpCAA2, 0, 0, 1 },
    { "FxOpdCAA3", asm_FxOpdCAA3, c_FxOpCAA3, 0, 0, 1 },
    { "FxOpdCB", asm_FxOpdCB, c_FxOpCB, 0, 0, 1 },
    { "FxOpdCBA1", asm_FxOpdCBA1, c_FxOpCBA1, 0, 0, 1 },
    { "FxOpdCBA2", asm_FxOpdCBA2, c_FxOpCBA2, 0, 0, 1 },
    { "FxOpdCBA3", asm_FxOpdCBA3, c_FxOpCBA3, 0, 0, 1 },
    { "FxOpdCC", asm_FxOpdCC, c_FxOpCC, 0, 0, 1 },
    { "FxOpdCCA1", asm_FxOpdCCA1, c_FxOpCCA1, 0, 0, 1 },
    { "FxOpdCCA2", asm_FxOpdCCA2, c_FxOpCCA2, 0, 0, 1 },
    { "FxOpdCCA3", asm_FxOpdCCA3, c_FxOpCCA3, 0, 0, 1 },
    { "FxOpdCD", asm_FxOpdCD, c_FxOpCD, 0, 0, 1 },
    { "FxOpdCDA1", asm_FxOpdCDA1, c_FxOpCDA1, 0, 0, 1 },
    { "FxOpdCDA2", asm_FxOpdCDA2, c_FxOpCDA2, 0, 0, 1 },
    { "FxOpdCDA3", asm_FxOpdCDA3, c_FxOpCDA3, 0, 0, 1 },
    { "FxOpdCE", asm_FxOpdCE, c_FxOpCE, 0, 0, 1 },
    { "FxOpdCEA1", asm_FxOpdCEA1, c_FxOpCEA1, 0, 0, 1 },
    { "FxOpdCEA2", asm_FxOpdCEA2, c_FxOpCEA2, 0, 0, 1 },
    { "FxOpdCEA3", asm_FxOpdCEA3, c_FxOpCEA3, 0, 0, 1 },
    { "FxOpdCF", asm_FxOpdCF, c_FxOpCF, 0, 0, 1 },
    { "FxOpdCFA1", asm_FxOpdCFA1, c_FxOpCFA1, 0, 0, 1 },
    { "FxOpdCFA2", asm_FxOpdCFA2, c_FxOpCFA2, 0, 0, 1 },
    { "FxOpdCFA3", asm_FxOpdCFA3, c_FxOpCFA3, 0, 0, 1 },
    { "FxOpdD0", asm_FxOpdD0, c_FxOpD0, 0, 0, 1 },
    { "FxOpdD1", asm_FxOpdD1, c_FxOpD1, 0, 0, 1 },
    { "FxOpdD2", asm_FxOpdD2, c_FxOpD2, 0, 0, 1 },
    { "FxOpdD3", asm_FxOpdD3, c_FxOpD3, 0, 0, 1 },
    { "FxOpdD4", asm_FxOpdD4, c_FxOpD4, 0, 0, 1 },
    { "FxOpdD5", asm_FxOpdD5, c_FxOpD5, 0, 0, 1 },
    { "FxOpdD6", asm_FxOpdD6, c_FxOpD6, 0, 0, 1 },
    { "FxOpdD7", asm_FxOpdD7, c_FxOpD7, 0, 0, 1 },
    { "FxOpdD8", asm_FxOpdD8, c_FxOpD8, 0, 0, 1 },
    { "FxOpdD9", asm_FxOpdD9, c_FxOpD9, 0, 0, 1 },
    { "FxOpdDA", asm_FxOpdDA, c_FxOpDA, 0, 0, 1 },
    { "FxOpdDB", asm_FxOpdDB, c_FxOpDB, 0, 0, 1 },
    { "FxOpdDC", asm_FxOpdDC, c_FxOpDC, 0, 0, 1 },
    { "FxOpdDD", asm_FxOpdDD, c_FxOpDD, 0, 0, 1 },
    { "FxOpdDE", asm_FxOpdDE, c_FxOpDE, 0, 0, 1 },
    { "FxOpdDF", asm_FxOpdDF, c_FxOpDF, 0, 0, 1 },
    { "FxOpdDFA2", asm_FxOpdDFA2, c_FxOpDFA2, 0, 0, 1 },
    { "FxOpdDFA3", asm_FxOpdDFA3, c_FxOpDFA3, 0, 0, 1 },
    { "FxOpdE0", asm_FxOpdE0, c_FxOpE0, 0, 0, 1 },
    { "FxOpdE1", asm_FxOpdE1, c_FxOpE1, 0, 0, 1 },
    { "FxOpdE2", asm_FxOpdE2, c_FxOpE2, 0, 0, 1 },
    { "FxOpdE3", asm_FxOpdE3, c_FxOpE3, 0, 0, 1 },
    { "FxOpdE4", asm_FxOpdE4, c_FxOpE4, 0, 0, 1 },
    { "FxOpdE5", asm_FxOpdE5, c_FxOpE5, 0, 0, 1 },
    { "FxOpdE6", asm_FxOpdE6, c_FxOpE6, 0, 0, 1 },
    { "FxOpdE7", asm_FxOpdE7, c_FxOpE7, 0, 0, 1 },
    { "FxOpdE8", asm_FxOpdE8, c_FxOpE8, 0, 0, 1 },
    { "FxOpdE9", asm_FxOpdE9, c_FxOpE9, 0, 0, 1 },
    { "FxOpdEA", asm_FxOpdEA, c_FxOpEA, 0, 0, 1 },
    { "FxOpdEB", asm_FxOpdEB, c_FxOpEB, 0, 0, 1 },
    { "FxOpdEC", asm_FxOpdEC, c_FxOpEC, 0, 0, 1 },
    { "FxOpdED", asm_FxOpdED, c_FxOpED, 0, 0, 1 },
    { "FxOpdEE", asm_FxOpdEE, c_FxOpEE, 0, 0, 1 },
    { "FxOpdEF", asm_FxOpdEF, c_FxOpEF, 0, 0, 1 },
    { "FxOpdEFA1", asm_FxOpdEFA1, c_FxOpEFA1, 0, 0, 1 },
    { "FxOpdEFA2", asm_FxOpdEFA2, c_FxOpEFA2, 0, 0, 1 },
    { "FxOpdEFA3", asm_FxOpdEFA3, c_FxOpEFA3, 0, 0, 1 },
    { "FxOpdF0", asm_FxOpdF0, c_FxOpF0, 0, 0, 1 },
    { "FxOpdF0A1", asm_FxOpdF0A1, c_FxOpF0A1, 2, 0, 1 },
    { "FxOpdF0A2", asm_FxOpdF0A2, c_FxOpF0A2, 2, 0, 1 },
    { "FxOpdF1", asm_FxOpdF1, c_FxOpF1, 0, 0, 1 },
    { "FxOpdF1A1", asm_FxOpdF1A1, c_FxOpF1A1, 2, 0, 1 },
    { "FxOpdF1A2", asm_FxOpdF1A2, c_FxOpF1A2, 2, 0, 1 },
    { "FxOpdF2", asm_FxOpdF2, c_FxOpF2, 0, 0, 1 },
    { "FxOpdF2A1", asm_FxOpdF2A1, c_FxOpF2A1, 2, 0, 1 },
    { "FxOpdF2A2", asm_FxOpdF2A2, c_FxOpF2A2, 2, 0, 1 },
    { "FxOpdF3", asm_FxOpdF3, c_FxOpF3, 0, 0, 1 },
    { "FxOpdF3A1", asm_FxOpdF3A1, c_FxOpF3A1, 2, 0, 1 },
    { "FxOpdF3A2", asm_FxOpdF3A2, c_FxOpF3A2, 2, 0, 1 },
    { "FxOpdF4", asm_FxOpdF4, c_FxOpF4, 0, 0, 1 },
    { "FxOpdF4A1", asm_FxOpdF4A1, c_FxOpF4A1, 2, 0, 1 },
    { "FxOpdF4A2", asm_FxOpdF4A2, c_FxOpF4A2, 2, 0, 1 },
    { "FxOpdF5", asm_FxOpdF5, c_FxOpF5, 0, 0, 1 },
    { "FxOpdF5A1", asm_FxOpdF5A1, c_FxOpF5A1, 2, 0, 1 },
    { "FxOpdF5A2", asm_FxOpdF5A2, c_FxOpF5A2, 2, 0, 1 },
    { "FxOpdF6", asm_FxOpdF6, c_FxOpF6, 0, 0, 1 },
    { "FxOpdF6A1", asm_FxOpdF6A1, c_FxOpF6A1, 2, 0, 1 },
    { "FxOpdF6A2", asm_FxOpdF6A2, c_FxOpF6A2, 2, 0, 1 },
    { "FxOpdF7", asm_FxOpdF7, c_FxOpF7, 0, 0, 1 },
    { "FxOpdF7A1", asm_FxOpdF7A1, c_FxOpF7A1, 2, 0, 1 },
    { "FxOpdF7A2", asm_FxOpdF7A2, c_FxOpF7A2, 2, 0, 1 },
    { "FxOpdF8", asm_FxOpdF8, c_FxOpF8, 0, 0, 1 },
    { "FxOpdF8A1", asm_FxOpdF8A1, c_FxOpF8A1, 2, 0, 1 },
    { "FxOpdF8A2", asm_FxOpdF8A2, c_FxOpF8A2, 2, 0, 1 },
    { "FxOpdF9", asm_FxOpdF9, c_FxOpF9, 0, 0, 1 },
    { "FxOpdF9A1", asm_FxOpdF9A1, c_FxOpF9A1, 2, 0, 1 },
    { "FxOpdF9A2", asm_FxOpdF9A2, c_FxOpF9A2, 2, 0, 1 },
    { "FxOpdFA", asm_FxOpdFA, c_FxOpFA, 0, 0, 1 },
    { "FxOpdFAA1", asm_FxOpdFAA1, c_FxOpFAA1, 2, 0, 1 },
    { "FxOpdFAA2", asm_FxOpdFAA2, c_FxOpFAA2, 2, 0, 1 },
    { "FxOpdFB", asm_FxOpdFB, c_FxOpFB, 0, 0, 1 },
    { "FxOpdFBA1", asm_FxOpdFBA1, c_FxOpFBA1, 2, 0, 1 },
    { "FxOpdFBA2", asm_FxOpdFBA2, c_FxOpFBA2, 2, 0, 1 },
    { "FxOpdFC", asm_FxOpdFC, c_FxOpFC, 0, 0, 1 },
    { "FxOpdFCA1", asm_FxOpdFCA1, c_FxOpFCA1, 2, 0, 1 },
    { "FxOpdFCA2", asm_FxOpdFCA2, c_FxOpFCA2, 2, 0, 1 },
    { "FxOpdFD", asm_FxOpdFD, c_FxOpFD, 0, 0, 1 },
    { "FxOpdFDA1", asm_FxOpdFDA1, c_FxOpFDA1, 2, 0, 1 },
    { "FxOpdFDA2", asm_FxOpdFDA2, c_FxOpFDA2, 2, 0, 1 },
    { "FxOpdFE", asm_FxOpdFE, c_FxOpFE, 0, 0, 1 },
    { "FxOpdFEA1", asm_FxOpdFEA1, c_FxOpFEA1, 2, 0, 1 },
    { "FxOpdFEA2", asm_FxOpdFEA2, c_FxOpFEA2, 2, 0, 1 },
    { "FxOpdFF", asm_FxOpdFF, c_FxOpFF, 0, 0, 1 },
    { "FxOpdFFA1", asm_FxOpdFFA1, c_FxOpFFA1, 2, 0, 1 },
    { "FxOpdFFA2", asm_FxOpdFFA2, c_FxOpFFA2, 2, 0, 1 },
};

/* A register value, biased towards the 16-bit boundaries. The arithmetic here
 * is 16-bit inside a 32-bit register, so plain random values almost never carry
 * or borrow out of the low half (1 in 65536) and would leave a wrong-width
 * INC/DEC or ADD/SUB indistinguishable from a correct one. */
static u4 dt_reg(void)
{
    u4 const hi = dt_u32() & 0xFFFF0000u;

    switch (dt_mod(8)) {
    case 0:
        return hi | 0xFFFFu;
    case 1:
        return hi;
    case 2:
        return hi | 0x8000u;
    case 3:
        return hi | 0x7FFFu;
    case 4:
        return hi | (dt_u32() & 0xFu); /* small, like the immediate forms */
    default:
        return dt_u32();
    }
}

/* SuperFX RAM for the load/store handlers. The address is the raw register
 * value added to SfxRAMMem, so the difftest masks the one register a given
 * handler uses as its address (fxcase.addr_reg) down to this window. A power of
 * two keeps STW's second byte, at addr^1, in range as well. */
#define FXRAM_SIZE 0x400
static u1 fxram[FXRAM_SIZE];
static u1 fxram_init[FXRAM_SIZE];

/* One instruction stream to branch around in. 0x100 of slack either side lets
 * a full -128..+127 displacement stay inside it. */
static u1 code[0x400];

/* Everything a handler is allowed to change. */
typedef struct {
    u4 pc, cx, src, dst;
    u4 spc, scx, ssrc, sdst, hits;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk;
    u4 table, idx, stub_b;
    u4 cbr, pbr, cacheactive, cpb;
    u4 rambr, rombr, rammem;
    u4 colr, por, clineloc;
    u4 sfr, pipe, numops, changeops, sfxproc, endloop, plotidx, plothits;
    u4 flagnz;
    u1 plot[FXPLOT_SIZE];
    u4 pcal[4], plottab[4];
    u4 lastramadr;
    u1 ram[FXRAM_SIZE];
} snapshot;

/* The state both runs start from, so the second run cannot see the first's
 * leftovers. */
typedef struct {
    u4 pc_off, cx;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk, cpb, crom;
    u4 src_reg, dst_reg, lastramadr;
    u4 cbr, pbr, cacheactive, stub_r15sk, stub_r15, stub_wr15sk;
    u4 rambr, rombr, nrambanks, romoff;
    u4 colr, por, scmr;
    u4 sfr, cfgr, pipe, numops, changeops, sfxproc;
    u4 scbr;
    int plot;
    int mem;
} setup;

static void run(void (*fn)(void), setup const* in, int asm_side, int is_d, snapshot* out)
{
    u4(*const tab)[1024] = asm_side ? tab_asm : tab_c;

    StubPC = StubCX = StubSrc = StubDst = StubHits = StubTable = StubB = StubEndLoop = StubPlotIdx = StubPlotHits = StubIdx = 0;
    if (in->mem) {
        memcpy(fxram, fxram_init, sizeof fxram);
    }
    SfxRAMMem = (u4)(uintptr_t)fxram;
    SfxLastRamAdr = (u4)(uintptr_t)fxram + in->lastramadr;
    memcpy(SfxR0, in->regs, sizeof SfxR0);
    SfxSignZero = in->signzero;
    SfxOverflow = in->overflow;
    SfxCarry = in->carry;
    SfxB = in->b;

    withr15sk = in->r15sk;
    SfxCPB = in->cpb;
    SfxCROM = in->crom;
    SfxCBR = in->cbr;
    SfxPBR = in->pbr;
    SfxCacheActive = in->cacheactive;
    StubR15sk = in->stub_r15sk;
    StubR15 = in->stub_r15;
    StubWrR15sk = in->stub_wr15sk;
    SfxRAMBR = in->rambr;
    SfxROMBR = in->rombr;
    SfxnRamBanks = in->nrambanks;
    /* RPIX addresses tile data through sfxramdata, so it points at the plot
       buffer; RAMB only ever computes an address from it, never derefs it. */
    sfxramdata = fxplot;
    SfxRomBuffer = (u4)(uintptr_t)fxrom + in->romoff;
    SfxCOLR = in->colr;
    SfxPOR = in->por;
    SfxSCMR = in->scmr;
    SfxSFR = in->sfr;
    SfxCFGR = in->cfgr;
    SfxPIPE = in->pipe;
    NumberOfOpcodes = in->numops;
    ChangeOps = in->changeops;
    SFXProc = in->sfxproc;
    SfxSCBR = in->scbr;
    SCBRrel = (u4)(uintptr_t)fxplot;
    sfxclineloc = (u4)(uintptr_t)fxlines;
    if (in->plot) {
        memcpy(fxplot, fxplot_init, sizeof fxplot);
    }
    /* Put this side's stubs back: CMODE patches $4C in all four tables. */
    memcpy(FxTable, tab[0], sizeof FxTable);
    memcpy(FxTableb, tab[1], sizeof FxTableb);
    memcpy(FxTablec, tab[2], sizeof FxTablec);
    memcpy(FxTabled, tab[3], sizeof FxTabled);

    FxSeamPC = code + in->pc_off;
    FxSeamSrc = SfxR0 + in->src_reg;
    FxSeamDst = SfxR0 + in->dst_reg;
    FxSeamCX = in->cx;

    if (asm_side) {
        asm_fxcall((void*)fn);
    } else if (is_d) {
        /* What the fxdop thunk and its FXReturn tail used to do around the
           body, and what MainLoop does now: run it, spend an opcode, then
           either chain to the next handler or leave the loop. */
        FxLoopDone = 0;
        fn();
        if (fx_loop_next()) {
            FxDispatch(FxTabled);
        } else {
            StubEndLoop++;
        }
    } else {
        fn();
    }

    out->pc = (u4)(FxSeamPC - code);
    out->cx = FxSeamCX;
    out->src = (u4)((u1*)FxSeamSrc - (u1*)SfxR0);
    out->dst = (u4)((u1*)FxSeamDst - (u1*)SfxR0);
    out->spc = StubPC ? StubPC - (u4)(uintptr_t)code : 0;
    out->scx = StubCX;
    out->ssrc = StubSrc ? StubSrc - (u4)(uintptr_t)SfxR0 : 0;
    out->sdst = StubDst ? StubDst - (u4)(uintptr_t)SfxR0 : 0;
    out->hits = StubHits;
    out->table = StubTable;
    out->idx = StubIdx;
    out->stub_b = StubB;
    /* Store as an offset: the absolute pointer is the same for both runs, but
     * an offset is what a reader can actually interpret. */
    out->lastramadr = SfxLastRamAdr - (u4)(uintptr_t)fxram;
    if (in->mem) {
        memcpy(out->ram, fxram, sizeof out->ram);
    }
    memcpy(out->regs, SfxR0, sizeof out->regs);
    out->signzero = SfxSignZero;
    out->overflow = SfxOverflow;
    out->carry = SfxCarry;
    out->b = SfxB;
    out->rombuffer = SfxRomBuffer;
    out->r15sk = withr15sk;
    out->cbr = SfxCBR;
    out->pbr = SfxPBR;
    out->cacheactive = SfxCacheActive;
    out->cpb = SfxCPB;
    out->rambr = SfxRAMBR;
    out->rombr = SfxROMBR;
    out->rammem = SfxRAMMem;
    out->colr = SfxCOLR;
    out->por = SfxPOR;
    out->clineloc = sfxclineloc;
    out->sfr = SfxSFR;
    out->pipe = SfxPIPE;
    out->numops = NumberOfOpcodes;
    out->changeops = ChangeOps;
    out->sfxproc = SFXProc;
    out->endloop = StubEndLoop;
    out->plotidx = StubPlotIdx;
    out->plothits = StubPlotHits;
    out->flagnz = flagnz;
    if (in->plot) {
        memcpy(out->plot, fxplot, sizeof out->plot);
    }
    out->pcal[0] = fxbit01pcal;
    out->pcal[1] = fxbit23pcal;
    out->pcal[2] = fxbit45pcal;
    out->pcal[3] = fxbit67pcal;
    /* Report zero when CMODE left the entry alone: the two sides start from
       different stubs, so only the PLOTJmp entry it picks is comparable. */
    out->plottab[0] = FxTable[0x4C] == tab[0][0x4C] ? 0 : FxTable[0x4C];
    out->plottab[1] = FxTableb[0x4C] == tab[1][0x4C] ? 0 : FxTableb[0x4C];
    out->plottab[2] = FxTablec[0x4C] == tab[2][0x4C] ? 0 : FxTablec[0x4C];
    out->plottab[3] = FxTabled[0x4C] == tab[3][0x4C] ? 0 : FxTabled[0x4C];
}

/* --- MainLoop -------------------------------------------------------------
 *
 * The loop is tested on its own, with stub handlers in the d table: each stub
 * eats one byte of the instruction stream, refetches the opcode byte, and
 * moves the source/destination register and the ALT mode, which is exactly the
 * state the prologue loads and the epilogue writes back. One opcode byte is
 * wired to a stub that ends the loop the way STOP does.
 */
u4 StubSetCh, StubSetSrc, StubSetDst;
extern void asm_MainLoop(void); /* _fxops.o */
extern void loopstub(void), loopstop(void); /* _fxops.o */

static void loopbody_c(void)
{
    StubHits++;
    FxSeamPC++;
    FxSeamCX = (StubSetCh << 8) | *FxSeamPC;
    FxSeamSrc = (u4*)(uintptr_t)StubSetSrc;
    FxSeamDst = (u4*)(uintptr_t)StubSetDst;
}

static void loopstub_c(void) { loopbody_c(); }

static void loopstop_c(void)
{
    loopbody_c();
    FxLoopDone = 1;
}

typedef struct {
    u4 pc, pipe, sfr, sreg, dreg, rambr, numops;
    u4 setch, setsrc, setdst, stopop;
} mainloop_setup;

typedef struct {
    u4 r15, pipe, sfr, sreg, dreg, rammem, numops, hits;
} mainloop_snapshot;

static void run_mainloop(mainloop_setup const* in, int asm_side, mainloop_snapshot* out)
{
    u4 const stub = (u4)(uintptr_t)(asm_side ? loopstub : loopstub_c);
    u4 const stop = (u4)(uintptr_t)(asm_side ? loopstop : loopstop_c);

    for (int i = 0; i < 1024; i++) {
        FxTabled[i] = (i & 0xFF) == in->stopop ? stop : stub;
    }
    StubHits = 0;
    SfxCPB = (u4)(uintptr_t)code;
    SfxR0[15] = in->pc;
    SfxPIPE = in->pipe;
    SfxSFR = in->sfr;
    SfxSREG = in->sreg;
    SfxDREG = in->dreg;
    SfxRAMBR = in->rambr;
    SfxRAMMem = 0;
    NumberOfOpcodes = in->numops;
    sfxramdata = fxplot;
    StubSetCh = in->setch;
    StubSetSrc = (u4)(uintptr_t)(SfxR0 + in->setsrc);
    StubSetDst = (u4)(uintptr_t)(SfxR0 + in->setdst);

    if (asm_side) {
        asm_fxcall((void*)asm_MainLoop);
    } else {
        MainLoop();
    }

    out->r15 = SfxR0[15];
    out->pipe = SfxPIPE;
    out->sfr = SfxSFR;
    out->sreg = SfxSREG;
    out->dreg = SfxDREG;
    out->rammem = SfxRAMMem;
    out->numops = NumberOfOpcodes;
    out->hits = StubHits;
}

int main(void)
{
    for (int i = 0; i < 256; i++) {
        fxbit01[i] = dt_u32();
        fxbit23[i] = dt_u32();
        fxbit45[i] = dt_u32();
        fxbit67[i] = dt_u32();
    }
    /* Each stub is 32 bytes; the a and b halves are distinct code addresses so
       a wrong table or index shows up as a different pointer. */
    for (int i = 0; i < 64; i++) {
        PLOTJmpa[i] = (u4)(uintptr_t)(plotstubs + i * 32);
        PLOTJmpb[i] = (u4)(uintptr_t)(plotstubs + (64 + i) * 32);
    }
    /* Small tile numbers keep every plane write inside fxplot; the occasional
       0xFFFFFFFF exercises the off-screen path. */
    for (int i = 0; i < FXLINE_ENTRIES; i++) {
        fxlines[i] = dt_mod(8) == 0 ? 0xFFFFFFFFu : dt_mod(64);
    }
    for (int i = 0; i < 256; i++) {
        fxxand[i] = ~(0x0101u << ((i & 7) ^ 7));
    }
    sfxramdata = fxplot;
    sfx128lineloc = (u4)(uintptr_t)fxlines;
    sfx160lineloc = (u4)(uintptr_t)fxlines;
    sfx192lineloc = (u4)(uintptr_t)fxlines;
    sfxobjlineloc = (u4)(uintptr_t)fxlines;

    for (int i = 0; i < 1024; i++) {
        tab_asm[0][i] = (u4)(uintptr_t)(idxa_asm + i * IDXSTUB);
        tab_asm[1][i] = (u4)(uintptr_t)(idxb_asm + i * IDXSTUB);
        tab_asm[2][i] = (u4)(uintptr_t)(idxc_asm + i * IDXSTUB);
        tab_asm[3][i] = (u4)(uintptr_t)(idxd_asm + i * IDXSTUB);
        tab_c[0][i] = (u4)(uintptr_t)(idxa_c + i * IDXSTUB);
        tab_c[1][i] = (u4)(uintptr_t)(idxb_c + i * IDXSTUB);
        tab_c[2][i] = (u4)(uintptr_t)(idxc_c + i * IDXSTUB);
        tab_c[3][i] = (u4)(uintptr_t)(idxd_c + i * IDXSTUB);
    }

    DT_MAIN(20260727, 1000000)
    {
        fxcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot a, c;
        setup in;

        dt_fill(code, sizeof code);
        in.pc_off = 0x100 + dt_mod(0x200);
        in.cx = (dt_mod(4) << 8) | dt_mod(256);
        for (int i = 0; i < 16; i++) {
            in.regs[i] = dt_reg();
        }
        /* Bias the flags towards the interesting bits so both arms of every
         * condition get hit, not just random noise. */
        in.mem = k->mem;
        in.lastramadr = dt_mod(FXRAM_SIZE);
        in.cbr = dt_mod(2) ? dt_u32() & 0xFFF0u : dt_u32();
        in.pbr = dt_u32();
        in.cacheactive = dt_mod(2) ? dt_u32() : (dt_u32() & ~0xFFu) | 1u;
        in.stub_r15sk = dt_mod(2) ? 1 : dt_u32();
        /* Kept inside the instruction stream: several handlers rebuild the
           program counter as SfxCPB + R15. */
        in.stub_r15 = dt_mod(sizeof code);
        in.stub_wr15sk = dt_mod(2);
        in.rambr = dt_u32();
        in.rombr = dt_u32();
        /* RAMB masks with nRamBanks-1, so keep it to the powers of two the
           hardware actually reports. */
        in.nrambanks = 1u << dt_mod(4);
        in.romoff = dt_mod(FXROM_SIZE);
        in.colr = dt_u32();
        /* POR bits 2, 3 and 4 steer COLOR and CMODE; SCMR bits 0-1 and 2/5
           pick the plot variant and the screen height. */
        in.por = dt_u32();
        in.scmr = dt_u32();
        in.sfr = dt_u32();
        /* CFGR bit 7 masks the STOP interrupt, so hit both settings. */
        in.cfgr = dt_mod(2) ? dt_u32() | 0x80u : dt_u32() & ~0x80u;
        in.pipe = dt_u32();
        /* The d table's FXReturn decrements this and leaves the loop when it
           goes negative, so cover both outcomes. */
        in.numops = dt_mod(2) ? dt_mod(3) : dt_u32();
        in.changeops = dt_u32();
        in.sfxproc = dt_u32();
        /* RPIX builds its address from SCBR<<10 + sfxramdata, so keep the base
           at zero and let the tile number do the addressing. */
        in.scbr = 0;
        in.plot = k->mem == 7;
        in.signzero = dt_u32() & (dt_mod(2) ? 0x000FFFFFu : 0x0008F00Fu);
        /* Keep the high bytes live: FROM rN writes only the *low* byte of
         * SfxOverflow, so a mask here would hide a wrong-width store. */
        in.overflow = dt_u32() & (dt_mod(2) ? 0xFFFFFFFFu : 0x01u);
        in.carry = dt_u32() & (dt_mod(2) ? 0xFFFFFFFFu : 0x01u);
        /* SfxB picks between the two forms of TO/FROM, so hit both halves. */
        in.b = dt_mod(2) ? dt_u32() | 1u : dt_u32() & ~1u;
        in.rombuffer = dt_u32();
        in.r15sk = dt_u32();
        in.crom = dt_u32();
        in.src_reg = dt_mod(16);
        in.dst_reg = dt_mod(16);
        /* TO/FROM R15 rebuild the program counter as SfxCPB + R15, so CPB has
         * to be the real code base or the result lands outside the buffer. */
        in.cpb = (u4)(uintptr_t)code;
        in.regs[15] = dt_mod(sizeof code);

        /* Clamp anything the handler will use as an address. Done last: some of
           it depends on src_reg. */
        if (k->mem) {
            /* Fresh RAM so a load cannot pass by reading a stale value. */
            dt_fill(fxram_init, sizeof fxram_init);
        }
        dt_fill(fxrom, sizeof fxrom);
        switch (k->mem) {
        case 1:
            in.regs[k->addr_reg] &= FXRAM_SIZE - 1;
            break;
        case 2:
            /* Mask the immediate's high byte in the instruction stream. */
            code[in.pc_off + 1] &= (FXRAM_SIZE >> 8) - 1;
            break;
        case 4:
            /* JMP builds the program counter as SfxCPB + Rn. */
            in.regs[k->addr_reg] &= sizeof code - 1;
            break;
        case 7:
            /* PLOT/RPIX read only the low bytes of R1 and R2, but the packed
               index uses R2's upper half, so keep both to 16 bits. */
            in.regs[1] &= 0xFFFFu;
            in.regs[2] &= 0xFFFFu;
            dt_fill(fxplot_init, sizeof fxplot_init);
            break;
        case 5:
            /* LJMP takes the bank base from SfxMemTable and adds the source
               register, so point every bank at the instruction stream. */
            for (int b = 0; b < 256; b++) {
                SfxMemTable[b] = (u4)(uintptr_t)code;
            }
            in.regs[in.src_reg] &= sizeof code - 1;
            break;
        default:
            break;
        }

        if (getenv("DTRACE"))
            fprintf(stderr, "it=%ld %s cx=%x\n", dt_it, k->name, in.cx);
        run(k->asm_fn, &in, 1, 0, &a);
        run(k->c_fn, &in, 0, k->c_is_d, &c);

        DT_EQ(k->name, a.pc, c.pc);
        DT_EQ("ecx", a.cx, c.cx);
        DT_EQ("esi", a.src, c.src);
        DT_EQ("edi", a.dst, c.dst);
        DT_EQ("next-opcode dispatch count", a.hits, c.hits);
        DT_EQ("next-opcode table", a.table, c.table);
        DT_EQ("next-opcode table index", a.idx, c.idx);
        DT_EQ("next-opcode SfxB", a.stub_b, c.stub_b);
        DT_EQ("next-opcode pc", a.spc, c.spc);
        DT_EQ("next-opcode ecx", a.scx, c.scx);
        DT_EQ("next-opcode esi", a.ssrc, c.ssrc);
        DT_EQ("next-opcode edi", a.sdst, c.sdst);
        DT_MEM("SfxR0..R15", a.regs, c.regs, sizeof a.regs);
        DT_EQ("SfxSignZero", a.signzero, c.signzero);
        DT_EQ("SfxOverflow", a.overflow, c.overflow);
        DT_EQ("SfxCarry", a.carry, c.carry);
        DT_EQ("SfxB", a.b, c.b);
        DT_EQ("SfxRomBuffer", a.rombuffer, c.rombuffer);
        DT_EQ("withr15sk", a.r15sk, c.r15sk);
        DT_EQ("SfxLastRamAdr", a.lastramadr, c.lastramadr);
        DT_EQ("SfxCBR", a.cbr, c.cbr);
        DT_EQ("SfxPBR", a.pbr, c.pbr);
        DT_EQ("SfxCacheActive", a.cacheactive, c.cacheactive);
        DT_EQ("SfxCPB", a.cpb, c.cpb);
        DT_EQ("SfxRAMBR", a.rambr, c.rambr);
        DT_EQ("SfxROMBR", a.rombr, c.rombr);
        DT_EQ("SfxRAMMem", a.rammem, c.rammem);
        DT_EQ("SfxCOLR", a.colr, c.colr);
        DT_EQ("SfxPOR", a.por, c.por);
        DT_EQ("sfxclineloc", a.clineloc, c.clineloc);
        DT_EQ("SfxSFR", a.sfr, c.sfr);
        DT_EQ("SfxPIPE", a.pipe, c.pipe);
        DT_EQ("NumberOfOpcodes", a.numops, c.numops);
        DT_EQ("ChangeOps", a.changeops, c.changeops);
        DT_EQ("SFXProc", a.sfxproc, c.sfxproc);
        DT_EQ("FXEndLoop reached", a.endloop, c.endloop);
        DT_EQ("PLOT stub index", a.plotidx, c.plotidx);
        DT_EQ("PLOT stub hits", a.plothits, c.plothits);
        DT_EQ("flagnz", a.flagnz, c.flagnz);
        if (k->mem == 7) {
            DT_MEM("plot buffer", a.plot, c.plot, sizeof a.plot);
        }
        DT_MEM("fxbitNNpcal", a.pcal, c.pcal, sizeof a.pcal);
        DT_MEM("FxTable[0x4C]", a.plottab, c.plottab, sizeof a.plottab);
        if (k->mem) {
            DT_MEM("SuperFX RAM", a.ram, c.ram, sizeof a.ram);
        }
        if (dt_bad && DT_SHOW()) {
            printf("  ^ case %s\n", k->name);
        }
    }
    if (dt_fails) {
        printf("SuperFX opcode handlers: FAIL (%d/%ld iterations mismatched)\n",
            dt_fails, dt_iters);
        return 1;
    }
    printf("SuperFX opcode handlers: PASS (%ld iterations bit-identical to asm)\n",
        dt_iters);

    /* Second phase: the loop itself. The handlers are replaced by stubs that
       walk the program counter and move the state the epilogue writes back, so
       what is under test is MainLoop's prologue, threading and epilogue. */
    DT_MAIN(20260729, 200000)
    {
        mainloop_setup m;
        mainloop_snapshot a, c;

        dt_fill(code, sizeof code);
        m.pc = 0x100 + dt_mod(0x100);
        m.pipe = dt_u32();
        m.sfr = dt_u32();
        m.sreg = dt_mod(16);
        m.dreg = dt_mod(16);
        m.rambr = dt_u32();
        /* Bounded: every stub consumes one byte of the instruction stream. */
        m.numops = dt_mod(64);
        m.setch = dt_mod(4);
        m.setsrc = dt_mod(16);
        m.setdst = dt_mod(16);
        /* Which opcode byte ends the loop the way STOP does. */
        m.stopop = dt_mod(256);

        run_mainloop(&m, 1, &a);
        run_mainloop(&m, 0, &c);

        DT_EQ("MainLoop SfxR15", a.r15, c.r15);
        DT_EQ("MainLoop SfxPIPE", a.pipe, c.pipe);
        DT_EQ("MainLoop SfxSFR", a.sfr, c.sfr);
        DT_EQ("MainLoop SfxSREG", a.sreg, c.sreg);
        DT_EQ("MainLoop SfxDREG", a.dreg, c.dreg);
        DT_EQ("MainLoop SfxRAMMem", a.rammem, c.rammem);
        DT_EQ("MainLoop NumberOfOpcodes", a.numops, c.numops);
        DT_EQ("MainLoop opcodes run", a.hits, c.hits);
    }
    DT_DONE("SuperFX MainLoop");
}
