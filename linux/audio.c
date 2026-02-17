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
#include <stdint.h>

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
static volatile bool pipewire_shutting_down = false;
bool sound_pipewire = false;
#endif

uint8_t* sdl_audio_buffer = NULL;
int sdl_audio_buffer_len = 0, sdl_audio_buffer_fill = 0;
int sdl_audio_buffer_head = 0, sdl_audio_buffer_tail = 0;
bool sound_sdl = false;
static bool sdl_audio_subsystem = false;
static char sdl_audio_last_error[256];
#ifdef __SDL3__
static SDL_AudioStream* sdl_audio_stream = NULL;
#endif

int SoundEnabled = 1;
uint8_t PrevStereoSound;
uint32_t PrevSoundQuality;

#define SAMPLE_NTSC_HI_SCALE 995ULL
#define SAMPLE_NTSC_LO 59649ULL
#define SAMPLE_PAL_HI_SCALE 1ULL
#define SAMPLE_PAL_LO 50ULL
static const int freqtab[7] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
#define RATE freqtab[SoundQuality = ((SoundQuality > 6) ? 1 : SoundQuality)]

struct
{
    uint64_t hi;
    uint64_t lo;
    uint64_t balance;
} sample_control;

void InitSampleControl()
{
    extern uint8_t romispal;
    if (romispal) {
        sample_control.hi = SAMPLE_PAL_HI_SCALE * RATE;
        sample_control.lo = SAMPLE_PAL_LO;
    } else {
        sample_control.hi = SAMPLE_NTSC_HI_SCALE * RATE;
        sample_control.lo = SAMPLE_NTSC_LO;
    }
    sample_control.balance = sample_control.hi;
}

static void MixSoundBlock(int16_t* const out, uint32_t const samples)
{
    extern uint32_t BufferSizeB, BufferSizeW;
    extern uint8_t DSPDisable;
    extern int DSPBuffer[1280];
    int *d = DSPBuffer, *end_d;
    int16_t* p = out;

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
static void SoundWriteSamples_ao(uint32_t samples)
{
    extern uint32_t BufferSizeB, BufferSizeW;
    extern int DSPBuffer[1280];
    int16_t stemp[1280];

    int *d = DSPBuffer, *end_d = NULL;
    int16_t* p = stemp;

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
    uint32_t samples = 0;

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
    uint32_t samples;

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

static int SoundInit_ao_driver(const char* driver_name)
{
    int driver_id = -1;

    if (!driver_name || !strcmp(driver_name, "auto")) {
        driver_id = ao_default_driver_id();
    } else {
        driver_id = ao_driver_id(driver_name);
    }
    if (driver_id < 0) {
        return (false);
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
        puts("Audio Open Failed");
        return (false);
    }
    return (true);
}

static int SoundInit_ao()
{
    return (SoundInit_ao_driver(libAoDriver));
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
    uint8_t* out;
    int16_t stemp[1280];
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
    out = (uint8_t*)d->data;
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
        uint32_t chunk = bytes >> 1;
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
    d->chunk->size = out - (uint8_t*)d->data;
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
#ifdef __SDL3__
    if (!sdl_audio_stream) {
        return;
    }

    for (;;) {
        int queued = SDL_GetAudioStreamQueued(sdl_audio_stream);
        int16_t chunk[256];

        if (queued < 0 || queued >= sdl_audio_buffer_len) {
            break;
        }

        MixSoundBlock(chunk, 256);
        if (!SDL_PutAudioStreamData(sdl_audio_stream, chunk, sizeof(chunk))) {
            break;
        }
    }
#else
    extern int DSPBuffer[];
    extern uint8_t DSPDisable;
    extern uint32_t BufferSizeB, BufferSizeW;

    // Process sound
    BufferSizeB = 256;
    BufferSizeW = BufferSizeB + BufferSizeB;

    // take care of the things we left behind last time
    SDL_LockAudio();
    while (sdl_audio_buffer_fill < sdl_audio_buffer_len) {
        int16_t* p = (int16_t*)&sdl_audio_buffer[sdl_audio_buffer_tail];

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
#endif
}

static void SoundUpdate_sdl(void* userdata, uint8_t* stream, int len)
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

static int EnsureSDLAudioSubsystem()
{
#ifdef __SDL3__
    if (!sdl_audio_subsystem) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            return (false);
        }
        sdl_audio_subsystem = true;
    }
#else
    if (!sdl_audio_subsystem) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            return (false);
        }
        sdl_audio_subsystem = true;
    }
#endif
    return (true);
}

