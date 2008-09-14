/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../zpath.h"
#include "../cfg.h"

/*
Decompression Code by anomie, Nach, and _Demo_
Based on a reference implementation by neviksti


You may use the code here under the GPL version 2 license.
If you would like to use the decomression code (and only the
decompression code) within a program incompatible with the GPLv2,
you may do so under the following 6 conditions:

1) The program you are using it in is a Super Nintendo emulator.
2) Your emulator's source code is publicly available.
3) In your emulator's credits and documentation's credits you
   thank anomie, Nach, and _Demo_.
4) In your changelogs provided with your emulator or on your
   website; thanks anomie, Nach, and _Demo_ for the SPC7110
   decompression code.
5) Any improvements you make to the decompression code are
   sent to the developers of ZSNES.
6) The GPLv2 header from the top of this file and this notice
   is replicated at the top of the copied decompression code.


If you would like to use any other code here, such as the graphics
caching code, or register emulation under a GPLv2 incompatible
license, or you would like to use the decompression code without
complying with the 6 conditions listed above, you must contact us
and request permission.
*/

static uint8_t EvolutionTable[53][4] =
{
  //prob, nextlps, nextmps, toggle invert
  {0x5a,  1,  1,1}, //0  l,m
  {0x25,  6,  2,0}, //1  l,m
  {0x11,  8,  3,0}, //2  l,m
  {0x08, 10,  4,0}, //3   ,m
  {0x03, 12,  5,0}, //4   ,m
  {0x01, 15,  5,0}, //5   ,m

  {0x5a,  7,  7,1}, //6  l,
  {0x3f, 19,  8,0}, //7  l,m
  {0x2c, 21,  9,0}, //8  l,m
  {0x20, 22, 10,0}, //9   ,m
  {0x17, 23, 11,0}, //10  ,m
  {0x11, 25, 12,0}, //11  ,m
  {0x0c, 26, 13,0}, //12  ,m
  {0x09, 28, 14,0}, //13  ,m
  {0x07, 29, 15,0}, //14  ,m
  {0x05, 31, 16,0}, //15  ,m
  {0x04, 32, 17,0}, //16  ,m
  {0x03, 34, 18,0}, //17  ,m
  {0x02, 35, 5,0},  //18  ,m

  {0x5a, 20, 20,1}, //19 l,m
  {0x48, 39, 21,0}, //20 l,m
  {0x3a, 40, 22,0}, //21 l,m
  {0x2e, 42, 23,0}, //22 l,m
  {0x26, 44, 24,0}, //23 l,m
  {0x1f, 45, 25,0}, //24 l,m
  {0x19, 46, 26,0}, //25 l,m
  {0x15, 25, 27,0}, //26 l,m
  {0x11, 26, 28,0}, //27 l,m
  {0x0e, 26, 29,0}, //28 l,m
  {0x0b, 27, 30,0}, //29  ,m
  {0x09, 28, 31,0}, //30  ,m
  {0x08, 29, 32,0}, //31 l,m
  {0x07, 30, 33,0}, //32 l,m
  {0x05, 31, 34,0}, //33 l,m  <--- changed lps
  {0x04, 33, 35,0}, //34  ,m ... this is NOT skipped
  {0x04, 33, 36,0}, //35  ,m
  {0x03, 34, 37,0}, //36  ,m
  {0x02, 35, 38,0}, //37  ,m ... this is NOT skipped
  {0x02, 36,  5,0}, //38  ,m

  {0x58, 39, 40,1}, //39 l,m
  {0x4d, 47, 41,0}, //40 l,m
  {0x43, 48, 42,0}, //41  ,m
  {0x3b, 49, 43,0}, //42  ,m
  {0x34, 50, 44,0}, //43 l,m
  {0x2e, 51, 45,0}, //44 l,m
  {0x29, 44, 46,0}, //45 l,m
  {0x25, 45, 24,0}, //46  ,m

  {0x56, 47, 48,1}, //47 l,m
  {0x4f, 47, 49,0}, //48 l,m
  {0x47, 48, 50,0}, //49 l,m
  {0x41, 49, 51,0}, //50 l,m
  {0x3c, 50, 52,0}, //51 l,m
  {0x37, 51, 43,0}  //52  ,m
};

