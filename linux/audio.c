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
#include <fcntl.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

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
static int pipewire_inited = 0;
static uint32_t pipewire_target_frames = 256;
static atomic_bool pipewire_shutting_down = false;
bool sound_pipewire = false;

/* Producer fills pw_ring; PipeWireProcess memcpys from it.
 * Producer waits until pw_format_ready and pw_stream_streaming are set. */
static pthread_t pw_producer_thread;
static int pw_producer_running = 0;
static atomic_int pw_producer_terminated = 0;
static pthread_mutex_t pw_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pw_audio_wait = PTHREAD_COND_INITIALIZER;
static atomic_uint pw_samples_waiting = 0;

static atomic_int pw_stream_streaming = 0; /* set by state_changed */
static atomic_int pw_format_ready = 0; /* set by param_changed */
static atomic_int pw_drained = 0; /* set by drained event */

static uint8_t* pw_ring = NULL;
static uint32_t pw_ring_size = 0; /* guarded by pw_ring_mutex */
static pthread_mutex_t pw_ring_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t pw_ring_head = 0; /* guarded by pw_ring_mutex */
static uint32_t pw_ring_tail = 0; /* guarded by pw_ring_mutex */

/* Ring grows on persistent underrun, capped at pw_ring_max_frames. */
static atomic_uint pw_observed_quantum_frames = 0;
static atomic_uint pw_ring_min_frames = 0;
static atomic_uint pw_ring_max_frames = 0;
static uint32_t pw_stride_bytes = 4;
static uint32_t pw_rate_hz = 0;
static int pw_user_forced_latency = 0;
static int pw_stats_enabled = 0;
static _Atomic uint64_t pw_stat_cycles = 0;
static _Atomic uint64_t pw_stat_underruns = 0;
static _Atomic uint64_t pw_stat_silent_pads = 0;
static _Atomic uint64_t pw_stat_resizes = 0;

static uint32_t pw_initial_ring_ms = 250;
static uint32_t pw_max_ring_ms = 2000;
#endif

uint8_t* sdl_audio_buffer = NULL;
int sdl_audio_buffer_len = 0, sdl_audio_buffer_fill = 0;
int sdl_audio_buffer_head = 0, sdl_audio_buffer_tail = 0;
bool sound_sdl = false;
static bool sdl_audio_subsystem = false;
static char sdl_audio_last_error[256];
static SDL_AudioStream* sdl_audio_stream = NULL;

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
        ProcessSoundBuffer();
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

    ProcessSoundBuffer();

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

/* Preferred libao live drivers when "-ad ao" is given, in order.
 * Order: pulse (modern Linux), alsa (Linux), sndio (OpenBSD/FreeBSD),
 * oss (FreeBSD/legacy), sun (NetBSD/Solaris), macosx (macOS),
 * wmm (Windows). The probe is silent, only the chosen one is announced. */
static const char* const ao_preferred_drivers[] = {
    "pulse", "alsa", "sndio", "oss", "sun", "macosx", "wmm", NULL
};

/* Temporarily redirect stderr to /dev/null so libao's chatty plugins
 * (e.g. ao_alsa) stay quiet during speculative probes. */
static int ao_stderr_silence(void)
{
    int saved = dup(STDERR_FILENO);
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) {
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }
    return saved;
}

static void ao_stderr_restore(int saved)
{
    if (saved >= 0) {
        dup2(saved, STDERR_FILENO);
        close(saved);
    }
}

static ao_device* SoundInit_ao_try_open(int driver_id, ao_sample_format* fmt, bool quiet)
{
    ao_device* dev;
    int saved = -1;
    if (quiet) {
        saved = ao_stderr_silence();
    }
    dev = ao_open_live(driver_id, fmt, 0);
    if (quiet) {
        ao_stderr_restore(saved);
    }
    return dev;
}

