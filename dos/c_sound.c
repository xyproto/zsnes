#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/spc700.h"
#include "../gblvars.h"
#include "../video/procvid.h"
#include "c_sound.h"
#include "sound.h"

u1 SBHDMA = 0;
u1 SBInt = 5 + 8;
u1 SBIrq = 5;
u1 vibracard = 0;

static u1 SBDMA = 1;
static u1 SBDMAPage = 83;
static u1 SBDeinitType;
static u1 SBHDMAPage = 0;
static u1 SBswitch; // Which block to process next
static u2 SBPort = 220; // SoundBlaster DSP Ports
static u4 memoryloc; // Memory offset in conventional memory

void SB_alloc_dma(void)
{
    // Allocate 16384 bytes of DOS memory.
    // To delocate this, use ax=0101h, dx=selector of block/int 31h
    u2 ax;
    u2 dx;
    u4 failed;
    asm volatile("int 0x31;  sbb %2, %2"
                 : "=a"(ax), "=d"(dx), "=b"(failed)
                 : "a"(0x0100), "b"(16384 / 16)
                 : "cc");
    if (failed) {
        // Use extended DOS API to print a string.
        u4 eax;
        asm volatile("int 0x21"
                     : "=a"(eax)
                     : "a"(0x0900), "d"("Unable to allocate conventional memory!\r\n$"));
        DosExit();
    }

    // Check which 8192 byte boundary doesn't cross a page
    memoryloc = ax << 4;
    sbselec = dx;
    sbpmofs = 0;
    if (memoryloc >> 16 != (memoryloc + 8192) >> 16) { // Ext area.
        sbpmofs = 8192;
        memoryloc += 8192;
    }

    // Clear DOS memory.
    memset(memoryloc, 0, 8192);
}

static void SB_dsp_reset(void)
{
    u2 const dx = SBPort;
    outb(dx + 0x06, 0x01);
    inb(dx + 0x06);
    inb(dx + 0x06);
    inb(dx + 0x06);
    inb(dx + 0x06);
    outb(dx + 0x06, 0x00);

    u2 si = 200;
    do {
        // Wait until bit 7 of SBDSPRdStat is set.
        u2 cx = 20000;
        while (!(inb(dx + 0x0E) & 0x80)) {
            if (--cx == 0)
                goto cardfailed;
        }
        if (inb(dx + 0x0A) == 0xAA)
            return;
    } while (--si != 0);

cardfailed:
    u4 eax;
    asm volatile("int 0x10"
                 : "=a"(eax)
                 : "a"(0x0003));
    // Use extended DOS API to print a string.
    asm volatile("int 0x21"
                 : "=a"(eax)
                 : "a"(0x0900), "d"("Sound card failed to initialize!\r\n$"));
    DosExit();
}

// Write value into DSP port
static void SB_dsp_write(u1 const al)
{
    u2 const dx = SBPort + 0x0C;
    while (inb(dx) & 0x80) { }
    outb(dx, al);
}

// Read DSP port
static u1 SB_dsp_read(void)
{
    u2 const dx = SBPort;
    while (!(inb(dx + 0x0E) & 0x80)) { }
    return inb(dx + 0x0A);
}

void DeInitSPC(void)
{
    if (SBDeinitType != 0) { // Double reset.
        SB_dsp_reset();
        SB_dsp_reset();
    }

    // Turn off speakers.
    SB_dsp_write(0xD3);

    // k) Perform Halt DMA Operation, 8-bit command (0xD0 - for virtual speaker).
    SB_dsp_write(0xD0);

    // l) Perform Exit Auto-Initialize DMA Operation, 8-bit command (0xDA).
    SB_dsp_write(SBHDMA != 0 ? /* 16 bit */ 0xD9 : /* 8 bit */ 0xDA);

    // m) Perform Halt DMA Operation, 8-bit command (0xD0 - for virtual speaker).
    SB_dsp_write(0xD0);

    // Disable DMA.
    outb(0x000A, SBDMA + 4);
}

static void outblh(u2 const port, u2 const val)
{
    outb(port, val);
    outb(port, val >> 8);
}

