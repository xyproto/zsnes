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

/* The video pass's register file, and the call that hands it to a ported
   entry point. video/c_mv16tline.c owns both in the real build; a difftest
   that links a file calling dl_call needs a working one, not a stub, and
   linking c_mv16tline.c itself would drag in the whole scanline driver. */
u4 DLR[7];

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

void dl_call(void (*fn)(u4*));

void dl_call(void (*const fn)(u4*))
{
    zreg r[8];

    r[R_EAX] = DLR[0];
    r[R_EBX] = DLR[1];
    r[R_ECX] = DLR[2];
    r[R_EDX] = DLR[3];
    r[R_ESI] = DLR[4];
    r[R_EDI] = DLR[5];
    r[R_EBP] = DLR[6];
    r[R_ESP] = 0;
    fn(r);
    DLR[0] = r[R_EAX];
    DLR[1] = r[R_EBX];
    DLR[2] = r[R_ECX];
    DLR[3] = r[R_EDX];
    DLR[4] = r[R_ESI];
    DLR[5] = r[R_EDI];
    DLR[6] = r[R_EBP];
}

u4 cs4_hits, cs4_last;

void c_cachesingle4bng(u4 ecx);
void c_cachesingle4bng(u4 ecx)
{
    cs4_last = ecx;
    cs4_hits++;
}

/* video/c_ngprocbg.c calls these by name. mkoracle renames every symbol the
   oracle defines, so the bare names do not exist in a difftest link - and
   unlike c_mv16tline.c this file has to be in the seam list, because the cur_
   object needs its entry points. No difftest reaches them, so a no-op is
   enough; a test that did would define its own recorder instead. */
void drawsprng16b(void);
void drawsprng16bhr(void);
void drawsprng16b(void) { }
void drawsprng16bhr(void) { }

/* video/c_ngbg.c hands the register block to these; they are video/c_ng2tile.c
   in the emulator, and linking that here would drag the whole renderer in for
   no gain - no newgfx16 difftest drives the background dispatchers. The same
   reasoning as drawsprng16b above. */
void c_ng_drawtileng2b16b(u4* r);
void c_ng_drawtileng4b16b(u4* r);
void c_ng_drawtileng8b16b(u4* r);
void c_ng_drawtileng16x162b16b(u4* r);
void c_ng_drawtileng16x164b16b(u4* r);
void c_ng_drawtileng16x168b16b(u4* r);
void c_ng_drawtileng2b16b(u4* r) { (void)r; }
void c_ng_drawtileng4b16b(u4* r) { (void)r; }
void c_ng_drawtileng8b16b(u4* r) { (void)r; }
void c_ng_drawtileng16x162b16b(u4* r) { (void)r; }
void c_ng_drawtileng16x164b16b(u4* r) { (void)r; }
void c_ng_drawtileng16x168b16b(u4* r) { (void)r; }
u4 ng2_mosaic;

/* Same again for the line renderers c_ngbg.c names. */
void c_ng_drawlineng2b16b(u4* r);
void c_ng_drawlineng4b16b(u4* r);
void c_ng_drawlineng8b16b(u4* r);
void c_ng_drawlineng16x162b16b(u4* r);
void c_ng_drawlineng16x164b16b(u4* r);
void c_ng_drawlineng16x168b16b(u4* r);
void c_ng_drawlineng16x82b16b(u4* r);
void c_ng_drawlineng16x84b16b(u4* r);
void c_ng_drawlinengom2b16b(u4* r);
void c_ng_drawlinengom4b16b(u4* r);
void c_ng_drawlinengom8b16b(u4* r);
void c_ng_drawlinengom16x162b16b(u4* r);
void c_ng_drawlinengom16x164b16b(u4* r);
void c_ng_drawlinengom16x168b16b(u4* r);
void c_ng_drawlineng2b16b(u4* r) { (void)r; }
void c_ng_drawlineng4b16b(u4* r) { (void)r; }
void c_ng_drawlineng8b16b(u4* r) { (void)r; }
void c_ng_drawlineng16x162b16b(u4* r) { (void)r; }
void c_ng_drawlineng16x164b16b(u4* r) { (void)r; }
void c_ng_drawlineng16x168b16b(u4* r) { (void)r; }
void c_ng_drawlineng16x82b16b(u4* r) { (void)r; }
void c_ng_drawlineng16x84b16b(u4* r) { (void)r; }
void c_ng_drawlinengom2b16b(u4* r) { (void)r; }
void c_ng_drawlinengom4b16b(u4* r) { (void)r; }
void c_ng_drawlinengom8b16b(u4* r) { (void)r; }
void c_ng_drawlinengom16x162b16b(u4* r) { (void)r; }
void c_ng_drawlinengom16x164b16b(u4* r) { (void)r; }
void c_ng_drawlinengom16x168b16b(u4* r) { (void)r; }

/* The sixteen background dispatchers. video/c_ngframe.c passes them along as
   function pointers, and every oracle renames its own copies, so the plain
   names resolve to nothing. Nothing in these difftests calls through them. */
#define NG_BG_STUB(n) \
    void n(void);     \
    void n(void) { }
NG_BG_STUB(drawbg1line16b)
NG_BG_STUB(drawbg2line16b)
NG_BG_STUB(drawbg3line16b)
NG_BG_STUB(drawbg4line16b)
NG_BG_STUB(drawbg1tile16b)
NG_BG_STUB(drawbg2tile16b)
NG_BG_STUB(drawbg3tile16b)
NG_BG_STUB(drawbg4tile16b)
NG_BG_STUB(drawbg1linepr116b)
NG_BG_STUB(drawbg2linepr116b)
NG_BG_STUB(drawbg3linepr116b)
NG_BG_STUB(drawbg4linepr116b)
NG_BG_STUB(drawbg1tilepr116b)
NG_BG_STUB(drawbg2tilepr116b)
NG_BG_STUB(drawbg3tilepr116b)
NG_BG_STUB(drawbg4tilepr116b)
