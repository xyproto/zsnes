#ifndef DSP4EMU_H
#define DSP4EMU_H

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef char int8;
typedef short int16;
typedef long int32;
#define FALSE 0
#define TRUE 1


struct DSP4_t
{
  bool8 waiting4command;
  bool8 half_command;
  uint16 command;
  uint32 in_count;
  uint32 in_index;
  uint32 out_count;
  uint32 out_index;
  uint8 parameters[512];
  uint8 output[512];
};

extern struct DSP4_t DSP4;

#endif
