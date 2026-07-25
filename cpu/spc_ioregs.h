/*
 * cpu/spc_ioregs.h - SPC700 I/O register ($00F0-$00FF) read/write handlers,
 * ported from the SPCRegF0..FF / RSPCRegF0..FF routines in cpu/spc700.asm.
 *
 * Textual include (cpu/c_spc700.c): the includer must first provide the u1/u4
 * typedefs and declarations for the globals used below - SPCRAM[], DSPMem,
 * SPCROM, spcextraram, dspWptr, disablespcclr, SPCSkipXtraROM,
 * reg1read..reg4read, spc700read, timeron, timincr0..2, timinl0..2, spcnumread.
 *
 * Register ABI of the original asm (preserved by the callers): the dispatch
 * passes the register number in ebx (0xF0..0xFF) and, for writes, the value in
 * al; reads return the value in al. Handlers must not disturb the SPC core's
 * other registers - in C that is automatic, so only eax has to be threaded
 * through explicitly.
 */
#ifndef SPC_IOREGS_H
#define SPC_IOREGS_H

/* SPCRegF3 hands the byte to the addressed DSP register's asm write handler,
 * which uses the register ABI (al=value, ebx=reg) - marshal to it the same way
 * the asm did (call through dspWptr, value in/out of eax). */
static inline u4 spc_call_dspw(u1 dspreg, u4 eax)
{
    u4 ebx = dspreg;
    __asm__ volatile("call *%[fn]"
        : "+a"(eax), "+b"(ebx)
        : [fn] "r"((void*)dspWptr[dspreg])
        : "ecx", "edx", "cc", "memory");
    return eax;
}

/* Write to SPC I/O register `reg` (0xF0..0xFF); the byte written is the low
 * half of `eax`. Returns eax as the asm left it: unchanged except for $F3,
 * where the DSP write handler's own return value shows through - callers of
 * WriteByte keep using ah afterwards, so the whole register has to be exact. */
static inline u4 spc_write_reg(u4 reg, u4 eax)
{
    const u1 al = (u1)eax;

    switch (reg) {
    case 0xF0: /* undocumented test register */
        SPCRAM[0xF0] = al;
        break;

    case 0xF1: /* control: clear input ports, page in IPL ROM, enable timers */
        if (disablespcclr != 1) {
            if (al & 0x10) { SPCRAM[0xF4] = 0; SPCRAM[0xF5] = 0; }
            if (al & 0x20) { SPCRAM[0xF6] = 0; SPCRAM[0xF7] = 0; }
        }
        if (SPCSkipXtraROM != 1) {
            const u1* src = (al & 0x80) ? SPCROM : spcextraram;
            for (u4 i = 0; i < 0x40; i++)
                SPCRAM[0xFFC0 + i] = src[i];
        }
        SPCRAM[0xF1] = al;
        timeron = al & 0x0F;
        break;

    case 0xF2: /* DSP register address; preview its current data at $F3 */
        SPCRAM[0xF2] = al;
        SPCRAM[0xF3] = DSPMem[al];
        break;

    case 0xF3: /* DSP register data write */
        eax = spc_call_dspw(SPCRAM[0xF2] & 0x7F, eax);
        SPCRAM[0xF3] = (u1)eax;
        break;

    case 0xF4: reg1read = al; spc700read++; break;
    case 0xF5: reg2read = al; spc700read++; break;
    case 0xF6: reg3read = al; spc700read++; break;
    case 0xF7: reg4read = al; spc700read++; break;

    case 0xF8: SPCRAM[0xF8] = al; break;
    case 0xF9: SPCRAM[0xF9] = al; break;

    case 0xFA: /* timer targets; latch into the counter only when it is idle */
        timincr0 = al;
        if (timinl0 == 0) timinl0 = al;
        SPCRAM[0xFA] = al;
        break;
    case 0xFB:
        timincr1 = al;
        if (timinl1 == 0) timinl1 = al;
        SPCRAM[0xFB] = al;
        break;
    case 0xFC:
        timincr2 = al;
        if (timinl2 == 0) timinl2 = al;
        SPCRAM[0xFC] = al;
        break;

    case 0xFD: /* timer outputs are read-only */
    case 0xFE:
    case 0xFF:
        break;
    }
    return eax;
}

/* Read SPC I/O register `reg` (0xF0..0xFF); returns the byte. */
static inline u1 spc_read_reg(u4 reg)
{
    switch (reg) {
    case 0xF8: /* $F8/$F9 read back as 0 in the asm */
    case 0xF9:
        return 0;

    case 0xFD: { /* reading a timer output returns its low nibble and clears it */
        u1 v = SPCRAM[0xFD] & 0x0F;
        SPCRAM[0xFD] = 0;
        spcnumread = 0;
        return v;
    }
    case 0xFE: {
        u1 v = SPCRAM[0xFE] & 0x0F;
        SPCRAM[0xFE] = 0;
        spcnumread = 0;
        return v;
    }
    case 0xFF: {
        u1 v = SPCRAM[0xFF] & 0x0F;
        SPCRAM[0xFF] = 0;
        spcnumread = 0;
        return v;
    }

    default: /* $F0-$F7, $FA-$FC read straight from SPC RAM */
        return SPCRAM[reg];
    }
}

#endif /* SPC_IOREGS_H */
