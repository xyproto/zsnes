#ifndef C_HQX_H
#define C_HQX_H

/* hq2x, hq3x and hq4x are MaxSt's filters, ported from the original assembly;
   each rule set serves both depths. See video/c_hqx.c and tools/hqxport.py. */
void hq2x_16b(void);
void hq2x_32b(void);
void hq3x_16b(void);
void hq3x_32b(void);
void hq4x_16b(void);
void hq4x_32b(void);

#endif
