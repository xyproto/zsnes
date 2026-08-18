#include <string.h>

#include "../types.h"
#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../chips/sa1regs.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../init.h"
#include "../initc.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_dispatch.h"
#include "c_execloop.h"
#include "c_execute.h"
#include "execute.h"
#include "memory.h"
#include "memtable.h"
#include "spc700.h"
#include "table.h"

enum { R_EDI, R_ESI, R_EBP, R_ESP, R_EBX, R_EDX, R_ECX, R_EAX };

#define DL(r) ((u1)(r)[R_EDX])
#define DH(r) ((u1)((r)[R_EDX] >> 8))

static void set_dh(u4* const r, u1 const v)
{
    r[R_EDX] = (r[R_EDX] & 0xFFFF00FFu) | (u4)v << 8;
}

static void add_dh(u4* const r, u1 const n)
{
    set_dh(r, (u1)(DH(r) + n));
}

/* A cheat's read and write go through the memory handler tables, which are
   still assembly with their own register ABI: al is the value, cx the address
   and bl the bank. Reads hand the value back in al. */
static u4 cheat_call(eop* const fn, u4 eax, u4 ebx, u4 ecx)
{
    __asm__ volatile("call *%3"
                     : "+a"(eax), "+b"(ebx), "+c"(ecx)
                     : "rm"(fn)
                     : "cc", "edx", "memory");
    return eax;
}

static void applycheats(void)
{
    u1 numcheat = NumCheats;
    u4 i = 0;

    do {
        /* The first pass reads cheatdata[-28]; that is how the assembly probed
           the previous record and it is preserved deliberately. */
        if (!(cheatdata[i] & 5) && !(cheatdata[i - 28] & 0x80)) {
            if (cheatdata[i] & 0x80) {
                if (numcheat == 1)
                    goto next;
                u4 eax = cheat_call(memtabler8[cheatdata[i + 4 + 28]],
                    0, cheatdata[i + 4 + 28], *(u2 const*)&cheatdata[i + 2 + 28]);
                cheat_call(memtablew8[cheatdata[i + 4]], eax, cheatdata[i + 4],
                    *(u2 const*)&cheatdata[i + 2]);
                i += 28;
                --numcheat;
            } else {
                cheat_call(memtablew8[cheatdata[i + 4]], cheatdata[i + 1],
                    cheatdata[i + 4], *(u2 const*)&cheatdata[i + 2]);
            }
        }
    next:
        i += 28;
    } while (--numcheat != 0);
}

/* The SA-1 speed hacks: patterns that mean the co-processor is spinning, so
   its slot can be skipped. */
static void sa1speedhacks(u4* const r)
{
    u1 const* ecx;

    SA1SHb = 0;
    if (*(u2 const*)(IRAM + 0xA0) == 0x80BF && *(u2 const*)(IRAM + 0x20) == 0) {
        u4 const off = (u4)(SA1Ptr - romdata);
        if (off >= 0x83 && off <= 0x97)
            SA1SHb = 1;
    }

    ecx = SA1Ptr;
    if ((*(u4 const*)ecx == 0xFCF04BA5 || *(u4 const*)(ecx - 2) == 0xFCF04BA5)
        && IRAM[0x4B] == 0)
        SA1SHb = 1;

    if (*(u4 const*)ecx == 0x80602EEE) {
        u4 const off = (u4)(ecx - romdata);
        if (off >= 0x4E5 && off <= 0x4E8) {
            SA1SHb = 1;
            *(u2*)(SA1BWPtr + 0x602E) += 4;
        }
    }

    if (!(*(u2 const*)(IRAM + 0x0A) & 0x8000) && (*(u2 const*)(IRAM + 0x0E) & 0x8000)) {
        u4 const off = (u4)(SA1Ptr - romdata);
        if (off >= 0xC93 && off <= 0xC9B)
            SA1SHb = 1;
        if (off >= 0xCB8 && off <= 0xCC0)
            SA1SHb = 1;
    }

    {
        u4 const w = (u4)((u1*)r[R_ESI] - wramdata);
        if (w >= 0x224 && w <= 0x22E) {
            SA1LBound = (u4)wramdata + 0x224;
            SA1UBound = (u4)wramdata + 0x22E;
            SA1SH = 1;
        }
        if (w >= 0x1F7C6 && w <= 0x1F7CC) {
            SA1LBound = (u4)wramdata + 0x1F7C6;
            SA1UBound = (u4)wramdata + 0x1F7CC;
            SA1SH = 1;
        }
        if (w >= 0x14 && w <= 0x1C && *(u4 const*)(wramdata + 0x14) == 0xF023002C) {
            SA1LBound = (u4)wramdata + 0x14;
            SA1UBound = (u4)wramdata + 0x1C;
            SA1SH = 1;
        }
    }

    {
        u4 const ro = (u4)((u1*)r[R_ESI] - romdata);
        if (ro >= 0xA56 && ro <= 0xA59) {
            SA1LBound = (u4)romdata + 0xA56;
            SA1UBound = (u4)romdata + 0xA59;
            SA1SH = 1;
        }
    }

    r[R_ECX] = 0;
    set_dh(r, 0);
    cycpl = 10;
    CurrentExecSA1 = 0;
}

