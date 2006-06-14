;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;version 2 as published by the Free Software Foundation.
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

EXTSYM PrintStr,PrintChar,ram7fa,wramdataa,MMXSupport
EXTSYM MMXextSupport,malloc_ptr,malloc_size,malloc_help
EXTSYM BitConv32Ptr,spcBuffera,spritetablea,vcache2bs,vcache4bs,vcache8bs
EXTSYM RGBtoYUVPtr,newgfx16b,vidbuffer,vidbufferofsa,vidbufferofsmos,ngwinptr
EXTSYM vidbufferofsb,headdata,romdata,sfxramdata,setaramdata,wramdata,ram7f,vram
EXTSYM sram,debugbuf,regptr,regptw,vcache2b,vcache4b,vcache8b
EXTSYM vidbufferofsc,Sup48mbit,Sup16mbit,init18_2hz

%ifdef __UNIXSDL__
EXTSYM LinuxExit
%elifdef __WIN32__
EXTSYM exit
%endif

; Function 0501h
; User Interface

; Search for CMDLINE= for commandline entry

SECTION .text

ALIGN32
NEWSYM outofmemory
	mov edx,outofmem
	call PrintStr
	jmp DosExit

SECTION .data
NEWSYM outofmem, db 'You don',39,'t have enough memory to run this program!',13,10,0
NEWSYM YesMMX, db 'MMX support found and enabled.',13,10,13,10,0

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

; dspproc.asm

NEWSYM spcRamcmp, resb 65536
NEWSYM VolumeConvTable, resw 32768
NEWSYM dspWptr,  resd 256
NEWSYM dspRptr,  resd 256

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
    mov eax,%1
    add eax,1000h
    mov [malloc_size],eax
    pushad
    call malloc_help
    popad
    mov eax,[malloc_ptr]
    cmp eax,0
    je near %3
    mov ebx,[cmemallocptr]
    add dword[cmemallocptr],4
    mov [ebx],eax
    and eax,0FFFFFFE0h
    add eax,40h
    mov [%2],eax
%endmacro

%macro AllocmemOkay 3
    mov eax,%1
    add eax,1000h
    mov [malloc_size],eax
    pushad
    call malloc_help
    popad
    mov eax,[malloc_ptr]
    push eax
    and eax,0FFFFFFE0h
    add eax,40h
    mov [%2],eax
    pop eax
    cmp eax,0
    je %%nomalloc
    mov ebx,[cmemallocptr]
    add dword[cmemallocptr],4
    mov [ebx],eax
%%nomalloc
    cmp eax,0
    jne near %3
%endmacro

NEWSYM allocptr
    mov dword[cmemallocptr],memfreearray


%ifndef __MSDOS__
    AllocmemFail 4096+65536*16,BitConv32Ptr,outofmemory
%endif

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
    mov [vidbuffer],eax
    mov [vidbufferofsa],eax
    add eax,75036
    mov [vidbufferofsmos],eax

    mov eax,[ngwinptrb]
    and eax,0FFFFFFF8h
    add eax,2048
    mov [ngwinptr],eax

    mov eax,[vbufeptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov [vidbufferofsb],eax

    mov eax,[vbufdptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov [vidbufferofsc],eax

    mov eax,[romaptr]
    and eax,0FFFFFFF8h
    add eax,8
    mov [headdata],eax
    mov [romdata],eax
    add eax,4194304
    mov [sfxramdata],eax
    mov dword[setaramdata],eax	; share ram data with sfx
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
    mov [vcache2b],eax
    ; 4-bit = 128k
    mov eax,vcache4ba
    and eax,0FFFFFFF8h
    add eax,8
    mov [vcache4b],eax
    ; 8 bit = 64k
    mov eax,vcache8ba
    and eax,0FFFFFFF8h
    add eax,8
    mov [vcache8b],eax
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

SECTION .data
NEWSYM InitDrive, db 2

SECTION .bss
NEWSYM InitDir, resb 512

SECTION .text

NEWSYM DosExit ; Terminate Program
%ifdef __WIN32__
	call exit
%elifdef __UNIXSDL__
	call LinuxExit
%elifdef __MSDOS__
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
    xor eax,eax
    jz .nommx

    ; MMX support
    mov byte[MMXSupport],0
    mov byte[MMXextSupport],0
    mov eax,1
    CPUID

    test edx,1 << 23
    jz .nommx
    mov byte[MMXSupport],1

    ; Check if CPU has SSE (also support mmxext)
    test edx,1 << 25
    jz .tryextmmx
    mov byte[MMXextSupport],1
    jmp .nommx

.tryextmmx
    ; Test extended CPU flag
    mov eax,80000001h
    CPUID
    test edx,1 << 22
    jz .nommx
    mov byte[MMXextSupport],1
.noprintstr
.nommx
    ret
