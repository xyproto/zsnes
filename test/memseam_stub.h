/* The seam block the memtable handlers pass their arguments through; the
   emulator defines it in cpu/c_memops.c. Each test is a single translation
   unit, so defining it in a header is safe here. */
#ifndef MEMSEAM_STUB_H
#define MEMSEAM_STUB_H

#include <stdint.h>

uint32_t MemSeamA, MemSeamB, MemSeamC, MemSeamD, MemSeamS;

#endif
