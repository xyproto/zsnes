/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * SPC7110 chip emulator - register handlers and init/reset logic.
 *
 * Ported from chips/7110proc.asm.
 * SPC7110 information fully reverse engineered by Dark Force and John Weidman.
 * Original ZSNES code by zsKnight, Jonas Quinn, and Nach.
 *
 * Register-handler calling convention (x86 / ZSNES ABI):
 *   Read  handlers – leave the result byte in AL on return (REGPTR dispatch).
 *   Write handlers – receive the byte to write in AL on entry (REGPTW dispatch).
 *
 * On x86/GCC we use the   register <type> <name> asm("al")   idiom to capture
 * or return AL without the compiler touching it between the asm constraint and
 * the first use.  All handlers are declared void() to match the eop* typedef.
 *
 * Memory-bank access functions (SPC7110ReadSRAM8b etc.) use standard C
 * calling conventions and typed parameters/return values, matching the mrwp
 * dispatch table (see cpu/memtable.h).
 */

#include <stdint.h>
#include <string.h>

#include "../cfg.h"
#include "../endmem.h"
#include "../types.h"
#include "../ui.h"
#include "../ztimec.h"
#include "7110proc.h"

/* External C-level decompression entry points (defined in 7110emu.c) */
extern void SPC7110initC(void);
extern void SPC7110_4800(void);
extern void SPC7110_4806w(void);

/* External variables required by register handlers and bank-switch logic */
extern u1* romdata;
extern u1* sram;
extern u4  ramsize;
extern u4  ramsizeand;
extern u4  sramb4save;
extern u1  curromsize;
extern u1* snesmmap[256];
extern u1* snesmap2[256];
#ifndef NO_DEBUGGER
extern u1  debuggeron;
#endif

/* ---------------------------------------------------------------------------
 * SPC7110 data registers (SECTION .data NEWSYMs in 7110proc.asm)
 *
 * The assembly laid these out contiguously starting at SPCMultA.  The save-
 * state system (zstate.c) copies the entire block via:
 *   copy_func(&buffer, &SPCMultA, PHnum2writespc7110reg);
 * so these variables MUST remain contiguous and in the same order.
 *
 * We use GCC's section attribute to force them all into a single named ELF
 * section (overriding -fdata-sections, which would otherwise put each symbol
 * in its own section).  GCC guarantees that symbols within the same section
 * are placed in declaration order when they come from a single translation
 * unit, so this preserves the original layout.
 * --------------------------------------------------------------------------- */
#ifdef __GNUC__
#  define SPC7110_SAVEBLOCK __attribute__((section(".data.spc7110_saveblock")))
#else
#  define SPC7110_SAVEBLOCK /* no guarantee of contiguity on non-GCC */
#endif

u4 SPCMultA      SPC7110_SAVEBLOCK = 0;   /* 16-bit multiplicand / 32-bit dividend – low dword */
u4 SPCMultB      SPC7110_SAVEBLOCK = 0;   /* 16-bit multiplier */
u4 SPCDivEnd     SPC7110_SAVEBLOCK = 0;   /* 16-bit divisor */
u4 SPCMulRes     SPC7110_SAVEBLOCK = 0;   /* 32-bit product / quotient */
u4 SPCDivRes     SPC7110_SAVEBLOCK = 0;   /* 16-bit remainder */
u4 SPC7110BankA  SPC7110_SAVEBLOCK = 0x020100; /* current data-ROM bank selection for D0/E0/F0 */
u4 SPC7110RTCStat SPC7110_SAVEBLOCK = 0;  /* RTC state machine: byte[0]=enable, [1]=index, [2]=cmd */
u1 SPC7110RTC[16]  SPC7110_SAVEBLOCK = { 0,0,0,0,0,0,1,0,1,0,0,0,0,0,0x0F,0 };
u1 SPC7110RTCB[16] SPC7110_SAVEBLOCK = { 0,0,0,0,0,0,1,0,1,0,0,0,0,1,0x0F,6 };
u4  SPCROMPtr    SPC7110_SAVEBLOCK = 0;   /* 24-bit data-ROM read pointer */
u4* SPCROMtoI    SPC7110_SAVEBLOCK = &SPCROMPtr; /* pointer to the field that auto-increments */
u4  SPCROMAdj    SPC7110_SAVEBLOCK = 0;   /* 16-bit signed offset used with the pointer */
u4  SPCROMInc    SPC7110_SAVEBLOCK = 0;   /* 16-bit signed increment added after $4810 reads */
u4  SPCROMCom    SPC7110_SAVEBLOCK = 0;   /* command mode: byte[0]=raw register, byte[1]=decoded */
u4  SPCCheckFix  SPC7110_SAVEBLOCK = 0;   /* 0 until $4811 has been written; gate for $4810/$481A */
u4  SPCSignedVal SPC7110_SAVEBLOCK = 0;   /* bit 0: 1 = signed mul/div mode */
u1  SPCCompressionRegs[13] SPC7110_SAVEBLOCK = { 0 }; /* compression-state registers 0x00–0x0C */

/*
 * PHnum2writespc7110reg – byte count of the save-state block.
 * The assembly computed this at assemble time as:  $ - SPCMultA
 *
 * Block layout (same order as assembly, all on 32-bit / NASM ABI):
 *   SPCMultA(4) + SPCMultB(4) + SPCDivEnd(4) + SPCMulRes(4) + SPCDivRes(4)
 *   + SPC7110BankA(4) + SPC7110RTCStat(4) + SPC7110RTC(16) + SPC7110RTCB(16)
 *   + SPCROMPtr(4) + SPCROMtoI(4) + SPCROMAdj(4) + SPCROMInc(4) + SPCROMCom(4)
 *   + SPCCheckFix(4) + SPCSignedVal(4) + SPCCompressionRegs(13)
 *   = 5×4 + 4 + 4 + 16 + 16 + 7×4 + 13 = 101 bytes
 */