static void SB_dsp_write_lh(u2 const val)
{
    SB_dsp_write(val);
    SB_dsp_write(val >> 8);
}

static void SB_dsp_write_hl(u2 const val)
{
    SB_dsp_write(val >> 8);
    SB_dsp_write(val);
}

void InitSB(void)
{
    static u2 const BufferSize[] = { 320, 320, 320, 500, 320, 400, 400 };
    static u2 const BufferSizes[] = { 320, 320, 500, 900, 400, 750, 750 };
    static u1 const SoundSpeeds[] = { 131, 165, 211, 233, 193, 225, 235 }; // 8khz, 11khz, 22khz, 44khz
    static u1 const SoundSpeedt[] = { 193, 210, 233 }; // 8khz, 11khz, 22khz

    BufferSizeB = (StereoSound == 1 ? BufferSizes : BufferSize)[SoundQuality];
    BufferSizeW = BufferSizeB * 2;

    SBswitch = 0;
    // Allocate pointer.
    // Set up SB.
    SB_dsp_reset();

    // Code added by peter santing.
    if (vibracard == 1) { /* Alternate ViBRA16X SB init code by Peter Santing.
                           * Copied portions of original code and modified it. */

        // Notify user that we're in ViBRA16x mode.
        Msgptr = "VIBRA16X MODE ENABLED";
        MessageOn = MsgCount;

        /* Set Time-Constant Data (= 256 - (1000000 / sampling rate))
         * 8000=131, 22050=210, 44100=233, 11025=165 */

        // Setup DMA.
        // Select DMA channel.
        outb(0x000A, SBDMA + 4);
        // Clear DMA.
        outb(0x000C, 0x00);
        // Set autoinit/write (set as DAC).
        outb(0x000B, SBDMA + 0x58);
        // Send Offset Address.
        u2 const dx = SBDMA << 1;
        outblh(dx, memoryloc);
        // Send length of entire block.
        outblh(dx + 1, BufferSizeW * 2 - 1);
        // Send page # (address / 65536).
        outb(SBDMAPage, memoryloc >> 16);
        // Turn on DMA.
        outb(0x000A, SBDMA);

        SB_dsp_write(0x41);
        SB_dsp_write_hl(SBToSPCSpeeds2[SoundQuality]);

        // Prepare SB for the first block.
        // 16-bit auto-init, mono, unsigned
        SB_dsp_write(0xB6); // Sb 16 version (DSP 4)
        SB_dsp_write(StereoSound == 1 ? /* stereo/signed */ 0x30 : /* mono/signed */ 0x10);

        // Send Length - 1 to DSP port.
        SB_dsp_write_lh(BufferSizeB - 1);

        // Turn on speakers.
        SB_dsp_write(0xD1);
    } else if (SBHDMA == 0) { // No 16 bit.
        // Determine Version #
        SB_dsp_write(0xE1);
        u1 Versionnum[2];
        Versionnum[0] = SB_dsp_read();
        Versionnum[1] = SB_dsp_read();

        // Turn on speakers
        SB_dsp_write(0xD1);

        /* Set Time-Constant Data (= 256 - (1000000 / sampling rate))
         * 8000=131, 22050=210, 44100=233, 11025=165 */
        SB_dsp_write(0x40);
        if (StereoSound == 1) {
            SB_dsp_write(SoundSpeedt[SoundQuality <= 2 || SoundQuality == 4 ? SoundQuality : 2]);
            // Set Stereo
            u2 const dx = SBPort;
            outb(dx + 0x04, 0x0E);
            outb(dx + 0x05, inb(dx + 0x05) | 0x22);
        } else {
            SB_dsp_write(SoundSpeeds[SoundQuality]);
            if (SoundSpeeds[SoundQuality] <= 211)
                Versionnum[0] = 1;
        }

        // Setup DMA.
        // Select DMA channel.
        outb(0x000A, SBDMA + 4);
        // Clear DMA.
        outb(0x000C, 0x00);
        // Set autoinit/write (set as DAC).
        outb(0x000B, SBDMA + 0x58);
        // Send Offset Address.
        u2 const dx = SBDMA << 1;
        outblh(dx, memoryloc);
        // Send length of entire block.
        outblh(dx + 1, BufferSizeW - 1);
        // Send page # (address / 65536).
        outb(SBDMAPage, memoryloc >> 16);
        // Turn on DMA.
        outb(0x000A, SBDMA);

        // Prepare SB for the first block.
        // 8-bit auto-init, mono, unsigned
        SB_dsp_write(0x48); // Sb 2.0 version.

        // Send Length - 1 to DSP port.
        SB_dsp_write_lh(BufferSizeB - 1);

        SBDeinitType = Versionnum[0] > 2 || (Versionnum[0] == 2 && Versionnum[1] != 0);
        SB_dsp_write(SBDeinitType ? /* Sb 2.0 version */ 0x90 : /* Slow speed. */ 0x1C);
    } else if (SBHDMA < 4) { // 16 bit low HDMA.
        /* Set Time-Constant Data (= 256 - (1000000 / sampling rate)).
         * 8000=131, 22050=210, 44100=233, 11025=165 */
        SB_dsp_write(0x40);
        SB_dsp_write(SoundSpeeds[SoundQuality]);

        // Setup DMA.

        // Turn off DMA.
#if 0 // XXX was commented out
		outb(0x00D4, SBHDMA & 0x03 | 0x04);
#endif

        // Setup DMA.
        // Select DMA channel.
        outb(0x000A, SBHDMA & 0x03 | 0x04);

        // Clear flip-flop.
        outb(0x00D8, 0x00);

        // Set autoinit/write (set as DAC).
        outb(0x00D6, (SBHDMA & 0x03) + 0x58);

        // Send Offset Address.
#if 0 // XXX was commented out
		outblh(0xC0 | (SBHDMA & 0x03) << 2, memoryloc / 2);
#endif

        // Send Offset Address.
        u2 const dx = SBDMA << 1;
        outblh(dx, memoryloc);

        // Send length of entire block.
        outblh(dx + 2, BufferSizeW - 1);

        // Send page # (address / 65536).
        outb(SBHDMAPage, memoryloc >> 16);

        // Prepare SB for the first block.
        // 16-bit auto-init, mono, unsigned
        SB_dsp_write(0xB6); // Sb 16 version (DSP 4)
        SB_dsp_write(StereoSound == 1 ? /* stereo/signed */ 0x30 : /* mono/signed */ 0x10);

        // Send Length - 1 to DSP port.
        SB_dsp_write_lh(BufferSizeB - 1);

        // Turn on DMA.
#if 0 // XXX was commented out
		outb(0x00D4, SBHDMA & 0x03);
#endif

        // Setup DMA.
        // Select DMA channel.
        outb(0x000A, SBHDMA & 0x03);

        // Turn on speakers.
        SB_dsp_write(0xD1);
    } else { // 16 bit.
        /* Set Time-Constant Data (= 256 - (1000000 / sampling rate)).
         * 8000=131, 22050=210, 44100=233, 11025=165 */
        SB_dsp_write(0x41);
        SB_dsp_write_hl(SBToSPCSpeeds2[SoundQuality]);

        // Setup DMA.

        // Turn off DMA.
        outb(0x00D4, SBHDMA & 0x03 | 0x04);

        // Clear flip-flop.
        outb(0x00D8, 0x00);

        // Set autoinit/write (set as DAC).
        outb(0x00D6, (SBHDMA & 0x03) + 0x58);

        // Send Offset Address.
        u2 const dx = 0xC0 | (SBHDMA & 0x03) << 2;
        outblh(dx, memoryloc / 2);

        // Send length of entire block.
        outblh(dx + 2, BufferSizeW - 1);

        // Send page # (address / 65536).
        outb(SBHDMAPage, memoryloc >> 16 & 0xFE);

        // Prepare SB for the first block.
        // 16-bit auto-init, mono, unsigned
        SB_dsp_write(0xB6); // Sb 16 version (DSP 4)
        SB_dsp_write(StereoSound == 1 ? /* stereo/signed */ 0x30 : /* mono/signed */ 0x10);

        // Send Length - 1 to DSP port.
        SB_dsp_write_lh(BufferSizeB - 1);

        // Turn on speakers.
        SB_dsp_write(0xD1);

        // Turn on DMA.
        outb(0x00D4, SBHDMA & 0x03);
    }
}

