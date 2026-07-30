/* Differential test: the direct-page memory handlers in cpu/memory.asm against
 * the C port in cpu/mem_ops.h.
 *
 * The handlers are leaf functions with a register ABI - ebx the direct-page
 * offset, ecx the direct page register, al/ax the value - and the caller keeps
 * whatever they leave in all three, so the test compares the registers on the
 * way out as well as the memory they wrote.
 *
 * The oracle (_memops.o, built by mkmemops.sh from the pre-port revision) is
 * driven through asm_memcall, which sets the registers up from the same seam
 * block the ported side reads. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../chips/regabi.h" /* REGABI_ENTRY/REGABI_SYM for the trampolines */
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* The assembly indexes wramdataa as a flat 64K array; the ROM map is a base
 * pointer the 8000-FFFF handlers add the address to. A handler can address up
 * to 0xFFFF past the base, so give the ROM window that much plus a byte of
 * slack for a 16-bit read at the very top. */
#define ROM_SIZE 0x10000
u1 wramdataa[65536];
u1 ram7fa[65536]; /* the 7F expansion RAM bank */

/* Cartridge SRAM. ramsizeand masks every access, so a small cart mirrors; the
   test drives several sizes plus the no-SRAM case. */
#define SRAM_SIZE 0x20000
/* sram2 is the second half of the same allocation in initc.c, so the store has
   to hold both windows plus the mask's reach. */
static u1 sram_store[SRAM_SIZE + 0x20000];
static u1 sram_init[SRAM_SIZE + 0x20000];
u1* sram = sram_store;
u1* sram2 = sram_store + 65536;
u4 ramsize, ramsizeand, sramb4save;
/* How much ROM the cart holds; with ramsize it picks whether the top half of
   a 70-7D bank is SRAM or ROM. */
u4 curromspace;

/* Cartridge window state for the regaccessbank* handlers. The mirror is 8K,
   but a 16-bit access at its last byte writes one past it - in the emulator
   sfxramdata points into a larger allocation, so give the buffer the same
   slack rather than letting the stray byte land on the next object. */
#define SFXRAM_SIZE (0x2000 + 2)
static u1 sfxram_store[SFXRAM_SIZE], sfxram_init[SFXRAM_SIZE];
u1* sfxramdata = sfxram_store;
u1 SFXEnable, DSP1Type;
/* An SA-1 cart hands the access to the still-assembly SA-1 handlers. Those are
   out of scope here, so the test keeps SA1Enable clear and stubs the symbols
   the oracle links against; MemCallAsm itself is covered by the paths above. */
u1 SA1Enable;
/* The SA-1's 2K of IRAM, and which processor currently owns the low window.
   Only ecx is range-checked before indexing, so ebx can push the index a
   little past 800h; the real array has the same slack. */
u1 SA1Status;
/* The emulator's IRAM is 2049 bytes; the test's is larger so that a write the
   bounds check should have stopped lands somewhere it can still be compared
   rather than quietly past the end of the array. */
#define IRAM_SIZE 0x2000
u1 IRAM[IRAM_SIZE];
static u1 iram_init[IRAM_SIZE];

/* The SA-1's own RAM: four 64K slices, plus a byte of slack for a 16-bit
   access at the top of one. */
#define SA1RAM_SIZE (4 * 0x10000 + 2)
static u1 sa1ram_store[SA1RAM_SIZE], sa1ram_init[SA1RAM_SIZE];
u1* SA1RAMArea = sa1ram_store;

/* The character-conversion DMA the reads divert to. The stub returns a value
   derived from the address so a wrong one is visible, and logs its calls. */
u4 SA1_in_cc1_dma;
u1 SA1_DMA_VALUE;
u4 SA1_DMA_ADDR;
static u4 cc1_hits, cc1_last;
void SA1_DMA_CC1(void);
/* SA-1 BW-RAM. CurBWPtr is the byte window; SA1BWPtr is the same memory seen
   as a 4- or 2-bit map, which is a different index, so give it its own base. */
#define BWRAM_SIZE 0x20000
static u1 bwram_store[BWRAM_SIZE], bwram_init[BWRAM_SIZE];
u1* CurBWPtr = bwram_store;
u1* SA1BWPtr = bwram_store;
u4 BWShift;
u2 SA1Overflow;

void SA1_DMA_CC1(void)
{
    cc1_hits++;
    cc1_last = SA1_DMA_ADDR;
    SA1_DMA_VALUE = (u1)((SA1_DMA_ADDR ^ (SA1_DMA_ADDR >> 8)) & 0xFFu);
}
void membank0r8SA1(void) { }
void membank0r16SA1(void) { }
void membank0w8SA1(void) { }
void membank0w16SA1(void) { }

/* The DSP1 halves the cartridge path calls. The oracle reaches them through
   the legacy trampolines in chips/dsp1proc.c, so the difftest needs both faces
   of the same pair of functions; these mirror what that file returns. */
u1 c_DSP1Read8b(u4 addr);
void c_DSP1Write8b(u4 addr, u1 val);
u2 c_DSP1Read16b(u4 addr);
void c_DSP1Write16b(u4 addr, u2 val);

static u4 dsp1_last_addr;
static u4 dsp1_last_val;
static u4 dsp1_hits;

u1 c_DSP1Read8b(u4 addr)
{
    dsp1_last_addr = addr;
    dsp1_hits++;
    return addr >= 0x7000 ? 0x80 : 0;
}

void c_DSP1Write8b(u4 addr, u1 val)
{
    dsp1_last_addr = addr;
    dsp1_last_val = val;
    dsp1_hits++;
}

