/*
 * Conversion between the 65816 P register and the split flag globals, from the
 * makedl / restoredl macros in cpu/65816d.inc.
 *
 * The core keeps N, V, Z and C as the x86 flags left them rather than in P:
 * Z is "the low 16 bits of flagnz are zero", N is its bit 15 or 16, and flagc
 * and flago are 0 or 0xFF. P is reassembled only when something outside the
 * core has to see it - PHP, an interrupt, REP/SEP with a bit outside MXD.
 *
 * Textual include; the includer declares flagnz, flago and flagc.
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
