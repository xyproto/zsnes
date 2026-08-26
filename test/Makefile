# test/Makefile, ZSNES2 headless test suite
#
# Usage:
#   make         , build the test binary
#   make run     , build and run
#   make clean   , remove build products
#
# The test binary links only the pre-compiled object files it needs from the
# parent build.  Because the parent compiles with -ffunction-sections, the
# linker's --gc-sections drops every function not reachable from main, so no
# SDL, zlib, or audio symbols are pulled in.

# The difftest targets assemble the original x86 Assembly out of git history,
# so they are 32-bit x86 only. The plain unit tests are portable C, and macOS
# has had no 32-bit userland since 10.15 and no GNU linker at all, so build
# those for the host there instead of not at all.
HOST_OS := $(shell uname -s)
ifeq ($(HOST_OS),Darwin)
CC      := cc
BITFLAG :=
NOPIE   :=
GCSECT  := -Wl,-dead_strip
else
CC      := gcc
BITFLAG := -m32
NOPIE   := -no-pie
GCSECT  := -Wl,--gc-sections
endif

CFLAGS := $(BITFLAG) -std=gnu99 -I.. -Wall -Wextra -Wno-unused-parameter

BIN       := zsnes_test
NETPLAY   := zsnes_netplay_test
ST10      := zsnes_st10_test
ST11      := zsnes_st11_test
OBC1      := zsnes_obc1_test
MSU1      := zsnes_msu1_test
DSP2      := zsnes_dsp2_test
SAI       := zsnes_2xsaiw_test
SA1       := zsnes_sa1_test
SPC7110   := zsnes_7110_test
VCACHE    := zsnes_vcache_test
SPRITES   := zsnes_sprites_test
SRC       := main.c
NETPLAY_SRC := netplay_test.c
ST10_SRC  := st10_test.c
ST11_SRC  := st11_test.c
OBC1_SRC  := obc1_test.c
MSU1_SRC  := msu1_test.c
DSP2_SRC  := dsp2_test.c
SAI_SRC   := 2xsaiw_test.c
SA1_SRC   := sa1_test.c
SPC7110_SRC := 7110_test.c
VCACHE_SRC := vcache_test.c
SPRITES_SRC := sprites_test.c
C4        := zsnes_c4_test
C4_SRC    := c4proc_test.c
MODE7     := zsnes_mode716b_test
MODE7_SRC := mode716b_test.c
ASMDATA   := zsnes_asmdata_test
ASMDATA_SRC := asmdata_test.c
OBJS      := ../zpath.o

.PHONY: all run clean spc64 bgt procwin spcdec fxtable fxops memops m7calc m7ext2 m7start m7win m7bw m7proc m7draw mv16 mvs mvall m716t cb16t cb16b spr16bt spr16t spr16p t8bt t16bt t8t t16t t8to thi mos sprwin ngspr regs ngeng ngframe

all: $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7) $(ASMDATA)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) $(NOPIE) $(GCSECT) -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) $(NOPIE) -lpthread -o $(NETPLAY)

$(ST10): $(ST10_SRC) zstest.h ../chips/st10proc.c
	$(CC) $(CFLAGS) $(ST10_SRC) ../chips/st10proc.c $(NOPIE) -o $(ST10)

$(ST11): $(ST11_SRC) zstest.h ../chips/st11proc.c
	$(CC) $(CFLAGS) $(ST11_SRC) ../chips/st11proc.c $(NOPIE) -o $(ST11)

$(OBC1): $(OBC1_SRC) zstest.h ../chips/obc1proc.c
	$(CC) $(CFLAGS) $(OBC1_SRC) ../chips/obc1proc.c $(NOPIE) -o $(OBC1)

$(MSU1): $(MSU1_SRC) zstest.h ../chips/msu1regs.c
	$(CC) $(CFLAGS) $(MSU1_SRC) ../chips/msu1regs.c $(NOPIE) -o $(MSU1)

$(DSP2): $(DSP2_SRC) zstest.h ../chips/dsp2proc.c
	$(CC) $(CFLAGS) $(DSP2_SRC) ../chips/dsp2proc.c $(NOPIE) -o $(DSP2)

