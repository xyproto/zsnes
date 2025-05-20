project('zsnes', ['c', 'cpp', 'nasm'], version: '2.0.12', meson_version: '>=1.5', default_options: {'pkg_config_path': '/usr/lib32/pkgconfig'})

zlib = dependency('zlib')
sdl = dependency('sdl')
x11 = dependency('x11')

if get_option('with-debugger')
    ncurses = dependency('ncurses')
endif

if get_option('with-png')
    libpng = dependency('libpng')
endif

parsegen = executable('parsegen', ['parsegen.cpp'], dependencies: [zlib], cpp_args: ['-m32'], link_args: ['-m32', '-std=gnu++14'])

parsegen_sources = []

foreach parsegen_name : ['cfg', 'input', 'md']
    current_target = custom_target(
        parsegen_name + '_parsegen',
        input: [parsegen_name + '.psr'],
        output: [parsegen_name + '.c', parsegen_name + '.h'],
        command: [parsegen, '-D__UNIXSDL__', '-cheader', parsegen_name + '.h', '-fname', parsegen_name, parsegen_name + '.c', '@INPUT@'],
        depends: [parsegen]
    )
    parsegen_sources += current_target
endforeach

sources = [
    'c_init.c',
    'c_vcache.c',
    'chips/7110emu.c',
    'chips/7110proc.asm',
    'chips/msu1emu.c',
    'chips/msu1regs.asm',
    'chips/c4emu.c',
    'chips/c4proc.asm',
    'chips/c_dsp2proc.c',
    'chips/c_sa1regs.c',
    'chips/c_sfxproc.c',
    'chips/dsp1emu.c',
    'chips/dsp1proc.asm',
    'chips/dsp2proc.asm',
    'chips/dsp3emu.c',
    'chips/dsp3proc.asm',
    'chips/dsp4emu.c',
    'chips/dsp4proc.asm',
    'chips/fxemu2.asm',
    'chips/fxemu2b.asm',
    'chips/fxemu2c.asm',
    'chips/fxtable.asm',
    'chips/obc1emu.c',
    'chips/obc1proc.asm',
    'chips/sa1emu.c',
    'chips/sa1proc.asm',
    'chips/sa1regs.asm',
    'chips/sdd1emu.c',
    'chips/seta10.c',
    'chips/seta11.c',
    'chips/sfxproc.asm',
    'chips/st10proc.asm',
    'chips/st11proc.asm',
    'cpu/c_65816d.c',
    'cpu/c_dma.c',
    'cpu/c_dsp.c',
    'cpu/c_dspproc.c',
    'cpu/c_execute.c',
    'cpu/c_irq.c',
    'cpu/c_memory.c',
    'cpu/c_regs.c',
    'cpu/c_regsw.c',
    'cpu/c_table.c',
    'cpu/c_tablec.c',
    'cpu/dma.asm',
    'cpu/dsp.asm',
    'cpu/dspproc.asm',
    'cpu/execute.asm',
    'cpu/executec.c',
    'cpu/irq.asm',
    'cpu/memory.asm',
    'cpu/memtable.c',
    'cpu/spc700.asm',
    'cpu/stable.asm',
    'cpu/table.asm',
    'cpu/tablec.asm',
    'effects/burn.c',
    'effects/smoke.c',
    'effects/water.c',
    'endmem.asm',
    'gui/c_gui.c',
    'gui/c_guiwindp.c',
    'gui/gui.asm',
    'gui/guicheat.c',
    'gui/guicombo.c',
    'gui/guifuncs.c',
    'gui/guikeys.c',
    'gui/guimisc.c',
    'gui/guimouse.c',
    'gui/guitools.c',
    'gui/menu.c',
    'init.asm',
    'initc.c',
    'mmlib/mm.c',
    'patch.c',
    'ui.c',
    'vcache.asm',
    'ver.c',
    'video/2xsaiw.asm',
    'video/c_2xsaiw.c',
    'video/c_makev16b.c',
    'video/c_makevid.c',
    'video/c_mode716.c',
    'video/c_newgfx16.c',
    'video/copyvwin.c',
    'video/hq2x16.asm',
    'video/hq2x32.asm',
    'video/hq3x16.asm',
    'video/hq3x32.asm',
    'video/hq4x16.asm',
    'video/hq4x32.asm',
    'video/m716text.asm',
    'video/makev16b.asm',
    'video/makev16t.asm',
    'video/makevid.asm',
    'video/mode716.asm',
    'video/mode716b.asm',
    'video/mode716d.asm',
    'video/mode716e.asm',
    'video/mode716t.asm',
    'video/mv16tms.asm',
    'video/newg162.asm',
    'video/newgfx.asm',
    'video/newgfx16.asm',
    'video/ntsc.c',
    'video/procvid.c',
    'video/procvidc.c',
    'video/sw_draw.c',
    'zdir.c',
    'zip/unzip.c',
    'zip/zpng.c',
    'zloader.c',
    'zmovie.c',
    'zpath.c',
    'zstate.c',
    'ztimec.c',
    'linux/audio.c',
    'linux/battery.c',
    'linux/c_sdlintrf.c',
    'linux/lib.c',
    'linux/safelib.c',
    'linux/sdlintrf.asm',
    'linux/sdllink.c',
    'linux/sockserv.c',
    'linux/sw_draw.c',
    'mmlib/linux.c',
]

