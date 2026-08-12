/* Definitions every makev16t difftest needs because they all link the whole
 * seam list, but which no emulator object in that list provides.
 *
 * video/c_mv16t8to.c calls c_cachesingle4bng, and the offset walk rebuilds its
 * map pointer as vram + a 16-bit offset every column. Only difftest_t8to.c
 * actually drives them; for the rest they exist so the link succeeds.
 */
#include <stdint.h>

typedef uint8_t u1;
typedef uint32_t u4;

u1* vram;
u4 cs4_hits, cs4_last;

void c_cachesingle4bng(u4 ecx);
void c_cachesingle4bng(u4 ecx)
{
    cs4_last = ecx;
    cs4_hits++;
}
