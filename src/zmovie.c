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
#define DIR_SLASH "\\"
#endif
#include "gblvars.h"
#include "asm_call.h"
#include "numconv.h"


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
3500:20:Did you see this?
*/

struct
{
  FILE *fp;
  char linebuf[256];
  size_t frame_current;
  size_t message_start;
  size_t message_duration;
} MovieSub;

void MovieSub_Open(const char *filename)
{
  MovieSub.fp = fopen(filename, "r");
  MovieSub.frame_current = 0;
  MovieSub.message_start = 0;
  MovieSub.message_duration = 0;
}

void MovieSub_Close()
{
  if (MovieSub.fp)
  {
    fclose(MovieSub.fp);
    MovieSub.fp = 0;
  }
}

char *MovieSub_GetData()
{
  if (MovieSub.fp)
  {
    char *i, *num;
    
    MovieSub.frame_current++;
        
    if (MovieSub.frame_current > MovieSub.message_start + MovieSub.message_duration)
    {
      MovieSub.message_duration = 0;
      fgets(MovieSub.linebuf, 256, MovieSub.fp);
      if (!(num = strtok(MovieSub.linebuf, ":"))) { return(0); }
      for (i = num; *i; i++)
      {
        if (!isdigit(*i)) { return(0); }
      }
      MovieSub.message_start = atoi(num);
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
  
    if (MovieSub.frame_current == MovieSub.message_start)
    {
      return(strtok(0, ":"));
    }
  }
  return(0);
}

size_t MovieSub_GetDuration()
{
  return(MovieSub.message_duration);
}


/////////////////////////////////////////////////////////

extern unsigned int versionNumber;
extern unsigned int CRC32;
extern unsigned int cur_zst_size;
extern bool romispal;
extern unsigned int PJoyAOrig, PJoyBOrig, PJoyCOrig, PJoyDOrig, PJoyEOrig;
extern unsigned int JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;


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
20 bytes -  Author name
3 bytes  -  ZST size
1 byte   -  Flag Byte
  2 bits -   Start from ZST/Power On/Reset
  1 bit  -   NTSC or PAL
  5 bits -   Reserved

-If start from ZST-

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
  2 bits -  Reserved

-If Chapter-

ZST size -  ZST
4 bytes  -  Frame #

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
4 bytes  -  Offset to input for current chapter from beginning of file


-----------------------------------------------------------------
External chapter count
-----------------------------------------------------------------

2 bytes  - Number of external chapters

*/


/*

ZMV header types, vars, and functions

*/

enum zmv_start_methods { zmv_sm_zst, zmv_sm_power, zmv_sm_reset };
enum zmv_video_modes { zmv_vm_ntsc, zmv_vm_pal };

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
  char author[20];
  unsigned int zst_size; //We only read/write 3 bytes for this
  struct
  {
    enum zmv_start_methods start_method;
    enum zmv_video_modes video_mode;
  } zmv_flag;
};

void zmv_header_write(struct zmv_header *zmv_head, FILE *fp)
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
  fwrite(zmv_head->author, 20, 1, fp);
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

bool zmv_header_read(struct zmv_header *zmv_head, FILE *fp)
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
  fread(zmv_head->author, 20, 1, fp);
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
      return(false);
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

#define INTERAL_CHAPTER_BUF_LIM 10
struct internal_chapter_buf
{
  size_t offsets[INTERAL_CHAPTER_BUF_LIM];
  unsigned char used;
  struct internal_chapter_buf *next;
};

void interal_chapter_add_offset(struct internal_chapter_buf *icb, size_t offset)
{
  while (icb->next)
  {
    icb = icb->next;
  }
  
  if (icb->used == INTERAL_CHAPTER_BUF_LIM)
  {
    icb->next = (struct internal_chapter_buf *)malloc(sizeof(struct internal_chapter_buf));
    icb = icb->next;
    memset(icb, 0, sizeof(struct internal_chapter_buf));
  }
  
  icb->offsets[icb->used] = offset;
  icb->used++;
}

void internal_chapter_free_chain(struct internal_chapter_buf *icb)
{
  if (icb->next)
  {
    internal_chapter_free_chain(icb->next);
  }
  free(icb);
}

void internal_chapter_write(struct internal_chapter_buf *icb, FILE *fp)
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

void internal_chapter_read(struct internal_chapter_buf *icb, FILE *fp, size_t count)
{
  while (count--)
  {
    interal_chapter_add_offset(icb, fread4(fp));
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
  return(0);
}

/*

Shared var between record/replay functions

*/

#define WRITE_BUFFER_SIZE 512
struct
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
} zmv_vars;

