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



EXTSYM initsfxregsw,reg420Bw,reg420Cw,regptwa,initSA1regsw,SDD1Reset
EXTSYM SPC7110Reset,RTCReset2,NextLineCache,vidmemch2,vidmemch4
EXTSYM vidmemch8,vrama,nmirept,SPCRAM,HIRQCycNext,HIRQNextExe,tableadc
EXTSYM cycpb268,cycpb358,cycpbl,cycpblt,opexec268,opexec268cph,opexec358
EXTSYM opexec358cph

%ifndef NO_DEBUGGER
EXTSYM sndwrit,debstop
%endif

SECTION .text

NEWSYM initregw
    ; Fill register pointer with invalid register accesses
    mov edi,regptwa
    mov ecx,3000h
    mov eax,regINVALIDw
.loopa
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .loopa
    ; Set all valid register accesses
    setregw 2100h,reg2100w
    setregw 2101h,reg2101w
    setregw 2102h,reg2102w
    setregw 2103h,reg2103w
    setregw 2104h,reg2104w
    setregw 2105h,reg2105w
    setregw 2106h,reg2106w
    setregw 2107h,reg2107w
    setregw 2108h,reg2108w
    setregw 2109h,reg2109w
    setregw 210Ah,reg210Aw
    setregw 210Bh,reg210Bw
    setregw 210Ch,reg210Cw
    setregw 210Dh,reg210Dw
    setregw 210Eh,reg210Ew
    setregw 210Fh,reg210Fw
    setregw 2110h,reg2110w
    setregw 2111h,reg2111w
    setregw 2112h,reg2112w
    setregw 2113h,reg2113w
    setregw 2114h,reg2114w
    setregw 2115h,reg2115w
    setregw 2116h,reg2116w
    setregw 2117h,reg2117w
    setregw 2118h,reg2118
    setregw 2119h,reg2119inc
    setregw 211Ah,reg211Aw
    setregw 211Bh,reg211Bw
    setregw 211Ch,reg211Cw
    setregw 211Dh,reg211Dw
    setregw 211Eh,reg211Ew
    setregw 211Fh,reg211Fw
    setregw 2120h,reg2120w
    setregw 2121h,reg2121w
    setregw 2122h,reg2122w
    setregw 2123h,reg2123w
    setregw 2124h,reg2124w
    setregw 2125h,reg2125w
    setregw 2126h,reg2126w
    setregw 2127h,reg2127w
    setregw 2128h,reg2128w
    setregw 2129h,reg2129w
    setregw 212Ah,reg212Aw
    setregw 212Bh,reg212Bw
    setregw 212Ch,reg212Cw
    setregw 212Dh,reg212Dw
    setregw 212Eh,reg212Ew
    setregw 212Fh,reg212Fw
    setregw 2130h,reg2130w
    setregw 2131h,reg2131w
    setregw 2132h,reg2132w
    setregw 2133h,reg2133w
    setregw 2140h,reg2140w
    setregw 2141h,reg2141w
    setregw 2142h,reg2142w
    setregw 2143h,reg2143w
    setregw 2144h,reg2140w
    setregw 2145h,reg2141w
    setregw 2146h,reg2142w
    setregw 2147h,reg2143w
    setregw 2148h,reg2140w
    setregw 2149h,reg2141w
    setregw 214Ah,reg2142w
    setregw 214Bh,reg2143w
    setregw 214Ch,reg2140w
    setregw 214Dh,reg2141w
    setregw 214Eh,reg2142w
    setregw 214Fh,reg2143w
    setregw 2150h,reg2140w
    setregw 2151h,reg2141w
    setregw 2152h,reg2142w
    setregw 2153h,reg2143w
    setregw 2154h,reg2140w
    setregw 2155h,reg2141w
    setregw 2156h,reg2142w
    setregw 2157h,reg2143w
    setregw 2158h,reg2140w
    setregw 2159h,reg2141w
    setregw 215Ah,reg2142w
    setregw 215Bh,reg2143w
    setregw 215Ch,reg2140w
    setregw 215Dh,reg2141w
    setregw 215Eh,reg2142w
    setregw 215Fh,reg2143w
    setregw 2160h,reg2140w
    setregw 2161h,reg2141w
    setregw 2162h,reg2142w
    setregw 2163h,reg2143w
    setregw 2164h,reg2140w
    setregw 2165h,reg2141w
    setregw 2166h,reg2142w
    setregw 2167h,reg2143w
    setregw 2168h,reg2140w
    setregw 2169h,reg2141w
    setregw 216Ah,reg2142w
    setregw 216Bh,reg2143w
    setregw 216Ch,reg2140w
    setregw 216Dh,reg2141w
    setregw 216Eh,reg2142w
    setregw 216Fh,reg2143w
    setregw 2170h,reg2140w
    setregw 2171h,reg2141w
    setregw 2172h,reg2142w
    setregw 2173h,reg2143w
    setregw 2174h,reg2140w
    setregw 2175h,reg2141w
    setregw 2176h,reg2142w
    setregw 2177h,reg2143w
    setregw 2178h,reg2140w
    setregw 2179h,reg2141w
    setregw 217Ah,reg2142w
    setregw 217Bh,reg2143w
    setregw 217Ch,reg2140w
    setregw 217Dh,reg2141w
    setregw 217Eh,reg2142w
    setregw 217Fh,reg2143w
    setregw 2180h,reg2180w
    setregw 2181h,reg2181w
    setregw 2182h,reg2182w
    setregw 2183h,reg2183w
    setregw 4016h,reg4016w
    setregw 4200h,reg4200w
    setregw 4201h,reg4201w
    setregw 4202h,reg4202w
    setregw 4203h,reg4203w
    setregw 4204h,reg4204w
    setregw 4205h,reg4205w
    setregw 4206h,reg4206w
    setregw 4207h,reg4207w
    setregw 4208h,reg4208w
    setregw 4209h,reg4209w
    setregw 420Ah,reg420Aw
    setregw 420Bh,reg420Bw
    setregw 420Ch,reg420Cw
    setregw 420Dh,reg420Dw
    setregw 4300h,reg43X0w
    setregw 4301h,reg43X1w
    setregw 4302h,reg43x2w
    setregw 4303h,reg43x3w
    setregw 4304h,reg43x4w
    setregw 4305h,reg43x5w
    setregw 4306h,reg43x6w
    setregw 4307h,reg43x7w
    setregw 4308h,reg43x8w
    setregw 4309h,reg43x9w
    setregw 430Ah,reg43XAw
    setregw 430Bh,reg43XBw
    setregw 430Fh,reg43XBw
    setregw 4310h,reg43X0w
    setregw 4311h,reg43X1w
    setregw 4312h,reg43x2w
    setregw 4313h,reg43x3w
    setregw 4314h,reg43x4w
    setregw 4315h,reg43x5w
    setregw 4316h,reg43x6w
    setregw 4317h,reg43x7w
    setregw 4318h,reg43x8w
    setregw 4319h,reg43x9w
    setregw 431Ah,reg43XAw
    setregw 431Bh,reg43XBw
    setregw 431Fh,reg43XBw
    setregw 4320h,reg43X0w
    setregw 4321h,reg43X1w
    setregw 4322h,reg43x2w
    setregw 4323h,reg43x3w
    setregw 4324h,reg43x4w
    setregw 4325h,reg43x5w
    setregw 4326h,reg43x6w
    setregw 4327h,reg43x7w
    setregw 4328h,reg43x8w
    setregw 4329h,reg43x9w
    setregw 432Ah,reg43XAw
    setregw 432Bh,reg43XBw
    setregw 432Fh,reg43XBw
    setregw 4330h,reg43X0w
    setregw 4331h,reg43X1w
    setregw 4332h,reg43x2w
    setregw 4333h,reg43x3w
    setregw 4334h,reg43x4w
    setregw 4335h,reg43x5w
    setregw 4336h,reg43x6w
    setregw 4337h,reg43x7w
    setregw 4338h,reg43x8w
    setregw 4339h,reg43x9w
    setregw 433Ah,reg43XAw
    setregw 433Bh,reg43XBw
    setregw 433Fh,reg43XBw
    setregw 4340h,reg43X0w
    setregw 4341h,reg43X1w
    setregw 4342h,reg43x2w
    setregw 4343h,reg43x3w
    setregw 4344h,reg43x4w
    setregw 4345h,reg43x5w
    setregw 4346h,reg43x6w
    setregw 4347h,reg43x7w
    setregw 4348h,reg43x8w
    setregw 4349h,reg43x9w
    setregw 434Ah,reg43XAw
    setregw 434Bh,reg43XBw
    setregw 434Fh,reg43XBw
    setregw 4350h,reg43X0w
    setregw 4351h,reg43X1w
    setregw 4352h,reg43x2w
    setregw 4353h,reg43x3w
    setregw 4354h,reg43x4w
    setregw 4355h,reg43x5w
    setregw 4356h,reg43x6w
    setregw 4357h,reg43x7w
    setregw 4358h,reg43x8w
    setregw 4359h,reg43x9w
    setregw 435Ah,reg43XAw
    setregw 435Bh,reg43XBw
    setregw 435Fh,reg43XBw
    setregw 4360h,reg43X0w
    setregw 4361h,reg43X1w
    setregw 4362h,reg43x2w
    setregw 4363h,reg43x3w
    setregw 4364h,reg43x4w
    setregw 4365h,reg43x5w
    setregw 4366h,reg43x6w
    setregw 4367h,reg43x7w
    setregw 4368h,reg43x8w
    setregw 4369h,reg43x9w
    setregw 436Ah,reg43XAw
    setregw 436Bh,reg43XBw
    setregw 436Fh,reg43XBw
    setregw 4370h,reg43X0w
    setregw 4371h,reg43X1w
    setregw 4372h,reg43x2w
    setregw 4373h,reg43x3w
    setregw 4374h,reg43x4w
    setregw 4375h,reg43x5w
    setregw 4376h,reg43x6w
    setregw 4377h,reg43x7w
    setregw 4378h,reg43x8w
    setregw 4379h,reg43x9w
    setregw 437Ah,reg43XAw
    setregw 437Bh,reg43XBw
    setregw 437Fh,reg43XBw
    cmp byte[SFXEnable],0
    je .nosfx
    call initsfxregsw
