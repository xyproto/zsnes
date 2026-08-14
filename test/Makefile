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

CC     := gcc
CFLAGS := -m32 -std=gnu99 -I.. -Wall -Wextra -Wno-unused-parameter

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

.PHONY: all run clean spcdec fxtable fxops memops m7calc m7ext2 m7start m7win m7bw m7proc m7draw mv16 mvs mvall m716t cb16t cb16b spr16bt spr16t spr16p t8bt t16bt t8t t16t t8to thi mos sprwin regs

all: $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7) $(ASMDATA)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) -no-pie -Wl,--gc-sections -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) -no-pie -lpthread -o $(NETPLAY)

$(ST10): $(ST10_SRC) zstest.h ../chips/st10proc.c
	$(CC) $(CFLAGS) $(ST10_SRC) ../chips/st10proc.c -no-pie -o $(ST10)

$(ST11): $(ST11_SRC) zstest.h ../chips/st11proc.c
	$(CC) $(CFLAGS) $(ST11_SRC) ../chips/st11proc.c -no-pie -o $(ST11)

$(OBC1): $(OBC1_SRC) zstest.h ../chips/obc1proc.c
	$(CC) $(CFLAGS) $(OBC1_SRC) ../chips/obc1proc.c -no-pie -o $(OBC1)

$(MSU1): $(MSU1_SRC) zstest.h ../chips/msu1regs.c
	$(CC) $(CFLAGS) $(MSU1_SRC) ../chips/msu1regs.c -no-pie -o $(MSU1)

$(DSP2): $(DSP2_SRC) zstest.h ../chips/dsp2proc.c
	$(CC) $(CFLAGS) $(DSP2_SRC) ../chips/dsp2proc.c -no-pie -o $(DSP2)

$(SAI): $(SAI_SRC) zstest.h ../video/2xsaiw.c
	$(CC) $(CFLAGS) $(SAI_SRC) ../video/2xsaiw.c -no-pie -o $(SAI)

$(SA1): $(SA1_SRC) zstest.h ../chips/sa1regs.c
	$(CC) $(CFLAGS) $(SA1_SRC) ../chips/sa1regs.c -no-pie -o $(SA1)

$(SPC7110): $(SPC7110_SRC) zstest.h ../chips/7110proc.c
	$(CC) $(CFLAGS) $(SPC7110_SRC) ../chips/7110proc.c -no-pie -o $(SPC7110)

$(C4): $(C4_SRC) zstest.h ../chips/c4proc.c ../chips/c4emu.c
	$(CC) $(CFLAGS) $(C4_SRC) ../chips/c4proc.c ../chips/c4emu.c -no-pie -lm -o $(C4)

$(MODE7): $(MODE7_SRC) zstest.h ../video/mode716b.c
	$(CC) $(CFLAGS) $(MODE7_SRC) ../video/mode716b.c -no-pie -o $(MODE7)

# ../endmem.o supplies winbgdata, which makevid's cwinptr points at.
$(ASMDATA): $(ASMDATA_SRC) zstest.h ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c ../endmem.o
	$(CC) $(CFLAGS) $(ASMDATA_SRC) ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c ../endmem.o -no-pie -o $(ASMDATA)

$(VCACHE): $(VCACHE_SRC) zstest.h ../video/tilecache.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(VCACHE_SRC) ../video/tilecache.c -no-pie -Wl,--gc-sections -o $(VCACHE)

$(SPRITES): $(SPRITES_SRC) zstest.h ../video/sprites.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(SPRITES_SRC) ../video/sprites.c -no-pie -Wl,--gc-sections -o $(SPRITES)

# Differential test for the SPC700 DAA/DAS opcodes against the assembly they
# replaced (cff2b437^). Not part of `all` - it needs that old assembly.
spcdec: difftest_spcdec.c difftest.h mkdifftest.sh ../cpu/spc_ops.h
	git -C .. show cff2b437^:cpu/spc700.asm > /tmp/spc_old.asm
	./mkdifftest.sh _spcdec /tmp/spc_old.asm "OpBE OpDF" \
	    "spcA spcNZ spcP" 439-459 1733-1781
	$(CC) $(CFLAGS) difftest_spcdec.c _spcdec.o -no-pie -o difftest_spcdec
	@./difftest_spcdec