u4 PHnum2writespc7110reg = 101;

/* 12-hour BCD conversion table (from SPCTimerVal in 7110proc.asm SECTION .data).
 * Referenced in the commented-out 12-hour clock branch of SPC4841. */
static const u1 SPCTimerVal[34] = {
    0x12,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0,0,0,0,0,0,
    0x10,0x11,0x32,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0,0,0,0,0,0,
    0x28,0x29
};

/* ===========================================================================
 * Helpers for the x86 register-handler ABI
 *
 * On x86/GCC:
 *   REGVAL_READ  – declares a local u1 whose value is *returned* in AL
 *   REGVAL_WRITE – declares a local u1 whose value is *received* from AL
 *
 * These rely on the GCC "register ... asm()" extension which pins the variable
 * to a specific hard register at the point of the constraint.  The assignment
 * (for REGVAL_READ) or the mere declaration (for REGVAL_WRITE) is all that
 * is needed; gcc guarantees AL holds the value at function entry/exit because
 * the asm ABI constraint propagates through the function.
 *
 * On non-x86 / non-GCC these expand to plain u1 with a value of 0 (the whole
 * register-based dispatch will be replaced by C wrappers when memory.asm is
 * ported, at which point these functions will be revisited).
 * =========================================================================== */
#if defined(__GNUC__) && defined(__i386__)
#  define REGVAL_WRITE  register u1 al asm("al")
#  define REGVAL_READ(v) register u1 al asm("al") = (v); (void)al
#else
/* Non-x86 placeholder – memory dispatch not yet ported, so these handlers
 * are unreachable; define stubs to keep the translation unit self-consistent. */
#  define REGVAL_WRITE  u1 al = 0
#  define REGVAL_READ(v) (void)(v)
#endif

/* ===========================================================================
 * SPC7110init – called from initc.c when a SPC7110 game is loaded.
 * =========================================================================== */
void SPC7110init(void)
{
    SPC7110initC();

    SPCMultA      = 0;
    SPCMultB      = 0;
    SPCDivEnd     = 0;
    SPCMulRes     = 0;
    SPCDivRes     = 0;
    SPC7110BankA  = 0x020100;
    SPC7110RTCStat = 0;
    SPCROMPtr     = 0;
    SPCROMtoI     = &SPCROMPtr;
    SPCROMAdj     = 0;
    SPCROMInc     = 0;
    SPCROMCom     = 0;
    SPCCheckFix   = 0;
}

/* ===========================================================================
 * BankSwitchSPC7110 – shared helper for $4831/$4832/$4833 writes.
 *
 * Parameters:
 *   al         – raw value written to the port
 *   byte_index – which byte of SPC7110BankA to update (0,1,2)
 *   map_base   – first SNES bank in snesmmap/snesmap2 to fill (0xD0,0xE0,0xF0)
 *
 * Logic (translated from the BankSwitchSPC7110 NASM macro):
 *   Store al in SPC7110BankA[byte_index].
 *   Increment al; wrap into the valid data-ROM bank range:
 *     40-Mbit (curromsize == 13): keep in [1..4] by subtracting 4
 *     24-Mbit (other)           : keep in [1..2] by subtracting 2
 *   Map 16 consecutive 64 KB pages starting at map_base in both
 *   snesmmap and snesmap2.
 * =========================================================================== */
static void BankSwitchSPC7110(u1 al, int byte_index, int map_base)
{
    ((u1*)&SPC7110BankA)[byte_index] = al;
    al++;

    if (curromsize == 13) {
        /* 40-Mbit ROM: data-ROM banks 1..4 */
        while (al >= 5)
            al -= 4;
    } else {
        /* 24-Mbit ROM: data-ROM banks 1..2 */
        while (al >= 3)
            al -= 2;
    }

    /* Each data-ROM bank is 1 MB; banks start at romdata + 0x100000 */
    u1* base = romdata + ((u4)(al & 0x07) << 20);
    for (int i = 0; i < 16; i++) {
        u1* pg = base + (u4)i * 0x10000;
        snesmap2[map_base + i] = pg;
        snesmmap[map_base + i] = pg;
    }
}

/* ===========================================================================
 * SPC7110Reset – register all SPC7110 write-port handlers.
 * (Assembly: setregw addr,handler → REGPTW(addr) = handler)
 * =========================================================================== */
void SPC7110Reset(void)
{
    REGPTW(0x4801) = SPC4801w;
    REGPTW(0x4802) = SPC4802w;
    REGPTW(0x4803) = SPC4803w;
    REGPTW(0x4804) = SPC4804w;
    REGPTW(0x4805) = SPC4805w;
    REGPTW(0x4806) = SPC4806w;
    REGPTW(0x4807) = SPC4807w;
    REGPTW(0x4808) = SPC4808w;
    REGPTW(0x4809) = SPC4809w;
    REGPTW(0x480A) = SPC480Aw;
    REGPTW(0x480B) = SPC480Bw;

    REGPTW(0x4811) = SPC4811w;
    REGPTW(0x4812) = SPC4812w;
    REGPTW(0x4813) = SPC4813w;
    REGPTW(0x4814) = SPC4814w;
    REGPTW(0x4815) = SPC4815w;
    REGPTW(0x4816) = SPC4816w;
    REGPTW(0x4817) = SPC4817w;
    REGPTW(0x4818) = SPC4818w;

    REGPTW(0x4820) = SPC4820w;
    REGPTW(0x4821) = SPC4821w;
    REGPTW(0x4822) = SPC4822w;
    REGPTW(0x4823) = SPC4823w;
    REGPTW(0x4824) = SPC4824w;
    REGPTW(0x4825) = SPC4825w;
    REGPTW(0x4826) = SPC4826w;
    REGPTW(0x4827) = SPC4827w;
    REGPTW(0x482E) = SPC482Ew;

    REGPTW(0x4831) = SPC4831w;
    REGPTW(0x4832) = SPC4832w;
    REGPTW(0x4833) = SPC4833w;

    REGPTW(0x4840) = SPC4840w;
    REGPTW(0x4841) = SPC4841w;
    REGPTW(0x4842) = SPC4842w;
}