#define PROB(x) EvolutionTable[Contexts[x].index][0]
#define NEXT_LPS(x) EvolutionTable[Contexts[x].index][1]
#define NEXT_MPS(x) EvolutionTable[Contexts[x].index][2]
#define TOGGLE_INVERT(x) EvolutionTable[Contexts[x].index][3]

static struct
{
  uint8_t index;
  uint8_t invert;
} Contexts[32];
static uint8_t top,val;
static uint32_t in;
static int mode,inverts,in_count;
static uint8_t *datain;
static uint8_t buffer[32];
static int buf_idx;
static uint32_t pixelorder[16];
static uint32_t pixel_left, pixel_above, pixel_above_left, pixel_context;

//Note that the following function doesn't neccesarily work right when x is 0
//So don't use it outside of SPC7110 code without a protect that x isn't 0,
//or you are happy with input = 0, output = 0.
static inline uint8_t highest_bit_position(uint8_t x)
{
  #if defined(__GNUC__) && defined(__i386__)
  uint16_t x2 = x;
  __asm__ __volatile__("bsrw %0,%0" : "=r" (x2) : "0" (x2));
  return(x2);
  #else
  if (x>>4)
  {
    x = ((0xFFA4>>((x>>4)&0xE))&3)+4;
  }
  else
  {
    x = (0xFFA4>>(x&0xE))&3;
  }
  return(x);
  #endif
}

static inline void update_context(uint8_t con)
{
  uint8_t prob;
  int flag_lps,shift;

  //get PROB
  prob = PROB(con);

  //get symbol
  top = top-prob;
  if(val <= top)
  {
    //mps
    flag_lps=0;
  }
  else
  {
    //lps
    val = val - top - 1;
    top = prob - 1;
    flag_lps=1;
  }

  //renormalize
  shift = 0;
  if (top < 0x7F)
  {
    if (in_count < 8)
    {
      in = (in << 8) | *datain++;
      in_count += 8;
    }
    shift = 7-highest_bit_position(top+1); //1+(top<63)+(top<31)+(top<15)+(top<7)+(top<3)+!top;
    top = ((top+1)<<shift)-1;
    val = (val<<shift)+((in>>(in_count-shift))&((1<<shift)-1));
    in_count -= shift;
  }

  //update processing info
  //update context state
  if (flag_lps)
  {
    inverts = (inverts<<1)+(1-Contexts[con].invert);
    if (TOGGLE_INVERT(con)) { Contexts[con].invert^=1; }
    Contexts[con].index = NEXT_LPS(con);
  }
  else
  {
    inverts = (inverts<<1)+Contexts[con].invert;
    if (shift) { Contexts[con].index = NEXT_MPS(con); }
  }
}



/*
For future calls, the value of pixel_left must be shifted into the first position,
with the rest of the array moved after the first position.
However, a pixel must be returned. The pixel returned is chosen by sorting
pixel_left, pixel_above, and pixel_above_left into the first three positions in a
copied array, with the rest of the array moved after the positions containing
pixel_left, pixel_above, and pixel_above_left. Then index into this copied array.
However this copied is never needed again.
A stable copy and move/sort of 3 values could be done optimally in 4 loops.
But since the array is then thrown away, it would be better to find the appropriate
values without needing to copy and move/sort.

These defines do a copy and move/sort:

#define PIXEL_SHIFT(array, value) \
  temp = array[0]; \
  for(m = 0; temp != value; ++m) \
  { \
    temp2 = temp; \
    temp = array[m+1]; \
    array[m+1] = temp2; \
  } \
  array[0] = temp

#define PIXEL_SHIFT_ALL(ct) \
  PIXEL_SHIFT(pixelorder, pixel_left); \
  memcpy(realorder, pixelorder, ct*sizeof(uint32_t)); \
  PIXEL_SHIFT(realorder, pixel_above_left); \
  PIXEL_SHIFT(realorder, pixel_above); \
  PIXEL_SHIFT(realorder, pixel_left)

The function below moves pixel_left where needed, but instead of copying and sorting
to find the pixel to return, it uses the following algorithm:
Check for equality between pixel_left, pixel_above, and pixel_above_left, and
determine if any of the first 3 positions of the array are desired. In those cases,
the value can be returned immediatly. In other cases, only a single pass is required
to go through the array to account for pixel_above and pixel_above_left (pixel_left
is always at the beginning), and then directly return the value.

This method saves needing a whole array, a copy, and extra sorting loops, replacing
with a method at maximum requiring a single loop through the array.
*/

