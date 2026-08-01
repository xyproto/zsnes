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

NEWSYM drawmode7win16b
    test byte[scrndis],1
    jz .notdisabled
    ret
.notdisabled
    CheckTransparency 01h,drawmode7win16bt
normal
    Mode7NonMainSub Mode7Normal
drawmode7win16bt
    test byte[scadtng+ebx],1h
    jz near drawmode7win16bnt
    test byte[BGMS1+ebx*2+1],1h
    jnz near drawmode716bmst
    Mode7NonMainSub Mode7Normalt
drawmode716bmst:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bst
drawmode7w16bmst
    Mode7NonMainSub Mode7Normalmst
drawmode7w16bmt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmst
    Mode7MainSub Mode7Normalmst,Mode7Normalst
drawmode7w16bst
    Mode7MainSub Mode7Normalmst,Mode7Normalt
drawmode7win16bnt:
    test byte[BGMS1+ebx*2+1],1h
    jnz near drawsprngm716bmsnt
    Mode7NonMainSub Mode7Normalnt
drawsprngm716bmsnt:
    cmp dword[ngwinen],0
    je drawmode7w16bmsnt
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmnt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bsnt
drawmode7w16bmsnt
    Mode7NonMainSub Mode7Normalmsnt
drawmode7w16bmnt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmsnt
    Mode7MainSub Mode7Normalmsnt,Mode7Normalsnt
drawmode7w16bsnt
    Mode7MainSub Mode7Normalmsnt,Mode7Normalnt

NEWSYM drawmode7ngextbg16b
    test byte[scrndis],1
    jz .notdisabled
    ret
.notdisabled
    mov byte[curmosaicsz],1
    push ecx
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov ecx,256
.loop
    mov byte[esi+75036*8],0
    add esi,2
    dec ecx
    jnz .loop
    pop ecx

    cmp byte[mode7hr+ebx],1
;    je near drawmode7winextbghr16e
    CheckTransparency 01h,drawmode7win16bte
    CheckTransparency 02h,drawmode7win16bte
    mov esi,[cwinptr]
    mov [winptrref],esi
    mov esi,[curvidoffset]
    Mode7NonMainSube Mode7ExtBG
drawmode7win16bte
    test byte[scadtng+ebx],1h
    jz near drawmode7win16bnte
    test byte[BGMS1+ebx*2+1],1h
    jnz near drawmode716bmste
    Mode7NonMainSube Mode7ExtBGt
drawmode716bmste:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmte
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bste
drawmode7w16bmste
    Mode7NonMainSube Mode7ExtBGmst
drawmode7w16bmte
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmste
    Mode7MainSube Mode7ExtBGmst,Mode7ExtBGst
drawmode7w16bste
    Mode7MainSube Mode7ExtBGmst,Mode7ExtBGt
drawmode7win16bnte:
    test byte[BGMS1+ebx*2+1],1h
    jnz near drawsprngm716bmsnte
    Mode7NonMainSube Mode7ExtBGnt
drawsprngm716bmsnte:
    cmp dword[ngwinen],0
    je drawmode7w16bmsnte
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmnte
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bsnte
drawmode7w16bmsnte
    Mode7NonMainSube Mode7ExtBGmsnt
drawmode7w16bmnte
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx],0
    jne near drawmode7w16bmsnte
    Mode7MainSube Mode7ExtBGmsnt,Mode7ExtBGsnt
drawmode7w16bsnte
    Mode7MainSube Mode7ExtBGmsnt,Mode7ExtBGnt

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
