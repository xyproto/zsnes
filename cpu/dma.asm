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

%include "cpu/regs.mac"
%include "cpu/regsw.mac"
%include "macros.mac"

EXTSYM c_reg420Bw
EXTSYM c_reg420Cw

;*******************************************************
; Transfer DMA                     Inits & Transfers DMA
;*******************************************************
; DMA transfer register

section .text

; DMA enable register
; use dmadata for input on dma
NEWSYM reg420Bw
    ccallv c_reg420Bw, eax
    ret

NEWSYM reg420Cw
    ccallv c_reg420Cw, eax
    ret