$(SAI): $(SAI_SRC) zstest.h ../video/2xsaiw.c
	$(CC) $(CFLAGS) $(SAI_SRC) ../video/2xsaiw.c $(NOPIE) -o $(SAI)

$(SA1): $(SA1_SRC) zstest.h ../chips/sa1regs.c
	$(CC) $(CFLAGS) $(SA1_SRC) ../chips/sa1regs.c $(NOPIE) -o $(SA1)

$(SPC7110): $(SPC7110_SRC) zstest.h ../chips/7110proc.c
	$(CC) $(CFLAGS) $(SPC7110_SRC) ../chips/7110proc.c $(NOPIE) -o $(SPC7110)

$(C4): $(C4_SRC) zstest.h ../chips/c4proc.c ../chips/c4emu.c
	$(CC) $(CFLAGS) $(C4_SRC) ../chips/c4proc.c ../chips/c4emu.c $(NOPIE) -lm -o $(C4)

$(MODE7): $(MODE7_SRC) zstest.h ../video/mode716b.c
	$(CC) $(CFLAGS) $(MODE7_SRC) ../video/mode716b.c $(NOPIE) -o $(MODE7)

# ../endmem.o supplies winbgdata, which makevid's cwinptr points at.
$(ASMDATA): $(ASMDATA_SRC) zstest.h ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c ../endmem.o
	$(CC) $(CFLAGS) $(ASMDATA_SRC) ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c ../endmem.o $(NOPIE) -o $(ASMDATA)

$(VCACHE): $(VCACHE_SRC) zstest.h ../video/tilecache.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(VCACHE_SRC) ../video/tilecache.c $(NOPIE) $(GCSECT) -o $(VCACHE)

$(SPRITES): $(SPRITES_SRC) zstest.h ../video/sprites.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(SPRITES_SRC) ../video/sprites.c $(NOPIE) $(GCSECT) -o $(SPRITES)

# Differential test for the SPC700 DAA/DAS opcodes against the assembly they
# replaced (cff2b437^). Not part of `all` - it needs that old assembly.
spcdec: difftest_spcdec.c difftest.h mkdifftest.sh ../cpu/spc_ops.h
	git -C .. show cff2b437^:cpu/spc700.asm > /tmp/spc_old.asm
	./mkdifftest.sh _spcdec /tmp/spc_old.asm "OpBE OpDF" \
	    "spcA spcNZ spcP" 439-459 1733-1781
	$(CC) $(CFLAGS) difftest_spcdec.c _spcdec.o $(NOPIE) -o difftest_spcdec
	@./difftest_spcdec

# Differential test for the SuperFX table builder against the assembly it
# replaced. Not part of `all` - it needs that old assembly out of git history.
fxtable: difftest_fxtable.c mkfxtable.sh ../chips/c_fxtable.c ../endmem.o
	./mkfxtable.sh
	$(CC) $(CFLAGS) -fcommon -fno-pic -c difftest_fxtable.c -o _fxtest.o
	$(CC) $(CFLAGS) -fcommon -fno-pic -c ../chips/c_fxtable.c -o _fxtable.o
	$(CC) -m32 $(NOPIE) -fno-pic _fxtest.o _fxtable.o _fxold.o _fxstubs.o ../endmem.o -o difftest_fxtable
	@./difftest_fxtable

# Differential test for the SuperFX opcode handlers against the assembly they
# replaced. Not part of `all` - it needs that assembly out of git history.
fxops: difftest_fxops.c difftest.h mkfxops.sh ../chips/fx_ops.h
	./mkfxops.sh
	$(CC) $(CFLAGS) difftest_fxops.c _fxops.o $(NOPIE) -o difftest_fxops
	@./difftest_fxops

# Differential test for the direct-page memory handlers against the assembly
# they replaced. Not part of `all` - it needs that assembly out of git history.
memops: difftest_memops.c difftest.h mkmemops.sh memops.list ../cpu/mem_ops.h
	./mkmemops.sh
	$(CC) $(CFLAGS) difftest_memops.c _memops.o $(NOPIE) -o difftest_memops
	@./difftest_memops

mvs: difftest_mvs.c difftest.h mkmvs.sh ../video/c_mv16tsms.c
	./mkmvs.sh
	$(CC) $(CFLAGS) difftest_mvs.c ../video/c_mv16tsms.c _mvs.o $(NOPIE) -o difftest_mvs
	@./difftest_mvs