static int SoundInit_ao_driver(const char* driver_name)
{
    int driver_id = -1;
    bool walk_preferred = false;
    ao_sample_format driver_format = { 0 };

    if (!driver_name || !strcmp(driver_name, "auto")) {
        walk_preferred = true;
    } else {
        driver_id = ao_driver_id(driver_name);
        if (driver_id < 0) {
            return (false);
        }
    }

    driver_format.bits = 16;
    driver_format.channels = StereoSound + 1;
    driver_format.rate = RATE;
    driver_format.byte_format = AO_FMT_LITTLE;
    driver_format.matrix = NULL;

    if (audio_device) {
        ao_close(audio_device);
        audio_device = NULL;
    } else {
        if (pthread_create(&audio_thread, 0, SoundThread_ao, 0)) {
            puts("pthread_create() failed.");
        }
        InitSampleControl();
    }

    if (walk_preferred) {
        int i;
        for (i = 0; ao_preferred_drivers[i]; i++) {
            int id = ao_driver_id(ao_preferred_drivers[i]);
            if (id < 0) {
                continue;
            }
            audio_device = SoundInit_ao_try_open(id, &driver_format, true);
            if (audio_device) {
                driver_id = id;
                break;
            }
        }
        if (!audio_device) {
            puts("libao: no usable native audio driver found.");
            return (false);
        }
    } else {
        audio_device = SoundInit_ao_try_open(driver_id, &driver_format, false);
        if (!audio_device) {
            puts("Audio Open Failed");
            return (false);
        }
    }

    {
        ao_info* di = ao_driver_info(driver_id);
        printf("%s: %u channels, %u Hz\n", di->name, driver_format.channels, driver_format.rate);
    }
    return (true);
}

static int SoundInit_ao()
{
    return (SoundInit_ao_driver(libAoDriver));
}

#endif

#ifdef __PIPEWIRE__
/* Emulator outputs at RATE; resample client-side to pw_out_rate with a
 * 16-tap polyphase Kaiser sinc. Ratio is biased +/-PW_RS_MAX_DELTA by
 * ring fill (bsnes-style DRC) to absorb clock drift. */
#define PW_RS_MAX_DELTA 0.005
#define PW_SINC_TAPS 16
#define PW_SINC_HALF (PW_SINC_TAPS / 2)
#define PW_SINC_PHASES 256

static uint32_t pw_out_rate = 0; /* set by param_changed */
static int pw_banner_shown = 0;
static double pw_sinc_table[PW_SINC_PHASES + 1][PW_SINC_TAPS];
static pthread_mutex_t pw_rs_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct {
    double pos;
    double ratio;
    double base_ratio;
    int16_t hist[2][PW_SINC_TAPS];
} pw_rs;

static double pw_i0(double x)
{
    double sum = 1.0;
    double term = 1.0;
    double y = x * 0.5;
    int k;
    for (k = 1; k < 64; k++) {
        double f = y / (double)k;
        term *= f * f;
        sum += term;
        if (term < 1e-14 * sum)
            break;
    }
    return sum;
}

static void pw_sinc_build(double ratio)
{
    /* fc = cycles/sample (output domain). Upsampling -> 0.5 (Nyquist).
     * Downsampling (ratio > 1) -> 0.5/ratio to prevent aliasing. */
    double fc = (ratio > 1.0) ? (0.5 / ratio) : 0.5;
    /* Kaiser beta=8.5 -> ~96 dB stopband attenuation. */
    double beta = 8.5;
    double inv_i0_beta = 1.0 / pw_i0(beta);
    int p, t;
    for (p = 0; p <= PW_SINC_PHASES; p++) {
        double frac = (double)p / (double)PW_SINC_PHASES;
        double sum = 0.0;
        for (t = 0; t < PW_SINC_TAPS; t++) {
            /* Tap t holds input sample at offset (PW_SINC_HALF-1-t) from
             * the (frac=0) reference; output sits at +frac. So x is the
             * time distance from the output to the tap's sample. */
            double x = (double)(PW_SINC_HALF - 1 - t) + frac;
            double xn = x / (double)PW_SINC_HALF;
            double sinc;
            double win;
            if (xn < -1.0)
                xn = -1.0;
            if (xn > 1.0)
                xn = 1.0;
            if (x == 0.0) {
                sinc = 2.0 * fc;
            } else {
                sinc = sin(M_PI * 2.0 * fc * x) / (M_PI * x);
            }
            win = pw_i0(beta * sqrt(1.0 - xn * xn)) * inv_i0_beta;
            pw_sinc_table[p][t] = sinc * win;
            sum += pw_sinc_table[p][t];
        }
        /* Normalize to unity DC gain. */
        if (sum != 0.0) {
            double inv = 1.0 / sum;
            for (t = 0; t < PW_SINC_TAPS; t++) {
                pw_sinc_table[p][t] *= inv;
            }
        }
    }
}

/* (Re)init resampler for the negotiated rate. Mutex keeps the producer
 * out while pw_sinc_table is being rebuilt. */
