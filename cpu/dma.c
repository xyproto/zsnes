#include "c_dma.h"

#include "../chips/regabi.h"

/* $420B and $420C, the DMA and HDMA enable registers. The bodies are in
   cpu/c_dma.c; these are the entry points the write table holds. */
REGABI_REG_WRITE8(reg420Bw);
REGABI_REG_WRITE8(reg420Cw);