# Differential test for the SuperFX table builder against the assembly it
# replaced. Not part of `all` - it needs that old assembly out of git history.
fxtable: difftest_fxtable.c mkfxtable.sh ../chips/c_fxtable.c ../endmem.o
	./mkfxtable.sh
	$(CC) $(CFLAGS) -fcommon -fno-pic -c difftest_fxtable.c -o _fxtest.o
	$(CC) $(CFLAGS) -fcommon -fno-pic -c ../chips/c_fxtable.c -o _fxtable.o
	$(CC) -m32 -no-pie -fno-pic _fxtest.o _fxtable.o _fxold.o _fxstubs.o ../endmem.o -o difftest_fxtable
	@./difftest_fxtable

# Differential test for the SuperFX opcode handlers against the assembly they
# replaced. Not part of `all` - it needs that assembly out of git history.
fxops: difftest_fxops.c difftest.h mkfxops.sh ../chips/fx_ops.h
	./mkfxops.sh
	$(CC) $(CFLAGS) difftest_fxops.c _fxops.o -no-pie -o difftest_fxops
	@./difftest_fxops

# Differential test for the direct-page memory handlers against the assembly
# they replaced. Not part of `all` - it needs that assembly out of git history.
memops: difftest_memops.c difftest.h mkmemops.sh memops.list ../cpu/mem_ops.h
	./mkmemops.sh
	$(CC) $(CFLAGS) difftest_memops.c _memops.o -no-pie -o difftest_memops
	@./difftest_memops

mvs: difftest_mvs.c difftest.h mkmvs.sh ../video/c_mv16tsms.c
	./mkmvs.sh
	$(CC) $(CFLAGS) difftest_mvs.c ../video/c_mv16tsms.c _mvs.o -no-pie -o difftest_mvs
	@./difftest_mvs

# The oracle is the whole pre-port file, so this one covers every routine in it
# and needs no extraction script; see tools/mkoracle.py.
mvall: difftest_mvall.c difftest.h ../video/c_mv16tsms.c ../video/c_mv16tms.c
	python3 ../tools/mkoracle.py video/mv16tms.asm -o _mvall.o
	$(CC) $(CFLAGS) difftest_mvall.c ../video/c_mv16tsms.c ../video/c_mv16tms.c \
	    ../video/mv16tms.o _mvall.o -no-pie -o difftest_mvall
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
M16T_SEAM := ../video/c_mv16tm7.c ../video/c_mv16tclr.c \
             ../video/c_mv16bclr.c ../video/c_mv16tspr.c \
             ../video/c_mv16tsprt.c ../video/c_mv16tsprp.c \
             ../video/c_mv16t8bt.c ../video/c_mv16t16bt.c \
             ../video/c_mv16t8t.c ../video/c_mv16t16t.c \
             ../video/c_mv16t8to.c ../video/c_mv16thi.c m16t_extra.c
M16T_OBJS := ../cpu/c_regsdata.o ../cpu/c_regswdata.o \
             ../video/c_mode716data.o ../video/makevid.o ../endmem.o
M16T_BSS  := ../video/c_makev16tdata.o

m716t: difftest_m716t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _m716t.o \
	    --stub-routine $(M716T_STUBBED) \
	    --stubs _m716t_stubs.c --provided-by $(M16T_OBJS) \
	    --exclude vram $(M716T_TAILS)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _m716tcur.o \
	    --worktree --prefix cur_ --stub-routine $(M716T_STUBBED)
	$(CC) $(CFLAGS) difftest_m716t.c $(M16T_SEAM) \
	    _m716t.o _m716tcur.o _m716t_stubs.c $(M16T_OBJS) \
	    -no-pie -o difftest_m716t
	@./difftest_m716t

cb16t: difftest_cb16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16t.o \
	    --stubs _cb16t_stubs.c --provided-by $(M16T_OBJS) \
	    --exclude vram transpbuf fulladdtab
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16tcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_cb16t.c $(M16T_SEAM) \
	    _cb16t.o _cb16tcur.o _cb16t_stubs.c $(M16T_OBJS) \
	    -no-pie -o difftest_cb16t
	@./difftest_cb16t

cb16b: difftest_cb16b.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16b.o \
	    --stubs _cb16b_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram
	python3 ../tools/mkoracle.py video/makev16t.asm -o _cb16bcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_cb16b.c $(M16T_SEAM) \
	    _cb16b.o _cb16bcur.o _cb16b_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_cb16b
	@./difftest_cb16b

