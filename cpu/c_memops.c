/* The memory access handlers ported from cpu/memory.asm. The bodies are in
   cpu/mem_ops.h, which the difftest includes too; this file supplies the seam
   they pass values through and the entry points the memtable holds. */
#include <stdbool.h>
#include <stdint.h>

#include "../asmdata.h"

#include "../chips/regabi.h" /* REGABI_ENTRY/REGABI_SYM for the trampolines */
#include "../types.h"
#include "../chips/sa1proc.h" /* SA1Status */
#include "../chips/sa1regs.h" /* IRAM, SA1RAMArea, SA1_in_cc1_dma */
#include "../ui.h" /* regptra, regptwa, sfxramdata, romdata */

extern u1 wramdataa[65536], ram7fa[65536]; /* ui.c */
extern u1* snesmmap[256]; /* endmem.h */
extern u1 cpu_mdr; /* cpu/regs.inc */
extern u1 writeon; /* cpu/regs.inc: ROM patching enabled */
extern u1 *sram, *sram2; /* ui.h, initc.c */
extern u4 ramsize, ramsizeand; /* initc.c */
extern u4 curromspace; /* initc.c: bytes of ROM the cart actually holds */
extern u4 sramb4save; /* frames left before the SRAM is flushed to disk */
extern u1 DSP1Type; /* initc.c */
extern bool SFXEnable, SA1Enable; /* initc.c */

/* The SA-1 character-conversion DMA (chips/sa1emu.c). */
extern u1 SA1_DMA_VALUE;
extern u4 SA1_DMA_ADDR;
void SA1_DMA_CC1(void);

/* SA-1 BW-RAM: the window pointer, the bit-map source and the mode bits. */
extern u4 BWShift; /* chips/sa1regs.c; the asm tests its low byte only */

/* S-DD1 software decompression (chips/sdd1emu.c, chips/sa1regs.c). */
extern u1 SDD1BankA[4];
extern u1 AddrNoIncr; /* cpu/c_dma.c: the DMA holds its address still */
extern void (*memtabler8[256])(); /* cpu/memtable.h */
extern u4 Sdd1Mode, Sdd1Bank, Sdd1Addr, Sdd1NewAddr;
void SDD1_init(u1* in);
u1 SDD1_get_byte(void);
void memaccessbankr8(void); /* what the S-DD1 table goes back to */

/* The DSP1 entry points already have cdecl halves (chips/dsp1proc.c). */
u1 c_DSP1Read8b(u4 addr);
void c_DSP1Write8b(u4 addr, u1 val);
u2 c_DSP1Read16b(u4 addr);
void c_DSP1Write16b(u4 addr, u2 val);

/* The seam a handler reads its arguments out of and leaves its results in;
   cpu/memseam.h describes the convention. edx is carried too: these bodies
   never read it, but the I/O handlers they call run with the core's registers
   live, and some look at more than ecx and al. */
u4 MemSeamB;
u4 MemSeamC;
u4 MemSeamA;
u4 MemSeamD;
/* esi, for the $2140-$2143 sound-skip hack. */
u4 MemSeamS;

#include "mem_ops.h"


/* The memtable and Bank0dat entry points.  cpu/memory.asm used to own these
   names and thunk into the c_ bodies above; the tables now hold the bodies
   themselves, under the seam convention in cpu/memseam.h.  The c_ prefix
   stays on the bodies because test/difftest_memops.c drives them alongside
   the pre-port assembly oracle, which still exports the public names. */
#define MEM_BANK(name)  \
    void name(void)     \
    {                   \
        c_##name();     \
    }

/* Register & memory access banks (00-3F / 80-BF) */
MEM_BANK(regaccessbankr8)
MEM_BANK(regaccessbankr16)
MEM_BANK(regaccessbankw8)
MEM_BANK(regaccessbankw16)

/* Bank 0 direct page, SA-1 RAM window */
MEM_BANK(membank0r8ramSA1)
MEM_BANK(membank0r16ramSA1)
MEM_BANK(membank0w8ramSA1)
MEM_BANK(membank0w16ramSA1)

/* Bank 0 direct page, 8-bit reads */
MEM_BANK(membank0r8ram)
MEM_BANK(membank0r8reg)
MEM_BANK(membank0r8inv)
MEM_BANK(membank0r8chip)
MEM_BANK(membank0r8rom)
MEM_BANK(membank0r8romram)

