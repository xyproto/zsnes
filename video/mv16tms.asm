;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;https://zsnes.bountysource.com
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
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif
EXTSYM bgcoloradder,bgofwptr,bgsubby,bshifter,curbgpr,curmosaicsz,curvidoffset
EXTSYM cwinptr,domosaic16b,drawn,pal16b,scaddtype,scrnon,temp,tempcach,temptile
EXTSYM tileleft16b,transpbuf,winon,winptrref,xtravbuf,yadd,yadder,yrevadder
EXTSYM bgmode,vcache2b,vcache4b,vcache8b,fulladdtab,pal16bcl
EXTSYM pal16bxcl,coadder16,a16x16xinc,a16x16yinc,curypos,yflipadd

EXTSYM MVAX,MVBX,MVCX,MVDX,MVSI,c_draw16tms_setup
EXTSYM MVSAX,MVSBX,MVSCX,MVSDX,MVSSI,MVSDI,MVSBP,c_draw8x816tsms,c_draw8x816tswinonms
; domosaic16b and curmosaicsz are already declared above - EXTSYM'ing a symbol
; twice re-prefixes it on PE/COFF and only the win32 link notices.
EXTSYM MVSMosaic,c_draw8x816tms_body,c_draw8x816twinonms_body
EXTSYM c_draw16x1616tsms,c_draw16x1616twinonms,c_draw16x16fulladdwinonms
EXTSYM c_draw16x1616tswinonms,c_draw8x8fulladdms,c_draw8x8fulladdwinonms
EXTSYM c_draw16x1616tms_body,c_draw16x16fulladdms

%include "video/vidmacro.mac"

;*******************************************************
; Processes & Draws 8x8 tiles in 2, 4, & 8 bit mode
;*******************************************************

SECTION .text

NEWSYM draw16x1616tms
    mov byte[drawn],0
    mov [temp],eax
    mov eax,esi
    mov [yadder],ecx
    mov [tempcach],ebx
    mov ebx,56
    sub ebx,ecx
    mov [yrevadder],ebx
    xor ebx,ebx
    mov bl,[curypos]
    mov [temptile],edx
    ; set up y adders
    test byte[a16x16yinc],01h
    jz .noincrc
    mov word[yadd],16
    mov word[yflipadd],0
    jmp .yesincrc
.noincrc
    mov word[yadd],0
    mov word[yflipadd],16
.yesincrc
    ; esi = pointer to video buffer
    mov esi,[cwinptr]
    sub esi,eax
    mov [winptrref],esi
    mov esi,[curvidoffset]
    sub esi,eax           ; esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    sub esi,eax
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+32
    mov ecx,128
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+32
    sub esi,eax
    sub esi,eax
.nomosaic
    mov [temptile],edx
    push ecx
    mov dword[bgsubby],262144
    mov ecx,[vcache2b]
    add ecx,262144
    mov [bgofwptr],ecx
    cmp dword[tempcach],ecx
    jb .nobit
    mov dword[bgsubby],131072
    mov ecx,[vcache4b]
    add ecx,131072
    mov [bgofwptr],ecx
    cmp dword[tempcach],ecx
    jb .nobit
    mov ecx,[vcache8b]
    add ecx,65536
    mov [bgofwptr],ecx
    mov dword[bgsubby],65536
.nobit
    pop ecx
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x1616twinonms
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616tsms
    test byte[scaddtype],40h
    jz near draw16x16fulladdms
    cmp byte[scrnon+1],0
    je near draw16x16fulladdms
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x1616tms_body
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    cmp byte[MVSMosaic],0
    jne near domosaic16b
    ret

draw16x16fulladdms:
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x16fulladdms
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    cmp byte[MVSMosaic],0
    jne near domosaic16b
    ret

NEWSYM draw16x1616tsms
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x1616tsms
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    cmp byte[MVSMosaic],0
    jne near domosaic16b
    ret

NEWSYM draw16x1616twinonms
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616tswinonms
    test byte[scaddtype],40h
    jz near draw16x16fulladdwinonms
    cmp byte[scrnon+1],0
    je near draw16x16fulladdwinonms
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x1616twinonms
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    ret

NEWSYM draw16x16fulladdwinonms
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x16fulladdwinonms
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    ret

NEWSYM draw16x1616tswinonms
    mov [MVSAX], eax
    mov [MVSBX], ebx
    mov [MVSCX], ecx
    mov [MVSDX], edx
    mov [MVSSI], esi
    mov [MVSDI], edi
    mov [MVSBP], ebp
    call c_draw16x1616tswinonms
    mov eax, [MVSAX]
    mov ebx, [MVSBX]
    mov ecx, [MVSCX]
    mov edx, [MVSDX]
    mov esi, [MVSSI]
    mov edi, [MVSDI]
    mov ebp, [MVSBP]
    ret

    ret
