;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

%include "macros.mac"

EXTSYM getcfg,soundon,SBHDMA,StereoSound,init,GUIRestoreVars,GUIClick,MouseDis
EXTSYM ConvertJoyMap,ConvertJoyMap1,ConvertJoyMap2,printhex,InitSPC
EXTSYM StartUp,PrintStr,WaitForKey,PrintChar,ZFileSystemInit
EXTSYM SPCDisable,SystemInit,allocmem
EXTSYM FPSOn,FPSAtStart,cfgsoundon
EXTSYM xa
EXTSYM ram7fa,wramdataa
EXTSYM malloc,free
EXTSYM StateBackup
;EXTSYM OSPort
EXTSYM ADSRGAINSwitch,MMXSupport,ScreenScale,SoundQuality
EXTSYM debugger,pl1contrl,pl2contrl,romtype,smallscreence
EXTSYM smallscreenon,spcon
EXTSYM statefileloc,LatestSave,firstsaveinc
EXTSYM Create_File,Delete_File,Open_File,Get_File_Date,Close_File,Change_Dir,Get_Dir
EXTSYM romloadskip
EXTSYM cfgloadgdir,cfgloadsdir
EXTSYM init18_2hz
EXTSYM OSExit,GUIOn2
%ifdef __LINUX__
EXTSYM LinuxExit
EXTSYM GetFilename
%endif

%ifdef OPENSPC
EXTSYM OSPC_Init
%endif

NEWSYM UIAsmStart






; Function 0501h
; User Interface

; Search for CMDLINE= for commandline entry

SECTION .text

NEWSYM zstart
	call StartUp

	mov edx,mydebug
	call PrintStr

	mov edx,welcome         ;welcome message
	call PrintStr

	call SystemInit

	cld                     ;clear direction flag

%ifdef OPENSPC
	call OSPC_Init
%else
	call setnoise
	call InitSPC
%endif
	call allocmem           ;allocate memory

	cmp byte[soundon],0
	jne .yessound
	cmp byte[SPCDisable],1
	je .yessound
	mov byte[soundon],1
	mov byte[spcon],1
	mov byte[DSPDisable],1
.yessound
	cmp byte[SPCDisable],1
	jne .nodissound
	mov byte[soundon],0
	mov byte[spcon],0
.nodissound
        cmp byte[frameskip],0
        jne .nofpsatstart
        mov al,[FPSAtStart]
        mov [FPSOn],al
        xor al,al
.nofpsatstart

	mov al,[gammalevel]
	shr al,1
	mov [gammalevel16b],al
	call MMXCheck

	mov edx,.failedalignc
	mov eax,outofmemory
	test eax,3h
	jnz .failalign
	mov edx,.failedalignd
	mov eax,xa
	test eax,3h
	jnz .failalign
	jmp init
.failalign
	push eax
	call PrintStr
	pop eax
	and eax,1Fh
	call printnum
	call WaitForKey
	jmp init

section .data
.failedalignd db 'Data Alignment Failure : ',0
.failedalignc db 'Code Alignment Failure : ',0
section .text

ALIGN32
NEWSYM	outofmemory
	mov edx,outofmem
	call PrintStr
	jmp DosExit

SECTION .data
NEWSYM mydebug, db '',13,10,0
NEWSYM outofmem, db 'You don',39,'t have enough memory to run this program!',13,10,0

NEWSYM welcome


                 db 'ZSNES v',ZVERSION,' (c) 1997-2002, ZSNES Team (zsKnight & _Demo_)',13,10,13,10
                 db 'Be sure to check http://www.zsnes.com/ for the latest version.',13,10
                 db 'Please report crashes to zsnes-devel@lists.sourceforge.net.',13,10,13,10
                 db 'ZSNES is written by the ZSNES Team (See AUTHORS.TXT)',13,10
                 db 'ZSNES comes with ABSOLUTELY NO WARRANTY.  This is free software,',10,13
                 db 'and you are welcome to redistribute it under certain conditions;',10,13
%ifdef __LINUX__
                 db 'please read ',39,'LICENSE',39,' thoroughly before using it.',10,13,10,13
%else
                 db 'please read ',39,'LICENSE.TXT',39,' thoroughly before using it.',10,13,10,13
%endif
                 db 'Use ZSNES -? for command line definitions.',13,10,13,10,0

;cpuidfname db 'nocpuzid.dat',0
;cpuidtext db 'NOTE: If ZSNES crashes here, then please re-run. ',0
;cpuidtext2 db 13,'                                                 ',13,0
YesMMX    db 'MMX support found and enabled.',13,10,13,10,0

; global variables

NEWSYM per2exec,      dd 100            ; percentage of opcodes to execute
NEWSYM SoundCompD,    db 1              ; Disable Sound Compression Buffering (1=y)

%ifdef __MSDOS__
NEWSYM cvidmode,      db 4              ; video mode, 0=320x240, 1=256x256
%else
NEWSYM cvidmode,      db 1              ; video mode, 0=320x240, 1=256x256
%endif

