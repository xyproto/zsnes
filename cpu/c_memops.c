/* Direct-page memory accessors ported from cpu/memory.asm. The bodies are in
   cpu/mem_ops.h, which the difftest includes too; this file only supplies the
   seam the assembly spills its registers into. */
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
void memaccessbankr8(void); /* cpu/memory.asm: what the table goes back to */

/* The DSP1 entry points already have cdecl halves (chips/dsp1proc.c). */
u1 c_DSP1Read8b(u4 addr);
void c_DSP1Write8b(u4 addr, u1 val);
u2 c_DSP1Read16b(u4 addr);
void c_DSP1Write16b(u4 addr, u2 val);

/* What the assembly keeps in ebx, ecx and eax across one handler call. The
   memcop macro in cpu/memory.asm writes all three on the way in and reads them
   back on the way out, so a handler can change any of them. */
u4 MemSeamB;
u4 MemSeamC;
u4 MemSeamA;
/* edx too: these bodies never read it, but the I/O handlers they call run
   with the core's registers live, and some look at more than ecx and al. */
u4 MemSeamD;

#include "mem_ops.h"

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