.nosfx
    cmp byte[SA1Enable],0
    je .nosa1
    call initSA1regsw
.nosa1
    cmp byte[SDD1Enable],0
    je .nosdd1
    call SDD1Reset
.nosdd1
    cmp byte[SPC7110Enable],0
    je .nospc7110
    call SPC7110Reset
.nospc7110
    cmp byte[RTCEnable],0
    je .nortc
    call RTCReset2
.nortc
    ret

; video memory change buffer for caching (65536/16=4096)
;vidmemch2, vidmemch4, vidmemch8. 4096 bytes each

;*******************************************************
; Registers    Note : restore AH, ECX, ESI, EDI, *S & DX
;*******************************************************


; Screen display register
reg2100w:
    mov [vidbright],al
    and byte[vidbright],0Fh
    mov [forceblnk],al
    and byte[forceblnk],80h
    ret

SECTION .bss
NEWSYM prevoamptr, resb 1
SECTION .text

; OAM size register
reg2101w:
    cmp byte[prevoamptr],0FFh
    je .noskip
    cmp byte[prevoamptr],al
    je near .noproc
.noskip
    mov [prevoamptr],al
    xor ebx,ebx
    mov bl,al
    and bl,03h
    shl bx,14
    mov [objptr],bx
    mov [objptrn],bx
    xor ebx,ebx
    mov bl,al
    and bl,18h
    shr bl,3
    shl bx,13
    add [objptrn],bx
    xor ebx,ebx
    mov bl,al
    shr bl,5
    push eax
    mov byte[NextLineCache],1
    mov al,[.objsize1+ebx]
    mov [objsize1],al
    mov al,[.objsize2+ebx]
    mov [objsize2],al
    mov al,[.objmovs1+ebx]
    mov [objmovs1],al
    mov al,[.objmovs2+ebx]
    mov [objmovs2],al
    mov ax,[.objadds1+ebx*2]
    mov [objadds1],ax
    mov ax,[.objadds2+ebx*2]
    mov [objadds2],ax
    pop eax
