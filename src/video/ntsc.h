
/* SNES NTSC composite video to RGB emulator/blitter */

/* snes_ntsc 0.1.1 */

#ifndef SNES_NTSC_H
#define SNES_NTSC_H

/* Picture parameters, ranging from -1.0 to 1.0 where 0.0 is normal. To easily
clear all fields, make it a static object then set whatever fields you want:
	static snes_ntsc_setup_t setup;
	setup.hue = ... */
typedef struct snes_ntsc_setup_t
{
	float hue;
	float saturation;
	float contrast;
	float brightness;
	float sharpness;
	float gamma;
	float hue_warping; /* < 0 expands purple and green, > 0 expands orange and cyan */
	int merge_fields; /* if 1, merges even and odd fields together to reduce flicker */
	unsigned long const* bsnes_colortbl; /* temporary feature for bsnes only; set to 0 */
} snes_ntsc_setup_t;

/* Initialize and adjust parameters. Can be called multiple times on the same
snes_ntsc_t object. */
struct snes_ntsc_t;
typedef struct snes_ntsc_t snes_ntsc_t;
void snes_ntsc_init( snes_ntsc_t*, snes_ntsc_setup_t const* setup );

/* Blit one or more scanlines of 1-bit BGR pixels to 16-bit 5-6-5 RGB output. For
every 16 output pixels, reads approximately 6 SNES pixels (12 if using hires blit).
Use constants below for definite input and output pixel counts. */
void snes_ntsc_blit( snes_ntsc_t const*, unsigned short const* snes_in, long in_pitch,
		int burst_phase, int out_width, int out_height, void* rgb16_out, long out_pitch );
void snes_ntsc_blit_hires( snes_ntsc_t const*, unsigned short const* snes_in, long in_pitch,
		int burst_phase, int out_width, int out_height, void* rgb16_out, long out_pitch );

/* Useful values to use for output width and number of input pixels read */
enum { snes_ntsc_min_out_width = 600 }; /* minimum width that doesn't cut off active area */
enum { snes_ntsc_min_in_width  = 256 };

/* private */
enum { snes_ntsc_entry_size = 128 };
enum { snes_ntsc_color_count = 0x2000 };
typedef unsigned long ntsc_rgb_t;

/* Caller must allocate space for blitter data, which uses over 4500 KB of memory. */
struct snes_ntsc_t
{
	ntsc_rgb_t table [snes_ntsc_color_count] [snes_ntsc_entry_size];
};

#endif

