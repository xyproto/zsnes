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

#ifdef __LINUX__
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

extern unsigned int versionNumber, CRC32, cur_zst_size;
extern unsigned int JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;
extern unsigned int MsgCount, MessageOn;
extern unsigned char MovieStartMethod, GUIReset, ReturnFromSPCStall, GUIQuit;
extern unsigned char MovieProcessing, *Msgptr, fnamest[512];
extern unsigned char CMovieExt;
extern bool romispal;

void GUIDoReset();
void powercycle(bool);
void zst_sram_load(FILE *);
void zst_save(FILE *, bool);
bool zst_load(FILE *);

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
4 bytes  -  Number of frames with slow down
4 bytes  -  Number of key combos
2 bytes  -  Number of internal chapters
2 bytes  -  Length of author name
3 bytes  -  ZST size
1 byte   -  Flag Byte
  2 bits -   Start from ZST/Power On/Reset/Power On + Clear SRAM
  1 bit  -   NTSC or PAL
  5 bits -   Reserved
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

ZST size -  ZST
4 bytes  -  Frame #
8 bytes  -  Previous input (60 controller bits [12*5] + 4 padded bits)

-Else-

variable - Input

  12 bits per controller where input changed padded to next full byte size
  Minimum 2 bytes (12 controller bits + 4 padded bits)
  Maximum 8 bytes (60 controller bits [12*5] + 4 padded bits)


-----------------------------------------------------------------
Internal chapter offsets  -  Repeated for all internal chapters
-----------------------------------------------------------------

4 bytes  -  Offset to chapter from beginning of file (after input flag byte for ZST)


-----------------------------------------------------------------
External chapters  -  Repeated for all external chapters
-----------------------------------------------------------------

ZST Size -  ZST
4 bytes  -  Frame #
8 bytes  -  Previous input (60 controller bits [12*5] + 4 padded bits)
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

#define INT_CHAP_SIZE (cur_zst_size+4+8)
#define EXT_CHAP_SIZE (cur_zst_size+4+8+4)

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
  unsigned int slow_frames;
  unsigned int key_combos;
  unsigned short internal_chapters;
  unsigned short author_len;
  unsigned int zst_size; //We only read/write 3 bytes for this
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
  fwrite4(zmv_head->slow_frames, fp);
  fwrite4(zmv_head->key_combos, fp);
  fwrite2(zmv_head->internal_chapters, fp);
  fwrite2(zmv_head->author_len, fp);
  fwrite3(zmv_head->zst_size, fp);

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
  zmv_head->slow_frames = fread4(fp);
  zmv_head->key_combos = fread4(fp);
  zmv_head->internal_chapters = fread2(fp);
  zmv_head->author_len = fread2(fp);
  zmv_head->zst_size = fread3(fp);
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
  return((greater == ~0) ? offset : greater);
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

Shared var between record/replay functions

*/

#define WRITE_BUFFER_SIZE 512
static struct
{
  struct zmv_header header;
  FILE *fp;
  struct
  {
    unsigned short A;
    unsigned short B;
    unsigned short C;
    unsigned short D;
    unsigned short E;
  } last_joy_state;
  unsigned char write_buffer[WRITE_BUFFER_SIZE];
  size_t write_buffer_loc;
  struct internal_chapter_buf internal_chapters;
  size_t last_internal_chapter_offset;
  char *filename;
  size_t rle_count;
} zmv_vars;

static void save_last_joy_state(unsigned char *buffer)
{
  buffer[0] = (zmv_vars.last_joy_state.A >> 4) & 0xFF;
  buffer[1] = ((zmv_vars.last_joy_state.A << 4) & 0xF0) | ((zmv_vars.last_joy_state.B >> 8) & 0x0F);
  buffer[2] = zmv_vars.last_joy_state.B & 0xFF;
  buffer[3] = (zmv_vars.last_joy_state.C >> 4) & 0xFF;
  buffer[4] = ((zmv_vars.last_joy_state.C << 4) & 0xF0) | ((zmv_vars.last_joy_state.D >> 8) & 0x0F);
  buffer[5] = zmv_vars.last_joy_state.D & 0xFF;
  buffer[6] = (zmv_vars.last_joy_state.E >> 4) & 0xFF;
  buffer[7] = (zmv_vars.last_joy_state.E << 4) & 0xF0;
}

static void load_last_joy_state(unsigned char *buffer)
{
  zmv_vars.last_joy_state.A = (((unsigned short)(buffer[0])) << 4) | ((buffer[1] & 0xF0) >> 4);
  zmv_vars.last_joy_state.B = (((unsigned short)(buffer[1] & 0x0F)) << 8) | buffer[2];
  zmv_vars.last_joy_state.C = (((unsigned short)(buffer[3])) << 4) | ((buffer[4] & 0xF0) >> 4);
  zmv_vars.last_joy_state.D = (((unsigned short)(buffer[4] & 0x0F)) << 8) | buffer[5];
  zmv_vars.last_joy_state.E = (((unsigned short)(buffer[6])) << 4) | ((buffer[7] & 0xF0) >> 4);
}