# The oracle is the whole pre-port file, so this one covers every routine in it
# and needs no extraction script; see tools/mkoracle.py.
mvall: difftest_mvall.c difftest.h ../video/c_mv16tsms.c ../video/c_mv16tms.c \
       ../video/c_mv16msgate.c
	python3 ../tools/mkoracle.py video/mv16tms.asm -o _mvall.o \
	    --define draw16x816t=draw16x816t_stub
	$(CC) $(CFLAGS) difftest_mvall.c ../video/c_mv16tsms.c ../video/c_mv16tms.c \
	    ../video/c_mv16msgate.c \
	    _mvall.o $(NOPIE) -o difftest_mvall
	@./difftest_mvall

# Routines that are only reachable by a PC-relative call from inside the file,
# so a difftest cannot see them without mkoracle rewriting them first.
M716T_STUBBED := draw8x816t draw16x1616t draw8x816bt draw16x1616bt \
                 drawsprites16t drawsprites16bt
# Symbols difftest_m716t.c defines itself, so they must not get a blanket stub.
M716T_TAILS := makewindow drawsprites16b draw8x816b draw16x1616b \
               $(addsuffix _stub,$(M716T_STUBBED)) \
               draw8x816tms draw16x1616tms \
               drawmode716t drawmode716b drawmode716tb drawmode716extbg \
               drawmode716textbg drawmode716extbg2 drawmode716textbg2

# Oracle is the whole pre-port makev16t.asm; the emulator's own data objects
# supply the globals, because several of these routines read them wider than
# their declared type and only the real layout makes that agree.
# Every difftest against makev16t.asm builds a `cur_` object from the whole
# working-tree file, so it needs *all* of that file's seam globals - adding a
# ported cluster used to break the other targets' links until its C file was
# added to each of them by hand. One list instead.
M16T_SEAM := ../video/c_m716gate.c ../video/c_mv16tm7.c ../video/c_mv16tclr.c \
             ../video/c_mv16bclr.c ../video/c_mv16tspr.c \
             ../video/c_mv16tsprt.c ../video/c_mv16tsprp.c \
             ../video/c_mv16t8bt.c ../video/c_mv16t16bt.c \
             ../video/c_mv16t8t.c ../video/c_mv16t16t.c \
             ../video/c_mv16t8to.c ../video/c_mv16thi.c m16t_extra.c
# makev16t.asm no longer contains a single `call c_`, so mkoracle's default
# "newest revision without the port marker" lands on the finished file. Pin it
# with a routine the port removed for good: the newest revision that still
# defines draw16x816t and has no thunks is the last fully original one.
M16T_ORIG := --requires 'NEWSYM draw16x816t'

M16T_OBJS := ../cpu/c_regsdata.o ../cpu/c_regswdata.o \
             ../video/c_mode716data.o ../video/makevid.o ../endmem.o
M16T_BSS  := ../video/c_makev16tdata.o

m716t: difftest_m716t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _m716t.o $(M16T_ORIG) \
	    --stub-routine $(M716T_STUBBED) \
	    --stubs _m716t_stubs.c --provided-by $(M16T_OBJS) \
	    --exclude vram $(M716T_TAILS)
	$(CC) $(CFLAGS) difftest_m716t.c $(M16T_SEAM) \
	    _m716t.o _m716t_stubs.c $(M16T_OBJS) \
	    $(NOPIE) -o difftest_m716t
	@./difftest_m716t

cb16t: difftest_cb16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16t.o $(M16T_ORIG) \
	    --stubs _cb16t_stubs.c --provided-by $(M16T_OBJS) \
	    --exclude vram transpbuf fulladdtab
	$(CC) $(CFLAGS) difftest_cb16t.c $(M16T_SEAM) \
	    _cb16t.o _cb16t_stubs.c $(M16T_OBJS) \
	    $(NOPIE) -o difftest_cb16t
	@./difftest_cb16t

cb16b: difftest_cb16b.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16b.o $(M16T_ORIG) \
	    --stubs _cb16b_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram
	$(CC) $(CFLAGS) difftest_cb16b.c $(M16T_SEAM) \
	    _cb16b.o _cb16b_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_cb16b
	@./difftest_cb16b

