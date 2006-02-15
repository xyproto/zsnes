/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __UNIXSDL__
#include "gblhdr.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <zlib.h>
#ifdef __WIN32__
#include <direct.h>
#include <io.h>
#define ftruncate chsize
#else
#include <unistd.h>
#endif
#define DIR_SLASH "\\"
#endif
#include "gblvars.h"
#include "asm_call.h"
#include "numconv.h"

#ifndef __WIN32__
#define mkdir(path) mkdir(path, (S_IRWXU|S_IRWXG|S_IRWXO)) //0777
#endif

#ifdef __GNUC__
typedef unsigned long long uint64;
#else //MSVC
typedef unsigned __int64 uint64;
#endif

extern unsigned int versionNumber, CRC32, cur_zst_size, MsgCount, MessageOn;
extern unsigned int JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;
extern unsigned char pl1contrl, pl2contrl, pl3contrl, pl4contrl, pl5contrl;
extern unsigned char MovieStartMethod, GUIReset, ReturnFromSPCStall, GUIQuit;
extern unsigned char MovieProcessing, CMovieExt, EmuSpeed;
extern char *Msgptr, fnamest[512];
extern bool romispal;
bool MovieWaiting = false;

enum MovieStatus { MOVIE_OFF = 0, MOVIE_PLAYBACK, MOVIE_RECORD, MOVIE_OLD_PLAY };
#define SetMovieMode(mode) (MovieProcessing = (unsigned char)mode)

extern unsigned char snesmouse;
extern unsigned short latchx, latchy;
#define IS_MOUSE_1() (snesmouse == 1)
#define IS_MOUSE_2() (snesmouse == 2)
#define IS_SCOPE()   (snesmouse == 3)

void GUIDoReset();
void powercycle(bool);
void zst_sram_load(FILE *);
void zst_sram_load_compressed(FILE *);
void zst_save(FILE *, bool, bool);
bool zst_load(FILE *, size_t);
bool zst_compressed_loader(FILE *);

/////////////////////////////////////////////////////////

#ifdef DEBUG_INPUT
#define debug_input_start useda = usedb = usedc = usedd = usede = 0;
#define debug_input print_bin(JoyAOrig, &useda); printf(" "); \
                    print_bin(JoyBOrig, &usedb); printf(" "); \
                    print_bin(JoyCOrig, &usedc); printf(" "); \
                    print_bin(JoyDOrig, &usedd); printf(" "); \
                    print_bin(JoyEOrig, &usede); printf("\n");

static unsigned int useda, usedb, usedc, usedd, usede;
static void print_bin(unsigned int num, unsigned int *used)
{
  unsigned int mask = BIT(31);
  while (mask)
  {
    *used |= num & mask;
    printf(num & mask ? "1" : "0");
    mask >>= 1;
  }
}

#define debug_input_used printf("Used: ");\
                         print_bin(useda, &useda); printf(" "); \
                         print_bin(usedb, &usedb); printf(" "); \
                         print_bin(usedc, &usedc); printf(" "); \
                         print_bin(usedd, &usedd); printf(" "); \
                         print_bin(usede, &usede); printf("\n");

#else
#define debug_input_start
#define debug_input
#define debug_input_used
#endif

/////////////////////////////////////////////////////////

/*
ZMV Format

-----------------------------------------------------------------
Header
-----------------------------------------------------------------

3 bytes  -  "ZMV"
2 bytes  -  ZMV Version # (version of ZSNES)
4 bytes  -  CRC32 of ROM
4 bytes  -  Number of frames in this movie
4 bytes  -  Number of rerecords
4 bytes  -  Number of frames removed by rerecord
4 bytes  -  Number of frames advanced step-by-step
1 byte   -  Average recording FPS (includes dropped frames) x4-5 for precision
4 bytes  -  Number of key combos
2 bytes  -  Number of internal chapters
2 bytes  -  Length of author name
3 bytes  -  Uncompressed ZST size
2 bytes  -  Initial input configuration
  1 bit  -   Input 1 enabled
  1 bit  -   Input 2 enabled
  1 bit  -   Input 3 enabled
  1 bit  -   Input 4 enabled
  1 bit  -   Input 5 enabled
  1 bit  -   Mouse in first port
  1 bit  -   Mouse in second port
  1 bit  -   Super Scope in second port
  8 bits -   Reserved
1 byte   -  Flag Byte
  2 bits -   Start from ZST/Power On/Reset/Power On + Clear SRAM
  1 bit  -   NTSC or PAL
  5 bits -   Reserved
3 bytes  -  1 bit for compressed or not, 23 bits for size
ZST size -  ZST (no thumbnail)


-----------------------------------------------------------------
Key input  -  Repeated for all input / internal chapters
-----------------------------------------------------------------

1 byte   - Flag Byte
  1 bit  -  Controller 1 changed
  1 bit  -  Controller 2 changed
  1 bit  -  Controller 3 changed
  1 bit  -  Controller 4 changed
  1 bit  -  Controller 5 changed
  1 bit  -  Chapter instead of input here
  1 bit  -  RLE instead of input
  1 bit  -  Command here

-If Command-
Remaining 7 bits of flag determine command

-Else If RLE-
4 bytes  -  Frame # to repeat previous input to

-Else If Chapter-

3 bytes  -  1 bit for compressed or not, 23 bits for size
ZST size -  ZST
4 bytes  -  Frame #
2 bytes  -  Controller Status
9 bytes  -  Maximum previous input (1 Scope [20] + 4 Regular [12*4] + 4 padded bits)

-Else-

variable - Input

  12 bits per regular controller, 18 per mouse, 20 for scope.
     Input changed padded to next full byte size
  Minimum 2 bytes (12 controller bits + 4 padded bits)
  Maximum 9 bytes (20 scope controller bits + 48 regular controller bits [12*4] + 4 padded bits)


-----------------------------------------------------------------
Internal chapter offsets  -  Repeated for all internal chapters
-----------------------------------------------------------------

4 bytes  -  Offset to chapter from beginning of file (after input flag byte for ZST)


-----------------------------------------------------------------
External chapters  -  Repeated for all external chapters
-----------------------------------------------------------------

ZST Size -  ZST (never compressed)
4 bytes  -  Frame #
2 bytes  -  Controller Status
9 bytes  -  Maximum previous input (1 Scope [20] + 4 Regular [12*4] + 4 padded bits)
4 bytes  -  Offset to input for current chapter from beginning of file


-----------------------------------------------------------------
External chapter count
-----------------------------------------------------------------

2 bytes  - Number of external chapters

-----------------------------------------------------------------
Author name
-----------------------------------------------------------------

Name Len - Author's name

*/


/*

ZMV header types, vars, and functions

*/

enum zmv_start_methods { zmv_sm_zst, zmv_sm_power, zmv_sm_reset, zmv_sm_clear_all };
enum zmv_video_modes { zmv_vm_ntsc, zmv_vm_pal };
enum zmv_commands { zmv_command_reset };

#define INT_CHAP_SIZE(offset) (internal_chapter_length(offset)+4+2+9)
#define EXT_CHAP_SIZE (cur_zst_size+4+2+9+4)

#define INT_CHAP_INDEX_SIZE (zmv_vars.header.internal_chapters*4)
#define EXT_CHAP_BLOCK_SIZE (zmv_open_vars.external_chapter_count*EXT_CHAP_SIZE + 2)

#define EXT_CHAP_END_DIST (EXT_CHAP_BLOCK_SIZE + (size_t)zmv_vars.header.author_len)
#define INT_CHAP_END_DIST (INT_CHAP_INDEX_SIZE + EXT_CHAP_END_DIST)

#define EXT_CHAP_COUNT_END_DIST ((size_t)zmv_vars.header.author_len + 2)

struct zmv_header
{
  char magic[3];
  unsigned short zsnes_version;
  unsigned int rom_crc32;
  unsigned int frames;
  unsigned int rerecords;
  unsigned int removed_frames;
  unsigned int incr_frames;
  unsigned char average_fps;
  unsigned int key_combos;
  unsigned short internal_chapters;
  unsigned short author_len;
  unsigned int zst_size; //We only read/write 3 bytes for this
  unsigned short initial_input;
  struct
  {
    enum zmv_start_methods start_method;
    enum zmv_video_modes video_mode;
  } zmv_flag;
};

static void zmv_header_write(struct zmv_header *zmv_head, FILE *fp)
{
  unsigned char flag = 0;

  fwrite(zmv_head->magic, 3, 1, fp);
  fwrite2(zmv_head->zsnes_version, fp);
  fwrite4(zmv_head->rom_crc32, fp);
  fwrite4(zmv_head->frames, fp);
  fwrite4(zmv_head->rerecords, fp);
  fwrite4(zmv_head->removed_frames, fp);
  fwrite4(zmv_head->incr_frames, fp);
  fwrite(&zmv_head->average_fps, 1, 1, fp);
  fwrite4(zmv_head->key_combos, fp);
  fwrite2(zmv_head->internal_chapters, fp);
  fwrite2(zmv_head->author_len, fp);
  fwrite3(zmv_head->zst_size, fp);
  fwrite2(zmv_head->initial_input, fp);

  switch (zmv_head->zmv_flag.start_method)
  {
    case zmv_sm_zst:
      flag &= ~BIT(7);
      flag &= ~BIT(6);
      break;

    case zmv_sm_power:
      flag |= BIT(7);
      flag &= ~BIT(6);
      break;

    case zmv_sm_reset:
      flag &= ~BIT(7);
      flag |= BIT(6);
      break;

    case zmv_sm_clear_all:
      flag |= BIT(7);
      flag |= BIT(6);
      break;
  }

  switch (zmv_head->zmv_flag.video_mode)
  {
    case zmv_vm_ntsc:
      flag &= ~BIT(5);
      break;

    case zmv_vm_pal:
      flag |= BIT(5);
      break;
  }

  //Not needed, but oh well, it makes it easier to read for some.
  //Reserved bits:
  flag &= ~BIT(4);
  flag &= ~BIT(3);
  flag &= ~BIT(2);
  flag &= ~BIT(1);
  flag &= ~BIT(0);

  fwrite(&flag, 1, 1, fp);
}