section .bss
NEWSYM string,  resb 512
NEWSYM fname,   resb 512
NEWSYM fnames,  resb 512  ; sram filename
NEWSYM fnamest, resb 512  ; state filename

NEWSYM prevvid,       resd 1
NEWSYM selc0040,      resd 1
NEWSYM selcA000,      resd 1
NEWSYM selcB800,      resd 1
NEWSYM filefound,     resd 1            ; Parameter String Found
NEWSYM prevdash,      resd 1            ; Previous Dash Value
NEWSYM frameskip,     resd 1            ; 0 = Auto, 1-10 = Skip 0 .. 9
NEWSYM vidbuffer,     resd 1            ; video buffer (1024x239 = 244736)
NEWSYM ngwinptr,      resd 1
NEWSYM vidbufferm,    resd 1            ; video buffer mirror
NEWSYM vidbufferofsa, resd 1            ; offset 1
NEWSYM vidbufferofsb, resd 1            ; offset 2
NEWSYM vidbufferofsc, resd 1            ; offset 2
NEWSYM vidbufferofsmos, resd 1          ; mosaic offset for new graphics engine
NEWSYM headdata,      resd 1
NEWSYM romdata,       resd 1            ; rom data  (4MB = 4194304)
NEWSYM sfxramdata,    resd 1            ; SuperFX Ram Data
NEWSYM wramdata,      resd 1            ; stack (64K = 32768)
NEWSYM ram7f,         resd 1            ; ram @ 7f = 65536
NEWSYM vram,          resd 1            ; vram = 65536
NEWSYM sram,          resd 1            ; sram = 32768
NEWSYM spritetablea,  resd 1            ; 
NEWSYM spcBuffera,    resd 1            ; 
NEWSYM debugbuf,      resd 1            ; debug buffer = 38x1000 = 38000
NEWSYM regptr,        resd 1            ; pointer to registers
NEWSYM regptw,        resd 1            ; pointer to registers
NEWSYM vcache2b,      resd 1            ; 2-bit video cache
NEWSYM vcache4b,      resd 1            ; 4-bit video cache
NEWSYM vcache8b,      resd 1            ; 8-bit video cache
NEWSYM vcache2bs,     resd 1            ; 2-bit video secondary cache
NEWSYM vcache4bs,     resd 1            ; 4-bit video secondary cache
NEWSYM vcache8bs,     resd 1            ; 8-bit video secondary cache
NEWSYM romispal,      resb 1            ; 0 = NTSC, 1 = PAL
NEWSYM enterpress,    resb 1            ; if enter is to be issued (0 = yes)
NEWSYM newgfx16b,     resb 1
NEWSYM BitConv32Ptr,     resd 1
NEWSYM RGBtoYUVPtr,   resd 1

NEWSYM previdmode,    resb 1            ; previous video mode
NEWSYM cbitmode,      resb 1            ; bit mode, 0=8bit, 1=16bit

SECTION .data

NEWSYM opexec268,     db 171            ; # of opcodes/scanline in 2.68Mhz mode
NEWSYM opexec358,     db 180            ; # of opcodes/scanline in 3.58Mhz mode (228/180)
NEWSYM opexec268cph,  db 42             ; # of opcodes/hblank in 2.68Mhz mode
NEWSYM opexec358cph,  db 45             ; # of opcodes/hblank in 3.58Mhz mode (56/50)
NEWSYM opexec268b,    db 171            ; # of opcodes/scanline in 2.68Mhz mode
NEWSYM opexec358b,    db 180            ; # of opcodes/scanline in 3.58Mhz mode (228/180)
NEWSYM opexec268cphb, db 42             ; # of opcodes/hblank in 2.68Mhz mode
NEWSYM opexec358cphb, db 45             ; # of opcodes/hblank in 3.58Mhz mode (56/50)
NEWSYM debugdisble,   db 1              ; debugger disable.  0 = no, 1 = yes
NEWSYM gammalevel,    db 0              ; gamma level
NEWSYM gammalevel16b, db 0              ; gamma level
NEWSYM scanlines,     db 0              ; scanlines on/off
NEWSYM vsyncon,       db 0              ; vsync on/off
NEWSYM guioff,        db 0              ; gui off (1)
NEWSYM AddSub256,     db 0              ; screen add/sub in 256 colors
NEWSYM Sup48mbit,     db 1              ; Support 48mbit roms
NEWSYM Sup16mbit,     db 0              ; Support 16mbit roms
NEWSYM dmadeddis,     db 0              ; DMA deduction
NEWSYM antienab,      db 0              ; Interpolation Enabled
NEWSYM snesmouse,     db 0              ; Mouse enabled (=1)
NEWSYM OldStyle,      db 1              ; Old style joystick on
NEWSYM SecondPort,    db 0              ; Secondary Joystick Port Enabled (209h)

