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

%macro ALIGN32 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro

%macro ALIGN16 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro
EXTSYM mode7tab,winptrref,nglogicval,winlogicaval,curmosaicsz,curvidoffset
EXTSYM cwinptr,mode7A,mode7B,mode7C,mode7D,mode7X0,mode7Y0,mode7set,vram,vrama
EXTSYM xtravbuf,mode7hr,UnusedBitXor,UnusedBit,scrndis
EXTSYM mode7ab,mode7cd,BGMA,BG1SXl,BG1SYl

; The scratch block moved to video/c_mode716data.c; the spacers between the
; entries are part of its contract, so see that file before adding anything.
EXTSYM mtemp,mmode7xpos,mtempa2,mmode7xrpos,mtempa,mmode7ypos,mtempb2
EXTSYM mmode7yrpos,mtempb,mmode7xadder,mtempc2,mmode7xadd2,mtempc
EXTSYM mmode7yadder,mtempd2,mmode7yadd2,mtempd,mmode7ptr,mmode7xinc
EXTSYM mmode7xincc,mmode7yinc,mmode7xsloc,mmode7ysloc,mmode7xsrl,mmode7ysrl
EXTSYM mcxloc,mcyloc,M7HROn,switchtorep3,m7xaddof,m7xaddof2,m7yaddof
EXTSYM m7yaddof2,pixelsleft,mm7xaddof,mm7xaddof2,mm7yaddof,mm7yaddof2
EXTSYM ngwleft,ngwleftb,mode7xpos,mode7ypos,mode7xrpos,mode7yrpos
EXTSYM mode7xadder,mode7yadder,m7starty
EXTSYM M7SeamA,M7SeamB,M7SeamC,M7SeamD,c_CalculateNewValues
EXTSYM M7SeamSI,M7SeamDI,M7SeamBP,c_processmode7hires16b
EXTSYM c_drawmode7ngextbg216b
; Only the genuinely new names: M7P* and domosaicng16b are already declared in
; video/mode716.mac, and EXTSYM'ing a symbol twice re-prefixes it on PE/COFF.
EXTSYM M7DrawAX,M7DrawDX,M7DrawBX,M7DrawSI,M7DrawDI,M7DrawBP,M7DrawMosaic
EXTSYM c_drawmode7win16b,c_drawmode7ngextbg16b

%include "video/mode716.mac"

;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

%macro Mode7Normal 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalnt 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalt 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2+512]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalmsnt 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalmst 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2+512]
    mov [esi],dx
    and dx,[UnusedBitXor]
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalsnt 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Normalst 0
    or dl,dl
    jz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBG 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGnt 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGt 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2+512]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGmsnt 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi],dx
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGmst 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2+512]
    mov [esi+75036*2],dx
    or dx,[UnusedBit]
    mov [esi],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGsnt 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7ExtBGst 0
    mov [esi+75036*8],dl
    or dl,dl
    jz %%nodrawb
    test dl,80h
    jnz %%nodrawb
    mov dx,[ebp+edx*2]
    mov [esi+75036*2],dx
    xor edx,edx
%%nodrawb
    add esi,2
%endmacro

SECTION .text

; drawmode7win16b and drawmode7ngextbg16b moved to video/c_mode716draw.c,
; along with the four Mode7*Sub wrappers they were built from. Only the mosaic
; tail stays here: it is a tail-jump into domosaicng16b with dh live, which a
; C return cannot express.
%macro M7DRAW 1
NEWSYM %1
    mov [M7DrawAX], eax
    mov [M7DrawDX], edx
    mov [M7DrawBX], ebx
    mov [M7DrawSI], esi
    mov [M7DrawDI], edi
    mov [M7DrawBP], ebp
    call c_%1
    mov eax, [M7PAX]
    mov ebx, [M7PBX]
    mov ecx, [M7PCX]
    mov edx, [M7PDX]
    mov esi, [M7PSI]
    mov edi, [M7PDI]
    cmp dword[M7DrawMosaic], 0
    je %%nomosaic
    xor eax,eax
    mov dh,[curmosaicsz]
    jmp near domosaicng16b
%%nomosaic
    xor eax,eax
    mov dh,[curmosaicsz]
    ret
%endmacro

M7DRAW drawmode7win16b
M7DRAW drawmode7ngextbg16b

NEWSYM drawmode7ngextbg216b
    mov [M7SeamA], eax
    mov [M7SeamB], ebx
    mov [M7SeamC], ecx
    mov [M7SeamD], edx
    mov [M7SeamSI], esi
    mov [M7SeamBP], ebp
    call c_drawmode7ngextbg216b
    mov eax, [M7SeamA]
    mov ebx, [M7SeamB]
    mov ecx, [M7SeamC]
    mov edx, [M7SeamD]
    mov esi, [M7SeamSI]
    mov ebp, [M7SeamBP]
    ret

ALIGN32

SECTION .text

; CalculateNewValues moved to video/c_mode716calc.c. The seam carries the
; registers it reads and the three it hands back; a plain call is enough
; because the C half takes no arguments and cdecl keeps esi, edi and ebp.
CalculateNewValues:
    mov [M7SeamA], eax
    mov [M7SeamB], ebx
    mov [M7SeamD], edx
    call c_CalculateNewValues
    mov eax, [M7SeamA]
    mov ebx, [M7SeamB]
    mov ecx, [M7SeamC]
    mov edx, [M7SeamD]
    ret

NEWSYM processmode7hires16b
    mov [M7SeamA], eax
    mov [M7SeamB], ebx
    mov [M7SeamC], ecx
    mov [M7SeamD], edx
    mov [M7SeamSI], esi
    mov [M7SeamDI], edi
    mov [M7SeamBP], ebp
    call c_processmode7hires16b
    mov eax, [M7SeamA]
    mov ebx, [M7SeamB]
    mov ecx, [M7SeamC]
    mov edx, [M7SeamD]
    mov esi, [M7SeamSI]
    mov edi, [M7SeamDI]
    mov ebp, [M7SeamBP]
    ret