static inline uint32_t pixel_shift(int index)
{
  uint32_t *p;

  if (*pixelorder != pixel_left)
  {
    uint32_t previous = pixel_left;
    for (p = pixelorder; *p != pixel_left; ++p)
    {
      uint32_t hold = *p;
      *p = previous;
      previous = hold;
    }
    *p = previous;
  }

  if (index)
  {
    switch (pixel_context)
    {
      case 0: //((pixel_left == pixel_above) && (pixel_above == pixel_above_left))
        return(pixelorder[index]);

      case 1: //(pixel_left == pixel_above)
        if (index == 1) { return(pixel_above_left); }
        for (p = pixelorder+1; p < pixelorder+index; ++p)
        {
          if (*p == pixel_above_left) { return(pixelorder[index]); }
        }
        return(pixelorder[index-1]);

      case 2: case 3: //((pixel_left == pixel_above_left) || (pixel_above == pixel_above_left))
        if (index == 1) { return(pixel_above); }
        for (p = pixelorder+1; p < pixelorder+index; ++p)
        {
          if (*p == pixel_above) { return(pixelorder[index]); }
        }
        return(pixelorder[index-1]);

      case 4: //pixel_left != pixel_above != pixel_above_left != pixel_left
        if (index == 1) { return(pixel_above); }
        if (index == 2) { return(pixel_above_left); }
        for (p = pixelorder+1; p < pixelorder+index; ++p)
        {
          if (*p == pixel_above)
          {
            for (p = p+1; p < pixelorder+index; ++p)
            {
              if (*p == pixel_above_left) { return(pixelorder[index]); }
            }
            return((pixelorder[index-1] == pixel_above) ? pixelorder[index-2] : pixelorder[index-1]);
          }
          if (*p == pixel_above_left)
          {
            for (p = p+1; p < pixelorder+index; ++p)
            {
              if (*p == pixel_above) { return(pixelorder[index]); }
            }
            return((pixelorder[index-1] == pixel_above_left) ? pixelorder[index-2] : pixelorder[index-1]);
          }
        }
        return(pixelorder[index-2]);
    }
  }
  return(pixel_left); //pixel_left is always index 0
}

static void InitDecompression(int inmode, uint8_t *data)
{
  int i;

  mode=inmode;
  datain=data;
  top=0xFF;
  val=*datain++;
  in=*datain++;
  inverts=0;
  in_count=8;
  memset(Contexts, 0, sizeof(Contexts));
  memset(buffer, 0, sizeof(buffer));
  buf_idx=32;
  for (i=0; i<16; i++) { pixelorder[i]=((i&8)<<21)|((i&4)<<14)|((i&2)<<7)|(i&1); }
  pixel_left=pixel_above=pixel_above_left=0;
}

#define CONTEXT() (pixel_left == pixel_above ? pixel_above != pixel_above_left : pixel_above == pixel_above_left ? 2 : 3 + (pixel_left != pixel_above_left))
//#define CONTEXT() ((pixel_left==pixel_above && pixel_above==pixel_above_left)?0:(pixel_left==pixel_above)?1:(pixel_above==pixel_above_left)?2:(pixel_left==pixel_above_left)?3:4)


