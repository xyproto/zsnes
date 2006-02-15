# SConstruct file for ZSNES
# Run 'scons' in this directory to build.
# Run 'scons -c' in this directory to cleanup.

import os
env = Environment ()
platform = env['PLATFORM']

parsegen_tool = env.Program(target= 'parsegen', source = 'parsegen.cpp')

def parsegen_emitter(target,source,env):
	env.Depends(target,parsegen_tool)
	return (target,source)
	
if platform == 'posix':
	os.system('rm cfgparse.c version.o')
	print 'cfgparse.c and version.o removed'
	psrbld = Builder(action ='./parsegen -D__UNIXSDL__ $TARGET $SOURCE',
		emitter = parsegen_emitter,
		suffix = '.c', src_suffix = '.psr')

env['BUILDERS']['PARSEGEN'] = psrbld	
env.PARSEGEN('cfgparse.c', 'cfgparse.psr')

objfix_tool = env.Program(target= 'objfix.exe', source= 'objfix.c')

def objfix_emitter(target,source,env):
	env.Depends(target,objfix_tool)
	return (target,source)

objbld = Builder(action = 'objfix $TARGET',
	emitter = objfix_emitter,
	suffix = '.obj', src_suffix = '.obj')

env['BUILDERS']['OBJFIX'] = objbld

chipssrc = Split('''
	chips/c4emu.c
	chips/dsp1emu.c
	chips/dsp1proc.asm
	chips/dsp2proc.asm
	chips/dsp4emu.c
	chips/dsp4proc.asm
	chips/fxemu2.asm
	chips/fxemu2b.asm
	chips/fxemu2c.asm
	chips/fxtable.asm
	chips/sa1proc.asm
	chips/sa1regs.asm
	chips/sdd1emu.c
	chips/seta10.c
	chips/sfxproc.asm
	chips/st10proc.asm
''')
#ztcp is no longer used
netsrc = Split('''
	net/ztcp.c
''')
cpusrc = Split('''
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
	cpu/tableb.asm
	cpu/tablec.asm
''')
dossrc = Split('''
	dos/debug.asm
	dos/joy.asm
	dos/vesa2.asm
''')
effectssrc = Split('''
	effects/burn.c
	effects/water.c
	effects/smoke.c
''')
guisrc = Split('''
	gui/gui.asm
	gui/guifuncs.c
	gui/menu.asm
''')
videosrc = Split('''
	video/makev16b.asm
	video/makev16t.asm
	video/makevid.asm
	video/mode716.asm
	video/mode716b.asm
	video/mode716d.asm
	video/mode716e.asm
	video/mode716t.asm
	video/mode7.asm
	video/mode7ext.asm
	video/mv16tms.asm
	video/newg162.asm
	video/newgfx16.asm
	video/newgfx2.asm
	video/newgfx.asm
	video/m716text.asm
	video/2xsaiw.asm
	video/procvid.asm
	video/procvidc.c
	video/sw_draw.asm
	video/hq2x16.asm
	video/hq2x32.asm
	video/hq3x16.asm
	video/hq3x32.asm
	video/hq4x16.asm
	video/hq4x32.asm
''')
zipsrc = Split('''
	zip/unzip.c
	zip/zpng.c
''')
jmasrc = Split('''
	jma/7zlzma.cpp
	jma/crc32.cpp
	jma/iiostrm.cpp
	jma/inbyte.cpp
	jma/zsnesjma.cpp
	jma/jma.cpp
	jma/lzma.cpp
	jma/lzmadec.cpp
	jma/winout.cpp
''')
basesrc = Split('''
	cfgparse.c
	cfgload.c
	endmem.asm
	init.asm
	initc.c
	uic.c
	patch.c
	ui.asm
	vcache.asm
	version.c
	zmovie.c
	zstate.c
	zloader.c
''')

linuxsrc = Split('''
	linux/copyvwin.asm
	linux/sdlintrf.asm
	linux/sdllink.c
	linux/sw_draw.c
	linux/zfilew.c
''')
winsrc = Split('''
	win/copyvwin.asm
	win/winintrf.asm
	win/winlink.cpp
	win/zfilew.c
	win/zipxw.c
''')

platform = env['PLATFORM']

# Setup environment for nasm
env.Replace (AS = 'nasm')
env.Replace (ASFLAGS = '-f elf ')

#sdl_config = Builder (action = 'sdl-config --include')

# Run config tests.
conf = Configure (env)

# Must have SDL to compile
if not conf.CheckLib ('SDL', 'SDL_Init'):
	print 'SDL not found! Please install SDL and try again.'
	Exit (1)

if not conf.CheckLib ('png'):
	print 'libpng not found! Please install libpng and try again.'
	Exit (1)

if not conf.CheckLib ('z'):
	print 'zlib not found! Please install zlib and try again.'
	Exit (1)

# Check for nasm's existence
if not conf.TryCompile ('db __NASM_VER__', '.asm'):
	print 'NASM not found! Please install NASM and try again.'
	Exit (1)

env = conf.Finish ()

