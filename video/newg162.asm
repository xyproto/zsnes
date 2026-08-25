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
EXTSYM c_drawtile2b_nt,c_drawtile2b_t,c_drawtile2b_mst,c_drawtile2b_msnt
EXTSYM c_drawtile4b_nt,c_drawtile4b_t,c_drawtile4b_mst,c_drawtile4b_msnt
EXTSYM c_drawtile8b_nt,c_drawtile8b_t,c_drawtile8b_mst,c_drawtile8b_msnt
EXTSYM c_drawtile2b_win,c_drawtile2b_wint,c_drawtile2b_mstmsw,c_drawtile2b_msntmsw
EXTSYM c_drawtile2b_mstmw,c_drawtile2b_mstsw,c_drawtile2b_msntmw,c_drawtile2b_msntsw
EXTSYM c_drawtile4b_win,c_drawtile4b_wint,c_drawtile4b_mstmsw,c_drawtile4b_msntmsw
EXTSYM c_drawtile4b_mstmw,c_drawtile4b_mstsw,c_drawtile4b_msntmw,c_drawtile4b_msntsw
EXTSYM c_drawtile8b_win,c_drawtile8b_wint,c_drawtile8b_mstmsw,c_drawtile8b_msntmsw
EXTSYM c_drawtile8b_mstmw,c_drawtile8b_mstsw,c_drawtile8b_msntmw,c_drawtile8b_msntsw
EXTSYM c_drawtile16x162b_nt,c_drawtile16x162b_t,c_drawtile16x162b_mst,c_drawtile16x162b_msnt
EXTSYM c_drawtile16x164b_nt,c_drawtile16x164b_t,c_drawtile16x164b_mst,c_drawtile16x164b_msnt
EXTSYM c_drawtile16x168b_nt,c_drawtile16x168b_t,c_drawtile16x168b_mst,c_drawtile16x168b_msnt
EXTSYM c_drawtile16x162b_win,c_drawtile16x162b_wint,c_drawtile16x162b_mstmsw,c_drawtile16x162b_msntmsw
EXTSYM c_drawtile16x162b_mstmw,c_drawtile16x162b_mstsw,c_drawtile16x162b_msntmw,c_drawtile16x162b_msntsw
EXTSYM c_drawtile16x164b_win,c_drawtile16x164b_wint,c_drawtile16x164b_mstmsw,c_drawtile16x164b_msntmsw
EXTSYM c_drawtile16x164b_mstmw,c_drawtile16x164b_mstsw,c_drawtile16x164b_msntmw,c_drawtile16x164b_msntsw
EXTSYM c_drawtile16x168b_win,c_drawtile16x168b_wint,c_drawtile16x168b_mstmsw,c_drawtile16x168b_msntmsw
EXTSYM c_drawtile16x168b_mstmw,c_drawtile16x168b_mstsw,c_drawtile16x168b_msntmw,c_drawtile16x168b_msntsw
EXTSYM c_drawline2b_nt,c_drawline2b_t,c_drawline2b_mst,c_drawline2b_msnt
EXTSYM c_drawline4b_nt,c_drawline4b_t,c_drawline4b_mst,c_drawline4b_msnt
EXTSYM c_drawline8b_nt,c_drawline8b_t,c_drawline8b_mst,c_drawline8b_msnt
EXTSYM c_drawline2b_win,c_drawline2b_wint,c_drawline2b_mstmsw,c_drawline2b_msntmsw
EXTSYM c_drawline2b_mstmw,c_drawline2b_mstsw,c_drawline2b_msntmw,c_drawline2b_msntsw
EXTSYM c_drawline4b_win,c_drawline4b_wint,c_drawline4b_mstmsw,c_drawline4b_msntmsw
EXTSYM c_drawline4b_mstmw,c_drawline4b_mstsw,c_drawline4b_msntmw,c_drawline4b_msntsw
EXTSYM c_drawline8b_win,c_drawline8b_wint,c_drawline8b_mstmsw,c_drawline8b_msntmsw
EXTSYM c_drawline8b_mstmw,c_drawline8b_mstsw,c_drawline8b_msntmw,c_drawline8b_msntsw
EXTSYM c_drawline16x162b_nt,c_drawline16x162b_t,c_drawline16x162b_mst,c_drawline16x162b_msnt
EXTSYM c_drawline16x164b_nt,c_drawline16x164b_t,c_drawline16x164b_mst,c_drawline16x164b_msnt
EXTSYM c_drawline16x168b_nt,c_drawline16x168b_t,c_drawline16x168b_mst,c_drawline16x168b_msnt
EXTSYM c_drawline16x162b_win,c_drawline16x162b_wint,c_drawline16x162b_mstmsw,c_drawline16x162b_msntmsw
EXTSYM c_drawline16x162b_mstmw,c_drawline16x162b_mstsw,c_drawline16x162b_msntmw,c_drawline16x162b_msntsw
EXTSYM c_drawline16x164b_win,c_drawline16x164b_wint,c_drawline16x164b_mstmsw,c_drawline16x164b_msntmsw
EXTSYM c_drawline16x164b_mstmw,c_drawline16x164b_mstsw,c_drawline16x164b_msntmw,c_drawline16x164b_msntsw
EXTSYM c_drawline16x168b_win,c_drawline16x168b_wint,c_drawline16x168b_mstmsw,c_drawline16x168b_msntmsw
EXTSYM c_drawline16x168b_mstmw,c_drawline16x168b_mstsw,c_drawline16x168b_msntmw,c_drawline16x168b_msntsw
EXTSYM c_drawline16x82b_nt,c_drawline16x82b_t,c_drawline16x82b_mst,c_drawline16x82b_msnt
EXTSYM c_drawline16x84b_nt,c_drawline16x84b_t,c_drawline16x84b_mst,c_drawline16x84b_msnt
EXTSYM c_drawlineom2b_nt,c_drawlineom2b_t,c_drawlineom2b_mst,c_drawlineom2b_msnt
EXTSYM c_drawlineom4b_nt,c_drawlineom4b_t,c_drawlineom4b_mst,c_drawlineom4b_msnt
EXTSYM c_drawlineom8b_nt,c_drawlineom8b_t,c_drawlineom8b_mst,c_drawlineom8b_msnt
EXTSYM c_drawlineom2b_win,c_drawlineom2b_wint,c_drawlineom2b_mstmsw,c_drawlineom2b_msntmsw
EXTSYM c_drawlineom2b_mstmw,c_drawlineom2b_mstsw,c_drawlineom2b_msntmw,c_drawlineom2b_msntsw
EXTSYM c_drawlineom4b_win,c_drawlineom4b_wint,c_drawlineom4b_mstmsw,c_drawlineom4b_msntmsw
EXTSYM c_drawlineom4b_mstmw,c_drawlineom4b_mstsw,c_drawlineom4b_msntmw,c_drawlineom4b_msntsw
EXTSYM c_drawlineom8b_win,c_drawlineom8b_wint,c_drawlineom8b_mstmsw,c_drawlineom8b_msntmsw
EXTSYM c_drawlineom8b_mstmw,c_drawlineom8b_mstsw,c_drawlineom8b_msntmw,c_drawlineom8b_msntsw
EXTSYM c_drawlineom16x162b_nt,c_drawlineom16x162b_t,c_drawlineom16x162b_mst,c_drawlineom16x162b_msnt
EXTSYM c_drawlineom16x164b_nt,c_drawlineom16x164b_t,c_drawlineom16x164b_mst,c_drawlineom16x164b_msnt
EXTSYM c_drawlineom16x168b_nt,c_drawlineom16x168b_t,c_drawlineom16x168b_mst,c_drawlineom16x168b_msnt
EXTSYM c_drawlineom16x162b_win,c_drawlineom16x162b_wint,c_drawlineom16x162b_mstmsw,c_drawlineom16x162b_msntmsw
EXTSYM c_drawlineom16x162b_mstmw,c_drawlineom16x162b_mstsw,c_drawlineom16x162b_msntmw,c_drawlineom16x162b_msntsw
EXTSYM c_drawlineom16x164b_win,c_drawlineom16x164b_wint,c_drawlineom16x164b_mstmsw,c_drawlineom16x164b_msntmsw
EXTSYM c_drawlineom16x164b_mstmw,c_drawlineom16x164b_mstsw,c_drawlineom16x164b_msntmw,c_drawlineom16x164b_msntsw
EXTSYM c_drawlineom16x168b_win,c_drawlineom16x168b_wint,c_drawlineom16x168b_mstmsw,c_drawlineom16x168b_msntmsw
EXTSYM c_drawlineom16x168b_mstmw,c_drawlineom16x168b_mstsw,c_drawlineom16x168b_msntmw,c_drawlineom16x168b_msntsw
EXTSYM c_ng_drawtileng2b16b
EXTSYM c_ng_drawtileng4b16b
EXTSYM c_ng_drawtileng8b16b
EXTSYM c_ng_drawtileng16x162b16b
EXTSYM c_ng_drawtileng16x164b16b
EXTSYM c_ng_drawtileng16x168b16b
EXTSYM c_ng_drawlineng2b16b
EXTSYM c_ng_drawlineng4b16b
EXTSYM c_ng_drawlineng8b16b
EXTSYM c_ng_drawlineng16x162b16b
EXTSYM c_ng_drawlineng16x164b16b
EXTSYM c_ng_drawlineng16x168b16b
EXTSYM c_ng_drawlineng16x84b16b
EXTSYM c_ng_drawlineng16x82b16b
EXTSYM c_ng_drawlinengom2b16b
EXTSYM c_ng_drawlinengom4b16b
EXTSYM c_ng_drawlinengom8b16b
EXTSYM c_ng_drawlinengom16x162b16b
EXTSYM c_ng_drawlinengom16x164b16b
EXTSYM c_ng_drawlinengom16x168b16b
EXTSYM ng2_mosaic
EXTSYM c_determinetransp,c_checkwindowing,c_determinewindow,ng_branch
EXTSYM ngwintable,ngwinen,ngcwinptr,ngcpixleft,ngcwinmode,tleftn,ng16bprval
EXTSYM vrama,bg1drwng,ng16bbgval,bg1totng,bgtxadd,taddnfy16x16,taddfy16x16
EXTSYM switch16x16,curmosaicsz,domosaicng16b,vidmemch2,vidmemch4,vidmemch8
EXTSYM mode0add,vcache4b,vcache2b,vcache8b,cachesingle2bng,cachesingle8bng
EXTSYM ngpalcon4b,ngpalcon8b,ngpalcon2b,tleftnb,tltype2b,tltype4b,tltype8b
EXTSYM yposng,flipyposng,ofsmcptr,ofsmtptr,ofsmmptr,ofsmcyps,ofsmady,ofsmadx
EXTSYM FillSubScr,UnusedBitXor,yposngom,flipyposngom,ofsmval,ofsmvalh
EXTSYM CPalPtrng,BGMS1,scadtng,CMainWinScr,CSubWinScr,UnusedBit,res640
EXTSYM mosclineval,mostranspval,vcache2bs,vcache4bs,vcache8bs,vidmemch2s
EXTSYM vidmemch4s,vidmemch8s,bgtxadd2,SpecialLine,cachesingle4bng
EXTSYM ofshvaladd,ofsmtptrs,ofsmcptr2,ngptrdat2

; video/newg162.mac and video/newg16wn.mac held the tile and line renderers
; and their pixel writers, and the gating trees above them were here. All of
; that is video/c_ng2tile.c and video/c_ng2gate.c now; what is left is twenty
; entry seams, one per routine.

SECTION .text

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro








NEWSYM drawtileng2b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng2b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng4b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng4b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng8b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng8b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng16x162b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng16x162b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng16x164b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng16x164b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng16x168b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawtileng16x168b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng2b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng2b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng4b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng4b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng8b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng8b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x162b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng16x162b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x164b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng16x164b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x168b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng16x168b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x84b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng16x84b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x82b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlineng16x82b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom2b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom2b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom4b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom4b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom8b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom8b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom16x162b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom16x162b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom16x164b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom16x164b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlinengom16x168b16b
    ; The gating tree moved to video/c_ng2tile.c; it picks the leaf and calls
    ; it, so one seam at the entry serves the whole routine. Reached by jmp
    ; with one word pushed, hence the pop.
    pushad
    mov eax, esp
    ccall c_ng_drawlinengom16x168b16b, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
