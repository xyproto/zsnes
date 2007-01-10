/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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
#include <stdbool.h>

#ifdef __LIBAO__
#include <ao/ao.h>
#include <pthread.h>
#include <signal.h>
#endif

#include "../asm_call.h"
#include "../cfg.h"

#ifdef __LIBAO__
static pthread_t audio_thread;
static pthread_mutex_t audio_mutex;
static pthread_cond_t audio_wait;
static ao_device *audio_device = 0;
static volatile unsigned int samples_waiting = 0;
#endif

unsigned char *sdl_audio_buffer = 0;
int sdl_audio_buffer_len = 0, sdl_audio_buffer_fill = 0;
int sdl_audio_buffer_head = 0, sdl_audio_buffer_tail = 0;
unsigned char sound_sdl = false;

int SoundEnabled = 1;
unsigned char PrevStereoSound;
unsigned int PrevSoundQuality;

#define SAMPLE_NTSC_HI_SCALE 995ULL
#define SAMPLE_NTSC_LO 59649ULL
#define SAMPLE_PAL_HI_SCALE  1ULL
#define SAMPLE_PAL_LO       50ULL
static const int freqtab[7] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
#define RATE freqtab[SoundQuality = ((SoundQuality > 6) ? 1 : SoundQuality)]

struct
{
  unsigned long long hi;
  unsigned long long lo;
  unsigned long long balance;
} sample_control;

void InitSampleControl()
{
  extern unsigned char romispal;
  if (romispal)
  {
    sample_control.hi = SAMPLE_PAL_HI_SCALE*RATE;
    sample_control.lo = SAMPLE_PAL_LO;
  }
  else
  {
    sample_control.hi = SAMPLE_NTSC_HI_SCALE*RATE;
    sample_control.lo = SAMPLE_NTSC_LO;
  }
  sample_control.balance = sample_control.hi;
}


#ifdef __LIBAO__
static void SoundWriteSamples_ao(unsigned int samples)
{
  extern unsigned int BufferSizeB, BufferSizeW;
  extern int DSPBuffer[1280];
  void ProcessSoundBuffer();
  short stemp[1280];

  int *d = DSPBuffer, *end_d = 0;
  short *p = stemp;

  while (samples > 1280)
  {
    SoundWriteSamples_ao(1280);
    samples -= 1280;
  }

  //printf("samples %d\n", samples);

  BufferSizeB = samples;
  BufferSizeW = samples<<1;

  asm_call(ProcessSoundBuffer);

  end_d = DSPBuffer+samples;
  for (; d < end_d; d++, p++)
  {
    if ((unsigned int)(*d + 0x7FFF) < 0xFFFF) { *p = *d; continue; }
    if (*d > 0x7FFF) { *p = 0x7FFF; }
    else { *p = 0x8001; }
  }

  ao_play(audio_device, (char *)stemp, samples*2);
}

void SoundWrite_ao()
{
  unsigned int samples = 0;

  if (!pthread_mutex_trylock(&audio_mutex))
  {
    if (!samples_waiting && sample_control.lo)
    {
      samples = (unsigned int)((sample_control.balance/sample_control.lo) << StereoSound);
      sample_control.balance %= sample_control.lo;
      sample_control.balance += sample_control.hi;

      samples_waiting = samples;
      pthread_cond_broadcast(&audio_wait); //Send signal
    }
    pthread_mutex_unlock(&audio_mutex);
  }
  else
  {
    pthread_cond_broadcast(&audio_wait); //Send signal
  }
}

static void *SoundThread_ao(void *useless)
{
  unsigned int samples;

  for (;;)
  {
    pthread_mutex_lock(&audio_mutex);

    //The while() is there to prevent error codes from breaking havoc
    while (!samples_waiting)
    {
      pthread_cond_wait(&audio_wait, &audio_mutex); //Wait for signal
    }

    samples = samples_waiting;
    samples_waiting = 0;
    pthread_mutex_unlock(&audio_mutex);

    SoundWriteSamples_ao(samples);
  }
  return(0);
}

static int SoundInit_ao()
{
  int driver_id = ao_driver_id(libAoDriver);
  if (driver_id < 0) { driver_id = ao_default_driver_id(); }

  ao_sample_format driver_format;
  driver_format.bits = 16;
  driver_format.channels = StereoSound+1;
  driver_format.rate = freqtab[SoundQuality = ((SoundQuality > 6) ? 1 : SoundQuality)];
  driver_format.byte_format = AO_FMT_LITTLE;

  if (audio_device)
  {
    ao_close(audio_device);
  }
  else
  {
    if (pthread_create(&audio_thread, 0, SoundThread_ao, 0))
    {
      puts("pthread_create() failed.");
    }
    else if (pthread_mutex_init(&audio_mutex, 0))
    {
      puts("pthread_mutex_init() failed.");
    }
    else if (pthread_cond_init(&audio_wait, 0))
    {
      puts("pthread_cond_init() failed.");
    }
    InitSampleControl();
  }

  //ao_option driver_options = { "buf_size", "32768", 0 };

  audio_device = ao_open_live(driver_id, &driver_format, 0);
  if (audio_device)
  {
    ao_info *di = ao_driver_info(driver_id);
    printf("\nAudio Opened.\nDriver: %s\nChannels: %u\nRate: %u\n\n", di->name, driver_format.channels, driver_format.rate);
  }
  else
  {
    SoundEnabled = 0;
    puts("Audio Open Failed");
    return(false);
  }
  return(true);
}

