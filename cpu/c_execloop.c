/* The scanline half of the 65816 dispatch loop from cpu/execute.asm: cpuover,
   the SA-1 speed hacks, the cheat engine, pexecs and execsingle.

   The assembly jumped back into the dispatch loop rather than returning, so
   c_cpuover reports which jump it wants as an enum and exec_loop
   (cpu/c_execute.c) takes it. Registers travel in a pushad-ordered block. */
#include <string.h>

#include "../types.h"

#include "../c_init.h"
#include "../c_vcache.h"
#include "../chips/sa1proc.h"
#include "../chips/sa1regs.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../initc.h"
#include "../ui.h"
#include "../video/newgfx.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "c_dispatch.h"
#include "c_dma.h"
#include "c_dsp.h"
#include "c_execloop.h"
#include "c_execute.h"
#include "c_irq.h"
#include "c_rewind.h"
#include "execute.h"
#include "memtable.h"
#include "table.h"

/* Symbols the assembly reached by name and no header declares yet. Sizes match
   the storage: several are dword cells the assembly only ever touched a byte
   of, so those are read and written through GETB/SETB below. */
extern u1 rtoflags;
extern u1 cycpl;
extern u4 dmaowedcyc; /* cpu/c_dma.c */
extern u1 intrset;
extern u1 CurrentExecSA1;
extern u1 SA1SHb; /* low byte of a dword */
extern u4 SA1SH, SA1LBound, SA1UBound;
extern u1* SA1Ptr;
extern u4 lowestspc, highestspc;
extern u4 SPC700read, SPC700write;
extern u1 tempdh;
extern u1 newengen, ForceNewGfxOff, scanlines, ppustatus;
extern u1 device2, MultiTapStat, nosprincr, NMIEnab, TimerEnable;
extern u1 MovieWaiting, PauseFrameMode;
extern u2 oamaddrs, totlines, VIRQLoc;
extern u4 SfxSFR;
extern u4 JoyARead, JoyBRead, JoyCRead2, JoyDRead, JoyERead;
extern u1 JoyCRead;
extern u4 KeyQuickSnapShot, KeyQuickClock, KeyQuickSaveSPC, EMUPauseKey;
extern u4 INCRFrameKey, KeySaveState, KeyLoadState, KeyInsrtChap;
extern u4 KeyPrevChap, KeyNextChap, KeyQuickRst, KeyQuickExit, KeyQuickLoad;
extern u1 KeyOnStA, KeyOnStB;
extern u1 INCRFrame;
extern u1 ZMVZClose;

int c_process_irq(zreg* r);
void drawline(void); /* video/c_makevid.c */
void ProcessMovies(void);
void BackupPauseFrame(void);
void RestorePauseFrame(void);
void UpdateTimer(u4 edx, zreg* pedi);
void SfxVblankCatchup(void);

/* Byte-wide access to a dword cell, as the assembly had it. */
#define GETB(v) ((u1)(v))
#define SETB(v, x) ((v) = ((v) & ~0xFFu) | (u1)(x))

static u4 peek32(u1 const* const p)
{
    u4 v;
    memcpy(&v, p, sizeof v);
    return v;
}

static u2 peek16(u1 const* const p)
{
    u2 v;
    memcpy(&v, p, sizeof v);
    return v;
}

static void poke16(u1* const p, u2 const v)
{
    memcpy(p, &v, sizeof v);
}

/* rol eax,16 */
static u4 rol16(u4 const v)
{
    return v >> 16 | v << 16;
}

/* Per-game patterns where the SA-1 is known to be spinning. Recognising one
   lets the emulator hand the slot straight back to the 65816. The addresses
   are ROM and I-RAM offsets, so each test is a signature, not a heuristic. */
