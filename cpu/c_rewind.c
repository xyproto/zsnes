/*
 * Rewind bookkeeping, from cpu/execute.asm. Both routines run inside the 65816
 * execute loop with the core's register ABI live - esi the PC pointer, edi the
 * opcode table, ebp the SPC program counter, edx the cycle/flag word - and
 * they *restore* those registers, because a rewind resumes from the ones the
 * saved frame was taken with. Hence the register file is handed over as a
 * block and written in place, the same seam SA1Swap uses.
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

void ProcessRewindC(zreg* const r)
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

void UpdateRewindC(zreg* const r)
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
