#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

/* The DSP's own rate. Every backend here renders at it and lets the sound
   server resample onward, so it is what the emulator actually outputs and
   what the GUI reports. SoundQuality does not choose it. */
#define AUDIO_OUTPUT_RATE 32000

void InitSampleControl(void);
int InitSound(void);
void DeinitSound(void);

#ifdef __LIBAO__
void SoundWrite_ao(void);
#endif
#ifdef __PIPEWIRE__
void SoundWrite_pipewire(void);
#endif
void SoundWrite_sdl(void);

extern int SoundEnabled;
extern uint8_t PrevStereoSound;
extern uint32_t PrevSoundQuality;
extern bool sound_sdl;
#ifdef __PIPEWIRE__
extern bool sound_pipewire;
#endif

#endif
