/*
 * cpu/flags65816.h - conversion between the 65816 P register and the split
 * flag globals, from the makedl / restoredl macros in cpu/65816d.inc.
 *
 * The core does not keep N, V, Z and C in P. It keeps them spread over three
 * globals in the form the x86 flags left them, because that is free to write
 * and cheap to test: Z is "the low 16 bits of flagnz are zero", N is bit 15 or
 * bit 16 of it, and flagc / flago are 0 or 0xFF. P is only reassembled when
 * something outside the core has to see it - PHP, an interrupt, REP/SEP with a
 * bit outside MXD - which is what these two do.
 *
 * Textual include: the includer must have declared flagnz, flago and flagc.
 */
#ifndef FLAGS65816_H
#define FLAGS65816_H

/* Gather the split flags into dl. Only the low byte of edx is touched. */
static inline u4 makedl(u4 edx)
{
    edx &= 0xFFFFFF3Cu;
    if ((flagnz & 0x00018000u) != 0)
        edx |= 0x80; /* neg */
    if ((flagnz & 0x0000FFFFu) == 0)
        edx |= 0x02; /* zero */
    if ((flagc & 0x000000FFu) != 0)
        edx |= 0x01; /* carry */
    if ((flago & 0x000000FFu) != 0)
        edx |= 0x40; /* v */
    return edx;
}

/* And scatter them back out of it. */
static inline void restoredl(u4 const edx)
{
    u4 nz = 0;
    if (edx & 0x80)
        nz |= 0x00010000; /* neg */
    if (!(edx & 0x02))
        nz |= 0x00000001; /* no zero */
    flagnz = nz;
    flagc = edx & 0x01 ? 0x000000FF : 0; /* carry */
    flago = edx & 0x40 ? 0x000000FF : 0; /* v */
}

#endif /* FLAGS65816_H */