; New Variables
NEWSYM ForcePal,      db 0              ; 1 = NTSC, 2 = PAL
NEWSYM Force8b,       db 0              ; Force 8-bit sound on
NEWSYM Doublevbuf,    db 1              ; Double video buffer
NEWSYM V8Mode,        db 0              ; Vegetable mode! =)
NEWSYM fastmemptr,    db 0
NEWSYM showallext,    db 0
NEWSYM finterleave,   db 0
NEWSYM DSPDisable,    db 0
NEWSYM Palette0,      db 0
NEWSYM DisplayS,      db 0
NEWSYM SPC700sh,      db 0
NEWSYM OffBy1Line,    db 0
NEWSYM DosPort,       db 3
NEWSYM spc7110romptr, dd 0

NEWSYM MusicRelVol,   db 75
NEWSYM MusicVol,      db 0
;NEWSYM BetaUser, db 37,38,210,56,78,23,7,7,0

SECTION .text

;*******************************************************
; Set Noise Data
;*******************************************************

NEWSYM setnoise
    mov edi,NoiseData
    mov ebx,256
    mov ecx,128
    xor esi,esi
    xor edx,edx
.next
    mov al,[.samplenoise+esi]
    add al,[.samplenoise+edx]
    mov [edi],al
    inc edi
    inc esi
    and esi,07Fh
    dec edx
    and edx,07Fh
    dec ecx
    jnz .next
    dec edx
    and edx,07Fh
    mov ecx,128
    dec ebx
    jnz .next
    ret

SECTION .data
.samplenoise
  db 27,232,234,138,187,246,176,81,25,241,1,127,154,190,195,103,231,165,220,238
  db 232,189,57,201,123,75,63,143,145,159,13,236,191,142,56,164,222,80,88,13
  db 148,118,162,212,157,146,176,0,241,88,244,238,51,235,149,50,77,212,186,241
  db 88,32,23,206,1,24,48,244,248,210,253,77,19,100,83,222,108,68,11,58
  db 152,161,223,245,4,105,3,82,15,130,171,242,141,2,172,218,152,97,223,157
  db 93,75,83,238,104,238,131,70,22,252,180,82,110,123,106,133,183,209,48,230
  db 157,205,27,21,107,63,85,164
SECTION .text

;*******************************************************
; Allocate Memory, ebx = size,eax returned = LFB pointer
;*******************************************************

NEWSYM AllocMem
%ifndef __LINUX__
    mov ax,0501h
    mov cx,bx
    shr ebx,16
    int 31h
    jc near outofmemory
    mov ax,bx
    shl eax,16
    mov ax,cx
%endif
    ret

;*******************************************************
; Get Command Line       Locates SET CMDLINE environment
;*******************************************************

%ifdef __WIN32__
EXTSYM _imp__GetModuleFileNameA@12
EXTSYM memcpy
%elifdef __MSDOS__
EXTSYM argv
%endif

NEWSYM getcmdline
%ifdef __MSDOS__
%if 0
    mov edx,.string2s
    push edx
    call getenv
    pop edx
%else
    mov eax,[argv]
%endif
    cmp eax,0
    je near .nfound
    mov esi,eax
    mov edi,CMDLineStr
    cmp byte[esi],0
    je near .nfound
.a
    mov dl,[esi]
    cmp dl,'a'
    jb .nocap
    cmp dl,'z'
    ja .nocap
    sub dl,'a'-'A'
.nocap
    mov [edi],dl
    mov [edi+256],dl
    mov [edi+512],dl
    inc esi
    inc edi
    cmp dl,32
    je .b
    cmp dl,0
    jne .a
.b
%elifdef __WIN32__
    push dword 256
    push dword CMDLineStr
    push byte +0
    call [_imp__GetModuleFileNameA@12]
    push eax
    push eax
    push dword CMDLineStr
    push dword GUIFName
    call memcpy
    add esp,+12
    mov eax,[esp]
    push eax
    push dword CMDLineStr
    push dword GUICName
    call memcpy
    add esp,+12
    pop edi
    add edi,CMDLineStr
%else
    mov esi,CMDLineStr
    ret ; *sigh*
%endif
    mov esi,CMDLineStr
    mov eax,esi
.next2
    cmp eax,edi
    je .nomore
%ifdef __LINUX__
    cmp byte[eax],'/'
%else
    cmp byte[eax],'\'
    je .found
%endif
    cmp byte[eax],':'
    jne .next
.found
    mov esi,eax
    inc esi
.next
    inc eax
    jmp .next2