static void pw_rs_apply_rate(uint32_t rate)
{
    if (!rate) {
        return;
    }
    pthread_mutex_lock(&pw_rs_mutex);
    pw_out_rate = rate;
    pw_rs.pos = (double)PW_SINC_HALF;
    pw_rs.base_ratio = (double)RATE / (double)rate;
    pw_rs.ratio = pw_rs.base_ratio;
    memset(pw_rs.hist, 0, sizeof(pw_rs.hist));
    pw_sinc_build(pw_rs.base_ratio);
    pthread_mutex_unlock(&pw_rs_mutex);
}

static uint32_t pw_rs_process(const int16_t* in, uint32_t in_frames,
    int16_t* out, uint32_t out_max_frames, uint32_t channels)
{
    uint32_t out_idx = 0;
    uint32_t in_idx = 0;
    while (out_idx < out_max_frames) {
        uint32_t c;
        double pf;
        int p0;
        double pf_frac;
        const double* k0;
        const double* k1;
        while (pw_rs.pos >= 1.0) {
            if (in_idx >= in_frames) {
                return out_idx;
            }
            for (c = 0; c < channels; c++) {
                memmove(&pw_rs.hist[c][0], &pw_rs.hist[c][1],
                    (PW_SINC_TAPS - 1) * sizeof(int16_t));
                pw_rs.hist[c][PW_SINC_TAPS - 1] = in[in_idx * channels + c];
            }
            in_idx++;
            pw_rs.pos -= 1.0;
        }
        pf = pw_rs.pos * (double)PW_SINC_PHASES;
        p0 = (int)pf;
        if (p0 < 0)
            p0 = 0;
        if (p0 >= PW_SINC_PHASES)
            p0 = PW_SINC_PHASES - 1;
        pf_frac = pf - (double)p0;
        k0 = pw_sinc_table[p0];
        k1 = pw_sinc_table[p0 + 1];
        for (c = 0; c < channels; c++) {
            double acc = 0.0;
            int t;
            for (t = 0; t < PW_SINC_TAPS; t++) {
                double h = k0[t] + (k1[t] - k0[t]) * pf_frac;
                acc += h * (double)pw_rs.hist[c][t];
            }
            if (acc > 32767.0)
                acc = 32767.0;
            else if (acc < -32768.0)
                acc = -32768.0;
            out[out_idx * channels + c] = (int16_t)acc;
        }
        out_idx++;
        pw_rs.pos += pw_rs.ratio;
    }
    return out_idx;
}

/* Ring helpers, head and tail are monotonically increasing byte counters.
 * Modulo arithmetic against pw_ring_size yields the physical position. */
static inline uint32_t pw_ring_fill_locked(void) { return pw_ring_head - pw_ring_tail; }
static inline uint32_t pw_ring_space_locked(void) { return pw_ring_size - pw_ring_fill_locked(); }

static void pw_rs_update_dynamic(void)
{
    uint32_t fill_bytes;
    double fill;
    if (!pw_ring_size) {
        return;
    }
    pthread_mutex_lock(&pw_ring_mutex);
    fill_bytes = pw_ring_fill_locked();
    pthread_mutex_unlock(&pw_ring_mutex);
    fill = (double)fill_bytes / (double)pw_ring_size;
    if (fill > 1.0)
        fill = 1.0;
    /* fill=0.5 -> ratio=base. fill=1 -> +max (drain). fill=0 -> -max (fill). */
    pw_rs.ratio = pw_rs.base_ratio * (1.0 + PW_RS_MAX_DELTA * (2.0 * fill - 1.0));
}

static void pw_ring_write(const uint8_t* src, uint32_t n)
{
    pthread_mutex_lock(&pw_ring_mutex);
    if (n >= pw_ring_size) {
        /* Producer outran us by more than the ring, keep newest only. */
        src += n - pw_ring_size;
        n = pw_ring_size;
        pw_ring_head = pw_ring_tail = 0;
    }
    uint32_t space = pw_ring_space_locked();
    if (space < n) {
        /* Drop oldest samples to make room (graceful overflow). */
        pw_ring_tail += n - space;
    }
    uint32_t pos = pw_ring_head % pw_ring_size;
    uint32_t first = (n <= pw_ring_size - pos) ? n : (pw_ring_size - pos);
    memcpy(pw_ring + pos, src, first);
    if (n > first) {
        memcpy(pw_ring, src + first, n - first);
    }
    pw_ring_head += n;
    pthread_mutex_unlock(&pw_ring_mutex);
}

