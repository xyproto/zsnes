/*
 * difftest_mixers.c - proves the 8 non-interpolated DSP voice mixers in
 * cpu/dsp_mixers.h bit-identical to their assembly (cpu/dspproc.asm). Run with
 * `make difftest`. Transient: remove once the mixer asm is deleted.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;

#define NBUF 4096 /* byte-addressable DSP/echo/pmod buffers */

/* shared globals (same names/types the asm references) */
u1 Voice0Volume[8], Voice0VolumeR[8], Voice0VolumeL[8];
u1 Voice0Volumee[8], Voice0VolumeRe[8], Voice0VolumeLe[8];
u4 Voice0EnvInc[8], Voice0Freq[8], BRRPlace0[8][2];
u2 VolumeConvTable[65536];
u1 UniqueSoundv;
u4 powhack;
u1 DSPMem[256];
u4 NoiseInc, NoisePointer;
u1 NoiseData[131072];
u1 PModBuffer[NBUF];
s4 DSPBuffer[NBUF / 4];
s4 EchoBuffer[NBUF / 4];
static u1 brrbuf[262144]; /* decoded-sample buffer (edi) */

/* the C ports under test - the same source the emulator ships */
#include "../cpu/dsp_mixers.h"
typedef void mixfn(u4, u4*, u4*, s2*);

extern void asm_NonEchoMono(void), asm_NonEchoStereo(void), asm_EchoMono(void), asm_EchoStereo(void);
extern void asm_NonEchoMonoPM(void), asm_NonEchoStereoPM(void), asm_EchoMonoPM(void), asm_EchoStereoPM(void);

/* call any mixer via the shared register ABI (ebp=voice, esi/ebx in-out, edi in) */
static void call_asm(void (*fn)(void), u4 voice, u4* pesi, u4* pebx, s2* edi)
{
    u4 eax = voice, ebx = *pebx, esi = *pesi;
    /* load fn into ecx before ebp is repurposed for the voice */
    __asm__ volatile("mov %[fn],%%ecx; push %%ebp; mov %%eax,%%ebp; call *%%ecx; pop %%ebp"
        : "+S"(esi), "+b"(ebx), "+a"(eax)
        : [fn] "m"(fn), "D"(edi)
        : "ecx", "edx", "cc", "memory");
    *pesi = esi;
    *pebx = ebx;
}

int main(void)
{
    static const struct {
        const char* name;
        void (*a)(void);
        mixfn* c;
    } M[] = {
        { "NonEchoMono", asm_NonEchoMono, w_NonEchoMono },
        { "NonEchoStereo", asm_NonEchoStereo, w_NonEchoStereo },
        { "EchoMono", asm_EchoMono, w_EchoMono },
        { "EchoStereo", asm_EchoStereo, w_EchoStereo },
        { "NonEchoMonoPM", asm_NonEchoMonoPM, w_NonEchoMonoPM },
        { "NonEchoStereoPM", asm_NonEchoStereoPM, w_NonEchoStereoPM },
        { "EchoMonoPM", asm_EchoMonoPM, w_EchoMonoPM },
        { "EchoStereoPM", asm_EchoStereoPM, w_EchoStereoPM },
    };

    for (int i = 0; i < 65536; i++)
        VolumeConvTable[i] = (u2)(i * 131 + 7);
    dt_fill(brrbuf, sizeof brrbuf);
    dt_fill(NoiseData, sizeof NoiseData);
    /* fixed non-zero initial buffer contents to exercise the += accumulation */
    static u1 dsp0[NBUF], echo0[NBUF], pm0[NBUF];
    dt_fill(dsp0, sizeof dsp0);
    dt_fill(echo0, sizeof echo0);
    dt_fill(pm0, sizeof pm0);

    static u1 A_dsp[NBUF], A_echo[NBUF], A_pm[NBUF];
    static u1 C_dsp[NBUF], C_echo[NBUF], C_pm[NBUF];
    u4 A_brr[16], C_brr[16];
    int total = 0;

    for (unsigned mi = 0; mi < sizeof M / sizeof M[0]; mi++) {
        DT_MAIN(2024u + mi, 150000)
        {
            u4 ebp = dt_mod(8);
            u4 esi = dt_mod(NBUF / 8) * 2; /* even, leaves room for stereo esi*4+4 */
            u4 ebx = dt_u32();
            for (int i = 0; i < 8; i++) {
                Voice0Volume[i] = (u1)rand();
                Voice0VolumeR[i] = (u1)rand();
                Voice0VolumeL[i] = (u1)rand();
                Voice0Volumee[i] = (u1)rand();
                Voice0VolumeRe[i] = (u1)rand();
                Voice0VolumeLe[i] = (u1)rand();
                Voice0EnvInc[i] = dt_u32();
                Voice0Freq[i] = dt_u32();
                BRRPlace0[i][0] = dt_mod(256);
                BRRPlace0[i][1] = dt_mod(200);
            }
            UniqueSoundv = dt_mod(3) ? 0 : 1;
            powhack = 1u << ebp;
            DSPMem[0x3D] = (u1)rand();
            NoiseInc = dt_u32();
            u4 np0 = dt_u32();
            u4 brr0[16];
            memcpy(brr0, BRRPlace0, sizeof brr0);

            /* asm */
            memcpy(DSPBuffer, dsp0, NBUF);
            memcpy(EchoBuffer, echo0, NBUF);
            memcpy(PModBuffer, pm0, NBUF);
            memcpy(BRRPlace0, brr0, sizeof brr0);
            NoisePointer = np0;
            u4 a_esi = esi, a_ebx = ebx;
            call_asm(M[mi].a, ebp, &a_esi, &a_ebx, (s2*)brrbuf);
            memcpy(A_dsp, DSPBuffer, NBUF);
            memcpy(A_echo, EchoBuffer, NBUF);
            memcpy(A_pm, PModBuffer, NBUF);
            memcpy(A_brr, BRRPlace0, sizeof A_brr);
            u4 a_np = NoisePointer;

            /* C */
            memcpy(DSPBuffer, dsp0, NBUF);
            memcpy(EchoBuffer, echo0, NBUF);
            memcpy(PModBuffer, pm0, NBUF);
            memcpy(BRRPlace0, brr0, sizeof brr0);
            NoisePointer = np0;
            u4 c_esi = esi, c_ebx = ebx;
            M[mi].c(ebp, &c_esi, &c_ebx, (s2*)brrbuf);
            memcpy(C_dsp, DSPBuffer, NBUF);
            memcpy(C_echo, EchoBuffer, NBUF);
            memcpy(C_pm, PModBuffer, NBUF);
            memcpy(C_brr, BRRPlace0, sizeof C_brr);
            u4 c_np = NoisePointer;

            DT_EQ("esi", a_esi, c_esi);
            DT_EQ("ebx", a_ebx, c_ebx);
            DT_EQ("NoisePointer", a_np, c_np);
            DT_MEM("DSPBuffer", A_dsp, C_dsp, NBUF);
            DT_MEM("EchoBuffer", A_echo, C_echo, NBUF);
            DT_MEM("PModBuffer", A_pm, C_pm, NBUF);
            DT_MEM("BRRPlace0", A_brr, C_brr, sizeof A_brr);
        }
        printf("  %-16s %s (%d/%ld)\n", M[mi].name, dt_fails ? "FAIL" : "PASS",
            dt_fails, dt_iters);
        total += dt_fails;
    }
    printf(total ? "mixers: FAIL\n" : "mixers: PASS (all 8 bit-identical to asm)\n");
    return total ? 1 : 0;
}