/* ===========================================================================
 * initSPC7110regs – register all SPC7110 read-port handlers.
 * (Assembly: setreg addr,handler → REGPTR(addr) = handler)
 * =========================================================================== */
void initSPC7110regs(void)
{
    REGPTR(0x4800) = SPC4800;
    REGPTR(0x4801) = SPC4801;
    REGPTR(0x4802) = SPC4802;
    REGPTR(0x4803) = SPC4803;
    REGPTR(0x4804) = SPC4804;
    REGPTR(0x4805) = SPC4805;
    REGPTR(0x4806) = SPC4806;
    REGPTR(0x4807) = SPC4807;
    REGPTR(0x4808) = SPC4808;
    REGPTR(0x4809) = SPC4809;
    REGPTR(0x480A) = SPC480A;
    REGPTR(0x480B) = SPC480B;
    REGPTR(0x480C) = SPC480C;

    REGPTR(0x4810) = SPC4810;
    REGPTR(0x4811) = SPC4811;
    REGPTR(0x4812) = SPC4812;
    REGPTR(0x4813) = SPC4813;
    REGPTR(0x4814) = SPC4814;
    REGPTR(0x4815) = SPC4815;
    REGPTR(0x4816) = SPC4816;
    REGPTR(0x4817) = SPC4817;
    REGPTR(0x4818) = SPC4818;
    REGPTR(0x481A) = SPC481A;

    REGPTR(0x4820) = SPC4820;
    REGPTR(0x4821) = SPC4821;
    REGPTR(0x4822) = SPC4822;
    REGPTR(0x4823) = SPC4823;
    REGPTR(0x4824) = SPC4824;
    REGPTR(0x4825) = SPC4825;
    REGPTR(0x4826) = SPC4826;
    REGPTR(0x4827) = SPC4827;
    REGPTR(0x4828) = SPC4828;
    REGPTR(0x4829) = SPC4829;
    REGPTR(0x482A) = SPC482A;
    REGPTR(0x482B) = SPC482B;
    REGPTR(0x482C) = SPC482C;
    REGPTR(0x482D) = SPC482D;
    REGPTR(0x482E) = SPC482E;
    REGPTR(0x482F) = SPC482F;

    REGPTR(0x4831) = SPC4831;
    REGPTR(0x4832) = SPC4832;
    REGPTR(0x4833) = SPC4833;
    REGPTR(0x4834) = SPC4834;

    REGPTR(0x4840) = SPC4840;
    REGPTR(0x4841) = SPC4841;
    REGPTR(0x4842) = SPC4842;

    /* $4850–$485F: direct read of SPC7110RTC[0..15] */
    REGPTR(0x4850) = SPC4850;
    REGPTR(0x4851) = SPC4851;
    REGPTR(0x4852) = SPC4852;
    REGPTR(0x4853) = SPC4853;
    REGPTR(0x4854) = SPC4854;
    REGPTR(0x4855) = SPC4855;
    REGPTR(0x4856) = SPC4856;
    REGPTR(0x4857) = SPC4857;
    REGPTR(0x4858) = SPC4858;
    REGPTR(0x4859) = SPC4859;
    REGPTR(0x485A) = SPC485A;
    REGPTR(0x485B) = SPC485B;
    REGPTR(0x485C) = SPC485C;
    REGPTR(0x485D) = SPC485D;
    REGPTR(0x485E) = SPC485E;
    REGPTR(0x485F) = SPC485F;
}

/* ===========================================================================
 * Register READ handlers
 *
 * All are void() to match eop*.  They leave their result byte in AL on
 * return; the register-read dispatch (regaccessbankr8) reads AL after the
 * call and returns it to the CPU core.
 *
 * REGVAL_READ(expr) expands to   register u1 al asm("al") = (expr); (void)al
 * which forces the compiler to put expr into AL before the function returns.
 * =========================================================================== */

/* -- Compression / decompressed-data read ports ($4800–$480C) -------------- */

/* $4800 – trigger one decompression step; result ends up in SPCCompressionRegs[0] */
void SPC4800(void)
{
    SPC7110_4800();
    REGVAL_READ(SPCCompressionRegs[0]);
}

void SPC4801(void) { REGVAL_READ(SPCCompressionRegs[1]); }
void SPC4802(void) { REGVAL_READ(SPCCompressionRegs[2]); }
void SPC4803(void) { REGVAL_READ(SPCCompressionRegs[3]); }
void SPC4804(void) { REGVAL_READ(SPCCompressionRegs[4]); }
void SPC4805(void) { REGVAL_READ(SPCCompressionRegs[5]); }
void SPC4806(void) { REGVAL_READ(SPCCompressionRegs[6]); }
void SPC4807(void) { REGVAL_READ(SPCCompressionRegs[7]); }
void SPC4808(void) { REGVAL_READ(SPCCompressionRegs[8]); }
void SPC4809(void) { REGVAL_READ(SPCCompressionRegs[9]); }
void SPC480A(void) { REGVAL_READ(SPCCompressionRegs[0xA]); }
void SPC480B(void) { REGVAL_READ(SPCCompressionRegs[0xB]); }
/* $480C – status register: read and clear */
void SPC480C(void)
{
    u1 v = SPCCompressionRegs[0xC];
    SPCCompressionRegs[0xC] = 0;
    REGVAL_READ(v);
}

/* -- Data-ROM pointer read ports ($4810–$481A) ------------------------------ */