static bool zmv_header_read(struct zmv_header *zmv_head, FILE *fp)
{
  unsigned char flag;

  fread(zmv_head->magic, 3, 1, fp);
  zmv_head->zsnes_version = fread2(fp);
  zmv_head->rom_crc32 = fread4(fp);
  zmv_head->frames = fread4(fp);
  zmv_head->rerecords = fread4(fp);
  zmv_head->removed_frames = fread4(fp);
  zmv_head->incr_frames = fread4(fp);
  fread(&zmv_head->average_fps, 1, 1, fp);
  zmv_head->key_combos = fread4(fp);
  zmv_head->internal_chapters = fread2(fp);
  zmv_head->author_len = fread2(fp);
  zmv_head->zst_size = fread3(fp);
  zmv_head->initial_input = fread2(fp);
  fread(&flag, 1, 1, fp);

  if (feof(fp))
  {
    return(false);
  }

  switch (flag & (BIT(7)|BIT(6)))
  {
    case 0:
      zmv_head->zmv_flag.start_method = zmv_sm_zst;
      break;

    case BIT(7):
      zmv_head->zmv_flag.start_method = zmv_sm_power;
      break;

    case BIT(6):
      zmv_head->zmv_flag.start_method = zmv_sm_reset;
      break;

    case BIT(7)|BIT(6):
      zmv_head->zmv_flag.start_method = zmv_sm_clear_all;
      break;
  }

  switch (flag & BIT(5))
  {
    case 0:
      zmv_head->zmv_flag.video_mode = zmv_vm_ntsc;
      break;

    case BIT(5):
      zmv_head->zmv_flag.video_mode = zmv_vm_pal;
      break;
  }

  if (flag & (BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0)))
  {
    return(false);
  }

  return(true);
}

/*

Internal chapter types, vars, and functions

*/

#define INTERNAL_CHAPTER_BUF_LIM 10
struct internal_chapter_buf
{
  size_t offsets[INTERNAL_CHAPTER_BUF_LIM];
  unsigned char used;
  struct internal_chapter_buf *next;
};

static void internal_chapter_add_offset(struct internal_chapter_buf *icb, size_t offset)
{
  while (icb->next)
  {
    icb = icb->next;
  }

  if (icb->used == INTERNAL_CHAPTER_BUF_LIM)
  {
    icb->next = (struct internal_chapter_buf *)malloc(sizeof(struct internal_chapter_buf));
    icb = icb->next;
    memset(icb, 0, sizeof(struct internal_chapter_buf));
  }

  icb->offsets[icb->used] = offset;
  icb->used++;
}

static void internal_chapter_free_chain(struct internal_chapter_buf *icb)
{
  if (icb)
  {
    if (icb->next)
    {
      internal_chapter_free_chain(icb->next);
    }
    free(icb);
  }
}

static void internal_chapter_write(struct internal_chapter_buf *icb, FILE *fp)
{
  unsigned char i;
  for (i = 0; i < icb->used; i++)
  {
    fwrite4(icb->offsets[i], fp);
  }
  if (icb->next)
  {
    internal_chapter_write(icb->next, fp);
  }
}

static void internal_chapter_read(struct internal_chapter_buf *icb, FILE *fp, size_t count)
{
  while (count--)
  {
    internal_chapter_add_offset(icb, fread4(fp));
  }
}

size_t internal_chapter_pos(struct internal_chapter_buf *icb, size_t offset)
{
  size_t pos = 0;
  do
  {
    unsigned char i;
    for (i = 0; i < icb->used; i++, pos++)
    {
      if (icb->offsets[i] == offset)
      {
        return(pos);
      }
    }
  } while ((icb = icb->next));
  return(~0);
}

static size_t internal_chapter_greater(struct internal_chapter_buf *icb, size_t offset)
{
  size_t greater = ~0;
  do
  {
    unsigned char i;
    for (i = 0; i < icb->used; i++)
    {
      if ((icb->offsets[i] > offset) && (icb->offsets[i] < greater))
      {
        greater = icb->offsets[i];
      }
    }
  } while ((icb = icb->next));
  return((greater == 0xFFFFFFFF) ? offset : greater);
}

static size_t internal_chapter_lesser(struct internal_chapter_buf *icb, size_t offset)
{
  size_t lesser = 0;
  do
  {
    unsigned char i;
    for (i = 0; i < icb->used; i++)
    {
      if ((icb->offsets[i] < offset) && (icb->offsets[i] > lesser))
      {
        lesser = icb->offsets[i];
      }
    }
  } while ((icb = icb->next));
  return((lesser == 0) ? offset : lesser);
}

static size_t internal_chapter_delete_after(struct internal_chapter_buf *icb, size_t offset)
{
  if (icb->used)
  {
    size_t last_offset = 0;
    if (icb->offsets[0] == offset)
    {
      last_offset = offset;
    }
    else
    {
      last_offset = internal_chapter_lesser(icb, offset);
      if (last_offset == offset)
      {
        internal_chapter_free_chain(icb->next);
        icb->next = 0;
        icb->used = 0;
        return(0);
      }
      if (internal_chapter_greater(icb, last_offset) == offset)
      {
        last_offset = offset;
      }
    }

    if (last_offset)
    {
      size_t buffer_pos = internal_chapter_pos(icb, last_offset);
      size_t link = buffer_pos/INTERNAL_CHAPTER_BUF_LIM;

      while (link--)
      {
        icb = icb->next;
      }

      internal_chapter_free_chain(icb->next);
      icb->next = 0;
      icb->used = (buffer_pos%INTERNAL_CHAPTER_BUF_LIM)+1;
      return(buffer_pos+1);
    }
  }
  return(0);
}

/*

Bit Encoder and Decoder

*/

/*
When working with bits, you have to find the bits in a byte.

Devide the amount of bits by 8 (bit_count >> 3) to find the proper byte.
The proper bit number in the byte is the amount of bits modulo 8 (bit_count & 7).
To get the most signifigant bit, you want the bit which is 7 minus the proper bit number.
*/
size_t bit_encoder(unsigned int data, unsigned int mask, unsigned char *buffer, size_t skip_bits)
{
  unsigned char bit_loop;

  for (bit_loop = 31; ; bit_loop--)
  {
    if (mask & BIT(bit_loop))
    {
      if (data & BIT(bit_loop))
      {
        buffer[skip_bits>>3] |= BIT(7-(skip_bits&7));
      }
      else
      {
        buffer[skip_bits>>3] &= ~BIT(7-(skip_bits&7));
      }
      skip_bits++;
    }

    if (!bit_loop) { break; }
  }

  return(skip_bits);
}

size_t bit_decoder(unsigned int *data, unsigned int mask, unsigned char *buffer, size_t skip_bits)
{
  unsigned char bit_loop;
  *data = 0;

  for (bit_loop = 31; ; bit_loop--)
  {
    if (mask & BIT(bit_loop))
    {
      if (buffer[skip_bits>>3] & BIT(7-(skip_bits&7)))
      {
        *data |= BIT(bit_loop);
      }
      skip_bits++;
    }

    if (!bit_loop) { break; }
  }

  return(skip_bits);
}

/*

Shared var between record/replay functions

*/

#define WRITE_BUFFER_SIZE 1024
static struct
{
  struct zmv_header header;
  FILE *fp;
  struct
  {
    unsigned int A;
    unsigned int B;
    unsigned int C;
    unsigned int D;
    unsigned int E;
    unsigned short latchx;
    unsigned short latchy;
  } last_joy_state;
  unsigned short inputs_enabled;
  unsigned char write_buffer[WRITE_BUFFER_SIZE];
  size_t write_buffer_loc;
  struct internal_chapter_buf internal_chapters;
  size_t last_internal_chapter_offset;
  char *filename;
  size_t rle_count;
} zmv_vars;

#define GAMEPAD_MASK 0xFFF00000
#define MOUSE_MASK 0x00C0FFFF
#define SCOPE_MASK 0xF0000000

#define GAMEPAD_ENABLE 0x00008000
#define MOUSE_ENABLE 0x00010000
#define SCOPE_ENABLE 0x00FF0000

static size_t pad_bit_encoder(unsigned char pad, unsigned char *buffer, size_t skip_bits)
{
  unsigned int last_state = 0;

  switch (pad)
  {
    case 1:
      last_state = zmv_vars.last_joy_state.A;
      break;

    case 2:
      last_state = zmv_vars.last_joy_state.B;
      break;

    case 3:
      last_state = zmv_vars.last_joy_state.C;
      break;

    case 4:
      last_state = zmv_vars.last_joy_state.D;
      break;

    case 5:
      last_state = zmv_vars.last_joy_state.E;
      break;
  }

  switch (pad)
  {
    case 2:
      if ((zmv_vars.inputs_enabled & BIT(0x8))) //Super Scope
      {
        unsigned int xdata = (zmv_vars.last_joy_state.latchx - 40) & 0xFF;
        unsigned int ydata = zmv_vars.last_joy_state.latchy & 0xFF;

        skip_bits = bit_encoder(last_state, SCOPE_MASK, buffer, skip_bits);
        skip_bits = bit_encoder(xdata, 0x000000FF, buffer, skip_bits);
        skip_bits = bit_encoder(ydata, 0x000000FF, buffer, skip_bits);

        break;
      }

    case 1:
      if ((zmv_vars.inputs_enabled & ((pad == 1) ? BIT(0xA) : BIT(0x9)))) //Mouse ?
      {
        skip_bits = bit_encoder(last_state, MOUSE_MASK, buffer, skip_bits);
      }
      else
      {
        skip_bits = bit_encoder(last_state, GAMEPAD_MASK, buffer, skip_bits);
      }
      break;

    case 3: case 4: case 5:
      //No multitap if both ports use special devices
      if ((zmv_vars.inputs_enabled & (BIT(0xA)|BIT(0x9)|BIT(0x8))) <= BIT(0xA))
      {
        skip_bits = bit_encoder(last_state, GAMEPAD_MASK, buffer, skip_bits);
      }
      break;
  }
  return(skip_bits);
}