# Perform any platform-specific initialization
if platform == 'posix':
	src = chipssrc  + cpusrc + dossrc + effectssrc + guisrc + videosrc + zipsrc + jmasrc + linuxsrc + basesrc
	env.Append (CCFLAGS = '-D__UNIXSDL__')
	env.Append (ASFLAGS = '-DELF -D__UNIXSDL__')
	env.Append (CCFLAGS = '-I.')
	env.Append (CCFLAGS = '-I/usr/include/SDL')

	#Build ZSNES
	env.Program('zsnes',src)

if platform == 'win32':
	if env['CC'] == 'cl':
		def freeMSVCHack(env, vclibs):
			# SCons automatically finds full versions of msvc via the registry, so
			# if it can't find 'cl', it may be because we're trying to use the
			# free version
			def isMicrosoftSDKDir(dir):
				return os.path.exists(dir+os.sep+'Include'+os.sep+'Windows.h') and os.path.exists(dir+os.sep+'Lib'+os.sep+'WinMM.lib')
 	
			def findMicrosoftSDK():
				import SCons.Platform.win32
				import SCons.Util
				import re
				if not SCons.Util.can_read_reg:
					return None
				HLM = SCons.Util.HKEY_LOCAL_MACHINE
				K = r'Software\Microsoft\.NETFramework\AssemblyFolders\PSDK Assemblies'
            		try:
                		k = SCons.Util.RegOpenKeyEx(HLM, K)
				p = SCons.Util.RegQueryValueEx(k,'')[0]
				# this should have \include at the end, so chop that off
				p = re.sub(r'(?i)\\+Include\\*$','',p)
				if isMicrosoftSDKDir(p): return p
			except SCons.Util.RegError:
		                pass
 
			K = r'SOFTWARE\Microsoft\MicrosoftSDK\InstalledSDKs'
			try:
				k = SCons.Util.RegOpenKeyEx(HLM, K)
				i=0
				while 1:
					p = SCons.Util.RegEnumKey(k,i)
					i+=1
					subk = SCons.Util.RegOpenKeyEx(k, p)
					try:
						p = SCons.Util.RegQueryValueEx(subk,'Install Dir')[0]
						# trim trailing backslashes
						p = re.sub(r'\\*$','',p)
						if isMicrosoftSDKDir(p): return p
					except SCons.Util.RegError:
						pass
			except SCons.Util.RegError:
				pass

			return None
 
		# End of local defs. Actual freeMSVCHack begins here
		if not env['MSVS'].get('VCINSTALLDIR'):
			if os.environ.get('VCToolkitInstallDir'):
				vcdir=os.environ['VCToolkitInstallDir']
				env.PrependENVPath('INCLUDE', vcdir+os.sep+'Include')
				env.PrependENVPath('LIB', vcdir+os.sep+'Lib')
				env.PrependENVPath('PATH', vcdir+os.sep+'Bin')
				env['MSVS']['VERSION'] = '7.1'
				env['MSVS']['VERSIONS'] = ['7.1']
			if not env['MSVS'].get('PLATFORMSDKDIR'):
				sdkdir = findMicrosoftSDK()
				if sdkdir:
					env.PrependENVPath('INCLUDE', sdkdir+os.sep+'Include')
					env.PrependENVPath('LIB', sdkdir+os.sep+'Lib')
					env.PrependENVPath('PATH', sdkdir+os.sep+'Bin')
					env['MSVS']['PLATFORMSDKDIR']=sdkdir
					# FREE MSVC7 only allows
					# /ML(libc) /MT(libcmt) or /MLd(libcd)
					# Full IDE versions also have
					# /MD(msvcrtd) /MTd(libcmtd) and /MDd(msvcrtd)
					# So if you want to debug with the freever, the only option is
					# the single-threaded lib, /MLd
					vclibs['Debug']='/MLd'
					vclibs['Release']='/MT'
 
		# MSVC SETUP
		# MDd is for multithreaded debug dll CRT (msvcrtd)
		# MD is for multithreaded dll CRT (msvcrt)
		# These are just my preferences
		vclibs = {'Debug':'/MDd','Release':'/MD'}
		freeMSVCHack(env, vclibs)
		
		env.Append(CCFLAGS=[vclibs[variant]])
		if debug:
			env.Append(CCFLAGS=Split('/Zi /Fd${TARGET}.pdb'))
			env.Append(LINKFLAGS = ['/DEBUG'])
			# env.Clean('.', '${TARGET}.pdb')
			# Need to clean .pdbs somehow! The above line doesn't work!
		else:
			env.Append(CCFLAGS=Split('/Og /Ot /Ob1 /Op /G6'))

			env.Append(CCFLAGS=Split('/EHsc /J /W3 /Gd'))
			env.Append(CPPDEFINES=Split('WIN32 _WINDOWS'))
		src = chipssrc + cpusrc + dossrc + effectssrc + guisrc + videosrc + zipsrc + jmasrc + winsrc
		env.Append (CCFLAGS = '-D__WIN32__')
		env.Append (ASFLAGS = '-f win32')
		
		#Build ZSNESW
		env.Program('zsnesw.exe',src)
	else:
		if debug:
			env.Append (CCFLAGS = '-g -D__WIN32__')
			env.Append (CCFLAGS = '-D__WIN32__')
			env.Append (ASFLAGS = '-f win32 -O0')
		else:
			env.Append (CCFLAGS = '-D__WIN32__')
			env.Append (CCFLAGS = '-f win32')
		
		#Build ZSNESW
		env.Program('zsnesw.exe',src)