/*

Create and record ZMV

*/

void zmv_create(char *filename)
{
  memset(&zmv_vars, 0, sizeof(zmv_vars));
  zmv_vars.fp = fopen(filename,"wb");
  if (zmv_vars.fp) 
  {
    strncpy(zmv_vars.header.magic, "ZMV", 3);
    zmv_vars.header.zsnes_version = versionNumber & 0xFFFF;
    zmv_vars.header.rom_crc32 = CRC32;
    zmv_vars.header.zst_size = cur_zst_size;
    zmv_vars.header.zmv_flag.start_method = zmv_sm_zst;
    zmv_vars.header.zmv_flag.video_mode = romispal ? zmv_vm_pal : zmv_vm_ntsc;
    zmv_header_write(&zmv_vars.header, zmv_vars.fp);
    zst_save(zmv_vars.fp, false);
  }
  else
  {
  
  }
}

#define RECORD_PAD(prev, cur, bit)                                        \
  if (cur != prev)                                                        \
  {                                                                       \
    prev = cur;                                                           \
    flag |= BIT(bit);                                                     \
                                                                          \
    if (nibble & 1)                                                       \
    {                                                                     \
      press_buf[nibble/2] |= (unsigned char)(prev & 0x0F);                \
      nibble++;                                                           \
      press_buf[nibble/2] = (unsigned char)((prev >> 4) & 0xFF);          \
      nibble += 2;                                                        \
    }                                                                     \
    else                                                                  \
    {                                                                     \
      press_buf[nibble/2] = (unsigned char)(prev & 0xFF);                 \
      nibble += 2;                                                        \
      press_buf[nibble/2] = ((unsigned char)((prev >> 8) & 0x0F)) << 4;   \
      nibble++;                                                           \
    }                                                                     \
  }

void zmv_record()
{
  unsigned char flag = 0;
  unsigned char press_buf[] = { 0, 0, 0, 0, 0, 0, 0, 0 }; 
  unsigned char nibble = 0;
  
  zmv_vars.header.frames++;
  
  RECORD_PAD(zmv_vars.last_joy_state.A, PJoyAOrig, 7);
  RECORD_PAD(zmv_vars.last_joy_state.B, PJoyBOrig, 6);
  RECORD_PAD(zmv_vars.last_joy_state.C, PJoyCOrig, 5);
  RECORD_PAD(zmv_vars.last_joy_state.D, PJoyDOrig, 4);
  RECORD_PAD(zmv_vars.last_joy_state.E, PJoyEOrig, 3);

  zmv_vars.write_buffer[zmv_vars.write_buffer_loc] = flag;
  zmv_vars.write_buffer_loc++;
  
  if (flag)
  {
    unsigned char buffer_used = (nibble/2) + (nibble&1);
    memcpy(zmv_vars.write_buffer+zmv_vars.write_buffer_loc, press_buf, buffer_used);
    zmv_vars.write_buffer_loc += buffer_used;
  }
  
  if (zmv_vars.write_buffer_loc > WRITE_BUFFER_SIZE - (1+sizeof(press_buf)))
  {
    fwrite(zmv_vars.write_buffer, zmv_vars.write_buffer_loc, 1, zmv_vars.fp);
    zmv_vars.write_buffer_loc = 0;
  }
}

void zmv_insert_chapter()
{
  if ((zmv_vars.header.internal_chapters < 65535) && zmv_vars.header.frames &&
      (zmv_vars.last_internal_chapter_offset != ftell(zmv_vars.fp)))
  {
    unsigned char flag = BIT(2);
  
    if (zmv_vars.write_buffer_loc)
    {
      fwrite(zmv_vars.write_buffer, zmv_vars.write_buffer_loc, 1, zmv_vars.fp);
      zmv_vars.write_buffer_loc = 0;  
    }
  
    fwrite(&flag, 1, 1, zmv_vars.fp);
  
    interal_chapter_add_offset(&(zmv_vars.internal_chapters), ftell(zmv_vars.fp));
    zmv_vars.header.internal_chapters++;
    zmv_vars.last_internal_chapter_offset = ftell(zmv_vars.fp);
    
    zst_save(zmv_vars.fp, false);
    fwrite4(zmv_vars.header.frames, zmv_vars.fp);
  }
}

