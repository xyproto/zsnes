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

#if defined(__LIBAO__) || defined(__PIPEWIRE__)
#include <pthread.h>
#endif
#ifdef __LIBAO__
#include <ao/ao.h>
#include <signal.h>
#endif
#ifdef __PIPEWIRE__
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
#endif

#include "../asm_call.h"
#include "../cfg.h"
#include "../chips/msu1emu.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../gui/gui.h"
#include "../link.h"
#include "../zmovie.h"

#ifdef __LIBAO__
static int terminated = 0;
static pthread_t audio_thread;
static pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t audio_wait = PTHREAD_COND_INITIALIZER;
static ao_device* audio_device = 0;
static volatile unsigned int samples_waiting = 0;
#endif
#ifdef __PIPEWIRE__
static struct pw_thread_loop* pipewire_loop = 0;
static struct pw_stream* pipewire_stream = 0;
static struct spa_audio_info_raw pipewire_info;
static int pipewire_inited = 0;
static uint32_t pipewire_target_frames = 256;
static volatile unsigned char pipewire_shutting_down = 0;
unsigned char sound_pipewire = false;
#endif

unsigned char* sdl_audio_buffer = 0;
int sdl_audio_buffer_len = 0, sdl_audio_buffer_fill = 0;
int sdl_audio_buffer_head = 0, sdl_audio_buffer_tail = 0;
unsigned char sound_sdl = false;

int SoundEnabled = 1;
unsigned char PrevStereoSound;
unsigned int PrevSoundQuality;

#define SAMPLE_NTSC_HI_SCALE 995ULL
#define SAMPLE_NTSC_LO 59649ULL
#define SAMPLE_PAL_HI_SCALE 1ULL
#define SAMPLE_PAL_LO 50ULL
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
    if (romispal) {
        sample_control.hi = SAMPLE_PAL_HI_SCALE * RATE;
        sample_control.lo = SAMPLE_PAL_LO;
    } else {
        sample_control.hi = SAMPLE_NTSC_HI_SCALE * RATE;
        sample_control.lo = SAMPLE_NTSC_LO;
    }
    sample_control.balance = sample_control.hi;
}

static void MixSoundBlock(short* const out, unsigned int const samples)
{
    extern unsigned int BufferSizeB, BufferSizeW;
    extern unsigned char DSPDisable;
    extern int DSPBuffer[1280];
    int *d = DSPBuffer, *end_d;
    short* p = out;

    BufferSizeB = samples;
    BufferSizeW = samples << 1;

    if (soundon && !DSPDisable) {
        asm_call(ProcessSoundBuffer);
        if (MSUEnable) {
            mixMSU1Audio(DSPBuffer, DSPBuffer + BufferSizeB, RATE);
        }
    }

    if (T36HZEnabled || !soundon || DSPDisable) {
        memset(out, 0, BufferSizeW);
        return;
    }

    end_d = DSPBuffer + samples;
    for (; d < end_d; d++, p++) {
        if ((unsigned int)(*d + 0x7FFF) < 0xFFFF) {
            *p = *d;
            continue;
        }
        if (*d > 0x7FFF) {
            *p = 0x7FFF;
        } else {
            *p = 0x8001;
        }
    }
}

#ifdef __LIBAO__
static void SoundWriteSamples_ao(unsigned int samples)
{
    extern unsigned int BufferSizeB, BufferSizeW;
    extern int DSPBuffer[1280];
    short stemp[1280];

    int *d = DSPBuffer, *end_d = 0;
    short* p = stemp;

    if (!audio_device) {
        return;
    }

    while (samples > 1280) {
        SoundWriteSamples_ao(1280);
        samples -= 1280;
    }

    // printf("samples %d\n", samples);

    BufferSizeB = samples;
    BufferSizeW = samples << 1;

    asm_call(ProcessSoundBuffer);

    if (MSUEnable) {
        mixMSU1Audio(DSPBuffer, DSPBuffer + BufferSizeB, RATE);
    }

    end_d = DSPBuffer + samples;
    for (; d < end_d; d++, p++) {
        if ((unsigned int)(*d + 0x7FFF) < 0xFFFF) {
            *p = *d;
            continue;
        }
        if (*d > 0x7FFF) {
            *p = 0x7FFF;
        } else {
            *p = 0x8001;
        }
    }

    ao_play(audio_device, (char*)stemp, samples * 2);
}

