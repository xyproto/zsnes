/*
 * cpu/c_rewind.c - rewind bookkeeping, ported from cpu/execute.asm.
 *
 * Both routines run inside the 65816 execute loop, where the core's register
 * ABI is live: esi is the 65816 PC pointer, edi the opcode table, ebp the SPC
 * program counter and edx the cycle/flag word. They do not just read those
 * registers, they *restore* them - a rewind swaps in a previously saved frame,
 * so the core has to resume from the registers that frame was saved with.
 *
 * cpu/execute.asm therefore keeps the ProcessRewind and UpdateRewind entry
 * points and reduces each to a pushad thunk; the register file is handed over
 * as a block and read and written in place. This is the same seam that
 * chips/sa1proc.asm uses for SA1Swap, and is deliberately not an attempt to
 * restructure execute() itself.
 */
#include "../types.h"
#include "c_memory.h" /* UpdateDPage */
#include "c_rewind.h"
#include "execute.h" /* pressed */

/* Order of the dwords pushad leaves on the stack, lowest address first. */
enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u4 KeyRewind; /* scancode of the rewind key, 0 when unbound */
extern u1 AllocatedRewindStates;
extern u1 PauseFrameMode;

/* Defined alongside the thunks in cpu/execute.asm; the pause-frame path there
 * writes them too, so they stay in the assembly's .data. */
extern u4 tempedx, tempesi, tempedi, tempebp;
extern u4 RewindTimer, DblRewTimer;

void BackupCVFrame(void);
void RestoreCVFrame(void);
void BackupPauseFrame(void);

void ProcessRewindC(u4* const r)
{
    u4 const key = KeyRewind;
    if (pressed[key] != 1)
        return;
    pressed[key] = 2;

    RestoreCVFrame();

    if (PauseFrameMode == 1)
        BackupPauseFrame();

    UpdateDPage();

    /* Resume the core from the state the restored frame was saved with. */
    r[R_ESI] = tempesi;
    r[R_EDI] = tempedi;
    r[R_EBP] = tempebp;
    r[R_EDX] = tempedx;
}

void UpdateRewindC(u4* const r)
{
    if (AllocatedRewindStates == 0 || KeyRewind == 0)
        return;

    /* The assembly decrements both counters but branches on the flags from the
     * second one, so only RewindTimer reaching zero triggers a backup. */
    --DblRewTimer;
    if (--RewindTimer == 0) {
        tempedx = r[R_EDX];
        tempesi = r[R_ESI];
        tempedi = r[R_EDI];
        tempebp = r[R_EBP];
        BackupCVFrame();
    }

    ProcessRewindC(r);
    UpdateDPage();
}
