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

/* calldl16t's register block. video/c_mv16tline.c owns these in the real
   build, but that file calls the seam thunks by name and mkoracle renames
   every one of them, so no difftest can link it - the globals come from here
   instead. */
u4 DLR[7];
void (*DLFN)(void);

/* calldl16t itself, which lives in video/makev16t.asm in the real build. A
   difftest that links a ported file calling it needs a working one, not a
   stub - this is the same shim written where C can see it. */
__asm__(".pushsection .text\n"
        ".globl calldl16t\n"
        "calldl16t:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl DLR+4, %ebx\n"
        "  movl DLR+8, %ecx\n"
        "  movl DLR+12, %edx\n"
        "  movl DLR+16, %esi\n"
        "  movl DLR+20, %edi\n"
        "  movl DLR+24, %ebp\n"
        "  movl DLR, %eax\n"
        "  call *DLFN\n"
        "  movl %eax, DLR\n"
        "  movl %ebx, DLR+4\n"
        "  movl %ecx, DLR+8\n"
        "  movl %edx, DLR+12\n"
        "  movl %esi, DLR+16\n"
        "  movl %edi, DLR+20\n"
        "  movl %ebp, DLR+24\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n"
        "  ret\n"
        ".popsection\n");
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