void SoundWrite_ao()
{
    unsigned int samples = 0;

    if (!audio_device) {
        return;
    }

    if (!pthread_mutex_trylock(&audio_mutex)) {
        if (!samples_waiting && sample_control.lo) {
            samples = (unsigned int)((sample_control.balance / sample_control.lo) << StereoSound);
            sample_control.balance %= sample_control.lo;
            sample_control.balance += sample_control.hi;

            samples_waiting = samples;
            pthread_cond_broadcast(&audio_wait); // Send signal
        }
        pthread_mutex_unlock(&audio_mutex);
    } else {
        pthread_cond_broadcast(&audio_wait); // Send signal
    }
}

static void* SoundThread_ao(void* useless)
{
    unsigned int samples;

    while (!terminated) {
        pthread_mutex_lock(&audio_mutex);

        // The while() is there to prevent error codes from breaking havoc
        while (!samples_waiting && !terminated) {
            pthread_cond_wait(&audio_wait, &audio_mutex); // Wait for signal
        }

        samples = samples_waiting;
        samples_waiting = 0;
        pthread_mutex_unlock(&audio_mutex);

        if (!terminated && audio_device) {
            SoundWriteSamples_ao(samples);
        }
    }

    return (0);
}

static int SoundInit_ao()
{
    int driver_id = ao_driver_id(libAoDriver);
    if (driver_id < 0) {
        driver_id = ao_default_driver_id();
    }

    ao_sample_format driver_format = { 0 };
    driver_format.bits = 16;
    driver_format.channels = StereoSound + 1;
    driver_format.rate = RATE;
    driver_format.byte_format = AO_FMT_LITTLE;
    driver_format.matrix = NULL;

    if (audio_device) {
        ao_close(audio_device);
    } else {
        if (pthread_create(&audio_thread, 0, SoundThread_ao, 0)) {
            puts("pthread_create() failed.");
        }
        InitSampleControl();
    }

    // ao_option driver_options = { "buf_size", "32768", 0 };

    audio_device = ao_open_live(driver_id, &driver_format, 0);
    if (audio_device) {
        ao_info* di = ao_driver_info(driver_id);
        printf("%s: %u channels, %u Hz\n", di->name, driver_format.channels, driver_format.rate);
    } else {
        SoundEnabled = 0;
        puts("Audio Open Failed");
        return (false);
    }
    return (true);
}

#endif

#ifdef __PIPEWIRE__
static void PipeWireProcess(void* userdata)
{
    struct pw_buffer* b;
    struct spa_buffer* buf;
    struct spa_data* d;
    uint32_t bytes;
    uint32_t stride = (StereoSound + 1) * 2;
    unsigned char* out;
    short stemp[1280];
    bool render_audio;

    (void)userdata;
    if (!pipewire_stream) {
        return;
    }

    if (!(b = pw_stream_dequeue_buffer(pipewire_stream))) {
        return;
    }
    buf = b->buffer;
    d = &buf->datas[0];
    if (!d->data) {
        pw_stream_queue_buffer(pipewire_stream, b);
        return;
    }

    if (b->requested > 0) {
        uint64_t requested_bytes = b->requested * stride;
        if (requested_bytes > d->maxsize) {
            requested_bytes = d->maxsize;
        }
        /* Avoid bursty very-large requests that increase jitter under load. */
        {
            uint64_t max_bytes = (uint64_t)pipewire_target_frames * 4U * stride;
            if (requested_bytes > max_bytes) {
                requested_bytes = max_bytes;
            }
        }
        bytes = (uint32_t)requested_bytes;
    } else {
        bytes = pipewire_target_frames * stride;
        if (bytes > d->maxsize) {
            bytes = d->maxsize;
        }
    }
    bytes -= bytes % stride;
    out = (unsigned char*)d->data;
    render_audio = !pipewire_shutting_down && !GUIOn2 && !GUIOn && !EMUPause && !RawDumpInProgress && !T36HZEnabled && soundon;

    if (!render_audio) {
        memset(out, 0, bytes);
        d->chunk->offset = 0;
        d->chunk->stride = (StereoSound + 1) * 2;
        d->chunk->size = bytes;
        pw_stream_queue_buffer(pipewire_stream, b);
        return;
    }

    while (bytes > 0) {
        unsigned int chunk = bytes >> 1;
        if (chunk > sizeof(stemp) / sizeof(stemp[0])) {
            chunk = (unsigned int)(sizeof(stemp) / sizeof(stemp[0]));
        }
        MixSoundBlock(stemp, chunk);
        memcpy(out, stemp, chunk << 1);
        out += chunk << 1;
        bytes -= chunk << 1;
    }

    d->chunk->offset = 0;
    d->chunk->stride = (StereoSound + 1) * 2;
    d->chunk->size = out - (unsigned char*)d->data;
    pw_stream_queue_buffer(pipewire_stream, b);
}