static void sa1speedhacks(zreg* const r)
{
    SA1SHb = 0;

    if (peek16(IRAM + 0xA0) == 0x80BF && peek16(IRAM + 0x20) == 0) {
        u4 const off = (u4)(SA1Ptr - romdata);
        if (off >= 0x83 && off <= 0x97)
            SA1SHb = 1;
    }

    u1 const* p = SA1Ptr;
    if ((peek32(p) == 0xFCF04BA5 || peek32(p - 2) == 0xFCF04BA5) && IRAM[0x4B] == 0)
        SA1SHb = 1;

    if (peek32(p) == 0x80602EEE) {
        u4 const off = (u4)(p - romdata);
        if (off >= 0x4E5 && off <= 0x4E8) {
            SA1SHb = 1;
            poke16(SA1BWPtr + 0x602E, (u2)(peek16(SA1BWPtr + 0x602E) + 4));
        }
    }

    if (!(peek16(IRAM + 0x0A) & 0x8000) && (peek16(IRAM + 0x0E) & 0x8000)) {
        u4 const off = (u4)(SA1Ptr - romdata);
        if (off >= 0xC93 && off <= 0xC9B)
            SA1SHb = 1;
        if (off >= 0xCB8 && off <= 0xCC0)
            SA1SHb = 1;
    }

    /* The 65816's program counter as a WRAM then a ROM offset. Both bases are
       host pointers, so the subtraction is pointer-wide and the difference
       fits a u4. SA1LBound and SA1UBound are written here and read nowhere -
       the assembly that consumed them is gone - but stay because
       cpu/c_execdata.c pins the layout. */
    u4 const woff = (u4)(r[R_ESI] - (zreg)wramdata);
    if (woff >= 0x224 && woff <= 0x22E) {
        SA1LBound = 0x224 + (u4)(uintptr_t)wramdata;
        SA1UBound = 0x22E + (u4)(uintptr_t)wramdata;
        SETB(SA1SH, 1);
    }
    if (woff >= 0x1F7C6 && woff <= 0x1F7CC) {
        SA1LBound = 0x1F7C6 + (u4)(uintptr_t)wramdata;
        SA1UBound = 0x1F7CC + (u4)(uintptr_t)wramdata;
        SETB(SA1SH, 1);
    }
    if (woff >= 0x14 && woff <= 0x1C && peek32(wramdata + 0x14) == 0xF023002C) {
        SA1LBound = 0x14 + (u4)(uintptr_t)wramdata;
        SA1UBound = 0x1C + (u4)(uintptr_t)wramdata;
        SETB(SA1SH, 1);
    }

    u4 const roff = (u4)(r[R_ESI] - (zreg)romdata);
    if (roff >= 0xA56 && roff <= 0xA59) {
        SA1LBound = 0xA56 + (u4)(uintptr_t)romdata;
        SA1UBound = 0xA59 + (u4)(uintptr_t)romdata;
        SETB(SA1SH, 1);
    }

    r[R_ECX] = 0;
    set_dh(r, 0);
    cycpl = 10;
    CurrentExecSA1 = 0;
}

/* One pass of the cheat list, applied once per frame. Entries are 28 bytes:
   byte 0 flags, byte 1 value, bytes 2-3 address, byte 4 bank. */
static void applycheats(void)
{
    u1 numcheat = (u1)NumCheats;
    u4 i = 0;

    do {
        /* The assembly probed the flags byte of the *previous* entry, which on
           the first pass reads the 28 bytes in front of the array. Kept: the
           padding is part of the block and games rely on the result. */
        if (!(cheatdata[i] & 5) && !(cheatdata[i - 28] & 0x80)) {
            if (cheatdata[i] & 0x80) {
                if (numcheat != 1) {
                    u1 const val = memr8(cheatdata[i + 4 + 28], peek16(cheatdata + i + 2 + 28));
                    memw8no_rom(cheatdata[i + 4], peek16(cheatdata + i + 2), val);
                    i += 28;
                    numcheat--;
                }
            } else {
                memw8no_rom(cheatdata[i + 4], peek16(cheatdata + i + 2), cheatdata[i + 1]);
            }
        }
        i += 28;
    } while (--numcheat != 0);
}

