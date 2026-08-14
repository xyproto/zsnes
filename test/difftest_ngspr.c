/* Differential test: the sprite pixel writers of video/newgfx16.asm against
 * the four-skeleton model in video/c_ngspr.c.
 *
 * No oracle: the writers are macros with no symbols of their own, so the four
 * representative shapes are transcribed verbatim into inline assembly below.
 *
 * It exists because the windowed skeletons are unreachable from any local ROM
 * in an attract run - the pixel A/B validates skeleton A and the dispatch, and
 * nothing else. B, C and D are where the subtle ordering lives: which of the
 * two stores the window gates, and whether the priority bit is claimed before
 * or after it.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 sprpriodata[288];
extern u4 UnusedBit[2], UnusedBitXor[2];

/* c_ngspr.c's dispatch comes along with the writers and needs these; only the
   writers are exercised here. */
u4 ngwinptr;

void ng_spr_test(u1 pixel, u4 x, u4 n, u2* edi, u2 const* pal, u1 const* win,
    u1 adder, u1 dl, int sk, int pal512, int transp, int sub, int orbit,
    int mask, int hires, u4* eax_out);

#define VW 200000u
#define N 3u

static u2 vid_a[VW], vid_c[VW];
static u2 pal[512];
static u1 win[512];

/* eax = pixel, ebx = x, ecx = window pointer, dl = the priority bit,
   dh = the palette adder, ebp = the palette, edi = the video line. The
   palette arrives in esi and moves to ebp inside, because clobbering ebp
   before gcc has read its own operands is a segfault. */
#define WRITER(name, body)                                                    \
    static void name(u4 pixel, u4 x, u2* edi, u2 const* p, u1 const* w,       \
        u1 adder, u1 dl, u4* out)                                             \
    {                                                                         \
        u4 eax = pixel;                                                       \
        u4 edx = ((u4)adder << 8) | dl;                                       \
        __asm__ volatile("pushl %%ebp\n  movl %5, %%ebp\n" body              \
                         "  popl %%ebp\n"                                     \
                         : "+a"(eax)                                          \
                         : "b"(x), "c"(w), "d"(edx), "D"(edi), "S"(p)         \
                         : "memory", "cc");                                   \
        *out = eax;                                                           \
    }

/* sprdrawpra16bngmst - skeleton A */
WRITER(a_A,
    "  orb %%al,%%al\n  jz 9f\n"
    "  addb %%dh,%%al\n"
    "  testb %%dl,sprpriodata+16-3(%%ebx)\n  jnz 9f\n"
    "  movw 512(%%ebp,%%eax,2),%%ax\n"
    "  cmpb $0xC0,%%dh\n  jae 1f\n  andl UnusedBitXor,%%eax\n1:\n"
    "  movw %%ax,-6(%%edi,%%ebx,2)\n"
    "  andl UnusedBitXor,%%eax\n"
    "  movw %%ax,150066(%%edi,%%ebx,2)\n"
    "  xorl %%eax,%%eax\n"
    "  orb %%dl,sprpriodata+16-3(%%ebx)\n9:\n")

/* sprdrawprawb16bngmst - skeleton B, the window gates both stores */
WRITER(a_B,
    "  orb %%al,%%al\n  jz 9f\n"
    "  addb %%dh,%%al\n"
    "  testb %%dl,sprpriodata+16-3(%%ebx)\n  jnz 9f\n"
    "  cmpb $1,-3(%%ecx,%%ebx)\n  je 9f\n"
    "  orb %%dl,sprpriodata+16-3(%%ebx)\n"
    "  movw 512(%%ebp,%%eax,2),%%ax\n"
    "  cmpb $0xC0,%%dh\n  jae 1f\n  andl UnusedBitXor,%%eax\n1:\n"
    "  movw %%ax,-6(%%edi,%%ebx,2)\n"
    "  andl UnusedBitXor,%%eax\n"
    "  movw %%ax,150066(%%edi,%%ebx,2)\n"
    "  xorl %%eax,%%eax\n9:\n")

