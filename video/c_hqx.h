#ifndef C_HQX_H
#define C_HQX_H

// Scalar nearest-neighbor block scalers (C ports of the gutted hqNx asm).
void hq2x_16b(void);
void hq2x_32b(void);
void hq3x_16b(void);
void hq3x_32b(void);
void hq4x_16b(void);
void hq4x_32b(void);

#endif