static void write_last_joy_state(FILE *fp)
{
  save_last_joy_state(zmv_vars.write_buffer);
  fwrite(zmv_vars.write_buffer, 8, 1, fp);
}

static void read_last_joy_state(FILE *fp)
{
  fread(zmv_vars.write_buffer, 8, 1, fp);
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
    zmv_header_write(&zmv_vars.header, zmv_vars.fp);

    switch (zmv_vars.header.zmv_flag.start_method)
    {
      case zmv_sm_zst:
        break;
      case zmv_sm_power:
        powercycle(true);
        break;
      case zmv_sm_reset:
        GUIReset = 1;
        asm_call(GUIDoReset);
        ReturnFromSPCStall = 0;
        break;
      case zmv_sm_clear_all:
        powercycle(false);
        break;
    }

    zst_save(zmv_vars.fp, false);
    zmv_vars.filename = (char *)malloc(filename_len+1); //+1 for null
    strcpy(zmv_vars.filename, filename);
  }
  else
  {

  }
}

#define RECORD_PAD(prev, cur, bit)                                    \
  if ((unsigned short)(cur >> 20) != prev)                            \
  {                                                                   \
    prev = (unsigned short)(cur >> 20);                               \
    flag |= BIT(bit);                                                 \
                                                                      \
    if (nibble & 1)                                                   \
    {                                                                 \
      press_buf[nibble/2] |= ((unsigned char)(prev & 0x0F)) << 4;     \
      nibble++;                                                       \
      press_buf[nibble/2] = (unsigned char)(prev >> 4);               \
      nibble += 2;                                                    \
    }                                                                 \
    else                                                              \
    {                                                                 \
      press_buf[nibble/2] = (unsigned char)(prev & 0xFF);             \
      nibble += 2;                                                    \
      press_buf[nibble/2] = (unsigned char)(prev >> 8);               \
      nibble++;                                                       \
    }                                                                 \
  }