static size_t pad_bit_decoder(unsigned char pad, unsigned char *buffer, size_t skip_bits)
{
  unsigned int *last_state = 0;
  unsigned short input_enable_mask = 0;

  switch (pad)
  {
    case 1:
      last_state = &zmv_vars.last_joy_state.A;
      input_enable_mask = BIT(0xF);
      break;

    case 2:
      last_state = &zmv_vars.last_joy_state.B;
      input_enable_mask = BIT(0xE);
      break;

    case 3:
      last_state = &zmv_vars.last_joy_state.C;
      input_enable_mask = BIT(0xD);
      break;

    case 4:
      last_state = &zmv_vars.last_joy_state.D;
      input_enable_mask = BIT(0xC);
      break;

    case 5:
      last_state = &zmv_vars.last_joy_state.E;
      input_enable_mask = BIT(0xB);
      break;
  }

  switch (pad)
  {
    case 2:
      if ((zmv_vars.inputs_enabled & BIT(0x8))) //Super Scope
      {
        unsigned int xdata, ydata;

        skip_bits = bit_decoder(last_state, SCOPE_MASK, buffer, skip_bits);
        skip_bits = bit_decoder(&xdata, 0x000000FF, buffer, skip_bits);
        skip_bits = bit_decoder(&ydata, 0x000000FF, buffer, skip_bits);
        *last_state |= SCOPE_ENABLE;

        zmv_vars.last_joy_state.latchx = (unsigned short)(xdata + 40);
        zmv_vars.last_joy_state.latchy = (unsigned short)ydata;

        break;
      }

    case 1:
      if (zmv_vars.inputs_enabled & ((pad == 1) ? BIT(0xA) : BIT(0x9))) //Mouse ?
      {
        skip_bits = bit_decoder(last_state, MOUSE_MASK, buffer, skip_bits);
        *last_state |= MOUSE_ENABLE;
      }
      else
      {
        skip_bits = bit_decoder(last_state, GAMEPAD_MASK, buffer, skip_bits);
        *last_state |= (zmv_vars.inputs_enabled & input_enable_mask) ? GAMEPAD_ENABLE : 0;
      }
      break;

    case 3: case 4: case 5:
      //No multitap if both ports use special devices
      if ((zmv_vars.inputs_enabled & (BIT(0xA)|BIT(0x9)|BIT(0x8))) > BIT(0xA))
      {
        *last_state = 0;
      }
      else
      {
        skip_bits = bit_decoder(last_state, GAMEPAD_MASK, buffer, skip_bits);
        *last_state |= (zmv_vars.inputs_enabled & input_enable_mask) ? GAMEPAD_ENABLE : 0;
      }
      break;
  }
  return(skip_bits);
}

static void save_last_joy_state(unsigned char *buffer)
{
  size_t skip_bits = 16;
  memcpy(buffer, uint16_to_bytes(zmv_vars.inputs_enabled), 2);

  skip_bits = pad_bit_encoder(1, buffer, skip_bits);
  skip_bits = pad_bit_encoder(2, buffer, skip_bits);
  skip_bits = pad_bit_encoder(3, buffer, skip_bits);
  skip_bits = pad_bit_encoder(4, buffer, skip_bits);
  skip_bits = pad_bit_encoder(5, buffer, skip_bits);
}

static void load_last_joy_state(unsigned char *buffer)
{
  size_t skip_bits = 16;
  zmv_vars.inputs_enabled = bytes_to_uint16(buffer);

  skip_bits = pad_bit_decoder(1, buffer, skip_bits);
  skip_bits = pad_bit_decoder(2, buffer, skip_bits);
  skip_bits = pad_bit_decoder(3, buffer, skip_bits);
  skip_bits = pad_bit_decoder(4, buffer, skip_bits);
  skip_bits = pad_bit_decoder(5, buffer, skip_bits);
}

//These things are a total of 11 bytes (2 byte enable field, up to 9 bytes for input bits)
static void write_last_joy_state(FILE *fp)
{
  save_last_joy_state(zmv_vars.write_buffer);
  fwrite(zmv_vars.write_buffer, 11, 1, fp);
}

static void read_last_joy_state(FILE *fp)
{
  fread(zmv_vars.write_buffer, 11, 1, fp);
  load_last_joy_state(zmv_vars.write_buffer);
}

static void flush_input_buffer()
{
  if (zmv_vars.write_buffer_loc)
  {
    fwrite(zmv_vars.write_buffer, zmv_vars.write_buffer_loc, 1, zmv_vars.fp);
    zmv_vars.write_buffer_loc = 0;
  }

  if (zmv_vars.rle_count)
  {
    if (zmv_vars.rle_count > 5)
    {
        zmv_vars.write_buffer[0] = BIT(1); //RLE bit
        fwrite(zmv_vars.write_buffer, 1, 1, zmv_vars.fp);
        fwrite4(zmv_vars.header.frames, zmv_vars.fp);
    }
    else
    {
      memset(zmv_vars.write_buffer, 0, zmv_vars.rle_count);
      fwrite(zmv_vars.write_buffer, zmv_vars.rle_count, 1, zmv_vars.fp);
    }
    zmv_vars.rle_count = 0;
  }

  zmv_vars.header.author_len = 0; //If we're writing, then author is erased if there
}

static void flush_input_if_needed()
{
  if (zmv_vars.write_buffer_loc > WRITE_BUFFER_SIZE - 15) //14 is a RLE buffer (5) + flag (1) + largest input (9)
  {
    flush_input_buffer();
  }
}

//For various ZMV calculations, the length of the last chapter needs to be known
static size_t internal_chapter_length(size_t offset)
{
  size_t current_loc = ftell(zmv_vars.fp);
  size_t icl = 0;

  fseek(zmv_vars.fp, offset, SEEK_SET);
  icl = fread3(zmv_vars.fp) & 0x007FFFFF; //The upper 9 bits are not part of the length
  icl += 3; //Add 3 for the header which says how long it is

  fseek(zmv_vars.fp, current_loc, SEEK_SET);
  return(icl);
}

/*

Create and record ZMV

*/

static void zmv_create(char *filename)
{
  memset(&zmv_vars, 0, sizeof(zmv_vars));
  if ((zmv_vars.fp = fopen(filename,"w+b")))
  {
    size_t filename_len = strlen(filename);
    strncpy(zmv_vars.header.magic, "ZMV", 3);
    zmv_vars.header.zsnes_version = versionNumber & 0xFFFF;
    zmv_vars.header.rom_crc32 = CRC32;
    zmv_vars.header.zst_size = cur_zst_size;
    zmv_vars.header.zmv_flag.start_method = (enum zmv_start_methods)MovieStartMethod;
    zmv_vars.header.zmv_flag.video_mode = romispal ? zmv_vm_pal : zmv_vm_ntsc;
    zmv_vars.header.average_fps = romispal ? 250 : 240;
    zmv_vars.header.initial_input = (pl1contrl    ? BIT(0xF) : 0) |
                                    (pl2contrl    ? BIT(0xE) : 0) |
                                    (pl3contrl    ? BIT(0xD) : 0) |
                                    (pl4contrl    ? BIT(0xC) : 0) |
                                    (pl5contrl    ? BIT(0xB) : 0) |
                                    (IS_MOUSE_1() ? BIT(0xA) : 0) |
                                    (IS_MOUSE_2() ? BIT(0x9) : 0) |
                                    (IS_SCOPE()   ? BIT(0x8) : 0);

    zmv_header_write(&zmv_vars.header, zmv_vars.fp);

    zmv_vars.inputs_enabled = zmv_vars.header.initial_input;

    switch (zmv_vars.header.zmv_flag.start_method)
    {
      case zmv_sm_zst:
        break;
      case zmv_sm_power:
        MovieWaiting = true;
        powercycle(true);
        break;
      case zmv_sm_reset:
        GUIReset = 1;
        asm_call(GUIDoReset);
        ReturnFromSPCStall = 0;
        break;
      case zmv_sm_clear_all:
        MovieWaiting = true;
        powercycle(false);
        break;
    }

    zst_save(zmv_vars.fp, false, true);
    zmv_vars.filename = (char *)malloc(filename_len+1); //+1 for null
    strcpy(zmv_vars.filename, filename);

    debug_input_start;
  }
  else
  {

  }
}

static void zmv_rle_flush()
{
  if (zmv_vars.rle_count)
  {
    if (zmv_vars.rle_count > 5)
    {
      zmv_vars.write_buffer[zmv_vars.write_buffer_loc++] = BIT(1); //RLE bit
      memcpy(zmv_vars.write_buffer+zmv_vars.write_buffer_loc, uint32_to_bytes(zmv_vars.header.frames), 4);
      zmv_vars.write_buffer_loc += 4;
    }
    else
    {
      memset(zmv_vars.write_buffer+zmv_vars.write_buffer_loc, 0, zmv_vars.rle_count);
      zmv_vars.write_buffer_loc += zmv_vars.rle_count;
    }
    zmv_vars.rle_count = 0;
  }
}

static void zmv_record_command(enum zmv_commands command)
{
  zmv_rle_flush();

  zmv_vars.write_buffer[zmv_vars.write_buffer_loc++] = (command << 1) | BIT(0);

  flush_input_if_needed();
}

