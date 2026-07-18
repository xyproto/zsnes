/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#ifndef NO_PNG
#include <png.h>
#endif

#ifdef __UNIXSDL__
#include "../gblhdr.h"
#else
#include <stdlib.h>
#include <string.h>
#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#include "../zpath.h"

#define NUMCONV_FW2
#define NUMCONV_FW3
#define NUMCONV_FW4
#include "../numconv.h"

#define MAX_PNGNAME_LEN (strlen(ZSaveName) + 11) // 11 = _12345.png\0

char* generate_image_filename(const char* image_suffix)
{
    char* filename = (char*)malloc(MAX_PNGNAME_LEN);
    if (filename) {
        unsigned int i;
        char* p;
        strcpy(filename, ZSaveName);
        p = strrchr(filename, '.');
        if (!p) {
            p = filename + strlen(filename);
        }
        *p++ = '_';
        for (i = 0; i < 100000; i++) {
            sprintf(p, "%05d.%s", i, image_suffix);
            if (access_dir(ZSnapPath, filename, F_OK)) {
                break;
            }
        }
        if (i == 100000) {
            free(filename);
            filename = 0;
        }
    }
    return (filename);
}

extern uint8_t* vidbuffer;
extern uint16_t resolutn;

#define SNAP_HEIGHT resolutn
#define SNAP_WIDTH 256
#define PIXEL (((uint16_t*)vidbuffer)[((y + 1) * 288) + x + 16])

#ifndef NO_PNG

#define PIXEL_SIZE 3
int Png_Dump(const char* filename, unsigned short width, unsigned short height, unsigned char* image_data, bool usebgr)
{
    FILE* fp = fopen_dir(ZSnapPath, filename, "wb");
    if (fp) {
        // Try to create png write struct, fail if we cannot.
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (png_ptr) {
            png_infop info_ptr;

            // set png I/O source.
            png_init_io(png_ptr, fp);

            // set the zlib compression level
            png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

            // set other zlib parameters
            png_set_compression_mem_level(png_ptr, 8);
            png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
            png_set_compression_window_bits(png_ptr, 15);
            png_set_compression_method(png_ptr, 8);
            png_set_compression_buffer_size(png_ptr, 8192);

            // try to create info struct. Fail and delete existing structs if info struct cannot be created.
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr) {
                png_bytep* row_pointers;

                // Set scanline width for 32-bit color data
                unsigned int scanline = width * PIXEL_SIZE;
                int png_transforms = 0;

                unsigned int i;

                // set a lot of image info (code adapted from libpng documentation!)
                png_set_IHDR(png_ptr, info_ptr, width, height, 8,
                    PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
                // info_ptr->color_type = PNG_COLOR_TYPE_RGB;

                // Allocate an array of scanline pointers
                row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
                for (i = 0; i < height; i++) {
#ifdef __UPSIDE_DOWN__
                    // invert to normal image format.
                    row_pointers[i] = image_data + scanline * (height - i - 1);
#else
                    row_pointers[i] = image_data + scanline * i;
#endif
                }

                // tell the png library what to encode.
                png_set_rows(png_ptr, info_ptr, row_pointers);

                if (usebgr) {
                    png_transforms |= PNG_TRANSFORM_BGR;
                }
                // Write image to file
                png_write_png(png_ptr, info_ptr, png_transforms, NULL);
                fclose(fp);

                // Destroy PNG structs
                png_destroy_write_struct(&png_ptr, &info_ptr);

                // clean up dynamically allocated RAM.
                free(row_pointers);

                return (0);
            }
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        }
        fclose(fp);
    }
    return (-1);
}

void Grab_PNG_Data(void)
{
    char* filename = generate_image_filename("png");
    if (filename) {
        unsigned char* DBits = (unsigned char*)malloc(SNAP_HEIGHT * SNAP_WIDTH * PIXEL_SIZE);
        if (DBits) {
            // These are the variables used to perform the 24-bit conversion
            unsigned int y = SNAP_HEIGHT, x;
            // We can fill the array in any order, so might as well optimize loops
            while (y--) {
                for (x = SNAP_WIDTH; x--;) {
                    DBits[PIXEL_SIZE * (y * SNAP_WIDTH + x)] = (PIXEL & 0xF800) >> 8;
                    DBits[PIXEL_SIZE * (y * SNAP_WIDTH + x) + 1] = (PIXEL & 0x07E0) >> 3;
                    DBits[PIXEL_SIZE * (y * SNAP_WIDTH + x) + 2] = (PIXEL & 0x001F) << 3;
                }
            }
            // compress and write the PNG
            Png_Dump(filename, SNAP_WIDTH, SNAP_HEIGHT, DBits, false);
            free(DBits);
        }
        free(filename);
    }
}

#endif