/*
 * $4810 – continuous read from data ROM at (romdata + 0x100000 + SPCROMPtr).
 *
 * Returns 0 until $4811 has been written (SPCCheckFix == 0).
 *
 * Post-read auto-increment depends on SPCROMCom byte[1] (the decoded mode):
 *   mode 0     – add 1 to *SPCROMtoI
 *   mode 1     – add SPCROMInc to *SPCROMtoI
 *   mode other – no increment after $4810 (adjust is triggered by $4814/$481A)
 *
 * If bit 1 of the raw command byte is set (mode 2/3/4 encoded in byte[1]),
 * the read uses the adjust value and increments SPCROMAdj by 1 instead.
 */
void SPC4810(void)
{
    if (SPCCheckFix == 0) {
        REGVAL_READ(0);
        return;
    }
    u1 v;
    if (SPCROMCom & 0x02) {
        /* Adjust mode: read at ptr+adj, then post-increment adj by 1 */
        v = romdata[0x100000 + SPCROMPtr + (u2)SPCROMAdj];
        SPCROMAdj = (u2)(SPCROMAdj + 1);
    } else {
        v = romdata[0x100000 + SPCROMPtr];
        u1 incmode = (u1)(SPCROMCom >> 8);
        if (incmode == 0)
            (*SPCROMtoI)++;
        else if (incmode == 1)
            *SPCROMtoI += SPCROMInc;
        /* modes 2,3,4,0xFF: increment triggered elsewhere */
    }
    REGVAL_READ(v);
}

void SPC4811(void) { REGVAL_READ(((u1*)&SPCROMPtr)[0]); }
void SPC4812(void) { REGVAL_READ(((u1*)&SPCROMPtr)[1]); }
void SPC4813(void) { REGVAL_READ(((u1*)&SPCROMPtr)[2]); }
void SPC4814(void) { REGVAL_READ(((u1*)&SPCROMAdj)[0]); }
void SPC4815(void) { REGVAL_READ(((u1*)&SPCROMAdj)[1]); }
void SPC4816(void) { REGVAL_READ(((u1*)&SPCROMInc)[0]); }
void SPC4817(void) { REGVAL_READ(((u1*)&SPCROMInc)[1]); }
void SPC4818(void) { REGVAL_READ(((u1*)&SPCROMCom)[0]); }

/*
 * $481A – read data ROM at SPCROMPtr + SPCROMAdj (sign-extended 16-bit).
 *
 * Returns 0 until $4811 has been written.
 * After the read, if decoded mode == 4 ("16-bit 4814 after $481A"), add
 * the full (sign-extended) SPCROMAdj to *SPCROMtoI.
 */
void SPC481A(void)
{
    if (SPCCheckFix == 0) {
        REGVAL_READ(0);
        return;
    }
    u1 v = romdata[0x100000 + SPCROMPtr + (u2)SPCROMAdj];
    if ((u1)(SPCROMCom >> 8) == 4)
        *SPCROMtoI += SPCROMAdj;
    REGVAL_READ(v);
}

/* -- Multiply / divide read ports ($4820–$482F) ----------------------------- */

void SPC4820(void) { REGVAL_READ(((u1*)&SPCMultA)[0]); }
void SPC4821(void) { REGVAL_READ(((u1*)&SPCMultA)[1]); }
void SPC4822(void) { REGVAL_READ(((u1*)&SPCMultA)[2]); }
void SPC4823(void) { REGVAL_READ(((u1*)&SPCMultA)[3]); }
void SPC4824(void) { REGVAL_READ(((u1*)&SPCMultB)[0]); }
void SPC4825(void) { REGVAL_READ(((u1*)&SPCMultB)[1]); }
void SPC4826(void) { REGVAL_READ(((u1*)&SPCDivEnd)[0]); }
void SPC4827(void) { REGVAL_READ(((u1*)&SPCDivEnd)[1]); }
void SPC4828(void) { REGVAL_READ(((u1*)&SPCMulRes)[0]); }
void SPC4829(void) { REGVAL_READ(((u1*)&SPCMulRes)[1]); }
void SPC482A(void) { REGVAL_READ(((u1*)&SPCMulRes)[2]); }
void SPC482B(void) { REGVAL_READ(((u1*)&SPCMulRes)[3]); }
void SPC482C(void) { REGVAL_READ(((u1*)&SPCDivRes)[0]); }
void SPC482D(void) { REGVAL_READ(((u1*)&SPCDivRes)[1]); }
void SPC482E(void) { REGVAL_READ(0); } /* always 0 */
void SPC482F(void) { REGVAL_READ(0); } /* always 0: operations are instantaneous */

/* -- Bank-switch read ports ($4831–$4834) ---------------------------------- */

void SPC4831(void) { REGVAL_READ(((u1*)&SPC7110BankA)[0]); }
void SPC4832(void) { REGVAL_READ(((u1*)&SPC7110BankA)[1]); }
void SPC4833(void) { REGVAL_READ(((u1*)&SPC7110BankA)[2]); }
void SPC4834(void) { REGVAL_READ(0); } /* always 0 */

/* -- RTC read ports ($4840–$4842) ------------------------------------------ */

void SPC4840(void) { REGVAL_READ((u1)SPC7110RTCStat); }

/*
 * $4841 – RTC index/data port, read side.
 *
 * State machine (SPC7110RTCStat byte[1]):
 *   0xFE → awaiting command byte: return command, advance state
 *   0xFF → awaiting index byte: same as 0xFE (return command, advance)
 *   0x00–0x0F → return SPC7110RTC[index]; optionally refresh from host clock
 *               when index == 0 and not held/stopped; advance index mod 16
 *
 * Host-clock update fills registers 0x00–0x0C from GetTime()/GetDate().
 * GetTime() returns BCD-packed: seconds in bits 7:0, minutes 15:8, hours 23:16.
 * GetDate() returns: BCD day in 7:0, binary month+1 in 15:8, BCD year in 23:16,
 *                    day-of-week in 31:28.
 */