static void record_pad(unsigned char pad, unsigned char *flag, unsigned char *buffer, size_t *skip_bits)
{
  unsigned int *last_state = 0, current_state = 0;
  unsigned char bit_mask = 0;

  switch (pad)
  {
    case 1:
      last_state = &zmv_vars.last_joy_state.A;
      current_state = JoyAOrig;
      bit_mask = BIT(7);
      break;

    case 2:
      last_state = &zmv_vars.last_joy_state.B;
      current_state = JoyBOrig;
      bit_mask = BIT(6);
      break;

    case 3:
      last_state = &zmv_vars.last_joy_state.C;
      current_state = JoyCOrig;
      bit_mask = BIT(5);
      break;

    case 4:
      last_state = &zmv_vars.last_joy_state.D;
      current_state = JoyDOrig;
      bit_mask = BIT(4);
      break;

    case 5:
      last_state = &zmv_vars.last_joy_state.E;
      current_state = JoyEOrig;
      bit_mask = BIT(3);
      break;
  }

  if ((current_state != *last_state) ||
      ((zmv_vars.inputs_enabled & BIT(0x8)) &&
       ((zmv_vars.last_joy_state.latchx != latchx) || (zmv_vars.last_joy_state.latchy != latchy))))
  {
    zmv_vars.last_joy_state.latchx = latchx;
    zmv_vars.last_joy_state.latchy = latchy;

    *last_state = current_state;
    *flag |= bit_mask;
    *skip_bits = pad_bit_encoder(pad, buffer, *skip_bits);
  }
}