u2 c_DSP1Read16b(u4 addr)
{
    dsp1_last_addr = addr;
    dsp1_hits++;
    return addr >= 0x7000 ? 0x180 : 0;
}

void c_DSP1Write16b(u4 addr, u2 val)
{
    dsp1_last_addr = addr;
    dsp1_last_val = val;
    dsp1_hits++;
}

/* The legacy-ABI faces the oracle calls. */
REGABI_BANK_READ8(DSP1Read8b);
REGABI_BANK_WRITE8(DSP1Write8b);
REGABI_BANK_READ16(DSP1Read16b);
REGABI_BANK_WRITE16(DSP1Write16b);
/* The memaccessbank* handlers pick a ROM map entry by bank, so give every bank
   a different base - a wrong bank then reads a different byte. The extra slack
   covers the largest base plus a 16-bit read at the top of the window. */
static u1 rom[ROM_SIZE + 0x200];
/* The assembly indexes this with the full ebx, not a masked bank, so the test
   has to be able to drive it past 255 and still stay in bounds. */
#define MMAP_ENTRIES 0x1000
u1* snesmmap[MMAP_ENTRIES];
u1 writeon;

/* S-DD1 state. The decompressor itself is stubbed - what matters here is which
   calls the handler makes and what it leaves in memtabler8, which is how it
   takes itself out of the picture. */
u1 AddrNoIncr, SDD1BankA[4];
u4 Sdd1Mode, Sdd1Bank, Sdd1Addr, Sdd1NewAddr;
u1* romdata;
void (*memtabler8[256])();
static u4 sdd1_init_hits, sdd1_init_arg, sdd1_byte_hits;

/* The plain accessor the handler puts back in the table. Only its address is
   ever used, and both sides have to store the same one, so it is a real symbol
   rather than the asm or C body. */
void memaccessbankr8(void) { }

void SDD1_init(u1* in)
{
    sdd1_init_hits++;
    sdd1_init_arg = (u4)(uintptr_t)in;
}

u1 SDD1_get_byte(void)
{
    sdd1_byte_hits++;
    return (u1)(0xA5u + sdd1_byte_hits);
}

static u1 wram_init[65536], eram_init[65536];
static u1 rom_init[ROM_SIZE + 0x200];

/* The seam block (normally cpu/c_memops.c). */
u4 MemSeamB, MemSeamC, MemSeamA, MemSeamD;

/* The I/O register tables, indexed regptra[addr - 0x2000] as in ui.h, and the
   open-bus latch every register read updates. */
void (*regptra[0x3000])(void), (*regptwa[0x3000])(void);
u1 cpu_mdr;

/* What the stub handlers saw: the last four calls, since a 16-bit access makes
   two. */
u4 StubRegAddr[4], StubRegVal[4], StubRegHits;
/* edx as the handler saw it: the assembly passes the core's register through
   implicitly, so a port that does not carry it diverges only here. */
u4 StubRegEdx[4];
/* Drives the stub's nested-access model; see mkmemops.sh. */
u4 StubReenter;
extern void regstub_r(void), regstub_w(void); /* _memops.o */

void asm_memcall(void* fn); /* _memops.o */

#include "../cpu/mem_ops.h"

extern void asm_membank0r8ram(void), asm_membank0r8inv(void);
extern void asm_membank0r8rom(void), asm_membank0r8romram(void);
extern void asm_membank0r16ram(void), asm_membank0r16ramh(void);
extern void asm_membank0r16rom(void), asm_membank0r16romram(void);
extern void asm_membank0w8ram(void), asm_membank0w8inv(void);
extern void asm_membank0w8rom(void), asm_membank0w8romram(void);
extern void asm_membank0w16ram(void), asm_membank0w16ramh(void);
extern void asm_membank0w16inv(void), asm_membank0w16romram(void);
extern void asm_membank0r8reg(void), asm_membank0r16reg(void);
extern void asm_membank0w8reg(void), asm_membank0w16reg(void);
extern void asm_memaccessbankr8(void), asm_memaccessbankr16(void);
extern void asm_memaccessbankw8(void), asm_memaccessbankw16(void);
extern void asm_wramaccessbankr8(void), asm_wramaccessbankr16(void);
extern void asm_wramaccessbankw8(void), asm_wramaccessbankw16(void);
extern void asm_eramaccessbankr8(void), asm_eramaccessbankr16(void);
extern void asm_eramaccessbankw8(void), asm_eramaccessbankw16(void);
extern void asm_sramaccessbankr8(void), asm_sramaccessbankr16(void);
extern void asm_sramaccessbankw8(void), asm_sramaccessbankw16(void);
extern void asm_sramaccessbankr8b(void), asm_sramaccessbankr16b(void);
extern void asm_sramaccessbankw8b(void), asm_sramaccessbankw16b(void);
extern void asm_sramaccessbankr8s(void), asm_sramaccessbankr16s(void);
extern void asm_sramaccessbankw8s(void), asm_sramaccessbankw16s(void);
extern void asm_stsramr8(void), asm_stsramr16(void), asm_stsramw8(void);
extern void asm_stsramw16(void), asm_stsramr8b(void), asm_stsramr16b(void);
extern void asm_stsramw8b(void), asm_stsramw16b(void);
extern void asm_regaccessbankr8(void), asm_regaccessbankw8(void);
extern void asm_regaccessbankr16(void), asm_regaccessbankw16(void);
extern void asm_membank0r8(void), asm_membank0r16(void);
extern void asm_membank0w8(void), asm_membank0w16(void);
extern void asm_membank0r8ramSA1(void), asm_membank0r16ramSA1(void);
extern void asm_membank0w8ramSA1(void), asm_membank0w16ramSA1(void);
extern void asm_SA1RAMaccessbankr8(void), asm_SA1RAMaccessbankr16(void);
extern void asm_SA1RAMaccessbankw8(void), asm_SA1RAMaccessbankw16(void);
extern void asm_regaccessbankr8SA1(void), asm_regaccessbankw8SA1(void);
extern void asm_regaccessbankr16SA1(void), asm_regaccessbankw16SA1(void);
extern void asm_membank0r8SA1(void), asm_membank0r16SA1(void);
extern void asm_membank0w8SA1(void), asm_membank0w16SA1(void);
extern void asm_membank0r8chip(void), asm_membank0r16chip(void);
extern void asm_membank0w8chip(void), asm_membank0w16chip(void);
extern void asm_membank0r16inv(void), asm_membank0w16rom(void);
extern void asm_SA1RAMaccessbankr8b(void), asm_SA1RAMaccessbankr16b(void);
extern void asm_SA1RAMaccessbankw8b(void), asm_SA1RAMaccessbankw16b(void);
extern void asm_memaccessbankr8sdd1(void);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* Which window the handler addresses, so the setup can keep it in range:
       0 = WRAM via ebx+ecx, 1 = WRAM via ecx after `add cx,bx`, 2 = the ROM
       map, 3 = the 1F00-1FFF page, 4 = neither, 5 = an I/O register,
       6 = a whole bank of the ROM map, picked by ebx, 7 = a flat 64K RAM
       bank addressed by ecx alone, 8 = cartridge SRAM, 9 = an ST SRAM bank
       with ROM in its low half, 10 = a mixed ROM/WRAM/IO/cartridge bank,
       11 = the same through the general dispatchers, which mask the address
       to 16 bits and take ROM from map entry 0, 12 = the SA-1 low window,
       WRAM or IRAM depending on who has the bus, 13 = the SA-1's own RAM,
       14 = an SA-1 bank: ROM, IRAM, I/O or BW-RAM byte/bit-map. */
    int win;
} memcase;