.noproc
    ret

SECTION .data
.objsize1 db 1,1,1,4,4,16,2*4,2*4
.objsize2 db 4,16,64,16,64,64,2*16,16
.objmovs1 db 2,2,2,2,2,4,2,2
.objmovs2 db 2,4,8,4,8,8,4,4
.objadds1 dw 14,14,14,14,14,12,14,14
.objadds2 dw 14,12,8,12,8,8,12,12

SECTION .bss
NEWSYM oamlow, resb 1
SECTION .text

; OAM address register
reg2102w:
    mov bx,[oamaddrs]
    mov [poamaddrs],bx
    shr bx,1
    mov bl,al
    shl bx,1
    mov [oamaddr],bx
;    or al,al
;    jz .skipstore
    mov bx,[oamaddr]
    mov [oamaddrs],bx
.skipstore
    cmp byte[nexthprior],1
    je .priorset
    mov byte[objhipr],0
    jmp .cachespr
.priorset
    mov bx,[oamaddr]
    shr bx,2
    and bl,7fh
    mov [objhipr],bl
.cachespr
    mov byte[NextLineCache],1
    ret

; OAM address register
reg2103w:
    mov bx,[oamaddrs]
    shr bx,1
    mov bh,al
    and bh,01h
    shl bx,1
    mov [oamaddr],bx
    cmp word[poamaddrs],200h
    jbe .notinvptr
    cmp word[oamaddr],200h
    jne .notinvptr
    mov bx,[poamaddrs]
    mov [oamaddr],bx
    mov byte[nosprincr],1
.notinvptr
    mov bx,[oamaddr]
    mov [oamaddrs],bx
    test al,80h
    jnz .hipri
    mov byte[nexthprior],0
    mov byte[NextLineCache],1
    ret