static void zmv_record(bool pause, unsigned char combos_used, unsigned char slow)
{
  unsigned char flag = 0;
  unsigned char press_buf[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  size_t skip_bits = 0;

  static float average = 0.0f;
  if (!average) { average = (float)zmv_vars.header.average_fps/((romispal) ? 250.0f : 240.0f); }

  if (pause) { zmv_vars.header.incr_frames++; }

  zmv_vars.header.key_combos += combos_used;

  debug_input;

  record_pad(1, &flag, press_buf, &skip_bits);
  record_pad(2, &flag, press_buf, &skip_bits);
  record_pad(3, &flag, press_buf, &skip_bits);
  record_pad(4, &flag, press_buf, &skip_bits);
  record_pad(5, &flag, press_buf, &skip_bits);

  if (flag)
  {
    unsigned char buffer_used = (skip_bits>>3) + ((skip_bits&7) ? 1 : 0);

    zmv_rle_flush();

    zmv_vars.write_buffer[zmv_vars.write_buffer_loc++] = flag;
    memcpy(zmv_vars.write_buffer+zmv_vars.write_buffer_loc, press_buf, buffer_used);
    zmv_vars.write_buffer_loc += buffer_used;

    flush_input_if_needed();
  }
  else
  {
    zmv_vars.rle_count++;
  }

  // it's the right formula, don't waste time busting your brain on it
  average = (average * (zmv_vars.header.frames + zmv_vars.header.removed_frames) + 1.0f/(float)(slow+1)) /
            (float)(zmv_vars.header.frames + zmv_vars.header.removed_frames + 1);
  // 1/4 precision for NTSC, 1/5 precision for PAL
  zmv_vars.header.average_fps = (unsigned char)(average * ((romispal) ? 250 : 240));
  zmv_vars.header.frames++;
}

static bool zmv_insert_chapter()
{
  if ((zmv_vars.header.internal_chapters < 65535) && zmv_vars.header.frames &&
      (zmv_vars.last_internal_chapter_offset != (ftell(zmv_vars.fp) +
       zmv_vars.write_buffer_loc - INT_CHAP_SIZE(zmv_vars.last_internal_chapter_offset))))
  {
    unsigned char flag = BIT(2);

    flush_input_buffer();

    fwrite(&flag, 1, 1, zmv_vars.fp);

    internal_chapter_add_offset(&zmv_vars.internal_chapters, ftell(zmv_vars.fp));
    zmv_vars.header.internal_chapters++;
    zmv_vars.last_internal_chapter_offset = ftell(zmv_vars.fp);

    zst_save(zmv_vars.fp, false, true);
    fwrite4(zmv_vars.header.frames, zmv_vars.fp);
    write_last_joy_state(zmv_vars.fp);

    return(true);
  }
  return(false);
}

static void zmv_record_finish()
{
  flush_input_buffer();

  internal_chapter_write(&zmv_vars.internal_chapters, zmv_vars.fp);
  internal_chapter_free_chain(zmv_vars.internal_chapters.next);

  free(zmv_vars.filename);

  fwrite2(0, zmv_vars.fp); //External chapter count

  rewind(zmv_vars.fp);
  zmv_header_write(&zmv_vars.header, zmv_vars.fp);

  fclose(zmv_vars.fp);

  debug_input_used;
}

static size_t zmv_frames_recorded()
{
  return(zmv_vars.header.frames);
}

/*

Open and replay ZMV

*/

typedef struct internal_chapter_buf external_chapter_buf;

static struct
{
  external_chapter_buf external_chapters;
  unsigned short external_chapter_count;
  unsigned int frames_replayed;
  size_t last_chapter_frame;
  size_t first_chapter_pos;
  size_t input_start_pos;
} zmv_open_vars; //Additional vars for open/replay of a ZMV

static bool zmv_open(char *filename)
{
  memset(&zmv_vars, 0, sizeof(zmv_vars));
  memset(&zmv_open_vars, 0, sizeof(zmv_open_vars));

  zmv_vars.fp = fopen(filename,"r+b");
  if (zmv_vars.fp && zmv_header_read(&zmv_vars.header, zmv_vars.fp) &&
      !strncmp(zmv_vars.header.magic, "ZMV", 3))
  {
    unsigned short i;
    size_t filename_len = strlen(filename);

    if (zmv_vars.header.rom_crc32 != CRC32)
    {

    }

    MovieStartMethod = (unsigned char)zmv_vars.header.zmv_flag.start_method;
    zmv_vars.inputs_enabled = zmv_vars.header.initial_input;
    zmv_open_vars.first_chapter_pos = ftell(zmv_vars.fp);

    if (zmv_vars.header.zsnes_version != (versionNumber & 0xFFFF))
    {
      zst_compressed_loader(zmv_vars.fp);
      Msgptr = "MOVIE VERSION MISMATCH - MAY DESYNC.";
    }
    else
    {
      switch (zmv_vars.header.zmv_flag.start_method)
      {
        case zmv_sm_zst:
          zst_compressed_loader(zmv_vars.fp);
          break;
        case zmv_sm_power:
          MovieWaiting = true;
          powercycle(false);
          zst_sram_load_compressed(zmv_vars.fp);
          break;
        case zmv_sm_reset:
          GUIReset = 1;
          asm_call(GUIDoReset);
          ReturnFromSPCStall = 0;
          zst_sram_load_compressed(zmv_vars.fp);
          break;
        case zmv_sm_clear_all:
          MovieWaiting = true;
          powercycle(false);
          fseek(zmv_vars.fp, internal_chapter_length(ftell(zmv_vars.fp)), SEEK_CUR);
          break;
      }

      zmv_open_vars.input_start_pos = ftell(zmv_vars.fp);
      Msgptr = "MOVIE STARTED.";
    }

    fseek(zmv_vars.fp, -((signed)EXT_CHAP_COUNT_END_DIST), SEEK_END);
    zmv_open_vars.external_chapter_count = fread2(zmv_vars.fp);

    fseek(zmv_vars.fp, -((signed)INT_CHAP_END_DIST), SEEK_END);

    internal_chapter_read(&zmv_vars.internal_chapters, zmv_vars.fp, zmv_vars.header.internal_chapters);

    for (i = 0; i < zmv_open_vars.external_chapter_count; i++)
    {
      //Seek to 4 bytes before end of chapter, since last 4 bytes is where it contains offset value
      fseek(zmv_vars.fp, EXT_CHAP_SIZE-4, SEEK_CUR);
      internal_chapter_add_offset(&zmv_open_vars.external_chapters, fread4(zmv_vars.fp));
    }

    fseek(zmv_vars.fp, zmv_open_vars.input_start_pos, SEEK_SET);

    zmv_vars.filename = (char *)malloc(filename_len+1); //+1 for null
    strcpy(zmv_vars.filename, filename);

    debug_input_start;

    return(true);
  }
  return(false);
}

static bool zmv_replay_command(enum zmv_commands command)
{
  switch (command)
  {

    default:
      break;
  }
  return(false);
}

static void replay_pad(unsigned char pad, unsigned char flag, unsigned char *buffer, size_t *skip_bits)
{
  unsigned int *last_state = 0, *current_state = 0;
  unsigned char bit_mask = 0;

  switch (pad)
  {
    case 1:
      last_state = &zmv_vars.last_joy_state.A;
      current_state = &JoyAOrig;
      bit_mask = BIT(7);
      break;

    case 2:
      last_state = &zmv_vars.last_joy_state.B;
      current_state = &JoyBOrig;
      bit_mask = BIT(6);
      break;

    case 3:
      last_state = &zmv_vars.last_joy_state.C;
      current_state = &JoyCOrig;
      bit_mask = BIT(5);
      break;

    case 4:
      last_state = &zmv_vars.last_joy_state.D;
      current_state = &JoyDOrig;
      bit_mask = BIT(4);
      break;

    case 5:
      last_state = &zmv_vars.last_joy_state.E;
      current_state = &JoyEOrig;
      bit_mask = BIT(3);
      break;
  }

  if (flag & bit_mask)
  {
    size_t bits_needed = pad_bit_decoder(pad, buffer, 0);
    size_t leftover_bits = (8 - (*skip_bits&7)) & 7;
    bits_needed -= leftover_bits;

    fread(buffer + (*skip_bits>>3), 1, (bits_needed>>3) + ((bits_needed&7) ? 1 : 0), zmv_vars.fp);
    *skip_bits = pad_bit_decoder(pad, buffer, *skip_bits);
  }
  *current_state = *last_state;
  latchx = zmv_vars.last_joy_state.latchx;
  latchy = zmv_vars.last_joy_state.latchy;
}

static bool zmv_replay()
{
  if (zmv_open_vars.frames_replayed < zmv_vars.header.frames)
  {
    if (zmv_vars.rle_count)
    {
      JoyAOrig = zmv_vars.last_joy_state.A;
      JoyBOrig = zmv_vars.last_joy_state.B;
      JoyCOrig = zmv_vars.last_joy_state.C;
      JoyDOrig = zmv_vars.last_joy_state.D;
      JoyEOrig = zmv_vars.last_joy_state.E;
      latchx = zmv_vars.last_joy_state.latchx;
      latchy = zmv_vars.last_joy_state.latchy;
      zmv_vars.rle_count--;

      debug_input;
    }
    else
    {
      unsigned char flag = 0;
      zmv_vars.rle_count = 0;

      fread(&flag, 1, 1, zmv_vars.fp);

      if (flag & BIT(0)) //Command
      {
        unsigned char command = flag >> 1;
        if (command == zmv_command_reset)
        {
          GUIReset = 1;
          ReturnFromSPCStall = 0;
          return(true);
        }
        if (zmv_replay_command(command))
        {
          return(zmv_replay());
        }
        return(false);
      }

      else if (flag & BIT(1)) //RLE
      {
        zmv_vars.rle_count = fread4(zmv_vars.fp) - zmv_open_vars.frames_replayed;
        return(zmv_replay());
      }

      else if (flag & BIT(2)) //Internal Chapter
      {
        fseek(zmv_vars.fp, INT_CHAP_SIZE(ftell(zmv_vars.fp)), SEEK_CUR);
        return(zmv_replay());
      }

      else
      {
        unsigned char press_buf[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        size_t skip_bits = 0;

        replay_pad(1, flag, press_buf, &skip_bits);
        replay_pad(2, flag, press_buf, &skip_bits);
        replay_pad(3, flag, press_buf, &skip_bits);
        replay_pad(4, flag, press_buf, &skip_bits);
        replay_pad(5, flag, press_buf, &skip_bits);

        debug_input;
      }
    }

    zmv_open_vars.frames_replayed++;
    return(true);
  }

  return(false);
}

static bool zmv_next_chapter()
{
  size_t current_loc = ftell(zmv_vars.fp);

  size_t next_internal = internal_chapter_greater(&zmv_vars.internal_chapters, current_loc);
  size_t next_external = internal_chapter_greater(&zmv_open_vars.external_chapters, current_loc);

  size_t next = 0;

  if (next_internal != current_loc)
  {
    next = next_internal;
  }
  else
  {
    next_internal = ~0;
  }
  if ((next_external != current_loc) && next_external < next_internal)
  {
    next = next_external;
  }

  if (next)
  {
    if (next == next_internal)
    {
      fseek(zmv_vars.fp, next_internal, SEEK_SET);
      zst_compressed_loader(zmv_vars.fp);
      zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
      read_last_joy_state(zmv_vars.fp);
    }
    else
    {
      size_t ext_chapter_loc = EXT_CHAP_END_DIST - internal_chapter_pos(&zmv_open_vars.external_chapters, next)*EXT_CHAP_SIZE;
      fseek(zmv_vars.fp, -((signed)ext_chapter_loc), SEEK_END);
      zst_load(zmv_vars.fp, 0);
      zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
      read_last_joy_state(zmv_vars.fp);

      fseek(zmv_vars.fp, next_external, SEEK_SET);
    }

    zmv_vars.rle_count = 0;
    return(true);
  }

  return(false);
}

//Have playback start movie from beginning
static void zmv_rewind_playback()
{
  fseek(zmv_vars.fp, zmv_open_vars.first_chapter_pos, SEEK_SET);
  zst_compressed_loader(zmv_vars.fp);
  zmv_open_vars.frames_replayed = 0;
  zmv_open_vars.last_chapter_frame = 0;
  zmv_vars.rle_count = 0;
  zmv_vars.inputs_enabled = zmv_vars.header.initial_input;
  memset(&zmv_vars.last_joy_state, 0, sizeof(zmv_vars.last_joy_state));
}

static void zmv_prev_chapter()
{
  size_t current_loc = ftell(zmv_vars.fp);

  size_t prev_internal = internal_chapter_lesser(&zmv_vars.internal_chapters, current_loc);
  size_t prev_external = internal_chapter_lesser(&zmv_open_vars.external_chapters, current_loc);

  size_t prev = 0;

  if (prev_internal != current_loc)
  {
    prev = prev_internal;
  }
  else
  {
    prev_internal = 0;
  }
  if ((prev_external != current_loc) && prev_external > prev_internal)
  {
    prev = prev_external;
  }

  if (!prev)
  {
    zmv_rewind_playback();
    return;
  }

  //Code to go back before the previous chapter if the previous chapter was loaded recently
  if ((zmv_open_vars.frames_replayed - zmv_open_vars.last_chapter_frame) < 5*((unsigned int)EmuSpeed+1))
  { //2.5 seconds NTSC when in 100% speed - altered to make slowmo nicer
    size_t pprev = prev-1;
    size_t pprev_internal = internal_chapter_lesser(&zmv_vars.internal_chapters, pprev);
    size_t pprev_external = internal_chapter_lesser(&zmv_open_vars.external_chapters, pprev);

    if ((pprev_internal == pprev) && (pprev_external == pprev))
    {
      zmv_rewind_playback();
      return;
    }

    if (pprev_internal != pprev)
    {
      prev = prev_internal = pprev_internal;
    }
    else
    {
      pprev_internal = 0;
    }
    if ((pprev_external != pprev) && pprev_external > pprev_internal)
    {
      prev = prev_external = pprev_external;
    }
  }

  if (prev == prev_internal)
  {
    fseek(zmv_vars.fp, prev_internal, SEEK_SET);
    zst_compressed_loader(zmv_vars.fp);
    zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
    read_last_joy_state(zmv_vars.fp);
  }
  else
  {
    size_t ext_chapter_loc = EXT_CHAP_END_DIST - internal_chapter_pos(&zmv_open_vars.external_chapters, prev)*EXT_CHAP_SIZE;
    fseek(zmv_vars.fp, -((signed)ext_chapter_loc), SEEK_END);
    zst_load(zmv_vars.fp, 0);
    zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
    read_last_joy_state(zmv_vars.fp);

    fseek(zmv_vars.fp, prev_external, SEEK_SET);
  }
  zmv_open_vars.last_chapter_frame = zmv_open_vars.frames_replayed;
  zmv_vars.rle_count = 0;
}

//External chapter
static void zmv_add_chapter()
{
  if ((zmv_open_vars.external_chapter_count < 65535) && zmv_open_vars.frames_replayed)
  {
    size_t current_loc = ftell(zmv_vars.fp);

    //Check if there is external here already - should possibly modify this to use
    //intern_chapter_lesser() to see if an internal chapter was made right before this
    //point in the stream
    if ((internal_chapter_pos(&zmv_open_vars.external_chapters, current_loc)) == 0xFFFFFFFF)
    {
      //Check if we have internal right here
      unsigned char flag;
      fread(&flag, 1, 1, zmv_vars.fp);

      if (!(flag & BIT(2)))
      {
        char *author = 0;

        internal_chapter_add_offset(&zmv_open_vars.external_chapters, current_loc);
        zmv_open_vars.external_chapter_count++;

        if (zmv_vars.header.author_len)
        {
          if ((author = (char *)malloc(zmv_vars.header.author_len)))
          {
            fseek(zmv_vars.fp, -(zmv_vars.header.author_len), SEEK_END);
            fread(author, zmv_vars.header.author_len, 1, zmv_vars.fp);
          }
        }

        fseek(zmv_vars.fp, -((signed)EXT_CHAP_COUNT_END_DIST), SEEK_END);
        zst_save(zmv_vars.fp, false, false);
        fwrite4(zmv_open_vars.frames_replayed, zmv_vars.fp);
        write_last_joy_state(zmv_vars.fp);
        fwrite4(current_loc, zmv_vars.fp);

        fwrite2(zmv_open_vars.external_chapter_count, zmv_vars.fp);

        if (author)
        {
          fwrite(author, zmv_vars.header.author_len, 1, zmv_vars.fp);
          free(author);
        }

        fseek(zmv_vars.fp, current_loc, SEEK_SET);
      }
      else //Just skip the internal
      {
        fseek(zmv_vars.fp, INT_CHAP_SIZE(ftell(zmv_vars.fp)), SEEK_CUR);
      }
    }
  }
}

static void zmv_replay_finished()
{
  internal_chapter_free_chain(zmv_vars.internal_chapters.next);
  internal_chapter_free_chain(zmv_open_vars.external_chapters.next);
  free(zmv_vars.filename);
  fclose(zmv_vars.fp);
}

static void zmv_replay_to_record()
{
  internal_chapter_free_chain(zmv_open_vars.external_chapters.next);
  zmv_vars.header.rerecords++;
  zmv_vars.header.removed_frames += zmv_vars.header.frames - zmv_open_vars.frames_replayed;
  zmv_vars.header.frames = zmv_open_vars.frames_replayed;
  zmv_vars.header.internal_chapters = internal_chapter_delete_after(&zmv_vars.internal_chapters, ftell(zmv_vars.fp));
  zmv_vars.last_internal_chapter_offset = internal_chapter_lesser(&zmv_vars.internal_chapters, ~0);

  if (zmv_vars.rle_count)
  {
    fseek(zmv_vars.fp, -4, SEEK_CUR);
    fwrite4(zmv_vars.header.frames, zmv_vars.fp);
    zmv_vars.rle_count = 0;
  }

  ftruncate(fileno(zmv_vars.fp), ftell(zmv_vars.fp));
}

static size_t zmv_frames_replayed()
{
  return(zmv_open_vars.frames_replayed);
}

/*

Rewind related functions and vars

*/

struct zmv_rewind
{
  unsigned char last_joy_state[10];
  size_t file_pos;
  unsigned int frames;
  size_t rle_count;
};


struct zmv_rewind *zmv_rewind_buffer = 0;

static void zmv_alloc_rewind_buffer(unsigned char rewind_states)
{
  zmv_rewind_buffer = (struct zmv_rewind *)malloc(sizeof(struct zmv_rewind)*rewind_states);
}

static void zmv_dealloc_rewind_buffer()
{
  if (zmv_rewind_buffer)
  {
    free(zmv_rewind_buffer);
    zmv_rewind_buffer = 0;
  }
}

void zmv_rewind_save(size_t state, bool playback)
{
  save_last_joy_state(zmv_rewind_buffer[state].last_joy_state);
  zmv_rewind_buffer[state].file_pos = ftell(zmv_vars.fp) + zmv_vars.write_buffer_loc;
  zmv_rewind_buffer[state].frames = playback ? zmv_open_vars.frames_replayed : zmv_vars.header.frames;
  zmv_rewind_buffer[state].rle_count = zmv_vars.rle_count;
}

void zmv_rewind_load(size_t state, bool playback)
{
  size_t file_pos = zmv_rewind_buffer[state].file_pos;
  load_last_joy_state(zmv_rewind_buffer[state].last_joy_state);

  if (playback)
  {
    zmv_open_vars.frames_replayed = zmv_rewind_buffer[state].frames;
    fseek(zmv_vars.fp, file_pos, SEEK_SET);
    zmv_vars.rle_count = zmv_rewind_buffer[state].rle_count;
  }
  else
  {
    flush_input_buffer();

    zmv_vars.header.rerecords++;
    zmv_vars.header.removed_frames += zmv_vars.header.frames - zmv_rewind_buffer[state].frames;
    zmv_vars.header.frames = zmv_rewind_buffer[state].frames;
    zmv_vars.rle_count = zmv_rewind_buffer[state].rle_count;

    fseek(zmv_vars.fp, file_pos, SEEK_SET);
    ftruncate(fileno(zmv_vars.fp), file_pos);

    zmv_vars.header.internal_chapters = internal_chapter_delete_after(&zmv_vars.internal_chapters, file_pos);
    zmv_vars.last_internal_chapter_offset = internal_chapter_lesser(&zmv_vars.internal_chapters, ~0);
  }
}

/*

Save and load MZT

*/

void mzt_chdir()
{
  size_t filename_len = strlen(zmv_vars.filename);
  memcpy(zmv_vars.filename+filename_len-3, "mz", 2);
  if (!isdigit(zmv_vars.filename[filename_len-1]))
  {
    zmv_vars.filename[filename_len-1] = 't';
  }
  chdir(zmv_vars.filename);
}

//Currently this doesn't work right in playback
bool mzt_save(char *statename, bool thumb, bool playback)
{
  size_t filename_len = strlen(zmv_vars.filename);
  struct stat stat_buffer;
  bool mzt_saved = false;

  memcpy(zmv_vars.filename+filename_len-3, "mz", 2);
  if (!isdigit(zmv_vars.filename[filename_len-1]))
  {
    zmv_vars.filename[filename_len-1] = 't';
  }

  if (stat(zmv_vars.filename, &stat_buffer))
  {
    mkdir(zmv_vars.filename);
  }

  if (!chdir(zmv_vars.filename))
  {
    FILE *fp = 0;

    if ((fp = fopen(statename,"wb")))
    {
      char FileExt[3];
      gzFile gzp = 0;
      size_t rewind_point;

      zst_save(fp, thumb, false);
      fclose(fp);

      flush_input_buffer();
      rewind_point = ftell(zmv_vars.fp);
      internal_chapter_write(&zmv_vars.internal_chapters, zmv_vars.fp);

      memcpy(FileExt, statename+filename_len-3, 3);
      memcpy(statename+filename_len-3, "zm", 2);
      if (!isdigit(statename[filename_len-1]))
      {
        statename[filename_len-1] = 'v';
      }

      if ((gzp = gzopen(statename, "wb9")))
      {
        rewind(zmv_vars.fp);
        zmv_header_write(&zmv_vars.header, zmv_vars.fp);
        rewind(zmv_vars.fp);

        while (!feof(zmv_vars.fp))
        {
          size_t amount_read = fread(zmv_vars.write_buffer, 1, WRITE_BUFFER_SIZE, zmv_vars.fp);
          gzwrite(gzp, zmv_vars.write_buffer, amount_read);
        }
        gzclose(gzp);

        memcpy(statename+filename_len-3, "mz", 2);
        if (!isdigit(statename[filename_len-1]))
        {
          statename[filename_len-1] = 'i';
        }

        if ((fp = fopen(statename,"wb")))
        {
          fwrite4((playback) ? zmv_open_vars.frames_replayed : zmv_vars.header.frames, fp);
          write_last_joy_state(fp);
          fwrite4(rewind_point, fp);
          fclose(fp);

          mzt_saved = true;
        }

        fseek(zmv_vars.fp, rewind_point, SEEK_SET);
      }
      memcpy(statename+filename_len-3, FileExt, 3);
    }
    chdir("..");
  }
  return(mzt_saved);
}

bool mzt_load(char *statename, bool playback)
{
  size_t filename_len = strlen(zmv_vars.filename);
  bool mzt_saved = false;

  memcpy(zmv_vars.filename+filename_len-3, "mz", 2);
  if (!isdigit(zmv_vars.filename[filename_len-1]))
  {
    zmv_vars.filename[filename_len-1] = 't';
  }

  if (!chdir(zmv_vars.filename))
  {
    FILE *fp = 0;

    if ((fp = fopen(statename,"rb")))
    {
      char FileExt[3];

      zst_load(fp, 0);
      fclose(fp);

      memcpy(FileExt, statename+filename_len-3, 3);
      memcpy(statename+filename_len-3, "mz", 2);
      if (!isdigit(statename[filename_len-1]))
      {
        statename[filename_len-1] = 'i';
      }

      if ((fp = fopen(statename,"rb")))
      {
        size_t rewind_point;

        size_t current_frame = fread4(fp);
        read_last_joy_state(fp);
        rewind_point = fread4(fp);
        fclose(fp);

        zmv_vars.rle_count = 0;

        if (!playback)
        {
          gzFile gzp = 0;

          memcpy(statename+filename_len-3, "zm", 2);
          if (!isdigit(statename[filename_len-1]))
          {
            statename[filename_len-1] = 'v';
          }

          if ((gzp = gzopen(statename, "rb")))
          {
            size_t rerecords = zmv_vars.header.rerecords+1;
            size_t removed_frames = zmv_vars.header.removed_frames + (zmv_vars.header.frames - current_frame);

            internal_chapter_free_chain(zmv_vars.internal_chapters.next);
            memset(&zmv_vars.internal_chapters, 0, sizeof(struct internal_chapter_buf));

            rewind(zmv_vars.fp);
            while (!gzeof(gzp))
            {
              size_t amount_read = gzread(gzp, zmv_vars.write_buffer, WRITE_BUFFER_SIZE);
              fwrite(zmv_vars.write_buffer, 1, amount_read, zmv_vars.fp);
            }
            gzclose(gzp);

            rewind(zmv_vars.fp);
            zmv_header_read(&zmv_vars.header, zmv_vars.fp);
            zmv_vars.header.removed_frames = removed_frames;
            zmv_vars.header.rerecords = rerecords;
            zmv_vars.write_buffer_loc = 0;

            fseek(zmv_vars.fp, rewind_point, SEEK_SET);
            internal_chapter_read(&zmv_vars.internal_chapters, zmv_vars.fp, zmv_vars.header.internal_chapters);

            fseek(zmv_vars.fp, rewind_point, SEEK_SET);
            zmv_vars.last_internal_chapter_offset = internal_chapter_lesser(&zmv_vars.internal_chapters, ~0);
            ftruncate(fileno(zmv_vars.fp), ftell(zmv_vars.fp));
          }
        }
        else
        {
          zmv_open_vars.frames_replayed = current_frame;
          fseek(zmv_vars.fp, rewind_point, SEEK_SET);
        }

        mzt_saved = true;
      }
      memcpy(statename+filename_len-3, FileExt, 3);
    }
    chdir("..");
  }
  return(mzt_saved);
}

/////////////////////////////////////////////////////////

/*

Code for dumping raw video

*/

#define RAW_BUFFER_FRAMES 10
#define RAW_BUFFER_SAMPLES 12800
#define RAW_WIDTH 256
#define RAW_HEIGHT 224
#define RAW_PIXEL_SIZE 4
#define RAW_FRAME_SIZE (RAW_WIDTH*RAW_HEIGHT)
#define RAW_PIXEL_FRAME_SIZE (RAW_FRAME_SIZE*RAW_PIXEL_SIZE)

//NTSC FPS is  59.948743718592964824120603015060 in AVI that's a fraction of 59649/995
//which equals 59.948743718592964824120603015075, so videos should not desync for several millinium

//FPS = 64000 / Samples per Frame

//These two numbers help with calculating how many samples are needed per frame
//59.948743718592964824120603015060 = SAMPLE_NTSC_HI*32000/SAMPLE_NTSC_LO *2
#define SAMPLE_NTSC_HI 31840000ULL
#define SAMPLE_NTSC_LO 59649ULL


//0 = None; 1 Logging, but not now, 2 Log now
unsigned char AudioLogging;

struct
{
  FILE *vp;
  unsigned int *frame_buffer;
  size_t frame_index;

  FILE *ap;
  unsigned short *sample_buffer;
  size_t sample_index;

  size_t aud_dsize_pos;
  //These next three are only used for NTSC videos to keep sample rate correct
  uint64 sample_ntsc_hi;
  uint64 sample_ntsc_lo;
  uint64 sample_ntsc_balance;
} raw_vid;

static void raw_video_close()
{
  if (raw_vid.vp)
  {
    if (raw_vid.frame_buffer)
    {
      if (raw_vid.frame_index)
      {
        fwrite(raw_vid.frame_buffer, RAW_PIXEL_FRAME_SIZE, raw_vid.frame_index, raw_vid.vp);
      }

      free(raw_vid.frame_buffer);
    }

    fclose(raw_vid.vp);
    raw_vid.vp = 0;

    if (raw_vid.ap)
    {
      if (raw_vid.sample_buffer)
      {
        size_t file_size;
        if (raw_vid.sample_index)
        {
          fwrite(raw_vid.sample_buffer, 2, raw_vid.frame_index, raw_vid.ap); //Sample is 2 bytes
        }

        free(raw_vid.sample_buffer);

        file_size = ftell(raw_vid.ap);                            //Get file size
        if (!fseek(raw_vid.ap, 4, SEEK_SET))                      //Seek to after RIFF header
        {
          fwrite4(file_size - 8, raw_vid.ap);                     //Don't include header or this write, -8
        }
        if (!fseek(raw_vid.ap, raw_vid.aud_dsize_pos, SEEK_SET))  //Seek to where the audio data size goes
        {
          //Data size is remainder of file, which is file size, less current position, plus
          //The 4 bytes needed to hold the data size
          fwrite4(file_size - (raw_vid.aud_dsize_pos+4), raw_vid.ap);
        }
        fclose(raw_vid.ap);
        raw_vid.ap = 0;
      }
      AudioLogging = 0;
    }
  }
}

static bool raw_video_open(const char *video_filename, const char *audio_filename)
{
  memset(&raw_vid, 0, sizeof(raw_vid));
  if ((raw_vid.vp = fopen(video_filename, "wb")))
  {
    if ((raw_vid.frame_buffer = (unsigned int *)malloc(RAW_PIXEL_FRAME_SIZE*RAW_BUFFER_FRAMES)))
    {
      if ((raw_vid.ap = fopen(audio_filename, "wb")))
      {
        if ((raw_vid.sample_buffer = (unsigned short *)malloc(RAW_BUFFER_SAMPLES*sizeof(short))))
        {
          fputs("RIFF", raw_vid.ap);                 //header
          fwrite4(~0, raw_vid.ap);                   //file size - unknown till file close
          fputs("WAVEfmt ", raw_vid.ap);             //format
          fwrite4(0x12, raw_vid.ap);                 //fmt size
          fwrite2(1, raw_vid.ap);                    //fmt type (PCM)
          fwrite2(2, raw_vid.ap);                    //channels
          fwrite4(32000, raw_vid.ap);                //sample rate
          fwrite4(32000*4, raw_vid.ap);              //byte rate (sample rate*block align)
          fwrite2(16/8*2, raw_vid.ap);               //block align (SignificantBitsPerSample / 8 * NumChannels)
          fwrite2(16, raw_vid.ap);                   //Significant bits per sample
          fwrite2(0, raw_vid.ap);                    //Extra format bytes
          fputs("data", raw_vid.ap);                 //data header
          raw_vid.aud_dsize_pos = ftell(raw_vid.ap); //Save current position for use later
          fwrite4(~0, raw_vid.ap);                   //data size - unknown till file close

          if (!romispal)
          {
            raw_vid.sample_ntsc_hi = SAMPLE_NTSC_HI;
            raw_vid.sample_ntsc_lo = SAMPLE_NTSC_LO;
            raw_vid.sample_ntsc_balance = SAMPLE_NTSC_HI;
          }

          AudioLogging = 1;
          return(true);
        }
      }
    }
    raw_video_close();
  }
  return(false);
}

//Used by raw videos for calculating sample rate in NTSC mode
//Code by Bisqwit

#define PIXEL (vidbuffer[(i*288) + j + 16])
static void raw_video_write_frame()
{
  if (raw_vid.vp)
  {
    extern unsigned short *vidbuffer;
    size_t i, j;
    unsigned int *pixel_dest = raw_vid.frame_buffer + raw_vid.frame_index*RAW_FRAME_SIZE;

    //Use zsKnight's 16 to 32 bit color conversion (optimized by Nach)
    for (i = 0; i < RAW_HEIGHT; i++)
    {
      unsigned int *scanline = pixel_dest + i*RAW_WIDTH;
      for (j = 0; j < RAW_WIDTH; j++)
      {
        scanline[j] = ((PIXEL&0xF800) << 8) | ((PIXEL&0x07E0) << 5) |
                      ((PIXEL&0x001F) << 3) | 0xFF000000;
      }
    }

    raw_vid.frame_index++;

    if (raw_vid.frame_index == RAW_BUFFER_FRAMES)
    {
      fwrite(raw_vid.frame_buffer, RAW_PIXEL_FRAME_SIZE, RAW_BUFFER_FRAMES, raw_vid.vp);
      raw_vid.frame_index = 0;
    }
  }

  if (raw_vid.ap)
  {
    void ProcessSoundBuffer();
    extern int DSPBuffer[1280];
    extern unsigned int BufferSizeB, BufferSizeW;
    unsigned int i = 0;
    int temp;

    if (romispal)
    {
      BufferSizeB = 1280;
    }
    else
    {
      //Thanks Bisqwit for this algorithm
      BufferSizeB = (raw_vid.sample_ntsc_balance/raw_vid.sample_ntsc_lo) << 1;
      raw_vid.sample_ntsc_balance %= raw_vid.sample_ntsc_lo;
      raw_vid.sample_ntsc_balance += raw_vid.sample_ntsc_hi;
      //printf("Frame %u: %u samples\n", raw_vid.frame_index, BufferSizeB);
    }
    BufferSizeW = BufferSizeB<<1;

    AudioLogging = 2;
    asm_call(ProcessSoundBuffer);
    AudioLogging = 1;

    for (i = 0; i < BufferSizeB; i++)
    {
      temp = DSPBuffer[i];
      if (temp > 32767) { temp = 32767; }
      else if (temp < -32768) { temp =-32768; }
      raw_vid.sample_buffer[raw_vid.sample_index] = temp;

      raw_vid.sample_index++;

      if (raw_vid.sample_index == RAW_BUFFER_SAMPLES)
      {
        fwrite(raw_vid.sample_buffer, 2, RAW_BUFFER_SAMPLES, raw_vid.ap); //Each sample is 2 bytes
        raw_vid.sample_index = 0;
      }
    }
  }
}


/////////////////////////////////////////////////////////

/*
Nach's insane subtitle library for movies files :)

The filename would be gamename.sub in the same directory the ZMV would be in.
If you're playing gamename.zm1, then the sub file will be gamename.su1 etc...

Format of the sub file:
Start Frame:Frame Duration:Message

Example:
1:180:Hi how are you?
300:180:Isn't this cool?
700:180:This is great :)
2500:375:Kill 'em!
3500:20:Did you see this? Of course not
*/

static struct
{
  FILE *fp;
  char linebuf[256];
  size_t message_start;
  size_t message_duration;
} MovieSub;

static void MovieSub_Open(const char *filename)
{
  memset(&MovieSub, 0, sizeof(MovieSub));
  MovieSub.fp = fopen(filename, "r");
}

static void MovieSub_Close()
{
  if (MovieSub.fp)
  {
    fclose(MovieSub.fp);
    MovieSub.fp = 0;
  }
}

static char *MovieSub_GetData(size_t frame_count)
{
  if (MovieSub.fp)
  {
    char *i, *num;

    if (frame_count > MovieSub.message_start + MovieSub.message_duration)
    {
      MovieSub.message_duration = 0;
      do
      {
        if (!fgets(MovieSub.linebuf, 256, MovieSub.fp))
        {
          return(0);
        }
        if (!(num = strtok(MovieSub.linebuf, ":"))) { return(0); }
        for (i = num; *i; i++)
        {
          if (!isdigit(*i)) { return(0); }
        }
        MovieSub.message_start = atoi(num);
      } while(MovieSub.message_start < zmv_frames_replayed());
      if (!(num = strtok(0, ":"))) { return(0); }
      for (i = num; *i; i++)
      {
        if (!isdigit(*i))
        {
          MovieSub.message_start = 0;
          return(0);
        }
      }
      MovieSub.message_duration = atoi(num);
    }

    if (frame_count == MovieSub.message_start)
    {
      return(strtok(0, ":"));
    }
  }
  return(0);
}

static void MovieSub_ResetStream()
{
  if (MovieSub.fp)
  {
    rewind(MovieSub.fp);
    MovieSub.message_start = 0;
    MovieSub.message_duration = 0;
  }
}

static size_t MovieSub_GetDuration()
{
  return(MovieSub.message_duration);
}


/////////////////////////////////////////////////////////

bool RawDumpInProgress = false;
bool PrevSRAMState;

extern unsigned char ComboCounter, MovieRecordWinVal, AllocatedRewindStates;
extern unsigned char SloMo, EMUPause;
char MovieFrameStr[10];
void SRAMChdir();
void ChangetoLOADdir();

/*

Code to playback old ZMVs

*/

struct
{
  FILE *fp;
  size_t frames_replayed;
  struct
  {
    unsigned int A;
    unsigned int B;
    unsigned int C;
    unsigned int D;
    unsigned int E;
  } last_joy_state;
} old_movie;

static void OldMovieReplay()
{
  unsigned char byte;

  if (fread(&byte, 1, 1, old_movie.fp))
  {
    if (byte < 2) // 1 or 0 are correct values
    {
      char *sub;

      if (byte == 0) // 0 means the input has changed
      {
        fread(&old_movie.last_joy_state.A, 1, 4, old_movie.fp);
        fread(&old_movie.last_joy_state.B, 1, 4, old_movie.fp);
        fread(&old_movie.last_joy_state.C, 1, 4, old_movie.fp);
        fread(&old_movie.last_joy_state.D, 1, 4, old_movie.fp);
        fread(&old_movie.last_joy_state.E, 1, 4, old_movie.fp);
      }

      JoyAOrig = old_movie.last_joy_state.A;
      JoyBOrig = old_movie.last_joy_state.B;
      JoyCOrig = old_movie.last_joy_state.C;
      JoyDOrig = old_movie.last_joy_state.D;
      JoyEOrig = old_movie.last_joy_state.E;

      if ((sub = MovieSub_GetData(old_movie.frames_replayed)))
      {
        Msgptr = sub;
        MessageOn = MovieSub_GetDuration();
      }

      if (RawDumpInProgress)
      {
        raw_video_write_frame();
      }

      old_movie.frames_replayed++;
    }
    else // anything else is bad - the file isn't a movie.
    {
      SetMovieMode(MOVIE_OFF);
      MessageOn = 0;
      fclose(old_movie.fp);
      MovieSub_Close();
    }
  }
  else
  {
    if (old_movie.frames_replayed)
    {
      Msgptr = "MOVIE FINISHED.";
    }
    else
    {
      Msgptr = "STATE LOADED.";
    }

    MessageOn = MsgCount;
    SetMovieMode(MOVIE_OFF);

    fclose(old_movie.fp);
    MovieSub_Close();

    if (RawDumpInProgress)
    {
      raw_video_close();
      RawDumpInProgress = false;
    }
  }
}

static void OldMoviePlay(FILE *fp)
{
  unsigned char RecData[16];
  extern unsigned char NextLineCache, soundon, sramsavedis;
  extern size_t Totalbyteloaded;
  extern unsigned int curexecstate;
  extern unsigned int nmiprevaddrl, nmiprevaddrh, nmirept, nmiprevline, nmistatus;
  void loadstate2();

  memset(&old_movie, 0, sizeof(old_movie));
  old_movie.fp = fp;

  loadstate2();

  fseek(fp, Totalbyteloaded, SEEK_SET);
  fread(RecData, 1, 16, fp);
  printf("Movie made with version: %d\n", RecData[1]);

  if (RecData[2] == 1)
  {
    timer2upd = bytes_to_uint32(RecData+3);
    curexecstate = bytes_to_uint32(RecData+7);
    nmiprevaddrl = 0;
    nmiprevaddrh = 0;
    nmirept = 0;
    nmiprevline = 224;
    nmistatus = 0;
    spcnumread = 0;
    NextLineCache = 0;
  }

  if (soundon == RecData[0])
  {
    if (ramsize) { fread(sram, 1, ramsize, fp); }

    SetMovieMode(MOVIE_OLD_PLAY);
    sramsavedis = 1;
    DSPMem[0x08] = 0;
    DSPMem[0x18] = 0;
    DSPMem[0x28] = 0;
    DSPMem[0x38] = 0;
    DSPMem[0x48] = 0;
    DSPMem[0x58] = 0;
    DSPMem[0x68] = 0;
    DSPMem[0x78] = 0;

    Msgptr = "OLD MOVIE REPLAYING.";
  }
  else
  {
    Msgptr = (!soundon) ? "MUST PLAY WITH SOUND ON." : "MUST PLAY WITH SOUND OFF.";
    fclose(fp);
  }
  MessageOn = MsgCount;
}

void MovieInsertChapter()
{
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK: // replaying - external
      zmv_add_chapter();
      Msgptr = "EXTERNAL CHAPTER ADDED.";
      break;
    case MOVIE_RECORD: // recording - internal
      if (zmv_insert_chapter())
      {
        Msgptr = "INTERNAL CHAPTER ADDED.";
      }
      else
      {
        Msgptr = "";
      }
      break;
    case MOVIE_OLD_PLAY:
      Msgptr = "OLD MOVIES DO NOT SUPPORT CHAPTERS.";
      break;
    default:	// no movie processing
      Msgptr = "NO MOVIE PROCESSING.";
  }

  MessageOn = MsgCount;
}

void MovieSeekAhead()
{
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK: // replay seeking ok
      if (zmv_next_chapter()) { Msgptr = "NEXT CHAPTER LOADED."; }
      else { Msgptr = "NO CHAPTERS AHEAD."; }
      break;
    case MOVIE_RECORD: // record will use MZTs
      Msgptr = "NO SEEKING DURING RECORD.";
      break;
    case MOVIE_OLD_PLAY:
      Msgptr = "OLD MOVIES DO NOT SUPPORT CHAPTERS.";
      break;
    default:
      Msgptr = "NO MOVIE PROCESSING.";
  }

  MessageOn = MsgCount;
}