/* sprdrawprawb16bngst - skeleton C, the window gates only the sub store */
WRITER(a_C,
    "  orb %%al,%%al\n  jz 9f\n"
    "  addb %%dh,%%al\n"
    "  testb %%dl,sprpriodata+16-3(%%ebx)\n  jnz 9f\n"
    "  orb %%dl,sprpriodata+16-3(%%ebx)\n"
    "  movw 512(%%ebp,%%eax,2),%%ax\n"
    "  cmpb $0xC0,%%dh\n  jae 1f\n  andl UnusedBitXor,%%eax\n1:\n"
    "  movw %%ax,-6(%%edi,%%ebx,2)\n"
    "  cmpb $1,-3(%%ecx,%%ebx)\n  je 9f\n"
    "  andl UnusedBitXor,%%eax\n"
    "  movw %%ax,150066(%%edi,%%ebx,2)\n"
    "9:\n  xorl %%eax,%%eax\n")

/* sprdrawprawb16bngmt - skeleton D, the window gates only the main store */
WRITER(a_D,
    "  orb %%al,%%al\n  jz 9f\n"
    "  addb %%dh,%%al\n"
    "  testb %%dl,sprpriodata+16-3(%%ebx)\n  jnz 9f\n"
    "  orb %%dl,sprpriodata+16-3(%%ebx)\n"
    "  movw (%%ebp,%%eax,2),%%ax\n"
    "  movw %%ax,150066(%%edi,%%ebx,2)\n"
    "  cmpb $1,-3(%%ecx,%%ebx)\n  je 9f\n"
    "  cmpb $0xC0,%%dh\n  jae 1f\n  andl UnusedBitXor,%%eax\n1:\n"
    "  orw UnusedBit,%%ax\n"
    "  movw %%ax,-6(%%edi,%%ebx,2)\n"
    "9:\n  xorl %%eax,%%eax\n")

int main(void)
{
    long cov[4];
    memset(cov, 0, sizeof cov);
    UnusedBit[0] = 0x8000u;
    UnusedBitXor[0] = 0x7FFFu;

    DT_MAIN(20260815, 200000)
    {
        u4 pixel = dt_mod(3) ? dt_mod(256) : 0;
        u4 x = dt_mod(200) + 8u;
        u1 adder = (u1)(dt_mod(2) ? dt_mod(256) : 0xC0u + dt_mod(64));
        u1 dl = (u1)(1u << dt_mod(8));
        u1 pr = (u1)dt_u32();
        u4 ea, ec;
        int sk = (int)dt_mod(4);

        for (u4 i = 0; i < 512; i++) {
            pal[i] = (u2)dt_u32();
            win[i] = (u1)(dt_mod(2) ? 1 : dt_mod(3));
        }
        memset(vid_a, 0xCC, sizeof vid_a);
        memset(vid_c, 0xCC, sizeof vid_c);
        memset(sprpriodata, pr, sizeof sprpriodata);

        switch (sk) {
        case 0:
            a_A(pixel, x, vid_a, pal, win, adder, dl, &ea);
            break;
        case 1:
            a_B(pixel, x, vid_a, pal, win, adder, dl, &ea);
            break;
        case 2:
            a_C(pixel, x, vid_a, pal, win, adder, dl, &ea);
            break;
        default:
            a_D(pixel, x, vid_a, pal, win, adder, dl, &ea);
            break;
        }
        {
            u1 pa[288];
            memcpy(pa, sprpriodata, sizeof pa);
            memset(sprpriodata, pr, sizeof sprpriodata);
            ng_spr_test((u1)pixel, x, N, vid_c, pal, win, adder, dl, sk,
                sk == 3 ? 0 : 1, 1, 1, sk == 3 ? 1 : 0, 1, 0, &ec);
            DT_MEM("sprpriodata", pa, sprpriodata, sizeof pa);
        }
        cov[sk]++;
        DT_EQ("eax", ea, ec);
        DT_MEM("video", vid_a, vid_c, sizeof vid_a);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ sk=%d pixel=%u x=%u adder=%02x win=%u\n", sk, pixel, x,
                adder, win[x - N]);
        }
    }
    printf("  A=%ld B=%ld C=%ld D=%ld\n", cov[0], cov[1], cov[2], cov[3]);
    DT_DONE("newgfx16 sprite pixel writers");
}