.hipri
    mov bx,[oamaddr]
    shr bx,2
    and bl,7fh
    mov [objhipr],bl
    mov byte[nexthprior],1
    mov byte[NextLineCache],1
    ret

; OAM data register
reg2104w:
    mov byte[NextLineCache],1
    mov ebx,[oamaddr]
    cmp byte[nosprincr],1
    je .noinc
    inc dword[oamaddr]
    cmp ebx,544
    jae .overflow
.noinc
    test ebx,200h
    jz .lowtable
    mov [oamram+ebx],al
    ret
.lowtable
    test ebx,1
    jz .oamlow
    mov [oamram+ebx],al
    dec ebx
    push eax
    mov al,[oamlow]
    mov [oamram+ebx],al
    pop eax
    ret
.overflow
    xor ebx,ebx
    mov dword[oamaddr],1
.oamlow
    mov [oamlow],al
    ret

; Screen mode register
reg2105w:
    mov bl,al
    and bl,00000111b
    mov [bgmode],bl
    mov bl,al
    shr bl,3
    and bl,01h
    mov [bg3highst],bl
    mov bl,al
    shr bl,4
    mov [bgtilesz],bl
    mov bl,al
    mov dword[BG116x16t],0
    add bl,bl
    adc byte[BG416x16t],0
    add bl,bl
    adc byte[BG316x16t],0
    add bl,bl
    adc byte[BG216x16t],0
    add bl,bl
    adc byte[BG116x16t],0
    ret

; Screen pixelation register
reg2106w:
    mov bl,al
    and bl,0Fh
    mov [mosaicon],bl
    mov bl,al
    shr bl,4
    mov [mosaicsz],bl
    ret

; BG1 VRAM location register
reg2107w:
    xor ebx,ebx
    mov bl,al
    shr bl,2
    shl bx,11
    mov [bg1ptr],bx
    mov [bg1ptrb],bx
    mov [bg1ptrc],bx
    mov [bg1ptrd],bx
    mov dword[bg1ptrx],0
    mov dword[bg1ptry],0
    mov bl,al
    and bl,00000011b
    mov [bg1scsize],bl
    cmp bl,1
    jne .skipa
    add word[bg1ptrb],800h
    add word[bg1ptrd],800h
    mov dword[bg1ptrx],800h
.skipa
    cmp bl,2
    jne .skipb
    add word[bg1ptrc],800h
    add word[bg1ptrd],800h
    mov dword[bg1ptry],800h
.skipb
    cmp bl,3
    jne .skipc
    add word[bg1ptrb],800h
    add word[bg1ptrc],1000h
    add word[bg1ptrd],1800h
    mov dword[bg1ptrx],800h
    mov dword[bg1ptry],1000h
.skipc
    ret

; BG2 VRAM location register
reg2108w:
    xor ebx,ebx
    mov bl,al
    shr bl,2
    shl bx,11
    mov [bg2ptr],bx
    mov [bg2ptrb],bx
    mov [bg2ptrc],bx
    mov [bg2ptrd],bx
    mov dword[bg2ptrx],0
    mov dword[bg2ptry],0
    mov bl,al
    and bl,00000011b
    mov [bg2scsize],bl
    cmp bl,1
    jne .skipa
    add word[bg2ptrb],800h
    add word[bg2ptrd],800h
    mov dword[bg2ptrx],800h
.skipa
    cmp bl,2
    jne .skipb
    add word[bg2ptrc],800h
    add word[bg2ptrd],800h
    mov dword[bg2ptry],800h
.skipb
    cmp bl,3
    jne .skipc
    add word[bg2ptrb],800h
    add word[bg2ptrc],1000h
    add word[bg2ptrd],1800h
    mov dword[bg2ptrx],800h
    mov dword[bg2ptry],1000h
.skipc
    ret

; BG3 VRAM location register
reg2109w:
    xor ebx,ebx
    mov bl,al
    shr bl,2
    shl bx,11
    mov [bg3ptr],bx
    mov [bg3ptrb],bx
    mov [bg3ptrc],bx
    mov [bg3ptrd],bx
    mov dword[bg3ptrx],0
    mov dword[bg3ptry],0
    mov bl,al
    and bl,00000011b
    mov [bg3scsize],bl
    cmp bl,1
    jne .skipa
    add word[bg3ptrb],800h
    add word[bg3ptrd],800h
    mov dword[bg3ptrx],800h
.skipa
    cmp bl,2
    jne .skipb
    add word[bg3ptrc],800h
    add word[bg3ptrd],800h
    mov dword[bg3ptry],800h
.skipb
    cmp bl,3
    jne .skipc
    add word[bg3ptrb],800h
    add word[bg3ptrc],1000h
    add word[bg3ptrd],1800h
    mov dword[bg3ptrx],800h
    mov dword[bg3ptry],1000h
