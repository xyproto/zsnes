/* Direct-page memory accessors ported from cpu/memory.asm. The bodies are in
   cpu/mem_ops.h, which the difftest includes too; this file only supplies the
   seam the assembly spills its registers into. */
#include <stdint.h>

#include "../chips/regabi.h" /* REGABI_ENTRY/REGABI_SYM for the trampolines */
#include "../types.h"
#include "../ui.h" /* regptra, regptwa */

extern u1 wramdataa[65536]; /* ui.h */
extern u1* snesmmap[256]; /* endmem.h */
extern u1 cpu_mdr; /* cpu/regs.inc */

/* What the assembly keeps in ebx, ecx and eax across one handler call. The
   memcop macro in cpu/memory.asm writes all three on the way in and reads them
   back on the way out, so a handler can change any of them. */
u4 MemSeamB;
u4 MemSeamC;
u4 MemSeamA;

#include "mem_ops.h"