#include <stdlib.h>
#ifdef SCANLINE_PC_LOG
#include <stdio.h>
unsigned long scanline_pc_n = 0;
void scanline_pc_log(zreg const* const r)
{
    static FILE* fp = NULL;
#define n scanline_pc_n
    if (!fp) {
        char const* e = getenv("SCANLINE_PC_LOG");
        if (!(e && *e == '1')) { n++; return; }
        fp = fopen("/tmp/zsnes_scan.txt", "wb");
        if (!fp) return;
    }
    {
        extern u1 SPCRAM[];
        extern uint32_t cycpblt;
        fprintf(fp,
            "%lu ypos=%u cyc=%u pc=%04x dh=%02x cycpbl=%08x cycpblt=%08x spcpc=%04x dl=%02x\n",
            n++, (unsigned)curypos, (unsigned)curcyc,
            (unsigned)(u2)((u1*)(uintptr_t)r[R_ESI] - initaddrl),
            (unsigned)DH(r), (unsigned)cycpbl, (unsigned)cycpblt,
            (unsigned)(u2)((u1*)(uintptr_t)r[R_EBP] - SPCRAM),
            (unsigned)(u1)r[R_EDX]);
    }
#undef n
}
#endif

enum exec_act c_cpuover(zreg* const r)
{
    if (curypos == 0)
        rtoflags = 0;

    r[R_ESI]--;

#ifdef SCANLINE_PC_LOG
    /* One line per scanline: the point in the instruction stream the scanline
       boundary fell on. Two builds diverge first at one of these. */
    {
        extern void scanline_pc_log(zreg const*);
        scanline_pc_log(r);
    }
#endif

    if (HIRQNextExe != 0) {
        add_dh(r, (u1)HIRQCycNext);
        HIRQCycNext = 0;
        goto hirq;
    }

    if (!SA1Enable)
        goto nosa1b;
    if (exiter & 0x01)
        goto nosa1;
    if (SA1Control & 0x60)
        goto nosa1;

    SA1Swap(r);
    if (CurrentExecSA1 <= 15)
        return EXEC_NEXT;

    sa1speedhacks(r);

    if (r[R_EDX] & 0x04)
        goto nosa1;
    if (!(SA1IRQEnable & 0x80))
        goto nosa1;
    if (!(SA1DoIRQ & 4))
        goto nosa1;
    SA1DoIRQ &= 0xFFFFFFFBu;
    ((u1*)&SA1Message)[3] = ((u1*)&SA1Message)[1];
    SA1IRQExec |= 1;
    goto virq;

nosa1:
    if ((SA1IRQEnable & 0x20) && (SA1DoIRQ & 8)) {
        SA1DoIRQ &= 0xFFFFFFF7u;
        ((u1*)&SA1Message)[3] = ((u1*)&SA1Message)[1];
        SA1IRQExec |= 2;
        add_dh(r, 10);
        goto virq;
    }

nosa1b:
    if (NextLineCache != 0)
        Donextlinecache();

    if (KeyOnStB != 0)
        ProcessKeyOn(KeyOnStB);
    KeyOnStB = KeyOnStA;
    KeyOnStA = 0;

    if (exiter & 0x01) {
        ExecExitOkay = 0;
        return EXEC_EXIT;
    }
    if (MoviePassWaiting == 1) {
        ExecExitOkay = 0;
        return EXEC_EXIT;
    }

    if (SfxSFR & 0x20) {
        StartSFX();
        r[R_EBX] = 0;
        r[R_ECX] = 0;
    }

    curypos++;
    if (dmaowedcyc != 0) {
        dmaowedcyc = dmaowedcyc > 1364u ? dmaowedcyc - 1364u : 0;
    }
    if (curypos <= resolutn) {
        if (sprtilecnt[curypos] > 34)
            rtoflags |= 0x80;
        if (sprcnt[curypos] > 32)
            rtoflags |= 0x40;
    }

    add_dh(r, cycpl);

    if (curypos >= totlines)
        goto overy;