.skipc
    ret

; BG4 VRAM location register
reg210Aw:
    xor ebx,ebx
    mov bl,al
    shr bl,2
    shl bx,11
    mov [bg4ptr],bx
    mov [bg4ptrb],bx
    mov [bg4ptrc],bx
    mov [bg4ptrd],bx
    mov dword[bg4ptrx],0
    mov dword[bg4ptry],0
    mov bl,al
    and bl,00000011b
    mov [bg4scsize],bl
    cmp bl,1
    jne .skipa
    add word[bg4ptrb],800h
    add word[bg4ptrd],800h
    mov dword[bg4ptrx],800h
.skipa
    cmp bl,2
    jne .skipb
    add word[bg4ptrc],800h
    add word[bg4ptrd],800h
    mov dword[bg4ptry],800h
.skipb
    cmp bl,3
    jne .skipc
    add word[bg4ptrb],800h
    add word[bg4ptrc],1000h
    add word[bg4ptrd],1800h
    mov dword[bg4ptrx],800h
    mov dword[bg4ptry],1000h
.skipc
    ret

; BG1 & BG2 VRAM location register
reg210Bw:
    xor ebx,ebx
    mov bl,al
    and bl,0Fh
    shl bx,13
    mov [bg1objptr],bx
    mov bl,al
    shr bl,4
    shl bx,13
    mov [bg2objptr],bx
    ret

; BG3 & BG4 VRAM location register
reg210Cw:
    xor ebx,ebx
    mov bl,al
    and bl,0Fh
    shl bx,13
    mov [bg3objptr],bx
    mov bl,al
    shr bl,4
    shl bx,13
    mov [bg4objptr],bx
    ret

section .data
NEWSYM bgscrolPrev, db 0
NEWSYM bg1scrolx_m7, dw 0
NEWSYM bg1scroly_m7, dw 0

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
reg210Dw:
    UpdateScrollRegX bg1scrolx
;    mov bl,[m7byte]
    mov [bg1scrolx_m7],bx
;    mov [m7byte],al
    ret

; BG1 vertical scroll register
reg210Ew:
    UpdateScrollRegY bg1scroly
;    mov bl,[m7byte]
    mov [bg1scroly_m7],bx
;    mov [m7byte],al
    ret

; BG2 horizontal scroll register
reg210Fw:
    UpdateScrollRegX bg2scrolx
    ret

; BG2 vertical scroll register
reg2110w:
    UpdateScrollRegY bg2scroly
    ret

; BG3 horizontal scroll register
reg2111w:
    UpdateScrollRegX bg3scrolx
    ret

; BG3 vertical scroll register
reg2112w:
    UpdateScrollRegY bg3scroly
    ret

; BG4 horizontal scroll register
reg2113w:
    UpdateScrollRegX bg4scrolx
    ret

; BG4 vertical scroll register
reg2114w:
    UpdateScrollRegY bg4scroly
    ret

; Video port control
reg2115w:
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
    mov regptw(0x2118),dword reg2118inc8
    mov regptw(0x2119),dword reg2119inc8inc
    mov byte[vramincr],0
    jmp .from2119
.from2118
    mov regptw(0x2118),dword reg2118inc8inc
    mov regptw(0x2119),dword reg2119inc8
    mov byte[vramincr],1
.from2119
    ret
.noincby8
    test al,80h
    jz .from2118b
    mov regptw(0x2118),dword reg2118
    mov regptw(0x2119),dword reg2119inc
    mov byte[vramincr],0
    jmp .from2119b
.from2118b
    mov regptw(0x2118),dword reg2118inc
    mov regptw(0x2119),dword reg2119
    mov byte[vramincr],1
.from2119b
    ret

; Video port address (Low)
reg2116w:
    shr word[vramaddr],1
    mov [vramaddr],al
    shl word[vramaddr],1
    mov byte[vramread],0
    ret

; Video port address (High)
reg2117w:
    shr word[vramaddr],1
    mov [vramaddr+1],al
    shl word[vramaddr],1
    mov byte[vramread],0

    movzx ebx,word[vramaddr]
    add ebx,[vram]
    mov bl,[ebx]
    mov [vramread],bl
    movzx ebx,word[vramaddr]
    add ebx,[vram]
    mov bl,[ebx+1]
    mov [vramread2],bl
    ret

; Video port data (Low)
NEWSYM reg2118
    mov ebx,[vramaddr]
    mov [vrama+ebx],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
    ret

NEWSYM reg2118inc
    mov ebx,[vramaddr]
    mov [vrama+ebx],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
    mov ebx,[addrincr]
    add [vramaddr],bx
    ret