static memcase const cases[] = {
    { "membank0r8ram", asm_membank0r8ram, c_membank0r8ram, 0 },
    { "membank0r8inv", asm_membank0r8inv, c_membank0r8inv, 4 },
    { "membank0r8rom", asm_membank0r8rom, c_membank0r8rom, 2 },
    { "membank0r8romram", asm_membank0r8romram, c_membank0r8romram, 1 },
    { "membank0r16ram", asm_membank0r16ram, c_membank0r16ram, 0 },
    { "membank0r16ramh", asm_membank0r16ramh, c_membank0r16ramh, 3 },
    { "membank0r16rom", asm_membank0r16rom, c_membank0r16rom, 2 },
    { "membank0r16romram", asm_membank0r16romram, c_membank0r16romram, 1 },
    { "membank0w8ram", asm_membank0w8ram, c_membank0w8ram, 0 },
    { "membank0w8inv", asm_membank0w8inv, c_membank0w8inv, 4 },
    { "membank0w8rom", asm_membank0w8rom, c_membank0w8rom, 4 },
    { "membank0w8romram", asm_membank0w8romram, c_membank0w8romram, 1 },
    { "membank0w16ram", asm_membank0w16ram, c_membank0w16ram, 0 },
    { "membank0w16ramh", asm_membank0w16ramh, c_membank0w16ramh, 3 },
    { "membank0w16inv", asm_membank0w16inv, c_membank0w16inv, 4 },
    { "membank0w16romram", asm_membank0w16romram, c_membank0w16romram, 1 },
    { "membank0r8reg", asm_membank0r8reg, c_membank0r8reg, 5 },
    { "membank0r16reg", asm_membank0r16reg, c_membank0r16reg, 5 },
    { "membank0w8reg", asm_membank0w8reg, c_membank0w8reg, 5 },
    { "membank0w16reg", asm_membank0w16reg, c_membank0w16reg, 5 },
    { "memaccessbankr8", asm_memaccessbankr8, c_memaccessbankr8, 6 },
    { "memaccessbankr16", asm_memaccessbankr16, c_memaccessbankr16, 6 },
    { "memaccessbankw8", asm_memaccessbankw8, c_memaccessbankw8, 6 },
    { "memaccessbankw16", asm_memaccessbankw16, c_memaccessbankw16, 6 },
    { "wramaccessbankr8", asm_wramaccessbankr8, c_wramaccessbankr8, 7 },
    { "wramaccessbankr16", asm_wramaccessbankr16, c_wramaccessbankr16, 7 },
    { "wramaccessbankw8", asm_wramaccessbankw8, c_wramaccessbankw8, 7 },
    { "wramaccessbankw16", asm_wramaccessbankw16, c_wramaccessbankw16, 7 },
    { "eramaccessbankr8", asm_eramaccessbankr8, c_eramaccessbankr8, 7 },
    { "eramaccessbankr16", asm_eramaccessbankr16, c_eramaccessbankr16, 7 },
    { "eramaccessbankw8", asm_eramaccessbankw8, c_eramaccessbankw8, 7 },
    { "eramaccessbankw16", asm_eramaccessbankw16, c_eramaccessbankw16, 7 },
    { "sramaccessbankr8", asm_sramaccessbankr8, c_sramaccessbankr8, 17 },
    { "sramaccessbankr16", asm_sramaccessbankr16, c_sramaccessbankr16, 17 },
    { "sramaccessbankw8", asm_sramaccessbankw8, c_sramaccessbankw8, 17 },
    { "sramaccessbankw16", asm_sramaccessbankw16, c_sramaccessbankw16, 17 },
    { "sramaccessbankr8b", asm_sramaccessbankr8b, c_sramaccessbankr8b, 8 },
    { "sramaccessbankr16b", asm_sramaccessbankr16b, c_sramaccessbankr16b, 8 },
    { "sramaccessbankw8b", asm_sramaccessbankw8b, c_sramaccessbankw8b, 8 },
    { "sramaccessbankw16b", asm_sramaccessbankw16b, c_sramaccessbankw16b, 8 },
    { "sramaccessbankr8s", asm_sramaccessbankr8s, c_sramaccessbankr8s, 8 },
    { "sramaccessbankr16s", asm_sramaccessbankr16s, c_sramaccessbankr16s, 8 },
    { "sramaccessbankw8s", asm_sramaccessbankw8s, c_sramaccessbankw8s, 8 },
    { "sramaccessbankw16s", asm_sramaccessbankw16s, c_sramaccessbankw16s, 8 },
    { "stsramr8", asm_stsramr8, c_stsramr8, 9 },
    { "stsramr16", asm_stsramr16, c_stsramr16, 9 },
    { "stsramw8", asm_stsramw8, c_stsramw8, 9 },
    { "stsramw16", asm_stsramw16, c_stsramw16, 9 },
    { "stsramr8b", asm_stsramr8b, c_stsramr8b, 9 },
    { "stsramr16b", asm_stsramr16b, c_stsramr16b, 9 },
    { "stsramw8b", asm_stsramw8b, c_stsramw8b, 9 },
    { "stsramw16b", asm_stsramw16b, c_stsramw16b, 9 },
    { "regaccessbankr8", asm_regaccessbankr8, c_regaccessbankr8, 10 },
    { "regaccessbankw8", asm_regaccessbankw8, c_regaccessbankw8, 10 },
    { "regaccessbankr16", asm_regaccessbankr16, c_regaccessbankr16, 10 },
    { "regaccessbankw16", asm_regaccessbankw16, c_regaccessbankw16, 10 },
    { "membank0r8", asm_membank0r8, c_membank0r8, 11 },
    { "membank0r16", asm_membank0r16, c_membank0r16, 11 },
    { "membank0w8", asm_membank0w8, c_membank0w8, 11 },
    { "membank0w16", asm_membank0w16, c_membank0w16, 11 },
    { "membank0r8ramSA1", asm_membank0r8ramSA1, c_membank0r8ramSA1, 12 },
    { "membank0r16ramSA1", asm_membank0r16ramSA1, c_membank0r16ramSA1, 12 },
    { "membank0w8ramSA1", asm_membank0w8ramSA1, c_membank0w8ramSA1, 12 },
    { "membank0w16ramSA1", asm_membank0w16ramSA1, c_membank0w16ramSA1, 12 },
    { "SA1RAMaccessbankr8", asm_SA1RAMaccessbankr8, c_SA1RAMaccessbankr8, 13 },
    { "SA1RAMaccessbankr16", asm_SA1RAMaccessbankr16, c_SA1RAMaccessbankr16, 13 },
    { "SA1RAMaccessbankw8", asm_SA1RAMaccessbankw8, c_SA1RAMaccessbankw8, 13 },
    { "SA1RAMaccessbankw16", asm_SA1RAMaccessbankw16, c_SA1RAMaccessbankw16, 13 },
    { "regaccessbankr8SA1", asm_regaccessbankr8SA1, c_regaccessbankr8SA1, 14 },
    { "regaccessbankw8SA1", asm_regaccessbankw8SA1, c_regaccessbankw8SA1, 14 },
    { "regaccessbankr16SA1", asm_regaccessbankr16SA1, c_regaccessbankr16SA1, 14 },
    { "regaccessbankw16SA1", asm_regaccessbankw16SA1, c_regaccessbankw16SA1, 14 },
    { "membank0r8SA1", asm_membank0r8SA1, c_membank0r8SA1, 14 },
    { "membank0r16SA1", asm_membank0r16SA1, c_membank0r16SA1, 14 },
    { "membank0w8SA1", asm_membank0w8SA1, c_membank0w8SA1, 14 },
    { "membank0w16SA1", asm_membank0w16SA1, c_membank0w16SA1, 14 },
    { "membank0r8chip", asm_membank0r8chip, c_membank0r8chip, 15 },
    { "membank0r16chip", asm_membank0r16chip, c_membank0r16chip, 15 },
    { "membank0w8chip", asm_membank0w8chip, c_membank0w8chip, 15 },
    { "membank0w16chip", asm_membank0w16chip, c_membank0w16chip, 15 },
    { "membank0w16rom", asm_membank0w16rom, c_membank0w16rom, 15 },
    { "membank0r16inv", asm_membank0r16inv, c_membank0r16inv, 4 },
    { "SA1RAMaccessbankr8b", asm_SA1RAMaccessbankr8b, c_SA1RAMaccessbankr8b, 16 },
    { "SA1RAMaccessbankr16b", asm_SA1RAMaccessbankr16b, c_SA1RAMaccessbankr16b, 16 },
    { "SA1RAMaccessbankw8b", asm_SA1RAMaccessbankw8b, c_SA1RAMaccessbankw8b, 16 },
    { "SA1RAMaccessbankw16b", asm_SA1RAMaccessbankw16b, c_SA1RAMaccessbankw16b, 16 },
    { "memaccessbankr8sdd1", asm_memaccessbankr8sdd1, c_memaccessbankr8sdd1, 18 },
};