void MovieSeekBehind()
{
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK: // replay seeking ok
      zmv_prev_chapter();
      MovieSub_ResetStream();
      Msgptr = "PREVIOUS CHAPTER LOADED.";
      break;
    case MOVIE_RECORD: // record will use MZTs
      Msgptr = "NO SEEKING DURING RECORD.";
      break;
    case MOVIE_OLD_PLAY:
      Msgptr = "OLD MOVIES DO NOT SUPPORT CHAPTERS.";
      break;
    default:
      Msgptr = "NO MOVIE PROCESSING.";
  }

  MessageOn = MsgCount;
}

void Replay()
{
  if (zmv_replay())
  {
    char *sub;
    if ((sub = MovieSub_GetData(zmv_frames_replayed())))
    {
      Msgptr = sub;
      MessageOn = MovieSub_GetDuration();
    }

    if (RawDumpInProgress)
    {
      raw_video_write_frame();
    }
  }
  else
  {
    if (zmv_frames_replayed())
    {
      Msgptr = "MOVIE FINISHED.";
    }
    else
    {
      Msgptr = "STATE LOADED.";
    }
    MessageOn = MsgCount;
    SetMovieMode(MOVIE_OFF);

    zmv_replay_finished();
    zmv_dealloc_rewind_buffer();
    MovieSub_Close();

    if (RawDumpInProgress)
    {
      raw_video_close();
      RawDumpInProgress = false;
    }

    SRAMState = PrevSRAMState;
  }
}