NEWSYM reg2118inc8
    push ecx
    mov ebx,[vramaddr]
    movzx ecx,byte[vramincby8left]
    and ebx,ecx
    shl ebx,3
    push eax
    mov eax,[vramaddr]
    and ax,[vramincby8var]
    mov cl,[vramincby8totl]
    shr eax,cl
    add ebx,eax
    mov eax,[vramaddr]
    and ax,[vramincby8ptri]
    add ebx,eax
    pop eax
    pop ecx
    add ebx,[vram]
;    cmp [ebx],al
;    je .nochange2
    mov [ebx],al
    sub ebx,[vram]
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange2
    ret

NEWSYM reg2118inc8inc
    push ecx
    mov ebx,[vramaddr]
    movzx ecx,byte[vramincby8left]
    and ebx,ecx
    shl ebx,3
    push eax
    mov eax,[vramaddr]
    and ax,[vramincby8var]
    mov cl,[vramincby8totl]
    shr eax,cl
    add ebx,eax
    mov eax,[vramaddr]
    and ax,[vramincby8ptri]
    add ebx,eax
    pop eax
    pop ecx
    add ebx,[vram]
;    cmp [ebx],al
;    je .nochange2
    mov [ebx],al
    sub ebx,[vram]
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange2
    mov ebx,[addrincr]
    add [vramaddr],bx
    ret

NEWSYM reg2119
    cmp dword[vramaddr],0E000h
    jb .skip
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

NEWSYM reg2119inc
    mov ebx,[vramaddr]
;    cmp [vrama+ebx+1],al
;    je .nochange
    mov [vrama+ebx+1],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange
    mov ebx,[addrincr]
    add [vramaddr],bx
    ret

NEWSYM reg2119inc8
    push ecx
    mov ebx,[vramaddr]
    movzx ecx,byte[vramincby8left]
    and ebx,ecx
    shl ebx,3
    push eax
    mov eax,[vramaddr]
    and ax,[vramincby8var]
    mov cl,[vramincby8totl]
    shr eax,cl
    add ebx,eax
    mov eax,[vramaddr]
    and ax,[vramincby8ptri]
    add ebx,eax
    pop eax
    pop ecx
;    cmp [vrama+ebx+1],al
;    je .nochange2
    mov [vrama+ebx+1],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange2
    ret

NEWSYM reg2119inc8inc
    push ecx
    mov ebx,[vramaddr]
    movzx ecx,byte[vramincby8left]
    and ebx,ecx
    shl ebx,3
    push eax
    mov eax,[vramaddr]
    and ax,[vramincby8var]
    mov cl,[vramincby8totl]
    shr eax,cl
    add ebx,eax
    mov eax,[vramaddr]
    and ax,[vramincby8ptri]
    add ebx,eax
    pop eax
    pop ecx
;    cmp [vrama+ebx+1],al
;    je .nochange2
    mov [vrama+ebx+1],al
    shr ebx,4
    mov byte[vidmemch2+ebx],1
    mov byte[vidmemch4+ebx],1
    mov byte[vidmemch8+ebx],1
.nochange2
    mov ebx,[addrincr]
    add [vramaddr],bx
    ret

; MODE7 settings register
reg211Aw:
    mov [mode7set],al
    ret

SECTION .data
NEWSYM multchange, db 1
NEWSYM m7byte, db 0
SECTION .text

%macro Mode7Regs 1
    mov bl,[%1+1]
    ;mov bl,[m7byte]
    mov [%1],bl
    mov [%1+1],al
    ;mov [m7byte],al
%endmacro

; COS (COSINE) rotate angle / X Expansion
reg211Bw:
    Mode7Regs mode7A
    mov byte[multchange],1
    ret

; SIN (SIN)    rotate angle / X Expansion & Complement Multiplication Start
reg211Cw:
    Mode7Regs mode7B
    mov byte[multchange],1
    ret

; SIN (SIN)    rotate angle / Y Expansion
reg211Dw:
    Mode7Regs mode7C
    ret

; COS (COSINE) rotate angle / Y Expansion
reg211Ew:
    Mode7Regs mode7D
    ret

; Center position X (13-bit data only)
reg211Fw:
    Mode7Regs mode7X0
    ret

; Center position Y (13-bit data only)
reg2120w:
    Mode7Regs mode7Y0
    ret

; Colour # (or palette) selection register
reg2121w:
    xor bh,bh
    mov bl,al
    shl bx,1
    mov [cgaddr],bx
    and word[cgaddr],01FFh
    ret

; Colour data register
reg2122w:
    movzx ebx,word[cgaddr]
    cmp [cgram+ebx],al
    je .nomod
    mov [cgram+ebx],al
    mov byte[cgmod],1
.nomod
    inc word[cgaddr]
    and word[cgaddr],01FFh
    ret