.nomore
    mov [FilenameStart],esi
    mov byte[esi],'z'
    mov byte[esi+1],'s'
    mov byte[esi+2],'n'
    mov byte[esi+3],'e'
    mov byte[esi+4],'s'
    mov byte[esi+5],'.'
    mov byte[esi+6],'c'
    mov byte[esi+7],'f'
    mov byte[esi+8],'g'
    mov byte[esi+9],0
    mov byte[esi+256],'z'
    mov byte[esi+1+256],'g'
    mov byte[esi+2+256],'u'
    mov byte[esi+3+256],'i'
    mov byte[esi+4+256],'c'
    mov byte[esi+5+256],'f'
    mov byte[esi+6+256],'g'
    mov byte[esi+7+256],'.'
    mov byte[esi+8+256],'d'
    mov byte[esi+9+256],'a'
    mov byte[esi+10+256],'t'
    mov byte[esi+11+256],0
    mov byte[esi+512],'d'
    mov byte[esi+1+512],'a'
    mov byte[esi+2+512],'t'
    mov byte[esi+3+512],'a'
    mov byte[esi+4+512],'.'
    mov byte[esi+5+512],'c'
    mov byte[esi+6+512],'m'
    mov byte[esi+7+512],'b'
    mov byte[esi+8+512],0
    ret
.nfound
    mov edx,.stringnf
    call PrintStr
    mov esi,CMDLineStr
    mov [FilenameStart],esi
    mov byte[esi],'z'
    mov byte[esi+1],'s'
    mov byte[esi+2],'n'
    mov byte[esi+3],'e'
    mov byte[esi+4],'s'
    mov byte[esi+5],'.'
    mov byte[esi+6],'c'
    mov byte[esi+7],'f'
    mov byte[esi+8],'g'
    mov byte[esi+9],0
    mov byte[esi+256],'z'
    mov byte[esi+1+256],'g'
    mov byte[esi+2+256],'u'
    mov byte[esi+3+256],'i'
    mov byte[esi+4+256],'c'
    mov byte[esi+5+256],'f'
    mov byte[esi+6+256],'g'
    mov byte[esi+7+256],'.'
    mov byte[esi+8+256],'d'
    mov byte[esi+9+256],'a'
    mov byte[esi+10+256],'t'
    mov byte[esi+11+256],0
    mov byte[esi+512],'d'
    mov byte[esi+1+512],'a'
    mov byte[esi+2+512],'t'
    mov byte[esi+3+512],'a'
    mov byte[esi+4+512],'.'
    mov byte[esi+5+512],'c'
    mov byte[esi+6+512],'m'
    mov byte[esi+7+512],'b'
    mov byte[esi+8+512],0
    ret

SECTION .data
.string2s db 'CMDLINE',0
.stringnf db 'SET CMDLINE LINE NOT FOUND!',13,0

SECTION .bss
NEWSYM CMDLineStr, resb 256
NEWSYM GUIFName, resb 256
NEWSYM GUICName, resb 256
NEWSYM FilenameStart, resd 1

;SECTION .text
;*******************************************************
; Variable section
;*******************************************************

SECTION .bss

;ALIGN32

NEWSYM vrama,       resb 65536
NEWSYM mode7tab,    resb 65536 
NEWSYM srama,       resb 65536*2
NEWSYM debugbufa,   resb 10000
NEWSYM wramreadptr, resd 1
NEWSYM regptra,     resb 49152
NEWSYM wramwriteptr, resd 1
NEWSYM regptwa,     resb 49152

; vcache.asm

; table.asm

; vesa2.asm

NEWSYM fulladdtab, resw 65536

; init.asm
NEWSYM echobuf,       resb 90000

; dspproc.asm

NEWSYM spcRamcmp, resb 65536
NEWSYM VolumeConvTable, resw 32768
NEWSYM dspWptr,  resd 256
NEWSYM dspRptr,  resd 256
NEWSYM NoiseData, resb 32768

; makevid.asm

; makevid.asm
NEWSYM vcache2ba,   resb 262144+256 
NEWSYM vcache4ba,   resb 131072+256 
NEWSYM vcache8ba,   resb 65536+256 

ZSNESBase         resd 1
BlockSize         resd 1  ; Set before calling
LinearAddress     resd 1  ; Returned by function
BlockHandle       resd 1  ; Returned by function
ZSNESAddress      resd 1  ; Returned by function

SECTION .text

;*******************************************************
; Allocate Pointer    Sets variables with pointer values
;*******************************************************


AllocateLDTDescriptor:
%ifndef __LINUX__
;Get ZSNES Base
   mov ax,ds
   mov bx,ax
   mov eax,0006h
   int 31h
   jc .FatalError
   mov [ZSNESBase+2],cx
   mov [ZSNESBase],dx
   ret
.FatalError
; maybe dosexit?
%endif
   ret


AllocateBlock:
%ifndef __LINUX__
   mov eax,0501h
   mov bx,[BlockSize+2]
   mov cx,[BlockSize]
   int 31h
   jc .FatalError
   mov [LinearAddress+2],bx
   mov [LinearAddress],cx
   mov [BlockHandle+2],si
   mov [BlockHandle],di
   mov eax,[LinearAddress]
   sub eax,[ZSNESBase]
   and eax,0FFFFFFE0h
   add eax,40h
   mov [ZSNESAddress],eax
   xor ebx,ebx
   ret
.FatalError
   mov ebx,1
%endif
   ret


SECTION .bss

