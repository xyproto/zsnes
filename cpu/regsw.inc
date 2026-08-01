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



EXTSYM reg2118,reg2118inc,reg2118inc8,reg2118inc8inc
EXTSYM reg2119,reg2119inc,reg2119inc8,reg2119inc8inc
EXTSYM reg420Bw,reg420Cw,regptwa,NextLineCache,vidmemch2,vidmemch4
EXTSYM vidmemch8,vrama,nmirept,SPCRAM,HIRQCycNext,HIRQNextExe,tableadc
EXTSYM cycpb268,cycpb358,cycpbl,cycpblt,opexec268,opexec268cph,opexec358
EXTSYM opexec358cph

%ifndef NO_DEBUGGER
EXTSYM sndwrit,debstop
%endif

; video memory change buffer for caching (65536/16=4096)
;vidmemch2, vidmemch4, vidmemch8. 4096 bytes each

;*******************************************************
; Registers    Note : restore AH, ECX, ESI, EDI, *S & DX
;*******************************************************


section .text
%macro UpdateScrollRegX 1
    mov bh,al
    mov bl,[bgscrolPrev]
    mov [bgscrolPrev],al
    shl ebx,13
    mov bh,[%1+1]
    shl bh,5
    shr ebx,13
    mov [%1],bx
%endmacro

%macro UpdateScrollRegY 1
    mov bl,[bgscrolPrev]
    mov bh,al
    mov [%1],bx
    mov [bgscrolPrev],al
%endmacro

; BG1 horizontal scroll register
%ifndef NO_DEBUGGER
    mov byte[debstop],1
%endif
.skip
    mov ebx,[vramaddr]
;    cmp [vrama+ebx+1],al
;    je .nochange
    mov [vrama+ebx+1],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange
    ret

SECTION .text

%macro Mode7Regs 1
    mov bl,[%1+1]
    ;mov bl,[m7byte]
    mov [%1],bl
    mov [%1+1],al
    ;mov [m7byte],al
%endmacro


; Sound Register #1
NEWSYM reg2140w
%ifndef NO_DEBUGGER
    mov byte[sndwrit],1
%endif
    cmp byte[nmistatus],2
    jne .n
    mov byte[nmirept],0
.n
    mov [SPCRAM+0F4h],al
    inc dword[SPC700write]
    reenablespc
    ret
;    cmp dword[cycpbl],0FFFFh
;    ja .spcreset
;    ret
;.spcreset
;    mov dword[cycpbl],100
;    ret

; Sound Register #2
NEWSYM reg2141w
%ifndef NO_DEBUGGER
    mov byte[sndwrit],1
%endif
    mov [SPCRAM+0F5h],al
    inc dword[SPC700write]
    reenablespc
    ret
;    cmp dword[cycpbl],0FFFFh
;    ja .spcreset
;    ret
;.spcreset
;    mov dword[cycpbl],100
;    ret

; Sound Register #3
NEWSYM reg2142w
%ifndef NO_DEBUGGER
    mov byte[sndwrit],1
%endif
    mov [SPCRAM+0F6h],al
    inc dword[SPC700write]
    reenablespc
    ret
;    cmp dword[cycpbl],0FFFFh
;    ja .spcreset
;    ret
;.spcreset
;    mov dword[cycpbl],100
;    ret

; Sound Register #4
NEWSYM reg2143w
%ifndef NO_DEBUGGER
    mov byte[sndwrit],1
%endif
    mov [SPCRAM+0F7h],al
    inc dword[SPC700write]
    reenablespc
    ret
;    cmp dword[cycpbl],0FFFFh
;    ja .spcreset
;    ret
;.spcreset
;    mov dword[cycpbl],100
;    ret

; Joystick 1 & 2 status bytes
SECTION .text