static u2 inblh(u2 const port)
{
    u1 const l = inb(port);
    u1 const h = inb(port);
    return (u2)h << 8 | l;
}

static void GetCDMAPos(void)
{
    // Clear flip-flop.
    outb(0xD8, 0x00);

    static u1 const wordcountport[] = { 1, 3, 5, 7, 0xC2, 0xC6, 0xCA, 0xCE };
    u2 const dx = wordcountport[SBHDMA >= 4 ? SBHDMA : SBDMA];

    u2 bx = inblh(dx);
    if (SBHDMA >= 4)
        bx *= 2;

    // value returned = bx, # of bytes left for transfer
    u2 cx = BufferSizeB * 2;
    u2 dx = BufferSizeB;
    if (SBHDMA >= 4) {
        cx *= 2;
        dx *= 2;
    }
    SBswitch = cx - bx < dx;

#if 0 // XXX was commented out \
    // Old routines, doesn't work w/ SB live!
	do
	{
		u2       cx = inblh(dx);
		u2 const bx = inblh(dx);
		cx -= bx;

		if (cx & 0x8000) cx = -cx;

		if (SBHDMA >= 4) cx *= 2;
	}
	while (cx > 4);

	SB_quality_limiter();
#endif
}

static void SBHandler8(void)
{
    u4 ecx = BufferSizeB;
    s4* esi = DSPBuffer;
    u1* edi = memoryloc;
    if (SBswitch != 0)
        edi += ecx; // Process 2nd block.
    SBswitch ^= 1;

    if (csounddisable == 1 || DSPMem[0x6C] & 0xC0) {
#if 0 // XXX was commented out
		memset(&Voice0Status, 0, sizeof(Voice0Status));
#endif

        memset(esi, 0, ecx * sizeof(*esi));
        // Clear block.
        memset(edi, 0x80, ecx);
    } else { // Process the sound :I
        do {
            s4 eax = *esi++;
            if (eax < -32768)
                eax = -32768;
            if (eax > 32767)
                eax = 32767;
            *edi++ = eax >> 8 ^ 0x80;
        } while (--ecx != 0);

        // Move the good data at SPCRAM[0xF3].
        SPCRAM[0xF3] = DSPMem[SPCRAM[0xF2]];

        asm_call(ProcessSoundBuffer);
    }

    // Acknowledge SB for IRQing.
    inb(SBPort + 0x0E);
    outb(0x0020, 0x20);
    if (SBIrq >= 8)
        outb(0x00A0, 0x20); // High IRQ.
}