static uint32_t pw_ring_read(uint8_t* dst, uint32_t n)
{
    pthread_mutex_lock(&pw_ring_mutex);
    uint32_t avail = pw_ring_fill_locked();
    if (n > avail) {
        n = avail;
    }
    uint32_t pos = pw_ring_tail % pw_ring_size;
    uint32_t first = (n <= pw_ring_size - pos) ? n : (pw_ring_size - pos);
    memcpy(dst, pw_ring + pos, first);
    if (n > first) {
        memcpy(dst + first, pw_ring, n - first);
    }
    pw_ring_tail += n;
    pthread_mutex_unlock(&pw_ring_mutex);
    return n;
}

/* Resize the ring while preserving already-buffered data.  Cap at max_bytes. */
static void pw_ring_resize(uint32_t want_bytes)
{
    uint8_t* fresh;
    uint32_t cap;

    pthread_mutex_lock(&pw_ring_mutex);
    cap = pw_ring_max_frames * pw_stride_bytes;
    if (cap && want_bytes > cap) {
        want_bytes = cap;
    }
    if (want_bytes <= pw_ring_size) {
        pthread_mutex_unlock(&pw_ring_mutex);
        return;
    }
    fresh = (uint8_t*)malloc(want_bytes);
    if (!fresh) {
        pthread_mutex_unlock(&pw_ring_mutex);
        return;
    }
    uint32_t fill = pw_ring_fill_locked();
    if (fill > want_bytes) {
        pw_ring_tail += fill - want_bytes;
        fill = want_bytes;
    }
    if (fill) {
        uint32_t pos = pw_ring_tail % pw_ring_size;
        uint32_t first = (fill <= pw_ring_size - pos) ? fill : (pw_ring_size - pos);
        memcpy(fresh, pw_ring + pos, first);
        if (fill > first) {
            memcpy(fresh + first, pw_ring, fill - first);
        }
    }
    free(pw_ring);
    pw_ring = fresh;
    pw_ring_size = want_bytes;
    pw_ring_tail = 0;
    pw_ring_head = fill;
    atomic_fetch_add_explicit(&pw_stat_resizes, 1, memory_order_relaxed);
    pthread_mutex_unlock(&pw_ring_mutex);
}

/* Called from PipeWireProcess once the actual quantum is known.  Ensures the
 * ring is at least 8x the negotiated period (so producer jitter has headroom)
 * and at least pw_ring_min_frames worth.  Never shrinks. */
static void pw_smart_size_ring(uint32_t quantum_frames)
{
    uint32_t want_frames;
    uint32_t want_bytes;

    if (!quantum_frames) {
        return;
    }
    if (pw_observed_quantum_frames != quantum_frames) {
        pw_observed_quantum_frames = quantum_frames;
    }
    want_frames = quantum_frames * 8U;
    if (want_frames < pw_ring_min_frames) {
        want_frames = pw_ring_min_frames;
    }
    want_bytes = want_frames * pw_stride_bytes;
    if (want_bytes > pw_ring_size) {
        pw_ring_resize(want_bytes);
    }
}