if get_option('with-debugger')
    sources += [
        'debugasm.c',
        'debugger.c',
    ]
endif

if get_option('with-opengl')
    sources += [
        'linux/gl_draw.c',
    ]
endif

if get_option('with-jma')
    sources += [
        'jma/7zlzma.cpp',
        'jma/crc32.cpp',
        'jma/iiostrm.cpp',
        'jma/inbyte.cpp',
        'jma/jma.cpp',
        'jma/lzma.cpp',
        'jma/lzmadec.cpp',
        'jma/winout.cpp',
        'jma/zsnesjma.cpp',
    ]
endif

dependencies = [
    sdl,
    zlib,
    x11,
]

c_args = [
    '-m32', 
    '-pthread',
    '-no-pie',
    '-std=gnu99',
    '-fcommon',
    '-O1',
     '-march=pentium-mmx',
    '-fno-inline',
    '-fno-pic',
    '-mtune=generic',
    '-mmmx',
    '-D_FORTIY_SOURCE=2',
    '-mno-sse',
    '-mno-sse2',
    '-ffunction-sections',
    '-fdata-sections',
    '-D__UNIXSDL__',
]

cpp_args = [
    '-m32',
    '-pthread',
    '-no-pie',
    '-std=gnu++14',
    '-O1',
    '-march=pentium-mmx',
    '-fno-inline',
    '-fno-pic',
    '-mtune=generic',
    '-mmmx' ,
    '-D_FORTIFY_SOURCE=2',
    '-mno-sse',
    '-mno-sse2',
    '-ffunction-sections',
    '-fdata-sections',
]

asm_args = [
    '-O1',
    '-w-orphan-labels',
    '-felf32',
    '-DELF',
]

link_args = [
    '-m32',
    '-lm',
    '-no-pie',
]

if get_option('with-debugger')
    dependencies += ncurses
else
    c_args += '-DNO_DEBUGGER'
    asm_args += '-DNO_DEBUGGER'
endif

if not get_option('with-jma')
    c_flags += '-DNO_JMA'
endif

if get_option('with-opengl')
    c_args += '-D__OPENGL__'
    link_args += '-lGL'
endif

if get_option('with-png')
    dependencies += libpng
else
    c_args += '-DNO_PNG'
endif

executable('zsnes', sources, parsegen_sources,
    dependencies: dependencies,
    c_args: c_args,
    cpp_args: cpp_args,
    nasm_args: asm_args,
    link_args: link_args,
    install: true,
)

foreach size : ['16', '32', '48', '64', '128']
    install_data(
        'icons/' + size + 'x' + size + 'x32.png', 
        rename : 'io.github.xyproto.zsnes.png',
        install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', size + 'x' + size, 'apps'),
    )
endforeach

install_data('linux/zsnes.desktop', rename: 'io.github.xyproto.zsnes.desktop', install_dir: join_paths(get_option('datadir'), 'applications'))
install_data('linux/io.github.xyproto.zsnes.metainfo.xml', install_dir: join_paths(get_option('datadir'), 'metainfo'))
install_data('man/zsnes.1', install_dir: join_paths(get_option('mandir'), 'man1'))