static uint8_t DecompressByte(void)
{
  int i, bit;
  uint8_t con;
  uint32_t pixel;
  uint32_t out, out2;

  if (buf_idx>=32)
  {
    switch (mode)
    {
      case 0:
        out=(buffer[30]<<8)+buffer[31];
        for (i=0; i<32; i++)
        {
          update_context(0);
          update_context(1+(inverts&1));
          update_context(3+(inverts&3));
          update_context(7+(inverts&7));
          update_context(15);
          update_context(16+(inverts&1));
          update_context(18+(inverts&3));
          update_context(22+(inverts&7));
          out = (out<<8) + (((out>>8)^inverts)&0xff);
          buffer[i] = (uint8_t)out;
        }
        break;

      case 1:
        out=(buffer[30]<<8)+buffer[31];
        for (i=0; i<32; i+=2)
        {
          for (bit=7; bit>=0; bit--)
          {
            //get first symbol context
            con=pixel_context=CONTEXT();
            update_context(con);

            //get context of second symbol
            con = 5 + con*2 + (inverts&1);
            update_context(con);

            // Update pixel map
            pixel = pixel_shift(inverts&3);

            // Update reference pixels
            pixel_left = ((out >> 0) & 0x0101);
            pixel_above = ((out >> 6) & 0x0101);
            pixel_above_left = ((out >> 7) & 0x0101);

            //get new pixel
            out = ((out<<1)&0xfefe) + pixel;
          }
          buffer[i]=(uint8_t)(out >> 8);
          buffer[i+1]=(uint8_t)(out >> 0);
        }
        break;

      case 2:
        out=(buffer[14]<<24)+(buffer[15]<<16)+(buffer[30]<<8)+buffer[31];
        for (i=0; i<16; i+=2)
        {
          out2=0;
          for (bit=7; bit>=0; bit--)
          {
            //// First bit
            update_context(0);

            //// Second bit
            con = 1 + (inverts&1);
            update_context(con);

            pixel_context = CONTEXT();

            //// Third bit
            if (con == 1)
            {
              con = 5 + 5*(inverts&1) + pixel_context;
            }
            else
            {
              con = 3 + (inverts&1);
            }
            update_context(con);

            //// Fourth bit
            if (con<10)
            {
              con = 9 + con*2 + (inverts&1);
            }
            else
            {
              con = 29 + (inverts&1);
            }
            update_context(con);

            pixel = pixel_shift(inverts&0x0f);

            // Update reference pixels
            pixel_left = pixel;
            pixel_above = ((out >> (bit-1)) & 0x01010101);
            pixel_above_left = ((out >> bit) & 0x01010101);

            //get new pixel
            out2 += pixel<<bit;
          }
          // Miscalculated 'pixel_above' at the end of the loop above, so fix it now
          pixel_above = ((out2 >> 7) & 0x01010101);
          out=out2;

          buffer[i+ 0] = (uint8_t)(out >> 24);
          buffer[i+ 1] = (uint8_t)(out >> 16);
          buffer[i+16] = (uint8_t)(out >>  8);
          buffer[i+17] = (uint8_t)(out >>  0);
        }
        break;
    }
    buf_idx=0;
  }
  return(buffer[buf_idx++]);
}

static void DecompressSkipBytesBuffer(uint8_t *buffer, uint16_t amount)
{
  while (amount--)
  {
    *buffer++ = DecompressByte();
  }
}

static void DecompressSkipBytes(uint16_t amount)
{
  while (amount--)
  {
    DecompressByte();
  }
}

//Communication Code
extern uint32_t CRC32;
extern uint8_t SPCCompressionRegs[];
extern uint8_t *romdata;

#define READ_WORD16_LE(pos) (*(uint16_t *)(pos))
#define WRITE_WORD16_LE(pos, val) (*(uint16_t *)(pos) = (val))

#define READ_WORD24_LE(pos) ((uint32_t)(*(uint16_t *)(pos)) + (((uint32_t)((pos)[2])) << 16))
#define READ_WORD24_BE(pos) ((((uint32_t)((pos)[0])) << 16) + (((uint32_t)((pos)[1])) << 8) + (pos)[2])

#define NUM_ELEMENTS(x) (sizeof((x))/sizeof((x)[0]))



//Caching Code

#define TABLE_AMOUNT 256
#define LOOKUP_AMOUNT 64

struct decompression_table
{
  uint8_t *data;
  uint16_t length;
};

struct address_lookup
{
  uint32_t address;
  struct decompression_table *table;
};

static struct
{
  uint32_t rom_crc32;

  uint32_t last_address;
  uint8_t last_entry;

  uint8_t *compression_begin;
  uint8_t compression_mode;

  uint16_t decompression_used_length;