spr16bt: difftest_spr16bt.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spr.o $(M16T_ORIG) \
	    --stubs _spr_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram
	$(CC) $(CFLAGS) difftest_spr16bt.c $(M16T_SEAM) \
	    _spr.o _spr_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_spr16bt
	@./difftest_spr16bt

spr16t: difftest_spr16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spt.o $(M16T_ORIG) \
	    --stubs _spt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram fulladdtab
	$(CC) $(CFLAGS) difftest_spr16t.c $(M16T_SEAM) \
	    _spt.o _spt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_spr16t
	@./difftest_spr16t

spr16p: difftest_spr16p.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spp.o $(M16T_ORIG) \
	    --stubs _spp_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram fulladdtab
	$(CC) $(CFLAGS) difftest_spr16p.c $(M16T_SEAM) \
	    _spp.o _spp_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_spr16p
	@./difftest_spr16p

t8bt: difftest_t8bt.c difftest.h $(M16T_SEAM) ../video/c_mv16draw.c
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8bt.o $(M16T_ORIG) \
	    --stub-routine draw16x816t \
	    --stubs _t8bt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b draw8x816boffset draw16x816t_stub \
	    xtravbuf transpbuf vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_t8bt.c $(M16T_SEAM) ../video/c_mv16draw.c \
	    _t8bt.o _t8bt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_t8bt
	@./difftest_t8bt

t16bt: difftest_t16bt.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16bt.o $(M16T_ORIG) \
	    --stubs _t16bt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_t16bt.c $(M16T_SEAM) \
	    _t16bt.o _t16bt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_t16bt
	@./difftest_t16bt

# c_mv16draw.c but not c_mv16leaf.c: the test defines draw16x816t and
# draw8x816toffset itself, to see which way the dispatch went.
t8t: difftest_t8t.c difftest.h $(M16T_SEAM) ../video/c_mv16draw.c
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8t.o $(M16T_ORIG) \
	    --stub-routine draw16x816t draw8x816toffset \
	    --stubs _t8t_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b draw16x816t_stub draw8x816toffset_stub \
	    fulladdtab xtravbuf transpbuf vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_t8t.c $(M16T_SEAM) ../video/c_mv16draw.c \
	    _t8t.o _t8t_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_t8t
	@./difftest_t8t

t16t: difftest_t16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16t.o $(M16T_ORIG) \
	    --stubs _t16t_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_t16t.c $(M16T_SEAM) \
	    _t16t.o _t16t_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_t16t
	@./difftest_t16t

t8to: difftest_t8to.c difftest.h $(M16T_SEAM) ../video/c_mv16leaf.c
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8to.o $(M16T_ORIG) \
	    --stubs _t8to_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b cachesingle4bng fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_t8to.c $(M16T_SEAM) ../video/c_mv16leaf.c \
	    _t8to.o _t8to_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_t8to
	@./difftest_t8to

thi: difftest_thi.c difftest.h $(M16T_SEAM) ../video/c_mv16leaf.c
	python3 ../tools/mkoracle.py video/makev16t.asm -o _thi.o $(M16T_ORIG) \
	    --stubs _thi_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	$(CC) $(CFLAGS) difftest_thi.c $(M16T_SEAM) ../video/c_mv16leaf.c \
	    _thi.o _thi_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    $(NOPIE) -o difftest_thi
	@./difftest_thi

# newgfx16.asm - the *new* graphics engine, the one that runs by default.
NG_OBJS := ../endmem.o ../video/c_newgfx16data.o ../video/newgfx.o \
           ../video/makevid.o ../cpu/c_regsdata.o ../cpu/c_regswdata.o \
           ../video/c_ngtransp.o ../video/c_ng2gate.o ngtransp_extra.c

# Every newgfx16 difftest links a cur_ object built from the whole worktree
# file, so it needs all of that file's C entry points - the same rule as
# M16T_SEAM. c_ngprocbg.c also needs calldl16t's block, which m16t_extra.c has.
NG_SEAM := ../video/c_ngmosaic.c ../video/c_ngprocbg.c \
           ../video/c_ngline.c ../video/c_ngspr.c ../video/c_ngbg.c \
           ../video/c_ngframe.c m16t_extra.c