void SoundWrite_pipewire()
{
    if (!sound_pipewire) {
        return;
    }
}

static int SoundInit_pipewire()
{
    uint8_t buffer[1024];
    struct pw_properties* props;
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    struct spa_pod* params[1];
    static struct pw_stream_events stream_events = {
        PW_VERSION_STREAM_EVENTS,
        .process = PipeWireProcess,
    };
    enum pw_stream_flags const flags = PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS;
    int rc;
    int bytes_per_sample = (StereoSound + 1) * 2;
    char latency_str[64];

    if (!pipewire_inited) {
        pw_init(NULL, NULL);
        pipewire_inited = 1;
    }

    if (!pipewire_loop) {
        pipewire_loop = pw_thread_loop_new("zsnes-pipewire", NULL);
        if (!pipewire_loop) {
            return false;
        }
        if (pw_thread_loop_start(pipewire_loop) < 0) {
            pw_thread_loop_destroy(pipewire_loop);
            pipewire_loop = 0;
            return false;
        }
    }

    pipewire_shutting_down = 0;
    pw_thread_loop_lock(pipewire_loop);
    if (pipewire_stream) {
        pw_stream_set_active(pipewire_stream, false);
        pw_stream_disconnect(pipewire_stream);
        pw_stream_destroy(pipewire_stream);
        pipewire_stream = 0;
    }

    memset(&pipewire_info, 0, sizeof(pipewire_info));
    pipewire_info.format = SPA_AUDIO_FORMAT_S16_LE;
    pipewire_info.rate = RATE;
    pipewire_info.channels = StereoSound + 1;
    if (RATE >= 48000) {
        pipewire_target_frames = 320;
    } else if (RATE >= 44100) {
        pipewire_target_frames = 256;
    } else if (RATE >= 22050) {
        pipewire_target_frames = 128;
    } else {
        pipewire_target_frames = 64;
    }
    if (pipewire_info.channels == 1) {
        pipewire_info.position[0] = SPA_AUDIO_CHANNEL_MONO;
    } else {
        pipewire_info.position[0] = SPA_AUDIO_CHANNEL_FL;
        pipewire_info.position[1] = SPA_AUDIO_CHANNEL_FR;
    }
    snprintf(latency_str, sizeof(latency_str), "%u/%u", pipewire_target_frames, RATE);

    props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        PW_KEY_MEDIA_ROLE, "Game",
        PW_KEY_APP_NAME, "zsnes",
        PW_KEY_NODE_LATENCY, latency_str,
        PW_KEY_NODE_RATE, "1/1",
        NULL);
    pipewire_stream = pw_stream_new_simple(
        pw_thread_loop_get_loop(pipewire_loop),
        "zsnes",
        props,
        &stream_events,
        NULL);
    if (!pipewire_stream) {
        pw_thread_loop_unlock(pipewire_loop);
        return false;
    }

    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &pipewire_info);
    rc = pw_stream_connect(pipewire_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, flags, (const struct spa_pod* const*)params, 1);
    pw_thread_loop_unlock(pipewire_loop);
    if (rc < 0) {
        SoundEnabled = 0;
        return false;
    }

    sound_pipewire = true;
    printf("PipeWire audio: %u channels, %u Hz, target %u frames\n", StereoSound + 1, RATE, pipewire_target_frames);
    return true;
}
#endif

void SoundWrite_sdl()
{
    extern int DSPBuffer[];
    extern unsigned char DSPDisable;
    extern unsigned int BufferSizeB, BufferSizeW;

    // Process sound
    BufferSizeB = 256;
    BufferSizeW = BufferSizeB + BufferSizeB;

    // take care of the things we left behind last time
    SDL_LockAudio();
    while (sdl_audio_buffer_fill < sdl_audio_buffer_len) {
        short* p = (short*)&sdl_audio_buffer[sdl_audio_buffer_tail];

        if (soundon && !DSPDisable) {
            asm_call(ProcessSoundBuffer);
            if (MSUEnable) {
                mixMSU1Audio(DSPBuffer, DSPBuffer + BufferSizeB, RATE);
            }
        }

        if (T36HZEnabled) {
            memset(p, 0, BufferSizeW);
        } else {
            int *d = DSPBuffer, *end_d = DSPBuffer + BufferSizeB;

            for (; d < end_d; d++, p++) {
                if ((unsigned int)(*d + 0x7fff) < 0xffff) {
                    *p = *d;
                    continue;
                }
                if (*d > 0x7fff) {
                    *p = 0x7fff;
                } else {
                    *p = 0x8001;
                }
            }
        }

        sdl_audio_buffer_fill += BufferSizeW;
        sdl_audio_buffer_tail += BufferSizeW;
        if (sdl_audio_buffer_tail >= sdl_audio_buffer_len) {
            sdl_audio_buffer_tail = 0;
        }
    }
    SDL_UnlockAudio();
}