void SPC4841(void)
{
    u1 stat = ((u1*)&SPC7110RTCStat)[1];

    if (stat == 0xFE || stat == 0xFF) {
        ((u1*)&SPC7110RTCStat)[1]++;
        REGVAL_READ(((u1*)&SPC7110RTCStat)[2]);
        return;
    }

    u1 idx = stat;

    if (idx == 0
        && !(SPC7110RTC[0x0F] & 0x03) /* not stopped / held */
        && !(SPC7110RTC[0x0D] & 0x01) /* not timer-stop */
#ifndef NO_DEBUGGER
        && !debuggeron
#endif
    ) {
        u4 t = GetTime();
        u4 d = GetDate();

        /* Seconds: BCD nibbles in bits 7:4 and 3:0 of t */
        SPC7110RTC[0x00] = (u1)( t        & 0x0F);
        SPC7110RTC[0x01] = (u1)((t >>  4) & 0x0F);
        /* Minutes */
        SPC7110RTC[0x02] = (u1)((t >>  8) & 0x0F);
        SPC7110RTC[0x03] = (u1)((t >> 12) & 0x0F);
        /* Hours (24-hr; bits 23:16 of GetTime return value) */
        SPC7110RTC[0x04] = (u1)((t >> 16) & 0x0F);
        SPC7110RTC[0x05] = (u1)((t >> 20) & 0x0F);

        /* Day: BCD nibbles in bits 7:0 of d */
        SPC7110RTC[0x06] = (u1)( d        & 0x0F);
        SPC7110RTC[0x07] = (u1)((d >>  4) & 0x0F);

        /* Month: binary 1..12 in bits 15:8 of d; split into two BCD nibbles */
        u1 month = (u1)((d >> 8) & 0xFF);
        SPC7110RTC[0x08] = (month > 9) ? (u1)(month - 10) : month;
        SPC7110RTC[0x09] = (month > 9) ? 1 : 0;

        /* Year within decade: BCD digits from bits 23:16 of d.
         * GetDate() places (year mod 100) in BCD format (tens in 7:4, units in 3:0
         * of the 23:16 byte); we need to split into [0xA]=units, [0xB]=tens. */
        u1 yr = (u1)((d >> 16) & 0xFF);
        u1 yr10 = 0;
        u1 yr1  = yr & 0x1F;
        while (yr1 > 9) { yr10++; yr1 -= 10; }
        SPC7110RTC[0x0A] = yr1;
        SPC7110RTC[0x0B] = yr10;

        /* Day of week in bits 31:28 of d */
        SPC7110RTC[0x0C] = (u1)((d >> 28) & 0x0F);
    }

    ((u1*)&SPC7110RTCStat)[1] = (idx + 1) & 0x0F;
    REGVAL_READ(SPC7110RTC[idx]);
}

/* $4842 – RTC ready status: always reports ready (bit 7 = 1) */
void SPC4842(void) { REGVAL_READ(0x80); }

/* -- RTC shadow read ports ($4850–$485F): direct access to SPC7110RTC[] ---- */

void SPC4850(void) { REGVAL_READ(SPC7110RTC[0x0]); }
void SPC4851(void) { REGVAL_READ(SPC7110RTC[0x1]); }
void SPC4852(void) { REGVAL_READ(SPC7110RTC[0x2]); }
void SPC4853(void) { REGVAL_READ(SPC7110RTC[0x3]); }
void SPC4854(void) { REGVAL_READ(SPC7110RTC[0x4]); }
void SPC4855(void) { REGVAL_READ(SPC7110RTC[0x5]); }
void SPC4856(void) { REGVAL_READ(SPC7110RTC[0x6]); }
void SPC4857(void) { REGVAL_READ(SPC7110RTC[0x7]); }
void SPC4858(void) { REGVAL_READ(SPC7110RTC[0x8]); }
void SPC4859(void) { REGVAL_READ(SPC7110RTC[0x9]); }
void SPC485A(void) { REGVAL_READ(SPC7110RTC[0xA]); }
void SPC485B(void) { REGVAL_READ(SPC7110RTC[0xB]); }
void SPC485C(void) { REGVAL_READ(SPC7110RTC[0xC]); }
void SPC485D(void) { REGVAL_READ(SPC7110RTC[0xD]); }
void SPC485E(void) { REGVAL_READ(SPC7110RTC[0xE]); }
void SPC485F(void) { REGVAL_READ(SPC7110RTC[0xF]); }

/* ===========================================================================
 * Register WRITE handlers
 *
 * The REGVAL_WRITE macro declares   register u1 al asm("al")   which captures
 * the byte value that the assembly dispatcher placed in AL before the call.
 * =========================================================================== */

/* -- Compression control write ports ($4801–$480B) ------------------------- */

void SPC4801w(void) { REGVAL_WRITE; SPCCompressionRegs[1] = al; }
void SPC4802w(void) { REGVAL_WRITE; SPCCompressionRegs[2] = al; }
void SPC4803w(void) { REGVAL_WRITE; SPCCompressionRegs[3] = al; }
void SPC4804w(void) { REGVAL_WRITE; SPCCompressionRegs[4] = al; }
void SPC4805w(void) { REGVAL_WRITE; SPCCompressionRegs[5] = al; }
/* $4806 – store then trigger decompression init */
void SPC4806w(void)
{
    REGVAL_WRITE;
    SPCCompressionRegs[6] = al;
    SPC7110_4806w();
}
void SPC4807w(void) { REGVAL_WRITE; SPCCompressionRegs[7] = al; }
void SPC4808w(void) { REGVAL_WRITE; SPCCompressionRegs[8] = al; }
void SPC4809w(void) { REGVAL_WRITE; SPCCompressionRegs[9] = al; }
void SPC480Aw(void) { REGVAL_WRITE; SPCCompressionRegs[0xA] = al; }
void SPC480Bw(void) { REGVAL_WRITE; SPCCompressionRegs[0xB] = al; }