# No oracle: the loop has no symbol of its own, so difftest_sprwin.c carries a
# verbatim transcription of the assembly instead.
sprwin: difftest_sprwin.c difftest.h ../video/c_ngline.c
	$(CC) $(CFLAGS) difftest_sprwin.c ../video/c_ngline.c \
	    $(NG_OBJS) $(NOPIE) -o difftest_sprwin
	@./difftest_sprwin

ngspr: difftest_ngspr.c difftest.h ../video/c_ngspr.c
	$(CC) $(CFLAGS) difftest_ngspr.c ../video/c_ngspr.c \
	    $(NG_OBJS) $(NOPIE) -o difftest_ngspr
	@./difftest_ngspr

# procwindowback16t is a leaf: no arguments, and everything it touches is a
# window register or the run list. So the oracle needs nothing linked but a
# stub for each symbol the rest of makev16t.asm references.
#
# The revision selector is unlike every other target here. Before the port the
# macro was expanded inline inside processmode716t and had no symbol of its own,
# so "the newest revision without a thunk" is one that does not contain the
# routine at all - hence the empty --ported-marker, which turns that test off,
# and a --requires naming the label instead. That picks the last revision where
# the body was still assembly, which is what an oracle has to be.
procwin: difftest_procwin.c difftest.h ../video/c_procwin.c
	python3 ../tools/mkoracle.py video/makev16t.asm -o _procwin.o $(M16T_ORIG) \
	    --requires 'NEWSYM procwindowback16t' --ported-marker '' \
	    --stubs _procwin_stubs.c \
	    --exclude winon numwin windowdata wincolen scaddset \
	            winl1 winr1 winl2 winr2 makedualwincol
	$(CC) $(CFLAGS) difftest_procwin.c ../video/c_procwin.c \
	    _procwin.o _procwin_stubs.c $(NOPIE) -o difftest_procwin
	@./difftest_procwin

mos: difftest_mos.c difftest.h $(NG_SEAM)
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _mos.o \
	    --stubs _mos_stubs.c --provided-by $(NG_OBJS) \
	    --exclude vram xtravbuf pesimpng fulladdtab vidbuffer
	# No second oracle: the current side is C, and difftest_mos.c drives it
	# through the same MOS* block the thunk used to fill.
	$(CC) $(CFLAGS) difftest_mos.c $(NG_SEAM) \
	    _mos.o _mos_stubs.c $(NG_OBJS) \
	    $(NOPIE) -o difftest_mos
	@./difftest_mos

# The tail of newengine16b. The oracle is HEAD rather than the pre-port
# revision: the first two thirds of the routine are already C and the assembly
# reaches them by ccallv there, so both sides share those three and this
# compares only what was ported last - see difftest_ngeng.c.
ngeng: difftest_ngeng.c difftest.h ../video/c_ngline.c
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _ngeng.o \
	    --requires 'ccallv newengine16b_lines' --ported-marker '' \
	    --stubs _ngeng_stubs.c --provided-by $(NG_OBJS) \
	    --exclude BackAreaFill BuildWindow2 setpalette16bng vram \
	            vbufdptr res640 scanlines hiresstuff Mode7HiRes16b \
	            ngwinptr CSprWinPtr fulladdtab vidbuffer \
	            newengine16b_lines newengine16b_windows newengine16b_sprwin
	$(CC) $(CFLAGS) difftest_ngeng.c ../video/c_ngline.c \
	    _ngeng.o _ngeng_stubs.c $(NG_OBJS) $(NOPIE) -o difftest_ngeng
	@./difftest_ngeng