  uint8_t *graphics_buffer;
  uint32_t graphics_buffer_used;

  struct decompression_table *tables;
  uint16_t table_used;
  struct decompression_table *table_current;

  struct address_lookup *lookup;
  uint8_t lookup_used;
} decompression_state;


static void save_decompression_state()
{
  if (decompression_state.graphics_buffer)
  {
    char fname[13];
    FILE *fp_idx;

    sprintf(fname, "%08X.idx", decompression_state.rom_crc32);
    if ((fp_idx = fopen_dir(ZSramPath, fname, "wb")))
    {
      gzFile fp_gfx;

      sprintf(fname, "%08X.gfx", decompression_state.rom_crc32);
      if ((fp_gfx = gzopen_dir(ZSramPath, fname, "wb9")))
      {
        struct address_lookup *lookup_ptr = decompression_state.lookup,
                              *lookup_end = decompression_state.lookup+(decompression_state.lookup_used-1);
        for (; lookup_ptr < lookup_end; ++lookup_ptr)
        {
          unsigned int entry_index;
          for (entry_index = 0; entry_index < 256; ++entry_index)
          {
            if (lookup_ptr->table[entry_index].length) //We only write graphics that have completed decompressing
            {
              fwrite(&lookup_ptr->address, 3, 1, fp_idx);
              fwrite(&entry_index, 1, 1, fp_idx);
              fwrite(&lookup_ptr->table[entry_index].length, 2, 1, fp_idx);

              gzwrite(fp_gfx, lookup_ptr->table[entry_index].data, lookup_ptr->table[entry_index].length);
            }
          }
        }
        gzclose(fp_gfx);
      }
      fclose(fp_idx);
    }
  }
}

static void load_decompression_state()
{
  if (decompression_state.graphics_buffer)
  {
    char fname[13];
    FILE *fp_idx;

    sprintf(fname, "%08X.idx", decompression_state.rom_crc32);
    if ((fp_idx = fopen_dir(ZSramPath, fname, "rb")))
    {
      gzFile fp_gfx;

      sprintf(fname, "%08X.gfx", decompression_state.rom_crc32);
      if ((fp_gfx = gzopen_dir(ZSramPath, fname, "rb")))
      {
        struct address_lookup *lookup_ptr = decompression_state.lookup-1;

        uint32_t address = 0, last_address = 0;
        uint16_t length;
        uint8_t entry;

        for (;;)
        {
          fread(&address, 3, 1, fp_idx);
          fread(&entry, 1, 1, fp_idx);
          fread(&length, 2, 1, fp_idx);

          if (feof(fp_idx)) { break; }

          if (last_address != address)
          {
            ++decompression_state.lookup_used;
            (++lookup_ptr)->address = last_address = address;
            lookup_ptr->table = decompression_state.tables+decompression_state.table_used;
            decompression_state.table_used += TABLE_AMOUNT;
          }
          lookup_ptr->table[entry].data = decompression_state.graphics_buffer+decompression_state.graphics_buffer_used;
          lookup_ptr->table[entry].length = length;
          decompression_state.graphics_buffer_used += length;

          gzread(fp_gfx, lookup_ptr->table[entry].data, length);
        }
        gzclose(fp_gfx);
      }
      fclose(fp_idx);
    }
  }
}


static bool SPC7110_init_decompression_state()
{
  if (SPC7110Cache)
  {
    size_t lookup_bytes = LOOKUP_AMOUNT*sizeof(struct address_lookup);
    size_t table_bytes = TABLE_AMOUNT*LOOKUP_AMOUNT*sizeof(struct decompression_table);

    if (!decompression_state.graphics_buffer)
    {
      memset(&decompression_state, 0, sizeof(decompression_state));

      decompression_state.graphics_buffer = malloc(0x1000000); //16MB
      if (decompression_state.graphics_buffer)
      {
        decompression_state.lookup = malloc(lookup_bytes);
        if (decompression_state.lookup)
        {
          decompression_state.tables = malloc(table_bytes);
          if (decompression_state.tables)
          {
            memset(decompression_state.tables, 0, table_bytes);
            decompression_state.rom_crc32 = CRC32;
            load_decompression_state();
          }
          else
          {
            free(decompression_state.lookup);
            free(decompression_state.graphics_buffer);
          }
        }
        else
        {
          free(decompression_state.graphics_buffer);
        }
      }
    }
    else //Loading a second SPC7110 game right after another
    {
      uint8_t *graphics_buffer = decompression_state.graphics_buffer;
      struct decompression_table *tables = decompression_state.tables;
      struct address_lookup *lookup = decompression_state.lookup;

      save_decompression_state();

      memset(&decompression_state, 0, sizeof(decompression_state));

      decompression_state.graphics_buffer = graphics_buffer;
      decompression_state.tables = tables;
      decompression_state.lookup = lookup;

      memset(decompression_state.tables, 0, table_bytes);
      decompression_state.rom_crc32 = CRC32;
      load_decompression_state();
    }
  }

  return(decompression_state.graphics_buffer);
}