static void* SoundThread_pipewire(void* useless)
{
    int16_t stemp[1280];
    /* Output buffer sized for moderate upsampling (~2x). When the server
     * rate forces a larger expansion (e.g. RATE=32000 -> 96000Hz, ratio=3x),
     * the input is processed in sub-batches so output always fits. */
    int16_t outbuf[1280 * 2 + 32];
    uint32_t channels = StereoSound + 1;
    uint32_t out_max_frames;
    (void)useless;

    if (!channels) {
        channels = 1;
    }
    out_max_frames = (uint32_t)(sizeof(outbuf) / (sizeof(int16_t) * channels));

    while (!pw_producer_terminated) {
        unsigned int samples;

        pthread_mutex_lock(&pw_audio_mutex);
        while (!pw_producer_terminated
            && (!pw_format_ready || !pw_stream_streaming || !pw_samples_waiting)) {
            pthread_cond_wait(&pw_audio_wait, &pw_audio_mutex);
        }
        samples = pw_samples_waiting;
        pw_samples_waiting = 0;
        pthread_mutex_unlock(&pw_audio_mutex);

        if (pw_producer_terminated) {
            break;
        }

        pw_rs_update_dynamic();

        while (samples > 0) {
            uint32_t chunk = (samples > 1280) ? 1280 : samples;
            uint32_t in_frames;
            uint32_t in_done = 0;
            double ratio_now;
            uint32_t sub_max;

            MixSoundBlock(stemp, chunk);
            in_frames = chunk / channels;

            pthread_mutex_lock(&pw_rs_mutex);
            ratio_now = pw_rs.ratio;
            if (ratio_now <= 0.0) {
                ratio_now = pw_rs.base_ratio > 0.0 ? pw_rs.base_ratio : 1.0;
            }
            pthread_mutex_unlock(&pw_rs_mutex);

            /* out ~= in / ratio, so cap in per call to keep out <= out_max_frames.
             * 0.95 leaves headroom for dynamic-ratio drift; min 1 frame. */
            {
                double allow = (double)out_max_frames * ratio_now * 0.95;
                sub_max = (allow >= 1.0) ? (uint32_t)allow : 1U;
            }

            while (in_done < in_frames) {
                uint32_t sub = in_frames - in_done;
                uint32_t out_frames;
                if (sub > sub_max) {
                    sub = sub_max;
                }
                pthread_mutex_lock(&pw_rs_mutex);
                out_frames = pw_rs_process(stemp + in_done * channels, sub,
                    outbuf, out_max_frames, channels);
                pthread_mutex_unlock(&pw_rs_mutex);
                if (out_frames) {
                    pw_ring_write((uint8_t*)outbuf,
                        out_frames * channels * sizeof(int16_t));
                }
                in_done += sub;
            }
            samples -= chunk;
        }
    }
    return NULL;
}

/* Authoritative setter for pw_out_rate, pw_stride_bytes, and pw_ring.
 * Flips pw_format_ready last so the producer sees a consistent state. */
static void PipeWireParamChanged(void* userdata, uint32_t id, const struct spa_pod* param)
{
    struct spa_audio_info info;
    uint32_t rate;
    uint32_t channels;
    uint32_t stride;
    uint32_t want_bytes;
    (void)userdata;
    if (!param || id != SPA_PARAM_Format) {
        return;
    }
    memset(&info, 0, sizeof(info));
    if (spa_format_parse(param, &info.media_type, &info.media_subtype) < 0) {
        return;
    }
    if (info.media_type != SPA_MEDIA_TYPE_audio
        || info.media_subtype != SPA_MEDIA_SUBTYPE_raw) {
        return;
    }
    if (spa_format_audio_raw_parse(param, &info.info.raw) < 0) {
        return;
    }
    rate = info.info.raw.rate;
    channels = info.info.raw.channels;
    if (!rate || !channels) {
        return;
    }

    stride = channels * sizeof(int16_t); /* we negotiated S16_LE */
    pw_stride_bytes = stride;
    pw_rate_hz = rate;

    pw_rs_apply_rate(rate);

    pw_ring_min_frames = (rate * pw_initial_ring_ms) / 1000U;
    pw_ring_max_frames = (rate * pw_max_ring_ms) / 1000U;
    if (pw_ring_min_frames < 256U) {
        pw_ring_min_frames = 256U;
    }

    /* Size the ring for the actual negotiated rate, not our guess. */
    want_bytes = pw_ring_min_frames * stride;
    pthread_mutex_lock(&pw_ring_mutex);
    if (pw_ring) {
        free(pw_ring);
        pw_ring = NULL;
        pw_ring_size = 0;
    }
    pw_ring = (uint8_t*)malloc(want_bytes);
    if (pw_ring) {
        pw_ring_size = want_bytes;
        pw_ring_head = pw_ring_tail = 0;
    }
    pthread_mutex_unlock(&pw_ring_mutex);

    pipewire_target_frames = rate / 50U; /* ~20 ms */
    if (pipewire_target_frames < 64U) {
        pipewire_target_frames = 64U;
    }

    if (!pw_banner_shown) {
        pw_banner_shown = 1;
        printf("PipeWire audio: %u ch, %u Hz\n", channels, rate);
    }

    pthread_mutex_lock(&pw_audio_mutex);
    atomic_store_explicit(&pw_format_ready, 1, memory_order_release);
    pthread_cond_broadcast(&pw_audio_wait);
    pthread_mutex_unlock(&pw_audio_mutex);
}

