// Looks good
#ifndef C_HQX_H
#define C_HQX_H

/* hq2x_16b and hq3x_16b are the real filters, ported from MaxSt's assembly.
   The 32-bit twins and hq4x are still nearest-neighbour block scalers; see
   TODO.md. */
void hq2x_16b(void);
void hq2x_32b(void);
void hq3x_16b(void);
void hq3x_32b(void);
void hq4x_16b(void);
void hq4x_32b(void);

#endif