/*
 * The tail of a scanline: everything the assembly ran once the cycle budget
 * for the line was spent. Where it jumped back into the dispatch loop this
 * returns an action, since the loop is now C.
 */
enum exec_act c_cpuover(u4* const r)
{
    if (curypos == 0)
        rtoflags = 0;
    r[R_ESI]--;

    if (HIRQNextExe != 0) {
        add_dh(r, HIRQCycNext);
        HIRQCycNext = 0;
        goto hirq;
    }

    if (SA1Enable != 0) {
        if ((exiter & 1) || (SA1Control & 0x60))
            goto nosa1;
        SA1Swap(r);
        if (CurrentExecSA1 <= 15)
            return EXEC_NEXT;
        sa1speedhacks(r);
        if (!(DL(r) & 0x04) && (SA1IRQEnable & 0x80) && (SA1DoIRQ & 4)) {
            SA1DoIRQ &= 0xFFFFFFFBu;
            SA1Message[3] = SA1Message[1];
            SA1IRQExec |= 1;
            goto virq;
        }
    nosa1:
        if ((SA1IRQEnable & 0x20) && (SA1DoIRQ & 8)) {
            SA1DoIRQ &= 0xFFFFFFF7u;
            SA1Message[3] = SA1Message[1];
            SA1IRQExec |= 2;
            add_dh(r, 10);
            goto virq;
        }
    }