static void ShutdownSDLAudioSubsystem()
{
    if (!sdl_audio_subsystem) {
        return;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    sdl_audio_subsystem = false;
}

static void SaveSDLError()
{
    char const* err = SDL_GetError();

    if (err && *err) {
        strncpy(sdl_audio_last_error, err, sizeof(sdl_audio_last_error) - 1);
        sdl_audio_last_error[sizeof(sdl_audio_last_error) - 1] = '\0';
    } else {
        sdl_audio_last_error[0] = '\0';
    }
}

static int SoundInit_sdl_once()
{
    const int samptab[7] = { 1, 1, 2, 4, 2, 4, 4 };

    if (!EnsureSDLAudioSubsystem()) {
        SaveSDLError();
        SoundEnabled = 0;
        return (false);
    }
#ifdef __SDL3__
    SDL_AudioSpec wanted;

    if (sdl_audio_stream) {
        SDL_DestroyAudioStream(sdl_audio_stream);
        sdl_audio_stream = NULL;
    }

    if (sdl_audio_buffer) {
        free(sdl_audio_buffer);
        sdl_audio_buffer = 0;
    }
    sdl_audio_buffer_len = 0;
    sdl_audio_buffer_fill = 0;
    sdl_audio_buffer_head = 0;
    sdl_audio_buffer_tail = 0;

    wanted.freq = RATE;
    wanted.channels = StereoSound + 1;
    wanted.format = SDL_AUDIO_S16LE;

    sdl_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wanted, NULL, NULL);
    if (!sdl_audio_stream) {
        SaveSDLError();
        SoundEnabled = 0;
        return (false);
    }
    if (!SDL_ResumeAudioStreamDevice(sdl_audio_stream)) {
        SaveSDLError();
        SDL_DestroyAudioStream(sdl_audio_stream);
        sdl_audio_stream = NULL;
        SoundEnabled = 0;
        return (false);
    }

    sdl_audio_buffer_len = (samptab[SoundQuality] * 128 * wanted.channels * 4 + 255) & ~255;
    if (sdl_audio_buffer_len < 512) {
        sdl_audio_buffer_len = 512;
    }

    sound_sdl = true;
    printf("SDL audio: %u channels, %u Hz\n", wanted.channels, wanted.freq);
    return (true);
#else
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
        SaveSDLError();
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
#endif
}

static int SoundInit_sdl()
{
    int i;
    int num_drivers;
    char const* env_driver;
    char const* hinted_driver;

    sdl_audio_last_error[0] = '\0';
    if (SoundInit_sdl_once()) {
        return (true);
    }

#ifdef __SDL3__
    env_driver = getenv("SDL_AUDIO_DRIVER");
    hinted_driver = SDL_GetHint(SDL_HINT_AUDIO_DRIVER);
#else
    env_driver = getenv("SDL_AUDIODRIVER");
    hinted_driver = SDL_GetHint(SDL_HINT_AUDIODRIVER);
#endif
    if ((env_driver && *env_driver) || (hinted_driver && *hinted_driver)) {
        return (false);
    }

    num_drivers = SDL_GetNumAudioDrivers();
    for (i = 0; i < num_drivers; ++i) {
        char const* driver = SDL_GetAudioDriver(i);

        if (!driver || !*driver) {
            continue;
        }
#ifdef __SDL3__
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, driver);
#else
        SDL_SetHint(SDL_HINT_AUDIODRIVER, driver);
#endif
        ShutdownSDLAudioSubsystem();
        if (SoundInit_sdl_once()) {
            return (true);
        }
    }

    return (false);
}

int InitSound()
{
    static bool warned_auto_fallback = false;
    int prev_sound_enabled;

    sound_sdl = false;
#ifdef __PIPEWIRE__
    sound_pipewire = false;
#endif
    if (!SoundEnabled) {
        return (false);
    }

    PrevSoundQuality = SoundQuality;
    PrevStereoSound = StereoSound;

    if (!strcmp(libAoDriver, "pulse")) {
#ifdef __PIPEWIRE__
        strcpy(libAoDriver, "pipewire");
#else
        strcpy(libAoDriver, "auto");
#endif
    }

    if (!strcmp(libAoDriver, "none")) {
        SoundEnabled = 0;
        return (false);
    }

#ifdef __PIPEWIRE__
    if (!strcmp(libAoDriver, "pipewire")) {
        return (SoundInit_pipewire());
    }
#else
    if (!strcmp(libAoDriver, "pipewire")) {
        puts("WARNING: PipeWire backend requested, but this build has no PipeWire support.");
        SoundEnabled = 0;
        return (false);
    }
#endif

    if (!strcmp(libAoDriver, "auto")) {
#ifdef __PIPEWIRE__
        prev_sound_enabled = SoundEnabled;
        if (SoundInit_pipewire()) {
            return (true);
        }
        SoundEnabled = prev_sound_enabled;
#endif
#ifdef __LIBAO__
        prev_sound_enabled = SoundEnabled;
        if (SoundInit_ao_driver("alsa")) {
            return (true);
        }
        SoundEnabled = prev_sound_enabled;
#endif
        prev_sound_enabled = SoundEnabled;
        if (SoundInit_sdl()) {
            return (true);
        }
        SoundEnabled = prev_sound_enabled;
#ifdef __LIBAO__
        prev_sound_enabled = SoundEnabled;
        if (SoundInit_ao_driver("oss")) {
            return (true);
        }
        SoundEnabled = prev_sound_enabled;
#endif
        if (!warned_auto_fallback) {
            warned_auto_fallback = true;
            if (sdl_audio_last_error[0]) {
                printf("WARNING: SDL audio initialization failed: %s\n", sdl_audio_last_error);
            }
            puts("WARNING: Falling back to no audio backend.");
        }
        SoundEnabled = 0;
        return (false);
    }

    if (!strcmp(libAoDriver, "sdl")) {
        return (SoundInit_sdl());
    }

#ifdef __LIBAO__
    return (SoundInit_ao());
#else
    printf("WARNING: Audio backend \"%s\" requested, but this build has no matching support.\n", libAoDriver);
    SoundEnabled = 0;
    return (false);
#endif
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
#ifdef __SDL3__
    if (sdl_audio_stream) {
        SDL_DestroyAudioStream(sdl_audio_stream);
        sdl_audio_stream = NULL;
    }
#else
    if (sdl_audio_subsystem) {
        SDL_CloseAudio();
    }
#endif
    if (sdl_audio_buffer) {
        free(sdl_audio_buffer);
        sdl_audio_buffer = 0;
    }
    ShutdownSDLAudioSubsystem();
}
