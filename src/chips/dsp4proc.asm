;Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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

EXTSYM dsp4_address,dsp4_byte,DSP4GetByte,DSP4SetByte

NEWSYM DSP4Read8b
    mov word[dsp4_address],cx
    pushad
    call DSP4GetByte
    popad
    mov al,byte[dsp4_byte]
    ret

NEWSYM DSP4Write8b
    mov word[dsp4_address],cx
    mov byte[dsp4_byte],al
    pushad
    call DSP4SetByte
    popad
    ret
    
NEWSYM DSP4Read16b
    mov word[dsp4_address],cx
    pushad
    call DSP4GetByte
    popad
    mov al,byte[dsp4_byte]
    pushad
    call DSP4GetByte
    popad    
    mov ah,byte[dsp4_byte]    
    ret

NEWSYM DSP4Write16b
    mov word[dsp4_address],cx
    mov byte[dsp4_byte],al
    pushad
    call DSP4SetByte
    mov byte[dsp4_byte],ah
    call DSP4SetByte
    popad
    ret
    