void zmv_record_finish()
{
  if (zmv_vars.write_buffer_loc)
  {
    fwrite(zmv_vars.write_buffer, zmv_vars.write_buffer_loc, 1, zmv_vars.fp);
    zmv_vars.write_buffer_loc = 0;  
  }
  
  internal_chapter_write(&zmv_vars.internal_chapters, zmv_vars.fp);
  internal_chapter_free_chain(zmv_vars.internal_chapters.next);
  
  fwrite2(0, zmv_vars.fp); //External chapter count
  
  rewind(zmv_vars.fp);
  zmv_header_write(&zmv_vars.header, zmv_vars.fp);
  
  fclose(zmv_vars.fp);
}

/*

Open and replay ZMV

*/

typedef struct internal_chapter_buf external_chapter_buf;

struct
{
  external_chapter_buf external_chapters;
  unsigned short external_chapter_count;
  unsigned int frames_replayed;
} zmv_open_vars; //Additional vars for open/replay of a ZMV


bool zmv_open(char *filename)
{
  memset(&zmv_vars, 0, sizeof(zmv_vars));
  memset(&zmv_open_vars, 0, sizeof(zmv_open_vars));
  
  zmv_vars.fp = fopen(filename,"r+b");
  if (zmv_vars.fp && zmv_header_read(&zmv_vars.header, zmv_vars.fp) &&
      !strncpy(zmv_vars.header.magic, "ZMV", 3)) 
  {
    size_t input_start_pos;
    unsigned short i;
    
    if (zmv_vars.header.zsnes_version != (versionNumber & 0xFFFF))
    {
    
    }
    
    if (zmv_vars.header.rom_crc32 != CRC32)
    {
    
    }
    
    zst_load(zmv_vars.fp);
    input_start_pos = ftell(zmv_vars.fp);
    
    fseek(zmv_vars.fp, -2, SEEK_END);
    zmv_open_vars.external_chapter_count = fread2(zmv_vars.fp);
    
    fseek(zmv_vars.fp, -(zmv_open_vars.external_chapter_count*(cur_zst_size+8) + 2), SEEK_END);
    internal_chapter_read(&zmv_vars.internal_chapters, zmv_vars.fp, zmv_vars.header.internal_chapters);
    
    for (i = 0; i < zmv_open_vars.external_chapter_count; i++)
    {
      fseek(zmv_vars.fp, cur_zst_size+4, SEEK_CUR);
      interal_chapter_add_offset(&zmv_open_vars.external_chapters, fread4(zmv_vars.fp));
    }
    
    fseek(zmv_vars.fp, input_start_pos, SEEK_SET);
    
    return(true);
  }
  return(false);
}

#define REPLAY_PAD(cur, bit)                          \
  if (flag & BIT(bit))                                \
  {                                                   \
    if (mid_byte)                                     \
    {                                                 \
      cur = byte & 0x0F;                              \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      cur |= ((unsigned short)byte) << 4;             \
      mid_byte = false;                               \
    }                                                 \
    else                                              \
    {                                                 \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      cur = byte;                                     \
      fread(&byte, 1, 1, zmv_vars.fp);                \
      cur |= ((unsigned short)(byte & 0xF0)) << 4;    \
      mid_byte = true;                                \
    }                                                 \
  }

void zmv_replay()
{
  if (zmv_open_vars.frames_replayed < zmv_vars.header.frames)
  {
    unsigned char flag = 0;
    bool mid_byte = false;
    unsigned char byte;

    fread(&flag, 1, 1, zmv_vars.fp);
  
    if (flag & BIT(2))
    {
      fseek(zmv_vars.fp, cur_zst_size+4, SEEK_CUR);
      fread(&flag, 1, 1, zmv_vars.fp);
    }
  
    REPLAY_PAD(PJoyAOrig, 7);
    REPLAY_PAD(PJoyBOrig, 6);
    REPLAY_PAD(PJoyCOrig, 5);
    REPLAY_PAD(PJoyDOrig, 4);
    REPLAY_PAD(PJoyEOrig, 3);
  
    zmv_open_vars.frames_replayed++;
  }
}

void zmv_next_chapter()
{

}

void zmv_prev_chapter()
{

}