# StartDrawNewGfx16b and the screen clip. The three workers it calls are
# recorded, so video/c_ngprocbg.c is not linked - see difftest_ngframe.c.
ngframe: difftest_ngframe.c difftest.h ../video/c_ngframe.c
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _ngframe.o \
	    --requires 'ccallv c_procbg16b' --ported-marker '' \
	    --stubs _ngframe_stubs.c --provided-by $(NG_OBJS) \
	    --exclude c_procbg16b c_procspr16b c_procmode7ng16b BuildWindow \
	            c_process_transparencies vidbuffer spritetablea \
	            fulladdtab prdata prdatb prdatc \
	            drawbg1line16b drawbg2line16b drawbg3line16b \
	            drawbg4line16b drawbg1tile16b drawbg2tile16b \
	            drawbg3tile16b drawbg4tile16b drawbg1linepr116b \
	            drawbg2linepr116b drawbg3linepr116b drawbg4linepr116b \
	            drawbg1tilepr116b drawbg2tilepr116b drawbg3tilepr116b \
	            drawbg4tilepr116b
	# video/c_ngtransp.o is deliberately not here: its
	# c_process_transparencies is one of the recorded targets.
	$(CC) $(CFLAGS) difftest_ngframe.c ../video/c_ngframe.c \
	    _ngframe.o _ngframe_stubs.c ../endmem.o \
	    ../video/c_newgfx16data.o ../video/newgfx.o ../video/makevid.o \
	    ../cpu/c_regsdata.o ../cpu/c_regswdata.o ../video/c_ng2gate.o \
	    ngtransp_extra.c $(NOPIE) -o difftest_ngframe
	@./difftest_ngframe

mv16: difftest_mv16.c difftest.h mkmv16.sh ../video/c_mv16tms.c
	./mkmv16.sh
	$(CC) $(CFLAGS) difftest_mv16.c ../video/c_mv16tms.c _mv16.o $(NOPIE) -o difftest_mv16
	@./difftest_mv16

m7draw: difftest_m7draw.c difftest.h mkm7draw.sh ../video/c_mode716draw.c \
       ../video/c_mode716gate.c
	./mkm7draw.sh
	$(CC) $(CFLAGS) difftest_m7draw.c ../video/c_mode716draw.c ../video/c_mode716gate.c ../video/c_mode716proc.c ../video/c_mode716win.c ../video/c_mode716bw.c ../video/c_mode716start.c _m7draw.o $(NOPIE) -o difftest_m7draw
	@./difftest_m7draw

m7proc: difftest_m7proc.c difftest.h mkm7proc.sh ../video/c_mode716proc.c
	./mkm7proc.sh
	$(CC) $(CFLAGS) difftest_m7proc.c ../video/c_mode716proc.c ../video/c_mode716win.c _m7proc.o $(NOPIE) -o difftest_m7proc
	@./difftest_m7proc

m7bw: difftest_m7bw.c difftest.h mkm7bw.sh ../video/c_mode716bw.c
	./mkm7bw.sh
	$(CC) $(CFLAGS) difftest_m7bw.c ../video/c_mode716bw.c _m7bw.o $(NOPIE) -o difftest_m7bw
	@./difftest_m7bw

m7win: difftest_m7win.c difftest.h mkm7win.sh ../video/c_mode716win.c
	./mkm7win.sh
	$(CC) $(CFLAGS) difftest_m7win.c ../video/c_mode716win.c _m7win.o $(NOPIE) -o difftest_m7win
	@./difftest_m7win

m7start: difftest_m7start.c difftest.h mkm7start.sh ../video/c_mode716start.c
	./mkm7start.sh
	$(CC) $(CFLAGS) difftest_m7start.c ../video/c_mode716start.c _m7start.o $(NOPIE) -o difftest_m7start
	@./difftest_m7start

m7ext2: difftest_m7ext2.c difftest.h mkm7ext2.sh ../video/c_mode716ext2.c
	./mkm7ext2.sh
	$(CC) $(CFLAGS) difftest_m7ext2.c ../video/c_mode716ext2.c _m7ext2.o $(NOPIE) -o difftest_m7ext2
	@./difftest_m7ext2

m7calc: difftest_m7calc.c difftest.h mkm7calc.sh ../video/c_mode716calc.c
	./mkm7calc.sh
	$(CC) $(CFLAGS) difftest_m7calc.c ../video/c_mode716calc.c _m7calc.o $(NOPIE) -o difftest_m7calc
	@./difftest_m7calc

regs: difftest_regs.c difftest.h mkregs.sh regs.list ../cpu/c_regsppu.c
	./mkregs.sh
	$(CC) $(CFLAGS) difftest_regs.c ../cpu/c_regsppu.c _regs.o $(NOPIE) -o difftest_regs
	@./difftest_regs

run: all
	@./$(BIN)
	@./$(NETPLAY)
	@./$(ST10)
	@./$(ST11)
	@./$(OBC1)
	@./$(MSU1)
	@./$(DSP2)
	@./$(SAI)
	@./$(SA1)
	@./$(SPC7110)
	@./$(VCACHE)
	@./$(SPRITES)
	@./$(C4)
	@./$(MODE7)
	@./$(ASMDATA)