/* -- Data-ROM pointer write ports ($4811–$4818) ----------------------------- */

/* $4811 – low byte of data-ROM pointer; first write enables $4810/$481A */
void SPC4811w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCROMPtr)[0] = al;
    SPCCheckFix = 1;
}
void SPC4812w(void) { REGVAL_WRITE; ((u1*)&SPCROMPtr)[1] = al; }
void SPC4813w(void) { REGVAL_WRITE; ((u1*)&SPCROMPtr)[2] = al; }

/*
 * $4814 – low byte of offset adjust (SPCROMAdj).
 *
 * If decoded mode == 2 ("8-bit 4814"), immediately add the adjust value
 * (signed if bit 3 of the raw command byte is set) to *SPCROMtoI.
 */
void SPC4814w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCROMAdj)[0] = al;

    if ((u1)(SPCROMCom >> 8) == 2) {
        u4 adj = (SPCROMCom & 0x08)
            ? (u4)(s4)(s1)((u1*)&SPCROMAdj)[0]  /* signed */
            : ((u1*)&SPCROMAdj)[0];              /* unsigned */
        *SPCROMtoI += adj;
    }
}

/*
 * $4815 – high byte of offset adjust; sign-extend to 32 bits if signed mode.
 *
 * If decoded mode == 3 ("16-bit 4814"), immediately add the full signed
 * 16-bit adjust to *SPCROMtoI.
 */
void SPC4815w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCROMAdj)[1] = al;
    /* Clear upper 16 bits, then sign-extend from bit 15 if signed mode */
    ((u1*)&SPCROMAdj)[2] = 0;
    ((u1*)&SPCROMAdj)[3] = 0;
    if ((SPCROMCom & 0x08) && (((u1*)&SPCROMAdj)[1] & 0x80)) {
        ((u1*)&SPCROMAdj)[2] = 0xFF;
        ((u1*)&SPCROMAdj)[3] = 0xFF;
    }
    if ((u1)(SPCROMCom >> 8) == 3)
        *SPCROMtoI += SPCROMAdj;
}

void SPC4816w(void) { REGVAL_WRITE; ((u1*)&SPCROMInc)[0] = al; }

/*
 * $4817 – high byte of increment; sign-extend from bit 14 if signed mode
 *         (bit 2 of the raw command byte).
 */
void SPC4817w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCROMInc)[1] = al;
    ((u1*)&SPCROMInc)[2] = 0;
    ((u1*)&SPCROMInc)[3] = 0;
    if ((SPCROMCom & 0x04) && (((u1*)&SPCROMInc)[1] & 0x40)) {
        ((u1*)&SPCROMInc)[2] = 0xFF;
        ((u1*)&SPCROMInc)[3] = 0xFF;
    }
}

/*
 * $4818 – data-ROM command mode register.
 *
 * Bit assignments of the written byte:
 *   7   – unused
 *   6   – if bit 1 and bit 6 set: 16-bit adjust
 *   5   – if bit 1 and bit 5 set: add to adjust target instead of ptr
 *             (when combined with bit 6 → "after $481A" trigger)
 *   4   – 1 = increment target is SPCROMAdj, 0 = SPCROMPtr
 *   3   – 1 = signed calculation for $4814 (SPCROMAdj)
 *   2   – 1 = signed calculation for $4816 (SPCROMInc)
 *   1:0 – 00 = add 1 after $4810, 01 = add $4816 after $4810,
 *          10 = use $4814 as offset increment, 11 = unused
 *
 * Writing this register:
 *   1. Stores the raw byte in SPCROMCom[0].
 *   2. Re-applies sign extension to SPCROMAdj and SPCROMInc.
 *   3. Selects the increment target.
 *   4. Decodes a mode byte into SPCROMCom[1].
 *
 * Decoded modes stored in byte[1] of SPCROMCom:
 *   0x00 – add 1 to *SPCROMtoI after each $4810 read
 *   0x01 – add SPCROMInc to *SPCROMtoI after each $4810 read
 *   0x02 – 8-bit adjust: add SPCROMAdj[0] to *SPCROMtoI on $4814 write
 *   0x03 – 16-bit adjust: add SPCROMAdj to *SPCROMtoI on $4815 write
 *   0x04 – 16-bit adjust: add SPCROMAdj to *SPCROMtoI on $481A read
 *   0xFF – offset-add enabled but trigger point unclear (treated as no-op)
 */
void SPC4818w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCROMCom)[0] = al;

    /* Re-sign-extend SPCROMAdj (from bit 15) if signed-adjust mode (bit 3) */
    ((u1*)&SPCROMAdj)[2] = 0;
    ((u1*)&SPCROMAdj)[3] = 0;
    if ((al & 0x08) && (((u1*)&SPCROMAdj)[1] & 0x80)) {
        ((u1*)&SPCROMAdj)[2] = 0xFF;
        ((u1*)&SPCROMAdj)[3] = 0xFF;
    }

    /* Re-sign-extend SPCROMInc (from bit 14) if signed-increment mode (bit 2) */
    ((u1*)&SPCROMInc)[2] = 0;
    ((u1*)&SPCROMInc)[3] = 0;
    if ((al & 0x04) && (((u1*)&SPCROMInc)[1] & 0x40)) {
        ((u1*)&SPCROMInc)[2] = 0xFF;
        ((u1*)&SPCROMInc)[3] = 0xFF;
    }

    /* Bit 4: select increment target */
    SPCROMtoI = (al & 0x10) ? &SPCROMAdj : &SPCROMPtr;

    /* Decode the increment mode into byte[1] of SPCROMCom */
    u1 mode;
    if (al & 0x02) {
        /* $4814/$4815 offset-add modes (bits 1 set) */
        if (al & 0x40) {
            /* 16-bit adjust */
            mode = (al & 0x20) ? 4 : 3; /* 4=trigger after $481A, 3=after $4815 */
        } else if (al & 0x20) {
            mode = 2; /* 8-bit adjust, trigger after $4814 */
        } else {
            mode = 0xFF; /* offset-add enabled but no well-defined trigger */
        }
    } else if (al & 0x01) {
        mode = 1; /* add SPCROMInc after $4810 read */
    } else {
        mode = 0; /* add 1 after $4810 read */
    }
    ((u1*)&SPCROMCom)[1] = mode;
}