void zmv_add_chapter()
{
  if ((zmv_open_vars.external_chapter_count < 65535) && zmv_open_vars.frames_replayed)
  {
    size_t current_loc = ftell(zmv_vars.fp);
    
    if (!internal_chapter_pos(&zmv_vars.internal_chapters, current_loc-(cur_zst_size+4)) &&
        !internal_chapter_pos(&zmv_open_vars.external_chapters, current_loc))
    {
      unsigned char flag;
      fread(&flag, 1, 1, zmv_vars.fp);
  
      if (!(flag & BIT(2)))
      {
        interal_chapter_add_offset(&zmv_open_vars.external_chapters, current_loc);
        zmv_open_vars.external_chapter_count++;

        fseek(zmv_vars.fp, -2, SEEK_END);
        zst_save(zmv_vars.fp, false);
        fwrite4(zmv_open_vars.frames_replayed, zmv_vars.fp);
        fwrite4(current_loc, zmv_vars.fp);

        fwrite2(zmv_open_vars.external_chapter_count, zmv_vars.fp);
      
        fseek(zmv_vars.fp, current_loc, SEEK_SET); 
      }
      else
      {
        fseek(zmv_vars.fp, cur_zst_size+4, SEEK_CUR);
      }
    }
  }
}

void zmv_replay_finished()
{
  internal_chapter_free_chain(zmv_vars.internal_chapters.next);
  internal_chapter_free_chain(zmv_open_vars.external_chapters.next);  
  fclose(zmv_vars.fp);
}


/////////////////////////////////////////////////////////


extern unsigned int MsgCount, MessageOn;
extern unsigned char MovieTemp, txtmovieended[15], MovieProcessing, *Msgptr;

static FILE *movfhandle;

void Replay()
{
  if (fread(&MovieTemp, 1, 1, movfhandle))
  {
    if (MovieTemp < 2) // 1 or 0 are correct values
    {
      char *sub;
      
      if (MovieTemp == 0) // 0 means the input has changed
      {
	fread(&PJoyAOrig, 1, 4, movfhandle);
	fread(&PJoyBOrig, 1, 4, movfhandle);
	fread(&PJoyCOrig, 1, 4, movfhandle);
	fread(&PJoyDOrig, 1, 4, movfhandle);
	fread(&PJoyEOrig, 1, 4, movfhandle);
      }

      JoyAOrig = PJoyAOrig;
      JoyBOrig = PJoyBOrig;
      JoyCOrig = PJoyCOrig;
      JoyDOrig = PJoyDOrig;
      JoyEOrig = PJoyEOrig;
    
      if ((sub = MovieSub_GetData()))
      {
        Msgptr = sub;
        MessageOn = MovieSub_GetDuration();
      }
    }
    else // anything else is bad - the file isn't a movie.
    {
      MovieProcessing = 0;

      fclose(movfhandle);
      MovieSub_Close();
    }
  }
  else
  {
    Msgptr = txtmovieended;
    MessageOn = MsgCount;
    MovieProcessing = 0;

    fclose(movfhandle);
    MovieSub_Close();
  }
}

extern unsigned int MovieBuffFrame, MovieBuffSize;
unsigned char MovieBuffer[21*60];

void IncFrameWriteBuffer()
{
  MovieBuffFrame++;

  if (MovieBuffFrame == 60)
  {
    fwrite(MovieBuffer, 1, MovieBuffSize, movfhandle);

    MovieBuffSize = 0;
    MovieBuffFrame = 0;
  }
}

void intsplitter (unsigned char *buffer, unsigned int offset, unsigned int value)
{
  unsigned char i;

  for (i=0 ; i<4 ; i++)
  {
    buffer[offset + i] = ((value >> i*8) & 0xFF);
  }
}

unsigned int bytemerger (unsigned char heaviest, unsigned char heavy, unsigned char light, unsigned char lightest)
{
  return ((heaviest << 24) | (heavy << 16) | (light << 8) | (lightest));
}

extern unsigned int CReadHead, ReadHead, CFWriteStart, CFWriteHead;
extern unsigned char BackState, CNetType, StoreBuffer[128*32];