; Window mask settings register [W12SEL]
reg2123w:
    mov bl,al
    and bl,0Fh
    test bl,0Ah
    jnz .nen1
;    or bl,02h
.nen1
    mov [winbg1en],bl
    mov bl,al
    shr bl,4
    test bl,0Ah
    jnz .nen2
;    or bl,02h
.nen2
    mov [winbg2en],bl
    ret

; Window mask settings register [W34SEL]
reg2124w:
    mov bl,al
    and bl,0Fh
    test bl,0Ah
    jnz .nen1
;    or bl,02h
.nen1
    mov [winbg3en],bl
    mov bl,al
    shr bl,4
    test bl,0Ah
    jnz .nen2
;    or bl,02h
.nen2
    mov [winbg4en],bl
    ret

; Window mask settings register [WOBJSEL]
reg2125w:
    mov bl,al
    and bl,0Fh
    mov [winobjen],bl
    mov bl,al
    shr bl,4
    mov [wincolen],bl
    ret

; Window 1 left position register
reg2126w:
    mov [winl1],al
    ret

; Window 1 right position register
reg2127w:
    mov [winr1],al
    ret

; Window 2 left position register
reg2128w:
    mov [winl2],al
    ret

; Window 2 right position register
reg2129w:
    mov [winr2],al
    ret

; Mask logic settings for Window 1 & 2 per screen
reg212Aw:
    mov [winlogica],al
    ret

; Mask logic settings for Colour Windows & OBJ Windows
reg212Bw:
    mov [winlogicb],al
    ret

; Main screen designation
reg212Cw:
    mov [scrnon],al
    ret

; Sub-screen designation
reg212Dw:
    mov [scrnon+1],al
    ret

; Window mask main screen designation register
reg212Ew:
    mov [winenabm],al
    ret

; Window mask sub screen designation register
reg212Fw:
    mov [winenabs],al
    ret

; Fixed color addition or screen addition register
reg2130w:
    mov [scaddset],al
    ret

; Addition/subtraction for screens, BGs, & OBJs
reg2131w:
    mov [scaddtype],al
    ret

; Fixed colour data for fixed colour +/-
reg2132w:
    mov bl,al
    and bl,1Fh
    test al,20h
    jz .nored
    mov [coladdr],bl
.nored
    test al,40h
    jz .nogreen
    mov [coladdg],bl
.nogreen
    test al,80h
    jz .noblue
    mov [coladdb],bl
.noblue
    ret

; Screen mode/video select register
reg2133w:
    mov [interlval],al
    and byte[interlval],43h
    test al,04h
    jnz .line239
    mov word[resolutn],224
    ret
.line239
    mov word[resolutn],239
    ret

; Sound Register #1
reg2140w:
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
reg2141w:
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
reg2142w:
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
reg2143w:
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

; Read/write WRAM register
reg2180w:
    mov ebx,[wramrwadr]
    add ebx,[wramdata]
    mov [ebx],al
    inc dword[wramrwadr]
    and dword[wramrwadr],01FFFFh
    ret

; WRAM data register (low byte)
reg2181w:
    mov [wramrwadr],al
    ret

; WRAM data register (middle byte)
reg2182w:
    mov [wramrwadr+1],al
    ret

; WRAM data register (high byte)
reg2183w:
    mov bl,al
    and bl,01h
    mov [wramrwadr+2],bl
    ret

; Joystick 1 & 2 status bytes
SECTION .bss
NEWSYM MultiTapStat, resb 1
SECTION .text

reg4016w:
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

; Counter enable
reg4200w:
    mov [INTEnab],al
    ret

; Programmable I/O port (out-port)
reg4201w:
    cmp byte[iohvlatch],1
    jne .noiohvlatch
    test al,80h
    jnz .noiohvlatch
    mov byte[iohvlatch],0
.noiohvlatch
    test byte[ioportval],80h
    jnz .nolatch
    test al,80h
    jz .nolatch
    mov byte[iohvlatch],1
.nolatch
    mov [ioportval],al
    mov bl,al
    and bl,80h
    and byte[MultiTapStat],07Fh
    or byte[MultiTapStat],bl
    ret

; Multiplicand 'A'
reg4202w:
    mov [multa],al
    ret

; Multiplier 'B'
reg4203w:
    push edx
    push eax
    xor ah,ah
    xor bh,bh
    mov bl,[multa]
    mul bx
    mov [multres],ax
    pop eax
    pop edx
    ret

; Dividend C (Low)
reg4204w:
    mov [diva],al
    ret

; Dividend C (High)
reg4205w:
    mov [diva+1],al
    ret

; Divisor B
reg4206w:
    cmp al,0
    je .divby0
    push eax
    push edx
    xor edx,edx
    movzx ebx,al
    mov ax,[diva]
    div bx
    mov [divres],ax
    mov [multres],dx
    pop edx
    pop eax
    ret