    if (spcon != 0)
        UpdateTimer(r[R_EDX], &r[R_EDI]);

    if (curypos == (u2)(resolutn + 1))
        goto nmi;

    if (curypos == resolutn)
        exechdma();

    /* hdmacont */
    if (c_process_irq(r))
        goto virq;

    /* The assembly tested nmistatus against 0 here, which always falls
       through; only the bit 0 test below decides. */
    if (!(GETB(nmistatus) & 1)) {
        if (curypos <= resolutn)
            goto dodrawline;
        return EXEC_NEXT;
    }
    if (curypos < resolutn)
        goto dodrawline;
    return EXEC_NEXT;

dodrawline:
    if ((u1)curypos < (u1)nmiprevline)
        goto noskip;
    if (GETB(nmirept) < 10)
        goto noskip;
    if (GETB(curexecstate) == 0)
        set_dh(r, 0);
    if (GETB(nmistatus) >= 2)
        goto noskip;
    if (r[R_ESI] >= nmiprevaddrl && r[R_ESI] <= nmiprevaddrh) {
        if (GETB(nmiprevline) >= 20)
            SETB(nmiprevline, GETB(nmiprevline) - 10);
        curexecstate &= ~1u;
        /* nmistatus is set to 2 here and to 1 just below, so it ends at 1
           either way; only the two writes above are observable. */
    }
    SETB(nmiprevline, GETB(nmiprevline) + 1);
    SETB(nmistatus, 1);

noskip:
    if (hdmadelay != 0) {
        hdmadelay--;
        goto nodohdma;
    }
    if (curypos == 1 && (INTEnab & 0x20) && VIRQLoc == 0)
        goto nodohdma;
    if (curypos >= (u2)(resolutn - 1))
        goto nodohdma;
    exechdma();

nodohdma:
    if (curypos == 1)
        cachevideo();
    if (curblank == 0)
        drawline();
    if (GETB(curexecstate) == 2)
        return EXEC_SOUND;
    if (GETB(curexecstate) == 0)
        set_dh(r, 0);
    return EXEC_NEXT;

nmi:
    irqon = 0x80;
    doirqnext = 0;
    if (yesoutofmemory == 1)
        outofmemfix();
    if (SfxSFR & 0x20)
        SfxVblankCatchup();

    curypos--;
    tempdh = DH(r);
    set_dh(r, 0);
    doirqnext = 0;

    exechdma();
    exechdma();

    NextNGDisplay = 1;
    if (newengen != 0 && curblank == 0 && ForceNewGfxOff == 0)
        StartDrawNewGfx();

nonewgfx:
    if (GUIQuit == 1)
        endprog();

    if (KeyQuickSnapShot != 0 && (pressed[KeyQuickSnapShot] & 1)) {
        SSKeyPressed = 1;
        pressed[KeyQuickSnapShot] = 2;
        return EXEC_EXIT;
    }
    if (KeyQuickClock != 0 && (pressed[KeyQuickClock] & 1)) {
        TimerEnable ^= 1;
        pressed[KeyQuickClock] = 2;
    }
    if (KeyQuickSaveSPC != 0 && (pressed[KeyQuickSaveSPC] & 1)) {
        SPCKeyPressed = 1;
        pressed[KeyQuickSaveSPC] = 2;
        return EXEC_EXIT;
    }
    if (EMUPauseKey != 0 && (pressed[EMUPauseKey] & 1)) {
        EMUPause ^= 1;
        pressed[EMUPauseKey] = 2;
    }
    if (INCRFrameKey != 0 && (pressed[INCRFrameKey] & 1)) {
        INCRFrame ^= 1;
        pressed[INCRFrameKey] = 2;
    }

    if (pressed[1] & 0x01)
        return EXEC_EXIT;
    if (pressed[59] & 0x01)
        return EXEC_EXIT;
    if (nextmenupopup == 1)
        return EXEC_EXIT;
    if (nextmenupopup >= 2)
        nextmenupopup -= 2;