// Process 20 blocks * 8 voices (no pitch yet)
static void SBHandler16(void)
{
    if (vibracard != 1)
        GetCDMAPos();

    u4 ecx = BufferSizeB;
    s4* esi = DSPBuffer;
    s2* edi = memoryloc;
    uf(SBswitch != 0) edi += BufferSizeW / 2; // Process 2nd block.
    SBswitch ^= 1;

    if (csounddisable == 1 || DSPMem[0x6C] & 0xC0) {
#if 0 // XXX was commented out
		memset(&Voice0Status, 0, sizeof(Voice0Status));
#endif

        memset(esi, 0, BufferSizeB * sizeof(*esi));
        // Clear block.
        memset(edi, 0, ecx * sizeof(*edi));
    } else {
        do {
            s4 eax = *esi++;
            if (eax < -32768)
                eax = -32768;
            if (eax > 32767)
                eax = 32767;
            *edi++ = eax;
        } while (--ecx != 0);

        asm_call(ProcessSoundBuffer);
    }

    // Acknowledge SB for IRQing.
    inb(SBPort + 0x0F);
    outb(0x0020, 0x20);
    if (SBIrq >= 8)
        outb(0x00A0, 0x20); // High IRQ.
}

void c_SBHandler(void)
{
    if (vibracard != 1 && SBHDMA == 0) {
        SBHandler8();
    } else {
        SBHandler16();
    }
}