    if (NextLineCache != 0)
        Donextlinecache();
    if (KeyOnStB != 0)
        ProcessKeyOn(KeyOnStB);
    KeyOnStB = KeyOnStA;
    KeyOnStA = 0;
    if (exiter & 1) {
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

    ++curypos;
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
        updatetimer();
    if (curypos == (u2)(resolutn + 1))
        goto nmi;
    if (curypos == resolutn)
        exechdma();

    /* hdmacont */
    if (c_process_irq(r))
        goto virq;
    /* The assembly tested nmistatus against 0 here, which always falls
       through; only the bit 0 test below decides. */
    if (!(nmistatus & 1)) {
        if (curypos <= resolutn)
            goto drawline;
        return EXEC_NEXT;
    }
    if (curypos >= resolutn)
        return EXEC_NEXT;

drawline:
    if ((u1)curypos >= nmiprevline && nmirept >= 10) {
        if (curexecstate == 0)
            set_dh(r, 0);
        if (nmistatus < 2) {
            if (r[R_ESI] >= nmiprevaddrl && r[R_ESI] <= nmiprevaddrh) {
                if (nmiprevline >= 20)
                    nmiprevline -= 10;
                curexecstate &= 0xFE;
            }
            /* Both paths land here, so nmistatus ends at 1 either way. */
            nmiprevline += 1;
            nmistatus = 1;
        }
    }

    if (hdmadelay != 0) {
        --hdmadelay;
    } else {
        if (curypos == 1 && (INTEnab & 0x20) && VIRQLoc == 0)
            goto nodohdma;
        if (curypos < (u2)(resolutn - 1))
            exechdma();
    }
nodohdma:
    if (curypos == 1)
        cachevideo();
    if (curblank == 0)
        drawlinec();
    if (curexecstate == 2)
        return EXEC_SOUND;
    if (curexecstate == 0)
        set_dh(r, 0);
    return EXEC_NEXT;

nmi:
    irqon = 0x80;
    doirqnext = 0;
    if (yesoutofmemory == 1)
        outofmemfix();
    if (SfxSFR & 0x20)
        SfxVblankCatchup();

    --curypos;
    tempdh = DH(r);
    set_dh(r, 0);
    doirqnext = 0;
    exechdma();
    exechdma();

    NextNGDisplay = 1;
nonewgfx:
    if (newengen != 0 && curblank == 0 && ForceNewGfxOff == 0)
        StartDrawNewGfx();
    if (GUIQuit == 1)
        endprog();
    if (KeyQuickSnapShot != 0 && pressed[KeyQuickSnapShot] & 1) {
        SSKeyPressed = 1;
        pressed[KeyQuickSnapShot] = 2;
        return EXEC_EXIT;
    }
    if (KeyQuickClock != 0 && pressed[KeyQuickClock] & 1) {
        TimerEnable ^= 1;
        pressed[KeyQuickClock] = 2;
    }
    if (KeyQuickSaveSPC != 0 && pressed[KeyQuickSaveSPC] & 1) {
        SPCKeyPressed = 1;
        pressed[KeyQuickSaveSPC] = 2;
        return EXEC_EXIT;
    }
    if (EMUPauseKey != 0 && pressed[EMUPauseKey] & 1) {
        EMUPause ^= 1;
        pressed[EMUPauseKey] = 2;
    }
    if (INCRFrameKey != 0 && pressed[INCRFrameKey] & 1) {
        INCRFrame ^= 1;
        pressed[INCRFrameKey] = 2;
    }
    if (pressed[1] & 1 || pressed[59] & 1 || nextmenupopup == 1)
        return EXEC_EXIT;
    if (nextmenupopup >= 2)
        nextmenupopup -= 2;
    if (pressed[KeySaveState] & 1 || pressed[KeyLoadState] & 1
        || pressed[KeyInsrtChap] & 1 || pressed[KeyPrevChap] & 1
        || pressed[KeyNextChap] & 1 || pressed[KeyQuickRst] & 1
        || pressed[KeyQuickExit] & 1 || pressed[KeyQuickLoad] & 1)
        return EXEC_EXIT;
    if (ExecExitOkay != 0)
        --ExecExitOkay;

    set_dh(r, tempdh);
    ++curypos;
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
    } else {
        /* Rewind has to be updated before this frame's movie is processed, so
           it does not back up values already advanced for the next frame. */
        UpdateRewindC(r);

        if (MovieProcessing != 0) {
            ProcessMovies();
            if (GUIReset == 1) {
                MovieWaiting = 1;
                pressed[KeyQuickRst] = 1;
                return EXEC_EXIT;
            }
            if (MovieProcessing == 0 && ZMVZClose == 1) {
                DosExit();
                return EXEC_EXIT;
            }
        }
    }
noprocmovie:

    if (device2 == 3 || device2 == 4)
        JoyBNow = 0;

    if (INTEnab & 1) {
        JoyARead = JoyAOrig;
        JoyANow = JoyAOrig << 16 | JoyAOrig >> 16;
        JoyBRead = JoyBOrig;
        JoyBNow = JoyBOrig << 16 | JoyBOrig >> 16;
        JoyCRead2 = JoyCOrig;
        JoyCNow = JoyCOrig << 16 | JoyCOrig >> 16;
        JoyDRead = JoyDOrig;
        JoyDNow = JoyDOrig;
        JoyERead = JoyEOrig;
        JoyENow = JoyEOrig;
        JoyCRead = 0;
    }
    MultiTapStat = 0x80;
    joycontren = 0;
    curexecstate |= 0x01;
    if (CheatOn == 1)
        applycheats();