# The SPC700 core run at both word sizes and compared. Every other test here
# builds -m32 only, where zreg and u4 are the same type, so a 64-bit width bug
# cannot show up; this one exists to catch exactly that.
spc64: spc64_test.c ../cpu/spc_ops.h ../cpu/spc_ioregs.h
	@$(CC) -m32 -std=gnu99 -I.. -O2 -Wall -o zsnes_spc32_test spc64_test.c
	@$(CC) -m64 -std=gnu99 -I.. -O2 -Wall -o zsnes_spc64_test spc64_test.c
	@a=$$(./zsnes_spc32_test) && b=$$(./zsnes_spc64_test) && \
	  if [ "$$a" = "$$b" ]; then echo "spc700 32 vs 64: PASS ($$a)"; \
	  else echo "spc700 32 vs 64: FAIL"; echo "  m32 $$a"; echo "  m64 $$b"; exit 1; fi

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7) $(ASMDATA)
	rm -f zsnes_spc32_test zsnes_spc64_test
	rm -f difftest_procwin difftest_spcdec difftest_fxtable difftest_fxops difftest_memops difftest_m7calc difftest_m7ext2 difftest_m7start difftest_m7win difftest_m7bw difftest_m7proc difftest_m7draw difftest_mv16 difftest_mvs difftest_regs _*.o _*.asm _*.inc _*.raw

# Port-time differential testing (difftest.h + mkdifftest.sh) proves a C port
# bit-identical to the original assembly. It is transient by design: the oracle
# is the original asm, so a routine's difftest is removed once its asm is
# deleted. The DSP mixers were verified this way, then their asm and difftest
# were retired; the reusable difftest.h and mkdifftest.sh remain for future
# ports (see git history for difftest_mixers.c and difftest_spcio.c as worked
# examples).


op: difftest_op.c difftest.h ../cpu/ops65816.h
	python3 ../tools/mkoracle.py cpu/table.asm -o _op.o \
	    --requires 'e65816.inc' --ported-marker 'EXTSYM c_COp' --rewrite-macro 'endloop=    ret' \
	    --stubs _op_stubs.c \
	    --exclude xa xx xy xs xd xdb xpb xpc xe flagnz flago flagc \
	            tablead stackand stackor UpdateDPage snesmmap snesmap2 \
	            membank0r8 membank0r16 membank0w8 membank0w16 memtabler8 memtabler16 \
	            DPageR8 DPageR16 memtablew8 memtablew16 DPageW8 DPageW16 \
	            brkv brkv8 copv copv8 xirqb intrset doirqnext curnmi \
	            nmistatus curexecstate initaddrl wramdata dmadata \
	            regaccessbankr8 execloop switchtovirq
	$(CC) $(CFLAGS) -I../cpu difftest_op.c _op.o _op_stubs.c $(NOPIE) -o difftest_op
	@./difftest_op

sa1op: difftest_sa1.c difftest_op.c difftest.h ../cpu/ops65816.h
	python3 ../tools/mkoracle.py cpu/stable.asm -o _sa1.o \
	    --requires 'se65816.inc' --ported-marker 'EXTSYM c_SA1COp' --rewrite-macro 'endloop=    ret' \
	    --stubs _sa1_stubs.c \
	    --exclude SA1xa SA1xx SA1xy SA1xs SA1xd SA1xdb SA1xpb xpc xe \
	            Sflagnz Sflago Sflagc SA1tablead stackand stackor \
	            SA1UpdateDPage snesmmap snesmap2 membank0r8 membank0r16 \
	            membank0w8 membank0w16 memtabler8 memtabler16 memtablew8 \
	            memtablew16 SA1DPageR8 SA1DPageR16 SA1DPageW8 SA1DPageW16 \
	            brkv brkv8 copv copv8 xirqb intrset doirqnext curnmi \
	            nmistatus curexecstate initaddrl wramdata dmadata \
	            regaccessbankr8 execloop switchtovirq
	$(CC) $(CFLAGS) -I../cpu difftest_sa1.c _sa1.o _sa1_stubs.c $(NOPIE) -o difftest_sa1
	@./difftest_sa1