static void PipeWireStateChanged(void* userdata, enum pw_stream_state old,
    enum pw_stream_state state, const char* error)
{
    (void)userdata;
    (void)old;
    pthread_mutex_lock(&pw_audio_mutex);
    pw_stream_streaming = (state == PW_STREAM_STATE_STREAMING);
    pthread_cond_broadcast(&pw_audio_wait);
    pthread_mutex_unlock(&pw_audio_mutex);

    if (state == PW_STREAM_STATE_ERROR && error && *error) {
        fprintf(stderr, "PipeWire stream error: %s\n", error);
    } else if (pw_stats_enabled) {
        fprintf(stderr, "PipeWire stream state: %s -> %s\n",
            pw_stream_state_as_string(old), pw_stream_state_as_string(state));
    }
}

static void PipeWireDrained(void* userdata)
{
    (void)userdata;
    pthread_mutex_lock(&pw_audio_mutex);
    pw_drained = 1;
    pthread_cond_broadcast(&pw_audio_wait);
    pthread_mutex_unlock(&pw_audio_mutex);
}

static void PipeWireProcess(void* userdata)
{
    struct pw_buffer* b;
    struct spa_buffer* buf;
    struct spa_data* d;
    uint32_t stride;
    uint32_t bytes;
    uint32_t frames;
    uint8_t* out;
    bool render_audio;
    uint32_t got;

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

    stride = pw_stride_bytes ? pw_stride_bytes : ((uint32_t)d->maxsize >= 4 ? 4 : 2);

    if (b->requested > 0) {
        bytes = (uint32_t)b->requested * stride;
        if (bytes > d->maxsize) {
            bytes = d->maxsize;
        }
    } else {
        bytes = pipewire_target_frames * stride;
        if (bytes > d->maxsize) {
            bytes = d->maxsize;
        }
    }
    bytes -= bytes % stride;
    frames = bytes / stride;

    /* Smart adaptation: learn the server's actual quantum and ensure the ring
     * has at least 8x headroom against it. */
    if (frames && (!pw_observed_quantum_frames || frames > pw_observed_quantum_frames)) {
        pw_smart_size_ring(frames);
        if (frames > pipewire_target_frames) {
            pipewire_target_frames = frames;
        }
    }

    out = (uint8_t*)d->data;
    /* Silence until param_changed has set up resampler+ring. */
    render_audio = atomic_load_explicit(&pw_format_ready, memory_order_acquire)
        && !atomic_load_explicit(&pipewire_shutting_down, memory_order_relaxed)
        && !GUIOn2 && !GUIOn && !EMUPause && !RawDumpInProgress && !T36HZEnabled && soundon;
    atomic_fetch_add_explicit(&pw_stat_cycles, 1, memory_order_relaxed);

    if (!render_audio) {
        memset(out, 0, bytes);
    } else {
        got = pw_ring_read(out, bytes);
        if (got < bytes) {
            atomic_fetch_add_explicit(&pw_stat_underruns, 1, memory_order_relaxed);
            atomic_fetch_add_explicit(&pw_stat_silent_pads, bytes - got, memory_order_relaxed);
            /* On persistent underrun, grow the ring (capped by max_frames). */
            if (pw_observed_quantum_frames) {
                uint32_t bigger = (pw_ring_size / stride) * 2U;
                if (bigger > pw_ring_max_frames) {
                    bigger = pw_ring_max_frames;
                }
                if (bigger * stride > pw_ring_size) {
                    pw_ring_resize(bigger * stride);
                }
            }
            memset(out + got, 0, bytes - got);
        }
    }

    if (pw_stats_enabled && (pw_stat_cycles % 500U) == 0U) {
        printf("PipeWire stats: %llu cycles, %llu underruns, %llu silence bytes, %llu ring resizes, ring=%u/%u frames\n",
            (unsigned long long)pw_stat_cycles,
            (unsigned long long)pw_stat_underruns,
            (unsigned long long)pw_stat_silent_pads,
            (unsigned long long)pw_stat_resizes,
            pw_ring_size / stride,
            pw_ring_max_frames);
    }

    d->chunk->offset = 0;
    d->chunk->stride = stride;
    d->chunk->size = bytes;
    pw_stream_queue_buffer(pipewire_stream, b);
}