static void zmv_record(bool slow, unsigned char combos_used)
{
  unsigned char flag = 0;
  unsigned char press_buf[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned char nibble = 0;

  if (slow) { zmv_vars.header.slow_frames++; }

  zmv_vars.header.key_combos += combos_used;

  RECORD_PAD(zmv_vars.last_joy_state.A, JoyAOrig, 7);
  RECORD_PAD(zmv_vars.last_joy_state.B, JoyBOrig, 6);
  RECORD_PAD(zmv_vars.last_joy_state.C, JoyCOrig, 5);
  RECORD_PAD(zmv_vars.last_joy_state.D, JoyDOrig, 4);
  RECORD_PAD(zmv_vars.last_joy_state.E, JoyEOrig, 3);

  if (flag)
  {
    unsigned char buffer_used = (nibble/2) + (nibble&1);

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

    zmv_vars.write_buffer[zmv_vars.write_buffer_loc++] = flag;
    memcpy(zmv_vars.write_buffer+zmv_vars.write_buffer_loc, press_buf, buffer_used);
    zmv_vars.write_buffer_loc += buffer_used;

    if (zmv_vars.write_buffer_loc > WRITE_BUFFER_SIZE - (6+sizeof(press_buf)))
    {
      flush_input_buffer();
    }
  }
  else
  {
    zmv_vars.rle_count++;
  }
  zmv_vars.header.frames++;
}

static bool zmv_insert_chapter()
{
  if ((zmv_vars.header.internal_chapters < 65535) && zmv_vars.header.frames &&
      (zmv_vars.last_internal_chapter_offset != (ftell(zmv_vars.fp) + zmv_vars.write_buffer_loc - INT_CHAP_SIZE)))
  {
    unsigned char flag = BIT(2);

    flush_input_buffer();

    fwrite(&flag, 1, 1, zmv_vars.fp);

    internal_chapter_add_offset(&zmv_vars.internal_chapters, ftell(zmv_vars.fp));
    zmv_vars.header.internal_chapters++;
    zmv_vars.last_internal_chapter_offset = ftell(zmv_vars.fp);

    zst_save(zmv_vars.fp, false);
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

    if (zmv_vars.header.zsnes_version != (versionNumber & 0xFFFF))
    {
      zst_load(zmv_vars.fp);
      Msgptr = "MOVIE VERSION MISMATCH - MAY DESYNC.";
    }
    else
    {
      switch (zmv_vars.header.zmv_flag.start_method)
      {
        case zmv_sm_zst:
          zst_load(zmv_vars.fp);
          break;
        case zmv_sm_power:
          powercycle(false);
          zst_sram_load(zmv_vars.fp);
          break;
        case zmv_sm_reset:
          GUIReset = 1;
          asm_call(GUIDoReset);
          ReturnFromSPCStall = 0;
          zst_sram_load(zmv_vars.fp);
          break;
        case zmv_sm_clear_all:
          powercycle(false);
          fseek(zmv_vars.fp, cur_zst_size, SEEK_CUR);
          break;
      }

      zmv_open_vars.input_start_pos = ftell(zmv_vars.fp);
      Msgptr = "MOVIE STARTED.";
    }

    fseek(zmv_vars.fp, -(EXT_CHAP_COUNT_END_DIST), SEEK_END);
    zmv_open_vars.external_chapter_count = fread2(zmv_vars.fp);

    fseek(zmv_vars.fp, -(INT_CHAP_END_DIST), SEEK_END);

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

    return(true);
  }
  return(false);
}

#define RESTORE_PAD(cur, prev) cur = (((unsigned int)prev) << 20) | 0x8000

#define REPLAY_PAD(prev, cur, bit)                    \
  if (flag & BIT(bit))                                \
  {                                                   \
    if (mid_byte)                                     \
    {                                                 \
      prev = (byte & 0xF0) >> 4;                      \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      prev |= ((unsigned long)byte) << 4;             \
      mid_byte = false;                               \
    }                                                 \
    else                                              \
    {                                                 \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      prev = byte;                                    \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      prev |= ((unsigned long)(byte & 0xF)) << 8;     \
      mid_byte = true;                                \
    }                                                 \
  }                                                   \
  RESTORE_PAD(cur, prev)

static bool zmv_replay()
{
  if (zmv_open_vars.frames_replayed < zmv_vars.header.frames)
  {
    if (zmv_vars.rle_count)
    {
      RESTORE_PAD(JoyAOrig, zmv_vars.last_joy_state.A);
      RESTORE_PAD(JoyBOrig, zmv_vars.last_joy_state.B);
      RESTORE_PAD(JoyCOrig, zmv_vars.last_joy_state.C);
      RESTORE_PAD(JoyDOrig, zmv_vars.last_joy_state.D);
      RESTORE_PAD(JoyEOrig, zmv_vars.last_joy_state.E);
      zmv_vars.rle_count--;
    }
    else
    {
      unsigned char flag = 0;
      unsigned char byte;
      bool mid_byte = false;
      zmv_vars.rle_count = 0;

      fread(&flag, 1, 1, zmv_vars.fp);

      if (flag & BIT(0)) //Command
      {

        return(zmv_replay());
      }

      if (flag & BIT(1)) //RLE
      {
        zmv_vars.rle_count = fread4(zmv_vars.fp) - zmv_open_vars.frames_replayed;
        return(zmv_replay());
      }

      if (flag & BIT(2)) //Internal Chapter
      {
        fseek(zmv_vars.fp, INT_CHAP_SIZE, SEEK_CUR);
        return(zmv_replay());
      }

      REPLAY_PAD(zmv_vars.last_joy_state.A, JoyAOrig, 7);
      REPLAY_PAD(zmv_vars.last_joy_state.B, JoyBOrig, 6);
      REPLAY_PAD(zmv_vars.last_joy_state.C, JoyCOrig, 5);
      REPLAY_PAD(zmv_vars.last_joy_state.D, JoyDOrig, 4);
      REPLAY_PAD(zmv_vars.last_joy_state.E, JoyEOrig, 3);
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
      zst_load(zmv_vars.fp);
      zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
      read_last_joy_state(zmv_vars.fp);
    }
    else
    {
      size_t ext_chapter_loc = EXT_CHAP_END_DIST - internal_chapter_pos(&zmv_open_vars.external_chapters, next)*EXT_CHAP_SIZE;
      fseek(zmv_vars.fp, -(ext_chapter_loc), SEEK_END);
      zst_load(zmv_vars.fp);
      zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
      read_last_joy_state(zmv_vars.fp);

      fseek(zmv_vars.fp, next_external, SEEK_SET);
    }

    zmv_vars.rle_count = 0;
    return(true);
  }

  return(false);
}

static void zmv_rewind_playback()
{
  fseek(zmv_vars.fp, zmv_open_vars.input_start_pos - cur_zst_size, SEEK_SET);
  zst_load(zmv_vars.fp);
  zmv_open_vars.frames_replayed = 0;
  zmv_open_vars.last_chapter_frame = 0;
  zmv_vars.rle_count = 0;
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
  if (zmv_open_vars.frames_replayed - zmv_open_vars.last_chapter_frame < 150) //2.5 seconds NTSC
  {
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
    zst_load(zmv_vars.fp);
    zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
    read_last_joy_state(zmv_vars.fp);
  }
  else
  {
    size_t ext_chapter_loc = EXT_CHAP_END_DIST - internal_chapter_pos(&zmv_open_vars.external_chapters, prev)*EXT_CHAP_SIZE;
    fseek(zmv_vars.fp, -(ext_chapter_loc), SEEK_END);
    zst_load(zmv_vars.fp);
    zmv_open_vars.frames_replayed = fread4(zmv_vars.fp);
    read_last_joy_state(zmv_vars.fp);

    fseek(zmv_vars.fp, prev_external, SEEK_SET);
  }
  zmv_open_vars.last_chapter_frame = zmv_open_vars.frames_replayed;
  zmv_vars.rle_count = 0;
}

static void zmv_add_chapter()
{
  if ((zmv_open_vars.external_chapter_count < 65535) && zmv_open_vars.frames_replayed)
  {
    size_t current_loc = ftell(zmv_vars.fp);

    //Check if previous input contained internal chapter to here, or if there is external here already
    if ((internal_chapter_pos(&zmv_vars.internal_chapters, current_loc-(INT_CHAP_SIZE)) == ~0) &&
        (internal_chapter_pos(&zmv_open_vars.external_chapters, current_loc)) == ~0)
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

        fseek(zmv_vars.fp, -(EXT_CHAP_COUNT_END_DIST), SEEK_END);
        zst_save(zmv_vars.fp, false);
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
        fseek(zmv_vars.fp, INT_CHAP_SIZE, SEEK_CUR);
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
  unsigned char last_joy_state[8];
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

      zst_save(fp, thumb);
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

      zst_load(fp);
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

enum MovieStatus { MOVIE_OFF = 0, MOVIE_PLAYBACK, MOVIE_RECORD, MOVIE_OLD_PLAY };
#define SetMovieMode(mode) (MovieProcessing = (unsigned char)mode)

extern bool SRAMState, SloMo50;
bool PrevSRAMState;
extern unsigned int statefileloc;
extern unsigned char ComboCounter, MovieRecordWinVal, RewindStates;
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
    spchalted = 0xFFFFFFFF;
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
      zmv_record(SloMo50 ? true : false, ComboCounter);
      break;
    case MOVIE_OLD_PLAY:
      OldMovieReplay();
      break;
  }
}

void SkipMovie()
{
  MovieRecordWinVal = 0;
}

void MovieStop()
{
  if (MovieProcessing)
  {
    switch (MovieProcessing)
    {
      case MOVIE_PLAYBACK:
        zmv_replay_finished();
        MovieSub_Close();
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
        MessageOn = 0;
        break;
    }

    zmv_dealloc_rewind_buffer();
    SetMovieMode(MOVIE_OFF);
    SRAMState = PrevSRAMState;
  }
}

void MoviePlay()
{
  if (!MovieProcessing)
  {
    unsigned char FileExt[4];
    FILE *fp;

    PrevSRAMState = SRAMState;
    SRAMState = true;

    GUIQuit = 2;
    memcpy(FileExt, &fnamest[statefileloc-3], 4);
    memcpy(&fnamest[statefileloc-3], ".zmv", 4);
    fnamest[statefileloc] = CMovieExt;

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
          zmv_alloc_rewind_buffer(RewindStates);
          SetMovieMode(MOVIE_PLAYBACK);
          memcpy(&fnamest[statefileloc-3], ".sub", 4);
          if (isdigit(CMovieExt)) { fnamest[statefileloc] = CMovieExt; }
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

    memcpy(&fnamest[statefileloc-3], FileExt, 4);
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
    unsigned char FileExt[4];
    FILE *tempfhandle;

    memcpy(FileExt, &fnamest[statefileloc-3], 4);
    memcpy(&fnamest[statefileloc-3], ".zmv", 4);
    fnamest[statefileloc] = CMovieExt;

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

      zmv_create(fnamest+1);
      zmv_alloc_rewind_buffer(RewindStates);
      SetMovieMode(MOVIE_RECORD);
      Msgptr = "MOVIE RECORDING.";
      MessageOn = MsgCount;
    }
    else
    {
      fclose(tempfhandle);
      MovieRecordWinVal = 1;
    }

    asm_call(ChangetoLOADdir);

    memcpy (&fnamest[statefileloc-3], FileExt, 4);
  }
}

void GetMovieFrameStr()
{
  *MovieFrameStr = 0;
  switch (MovieProcessing)
  {
    case MOVIE_PLAYBACK:
      sprintf(MovieFrameStr, "%u",zmv_frames_replayed());
      break;
    case MOVIE_RECORD:
      sprintf(MovieFrameStr, "%u",zmv_frames_recorded());
      break;
    case MOVIE_OLD_PLAY:
      sprintf(MovieFrameStr, "%u",old_movie.frames_replayed);
      break;
  }
}