    if (pressed[KeySaveState] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyLoadState] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyInsrtChap] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyPrevChap] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyNextChap] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyQuickRst] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyQuickExit] & 0x01)
        return EXEC_EXIT;
    if (pressed[KeyQuickLoad] & 0x01)
        return EXEC_EXIT;

    if (ExecExitOkay != 0)
        ExecExitOkay--;

    set_dh(r, tempdh);
    curypos++;
    if (NoInputRead != 1)
        ReadInputDevice();

    if (PauseFrameMode == 3) {
        RestorePauseFrame();
        r[R_ESI] = tempesi;
        r[R_EDI] = tempedi;
        r[R_EBP] = tempebp;
        r[R_EDX] = tempedx;
    }

    if (EMUPause == 1 && RawDumpInProgress != 1) {
        if (PauseFrameMode == 1) {
            tempedx = r[R_EDX];
            tempesi = r[R_ESI];
            tempedi = r[R_EDI];
            tempebp = r[R_EBP];
            BackupPauseFrame();
        }

        ProcessRewindC(r);

        if (PauseFrameMode == 2) {
            PauseFrameMode = 3;
            goto noprocmovie;
        }
        if (INCRFrame != 1) {
            StartDrawNewGfx();
            showvideo();
            cachevideo();
            goto nonewgfx;
        }
        INCRFrame ^= 1;
    }

    /* The rewind update has to happen before this frame of the movie is
       processed, so rewind does not back up already incremented values. */
    UpdateRewindC(r);

    if (MovieProcessing != 0) {
        ProcessMovies();
        if (GUIReset == 1) {
            MovieWaiting = 1;
            pressed[KeyQuickRst] = 0x01;
            return EXEC_EXIT;
        }
        if (MovieProcessing == 0 && ZMVZClose == 1)
            DosExit();
    }

noprocmovie:
    if (device2 == 3)
        JoyBNow = 0;
    /* Todo, add second gun... */
    if (device2 == 4)
        JoyBNow = 0;

    if (INTEnab & 1) {
        JoyARead = JoyAOrig;
        JoyANow = rol16(JoyAOrig);
        JoyBRead = JoyBOrig;
        JoyBNow = rol16(JoyBOrig);
        JoyCRead2 = JoyCOrig;
        JoyCNow = rol16(JoyCOrig);
        JoyDRead = JoyDOrig;
        JoyDNow = JoyDOrig;
        JoyERead = JoyEOrig;
        JoyENow = JoyEOrig;
        JoyCRead = 0;
    }
    MultiTapStat = 0x80;

    SETB(joycontren, 0);
    curexecstate |= 1;

    if (CheatOn == 1)
        applycheats();

    if (curypos == VIRQLoc)
        doirqnext = 1;
    oamaddr = oamaddrs;
    nosprincr = 0;
    showvideo();
    r[R_EBX] = 0;
    NMIEnab = 0x81;

    if (!(INTEnab & 0x80))
        goto nonmi;

    curnmi = 1;
    if (intrset == 1)
        intrset = 2;
    if (GETB(nmistatus) == 1)
        SETB(nmirept, 0);
    SETB(nmistatus, 0);

    if (GETB(nmirept) == 0) {
        SETB(nmiprevline, (u1)(resolutn - 2));
        nmiprevaddrl = (zreg)-1; /* "no minimum yet", at the slot width */
        nmiprevaddrh = 0;
        SETB(nmirept, 1);
        doirqnext = 0;
    } else if (GETB(nmirept) != 10) {
        if (r[R_ESI] < nmiprevaddrl)
            nmiprevaddrl = r[R_ESI];
        if (r[R_ESI] > nmiprevaddrh)
            nmiprevaddrh = r[R_ESI];
        SETB(nmirept, GETB(nmirept) + 1);
    } else if (nmiprevaddrh - nmiprevaddrl <= 10 && r[R_ESI] >= nmiprevaddrl && r[R_ESI] <= nmiprevaddrh) {
        doirqnext = 0;
    } else {
        SETB(nmirept, 0);
        nmiprevaddrl = (zreg)-1; /* "no minimum yet", at the slot width */
        nmiprevaddrh = 0;
        doirqnext = 0;
    }
    switchtonmi(&r[R_EDX], &r[R_ESI]);
    r[R_EBX] = 0;
    return EXEC_RELOAD;