void Record()
{
  unsigned int offst, PJoyATemp, PJoyBTemp, PJoyCTemp, PJoyDTemp, PJoyETemp;

  if ((BackState == 1) && (CNetType >= 20))
  {
    if (CReadHead == ReadHead)
    {
      CFWriteStart++;
      CFWriteStart &= 0x7F;

      if (CFWriteStart == CFWriteHead)
      {
	offst = (CFWriteHead << 5);

	offst++;
	PJoyATemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyBTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyCTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyDTemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;
	PJoyETemp = bytemerger (StoreBuffer[offst+3],StoreBuffer[offst+2],StoreBuffer[offst+1],StoreBuffer[offst]);
	offst+=4;

//	if (StoreBuffer[offst]) - commented out in the ASM.
	if ((PJoyAOrig == PJoyATemp) && (PJoyBOrig == PJoyBTemp) && (PJoyCOrig == PJoyCTemp) && (PJoyDOrig == PJoyDTemp) && (PJoyEOrig == PJoyETemp))
	{
	  MovieBuffer[MovieBuffSize] = 1;
	  MovieBuffSize++;
	}
	else
	{
	  PJoyAOrig = PJoyATemp;
	  PJoyBOrig = PJoyBTemp;
	  PJoyCOrig = PJoyCTemp;
	  PJoyDOrig = PJoyDTemp;
	  PJoyEOrig = PJoyETemp;

	  MovieBuffer[MovieBuffSize] = 0;
	  MovieBuffSize++;

	  intsplitter (MovieBuffer, MovieBuffSize, PJoyAOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyBOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyCOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyDOrig);
	  MovieBuffSize += 4;
	  intsplitter (MovieBuffer, MovieBuffSize, PJoyEOrig);
	  MovieBuffSize += 4;
	}

	IncFrameWriteBuffer();

	CFWriteHead++;
	CFWriteHead &= 0x7F;
      }

      CReadHead++;
      CReadHead &= 0x7F;
    }

    offst = (ReadHead << 5);
    StoreBuffer[offst] = 0;
    offst++;

    intsplitter (StoreBuffer, offst, JoyAOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyBOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyCOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyDOrig);
    offst += 4;
    intsplitter (StoreBuffer, offst, JoyEOrig);
    offst +=4;

    ReadHead++;
    ReadHead &= 0x7F;
  }
  else
  {
    if ((PJoyAOrig != JoyAOrig) || (PJoyBOrig != JoyBOrig) || (PJoyCOrig != JoyCOrig) || (PJoyDOrig != JoyDOrig) || (PJoyEOrig != JoyEOrig))
    {
      PJoyAOrig = JoyAOrig;
      PJoyBOrig = JoyBOrig;
      PJoyCOrig = JoyCOrig;
      PJoyDOrig = JoyDOrig;
      PJoyEOrig = JoyEOrig;
      MovieTemp = 0;
      MovieBuffer[MovieBuffSize] = 0;
      MovieBuffSize++;

      intsplitter (MovieBuffer, MovieBuffSize, JoyAOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyBOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyCOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyDOrig);
      MovieBuffSize += 4;
      intsplitter (MovieBuffer, MovieBuffSize, JoyEOrig);
      MovieBuffSize += 4;
    }
    else
    {
      MovieTemp = 1;
      MovieBuffer[MovieBuffSize] = 1;
      MovieBuffSize++;
    }

    IncFrameWriteBuffer();
  }
}

void ProcessMovies()
{
  if (MovieProcessing == 2)	{ Record(); }
  else	{ Replay(); }
}

// The following will maybe end up in guic.c once we get it started.
// It came from guiwindp.inc and gui.asm, after all
extern unsigned char MovieRecordWinVal;
extern unsigned int GUICBHold;

void SkipMovie() 
{
  MovieRecordWinVal = 0;
  GUICBHold &= 0xFFFFFF00;
}

void MovieStop()
{
  if (MovieProcessing)
  {
    fclose(movfhandle);
    MovieProcessing = 0;
  }
}

extern unsigned int MovieCounter, statefileloc, Totalbyteloaded, curexecstate;
extern unsigned int nmiprevaddrl, nmiprevaddrh, nmirept, nmiprevline, nmistatus;
extern unsigned char GUIQuit, fnamest[512], CMovieExt, RecData[16], soundon;
extern unsigned char NextLineCache, sramsavedis, UseRemoteSRAMData;
extern unsigned char UnableMovie2[24], UnableMovie3[23];

void SRAMChdir();
void loadstate2();
void ChangetoLOADdir();