;ALIGN32
vbufaptr resd 1
vbufeptr resd 1
ngwinptrb resd 1
romaptr  resd 1
vbufcptr resd 1
NEWSYM vbufdptr, resd 1
vc2bptr  resd 1
vc4bptr  resd 1
vc8bptr  resd 1
cmemallocptr resd 1
memfreearray resd 12

SECTION .text




%macro AllocmemFail 3
    mov ebx,%1
    add ebx,1000h
    push ebx
    call malloc
    pop ebx
    cmp eax,0
    je near %3
    mov ebx,dword[cmemallocptr]
    add dword[cmemallocptr],4
    mov [ebx],eax
    and eax,0FFFFFFE0h
    add eax,40h
    mov [%2],eax
%endmacro

%macro AllocmemOkay 3
    mov ebx,%1
    add ebx,1000h
    push ebx
    call malloc
    pop ebx
    push eax
    and eax,0FFFFFFE0h
    add eax,40h
    mov [%2],eax
    pop eax
    cmp eax,0
    je %%nomalloc
    mov ebx,dword[cmemallocptr]
    add dword[cmemallocptr],4
    mov [ebx],eax
%%nomalloc
    cmp eax,0
    jne near %3
%endmacro

NEWSYM allocspc7110
    AllocmemFail 8192*1024+4096,spc7110romptr,outofmemoryb
    ret

outofmemoryb
;    cmp byte[OSPort],1
;    ja .notdos
%ifdef __MSDOS__
    mov ax,3
    int 10h
%endif
;.notdos
    jmp outofmemory

NEWSYM allocptr
    mov dword[cmemallocptr],memfreearray


;    cmp byte[OSPort],3
;    jne near .nostate
%ifndef __MSDOS__
    AllocmemFail 4096*128*16+4096+65536*16,StateBackup,outofmemory
    mov eax,[StateBackup]
    add eax,4096*128*16
    mov [BitConv32Ptr],eax
%endif
;.nostate

    ; Memory Allocation
    AllocmemFail 65536*4+4096,spcBuffera,outofmemory
    AllocmemFail 256*512+4096,spritetablea,outofmemory
    AllocmemFail 512*296*4+4096+512*296,vbufaptr,outofmemory
    AllocmemFail 288*2*256+4096,vbufeptr,outofmemory
    AllocmemFail 256*224+4096,ngwinptrb,outofmemory
    AllocmemFail 1024*296,vbufdptr,outofmemory
    AllocmemFail 65536*4*4+4096,vcache2bs,outofmemory
    AllocmemFail 65536*4*2+4096,vcache4bs,outofmemory
    AllocmemFail 65536*4+4096,vcache8bs,outofmemory
    AllocmemFail 65536*4+4096,RGBtoYUVPtr,outofmemory
    mov byte[newgfx16b],1
    AllocmemOkay 4096*1024+32768*2+2048*1024+4096,romaptr,.memoryokay
    mov byte[Sup48mbit],0
    AllocmemOkay 4096*1024+32768*2+4096,romaptr,.donememalloc
    mov byte[Sup16mbit],1
    AllocmemOkay 2048*1024+32768*2+4096,romaptr,.donememalloc
    jmp outofmemory
.memoryokay
.donememalloc

    ; Set up memory values
    mov eax,[vbufaptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vidbuffer],eax
    mov dword[vidbufferofsa],eax
    add eax,75036
    mov dword[vidbufferofsmos],eax

    mov eax,[ngwinptrb]
    and eax,0FFFFFFF8h
    add eax,2048
    mov [ngwinptr],eax

    mov eax,[vbufeptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vidbufferofsb],eax

    mov eax,[vbufdptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vidbufferofsc],eax

    mov eax,[romaptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[headdata],eax
    mov dword[romdata],eax
    add eax,4194304
    mov dword[sfxramdata],eax

    mov esi,[romdata]
    cmp byte[Sup48mbit],0
    je .no48mbit
    add esi,4096*1024+2048*1024
    jmp .done
.no48mbit
    cmp byte[Sup16mbit],0
    je .no16mbit
    add esi,2048*1024
    jmp .done
.no16mbit
    add esi,4096*1024
.done
    mov byte[esi],58h
    mov byte[esi+1],80h
    mov byte[esi+2],0FEh

    mov dword[wramdata],wramdataa
    mov dword[ram7f],ram7fa
    mov dword[vram],vrama
    mov dword[sram],srama
    mov dword[debugbuf],debugbufa
    mov dword[regptr],regptra
    sub dword[regptr],8000h   ; Since register address starts @ 2000h
    mov dword[regptw],regptwa
    sub dword[regptw],8000h   ; Since register address starts @ 2000h

    ; 2-bit = 256k
    mov eax,vcache2ba
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vcache2b],eax
    ; 4-bit = 128k
    mov eax,vcache4ba
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vcache4b],eax
    ; 8 bit = 64k
    mov eax,vcache8ba
    and eax,0FFFFFFF8h
    add eax,8
    mov dword[vcache8b],eax
    ret