.divby0
    push eax
    mov word[divres],0FFFFh
    mov ax,[diva]
    mov [multres],ax
    pop eax
    ret


DetermineHIRQExec
    add dh,[HIRQCycNext]
    mov byte[HIRQCycNext],0
    mov byte[HIRQNextExe],0
    push eax
    push ecx
    push edx
    mov ax,[HIRQLoc]
    movzx ecx,byte[cycpl]
    mul cx
    mov cx,340
    div cx
    mov cl,[cycpl]
    sub cl,al
    pop edx
    cmp dh,cl
    ja .hirqokay
.notokay
    pop ecx
    pop eax
    ret
.hirqokay
    sub dh,cl
    add dh,30
    add cl,16
    mov [HIRQCycNext],cl
    mov byte[HIRQNextExe],1
    pop ecx
    pop eax
    ret

; Video horizontal IRQ beam position/pointer (Low)
reg4207w:
    cmp [HIRQLoc],al
    je .nohirqc
    mov [HIRQLoc],al
    mov bx,[curypos]
    cmp bx,[VIRQLoc]
    je near DetermineHIRQExec
.nohirqc
    ret

; Video horizontal IRQ beam position/pointer (High)
reg4208w:
    cmp [HIRQLoc+1],al
    je .nohirqc
    mov [HIRQLoc+1],al
    mov bx,[curypos]
    cmp bx,[VIRQLoc]
    je near DetermineHIRQExec
.nohirqc
    ret

; Video vertical IRQ beam position/pointer (Low)
reg4209w:
    mov [VIRQLoc],al
;    mov bx,[curypos]
    cmp byte[HIRQNextExe],1
    je .nohirq
    ret
.nohirq
    mov bx,[curypos]
    cmp bx,[VIRQLoc]
    je .nocancelhirq
    add dh,[HIRQCycNext]
    mov byte[HIRQCycNext],0
    mov byte[HIRQNextExe],0
.nocancelhirq
    ret

; Video vertical IRQ beam position/pointer (High)
reg420Aw:
    and al,01h
    mov [VIRQLoc+1],al
    mov bx,[totlines]
    sub bx,1
    cmp word[VIRQLoc],bx
    jb .okvirqpos
    mov word[VIRQLoc],07FFFh
.okvirqpos
    cmp byte[HIRQNextExe],1
    je .nohirq
    ret
.nohirq
    mov bx,[curypos]
    cmp bx,[VIRQLoc]
    je .nocancelhirq
    add dh,[HIRQCycNext]
    mov byte[HIRQCycNext],0
    mov byte[HIRQNextExe],0
.nocancelhirq
    ret

; Cycle speed register
reg420Dw:
    test al,01h
    jnz .speed358
    ; 2.68 Mhz
    mov al,[opexec268]
    mov [cycpl],al      ; 2.68 Mhz
    mov al,[opexec268cph]
    mov [cycphb],al     ; 2.68 Mhz
    and byte[xirqb],00h
    mov bl,[cycpb268]
    mov [cycpblt],bl  ; percentage of CPU/SPC to run
    ret
.speed358
    ; 3.58 Mhz
    mov al,[opexec358]
    mov [cycpl],al      ; 3.58 Mhz
    mov al,[opexec358cph]
    mov [cycphb],al     ; 3.58 Mhz
    or byte[xirqb],80h
    mov bl,[cycpb358]
    mov [cycpblt],bl  ; percentage of CPU/SPC to run
    ret

; DMA Control register
reg43X0w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    mov byte[hdmarestart],1
    ret

; DMA Destination register
reg43X1w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    mov byte[hdmarestart],1
    ret

; Source address (Low)
reg43x2w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
;    mov [dmadata+ebx+6],al
    ret

; Source address (High)
reg43x3w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
;    mov [dmadata+ebx+6],al
    ret

; Source bank address
reg43x4w
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; DMA transfer size & HDMA address register (Low)
reg43x5w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; DMA transfer size & HDMA address register (High)
reg43x6w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; DMA transfer size & HDMA address register (Bank)
reg43x7w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; Table Address of A-BUS by DMA < A2 Table Address (Low)
reg43x8w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; Table Address of A-BUS by DMA < A2 Table Address (High)
reg43x9w:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

; Number of lines for HDMA transfer
reg43XAw:
    mov byte[nohdmaframe],0
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    mov bx,[resolutn]
    cmp word[curypos],bx
    jb .nodma
    cmp al,0
    je .nodma
    mov byte[nohdmaframe],1
    inc byte[hdmadelay]
.nodma
    ret

; Unknown DMA byte
reg43XBw:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov [dmadata+ebx],al
    ret

regINVALIDw:     ; Invalid Register
    ret