/* Bank 0 direct page, 16-bit reads */
MEM_BANK(membank0r16ram)
MEM_BANK(membank0r16ramh)
MEM_BANK(membank0r16reg)
MEM_BANK(membank0r16inv)
MEM_BANK(membank0r16chip)
MEM_BANK(membank0r16rom)
MEM_BANK(membank0r16romram)

/* Bank 0 direct page, 8-bit writes */
MEM_BANK(membank0w8ram)
MEM_BANK(membank0w8reg)
MEM_BANK(membank0w8inv)
MEM_BANK(membank0w8chip)
MEM_BANK(membank0w8rom)
MEM_BANK(membank0w8romram)

/* Bank 0 direct page, 16-bit writes */
MEM_BANK(membank0w16ram)
MEM_BANK(membank0w16ramh)
MEM_BANK(membank0w16reg)
MEM_BANK(membank0w16inv)
MEM_BANK(membank0w16chip)
MEM_BANK(membank0w16rom)
MEM_BANK(membank0w16romram)

/* Bank 0 dispatchers */
MEM_BANK(membank0r8)
MEM_BANK(membank0r16)
MEM_BANK(membank0w8)
MEM_BANK(membank0w16)
MEM_BANK(membank0r8SA1)
MEM_BANK(membank0r16SA1)
MEM_BANK(membank0w8SA1)
MEM_BANK(membank0w16SA1)

/* ROM only access banks (40-6F / C0-FF) */
MEM_BANK(memaccessbankr8)
MEM_BANK(memaccessbankr16)
MEM_BANK(memaccessbankw8)
MEM_BANK(memaccessbankw16)

/* SRAM access bank (70h) */
MEM_BANK(sramaccessbankr8)
MEM_BANK(sramaccessbankr16)
MEM_BANK(sramaccessbankw8)
MEM_BANK(sramaccessbankw16)
MEM_BANK(sramaccessbankr8s)
MEM_BANK(sramaccessbankr16s)
MEM_BANK(sramaccessbankw8s)
MEM_BANK(sramaccessbankw16s)
MEM_BANK(sramaccessbankr8b)
MEM_BANK(sramaccessbankr16b)
MEM_BANK(sramaccessbankw8b)
MEM_BANK(sramaccessbankw16b)

/* Seta SRAM windows */
MEM_BANK(stsramr8)
MEM_BANK(stsramr16)
MEM_BANK(stsramw8)
MEM_BANK(stsramw16)
MEM_BANK(stsramr8b)
MEM_BANK(stsramr16b)
MEM_BANK(stsramw8b)
MEM_BANK(stsramw16b)

/* WorkRAM access bank (7Eh) */
MEM_BANK(wramaccessbankr8)
MEM_BANK(wramaccessbankr16)
MEM_BANK(wramaccessbankw8)
MEM_BANK(wramaccessbankw16)

/* ExpandRAM access bank (7Fh) */
MEM_BANK(eramaccessbankr8)
MEM_BANK(eramaccessbankr16)
MEM_BANK(eramaccessbankw8)
MEM_BANK(eramaccessbankw16)

/* SA-1 bank accesses */
MEM_BANK(regaccessbankr8SA1)
MEM_BANK(regaccessbankr16SA1)
MEM_BANK(regaccessbankw8SA1)
MEM_BANK(regaccessbankw16SA1)
MEM_BANK(SA1RAMaccessbankr8)
MEM_BANK(SA1RAMaccessbankr16)
MEM_BANK(SA1RAMaccessbankw8)
MEM_BANK(SA1RAMaccessbankw16)
MEM_BANK(SA1RAMaccessbankr8b)
MEM_BANK(SA1RAMaccessbankr16b)
MEM_BANK(SA1RAMaccessbankw8b)
MEM_BANK(SA1RAMaccessbankw16b)

/* S-DD1 software decompression */
MEM_BANK(memaccessbankr8sdd1)

/* The last of cpu/memory.asm's own data. BWUsed2 and BWUsed are adjacent
   bytes the SA-1 BW-RAM paths read as a pair, so pin the layout rather than
   letting the compiler choose. */

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    ASM_GSYM(BWUsed2)
    ".skip 1\n"
    ASM_GSYM(BWUsed)
    ".skip 1\n"
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(LatestBank)
    ".long 0xFFFF\n"
    ASM_SEC_END);

/* clang-format on */