void SB_quality_limiter(void)
{
    if (StereoSound != 1)
        return;
    if (SBHDMA != 0)
        return;

    /* ViBRA16X support by Peter Santing
     * Before REALLY switching back to 8-bit sucky mono mode, check that we're
     * dealing with a ViBRA16X Creative Labs Card. */
    if (vibracard == 1)
        return;

    if (SoundQuality <= 2)
        return;
    if (SoundQuality == 4)
        return;
    SoundQuality = 2;
}

void SB_blank(void)
{
    memset(memoryloc, 0, 1280);
}

static void printnum(u4 val)
{
    char buf[10];
    char* b = buf;
    do
        *b++ = '0' + val % 10;
    while ((val /= 10) != 0);
    do
        PrintChar(*--b);
    while (b != buf);
}

// Locates SET BLASTER environment
void getblaster(void)
{
    char const* esi = getenv("BLASTER");
    if (!esi) {
        if (soundon != 0) {
            soundon = 0;
            PrintStr(
                "ERROR : SET BLASTER environment NOT found!\r\n"
                "Unable to enable sound.\r\n"
                "\r\n"
                "Press any key to continue.");
            WaitForKey();
        }
        return;
    }

    u1 cursetting = 0;
    for (;;) {
        char dl = *esi++;
        if ('a' <= dl && dl <= 'z')
            dl -= 'a' - 'A';

        switch (dl) {
        case ' ':
            cursetting = 0;
            continue;
        case 'A':
            cursetting = 1;
            SBPort = 0;
            continue;
        case 'I':
            cursetting = 2;
            SBIrq = 0;
            continue;
        case 'D':
            cursetting = 3;
            SBDMA = 0;
            continue;
        case 'H':
            cursetting = 4;
            SBHDMA = 0;
            continue;
        case '\0':
            goto end;

        deafult:
            switch (cursetting) {
            case 1: // Process A.
                SBPort = SBPort * 16 + (dl - '0');
                break;

            case 2: // Process I.
                if (SBIrq == 1)
                    SBIrq = 10;
                SBIrq += dl - '0';
                break;

            case 3: // Process D.
                SBDMA = dl - '0';
                break;

            case 4: // Process H.
                SBHDMA = dl - '0';
                break;
            }
            break;
        }
    }
end:

    if (SBIrq == 2)
        SBIrq = 9;

    u1 al = SBIrq + 8;
    if (SBIrq >= 8) { // High IRQ.
        al += 0x60;
        PICMaskP += 0x80;
    }
    SBInt = al;

    switch (SBHDMA) {
        /* This piece of code is added by Peter Santing.  It will enable ZSNES to use
         * the full STEREO capability of the ViBRA16X line of Creative instead of
         * playing 8-bit MONOURAL sound. */
#if 0 // XXX was commented out
		case 0:
#endif
    case 1:
    case 2:
    case 3:
        PrintStr(
            "Creative ViBRA16X PnP card detected (support coded by Peter Santing)\r\n"
            "High-DMA is below dma #4\r\n"
            "\r\n"
            "you have now full 16-bit stereo sound with the surround option!\r\n");
        vibracard = 1;
        SBDMA = SBHDMA;
        SBHDMA = 0;
        break;

    case 4:
        SBHDMAPage = 0x8F;
        break;
    case 5:
        SBHDMAPage = 0x8B;
        break;
    case 6:
        SBHDMAPage = 0x89;
        break;
    case 7:
        SBHDMAPage = 0x8A;
        break;
    }

    switch (SBDMA) {
    case 0:
        SBDMAPage = 0x87;
        break;
    case 1:
        SBDMAPage = 0x83;
        break;
    case 2:
        SBDMAPage = 0x81;
        break;
    case 3:
        SBDMAPage = 0x82;
        break;
    }

    if (DisplayS == 1) {
        PrintStr("Sound Blaster Detection Values : \r\n\r\nPORT  : ");
        printhex(SBPort);
        PrintStr("\r\nIRQ   : ");
        printnum(SBIrq);
        PrintStr("\r\nDMA   : ");
        printnum(SBDMA);
        PrintStr("\r\nHDMA  : ");
        printnum(SBHDMA);
        PrintStr("\r\n\r\nPress any key to continue.");
        WaitForKey();
    }
}