    if (curypos == VIRQLoc)
        doirqnext = 1;
    oamaddr = oamaddrs;
    nosprincr = 0;
    showvideo();
    r[R_EBX] = 0;
    NMIEnab = 0x81;
    if (!(INTEnab & 0x80)) {
        if (intrset == 1)
            intrset = 2;
        return EXEC_NEXT;
    }

    curnmi = 1;
    if (intrset == 1)
        intrset = 2;
    if (nmistatus == 1)
        nmirept = 0;
    nmistatus = 0;
    if (nmirept == 0) {
        nmiprevline = (u1)resolutn - 2;
        nmiprevaddrl = 0xFFFFFFFFu;
        nmiprevaddrh = 0;
        nmirept = 1;
        doirqnext = 0;
    } else if (nmirept != 10) {
        if (r[R_ESI] < nmiprevaddrl)
            nmiprevaddrl = r[R_ESI];
        if (r[R_ESI] > nmiprevaddrh)
            nmiprevaddrh = r[R_ESI];
        ++nmirept;
    } else if (nmiprevaddrh - nmiprevaddrl <= 10 && r[R_ESI] >= nmiprevaddrl
        && r[R_ESI] <= nmiprevaddrh) {
        doirqnext = 0;
    } else {
        nmirept = 0;
        nmiprevaddrl = 0xFFFFFFFFu;
        nmiprevaddrh = 0;
        doirqnext = 0;
    }
    {
        u1* esi = (u1*)r[R_ESI];
        switchtonmi(&r[R_EDX], &esi);
        r[R_ESI] = (u4)esi;
    }
    r[R_EBX] = 0;
    return EXEC_RELOAD;

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
            if (++spc700idle == 30) {
                idledetectspc();
                if (ReturnFromSPCStall == 1) {
                    ExecExitOkay = 0;
                    return EXEC_EXIT;
                }
            }
        } else {
            spc700idle = 0;
        }
        --numspcvblleft;
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
    curexecstate |= 0x01;
    doirqnext = 0;
    r[R_EBX] = 0;
    if (curypos < resolutn) {
        if (hdmadelay != 0)
            --hdmadelay;
        else
            exechdma();
        if (curypos == 1)
            cachevideo();
        if (curblank == 0)
            drawlinec();
    }
    if (intrset == 1)
        intrset = 2;
    {
        u1* esi = (u1*)r[R_ESI];
        switchtovirq(&r[R_EDX], &esi);
        r[R_ESI] = (u4)esi;
    }
    r[R_EBX] = 0;
    return EXEC_RELOAD;

hirq:
    HIRQNextExe = 0;
    if (INTEnab & 0x10) {
        curexecstate |= 0x01;
        doirqnext = 0;
        if (intrset == 1)
            intrset = 2;
        if (DL(r) & 0x04) {
            doirqnext = 1;
        } else {
            u1* esi = (u1*)r[R_ESI];
            switchtovirq(&r[R_EDX], &esi);
            r[R_ESI] = (u4)esi;
            r[R_EBX] = 0;
            return EXEC_RELOAD;
        }
    }
    return EXEC_NEXT;
}

/*
 * One instruction, for the debugger. The assembly reached the main cpuover
 * when the SA-1 had not yet used up its slot allowance, which hands the rest
 * of the frame to the ordinary dispatch loop; exec_loop keeps that.
 */