/* -- Multiply / divide write ports ($4820–$482E) --------------------------- */

void SPC4820w(void) { REGVAL_WRITE; ((u1*)&SPCMultA)[0] = al; }
void SPC4821w(void) { REGVAL_WRITE; ((u1*)&SPCMultA)[1] = al; }
void SPC4822w(void) { REGVAL_WRITE; ((u1*)&SPCMultA)[2] = al; }
void SPC4823w(void) { REGVAL_WRITE; ((u1*)&SPCMultA)[3] = al; }
void SPC4824w(void) { REGVAL_WRITE; ((u1*)&SPCMultB)[0] = al; }

/*
 * $4825 – high byte of 16-bit multiplier; perform multiplication immediately.
 *
 * SPCMultA[15:0] × SPCMultB[15:0] → SPCMulRes[31:0]
 * Signed if SPCSignedVal bit 0 is set (sign-extended from 16 bits).
 */
void SPC4825w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCMultB)[1] = al;

    if (!(SPCSignedVal & 1)) {
        SPCMulRes = (u4)(u2)SPCMultA * (u4)(u2)SPCMultB;
    } else {
        SPCMulRes = (u4)((s4)(s2)(u2)SPCMultA * (s4)(s2)(u2)SPCMultB);
    }
}

void SPC4826w(void) { REGVAL_WRITE; ((u1*)&SPCDivEnd)[0] = al; }

/*
 * $4827 – high byte of 16-bit divisor; perform division immediately.
 *
 * SPCMultA[31:0] ÷ SPCDivEnd[15:0] → quotient SPCMulRes, remainder SPCDivRes
 * Division by zero yields quotient = 0xFFFFFFFF, remainder = 0xFFFF.
 */
void SPC4827w(void)
{
    REGVAL_WRITE;
    ((u1*)&SPCDivEnd)[1] = al;

    u2 divisor = (u2)SPCDivEnd;
    if (divisor == 0) {
        SPCMulRes = 0xFFFFFFFF;
        SPCDivRes = 0x0000FFFF;
        return;
    }

    if (!(SPCSignedVal & 1)) {
        /* Unsigned 32÷16 */
        SPCMulRes = SPCMultA / divisor;
        ((u1*)&SPCDivRes)[0] = (u1)(SPCMultA % divisor);
        ((u1*)&SPCDivRes)[1] = (u1)((SPCMultA % divisor) >> 8);
    } else {
        /*
         * Signed 32÷16: dividend is the full 32-bit SPCMultA treated as a
         * signed value (the assembly sign-extended from bit 31 into EDX before
         * IDIV), divisor is sign-extended from 16 bits.
         */
        s4 dividend = (s4)SPCMultA;
        s2 div16    = (s2)divisor;
        s4 quot     = dividend / div16;
        s2 rem      = (s2)(dividend % div16);
        SPCMulRes = (u4)quot;
        ((u1*)&SPCDivRes)[0] = (u1)(u2)rem;
        ((u1*)&SPCDivRes)[1] = (u1)((u2)rem >> 8);
    }
}

/*
 * $482E – reset multiply/divide unit; store signed-mode flag.
 * Clears SPCMultA/B, SPCDivEnd, SPCMulRes, SPCDivRes.
 */
void SPC482Ew(void)
{
    REGVAL_WRITE;
    SPCSignedVal = al;
    SPCMultA  = 0;
    SPCMultB  = 0;
    SPCDivEnd = 0;
    SPCMulRes = 0;
    SPCDivRes = 0;
}

/* -- Bank-switch write ports ($4831–$4833) ---------------------------------- */

/* $4831/$4832/$4833 – map data-ROM bank for SNES banks $D0/$E0/$F0 range */
void SPC4831w(void) { REGVAL_WRITE; BankSwitchSPC7110(al, 0, 0xD0); }
void SPC4832w(void) { REGVAL_WRITE; BankSwitchSPC7110(al, 1, 0xE0); }
void SPC4833w(void) { REGVAL_WRITE; BankSwitchSPC7110(al, 2, 0xF0); }

/* -- RTC write ports ($4840–$4842) ----------------------------------------- */

/*
 * $4840 – RTC chip enable.
 * If bit 0 is set: latch the enable byte and set state to "await command"(0xFE).
 */
void SPC4840w(void)
{
    REGVAL_WRITE;
    if (al & 1) {
        ((u1*)&SPC7110RTCStat)[0] = al;
        ((u1*)&SPC7110RTCStat)[1] = 0xFE;
    }
}

/*
 * $4841 – RTC index/data write.
 *
 * State machine (SPC7110RTCStat byte[1]):
 *   0xFE → store command byte in RTCStat[2]; advance to 0xFF
 *   0xFF → latch lower nibble as register index
 *   0x00–0x0F → write to SPC7110RTC[index]; advance index mod 16.
 *               Special: writing reg 0xF with bit 0 set resets time/date.
 */