void SoundWrite_pipewire()
{
    uint32_t samples = 0;

    if (!sound_pipewire) {
        return;
    }

    if (!pthread_mutex_trylock(&pw_audio_mutex)) {
        if (!pw_samples_waiting && sample_control.lo) {
            samples = (unsigned int)((sample_control.balance / sample_control.lo) << StereoSound);
            sample_control.balance %= sample_control.lo;
            sample_control.balance += sample_control.hi;

            pw_samples_waiting = samples;
            pthread_cond_broadcast(&pw_audio_wait);
        }
        pthread_mutex_unlock(&pw_audio_mutex);
    } else {
        pthread_cond_broadcast(&pw_audio_wait);
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
        .state_changed = PipeWireStateChanged,
        .param_changed = PipeWireParamChanged,
        .process = PipeWireProcess,
        .drained = PipeWireDrained,
    };
    /* No RT_PROCESS: the process callback takes a pthread mutex. */
    enum pw_stream_flags const flags = PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS;
    int rc;
    long forced_latency_ms = 0; /* >0 -> pin PW_KEY_NODE_LATENCY */
    const char* env;

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

    /* Reset negotiation state; callbacks will repopulate. */
    pw_format_ready = 0;
    pw_stream_streaming = 0;
    pw_drained = 0;
    pw_out_rate = 0;
    pw_rate_hz = 0;
    pw_observed_quantum_frames = 0;
    pw_banner_shown = 0;
    pw_stat_cycles = pw_stat_underruns = pw_stat_silent_pads = pw_stat_resizes = 0;

    pw_user_forced_latency = 0;
    if ((env = getenv("ZSNES_PIPEWIRE_LATENCY_MS")) && *env) {
        long v = strtol(env, NULL, 10);
        if (v >= 1 && v <= 1000) {
            forced_latency_ms = v;
            pw_user_forced_latency = 1;
        }
    }
    pw_initial_ring_ms = 250;
    if ((env = getenv("ZSNES_PIPEWIRE_BUFFER_MS")) && *env) {
        long v = strtol(env, NULL, 10);
        if (v >= 50 && v <= 5000) {
            pw_initial_ring_ms = (uint32_t)v;
        }
    }
    pw_max_ring_ms = 2000;
    if ((env = getenv("ZSNES_PIPEWIRE_BUFFER_MAX_MS")) && *env) {
        long v = strtol(env, NULL, 10);
        if (v >= (long)pw_initial_ring_ms && v <= 10000) {
            pw_max_ring_ms = (uint32_t)v;
        }
    }
    pw_stats_enabled = 0;
    if ((env = getenv("ZSNES_AUDIO_STATS")) && *env && *env != '0') {
        pw_stats_enabled = 1;
    }

    /* Hint preferences via properties; server picks final values. */
    {
        char rate_str[32];
        char latency_str[64];
        snprintf(rate_str, sizeof(rate_str), "1/%u", (unsigned)RATE);
        if (pw_user_forced_latency) {
            uint32_t frames = ((uint32_t)RATE * (uint32_t)forced_latency_ms) / 1000U;
            if (frames < 32U)
                frames = 32U;
            snprintf(latency_str, sizeof(latency_str), "%u/%u", frames, (unsigned)RATE);
            props = pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Playback",
                PW_KEY_MEDIA_ROLE, "Game",
                PW_KEY_APP_NAME, "zsnes",
                PW_KEY_NODE_RATE, rate_str,
                PW_KEY_NODE_LATENCY, latency_str,
                NULL);
        } else {
            props = pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Playback",
                PW_KEY_MEDIA_ROLE, "Game",
                PW_KEY_APP_NAME, "zsnes",
                PW_KEY_NODE_RATE, rate_str,
                NULL);
        }
    }

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

    /* S16_LE, rate=0 means "any"; server reports actual rate in param_changed. */
    {
        struct spa_audio_info_raw info = { 0 };
        info.format = SPA_AUDIO_FORMAT_S16_LE;
        info.channels = (uint32_t)(StereoSound + 1);
        info.rate = 0;
        params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);
    }
    rc = pw_stream_connect(pipewire_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, flags,
        (const struct spa_pod**)params, 1);
    pw_thread_loop_unlock(pipewire_loop);
    if (rc < 0) {
        SoundEnabled = 0;
        return false;
    }

    if (!pw_producer_running) {
        InitSampleControl();
        pw_producer_terminated = 0;
        if (pthread_create(&pw_producer_thread, NULL, SoundThread_pipewire, NULL) == 0) {
            pw_producer_running = 1;
        } else {
            puts("pthread_create() for PipeWire producer failed.");
        }
    }

    sound_pipewire = true;
    /* Banner is printed from PipeWireParamChanged once the rate is negotiated. */
    return true;
}
#endif

