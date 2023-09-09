cmake_minimum_required(VERSION 3.8)
project(zsnes C CXX ASM_NASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

set(BINARY zsnes)
set(PSR parsegen)
set(ASM nasm)

 set(CMAKE_ASM_NASM_FLAGS "-O1 -w-orphan-labels -felf32 -DELF")
set(CMAKE_C_FLAGS "-m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic")
set(CMAKE_CXX_FLAGS "-m32 -pthread -no-pie -std=gnu++14 -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIFY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic")
set(CMAKE_LINKER_FLAGS "-Wl,--as-needed -no-pie -L/usr/lib32 -Wl,--gc-sections -lz -ldl -lX11 -lGL")

set(PSRFLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(DIVISIBLE_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

enable_language(ASM_NASM)


add_library(c_init OBJECT c_init.c asm_call.h c_init.h types.h c_intrf.h cfg.h
 cpu/c_execute.h cpu/../types.h cpu/c_regs.h cpu/c_regsw.h cpu/c_table.h
 cpu/c_tablec.h cpu/execute.h cpu/regs.h cpu/../macros.h cpu/stable.h
 debugger.h gui/c_gui.h gui/../types.h gui/c_guiwindp.h gui/gui.h
 gui/../macros.h gui/guikeys.h gui/guiwindp.h init.h initc.h input.h
 link.h macros.h ui.h video/c_mode716.h video/procvid.h video/../types.h
 zmovie.h gblvars.h zpath.h zip/zunzip.h jma/zsnesjma.h zstate.h)
target_include_directories(c_init PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:c_init>)
target_link_libraries(${PROJECT_NAME} c_init)


add_compile_definitions(NO_AO NO_DEBUGGER __OPENGL__ __UNIXSDL__)

# Find and use sdl and png
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL1 REQUIRED sdl)
PKG_SEARCH_MODULE(PNG REQUIRED libpng)
INCLUDE_DIRECTORIES(${SDL1_INCLUDE_DIRS} ${PKG_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL_LIBRARIES} ${PNG_LIBRARIES})

add_executable(parsegen parsegen.cpp)
target_link_libraries(parsegen LINK_PUBLIC z)

add_custom_target(generate_files DEPENDS cfg.h input.h md.h)
add_custom_command(
    OUTPUT cfg.h cfg.o
    COMMAND parsegen -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -gcc cc -compile -flags '-m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I/usr/include/libpng16  -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__' -cheader cfg.h -fname cfg cfg.o cfg.psr
    WORKDIR ${PROJECT_SOURCE_DIR}
    DEPENDS parsegen
)
add_custom_command(
    OUTPUT input.h input.o
    COMMAND parsegen -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -gcc cc -compile -flags '-m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I/usr/include/libpng16  -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__' -cheader input.h -fname input input.o input.psr
    WORKDIR ${PROJECT_SOURCE_DIR}
    DEPENDS parsegen
)
add_custom_command(
    OUTPUT md.h md.o
    COMMAND parsegen -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -gcc cc -compile -flags '-m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I/usr/include/libpng16  -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__' -cheader md.h -fname md md.o md.psr
    WORKDIR ${PROJECT_SOURCE_DIR}
    DEPENDS parsegen
)

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
cpu/regs.mac
cpu/regsw.mac
macros.mac
video/mode7.mac
video/mode716.mac
video/newg162.mac
video/newg16wn.mac
video/newgfx.mac
video/newgfx16.mac
video/newgfx2.mac
video/newgfxwn.mac
video/vidmacro.mac
chips/fxemu2.mac
chips/fxemu2b.mac
chips/fxemu2c.mac
)

# Set the project root directory
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Add the project root directory as a source directory
include_directories(${PROJECT_ROOT})