typedef struct {
    u4 b, c, a, d, writeon, ramsize, ramsizeand, sfxen, dsp1, reenter, sa1st;
    u4 curromspace;
    u4 ccdma, ccaddr, bwshift, bwover;
    u4 noincr, sdd1mode, sdd1bank, sdd1addr, banka;
} setup;

typedef struct {
    u4 b, c, a;
    u4 reghits, regaddr[4], regval[4], regedx[4];
    u4 d;
    u1 mdr;
    u1 wram[65536];
    u1 eram[65536];
    u1 sram[SRAM_SIZE + 0x20000];
    u1 sfxram[SFXRAM_SIZE];
    u1 iram[IRAM_SIZE];
    u1 sa1ram[SA1RAM_SIZE];
    u1 bwram[BWRAM_SIZE];
    u4 cc1hits, cc1last, ccaddr;
    u1 ccvalue;
    u4 dsp1addr, dsp1val, dsp1hits;
    u4 sramb4save;
    u4 sdd1mode, sdd1bank, sdd1addr, sdd1newaddr;
    u4 sdd1inithits, sdd1initarg, sdd1bytehits;
    void (*memtab[256])();
    u1 rom[ROM_SIZE + 0x200];
} snapshot;

static void run(void (*fn)(void), setup const* in, int asm_side, snapshot* out)
{
    memcpy(wramdataa, wram_init, sizeof wramdataa);
    memcpy(ram7fa, eram_init, sizeof ram7fa);
    memcpy(sram_store, sram_init, sizeof sram_store);
    memcpy(sfxram_store, sfxram_init, sizeof sfxram_store);
    memcpy(IRAM, iram_init, sizeof IRAM);
    memcpy(sa1ram_store, sa1ram_init, sizeof sa1ram_store);
    memcpy(bwram_store, bwram_init, sizeof bwram_store);
    BWShift = in->bwshift;
    SA1Overflow = (u2)in->bwover;
    SA1_in_cc1_dma = in->ccdma;
    SA1_DMA_ADDR = in->ccaddr;
    SA1_DMA_VALUE = 0;
    cc1_hits = cc1_last = 0;
    SA1Status = (u1)in->sa1st;
    SFXEnable = (u1)in->sfxen;
    DSP1Type = (u1)in->dsp1;
    dsp1_last_addr = dsp1_last_val = dsp1_hits = 0;
    ramsize = in->ramsize;
    curromspace = in->curromspace;
    ramsizeand = in->ramsizeand;
    sramb4save = 0;
    memcpy(rom, rom_init, sizeof rom);
    for (int i = 0; i < MMAP_ENTRIES; i++) {
        snesmmap[i] = rom + (i & 0x1FF);
    }
    writeon = in->writeon;
    AddrNoIncr = (u1)in->noincr;
    Sdd1Mode = in->sdd1mode;
    Sdd1Bank = in->sdd1bank;
    Sdd1Addr = in->sdd1addr;
    Sdd1NewAddr = 0;
    for (int i = 0; i < 4; i++) {
        SDD1BankA[i] = (u1)(in->banka >> (i * 8));
    }
    /* Distinct sentinels, so a loop that runs over the wrong 64 entries
       shows up rather than overwriting a value that was equal anyway. */
    for (int i = 0; i < 256; i++) {
        memtabler8[i] = (void (*)())(uintptr_t)(0x100000u + (u4)i * 4);
    }
    sdd1_init_hits = sdd1_init_arg = sdd1_byte_hits = 0;
    MemSeamB = in->b;
    MemSeamC = in->c;
    MemSeamA = in->a;
    MemSeamD = in->d;
    StubRegHits = 0;
    StubReenter = in->reenter;
    memset(StubRegAddr, 0, sizeof StubRegAddr);
    memset(StubRegVal, 0, sizeof StubRegVal);
    memset(StubRegEdx, 0, sizeof StubRegEdx);
    cpu_mdr = 0;

    if (asm_side) {
        asm_memcall((void*)fn);
    } else {
        fn();
    }

    /* The ROM base is a run-time address, so report it relative. */
    out->b = MemSeamB;
    out->c = MemSeamC;
    out->a = MemSeamA;
    out->reghits = StubRegHits;
    out->d = MemSeamD;
    memcpy(out->regaddr, StubRegAddr, sizeof out->regaddr);
    memcpy(out->regval, StubRegVal, sizeof out->regval);
    memcpy(out->regedx, StubRegEdx, sizeof out->regedx);
    out->mdr = cpu_mdr;
    memcpy(out->wram, wramdataa, sizeof out->wram);
    memcpy(out->eram, ram7fa, sizeof out->eram);
    memcpy(out->sram, sram_store, sizeof out->sram);
    memcpy(out->sfxram, sfxram_store, sizeof out->sfxram);
    memcpy(out->iram, IRAM, sizeof out->iram);
    memcpy(out->sa1ram, sa1ram_store, sizeof out->sa1ram);
    memcpy(out->bwram, bwram_store, sizeof out->bwram);
    out->cc1hits = cc1_hits;
    out->cc1last = cc1_last;
    out->ccaddr = SA1_DMA_ADDR;
    out->ccvalue = SA1_DMA_VALUE;
    out->dsp1addr = dsp1_last_addr;
    out->dsp1val = dsp1_last_val;
    out->dsp1hits = dsp1_hits;
    out->sramb4save = sramb4save;
    out->sdd1mode = Sdd1Mode;
    out->sdd1bank = Sdd1Bank;
    out->sdd1addr = Sdd1Addr;
    out->sdd1newaddr = Sdd1NewAddr;
    out->sdd1inithits = sdd1_init_hits;
    out->sdd1initarg = sdd1_init_arg;
    out->sdd1bytehits = sdd1_byte_hits;
    memcpy(out->memtab, memtabler8, sizeof out->memtab);
    memcpy(out->rom, rom, sizeof out->rom);
}