void SPC7110_deinit_decompression_state()
{
  if (decompression_state.graphics_buffer)
  {
    save_decompression_state();

    free(decompression_state.graphics_buffer);
    free(decompression_state.tables);
    free(decompression_state.lookup);

    memset(&decompression_state, 0, sizeof(decompression_state));
  }
}


static void get_lookup(uint32_t address)
{
  int low = 0,
      high = decompression_state.lookup_used-1,
      mid;

  decompression_state.table_current = 0;

  while (low <= high)
  {
    mid = (low+high)>>1;
    if (decompression_state.lookup[mid].address < address)
    {
      low = mid+1;
    }
    else if (decompression_state.lookup[mid].address > address)
    {
      high = mid-1;
    }
    else
    {
      decompression_state.table_current = decompression_state.lookup[mid].table;
      break;
    }
  }

  if (!decompression_state.table_current)
  {
    memmove(decompression_state.lookup+(low+1), decompression_state.lookup+low, (decompression_state.lookup_used-low)*sizeof(struct address_lookup));
    ++decompression_state.lookup_used;
    decompression_state.lookup[low].address = address;
    decompression_state.table_current = decompression_state.lookup[low].table = decompression_state.tables+decompression_state.table_used;
    decompression_state.table_used += TABLE_AMOUNT;
  }
}


static void init_buffered_decompression(uint32_t address, uint8_t entry, uint16_t skip_amount)
{
  if (decompression_state.graphics_buffer)
  {
    //First handle previous decompression cache
    if (decompression_state.last_address && //Check that there was indeed a last decompression
        !decompression_state.table_current->length) //And it exceeded the known length
    {
      decompression_state.table_current->length = decompression_state.decompression_used_length;
      decompression_state.graphics_buffer_used += decompression_state.decompression_used_length;
    }

    if ((decompression_state.last_address != address) || (decompression_state.last_entry != entry))
    {
      uint8_t *spc7110_table = romdata + 0x100000 + address + (((uint16_t)entry) << 2); //<<2 because each entry is 4 bytes
      decompression_state.last_address = address;
      decompression_state.last_entry = entry;
      decompression_state.compression_mode = *spc7110_table++;
      decompression_state.compression_begin = romdata + 0x100000 + READ_WORD24_BE(spc7110_table);
      decompression_state.decompression_used_length = skip_amount<<decompression_state.compression_mode;

      get_lookup(address);
      decompression_state.table_current += entry;

      if (!decompression_state.table_current->length)
      {
        decompression_state.table_current->data = decompression_state.graphics_buffer+decompression_state.graphics_buffer_used;
        InitDecompression(decompression_state.compression_mode, decompression_state.compression_begin);
        DecompressSkipBytesBuffer(decompression_state.table_current->data, decompression_state.decompression_used_length);
      }
    }
    else
    {
      decompression_state.decompression_used_length = skip_amount<<decompression_state.compression_mode;
    }
  }
}