void SPC4841w(void)
{
    REGVAL_WRITE;
    u1 stat = ((u1*)&SPC7110RTCStat)[1];

    if (stat == 0xFE) {
        ((u1*)&SPC7110RTCStat)[1]++;   /* → 0xFF */
        ((u1*)&SPC7110RTCStat)[2] = al;
        return;
    }
    if (stat == 0xFF) {
        ((u1*)&SPC7110RTCStat)[1] = al & 0x0F;
        return;
    }

    /* Data write */
    SPC7110RTC[stat] = al;
    if (stat == 0x0F && (al & 0x01)) {
        /* Clear time/date registers (keep control registers 0x0D–0x0F) */
        SPC7110RTC[0x00] = SPC7110RTC[0x01] = 0;
        SPC7110RTC[0x02] = SPC7110RTC[0x03] = 0;
        SPC7110RTC[0x04] = SPC7110RTC[0x05] = 0;
        SPC7110RTC[0x06] = 1; SPC7110RTC[0x07] = 0;
        SPC7110RTC[0x08] = 1; SPC7110RTC[0x09] = 0;
        SPC7110RTC[0x0A] = SPC7110RTC[0x0B] = 0;
        SPC7110RTC[0x0C] = 0;
    }
    ((u1*)&SPC7110RTCStat)[1] = (stat + 1) & 0x0F;
}

/* $4842 – RTC write: no operation */
void SPC4842w(void) { }

/* ===========================================================================
 * Memory-bank access functions for the SPC7110 SRAM mapping
 *
 * These are called through the mrwp dispatch table (cpu/memtable.c) with
 * standard C calling conventions:
 *   read:  u1/u2  f(u4 addr)
 *   write: void   f(u4 addr, u1/u2 val)
 * where addr is the 16-bit within-bank address.
 *
 * SPC7110 SRAM is mapped at:
 *   $00:6000–$00:7FFF  (slow, 8 KB)
 *   $30:6000–$30:7FFF  (fast mirror)
 *
 * SRAMAccessSPC7110 macro routing (from 7110proc.asm):
 *   addr bit 15 set → delegate to memaccessbank (ROM)
 *   addr < 0x6000   → delegate to regaccessbank (I/O registers)
 *   else            → SRAM access; offset = (addr − 0x6000) + bank × 8192
 *
 * Since both mapped banks (0x00, 0x30) map to the same 8 KB physical SRAM
 * (the offset is masked to 16 bits), bank_nr = 0 gives correct results for
 * both mappings.
 * =========================================================================== */

extern u1  memaccessbankr8(u4 addr);
extern void memaccessbankw8(u4 addr, u1 val);
extern u2  memaccessbankr16(u4 addr);
extern void memaccessbankw16(u4 addr, u2 val);

extern u1  regaccessbankr8(u4 addr);
extern void regaccessbankw8(u4 addr, u1 val);
extern u2  regaccessbankr16(u4 addr);
extern void regaccessbankw16(u4 addr, u2 val);

/* Inline SRAM helpers – equivalent to sramaccessbankr8b / sramaccessbankw8b */
static inline u1 sram_read8(u4 off)
{
    return ramsize ? sram[off & ramsizeand] : 0;
}
static inline u2 sram_read16(u4 off)
{
    if (!ramsize) return 0;
    return (u2)(sram[(off    ) & ramsizeand]
              | (u2)sram[(off + 1) & ramsizeand] << 8);
}
static inline void sram_write8(u4 off, u1 val)
{
    if (!ramsize) return;
    sram[off & ramsizeand] = val;
    sramb4save = 5 * 60;
}
static inline void sram_write16(u4 off, u2 val)
{
    if (!ramsize) return;
    sram[(off    ) & ramsizeand] = (u1)val;
    sram[(off + 1) & ramsizeand] = (u1)(val >> 8);
    sramb4save = 5 * 60;
}
static inline u4 spc7110_sram_off(u4 addr)
{
    /* (addr - 0x6000) + 0 * 8192, masked to 16 bits */
    return (addr - 0x6000) & 0xFFFF;
}

u1   SPC7110ReadSRAM8b(u4 addr)
{
    if (addr & 0x8000)  return memaccessbankr8(addr);
    if (addr < 0x6000)  return regaccessbankr8(addr);
    return sram_read8(spc7110_sram_off(addr));
}
void SPC7110WriteSRAM8b(u4 addr, u1 val)
{
    if (addr & 0x8000)  { memaccessbankw8(addr, val); return; }
    if (addr < 0x6000)  { regaccessbankw8(addr, val); return; }
    sram_write8(spc7110_sram_off(addr), val);
}
u2   SPC7110ReadSRAM16b(u4 addr)
{
    if (addr & 0x8000)  return memaccessbankr16(addr);
    if (addr < 0x6000)  return regaccessbankr16(addr);
    return sram_read16(spc7110_sram_off(addr));
}
void SPC7110WriteSRAM16b(u4 addr, u2 val)
{
    if (addr & 0x8000)  { memaccessbankw16(addr, val); return; }
    if (addr < 0x6000)  { regaccessbankw16(addr, val); return; }
    sram_write16(spc7110_sram_off(addr), val);
}

/* ===========================================================================
 * Decompressed-data bank access functions ($50:0000–$50:FFFF)
 *
 * memaccessspc7110r8  – one decompressed byte per call via SPC7110_4800()
 * memaccessspc7110r16 – two consecutive decompressed bytes (lo, hi)
 * memaccessspc7110w8  – no-op (writes to the decompressed area are ignored)
 * memaccessspc7110w16 – no-op
 * =========================================================================== */

u1 memaccessspc7110r8(u4 addr)
{
    (void)addr;
    SPC7110_4800();
    return SPCCompressionRegs[0];
}
u2 memaccessspc7110r16(u4 addr)
{
    (void)addr;
    SPC7110_4800();
    u1 lo = SPCCompressionRegs[0];
    SPC7110_4800();
    u1 hi = SPCCompressionRegs[0];
    return (u2)(lo | (u2)hi << 8);
}
void memaccessspc7110w8(u4 addr, u1 val)  { (void)addr; (void)val; }
void memaccessspc7110w16(u4 addr, u2 val) { (void)addr; (void)val; }
