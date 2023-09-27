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

#include "../gblhdr.h"
#include "../gblvars.h"
#include <stdbool.h>

#include "../chips/msu1emu.h"
#include "../asm_call.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../netplay/znet.h"
#include "../link.h"

unsigned char *sdl_audio_buffer = 0;
int sdl_audio_buffer_len = 0, sdl_audio_buffer_fill = 0;
int sdl_audio_buffer_head = 0, sdl_audio_buffer_tail = 0;
unsigned char sound_sdl = false;

int SoundEnabled = 1;
int SoundBusy = 0;
unsigned char PrevStereoSound;
unsigned int PrevSoundQuality;
extern int DSPBuffer[1280];
extern unsigned int BufferSizeB, BufferSizeW;

#define SAMPLE_NTSC_HI_SCALE 995ULL
#define SAMPLE_NTSC_LO 59649ULL
#define SAMPLE_PAL_HI_SCALE 1ULL
#define SAMPLE_PAL_LO 50ULL
static const int freqtab[7] = {8000, 11025, 22050, 44100, 16000, 32000, 48000};
#define RATE freqtab[SoundQuality = ((SoundQuality > 6) ? 1 : SoundQuality)]

struct {
	unsigned long long hi;
	unsigned long long lo;
	unsigned long long balance;
} sample_control;

void InitSampleControl() {
	extern unsigned char romispal;
	if (romispal) {
		sample_control.hi = SAMPLE_PAL_HI_SCALE * RATE;
		sample_control.lo = SAMPLE_PAL_LO;
	} else {
		sample_control.hi = SAMPLE_NTSC_HI_SCALE * RATE;
		sample_control.lo = SAMPLE_NTSC_LO;
	}
	sample_control.balance = sample_control.hi;
}

static void SoundUpdate_sdl(void *userdata, unsigned char *stream, int len) {
	BufferSizeB = len / 2;
	BufferSizeW = BufferSizeB + BufferSizeB;

	//normal mixer
	if (soundon && !T36HZEnabled) {
		SoundBusy = 1;
		short *buffer = (short *)stream;
		asm_call(ProcessSoundBuffer);
		if (MSUEnable) { mixMSU1Audio(DSPBuffer, DSPBuffer + BufferSizeB, RATE); }

		//handle audio capping
		int *d = DSPBuffer, *end_d = DSPBuffer + BufferSizeB;
		for (; d < end_d; d++, buffer++) {
			if ((unsigned int)(*d + 0x7fff) < 0xffff) {
				*buffer = *d;
				continue;
			}
			if (*d > 0x7fff) {
				*buffer = 0x7fff;
			} else {
				*buffer = 0x8001;
			}
		}
		SoundBusy = 0;
	} else {
		memset(stream, 0, len); //clear mixer
	}
}

static int SoundInit_sdl() {
	const int samptab[7] = {1, 1, 2, 4, 2, 4, 4};
	SDL_AudioSpec audiospec;
	SDL_AudioSpec wanted;

	SDL_CloseAudio();

	if (sdl_audio_buffer) {
		free(sdl_audio_buffer);
		sdl_audio_buffer = 0;
	}
	sdl_audio_buffer_len = 0;

	wanted.freq = RATE;
	wanted.channels = StereoSound + 1;
	wanted.samples = samptab[SoundQuality] * 32 * wanted.channels;
	wanted.format = AUDIO_S16LSB;
	wanted.userdata = 0;
	wanted.callback = SoundUpdate_sdl;

	if (SDL_OpenAudio(&wanted, &audiospec) < 0) {
		printf("Open audio SDL failed: %s\n", SDL_GetError());
		SoundEnabled = 0;
		return (false);
	}
	SDL_PauseAudio(0);

	sdl_audio_buffer_len = audiospec.size * 2;
	sdl_audio_buffer_len = (sdl_audio_buffer_len + 255) & ~255; // Align to SPCSize
	if (!(sdl_audio_buffer = malloc(sdl_audio_buffer_len))) {
		SDL_CloseAudio();
		printf("Audio buffer SDL failed: %s\n", SDL_GetError());
		SoundEnabled = 0;
		return (false);
	}

	sound_sdl = true;
	printf("SDL audio: %u channels, %u Hz\n", wanted.channels, wanted.freq);
	return (true);
}

int InitSound() {
	sound_sdl = false;
	if (!SoundEnabled) {
		return (false);
	}

	PrevSoundQuality = SoundQuality;
	PrevStereoSound = StereoSound;
	return (SoundInit_sdl());
}

void DeinitSound() {
	while (SoundBusy) {
		;
		;
	}
	SDL_CloseAudio();
	if (sdl_audio_buffer) {
		free(sdl_audio_buffer);
	}
}