static uint8_t read_buffered_decompress(uint8_t byte)
{
  if (decompression_state.table_current)
  {
    if (decompression_state.table_current->length && //There is a known length
        decompression_state.table_current->length <= decompression_state.decompression_used_length) //And it's about to exceed it
    {
      decompression_state.table_current->data = decompression_state.graphics_buffer+decompression_state.graphics_buffer_used;
      decompression_state.table_current->length = 0;

      InitDecompression(decompression_state.compression_mode, decompression_state.compression_begin);
      DecompressSkipBytesBuffer(decompression_state.table_current->data, decompression_state.decompression_used_length+1);

      //puts("Exceeded previous known length");
    }
    else if (!decompression_state.table_current->length)
    {
      decompression_state.table_current->data[decompression_state.decompression_used_length] = DecompressByte();
    }

    byte = decompression_state.table_current->data[decompression_state.decompression_used_length++];
  }
  return(byte);
}

static void init_non_buffered_decompression(uint32_t address, uint8_t entry, uint16_t skip_amount)
{
  uint8_t *spc7110_table = romdata + 0x100000 + address + (((uint16_t)entry) << 2); //<<2 because each entry is 4 bytes
  decompression_state.last_address = address;
  decompression_state.last_entry = entry;
  decompression_state.compression_mode = *spc7110_table++;
  decompression_state.compression_begin = romdata + 0x100000 + READ_WORD24_BE(spc7110_table);
  decompression_state.decompression_used_length = skip_amount<<decompression_state.compression_mode;

  InitDecompression(decompression_state.compression_mode, decompression_state.compression_begin);
  DecompressSkipBytes(decompression_state.decompression_used_length);
}

static uint8_t read_non_buffered_decompress(uint8_t byte)
{
  if (decompression_state.last_address)
  {
    ++decompression_state.decompression_used_length;
    byte = DecompressByte();
  }
  return(byte);
}

void copy_spc7110_state_data(uint8_t **buffer, void (*copy_func)(unsigned char **, void *, size_t), bool load)
{
  copy_func(buffer, &decompression_state.last_address, 3);
  copy_func(buffer, &decompression_state.last_entry, sizeof(uint8_t));
  copy_func(buffer, &decompression_state.decompression_used_length, sizeof(uint16_t));

  if (load && decompression_state.last_address)
  {
    uint32_t last_address = decompression_state.last_address;
    uint8_t last_entry = decompression_state.last_entry;
    uint16_t decompression_used_length = decompression_state.decompression_used_length;

    decompression_state.last_address = 0;
    decompression_state.last_entry = 0;
    decompression_state.decompression_used_length = 0;

    if (decompression_state.graphics_buffer)
    {
      init_buffered_decompression(last_address, last_entry, 0);
    }
    else
    {
      init_non_buffered_decompression(last_address, last_entry, 0);
      DecompressSkipBytes(decompression_used_length);
    }
    decompression_state.decompression_used_length = decompression_used_length;
  }
}

//Processing Code, a work in progress
/*
SPCCompressionRegs[x]
0 - Decompressed byte
1 - Compression table low
2 - Compression table high
3 - Compression table bank
4 - Compression table index
5 - Decompression buffer index low
6 - Decompression buffer index high
7 - DMA Channel
8 - ?
9 - Compression length low
A - Compression length high
B - Decompression control register
C - Decompression status
*/

void (*init_decompression)(uint32_t address, uint8_t entry, uint16_t skip_amount);
uint8_t (*read_decompress)(uint8_t byte);

void SPC7110initC()
{
  memset(SPCCompressionRegs, 0, 0x0C);
  if (SPC7110_init_decompression_state())
  {
    init_decompression = init_buffered_decompression;
    read_decompress = read_buffered_decompress;
  }
  else
  {
    init_decompression = init_non_buffered_decompression;
    read_decompress = read_non_buffered_decompress;
  }
}

//DECOMPRESSED DATA CONTINUOUS READ PORT
//Returns a decompressed value from bank $50 and decrements 16 bit counter value at $4809/A by 1
void SPC7110_4800()
{
  WRITE_WORD16_LE(SPCCompressionRegs+9, READ_WORD16_LE(SPCCompressionRegs+9)-1);
  SPCCompressionRegs[0] = read_decompress(SPCCompressionRegs[0]);
}

void SPC7110_4806w()
{
  init_decompression(READ_WORD24_LE(SPCCompressionRegs+1), SPCCompressionRegs[4], READ_WORD16_LE(SPCCompressionRegs+5));
  SPCCompressionRegs[0xC] = 0x80;
}