;*******************************************************
; Print Numbers                Prints # in EAX to screen
;*******************************************************
NEWSYM printnum
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
  .loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
  .loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    call PrintChar
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    ret

NEWSYM convertnum
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
  .loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
  .loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    mov [esi],dl
    inc esi
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    mov byte[esi],0
    ret

; eax = value, ecx = # of bytes
NEWSYM converthex
    mov ebx,ecx
    mov ecx,4
    sub ecx,ebx
    shl ecx,3
    shl eax,cl
    mov ecx,ebx
    xor ebx,ebx
    add ecx,ecx
.loopb
    mov ebx,eax
    and ebx,0F0000000h
    shr ebx,28
    mov dl,[.hexdat+ebx]
    mov [esi],dl
    inc esi
    shl eax,4
    dec ecx
    jnz .loopb
    mov byte[esi],0
    ret

section .data
.hexdat db '0123456789ABCDEF'

;*******************************************************
; Check Parameter          This Processes the Parameters
;*******************************************************

SECTION .bss
newestfileloc resb 1
newestfiledate resd 1
SECTION .text

NEWSYM makeextension
    xor ecx,ecx
    xor ebx,ebx
    xor ah,ah
    mov cl,[fname]
    mov [fnames],cl
    mov [fnamest],cl
    mov dl,cl
    inc ebx
.loopc
    mov al,[fname+ebx]
    mov [fnames+ebx],al
    mov [fnamest+ebx],al
    inc ebx
    inc ah
    dec ecx
    jnz .loopc
    ; find for '.' or '\'
    mov cl,dl
    mov edx,ebx
.loopz
    dec edx
    mov al,[fnames+edx]
%ifdef __LINUX__
    cmp al, '/'
%else
    cmp al,'\'
%endif
    je .addext
    cmp al,'.'
    je .addb
    dec cl
    jnz .loopz
    jmp .addext
.addb
    mov ebx,edx
.addext
    mov byte[fnames+ebx],'.'
    mov byte[fnamest+ebx],'.'
    inc ebx
    mov byte[fnames+ebx],'s'
    mov byte[fnamest+ebx],'z'
    inc ebx
    mov byte[fnames+ebx],'r'
    mov byte[fnamest+ebx],'s'
    inc ebx
    mov byte[fnames+ebx],'m'
    mov byte[fnamest+ebx],'t'
    mov dword[statefileloc],ebx
    inc ebx
    mov byte[fnames+ebx],0
    mov byte[fnamest+ebx],0
    add ah,4
    mov [fnames],ah
    mov [fnamest],ah
%ifdef __LINUX__
    pushad
    call GetFilename
    popad
%endif
    mov byte[firstsaveinc],1
    cmp byte[LatestSave],1
    je .latestsave
    ret
.latestsave
    ; change dir to Save Dir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir

    call DetermineNewest

    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
    ret

%macro determinenewhelp 2
    mov bl,%1
    mov byte[fnamest+eax],%2
    call DetermineNew
%endmacro

DetermineNew:
    push eax
    push ebx
    mov edx,fnamest+1
    call Open_File
    jc near .nodraw
    mov bx,ax
    mov edx,fnamest+1
    call Get_File_Date
%ifdef __MSDOS__
    shl edx,16
    mov dx,cx
%endif
    push edx
    call Close_File
    pop edx
    pop ebx
    pop eax
    ; date = edx, position = bl
    cmp edx,[newestfiledate]
    jbe .notlatest
    mov [newestfiledate],edx
    mov [newestfileloc],bl
.notlatest
    ret
.nodraw
    pop ebx
    pop eax
    ret

DetermineNewest:
    mov eax,[statefileloc]
    mov dword[newestfiledate],0
    mov byte[newestfileloc],0
    determinenewhelp 0,'t'
    determinenewhelp 1,'1'
    determinenewhelp 2,'2'
    determinenewhelp 3,'3'
    determinenewhelp 4,'4'
    determinenewhelp 5,'5'
    determinenewhelp 6,'6'
    determinenewhelp 7,'7'
    determinenewhelp 8,'8'
    determinenewhelp 9,'9'
    mov bl,[newestfileloc]
    add bl,'0'
    cmp bl,'0'
    jne .nott
    mov bl,'t'
.nott
    mov [fnamest+eax],bl
    ret

;*******************************************************
; Get Parameters             Get Parameters Individually
;*******************************************************

NEWSYM tparms
  .donestring
    test byte[.numparam],0FFh
    jz .nochars
    mov al,byte[filefound]
    test al,0FFh
    jz .nostring
    ret

  .nostring
    cmp byte[guioff],0
    je .yesgui

    mov edx,.nostr
    call PrintStr
    jmp DosExit

  .nochars
    cmp byte[guioff],0
    je .yesgui
    cmp byte[fname],0
    jne .yesgui
    jmp displayparams

.yesgui
    mov byte[romloadskip],1
    ret

SECTION .bss
.numparam resb 1