void MoviePlay()
{
  unsigned char FileExt[4];

  GUICBHold &= 0xFFFFFF00;

  if (CNetType != 20)
  {
    MovieCounter= 0;

    if (!MovieProcessing)
    {
      GUIQuit = 2;
      memcpy (FileExt, &fnamest[statefileloc-3], 4);
      memcpy (&fnamest[statefileloc-3], ".zmv", 4);
      fnamest[statefileloc] = CMovieExt;

      SRAMChdir();
      loadstate2();

      if ((movfhandle = fopen(fnamest+1,"rb")) != NULL)
      {
	memcpy(&fnamest[statefileloc-3], ".sub", 4);
        if (isdigit(CMovieExt)) { fnamest[statefileloc] = CMovieExt; }
	MovieSub_Open(fnamest+1);
	
	fseek(movfhandle, Totalbyteloaded, SEEK_SET);
	fread(RecData, 1, 16, movfhandle);
	printf("Movie made with version: %d\n", RecData[1]);

	if (RecData[2] == 1)
	{
	  timer2upd = bytemerger(RecData[6], RecData[5], RecData[4], RecData[3]);
	  curexecstate = bytemerger(RecData[10], RecData[9], RecData[9], RecData[7]);
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
	  if (ramsize)	{ fread(sram, 1, ramsize, movfhandle); }

	  MovieProcessing = 1;
	  PJoyAOrig = 0;
	  PJoyBOrig = 0;
	  PJoyCOrig = 0;
	  PJoyDOrig = 0;
	  PJoyEOrig = 0;
	  sramsavedis = 1;
	  UseRemoteSRAMData = 0;
	  DSPMem[0x08] = 0;
	  DSPMem[0x18] = 0;
	  DSPMem[0x28] = 0;
	  DSPMem[0x38] = 0;
	  DSPMem[0x48] = 0;
	  DSPMem[0x58] = 0;
	  DSPMem[0x68] = 0;
	  DSPMem[0x78] = 0;
	}
	else
	{
	  Msgptr = (!soundon) ? UnableMovie3 : UnableMovie2;
	  MessageOn = MsgCount;
	  fclose(movfhandle);
	}
      }

      memcpy (&fnamest[statefileloc-3], FileExt, 4);
      asm_call(ChangetoLOADdir);
    }
  }
}

extern unsigned char NoPictureSave;

void statesaver();

void MovieRecord()
{
  unsigned char FileExt[4];

  FILE *tempfhandle;

  GUICBHold &= 0xFFFFFF00;

  if (!MovieProcessing)
  {
    MovieCounter = 0;
    memcpy (FileExt, &fnamest[statefileloc-3], 4);
    memcpy (&fnamest[statefileloc-3], ".zmv", 4);
    fnamest[statefileloc] = CMovieExt;
    tempfhandle = fopen(fnamest+1,"rb");

	// check if file exists
    if ((MovieRecordWinVal == 1) || (tempfhandle == NULL))
    {
      if (!MovieProcessing)
      {
	CFWriteHead = 0;
	CReadHead = 0;
	ReadHead = 0;
	CFWriteStart = 64;
      }

      MovieRecordWinVal = 0;

      SRAMChdir();

      NoPictureSave = 1;
	// saves the statedata as first chunk of movie
      if (!MovieProcessing)	{ statesaver(); }

      NoPictureSave = 0;
	// it shouldn't fail, but paranoids can add an 'if'...
      movfhandle = fopen(fnamest+1,"r+b");
      fseek(movfhandle, 0, SEEK_END);

      if (!MovieProcessing)
      {
	RecData[0] = soundon;
	RecData[1] = (versionNumber & 0xFF); // valid for versions under 2.56
	RecData[2] = 1;
	intsplitter (RecData, 3, timer2upd);
	intsplitter (RecData, 7, curexecstate);
	fwrite(RecData, 1, 16, movfhandle);

	if (ramsize)	{ fwrite(sram, 1, ramsize, movfhandle); }

	MovieBuffSize = 0;
	MovieBuffFrame = 0;

	if ((CNetType != 20) && (CNetType != 21))
	{
	  nmiprevaddrl = 0;
	  nmiprevaddrh = 0;
	  nmirept = 0;
	  nmiprevline = 224;
	  nmistatus = 0;
	  spcnumread = 0;
	  spchalted = 0xFFFFFFFF;
	  NextLineCache = 0;
	  PJoyAOrig = 0;
	  PJoyBOrig = 0;
	  PJoyCOrig = 0;
	  PJoyDOrig = 0;
	  PJoyEOrig = 0;
	  DSPMem[0x08] = 0;
	  DSPMem[0x18] = 0;
	  DSPMem[0x28] = 0;
	  DSPMem[0x38] = 0;
	  DSPMem[0x48] = 0;
	  DSPMem[0x58] = 0;
	  DSPMem[0x68] = 0;
	  DSPMem[0x78] = 0;
	}
      }

      MovieProcessing = 2;

      asm_call(ChangetoLOADdir);
    }
    else
    {
      fclose(tempfhandle);
      MovieRecordWinVal = 1;
    }

    memcpy (&fnamest[statefileloc-3], FileExt, 4);
  }
}