dbgop: difftest_dbg.c difftest_op.c difftest.h ../cpu/ops65816_dbg.h
	python3 ../tools/mkoracle.py cpu/tablec.asm -o _dbg.o \
	    --requires 'e65816c.inc' --ported-marker 'EXTSYM c_dbgCOp' --rewrite-macro 'endloop=    ret' \
	    --stubs _dbg_stubs.c \
	    --exclude xa xx xy xs xd xdb xpb xpc xe flagnz flago flagc \
	            tableadc stackand stackor UpdateDPage snesmmap snesmap2 \
	            membank0r8 membank0r16 membank0w8 membank0w16 memtabler8 \
	            memtabler16 memtablew8 memtablew16 DPageR8 DPageR16 DPageW8 \
	            DPageW16 brkv brkv8 copv copv8 xirqb intrset doirqnext curnmi \
	            nmistatus curexecstate initaddrl wramdata dmadata \
	            regaccessbankr8 execloop switchtovirq
	$(CC) $(CFLAGS) -I../cpu difftest_dbg.c _dbg.o _dbg_stubs.c $(NOPIE) -o difftest_dbg
	@./difftest_dbg

transp: difftest_transp.c difftest.h ../video/c_ngtransp.c
	$(CC) $(CFLAGS) difftest_transp.c $(NOPIE) -o difftest_transp
	@./difftest_transp

# The four background tile dispatchers. The renderers they tail-jump into are
# recorded rather than run, so what this compares is the register state at the
# jump plus everything the dispatcher wrote - see difftest_bgt.c.
bgt: difftest_bgt.c difftest.h ../video/c_ngbg.c
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _bgt.o \
	    --stubs _bgt_stubs.c --provided-by $(NG_OBJS) \
	    --define BuildWindow=BuildWindow_reg \
	    --exclude drawtileng2b16b drawtileng4b16b drawtileng8b16b \
	            drawtileng16x162b16b drawtileng16x164b16b \
	            drawtileng16x168b16b BuildWindow_reg Gendcolortable \
	            colormodedef dcolortab vram fulladdtab vidbuffer \
	            drawlineng2b16b drawlineng4b16b \
	            drawlineng8b16b drawlineng16x162b16b \
	            drawlineng16x164b16b drawlineng16x168b16b \
	            drawlineng16x82b16b drawlineng16x84b16b \
	            drawlinengom2b16b drawlinengom4b16b \
	            drawlinengom8b16b drawlinengom16x162b16b \
	            drawlinengom16x164b16b drawlinengom16x168b16b
	$(CC) $(CFLAGS) difftest_bgt.c ../video/c_ngbg.c _bgt.o _bgt_stubs.c \
	    $(NG_OBJS) $(NOPIE) -o difftest_bgt
	@./difftest_bgt

# The worktree side is no longer assembly: video/newg162.asm is gone and the
# entry points are C, so difftest_ng2.c calls them directly and there is only
# one oracle here now.
ng2: difftest_ng2.c ng2_harness.c ng2_harness.h difftest.h \
     ../video/c_ng2tile.c ../video/c_ng2gate.c
	# --requires bounds the search from above now that the file is deleted:
	# the finished port has neither the .mac include nor the gate macros'
	# ccall, so the marker alone would select it. The include is the thing
	# only a revision with the renderers still in it has.
	python3 ../tools/mkoracle.py video/newg162.asm -o _ng2.o \
	    --requires '%include "video/newg162.mac"' \
	    --ported-marker 'ccall c_determinetransp' \
	    --stubs _ng2_stubs.c \
	    --provided-by ../video/tilecache.o ../endmem.o \
	    --exclude domosaicng16b vrama
	# The two ported files go in as *sources*. Linking the objects the main
	# build leaves behind means an edit to either can be tested against a
	# stale one - which happened, and showed up as an extern of the wrong
	# length reading whatever global followed it.
	$(CC) $(CFLAGS) difftest_ng2.c ng2_harness.c _ng2.o \
	    _ng2_stubs.c ../video/tilecache.o ../endmem.o ../video/c_ng2tile.c \
	    ../video/c_ng2gate.c $(NOPIE) -o difftest_ng2
	@./difftest_ng2