SECTION .data
.nostr db 'This emulator will not work without a filename!',13,10,0
.waitkey   db 'Press Any Key to Continue.',0
.ret       db 13,10,0

SECTION .text
NEWSYM displayparams
    mov edx,.noparams        ;use extended
    call PrintStr
%ifndef __LINUX__
    call WaitForKey
%endif
    mov edx,.noparms2        ;use extended
    call PrintStr
%ifndef __LINUX__
    call WaitForKey
%endif
    mov edx,.noparms3        ;use extended
    call PrintStr
    jmp DosExit

; yes it sucks, i had to change the one character options to have at least two characters
; that way i could distinguish from the other two character options that share the same
; first letter...only way getopt works correctly :|
; EvilTypeGuy
SECTION .data
%ifdef __LINUX__
.noparams db 'Usage : zsnes [-d,-f #, ... ] <filename.smc>',13,10
          db '   Eg : zsnes -s -r 2 game.smc',13,10,13,10
%else
.noparams db 'Usage : ZSNES [-d,-f #, ... ] <filename.SMC>',13,10
          db '   Eg : ZSNES -s -r 2 game.smc',13,10,13,10
%endif
          db '  -0      Disable Color 0 modification in 8-bit modes',13,10
          db '  -1 #/-2 #   Select Player 1/2 Input :',13,10
          db '                0 = None       1 = Keyboard   2 = Joystick   3 = Gamepad',13,10
          db '                4 = 4Button    5 = 6Button    6 = Sidewinder   ',13,10
          db '  -7      Disable SPC700 speedhack',13,10
          db '  -8      Force 8-bit sound',13,10
          db '  -9      Off by 1 line fix',13,10
          db '  -a      Enable automatic frame rate',13,10
%ifdef __LINUX__
          db '  -cs     Enable full/wide screen (when available)',13,10
%else
          db '  -c      Enable full screen (when available)',13,10
%endif
          db '  -cb     Remove Background Color in 256 color video modes',13,10
          db '  -cc     Enable small screen (when available)',13,10

; debugger not available in linux version
; because of bios interrupt code
%ifndef __LINUX__
          db '  -d      Start with debugger enabled',13,10
%endif
          db '  -dd     Disable sound DSP emulation',13,10
%ifndef __LINUX__
          ;db '  -e      Skip enter key press at the beginning',13,10
%endif
          db '  -f #    Enable fixed frame rate [0...9]',13,10
%ifndef __LINUX__
          db '  -g #    Specify gamma correction value [0...15]',13,10
          db '          (Only works properly in 8-bit modes)',13,10
%endif
          db '  -h      Force HiROM',13,10
          db '  -i      Uninterleave ROM Image',13,10
          db '  -j      Disable Mouse (Automatically turns off right mouse click)',13,10
          db '  -k #    Set Volume Level (0 .. 100)',13,10
          db 'Press any key to continue.',0
.noparms2 db 13,'  -l      Force LoROM        ',13,10
          db '  -m      Disable GUI',13,10
%ifdef __LINUX__
          db '  -n      Enable full scanlines (when available)',13,10
%else
          db '  -n #    Enable scanlines (when available)',13,10
          db '          Where # is: 1 = full, 2 = 25%, 3 = 50%',13,10
%endif
%ifdef __LINUX__
          db '  -of     Enable FPU copy   ',13,10
%else
;          db '  -o      Enable FPU copy   ',13,10
%endif
          db '  -om     Enable MMX support (when available)',13,10
          db '  -p #    Percentage of instructions to execute [50..120]',13,10
          db '  -r #    Set Sampling Sound Blaster Sampling Rate & Bit :',13,10
          db '             0 = 8000Hz  1 = 11025Hz 2 = 22050Hz 3 = 44100Hz',13,10
          db '             4 = 16000Hz 5 = 32000Hz',13,10
%ifdef __LINUX__
          db '  -se     Enable SPC700/DSP emulation (Sound)',13,10
%else
          db '  -s      Enable SPC700/DSP emulation (Sound)',13,10
%endif
          db '  -sa     Show all extensions in GUI (*.*)',13,10
          db '  -sn     Enable Snowy GUI Background',13,10
          db '  -t      Force NTSC timing',13,10
          db '  -u      Force PAL timing',13,10
%ifndef __LINUX__
          db '  -v #    Select Video Mode :',13,10
          db '           0 = 256x224x8B  (MODEQ)  1 = 256x240x8B (MODEQ)',13,10
          db '           2 = 256x256x8B  (MODEQ)  3 = 320x224x8B (MODEX)',13,10
          db '           4 = 320x240x8B  (MODEX)  5 = 320x256x8B (MODEX)',13,10
          db '           6 = 640x480x16B (VESA1)  7 = 320x240x8B (VESA2)',13,10
          db '           8 = 320x240x16B (VESA2)  9 = 320x480x8B (VESA2)',13,10
          db '          10 = 320x480x16B (VESA2) 11 = 512x384x8B (VESA2)',13,10
          db '          12 = 512x384x16B (VESA2) 13 = 640x400x8B (VESA2)',13,10
          db '          14 = 640x400x16B (VESA2) 15 = 640x480x8B (VESA2)',13,10
          db '          16 = 640x480x16B (VESA2) 17 = 800x600x8B (VESA2)',13,10
          db '          18 = 800x600x16B (VESA2)',13,10