spr16bt: difftest_spr16bt.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spr.o \
	    --stubs _spr_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram
	python3 ../tools/mkoracle.py video/makev16t.asm -o _sprcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_spr16bt.c $(M16T_SEAM) \
	    _spr.o _sprcur.o _spr_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_spr16bt
	@./difftest_spr16bt

spr16t: difftest_spr16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spt.o \
	    --stubs _spt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram fulladdtab
	python3 ../tools/mkoracle.py video/makev16t.asm -o _sptcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_spr16t.c $(M16T_SEAM) \
	    _spt.o _sptcur.o _spt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_spr16t
	@./difftest_spr16t

spr16p: difftest_spr16p.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _spp.o \
	    --stubs _spp_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram fulladdtab
	python3 ../tools/mkoracle.py video/makev16t.asm -o _sppcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_spr16p.c $(M16T_SEAM) \
	    _spp.o _sppcur.o _spp_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_spr16p
	@./difftest_spr16p

t8bt: difftest_t8bt.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8bt.o \
	    --stub-routine draw16x816t \
	    --stubs _t8bt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b draw8x816boffset draw16x816t_stub \
	    xtravbuf transpbuf vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8btcur.o \
	    --worktree --prefix cur_ --stub-routine draw16x816t
	$(CC) $(CFLAGS) difftest_t8bt.c $(M16T_SEAM) \
	    _t8bt.o _t8btcur.o _t8bt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_t8bt
	@./difftest_t8bt

t16bt: difftest_t16bt.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16bt.o \
	    --stubs _t16bt_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16btcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_t16bt.c $(M16T_SEAM) \
	    _t16bt.o _t16btcur.o _t16bt_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_t16bt
	@./difftest_t16bt

t8t: difftest_t8t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8t.o \
	    --stub-routine draw16x816t draw8x816toffset \
	    --stubs _t8t_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b draw16x816t_stub draw8x816toffset_stub \
	    fulladdtab xtravbuf transpbuf vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8tcur.o \
	    --worktree --prefix cur_ \
	    --stub-routine draw16x816t draw8x816toffset
	$(CC) $(CFLAGS) difftest_t8t.c $(M16T_SEAM) \
	    _t8t.o _t8tcur.o _t8t_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_t8t
	@./difftest_t8t

t16t: difftest_t16t.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16t.o \
	    --stubs _t16t_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t16tcur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_t16t.c $(M16T_SEAM) \
	    _t16t.o _t16tcur.o _t16t_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_t16t
	@./difftest_t16t

t8to: difftest_t8to.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8to.o \
	    --stubs _t8to_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b cachesingle4bng fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _t8tocur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_t8to.c $(M16T_SEAM) \
	    _t8to.o _t8tocur.o _t8to_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_t8to
	@./difftest_t8to

thi: difftest_thi.c difftest.h $(M16T_SEAM)
	python3 ../tools/mkoracle.py video/makev16t.asm -o _thi.o \
	    --stubs _thi_stubs.c --provided-by $(M16T_OBJS) $(M16T_BSS) \
	    --exclude vram domosaic16b fulladdtab xtravbuf transpbuf \
	    vcache2b vcache4b vcache8b
	python3 ../tools/mkoracle.py video/makev16t.asm -o _thicur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_thi.c $(M16T_SEAM) \
	    _thi.o _thicur.o _thi_stubs.c $(M16T_OBJS) $(M16T_BSS) \
	    -no-pie -o difftest_thi
	@./difftest_thi

# newgfx16.asm - the *new* graphics engine, the one that runs by default.
NG_OBJS := ../endmem.o ../video/c_newgfx16data.o ../video/newgfx.o \
           ../video/makevid.o ../cpu/c_regsdata.o ../cpu/c_regswdata.o

# Every newgfx16 difftest links a cur_ object built from the whole worktree
# file, so it needs all of that file's C entry points - the same rule as
# M16T_SEAM. c_ngprocbg.c also needs calldl16t's block, which m16t_extra.c has.
NG_SEAM := ../video/c_ngmosaic.c ../video/c_ngprocbg.c \
           ../video/c_ngline.c m16t_extra.c

# No oracle: the loop has no symbol of its own, so difftest_sprwin.c carries a
# verbatim transcription of the assembly instead.
sprwin: difftest_sprwin.c difftest.h ../video/c_ngline.c
	$(CC) $(CFLAGS) difftest_sprwin.c ../video/c_ngline.c \
	    $(NG_OBJS) -no-pie -o difftest_sprwin
	@./difftest_sprwin