static void SoundUpdate_sdl(void* userdata, unsigned char* stream, int len)
{
    int left = sdl_audio_buffer_len - sdl_audio_buffer_head;

    if (left > 0) {
        if (left <= len) {
            memcpy(stream, &sdl_audio_buffer[sdl_audio_buffer_head], left);
            stream += left;
            len -= left;
            sdl_audio_buffer_head = 0;
            sdl_audio_buffer_fill -= left;
        }

        if (len) {
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

    if (sdl_audio_buffer) {
        free(sdl_audio_buffer);
        sdl_audio_buffer = 0;
    }
    sdl_audio_buffer_len = 0;

    wanted.freq = RATE;
    wanted.channels = StereoSound + 1;
    wanted.samples = samptab[SoundQuality] * 128 * wanted.channels;
    wanted.format = AUDIO_S16LSB;
    wanted.userdata = 0;
    wanted.callback = SoundUpdate_sdl;

    if (SDL_OpenAudio(&wanted, &audiospec) < 0) {
        SoundEnabled = 0;
        return (false);
    }
    SDL_PauseAudio(0);

    sdl_audio_buffer_len = (audiospec.size * 2 + 255) & ~255; // Align to SPCSize
    if (!(sdl_audio_buffer = calloc(1, sdl_audio_buffer_len))) {
        SDL_CloseAudio();
        puts("Audio Open Failed");
        SoundEnabled = 0;
        return (false);
    }

    sound_sdl = true;
    printf("SDL audio: %u channels, %u Hz\n", wanted.channels, wanted.freq);
    return (true);
}

int InitSound()
{
    static unsigned char warned_sdl_fallback = false;
    sound_sdl = false;
#ifdef __PIPEWIRE__
    sound_pipewire = false;
#endif
    if (!SoundEnabled) {
        return (false);
    }

    PrevSoundQuality = SoundQuality;
    PrevStereoSound = StereoSound;

#ifdef __PIPEWIRE__
    if (!strcmp(libAoDriver, "pipewire")) {
        return (SoundInit_pipewire());
    }
    if (!strcmp(libAoDriver, "auto")) {
        if (SoundInit_pipewire()) {
            return (true);
        }
    }
#endif

#ifdef __LIBAO__
    if (strcmp(libAoDriver, "sdl")
#ifdef __PIPEWIRE__
        && strcmp(libAoDriver, "pipewire")
#endif
        && !(!strcmp(libAoDriver, "auto") && !strcmp(ao_driver_info(ao_default_driver_id())->name, "null"))) {
        return (SoundInit_ao());
    }
#endif

    if (!strcmp(libAoDriver, "auto")
#ifndef __PIPEWIRE__
        && true
#else
        && !sound_pipewire
#endif
#ifndef __LIBAO__
        && true
#else
        && !audio_device
#endif
        && !warned_sdl_fallback) {
        warned_sdl_fallback = true;
        puts("WARNING: Falling back to SDL audio (PipeWire/libao unavailable). Audio quality may suffer.");
    }

    return (SoundInit_sdl());
}

void DeinitSound()
{
#ifdef __PIPEWIRE__
    pipewire_shutting_down = 1;
    sound_pipewire = false;
    if (pipewire_stream) {
        pw_thread_loop_lock(pipewire_loop);
        pw_stream_set_active(pipewire_stream, false);
        pw_stream_disconnect(pipewire_stream);
        pw_stream_destroy(pipewire_stream);
        pipewire_stream = 0;
        pw_thread_loop_unlock(pipewire_loop);
    }
    if (pipewire_loop) {
        pw_thread_loop_stop(pipewire_loop);
        pw_thread_loop_destroy(pipewire_loop);
        pipewire_loop = 0;
    }
#endif
#ifdef __LIBAO__
    if (audio_device) {
        void* retval;
        terminated = 1;
        pthread_cond_broadcast(&audio_wait);
        pthread_join(audio_thread, &retval);
        pthread_cond_destroy(&audio_wait);
        pthread_mutex_destroy(&audio_mutex);
        ao_close(audio_device);
        audio_device = 0;
    }
#endif
    SDL_CloseAudio();
    if (sdl_audio_buffer) {
        free(sdl_audio_buffer);
        sdl_audio_buffer = 0;
    }
}