void ProcessMovies()
{
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK:
      Replay();
      break;
    case MOVIE_RECORD:
      zmv_record(EMUPause ? true : false, ComboCounter, SloMo);
      break;
    case MOVIE_OLD_PLAY:
      OldMovieReplay();
      break;
  }
}

void ResetDuringMovie()
{
  zmv_record_command(zmv_command_reset);
  SetMovieMode(MOVIE_OFF);
  asm_call(GUIDoReset);
  SetMovieMode(MOVIE_RECORD);
}

void SkipMovie()
{
  MovieRecordWinVal = 0;
}

void MovieStop()
{
  if (MovieProcessing && !MovieWaiting)
  {
    switch (MovieProcessing)
    {
      case MOVIE_PLAYBACK:
        zmv_replay_finished();
        MovieSub_Close();
        if (RawDumpInProgress)
        {
          raw_video_close();
          RawDumpInProgress = false;
        }
        MessageOn = 0;
        break;

      case MOVIE_RECORD:
        zmv_record_finish();
        if (!zmv_frames_recorded())
        {
          Msgptr = "STATE SAVED.";
          MessageOn = MsgCount;
        }
        break;
      case MOVIE_OLD_PLAY:
        fclose(old_movie.fp);
        MovieSub_Close();
        if (RawDumpInProgress)
        {
          raw_video_close();
          RawDumpInProgress = false;
        }
        MessageOn = 0;
        break;
    }

    zmv_dealloc_rewind_buffer();
    SetMovieMode(MOVIE_OFF);
    SRAMState = PrevSRAMState;
  }
  MovieWaiting = false;
}

