/*
 * SPC700 I/O register ($00F0-$00FF) handlers, from the SPCRegF0..FF and
 * RSPCRegF0..FF routines in cpu/spc700.asm. Textual include (cpu/c_spc700.c),
 * which supplies the integer typedefs, the SPC globals and DSPWriteReg().
 *
 * The dispatch passes the register number in ebx and, for a write, the value
 * in al; a read returns it in al. Callers save eax around the call because the
 * core keeps live data in ah.
 */
#ifndef SPC_IOREGS_H
#define SPC_IOREGS_H

/* Write `al` to SPC I/O register `reg` (0xF0..0xFF). */
static inline void spc_write_reg(u4 reg, u1 al)
{
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
        DSPWriteReg(SPCRAM[0xF2] & 0x7F, al);
        SPCRAM[0xF3] = al;
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