%endif
          db '  -w      Enable vsync (disables triple buffering)',13,10
          db 'Press any key to continue.',0
.noparms3 db 13,'  -y      Enable EAGLE (640x480x8B only) or Interpolation (640x480x16B only)',13,10
%ifdef __MSDOS__
          db '  -3      Enable triple buffering (disables vsync)',13,10
%endif
          db '  -z      Enable Stereo Sound',13,10
; FIX STATMAT
          db '  -zs #   Auto load specified save state slot on startup',13,10
; FIX STATMAT          
          db '',13,10
          db '  File Formats Supported by GUI : .SMC,.SFC,.SWC,.FIG,.MGD,.UFO,.BIN,',13,10
          db '                                  .058,.078,.1,.USA,.EUR,.JAP',13,10
%ifdef __MSDOS__
          db '',13,10
          db '  Microsoft-style options (/option) are also accepted',13,10
%endif
          db '',13,10,0

SECTION .text

%ifndef __LINUX__
NEWSYM obtaindir
    cmp byte[cfgloadsdir],1
    je .nosdriveb
    mov ebx,SRAMDir
    mov edx,SRAMDrive
    call Get_Dir
.nosdriveb
    cmp byte[cfgloadgdir],1
    je .noldriveb
    mov ebx,LoadDir
    mov edx,LoadDrive
    call Get_Dir
.noldriveb
    ret
%endif


NEWSYM preparedir
;Function 47h - Get current directory
;------------------------------------
;
;     AH = 47h
;     DL = drive number (0 = default, 1 = A: etc.)
;     DS:ESI -> 64 byte buffer to receive ASCIZ pathname
; get current drive, ah = 19h, al = A=0,B=1, etc.

%ifndef __LINUX__
    cmp byte[cfgloadsdir],0
    je near .nosdrivec
    ; verify sram drive/directory exists
    ; change dir to SRAMDrive/SRAMDir
    mov dl,[SRAMDrive]
    mov ebx,SRAMDir
    call Change_Dir
    jc .sramerror
    jmp .yessdrive
.sramerror
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir

    mov byte[cfgloadsdir],0
    ; Get drive/dir
    mov ebx,SRAMDir
    mov edx,SRAMDrive
    call Get_Dir

    mov edx,.sramerrorm
    call PrintStr
    call WaitForKey
    cmp al,27
    jne .noesc
    jmp DosExit
.noesc
    mov edx,.enter
    call PrintStr
    jmp .nosdrivec

.yessdrive
    mov dl,[InitDrive]
    mov ebx,InitDir
    call Change_Dir
.nosdrivec
%endif
    ret


SECTION .data
.sramerrorm db 'Invalid SRAM Directory in ZSNES.CFG!',13,10,13,10
            db 'Press any key to continue.',0
.enter      db 13,10,0

NEWSYM InitDrive, db 2
NEWSYM SRAMDrive, db 2
NEWSYM LoadDrive, db 2

%ifdef __LINUX__
NEWSYM gotoroot, db '/',0
%else
NEWSYM gotoroot, db '\',0
%endif

SECTION .bss
NEWSYM InitDir, resb 512
NEWSYM SRAMDir, resb 512
NEWSYM LoadDir, resb 512

SECTION .text

NEWSYM DosExit ; Terminate Program
%ifdef __WIN32__
	call OSExit
%elifdef __LINUX__
	call LinuxExit
%elifdef __MSDOS__
	jmp .nodeallocate
	mov ebx,memfreearray
	.nextdeallocate
	mov eax,[ebx]
	or eax,eax
	jz .nodeallocate
	push ebx
	push eax
	call free
	pop eax
	pop ebx
	add ebx,4
	jmp .nextdeallocate
.nodeallocate
	call init18_2hz
	mov    ax,4c00h            ;terminate
	int    21h
%endif

NEWSYM MMXCheck
    ; Check for cpu that doesn't support CPUID

    cmp byte[MMXSupport],0
    je .nommx

    ; Real way to check for presence of CPUID instruction  -kode54
    pushfd
    pop eax
    mov edx,eax
    xor eax,1 << 21
    push eax
    popfd
    pushfd
    pop eax
    xor eax,edx
    jz .nommx

    ; MMX support
    mov byte[MMXSupport],0
    mov eax,1
    CPUID

    test edx,1 << 23
    jz .nommx
    mov byte[MMXSupport],1
    cmp byte[GUIOn2],1
    je .noprintstr
    mov edx,YesMMX
    call PrintStr
.noprintstr
.nommx
    ret

NEWSYM UIAsmEnd