int main(void)
{
    /* Filled once: the contents only have to be unpredictable, so reading the
       wrong address reads a different byte. Refilling them every iteration
       costs far more than it buys. */
    dt_fill(wram_init, sizeof wram_init);
    dt_fill(rom_init, sizeof rom_init);
    dt_fill(eram_init, sizeof eram_init);
    dt_fill(sram_init, sizeof sram_init);
    dt_fill(sfxram_init, sizeof sfxram_init);
    dt_fill(iram_init, sizeof iram_init);
    dt_fill(sa1ram_init, sizeof sa1ram_init);
    dt_fill(bwram_init, sizeof bwram_init);
    for (int i = 0; i < 0x3000; i++) {
        regptra[i] = regstub_r;
        regptwa[i] = regstub_w;
    }

    DT_MAIN(20260729, 200000)
    {
        memcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        setup in;
        snapshot a, c;

        /* Keep the upper halves live: the handlers only touch al/ax and the
           low word of ecx, so a wrong-width write shows up here. */
        in.a = dt_u32();
        in.d = dt_u32();
        in.b = dt_mod(256);
        in.c = dt_u32();
        /* ROM writes are gated on this, so hit both settings. */
        in.writeon = dt_mod(2);
        /* initc.c derives the size from the ROM header as (8 << n) Kb, caps it
           at 1024, converts to bytes and sets ramsizeand = ramsize - 1. A cart
           with no SRAM therefore leaves the mask at 0xFFFFFFFF, not zero - the
           handlers that care guard on ramsize instead. */
        in.ramsize = dt_mod(8) == 0 ? 0 : (1024u << dt_mod(8));
        if (in.ramsize > SRAM_SIZE) {
            in.ramsize = SRAM_SIZE;
        }
        in.ramsizeand = in.ramsize - 1;
        /* The cartridge window is SuperFX RAM, DSP1 or HiROM SRAM by turns. */
        in.sfxen = dt_mod(2);
        in.dsp1 = dt_mod(2) ? 2 : dt_mod(3);
        in.reenter = dt_mod(2);
        /* Sits either side of the 2Mb threshold the 70-7D banks check. */
        in.curromspace = dt_mod(2) ? 0x200000u + dt_mod(2) : dt_mod(0x400001);
        /* The S-DD1 stream only continues while the DMA holds its address
           still and the bank and address still match the open stream. */
        in.noincr = dt_mod(4) != 0;
        in.sdd1mode = dt_mod(3);
        in.banka = dt_u32();
        /* 0 = the 65816 owns the low window, 1/2 = the SA-1 does. */
        in.sa1st = dt_mod(3);
        SA1Enable = (u1)dt_mod(2);
        /* Keep the upper half of the DMA address live: it advances as a word. */
        /* The asm tests only BWShift's low byte, and bit 15 of SA1Overflow
           picks the 2-bit map over the 4-bit one. */
        in.bwshift = dt_mod(2) ? (dt_u32() & ~0xFFu) : dt_u32();
        in.bwover = dt_mod(2) ? dt_u32() | 0x8000u : dt_u32() & ~0x8000u;
        in.ccdma = dt_mod(2);
        in.ccaddr = dt_mod(2) ? (dt_u32() & ~0xFFFFu) | 0xFFFFu : dt_u32();

        switch (k->win) {
        case 0:
            /* ebx + ecx addresses WRAM directly, and a 16-bit read needs one
               byte of headroom. */
            in.c = dt_mod(65536 - 256 - 1);
            break;
        case 1:
            /* `add cx,bx` first, then bit 15 picks WRAM or the ROM map. Both
               windows are addressed off the full ecx, so keep it 16-bit as the
               caller (mov ecx,[xd]) always leaves it. Half the time sit just
               under 0xFFFF so the add wraps inside cx: a 32-bit add lands
               somewhere else entirely, and nothing else here would notice. */
            in.c = dt_mod(2) ? 0xFF00u + dt_mod(0x100)
                             : dt_mod(65536 - 256 - 1);
            break;
        case 2:
            /* ebx is folded into the ROM base, ecx is the offset. */
            in.c = dt_mod(ROM_SIZE - 256 - 1);
            break;
        case 3:
            /* Bias hard at the 1FFF boundary, which is the whole point of the
               ramh variants. */
            in.c = dt_mod(2) ? 0x1FFFu - dt_mod(2) - in.b : dt_mod(0x2000);
            break;
        case 5:
            /* `add ecx,ebx` is a full 32-bit add here, and the 16-bit variants
               go on to touch ecx+1, so leave room below 48FF. */
            in.c = 0x2000u + dt_mod(0x28FEu - 256u);
            break;
        case 6:
            in.c = dt_mod(ROM_SIZE - 2);
            break;
        case 7:
            in.c = dt_mod(65535);
            break;
        case 10:
            /* Aim at each window in turn: ROM, the WRAM mirror, I/O, open bus
               and the 6000-7FFF cartridge area. Half the draws land exactly on
               a window edge - every boundary here is an inclusive/exclusive
               decision, and uniform addresses hit each edge about once in ten
               thousand, which is not often enough to pin one down. */
            if (dt_mod(2)) {
                static u4 const edges[] = { 0x0000, 0x1FFF, 0x2000, 0x48FE,
                    0x48FF, 0x4900, 0x5FFF, 0x6000, 0x7FFE, 0x7FFF, 0x8000,
                    0xFFFE, 0xFFFF };

                in.c = edges[dt_mod(sizeof edges / sizeof *edges)];
            } else {
                switch (dt_mod(5)) {
                case 0:
                    in.c = 0x8000u | dt_mod(0x8000);
                    break;
                case 1:
                    in.c = dt_mod(0x2000);
                    break;
                case 2:
                    in.c = 0x2000u + dt_mod(0x2900);
                    break;
                case 3:
                    in.c = 0x4900u + dt_mod(0x1700);
                    break;
                default:
                    in.c = 0x6000u + dt_mod(0x2000);
                    break;
                }
            }
            /* Same for the bank: the cartridge path splits at 10h and 30h and
               masks with 7Fh, so sit on those edges deliberately. */
            if (dt_mod(2)) {
                static u4 const banks[] = { 0x00, 0x0F, 0x10, 0x11, 0x2F, 0x30,
                    0x31, 0x7E, 0x7F, 0x80, 0x8F, 0x90, 0xB0, 0xFF };

                in.b = banks[dt_mod(sizeof banks / sizeof *banks)];
            } else if (dt_mod(4) == 0) {
                /* Past a byte: the ROM-map index is not masked in the asm. */
                in.b = dt_mod(MMAP_ENTRIES);
            } else {
                in.b = dt_mod(256);
            }
            /* Leave ramsize alone, zero included: a cart with no SRAM still
               reaches the 6000-7FFF path, and that is most carts. */
            break;
        case 14:
            /* Same window edges as case 10, plus the 800h IRAM bound. */
            if (dt_mod(2)) {
                static u4 const edges[] = { 0x0000, 0x07FF, 0x0800, 0x1FFF,
                    0x2000, 0x48FF, 0x4900, 0x5FFF, 0x6000, 0x7FFF, 0x8000,
                    0xFFFF };

                in.c = edges[dt_mod(sizeof edges / sizeof *edges)];
            } else {
                in.c = dt_mod(0x10000);
            }
            in.b = dt_mod(256);
            break;
        case 13:
            /* ebx picks one of four 64K slices; ecx addresses inside it. The
               DMA path overwrites the low half of SA1_DMA_ADDR with ecx before
               advancing it, so the word wrap is reachable only from here. */
            in.b = dt_mod(2) ? dt_mod(4) : dt_mod(256);
            in.c = dt_mod(2) ? 0xFFFEu + dt_mod(2) : dt_mod(0x10000);
            break;
        case 18:
            /* Banks C0-FF, one logical 1Mb bank per group of 16; below C0
               there is no mapping at all, which the bank log reports as 0Fh.
               Half the streams are already open on this exact address, so
               both the decompress and the give-up paths get exercised. */
            in.b = dt_mod(2) ? 0xC0u + dt_mod(0x40)
                             : (u4)(u1[]) { 0xBF, 0xC0, 0xCF, 0xD0, 0xDF,
                                   0xE0, 0xEF, 0xF0, 0xFF }[dt_mod(9)];
            /* The bank log reads bl, not ebx, so drive the index past a
               byte - the ROM map is wide enough to take it. */
            if (dt_mod(4) == 0) {
                in.b |= dt_mod(0x10) << 8;
            }
            in.c = dt_mod(0x10000);
            /* The SDD1_init pointer masks the address to 16 bits. That is
               only visible while opening a stream, and that path never
               reads ROM, so a dirty high half stays in bounds there. */
            if (in.noincr != 0 && in.sdd1mode != 2 && dt_mod(2)) {
                in.c |= dt_u32() & 0xFFFF0000u;
            }
            in.sdd1bank = dt_mod(2) ? in.b : dt_u32();
            in.sdd1addr = dt_mod(2) ? in.c : dt_u32();
            break;
        case 17:
            /* Bit 15 picks the half of the bank, and on a large cart the top
               half is ROM instead of the SRAM mirror - drive both sides of
               each threshold. The bank is masked to 7Fh and rebased at 70h,
               so straddle that too. */
            in.c = dt_mod(2) ? 0x8000u | dt_mod(0x8000) : dt_mod(0x8000);
            in.b = dt_mod(2) ? 0x70u + dt_mod(0x10) : dt_mod(256);
            if (dt_mod(2)) {
                in.ramsize = 0x8000u + dt_mod(2);
                in.ramsizeand = in.ramsize - 1;
            }
            break;
        case 16:
            /* A pixel index, not an address: ebx picks a 32K slice (16K in
               the 2-bit mode, where the bank field is a bit wider) and the
               low bits of ecx pick the field inside the byte, so every
               alignment matters. Bias the top of the window too - the 16-bit
               forms read or write the next byte from there. */
            in.b = dt_mod(256);
            in.c = dt_mod(2) ? (0xFFFCu | dt_mod(4)) : dt_mod(0x10000);
            break;
        case 15:
            /* The cartridge window. Both the BW-RAM byte view and the bit map
               index off the address itself, so keep it inside 6000-FFFF as
               the emulator's tables do - below that the bit-map offset goes
               negative and lands outside the buffer. The edges are where the
               8K SuperFX mirror wraps; ebx is added with a full 32-bit add,
               so it can carry the address past FFFFh. */
            in.b = dt_mod(256);
            if (dt_mod(2)) {
                static u4 const edges[] = { 0x6000, 0x6001, 0x7FFE, 0x7FFF,
                    0x8000, 0x8001, 0x9FFF, 0xA000, 0xFFFE, 0xFFFF };

                in.c = edges[dt_mod(sizeof edges / sizeof *edges)];
            } else {
                in.c = 0x6000u + dt_mod(0xA000);
            }
            break;
        case 12:
            /* ebx + ecx must stay inside the 2K window plus its slack, and the
               800h edge is the whole point of the bounds check. */
            in.b = dt_mod(0x100);
            in.c = dt_mod(2) ? 0x7FEu + dt_mod(4) : dt_mod(0x1000);
            break;
        case 11:
            /* Same windows as case 10, but these mask the address to 16 bits
               first, so drive the high half dirty - nothing else here would
               notice the mask going missing. */
            in.c = (dt_mod(2) ? dt_u32() & 0xFFFF0000u : 0u)
                | (dt_mod(2) ? dt_mod(0x10000)
                             : (0x6000u + dt_mod(0x2000)));
            in.b = dt_mod(256);
            break;
        case 9:
            /* Bit 15 picks ROM or SRAM, so hit both; on the ROM side ebx is a
               bank number into the map, on the SRAM side a 60h/70h slice.
               These handlers have no ramsize == 0 guard, and the emulator only
               installs them for carts that have SRAM, so keep one here. */
            in.c = dt_mod(2) ? 0x8000u | dt_mod(0x8000) : dt_mod(0x8000);
            in.b = dt_mod(2) ? 0x60u + dt_mod(0x20) : dt_mod(256);
            if (in.ramsize == 0) {
                in.ramsize = 1024u << dt_mod(8);
                if (in.ramsize > SRAM_SIZE) {
                    in.ramsize = SRAM_SIZE;
                }
                in.ramsizeand = in.ramsize - 1;
            }
            break;
        case 8:
            /* Every access is masked with ramsizeand, so any address and any
               bank stay in range. Two things need deliberate coverage: the
               16-bit wrap, which only shows when the masked address is already
               at the top of the cart, and `sub bl,78h` borrowing, which only
               shows for a bank below 78h. */
            in.c = dt_mod(2) ? in.ramsizeand : dt_u32();
            in.b = dt_mod(2) ? 0x78u + dt_mod(8) : dt_mod(256);
            break;
        default:
            in.c = dt_u32();
            break;
        }

        run(k->asm_fn, &in, 1, &a);
        run(k->c_fn, &in, 0, &c);

        DT_EQ(k->name, a.a, c.a);
        DT_EQ("ebx", a.b, c.b);
        DT_EQ("ecx", a.c, c.c);
        DT_MEM("wramdataa", a.wram, c.wram, sizeof a.wram);
        DT_MEM("ram7fa", a.eram, c.eram, sizeof a.eram);
        DT_MEM("cartridge SRAM", a.sram, c.sram, sizeof a.sram);
        DT_MEM("SuperFX RAM", a.sfxram, c.sfxram, sizeof a.sfxram);
        DT_MEM("SA-1 IRAM", a.iram, c.iram, sizeof a.iram);
        DT_MEM("SA-1 RAM", a.sa1ram, c.sa1ram, sizeof a.sa1ram);
        DT_MEM("SA-1 BW-RAM", a.bwram, c.bwram, sizeof a.bwram);
        DT_EQ("CC1 DMA calls", a.cc1hits, c.cc1hits);
        DT_EQ("CC1 DMA address seen", a.cc1last, c.cc1last);
        DT_EQ("SA1_DMA_ADDR", a.ccaddr, c.ccaddr);
        DT_EQ("SA1_DMA_VALUE", a.ccvalue, c.ccvalue);
        DT_EQ("DSP1 calls", a.dsp1hits, c.dsp1hits);
        DT_EQ("DSP1 address", a.dsp1addr, c.dsp1addr);
        DT_EQ("DSP1 value", a.dsp1val, c.dsp1val);
        DT_EQ("sramb4save", a.sramb4save, c.sramb4save);
        DT_EQ("Sdd1Mode", a.sdd1mode, c.sdd1mode);
        DT_EQ("Sdd1Bank", a.sdd1bank, c.sdd1bank);
        DT_EQ("Sdd1Addr", a.sdd1addr, c.sdd1addr);
        DT_EQ("Sdd1NewAddr", a.sdd1newaddr, c.sdd1newaddr);
        DT_EQ("SDD1_init calls", a.sdd1inithits, c.sdd1inithits);
        DT_EQ("SDD1_init pointer", a.sdd1initarg, c.sdd1initarg);
        DT_EQ("SDD1_get_byte calls", a.sdd1bytehits, c.sdd1bytehits);
        DT_MEM("memtabler8", a.memtab, c.memtab, sizeof a.memtab);
        DT_MEM("ROM window", a.rom, c.rom, sizeof a.rom);
        DT_EQ("edx", a.d, c.d);
        DT_EQ("register handler calls", a.reghits, c.reghits);
        DT_MEM("register handler address", a.regaddr, c.regaddr, sizeof a.regaddr);
        DT_MEM("register handler eax", a.regval, c.regval, sizeof a.regval);
        DT_MEM("register handler edx", a.regedx, c.regedx, sizeof a.regedx);
        DT_EQ("cpu_mdr", a.mdr, c.mdr);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ case %s b=%x c=%x a=%x\n", k->name, in.b, in.c, in.a);
        }
    }
    DT_DONE("direct-page memory handlers");
}