mos: difftest_mos.c difftest.h $(NG_SEAM)
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _mos.o \
	    --stubs _mos_stubs.c --provided-by $(NG_OBJS) \
	    --exclude vram xtravbuf pesimpng
	python3 ../tools/mkoracle.py video/newgfx16.asm -o _moscur.o \
	    --worktree --prefix cur_
	$(CC) $(CFLAGS) difftest_mos.c $(NG_SEAM) \
	    _mos.o _moscur.o _mos_stubs.c $(NG_OBJS) \
	    -no-pie -o difftest_mos
	@./difftest_mos

mv16: difftest_mv16.c difftest.h mkmv16.sh ../video/c_mv16tms.c
	./mkmv16.sh
	$(CC) $(CFLAGS) difftest_mv16.c ../video/c_mv16tms.c _mv16.o -no-pie -o difftest_mv16
	@./difftest_mv16

m7draw: difftest_m7draw.c difftest.h mkm7draw.sh ../video/c_mode716draw.c
	./mkm7draw.sh
	$(CC) $(CFLAGS) difftest_m7draw.c ../video/c_mode716draw.c ../video/c_mode716proc.c ../video/c_mode716win.c ../video/c_mode716bw.c ../video/c_mode716start.c _m7draw.o -no-pie -o difftest_m7draw
	@./difftest_m7draw

m7proc: difftest_m7proc.c difftest.h mkm7proc.sh ../video/c_mode716proc.c
	./mkm7proc.sh
	$(CC) $(CFLAGS) difftest_m7proc.c ../video/c_mode716proc.c ../video/c_mode716win.c _m7proc.o -no-pie -o difftest_m7proc
	@./difftest_m7proc

m7bw: difftest_m7bw.c difftest.h mkm7bw.sh ../video/c_mode716bw.c
	./mkm7bw.sh
	$(CC) $(CFLAGS) difftest_m7bw.c ../video/c_mode716bw.c _m7bw.o -no-pie -o difftest_m7bw
	@./difftest_m7bw

m7win: difftest_m7win.c difftest.h mkm7win.sh ../video/c_mode716win.c
	./mkm7win.sh
	$(CC) $(CFLAGS) difftest_m7win.c ../video/c_mode716win.c _m7win.o -no-pie -o difftest_m7win
	@./difftest_m7win

m7start: difftest_m7start.c difftest.h mkm7start.sh ../video/c_mode716start.c
	./mkm7start.sh
	$(CC) $(CFLAGS) difftest_m7start.c ../video/c_mode716start.c _m7start.o -no-pie -o difftest_m7start
	@./difftest_m7start

m7ext2: difftest_m7ext2.c difftest.h mkm7ext2.sh ../video/c_mode716ext2.c
	./mkm7ext2.sh
	$(CC) $(CFLAGS) difftest_m7ext2.c ../video/c_mode716ext2.c _m7ext2.o -no-pie -o difftest_m7ext2
	@./difftest_m7ext2

m7calc: difftest_m7calc.c difftest.h mkm7calc.sh ../video/c_mode716calc.c
	./mkm7calc.sh
	$(CC) $(CFLAGS) difftest_m7calc.c ../video/c_mode716calc.c _m7calc.o -no-pie -o difftest_m7calc
	@./difftest_m7calc

regs: difftest_regs.c difftest.h mkregs.sh regs.list ../cpu/c_regsppu.c
	./mkregs.sh
	$(CC) $(CFLAGS) difftest_regs.c ../cpu/c_regsppu.c _regs.o -no-pie -o difftest_regs
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

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7) $(ASMDATA)
	rm -f difftest_spcdec difftest_fxtable difftest_fxops difftest_memops difftest_m7calc difftest_m7ext2 difftest_m7start difftest_m7win difftest_m7bw difftest_m7proc difftest_m7draw difftest_mv16 difftest_mvs difftest_regs _*.o _*.asm _*.inc _*.raw

# Port-time differential testing (difftest.h + mkdifftest.sh) proves a C port
# bit-identical to the original assembly. It is transient by design: the oracle
# is the original asm, so a routine's difftest is removed once its asm is
# deleted. The DSP mixers were verified this way, then their asm and difftest
# were retired; the reusable difftest.h and mkdifftest.sh remain for future
# ports (see git history for difftest_mixers.c and difftest_spcio.c as worked
# examples).

