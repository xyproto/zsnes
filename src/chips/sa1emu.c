/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdint.h>
 
extern unsigned char SA1_BRF[16];
extern unsigned int SA1_CC2_line;  //should be cleared to zero on reset

extern unsigned int SA1DMAChar, SA1DMADest, SA1DMASource;
extern uint8_t IRAM[2048], *SA1RAMArea;

#define DMACB (SA1DMAChar&3)
#define DDA SA1DMADest
#define SA1_IRAM IRAM

void SA1_DMA_CC2() {
  //select register file index (0-7 or 8-15)
  const unsigned char *brf = &SA1_BRF[(SA1_CC2_line & 1) << 3];
  unsigned bpp = 2 << (2 - DMACB);
  unsigned addr = DDA & 0x07ff;
  unsigned byte;
  addr &= ~((1 << (7 - DMACB)) - 1);
  addr += (SA1_CC2_line & 8) * bpp;
  addr += (SA1_CC2_line & 7) * 2;

  for(byte = 0; byte < bpp; byte++) {
    uint8_t output = 0;
    unsigned bit;
    for(bit = 0; bit < 8; bit++) {
      output |= ((brf[bit] >> byte) & 1) << (7 - bit);
    }
    SA1_IRAM[addr + ((byte & 6) << 3) + (byte & 1)] = output;
  }

  SA1_CC2_line = (SA1_CC2_line + 1) & 15;
}

#define SA1_BWRAM SA1RAMArea
#define DSA SA1DMASource
#define BWRAM_SIZE 0x40000
#define DMASIZE ((SA1DMAChar>>2)&7)

unsigned char SA1_DMA_VALUE;
unsigned int SA1_DMA_ADDR;

void SA1_DMA_CC1()
{
  //16 bytes/char (2bpp); 32 bytes/char (4bpp); 64 bytes/char (8bpp)
  unsigned charmask = (1 << (6 - DMACB)) - 1;

  if((SA1_DMA_ADDR & charmask) == 0) {
    //buffer next character to I-RAM
    unsigned bpp = 2 << (2 - DMACB);
    unsigned bpl = (8 << DMASIZE) >> DMACB;
    unsigned bwmask = BWRAM_SIZE - 1;
    unsigned tile = ((SA1_DMA_ADDR - DSA) & bwmask) >> (6 - DMACB);
    unsigned ty = (tile >> DMASIZE);
    unsigned tx = tile & ((1 << DMASIZE) - 1);
    unsigned bwaddr = DSA + ty * 8 * bpl + tx * bpp;
    unsigned y;

    for(y = 0; y < 8; y++) {
      uint64_t data = 0;
      unsigned byte, x;
      for(byte = 0; byte < bpp; byte++) {
        data |= (uint64_t)SA1_BWRAM[(bwaddr + byte) & bwmask] << (byte << 3);
      }
      bwaddr += bpl;

      uint8_t out[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      for(x = 0; x < 8; x++) {
        out[0] |= (data & 1) << (7 - x); data >>= 1;
        out[1] |= (data & 1) << (7 - x); data >>= 1;
        if(DMACB == 2) continue;
        out[2] |= (data & 1) << (7 - x); data >>= 1;
        out[3] |= (data & 1) << (7 - x); data >>= 1;
        if(DMACB == 1) continue;
        out[4] |= (data & 1) << (7 - x); data >>= 1;
        out[5] |= (data & 1) << (7 - x); data >>= 1;
        out[6] |= (data & 1) << (7 - x); data >>= 1;
        out[7] |= (data & 1) << (7 - x); data >>= 1;
      }

      for(byte = 0; byte < bpp; byte++) {
        unsigned p = DDA + (y << 1) + ((byte & 6) << 3) + (byte & 1);
        SA1_IRAM[p & 0x07ff] = out[byte];
      }
    }
  }

  SA1_DMA_VALUE = SA1_IRAM[(DDA + (SA1_DMA_ADDR & charmask)) & 0x07ff];
}