void execsingle(u4* const pedx, u1** const pebp, u1** const pesi, eop*** const pedi)
{
    u4 r[8] = { 0 };
    u4 op;

    r[R_EDX] = *pedx;
    r[R_EBP] = (u4)*pebp;
    r[R_ESI] = (u4)*pesi;
    r[R_EDI] = (u4)*pedi;

    if (curexecstate & 2) {
        u4 const dspcyc = cycpbl;
        cycpbl = dspcyc - 55;
        if (dspcyc < 55) {
            cycpbl += cycpblt;
            /* 1260, 10000/12625 */
            u4 const sop = *(u1*)r[R_EBP];
            r[R_EBP]++;
            spc_step(r, sop);
        }
    }

    exiter = 1;
    r[R_EDI] = (u4)tablead[DL(r)];
    op = *(u1*)r[R_ESI];
    r[R_ESI]++;
    {
        u1 const c = cpucycle[op];
        u1 const dh = DH(r);
        set_dh(r, (u1)(dh - c));
        if (dh < c)
            goto cpuover;
    }
    pdh = DH(r);
    set_dh(r, 0);
    run_chain(r, op);
    goto done;

cpuover:
    if (SA1Enable != 0) {
        cycpl = 150;
        if (!(SA1Control & 0x60)) {
            r[R_ESI]--;
            SA1Swap(r);
            op = *(u1*)r[R_ESI];
            r[R_ESI]++;
            pdh = DH(r);
            set_dh(r, 0);
            if (CurrentExecSA1 < 17) {
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
        updatetimer();
        r[R_EDI] = (u4)tablead[DL(r)];
    }
    set_dh(r, 0);
    ++curypos;
    if (curypos == (u2)(resolutn + 1))
        goto nmi;
    if (curypos >= totlines)
        goto overy;
    if (c_process_irq(r))
        goto virq;
    if (curypos < resolutn) {
        if (hdmadelay != 0)
            --hdmadelay;
        else
            exechdma();
        if (curblank == 0)
            drawlinec();
    }
    run_chain(r, op);
    goto done;

nmi:
    irqon = 0x80;
    joycontren = 0;
    if (curypos == VIRQLoc)
        ++VIRQLoc;
    ReadInputDevice();
    if (INTEnab & 1) {
        JoyARead = JoyAOrig;
        JoyANow = JoyAOrig << 16 | JoyAOrig >> 16;
        JoyBRead = JoyBOrig;
        JoyBNow = JoyBOrig << 16 | JoyBOrig >> 16;
        JoyCRead2 = JoyCOrig;
        JoyCNow = JoyCOrig << 16 | JoyCOrig >> 16;
        JoyDRead = JoyDOrig;
        JoyDNow = JoyDOrig;
        JoyCRead = 0;
    }
    if (device2 == 3 || device2 == 4)
        JoyBNow = 0;
    MultiTapStat = 0x80;
    NMIEnab = 0x81;
    if (INTEnab & 0x80) {
        u1* esi;
        curnmi = 1;
        r[R_ESI]--;
        if (intrset == 1)
            intrset = 2;
        esi = (u1*)r[R_ESI];
        switchtonmi(&r[R_EDX], &esi);
        r[R_ESI] = (u4)esi;
        ExecExitOkay = 0;
        goto done;
    }
    if (intrset == 1)
        intrset = 2;
    if (*(u1*)r[R_ESI] == 0xCB)
        r[R_EDX] &= ~0x04u;
    run_chain(r, op);
    goto done;

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
    run_chain(r, op);
    goto done;

virq:
    if (curypos < resolutn) {
        if (hdmadelay != 0)
            --hdmadelay;
        else
            exechdma();
        if (curblank == 0)
            drawlinec();
    }
    r[R_ESI]--;
    if (intrset == 1)
        intrset = 2;
    {
        u1* esi = (u1*)r[R_ESI];
        switchtovirq(&r[R_EDX], &esi);
        r[R_ESI] = (u4)esi;
    }
    ExecExitOkay = 0;

done:
    *pedx = r[R_EDX];
    *pebp = (u1*)r[R_EBP];
    *pesi = (u1*)r[R_ESI];
    *pedi = (eop**)r[R_EDI];
}