nonmi:
    if (intrset == 1)
        intrset = 2;
    r[R_EBX] = 0;
    r[R_ECX] = 0;
    return EXEC_NEXT;

overy:
    set_dh(r, 80);
    if (scanlines == 0)
        cfield ^= 1;
    curypos = 0;
    ppustatus ^= 0x80;

    if (numspcvblleft != 0) {
        if (lowestspc > r[R_EBP] || highestspc < r[R_EBP]) {
            lowestspc = r[R_EBP] - 10;
            highestspc = r[R_EBP] + 10;
            spc700idle = 0;
        }
        if (SPC700write == 0 && spc700read != 0 && SPC700read >= 1500) {
            spc700idle++;
            if (spc700idle == 30) {
                idledetectspc();
                if (ReturnFromSPCStall == 1) {
                    ExecExitOkay = 0;
                    return EXEC_EXIT;
                }
            }
        } else {
            spc700idle = 0;
        }
        numspcvblleft--;
        SPC700write = 0;
        SPC700read = 0;
        spc700read = 0;
    }

    NMIEnab = 0x01;
    starthdma();
    if (c_process_irq(r))
        goto virq;
    return EXEC_NEXT;

virq:
    curexecstate |= 1;
    doirqnext = 0;
    r[R_EBX] = 0;
    if (curypos < resolutn) {
        if (hdmadelay != 0)
            hdmadelay--;
        else
            exechdma();
        if (curypos == 1)
            cachevideo();
        if (curblank == 0)
            drawline();
    }
    if (intrset == 1)
        intrset = 2;
    switchtovirq(&r[R_EDX], &r[R_ESI]);
    r[R_EBX] = 0;
    return EXEC_RELOAD;

hirq:
    HIRQNextExe = 0;
    if (INTEnab & 0x10) {
        curexecstate |= 1;
        doirqnext = 0;
        if (intrset == 1)
            intrset = 2;
        if (!(r[R_EDX] & 0x04)) {
            switchtovirq(&r[R_EDX], &r[R_ESI]);
            r[R_EBX] = 0;
            return EXEC_RELOAD;
        }
        doirqnext = 1;
    }
    r[R_EBX] = 0;
    return EXEC_NEXT;
}

/* Execute a single 65816 instruction (debugging purpose). Called from the
   debugger, which keeps the core state in ordinary variables between steps. */
