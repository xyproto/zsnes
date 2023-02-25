cmake_minimum_required(VERSION 3.7)
project(zsnes)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -pthread -no-pie -std=gnu++14 -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIFY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} Wl,--as-needed -no-pie -L/usr/lib32 -Wl,--gc-sections -lz -ldl -lX11 -lGL")

# Find and use sdl and png
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL1 REQUIRED sdl)
PKG_SEARCH_MODULE(PNG REQUIRED png)
INCLUDE_DIRECTORIES(${SDL1_INCLUDE_DIRS} ${PKG_INCLUDE_DIRS)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL_LIBRARIES} ${PNG_LIBRARIES})

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(DIVISIBLE_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

# include_directories(${DIVISIBLE_INSTALL_INCLUDE_DIR})
# include_directories(${DIVISION_HEADERS_DIR})
# add_subdirectory(src)
# add_subdirectory(test)

add_executable(${PROJECT_NAME} "")
target_sources(${PROJECT_NAME} PRIVATE
c_init.c
c_vcache.c
chips/7110emu.c
chips/7110proc.asm
chips/c4emu.c
chips/c4proc.asm
chips/c_dsp2proc.c
chips/c_sa1regs.c
chips/c_sfxproc.c
chips/dsp1emu.c
chips/dsp1proc.asm
chips/dsp2proc.asm
chips/dsp3emu.c
chips/dsp3proc.asm
chips/dsp4emu.c
chips/dsp4proc.asm
chips/fxemu2.asm
chips/fxemu2b.asm
chips/fxemu2c.asm
chips/fxtable.asm
chips/obc1emu.c
chips/obc1proc.asm
chips/sa1emu.c
chips/sa1proc.asm
chips/sa1regs.asm
chips/sdd1emu.c
chips/seta10.c
chips/seta11.c
chips/sfxproc.asm
chips/st10proc.asm
chips/st11proc.asm
cpu/c_65816d.c
cpu/c_dma.c
cpu/c_dsp.c
cpu/c_dspproc.c
cpu/c_execute.c
cpu/c_irq.c
cpu/c_memory.c
cpu/c_regs.c
cpu/c_regsw.c
cpu/c_table.c
cpu/c_tablec.c
cpu/dma.asm
cpu/dsp.asm
cpu/dspproc.asm
cpu/execute.asm
cpu/executec.c
cpu/irq.asm
cpu/memory.asm
cpu/memtable.c
cpu/spc700.asm
cpu/stable.asm
cpu/table.asm
cpu/tablec.asm
effects/burn.c
effects/smoke.c
effects/water.c
endmem.asm
gui/c_gui.c
gui/c_guiwindp.c
gui/gui.asm
gui/guicheat.c
gui/guicombo.c
gui/guifuncs.c
gui/guikeys.c
gui/guimisc.c
gui/guimouse.c
gui/guitools.c
gui/menu.c
init.asm
initc.c
mmlib/mm.c
patch.c
ui.c
vcache.asm
ver.c
video/2xsaiw.asm
video/c_2xsaiw.c
video/c_copyvid.c
video/c_makev16b.c
video/c_makevid.c
video/c_mode716.c
video/c_newgfx16.c
video/copyvid.asm
video/copyvwin.c
video/hq2x16.asm
video/hq2x32.asm
video/hq3x16.asm
video/hq3x32.asm
video/hq4x16.asm
video/hq4x32.asm
video/m716text.asm
video/makev16b.asm
video/makev16t.asm
video/makevid.asm
video/mode716.asm
video/mode716b.asm
video/mode716d.asm
video/mode716e.asm
video/mode716t.asm
video/mv16tms.asm
video/newg162.asm
video/newgfx.asm
video/newgfx16.asm
video/ntsc.c
video/procvid.c
video/procvidc.c
video/sw_draw.c
zdir.c
zip/unzip.c
zip/zpng.c
zloader.c
zmovie.c
zpath.c
zstate.c
ztimec.c
jma/7zlzma.cpp
jma/crc32.cpp
jma/iiostrm.cpp
jma/inbyte.cpp
jma/jma.cpp
jma/lzma.cpp
jma/lzmadec.cpp
jma/winout.cpp
jma/zsnesjma.cpp
linux/audio.c
linux/battery.c
linux/c_sdlintrf.c
linux/lib.c
linux/safelib.c
linux/sdlintrf.asm
linux/sdllink.c
linux/sockserv.c
linux/sw_draw.c
linux/gl_draw.c
mmlib/linux.c
)