void MoviePlay()
{
  if (!MovieProcessing)
  {
    size_t fname_len = strlen(fnamest+1);
    unsigned char FileExt[4];
    FILE *fp;

    PrevSRAMState = SRAMState;
    SRAMState = true;

    GUIQuit = 2;
    memcpy(FileExt, &fnamest[fname_len-3], 4);
    memcpy(&fnamest[fname_len-3], ".zmv", 4);
    fnamest[fname_len] = CMovieExt;

    SRAMChdir();

    if ((fp = fopen(fnamest+1, "rb")))
    {
      char header_buf[3];
      fread(header_buf, 3, 1, fp);

      if (!strncmp("ZMV", header_buf, 3)) //New Enhanced Format
      {
        fclose(fp);

        if (zmv_open(fnamest+1))
        {
          zmv_alloc_rewind_buffer(AllocatedRewindStates);
          SetMovieMode(MOVIE_PLAYBACK);
          memcpy(&fnamest[fname_len-3], ".sub", 4);
          if (isdigit(CMovieExt)) { fnamest[fname_len] = CMovieExt; }
          MovieSub_Open(fnamest+1);
          MessageOn = MsgCount;
        }
        else
        {
          Msgptr = "MOVIE COULD NOT BE STARTED.";
          MessageOn = MsgCount;
        }
      }
      else //Old Pathetic Format
      {
        OldMoviePlay(fp);
      }
    }
    else
    {
      Msgptr = "MOVIE COULD NOT BE OPENED.";
      MessageOn = MsgCount;
    }

    memcpy(&fnamest[fname_len-3], FileExt, 4);
    asm_call(ChangetoLOADdir);
  }
}

void MovieRecord()
{
  if (MovieProcessing == MOVIE_PLAYBACK)
  {
    zmv_replay_to_record();
    MovieProcessing = 2;
  }

  if (!MovieProcessing)
  {
    size_t fname_len = strlen(fnamest+1);
    unsigned char FileExt[4];
    FILE *tempfhandle;

    memcpy(FileExt, &fnamest[fname_len-3], 4);
    memcpy(&fnamest[fname_len-3], ".zmv", 4);
    fnamest[fname_len] = CMovieExt;

    SRAMChdir();

    if (MovieRecordWinVal == 1)
    {
      remove(fnamest+1);
      MovieRecordWinVal = 0;
    }

    if (!(tempfhandle = fopen(fnamest+1,"rb")))
    {
      PrevSRAMState = SRAMState;
      SRAMState = true;

      SetMovieMode(MOVIE_RECORD);
      zmv_create(fnamest+1);
      zmv_alloc_rewind_buffer(AllocatedRewindStates);
      Msgptr = "MOVIE RECORDING.";
      MessageOn = MsgCount;
    }
    else
    {
      fclose(tempfhandle);
      MovieRecordWinVal = 1;
    }

    asm_call(ChangetoLOADdir);

    memcpy (&fnamest[fname_len-3], FileExt, 4);
  }
}

void GetMovieFrameStr()
{
  *MovieFrameStr = 0;
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK:
      sprintf(MovieFrameStr, "%u",(unsigned int)zmv_frames_replayed());
      break;
    case MOVIE_RECORD:
      sprintf(MovieFrameStr, "%u",(unsigned int)zmv_frames_recorded());
      break;
    case MOVIE_OLD_PLAY:
      sprintf(MovieFrameStr, "%u",(unsigned int)(old_movie.frames_replayed));
      break;
  }
}

void MovieDumpRaw()
{
  switch (MovieProcessing)
  {
    case MOVIE_OFF:
      MoviePlay();
      SRAMChdir();
      RawDumpInProgress = raw_video_open("rawvideo.bin", "pcmaudio.wav");
      asm_call(ChangetoLOADdir);
      break;
  }
}