void SoundWrite_sdl()
{
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
}

static int EnsureSDLAudioSubsystem()
{
    if (!sdl_audio_subsystem) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            return (false);
        }
        sdl_audio_subsystem = true;
    }
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

    env_driver = getenv("SDL_AUDIO_DRIVER");
    hinted_driver = SDL_GetHint(SDL_HINT_AUDIO_DRIVER);
    if ((env_driver && *env_driver) || (hinted_driver && *hinted_driver)) {
        return (false);
    }

    num_drivers = SDL_GetNumAudioDrivers();
    for (i = 0; i < num_drivers; ++i) {
        char const* driver = SDL_GetAudioDriver(i);

        if (!driver || !*driver) {
            continue;
        }
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, driver);
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
        prev_sound_enabled = SoundEnabled;
        if (SoundInit_sdl()) {
            return (true);
        }
        SoundEnabled = prev_sound_enabled;
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
    if (!strcmp(libAoDriver, "ao")) {
        return (SoundInit_ao_driver(NULL));
    }
    if (!strncmp(libAoDriver, "ao:", 3)) {
        return (SoundInit_ao_driver(libAoDriver + 3));
    }
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
    /* Stop producer first so the ring stops growing. */
    if (pw_producer_running) {
        pthread_mutex_lock(&pw_audio_mutex);
        pw_producer_terminated = 1;
        pthread_cond_broadcast(&pw_audio_wait);
        pthread_mutex_unlock(&pw_audio_mutex);
        pthread_join(pw_producer_thread, NULL);
        pw_producer_running = 0;
    }
    /* Drain queued audio, then tear down. Lock for stream ops; release
     * before pw_thread_loop_stop (loop docs require this). */
    if (pipewire_stream && pipewire_loop) {
        pw_thread_loop_lock(pipewire_loop);
        pw_drained = 0;
        if (pw_stream_flush(pipewire_stream, true) == 0) {
            /* Up to 200 ms for the drained event. */
            struct timespec deadline;
            clock_gettime(CLOCK_REALTIME, &deadline);
            deadline.tv_nsec += 200 * 1000 * 1000;
            if (deadline.tv_nsec >= 1000000000L) {
                deadline.tv_sec += 1;
                deadline.tv_nsec -= 1000000000L;
            }
            pthread_mutex_lock(&pw_audio_mutex);
            while (!pw_drained) {
                if (pthread_cond_timedwait(&pw_audio_wait, &pw_audio_mutex, &deadline) != 0) {
                    break;
                }
            }
            pthread_mutex_unlock(&pw_audio_mutex);
        }
        pw_stream_set_active(pipewire_stream, false);
        pw_stream_disconnect(pipewire_stream);
        pw_stream_destroy(pipewire_stream);
        pipewire_stream = 0;
        pw_thread_loop_unlock(pipewire_loop);
    }
    if (pipewire_loop) {
        /* Must be unlocked: stop joins the loop thread. */
        pw_thread_loop_stop(pipewire_loop);
        pw_thread_loop_destroy(pipewire_loop);
        pipewire_loop = 0;
    }
    if (pw_stats_enabled && pw_stat_cycles) {
        printf("PipeWire final: %llu cycles, %llu underruns (%.4f%%), %llu silence bytes, ring grew %llu times to %u frames\n",
            (unsigned long long)pw_stat_cycles,
            (unsigned long long)pw_stat_underruns,
            (100.0 * (double)pw_stat_underruns) / (double)pw_stat_cycles,
            (unsigned long long)pw_stat_silent_pads,
            (unsigned long long)pw_stat_resizes,
            pw_stride_bytes ? (pw_ring_size / pw_stride_bytes) : 0);
    }
    if (pw_ring) {
        free(pw_ring);
        pw_ring = NULL;
        pw_ring_size = 0;
    }
    pw_banner_shown = 0;
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
    if (sdl_audio_stream) {
        SDL_DestroyAudioStream(sdl_audio_stream);
        sdl_audio_stream = NULL;
    }
    if (sdl_audio_buffer) {
        free(sdl_audio_buffer);
        sdl_audio_buffer = 0;
    }
    ShutdownSDLAudioSubsystem();
}
