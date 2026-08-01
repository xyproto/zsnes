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
NEWSYM reg2115w
    and al,11111111b
    mov [vraminctype],al
    mov bl,al
    and bl,00000011b
    cmp bl,0
    jne .skip1
    mov word[addrincr],2
.skip1
    cmp bl,1
    jne .skip2
    mov word[addrincr],64
.skip2
    cmp bl,2
    jne .skip3
    mov word[addrincr],256
.skip3
    cmp bl,3
    jne .skip4
    mov word[addrincr],256
.skip4
    mov bl,al
    mov byte[vramincby8on],0
    and bl,00001100b
    jz near .noincby8
    mov byte[vramincby8on],1
    cmp bl,4
    jne .nextinc8
    mov byte[vramincby8left],64-1
    mov byte[vramincby8totl],5
    mov word[vramincby8ptri],65535-511
    mov word[vramincby8var],256+128+64
.nextinc8
    cmp bl,8
    jne .nextinc8b
    mov byte[vramincby8left],128-1
    mov byte[vramincby8totl],6
    mov word[vramincby8ptri],65535-1023
    mov word[vramincby8var],512+256+128
.nextinc8b
    cmp bl,12
    jne .nextinc8c
    mov byte[vramincby8left],256-1
    mov byte[vramincby8totl],7
    mov word[vramincby8ptri],65535-2047
    mov word[vramincby8var],1024+512+256
.nextinc8c
    test al,80h
    jz .from2118
    mov dword regptw(0x2118),reg2118inc8
    mov dword regptw(0x2119),reg2119inc8inc
    mov byte[vramincr],0
    jmp .from2119
.from2118
    mov dword regptw(0x2118),reg2118inc8inc
    mov dword regptw(0x2119),reg2119inc8
    mov byte[vramincr],1
.from2119
    ret
.noincby8
    test al,80h
    jz .from2118b
    mov dword regptw(0x2118),reg2118
    mov dword regptw(0x2119),reg2119inc
    mov byte[vramincr],0
    jmp .from2119b
.from2118b
    mov dword regptw(0x2118),reg2118inc
    mov dword regptw(0x2119),reg2119
    mov byte[vramincr],1
.from2119b
    ret


; Video port data (Low)
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

NEWSYM reg4016w
    test byte[INTEnab],1
    jnz .nointenab
    mov ebx,[JoyAOrig]
    or ebx,0FFFFh
    mov [JoyANow],ebx
    mov ebx,[JoyBOrig]
    or ebx,0FFFFh
    mov [JoyBNow],ebx
    mov ebx,[JoyCOrig]
    or ebx,0FFFFh
    mov [JoyCNow],ebx
    mov ebx,[JoyDOrig]
    or ebx,0FFFFh
    mov [JoyDNow],ebx
    mov ebx,[JoyEOrig]
    or ebx,0FFFFh
    mov [JoyENow],ebx
    cmp al,01h
    jne .noreset
    or byte[MultiTapStat],1
    ret
.noreset
    and byte[MultiTapStat],0FEh
    ret
.nointenab
    cmp al,01h
    jne .noone
    or byte[MultiTapStat],1
    or byte[JoyCRead],2
    ret
.noone
    and byte[MultiTapStat],0FEh
    cmp al,0
    jne near .nozero
    or byte[JoyCRead],1
    cmp byte[JoyCRead],3
    jne near .nozero
.resetports
    mov ebx,[JoyAOrig]
    or ebx,0FFFFh
    mov [JoyANow],ebx
    or ebx,0FFFFh
    mov ebx,[JoyBOrig]
    or ebx,0FFFFh
    mov [JoyBNow],ebx
    or ebx,0FFFFh
    mov ebx,[JoyCOrig]
    or ebx,0FFFFh
    mov [JoyCNow],ebx
    or ebx,0FFFFh
    mov ebx,[JoyDOrig]
    or ebx,0FFFFh
    mov [JoyDNow],ebx
    or ebx,0FFFFh
    mov ebx,[JoyEOrig]
    or ebx,0FFFFh
    mov [JoyENow],ebx
.nozero
    ret




; DMA Control register
