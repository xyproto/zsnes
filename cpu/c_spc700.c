/* Entry points for the SPC700 I/O register ($00F0-$00FF) dispatch. The
   handlers are in cpu/spc_ioregs.h; this only declares the state they touch
   and exposes what the SPC core's WriteByte/ReadByte call. */
#include <stdbool.h>

#include "../types.h"
#include "c_dsp.h" /* DSPWriteReg */
#include "../endmem.h" /* tableadc */
#include "../gblvars.h" /* cycpbl, curexecstate, timer2upd */
#include "spc700.h" /* spcRamDP */

/* SPCRAM is 0xFFC0 bytes of RAM immediately followed, in cpu/spc700.asm's data
 * section, by the 64-byte IPL ROM window at $FFC0 - hence the unsized array. */
extern u1 SPCRAM[];
extern u1 DSPMem[256];
extern u1 SPCROM[64], spcextraram[64];
extern u1 disablespcclr, SPCSkipXtraROM;
extern u1 reg1read, reg2read, reg3read, reg4read;
extern u4 spc700read;
extern u1 timeron, timincr0, timincr1, timincr2, timinl0, timinl1, timinl2;
extern u1 spcnumread;
extern u1 timrcall;


#include "spc_ioregs.h"
void SPCWriteReg(u4 reg, u1 val)
{
    spc_write_reg(reg, val);
}

u1 SPCReadReg(u4 reg)
{
    return spc_read_reg(reg);
}

#include "spc_ops.h"

/* Re-arm the SPC core after a timer tick (the `reenablespc` macro): once the
 * budget has run away, zero it and, if the SPC was disabled, switch the 65816
 * back to the table selected by its current flags. */
static void reenablespc(u4 const edx, zreg* const pedi)
{
    if (cycpbl < 0x1000000) return;
    cycpbl = 0;
    if (curexecstate & 0x02) return;
    curexecstate |= 0x02;
    *pedi = (zreg)(uintptr_t)tableadc[(u1)edx];
}

/* One scanline of SPC timer service. Timers 0 and 1 run at 8 kHz (every other
 * call, tracked by timrcall); timer 2 runs at 64 kHz, i.e. four steps per call.
 * Every 60th call the whole body runs twice. */
/* pedi is the caller's edi slot, not an opfn*** - punning a zreg slot through
   an incompatible pointer type lets -O3 assume they do not alias, and the
   caller then keeps a stale opcode table across the call. */
void UpdateTimer(u4 const edx, zreg* const pedi)
{
    for (;;) {
        timrcall ^= 0x01;
        if (timrcall & 0x01) {
            if (timeron & 1 && --timinl0 == 0) {
                SPCRAM[0xFD]++;
                timinl0 = timincr0;
                if (SPCRAM[0xFD] == 1) {
                    reenablespc(edx, pedi);
                    cycpbl = 0;
                }
            }
            if (timeron & 2 && --timinl1 == 0) {
                SPCRAM[0xFE]++;
                timinl1 = timincr1;
                if (SPCRAM[0xFE] == 1) {
                    reenablespc(edx, pedi);
                    cycpbl = 0;
                }
            }
        }
        if (timeron & 4) {
            for (u4 i = 0; i != 4; ++i) {
                if (--timinl2 != 0) continue;
                SPCRAM[0xFF]++;
                timinl2 = timincr2;
                if (SPCRAM[0xFF] == 1) {
                    reenablespc(edx, pedi);
                    cycpbl = 0;
                }
            }
        }
        if (++timer2upd != 60) break;
        timer2upd = 0;
    }
}