void Grab_BMP_Data(void)
{
    char* filename = generate_image_filename("bmp");
    if (filename) {
        FILE* fp = fopen_dir(ZSnapPath, filename, "wb");
        if (fp) {
            const unsigned int header_size = 26;
            const unsigned short width = SNAP_WIDTH;
            const unsigned short height = SNAP_HEIGHT;
            unsigned short y = height, x;

            fputs("BM", fp); // Header
            fwrite4(width * height * 3 + header_size, fp); // File size
            fwrite4(0, fp); // Reserved
            fwrite4(header_size, fp); // Offset to bitmap
            fwrite4(12, fp); // Length of color explain field;
            fwrite2(width, fp); // Width
            fwrite2(height, fp); // Height
            fwrite2(1, fp); // Planes
            fwrite2(24, fp); // Bits per pixel

            while (y--) // Have to write image upside down
            {
                for (x = 0; x < width; x++) {
                    fwrite3(((PIXEL & 0xF800) << 8) | ((PIXEL & 0x07E0) << 5) | ((PIXEL & 0x001F) << 3), fp);
                }
            }
            fclose(fp);
        }
        free(filename);
    }
}

// Write current vidbuffer as a 32-wide ASCII art screenshot to an absolute path
void Grab_ASCII_Data_Path(const char* path)
{
    FILE* fp = fopen(path, "wb");
    if (!fp)
        return;

    static const char ramp[] = " .:-=+*#%@";
    const int ramp_n = (int)(sizeof(ramp) - 2);
    const int out_w = 32;
    // Account for character cells being ~2x taller than wide
    const int out_h = (SNAP_HEIGHT * out_w) / (SNAP_WIDTH * 2);

    for (int oy = 0; oy < out_h; oy++) {
        int y0 = (oy * SNAP_HEIGHT) / out_h;
        int y1 = ((oy + 1) * SNAP_HEIGHT) / out_h;
        if (y1 <= y0)
            y1 = y0 + 1;
        for (int ox = 0; ox < out_w; ox++) {
            int x0 = (ox * SNAP_WIDTH) / out_w;
            int x1 = ((ox + 1) * SNAP_WIDTH) / out_w;
            if (x1 <= x0)
                x1 = x0 + 1;
            unsigned long sum = 0;
            unsigned long cnt = 0;
            for (int y = y0; y < y1; y++) {
                for (int x = x0; x < x1; x++) {
                    unsigned short p = PIXEL;
                    unsigned int r = (p & 0xF800) >> 8;
                    unsigned int g = (p & 0x07E0) >> 3;
                    unsigned int b = (p & 0x001F) << 3;
                    sum += (r * 299 + g * 587 + b * 114) / 1000;
                    cnt++;
                }
            }
            unsigned int lum = cnt ? (unsigned int)(sum / cnt) : 0;
            int idx = (lum * ramp_n) / 255;
            if (idx > ramp_n)
                idx = ramp_n;
            fputc(ramp[idx], fp);
        }
        fputc('\n', fp);
    }
    fclose(fp);
}

// Debug: append a hash of the current frame to path, one hex value per line
void Grab_Frame_Hash_Path(const char* path)
{
    FILE* fp = fopen(path, "ab");
    if (!fp)
        return;

    unsigned long hash = 5381;
    for (int y = 0; y < SNAP_HEIGHT; y++) {
        for (int x = 0; x < SNAP_WIDTH; x++) {
            hash = hash * 33 + PIXEL;
        }
    }
    fprintf(fp, "%08lx\n", hash & 0xFFFFFFFF);
    fclose(fp);
}

void Grab_BMP_Data_8(void)
{
    char* filename = generate_image_filename("bmp");
    if (filename) {
        FILE* fp = fopen_dir(ZSnapPath, filename, "wb");
        if (fp) {
            const unsigned int colors = 256;
            const unsigned int palette_size = colors * 4;
            const unsigned int header_size = palette_size + 54;
            const unsigned short width = SNAP_WIDTH;
            const unsigned short height = SNAP_HEIGHT;
            unsigned short y, x;

            fputs("BM", fp); // Header
            fwrite4(width * height + header_size, fp); // File size
            fwrite4(0, fp); // Reserved
            fwrite4(header_size, fp); // Offset to bitmap
            fwrite4(40, fp); // Length of color explain field;
            fwrite4(width, fp); // Width
            fwrite4(height, fp); // Height
            fwrite2(1, fp); // Planes
            fwrite2(8, fp); // Bits per pixel
            fwrite4(0, fp); // Compression Format
            fwrite4(width * height, fp); // Bitmap data size
            fwrite4(0, fp); // H-Res?
            fwrite4(0, fp); // V-Res?
            fwrite4(colors, fp); // Colors
            fwrite4(colors, fp); // Important Colors

            for (y = 0; y < colors; y++) // Write palette
            {
                unsigned char byte = 0;
                fwrite((unsigned char*)vidbuffer + 100000 + y * 3 + 3, 1, 1, fp);
                fwrite((unsigned char*)vidbuffer + 100000 + y * 3 + 2, 1, 1, fp);
                fwrite((unsigned char*)vidbuffer + 100000 + y * 3 + 1, 1, 1, fp);
                fwrite(&byte, 1, 1, fp);
            }

            for (y = height; y--;) // Have to write image upside down
            {
                for (x = 0; x < width; x++) {
                    fwrite((unsigned char*)vidbuffer + (y + 1) * 288 + x + 16, 1, 1, fp);
                }
            }
            fclose(fp);
        }
        free(filename);
    }
}
