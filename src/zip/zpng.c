/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#ifndef NO_PNG
#include <png.h>

#ifdef __UNIXSDL__
#include "../gblhdr.h"
#else
#include <stdlib.h>
#include <io.h>
#endif
#include "../zpath.h"

#ifdef __MSDOS__
#define MAX_PNGNAME_LEN 13
#else
#define MAX_PNGNAME_LEN (strlen(ZCartName)+11) //11 = _12345.png\0
#endif

int Png_Dump(const char *filename, unsigned short width, unsigned short height, unsigned char *image_data, bool usebgr)
{
  FILE *fp = fopen(filename, "wb");
  if (fp)
  {
    //Try to create png write struct, fail if we cannot.
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (png_ptr)
    {
      png_infop info_ptr;

      //set png I/O source.
      png_init_io(png_ptr, fp);

      //set the zlib compression level
      png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

      //set other zlib parameters
      png_set_compression_mem_level(png_ptr, 8);
      png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
      png_set_compression_window_bits(png_ptr, 15);
      png_set_compression_method(png_ptr, 8);
      png_set_compression_buffer_size(png_ptr, 8192);

      //try to create info struct. Fail and delete existing structs if info struct cannot be created.
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr)
      {
        png_bytep *row_pointers;

        //Set scanline width for 32-bit color data
        unsigned int scanline = width*4;
        int png_transforms = 0;

        unsigned int i;

        //set a lot of image info (code adapted from libpng documentation!)
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;

        //Allocate an array of scanline pointers
        row_pointers = (png_bytep*)malloc(height*sizeof(png_bytep));
        for (i = 0; i < height; i++)
        {
          #ifdef __UPSIDE_DOWN__
          //invert to normal image format.
          row_pointers[i] = image_data + scanline*(height-i-1);
          #else
          row_pointers[i] = image_data + scanline*i;
          #endif
        }

        //tell the png library what to encode.
        png_set_rows(png_ptr, info_ptr, row_pointers);

        if (usebgr) { png_transforms|=PNG_TRANSFORM_BGR; }
        //Write image to file
        png_write_png(png_ptr, info_ptr, png_transforms, NULL);
        fclose(fp);

        //Destroy PNG structs
        png_destroy_write_struct(&png_ptr, &info_ptr);

        //clean up dynamically allocated RAM.
        free(row_pointers);

        return(0);
      }
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    }
    fclose(fp);
  }
  return(-1);
}

static char *generate_filename()
{
  char *filename = (char *)malloc(MAX_PNGNAME_LEN);
  if (filename)
  {
    unsigned int i;
#ifdef __MSDOS__
    char *p = filename+3;
    strcpy(filename, "img");
#else
    char *p;
    strcpy(filename, ZCartName);
    p = strrchr(filename, '.');
    if (!p) { p = filename+strlen(filename); }
    *p++ = '_';
#endif
    for (i = 0; i < 100000; i++)
    {
      sprintf(p, "%05d.png", i);
      if (access(filename, F_OK))
      {
        break;
      }
    }
    if (i == 100000)
    {
      free(filename);
      filename = 0;
    }
  }
  return(filename);
}

extern unsigned short *vidbuffer;

#define SNAP_HEIGHT 224
#define SNAP_WIDTH 256
#define PIXEL_SIZE 4
void Grab_PNG_Data()
{
  char *filename = generate_filename();
  if (filename)
  {
    unsigned int *DBits = (unsigned int *)malloc(SNAP_HEIGHT*SNAP_WIDTH*PIXEL_SIZE);
    if (DBits)
    {
      //These are the variables used to perform the 32-bit conversion
      int i,j;
      unsigned short *pixel;
      unsigned short conv_pixel;

      //Use zsKnight's 16 to 32 bit color conversion
      pixel = vidbuffer;
      for (i = 0; i < SNAP_HEIGHT; i++)
      {
        for(j = 0; j < SNAP_WIDTH; j++)
        {
          conv_pixel = pixel[(i*288)+j+16];
          DBits[i*SNAP_WIDTH+j] = ((conv_pixel&0xF800)<<8) | ((conv_pixel&0x07E0)<<5) | ((conv_pixel&0x001F)<<3) | 0xFF000000;
        }
      }

      //compress and write the PNG
      Png_Dump(filename, SNAP_WIDTH, SNAP_HEIGHT, (void *)DBits, true);
      free(DBits);
    }
    free(filename);
  }
}

#endif