#endif

void SoundWrite_sdl()
{
  extern int DSPBuffer[];
  extern unsigned char DSPDisable;
  extern unsigned int BufferSizeB, BufferSizeW, T36HZEnabled;

  // Process sound
  BufferSizeB = 256;
  BufferSizeW = BufferSizeB+BufferSizeB;

  // take care of the things we left behind last time
  SDL_LockAudio();
  while (sdl_audio_buffer_fill < sdl_audio_buffer_len)
  {
    short *p = (short*)&sdl_audio_buffer[sdl_audio_buffer_tail];

    if (soundon && !DSPDisable) { asm_call(ProcessSoundBuffer); }

    if (T36HZEnabled)
    {
      memset(p, 0, BufferSizeW);
    }
    else
    {
      int *d = DSPBuffer, *end_d = DSPBuffer+BufferSizeB;

      for (; d < end_d; d++, p++)
      {
        if ((unsigned int)(*d + 0x7fff) < 0xffff) { *p = *d; continue; }
        if (*d > 0x7fff) { *p = 0x7fff; }
        else { *p = 0x8001; }
      }
    }

    sdl_audio_buffer_fill += BufferSizeW;
    sdl_audio_buffer_tail += BufferSizeW;
    if (sdl_audio_buffer_tail >= sdl_audio_buffer_len) { sdl_audio_buffer_tail = 0; }
  }
  SDL_UnlockAudio();
}

static void SoundUpdate_sdl(void *userdata, unsigned char *stream, int len)
{
  int left = sdl_audio_buffer_len - sdl_audio_buffer_head;

  if (left > 0)
  {
    if (left <= len)
    {
      memcpy(stream, &sdl_audio_buffer[sdl_audio_buffer_head], left);
      stream += left;
      len -= left;
      sdl_audio_buffer_head = 0;
      sdl_audio_buffer_fill -= left;
    }

    if (len)
    {
      memcpy(stream, &sdl_audio_buffer[sdl_audio_buffer_head], len);
      sdl_audio_buffer_head += len;
      sdl_audio_buffer_fill -= len;
    }
  }
}

static int SoundInit_sdl()
{
  const int samptab[7] = { 1, 1, 2, 4, 2, 4, 4 };
  SDL_AudioSpec audiospec;
  SDL_AudioSpec wanted;

  SDL_CloseAudio();

  if (sdl_audio_buffer)
  {
    free(sdl_audio_buffer);
    sdl_audio_buffer = 0;
  }
  sdl_audio_buffer_len = 0;

  wanted.freq = RATE;
  wanted.channels = StereoSound+1;
  wanted.samples = samptab[SoundQuality] * 128 * wanted.channels;
  wanted.format = AUDIO_S16LSB;
  wanted.userdata = 0;
  wanted.callback = SoundUpdate_sdl;

  if (SDL_OpenAudio(&wanted, &audiospec) < 0)
  {
    SoundEnabled = 0;
    return(false);
  }
  SDL_PauseAudio(0);

  sdl_audio_buffer_len = audiospec.size*2;
  sdl_audio_buffer_len = (sdl_audio_buffer_len + 255) & ~255; // Align to SPCSize
  if (!(sdl_audio_buffer = malloc(sdl_audio_buffer_len)))
  {
    SDL_CloseAudio();
    puts("Audio Open Failed");
    SoundEnabled = 0;
    return(false);
  }

  sound_sdl = true;
  printf("\nAudio Opened.\nDriver: Simple DirectMedia Layer output\nChannels: %u\nRate: %u\n\n", wanted.channels, wanted.freq);
  return(true);
}


int InitSound()
{
  sound_sdl = false;
  if (!SoundEnabled)
  {
    return(false);
  }

  PrevSoundQuality = SoundQuality;
  PrevStereoSound = StereoSound;

  #ifdef __LIBAO__
  if (strcmp(libAoDriver, "sdl") && !(!strcmp(libAoDriver, "auto") && !strcmp(ao_driver_info(ao_default_driver_id())->name, "null")))
  {
    return(SoundInit_ao());
  }
  #endif
  return(SoundInit_sdl());
}

void DeinitSound()
{
  #ifdef __LIBAO__
  if (audio_device)
  {
    pthread_kill(audio_thread, SIGTERM);
    pthread_mutex_destroy(&audio_mutex);
    pthread_cond_destroy(&audio_wait);
    ao_close(audio_device);
  }
  #endif
  SDL_CloseAudio();
  if (sdl_audio_buffer) { free(sdl_audio_buffer); }
}