void execsingle(zreg* const pedx, u1** const pebp, u1** const pesi, opfn*** const pedi)
{
    zreg r[8] = { 0 };
    r[R_EDX] = *pedx;
    r[R_EBP] = (zreg)*pebp;
    r[R_ESI] = (zreg)*pesi;
    r[R_EDI] = (zreg)*pedi;

    if (curexecstate & 2) {
        u4 const dspcyc = cycpbl;
        cycpbl = dspcyc - 55;
        if (dspcyc < 55) {
            cycpbl += cycpblt;
            u4 const sop = *(u1*)r[R_EBP];
            r[R_EBP]++;
            spc_step(r, sop);
        }
    }

    exiter = 0x01;
    r[R_EDI] = (zreg)tablead[(u1)r[R_EDX]];

    u4 op = *(u1*)r[R_ESI];
    r[R_ESI]++;
    {
        u1 const c = cpucycle[op];
        u1 const dh = DH(r);
        set_dh(r, (u1)(dh - c));
        if (dh >= c) {
            /* Fits in what is left of the scanline: run it and stop. dh is
               zeroed first so the opcode's own tail returns after one. */
            pdh = DH(r);
            set_dh(r, 0);
            goto step;
        }
    }

    /* The scanline is spent. Everything below is the debug core's own,
       shorter version of cpuover. */
    if (SA1Enable) {
        cycpl = 150;
        if (!(SA1Control & 0x60)) {
            r[R_ESI]--;
            SA1Swap(r);
            op = *(u1*)r[R_ESI];
            r[R_ESI]++;
            pdh = DH(r);
            set_dh(r, 0);
            if (CurrentExecSA1 < 17) {
                /* The assembly fell through into the main dispatch loop here. */
                set_bl(r, (u1)op);
                exec_loop(r, 1);
                goto done;
            }
            CurrentExecSA1 = 0;
            cycpl = 5;
        }
    }

    if (KeyOnStB != 0)
        ProcessKeyOn(KeyOnStB);
    KeyOnStB = KeyOnStA;
    KeyOnStA = 0;

    if (SfxSFR & 0x20) {
        StartSFXdebugb();
        r[R_ECX] = 0;
    }

    add_dh(r, cycpl);
    pdh = DH(r);

    if (spcon != 0) {
        UpdateTimer(r[R_EDX], &r[R_EDI]);
        r[R_EDI] = (zreg)tablead[(u1)r[R_EDX]];
    }
    set_dh(r, 0);
    curypos++;

    if (curypos == (u2)(resolutn + 1))
        goto nmi;
    if (curypos >= totlines)
        goto overy;

    if (c_process_irq(r))
        goto virq;

    if (curypos >= resolutn)
        goto step;

    if (hdmadelay != 0)
        hdmadelay--;
    else
        exechdma();
    if (curblank == 0)
        drawline();
    goto step;

nmi:
    irqon = 0x80;
    SETB(joycontren, 0);
    if (curypos == VIRQLoc)
        VIRQLoc++;

    ReadInputDevice();

    if (INTEnab & 1) {
        JoyARead = JoyAOrig;
        JoyANow = rol16(JoyAOrig);
        JoyBRead = JoyBOrig;
        JoyBNow = rol16(JoyBOrig);
        JoyCRead2 = JoyCOrig;
        JoyCNow = rol16(JoyCOrig);
        JoyDRead = JoyDOrig;
        JoyDNow = JoyDOrig;
        JoyCRead = 0;
    }

    if (device2 == 3)
        JoyBNow = 0;
    if (device2 == 4)
        JoyBNow = 0;

    MultiTapStat = 0x80;
    NMIEnab = 0x81;
    if (INTEnab & 0x80) {
        curnmi = 1;
        r[R_ESI]--;
        if (intrset == 1)
            intrset = 2;
        switchtonmi(&r[R_EDX], &r[R_ESI]);
        ExecExitOkay = 0;
        goto done;
    }
    if (intrset == 1)
        intrset = 2;
    if (*(u1*)r[R_ESI] == 0xCB)
        r[R_EDX] &= ~0x04u; /* WAI: clear I so the next step makes progress */
    goto step;

overy:
    set_dh(r, 80);
    curypos = 0;
    ppustatus ^= 0x80;
    NMIEnab = 0x01;
    opcd += 170 * 262;
    cachevideo();
    starthdma();
    if (c_process_irq(r))
        goto virq;
    goto step;

virq:
    if (curypos < resolutn) {
        if (hdmadelay != 0)
            hdmadelay--;
        else
            exechdma();
        if (curblank == 0)
            drawline();
    }
    r[R_ESI]--;
    if (intrset == 1)
        intrset = 2;
    switchtovirq(&r[R_EDX], &r[R_ESI]);
    ExecExitOkay = 0;
    goto done;

step:
    /* The debug core's opcode tails do not chain: each one returns here, which
       is what makes this a single step. */
    set_bl(r, (u1)op);
    ((opfn**)r[R_EDI])[r[R_EBX]](r);
    ExecExitOkay = 0;

done:
    *pedx = r[R_EDX];
    *pebp = (u1*)r[R_EBP];
    *pesi = (u1*)r[R_ESI];
    *pedi = (opfn**)r[R_EDI];
}
