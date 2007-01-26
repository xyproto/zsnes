;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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



EXTSYM SFXEnable,regptr,initsfxregsr,initSA1regs,SA1Enable,initSDD1regs
EXTSYM SPC7110Enable,initSPC7110regs,RTCEnable,RTCReset,curypos,cycpl
EXTSYM pdh,vram,romispal,reg1read,spcnumread,spcon,reg2read,reg3read
EXTSYM reg4read,JoyEOrig,JoyENow,device2,cycphb,joycontren,totlines

%ifndef NO_DEBUGGER
EXTSYM debuggeron
%endif

SECTION .text

NEWSYM initregr
    ; Fill register pointer with invalid register accesses
    mov edi,[regptr]
    add edi,8000h
    mov ecx,3000h
    mov eax,regINVALID
.loopa
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .loopa
    ; Set all valid register accesses
    setreg 2100h*4,reg2100r
    setreg 2134h*4,reg2134r
    setreg 2135h*4,reg2135r
    setreg 2136h*4,reg2136r
    setreg 2137h*4,reg2137r
    setreg 2138h*4,reg2138r
    setreg 2139h*4,reg2139r
    setreg 213Ah*4,reg213Ar
    setreg 213Bh*4,reg213Br
    setreg 213Ch*4,reg213Cr
    setreg 213Dh*4,reg213Dr
    setreg 213Eh*4,reg213Er
    setreg 213Fh*4,reg213Fr
    setreg 2140h*4,reg2140r
    setreg 2141h*4,reg2141r
    setreg 2142h*4,reg2142r
    setreg 2143h*4,reg2143r
    setreg 2144h*4,reg2140r
    setreg 2145h*4,reg2141r
    setreg 2146h*4,reg2142r
    setreg 2147h*4,reg2143r
    setreg 2148h*4,reg2140r
    setreg 2149h*4,reg2141r
    setreg 214Ah*4,reg2142r
    setreg 214Bh*4,reg2143r
    setreg 214Ch*4,reg2140r
    setreg 214Dh*4,reg2141r
    setreg 214Eh*4,reg2142r
    setreg 214Fh*4,reg2143r
    setreg 2150h*4,reg2140r
    setreg 2151h*4,reg2141r
    setreg 2152h*4,reg2142r
    setreg 2153h*4,reg2143r
    setreg 2154h*4,reg2140r
    setreg 2155h*4,reg2141r
    setreg 2156h*4,reg2142r
    setreg 2157h*4,reg2143r
    setreg 2158h*4,reg2140r
    setreg 2159h*4,reg2141r
    setreg 215Ah*4,reg2142r
    setreg 215Bh*4,reg2143r
    setreg 215Ch*4,reg2140r
    setreg 215Dh*4,reg2141r
    setreg 215Eh*4,reg2142r
    setreg 215Fh*4,reg2143r
    setreg 2160h*4,reg2140r
    setreg 2161h*4,reg2141r
    setreg 2162h*4,reg2142r
    setreg 2163h*4,reg2143r
    setreg 2164h*4,reg2140r
    setreg 2165h*4,reg2141r
    setreg 2166h*4,reg2142r
    setreg 2167h*4,reg2143r
    setreg 2168h*4,reg2140r
    setreg 2169h*4,reg2141r
    setreg 216Ah*4,reg2142r
    setreg 216Bh*4,reg2143r
    setreg 216Ch*4,reg2140r
    setreg 216Dh*4,reg2141r
    setreg 216Eh*4,reg2142r
    setreg 216Fh*4,reg2143r
    setreg 2170h*4,reg2140r
    setreg 2171h*4,reg2141r
    setreg 2172h*4,reg2142r
    setreg 2173h*4,reg2143r
    setreg 2174h*4,reg2140r
    setreg 2175h*4,reg2141r
    setreg 2176h*4,reg2142r
    setreg 2177h*4,reg2143r
    setreg 2178h*4,reg2140r
    setreg 2179h*4,reg2141r
    setreg 217Ah*4,reg2142r
    setreg 217Bh*4,reg2143r
    setreg 217Ch*4,reg2140r
    setreg 217Dh*4,reg2141r
    setreg 217Eh*4,reg2142r
    setreg 217Fh*4,reg2143r
    setreg 2180h*4,reg2180r
    setreg 21C2h*4,reg21C2r
    setreg 21C3h*4,reg21C3r

    setreg 4016h*4,reg4016r
    setreg 4017h*4,reg4017r

    setreg 4100h*4,reg4100r

    setreg 420Ah*4,reg420Ar
    setreg 420Bh*4,reg420Br
    setreg 420Ch*4,reg420Cr
    setreg 420Dh*4,reg420Dr
    setreg 420Eh*4,reg420Er
    setreg 420Fh*4,reg420Fr

    setreg 4210h*4,reg4210r
    setreg 4211h*4,reg4211r
    setreg 4212h*4,reg4212r
    setreg 4213h*4,reg4213r
    setreg 4214h*4,reg4214r
    setreg 4215h*4,reg4215r
    setreg 4216h*4,reg4216r
    setreg 4217h*4,reg4217r
    setreg 4218h*4,reg4218r
    setreg 4219h*4,reg4219r
    setreg 421Ah*4,reg421Ar
    setreg 421Bh*4,reg421Br
    setreg 421Ch*4,reg421Cr
    setreg 421Dh*4,reg421Dr
    setreg 421Eh*4,reg421Er
    setreg 421Fh*4,reg421Fr

    setreg 4300h*4,reg43XXr
    setreg 4301h*4,reg43XXr
    setreg 4302h*4,reg43XXr
    setreg 4303h*4,reg43XXr
    setreg 4304h*4,reg43XXr
    setreg 4305h*4,reg43XXr
    setreg 4306h*4,reg43XXr
    setreg 4307h*4,reg43XXr
    setreg 4308h*4,reg43XXr
    setreg 4309h*4,reg43XXr
    setreg 430Ah*4,reg43XXr
    setreg 430Bh*4,reg43XXr
    setreg 430Fh*4,reg43XXr
    setreg 4310h*4,reg43XXr
    setreg 4311h*4,reg43XXr
    setreg 4312h*4,reg43XXr
    setreg 4313h*4,reg43XXr
    setreg 4314h*4,reg43XXr
    setreg 4315h*4,reg43XXr
    setreg 4316h*4,reg43XXr
    setreg 4317h*4,reg43XXr
    setreg 4318h*4,reg43XXr
    setreg 4319h*4,reg43XXr
    setreg 431Ah*4,reg43XXr
    setreg 431Bh*4,reg43XXr
    setreg 431Fh*4,reg43XXr
    setreg 4320h*4,reg43XXr
    setreg 4321h*4,reg43XXr
    setreg 4322h*4,reg43XXr
    setreg 4323h*4,reg43XXr
    setreg 4324h*4,reg43XXr
    setreg 4325h*4,reg43XXr
    setreg 4326h*4,reg43XXr
    setreg 4327h*4,reg43XXr
    setreg 4328h*4,reg43XXr
    setreg 4329h*4,reg43XXr
    setreg 432Ah*4,reg43XXr
    setreg 432Bh*4,reg43XXr
    setreg 432Fh*4,reg43XXr
    setreg 4330h*4,reg43XXr
    setreg 4331h*4,reg43XXr
    setreg 4332h*4,reg43XXr
    setreg 4333h*4,reg43XXr
    setreg 4334h*4,reg43XXr
    setreg 4335h*4,reg43XXr
    setreg 4336h*4,reg43XXr
    setreg 4337h*4,reg43XXr
    setreg 4338h*4,reg43XXr
    setreg 4339h*4,reg43XXr
    setreg 433Ah*4,reg43XXr
    setreg 433Bh*4,reg43XXr
    setreg 433Fh*4,reg43XXr
    setreg 4340h*4,reg43XXr
    setreg 4341h*4,reg43XXr
    setreg 4342h*4,reg43XXr
    setreg 4343h*4,reg43XXr
    setreg 4344h*4,reg43XXr
    setreg 4345h*4,reg43XXr
    setreg 4346h*4,reg43XXr
    setreg 4347h*4,reg43XXr
    setreg 4348h*4,reg43XXr
    setreg 4349h*4,reg43XXr
    setreg 434Ah*4,reg43XXr
    setreg 434Bh*4,reg43XXr
    setreg 434Fh*4,reg43XXr
    setreg 4350h*4,reg43XXr
    setreg 4351h*4,reg43XXr
    setreg 4352h*4,reg43XXr
    setreg 4353h*4,reg43XXr
    setreg 4354h*4,reg43XXr
    setreg 4355h*4,reg43XXr
    setreg 4356h*4,reg43XXr
    setreg 4357h*4,reg43XXr
    setreg 4358h*4,reg43XXr
    setreg 4359h*4,reg43XXr
    setreg 435Ah*4,reg43XXr
    setreg 435Bh*4,reg43XXr
    setreg 435Fh*4,reg43XXr
    setreg 4360h*4,reg43XXr
    setreg 4361h*4,reg43XXr
    setreg 4362h*4,reg43XXr
    setreg 4363h*4,reg43XXr
    setreg 4364h*4,reg43XXr
    setreg 4365h*4,reg43XXr
    setreg 4366h*4,reg43XXr
    setreg 4367h*4,reg43XXr
    setreg 4368h*4,reg43XXr
    setreg 4369h*4,reg43XXr
    setreg 436Ah*4,reg43XXr
    setreg 436Bh*4,reg43XXr
    setreg 436Fh*4,reg43XXr
    setreg 4370h*4,reg43XXr
    setreg 4371h*4,reg43XXr
    setreg 4372h*4,reg43XXr
    setreg 4373h*4,reg43XXr
    setreg 4374h*4,reg43XXr
    setreg 4375h*4,reg43XXr
    setreg 4376h*4,reg43XXr
    setreg 4377h*4,reg43XXr
    setreg 4378h*4,reg43XXr
    setreg 4379h*4,reg43XXr
    setreg 437Ah*4,reg43XXr
    setreg 437Bh*4,reg43XXr
    setreg 437Fh*4,reg43XXr

    cmp byte[SFXEnable],0
    je .nosfx
    call initsfxregsr
.nosfx
    cmp byte[SA1Enable],0
    je .nosa1
    call initSA1regs
.nosa1
    cmp byte[SDD1Enable],0
    je .nosdd1
    call initSDD1regs
.nosdd1
    cmp byte[SPC7110Enable],0
    je .nospc7110
    call initSPC7110regs
.nospc7110
    cmp byte[RTCEnable],0
    je .noRTC
    call RTCReset
.noRTC
    ret

; global variables
SECTION .data
NEWSYM invreg,    dw 0
NEWSYM sndrot,    db 0             ; rotates to use A,X or Y for sound skip
NEWSYM sndrot2,   db 0             ; rotates a random value for sound skip
NEWSYM INTEnab,   db 0             ; enables NMI(7)/VIRQ(5)/HIRQ(4)/JOY(0)
NEWSYM NMIEnab,   db 1             ; controlled in e65816 loop.  Sets to 81h
NEWSYM VIRQLoc,   dw 0             ; VIRQ Y location
NEWSYM vidbright, db 0             ; screen brightness (0 .. 15)
NEWSYM previdbr,  db 0             ; previous screen brightness
NEWSYM forceblnk, db 80h           ; force blanking on/off ($80=on)
NEWSYM objptr,    dd 0             ; pointer to object data in VRAM
NEWSYM objptrn,   dd 0             ; pointer2 to object data in VRAM
NEWSYM objsize1,  db 1             ; 1 = 8dot, 4=16 dot, 16=32 dot, 64=64dot
NEWSYM objsize2,  db 4             ; large object size
NEWSYM objmovs1,  db 2             ; number of bytes to move/paragraph
NEWSYM objadds1,  dw 14            ; number of bytes to add/paragraph
NEWSYM objmovs2,  db 2             ; number of bytes to move/paragraph
NEWSYM objadds2,  dw 14            ; number of bytes to add/paragraph
NEWSYM oamaddrt,  dw 0             ; oam address
NEWSYM oamaddrs,  dw 0             ; oam address at beginning of vblank
NEWSYM objhipr,   db 0             ; highest priority object #
NEWSYM bgmode,    db 0             ; graphics mode ( 0 .. 7 )
NEWSYM bg3highst, db 0             ; is 1 if background 3 has the highest priority
NEWSYM bgtilesz,  db 0             ; 0 = 8x8, 1 = 16x16, bit 0=bg1,bit1=bg2,etc
NEWSYM mosaicon,  db 0             ; mosaic on, bit 0=bg1,bit1=bg2, etc
NEWSYM mosaicsz,  db 0             ; mosaic size in pixels
NEWSYM bg1ptr,    dw 0             ; pointer to background1
NEWSYM bg2ptr,    dw 0             ; pointer to background2
NEWSYM bg3ptr,    dw 0             ; pointer to background3
NEWSYM bg4ptr,    dw 0             ; pointer to background4
NEWSYM bg1ptrb,   dw 0             ; pointer to background1
NEWSYM bg2ptrb,   dw 0             ; pointer to background2
NEWSYM bg3ptrb,   dw 0             ; pointer to background3
NEWSYM bg4ptrb,   dw 0             ; pointer to background4
NEWSYM bg1ptrc,   dw 0             ; pointer to background1
NEWSYM bg2ptrc,   dw 0             ; pointer to background2
NEWSYM bg3ptrc,   dw 0             ; pointer to background3
NEWSYM bg4ptrc,   dw 0             ; pointer to background4
NEWSYM bg1ptrd,   dw 0             ; pointer to background1
NEWSYM bg2ptrd,   dw 0             ; pointer to background2
NEWSYM bg3ptrd,   dw 0             ; pointer to background3
NEWSYM bg4ptrd,   dw 0             ; pointer to background4
NEWSYM bg1scsize, db 0             ; bg #1 screen size (0=1x1,1=1x2,2=2x1,3=2x2)
NEWSYM bg2scsize, db 0             ; bg #2 screen size (0=1x1,1=1x2,2=2x1,3=2x2)
NEWSYM bg3scsize, db 0             ; bg #3 screen size (0=1x1,1=1x2,2=2x1,3=2x2)
NEWSYM bg4scsize, db 0             ; bg #4 screen size (0=1x1,1=1x2,2=2x1,3=2x2)
NEWSYM bg1objptr, dw 0             ; pointer to tiles in background1
NEWSYM bg2objptr, dw 0             ; pointer to tiles in background2
NEWSYM bg3objptr, dw 0             ; pointer to tiles in background3
NEWSYM bg4objptr, dw 0             ; pointer to tiles in background4
NEWSYM bg1scrolx, dw 0             ; background 1 x position
NEWSYM bg2scrolx, dw 0             ; background 2 x position
NEWSYM bg3scrolx, dw 0             ; background 3 x position
NEWSYM bg4scrolx, dw 0             ; background 4 x position
NEWSYM bg1sx,     dw 0             ; Temporary Variable for Debugging purposes
NEWSYM bg1scroly, dw 0             ; background 1 y position
NEWSYM bg2scroly, dw 0             ; background 2 y position
NEWSYM bg3scroly, dw 0             ; background 3 y position
NEWSYM bg4scroly, dw 0             ; background 4 y position
NEWSYM addrincr,  dw 2             ; vram increment (2,64,128,256)
NEWSYM vramincr,  db 0             ; 0 = inrement at 2118/2139, 1 = 2119,213A
NEWSYM vramread,  db 0             ; previous character for vram read
NEWSYM vramaddr,  dd 0             ; vram address
NEWSYM cgaddr,    dw 0             ; cg (palette) address
NEWSYM cgmod,     db 0             ; if cgram is modified or not
NEWSYM scrnon,    dw 0             ; main & sub screen on
NEWSYM scrndist,  db 0             ; which background is disabled
NEWSYM resolutn,  dw 224           ; screen resolution
NEWSYM multa,     db 0             ; multiplier A
NEWSYM diva,      dw 0             ; divisor C
NEWSYM divres,    dw 0             ; quotent of divc/divb
NEWSYM multres,   dw 0             ; result of multa * multb/remainder of divc/divb
NEWSYM latchx,    dw 0             ; latched x value
NEWSYM latchy,    dw 0             ; latched y value
NEWSYM latchxr,   db 0             ; low or high byte read for x value
NEWSYM latchyr,   db 0             ; low or high byte read for y value
NEWSYM frskipper, db 0             ; used to control frame skipping
NEWSYM winl1,     db 0             ; window 1 left position
NEWSYM winr1,     db 0             ; window 1 right position
NEWSYM winl2,     db 0             ; window 2 left position
NEWSYM winr2,     db 0             ; window 2 right position
NEWSYM winbg1en,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG1
NEWSYM winbg2en,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG2
NEWSYM winbg3en,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG3
NEWSYM winbg4en,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG4
NEWSYM winobjen,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on sprites
NEWSYM wincolen,  db 0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on backarea
NEWSYM winlogica, db 0             ; Window logic type for BG1 to 4
NEWSYM winlogicb, db 0             ; Window logic type for Sprites and Backarea
NEWSYM winenabm,  db 0             ; Window logic enable for main screen
NEWSYM winenabs,  db 0             ; Window logic enable for sub screen
NEWSYM mode7set,  db 0             ; mode 7 settings
NEWSYM mode7A,    dw 0             ; A value for Mode 7
NEWSYM mode7B,    dw 0             ; B value for Mode 7
NEWSYM mode7C,    dw 0             ; C value for Mode 7
NEWSYM mode7D,    dw 0             ; D value for Mode 7
NEWSYM mode7X0,   dw 0             ; Center X for Mode 7
NEWSYM mode7Y0,   dw 0             ; Center Y for Mode 7
NEWSYM JoyAPos,   db 0             ; Old-Style Joystick Read Position for Joy 1 & 3
NEWSYM JoyBPos,   db 0             ; Old-Style Joystick Read Position for Joy 2 & 4
NEWSYM compmult,  dd 0             ; Complement Multiplication for Mode 7
NEWSYM joyalt,    db 0             ; temporary joystick alternation

NEWSYM wramrwadr, dd 0             ; continuous read/write to wram address
NEWSYM dmadata, times 129 db 0FFh  ; dma data (written from ports 43xx)
NEWSYM irqon,     db 0             ; if IRQ has been called (80h) or not (0)
NEWSYM nexthdma,  db 0             ; HDMA data to execute once vblank ends
NEWSYM curhdma,   db 0             ; Currently executed hdma
NEWSYM hdmadata, times 8*19 db 0   ; 4 dword register addresses, # of bytes to
                           ; transfer/line, address increment (word)
NEWSYM hdmatype,  db 0             ; if first time executing hdma or not
NEWSYM coladdr,   db 0             ; red value of color to add
NEWSYM coladdg,   db 0             ; green value of color to add
NEWSYM coladdb,   db 0             ; blue value of color to add
NEWSYM colnull,   db 0             ; keep this 0 (when accessing colors by dword)
NEWSYM scaddset,  db 0             ; screen/fixed color addition settings
NEWSYM scaddtype, db 0             ; which screen to add/sub
NEWSYM Voice0Disabl2, db 1         ; Disable Voice 0
NEWSYM Voice1Disabl2, db 1         ; Disable Voice 1
NEWSYM Voice2Disabl2, db 1         ; Disable Voice 2
NEWSYM Voice3Disabl2, db 1         ; Disable Voice 3
NEWSYM Voice4Disabl2, db 1         ; Disable Voice 4
NEWSYM Voice5Disabl2, db 1         ; Disable Voice 5
NEWSYM Voice6Disabl2, db 1         ; Disable Voice 6
NEWSYM Voice7Disabl2, db 1         ; Disable Voice 7

NEWSYM oamram,  times 1024 db 0    ; OAMRAM (544 bytes)
NEWSYM cgram,   times 512 db 0     ; CGRAM
NEWSYM pcgram,  times 512 db 0     ; Previous CGRAM

NEWSYM vraminctype,  db 0

; New variables
NEWSYM vramincby8on,   db 0        ; if increment by 8 is on
NEWSYM vramincby8left, db 0        ; how many left
NEWSYM vramincby8totl, db 0        ; how many in total (32,64,128)
NEWSYM vramincby8rowl, db 0        ; how many left in that row (start at 8)
NEWSYM vramincby8ptri, dw 0        ; increment by how many when rowl = 0

NEWSYM nexthprior,     db 0
NEWSYM doirqnext,      db 0

NEWSYM vramincby8var,  dw 0
NEWSYM screstype,      db 0
NEWSYM extlatch,       db 0
NEWSYM cfield,         db 0
NEWSYM interlval,      db 0

NEWSYM HIRQLoc,   dw 0             ; HIRQ X location

NEWSYM KeyOnStA, db 0
NEWSYM KeyOnStB, db 0

NEWSYM SDD1BankA, db 0
NEWSYM SDD1BankB, db 1
NEWSYM SDD1BankC, db 2
NEWSYM SDD1BankD, db 3
NEWSYM vramread2, db 0             ; previous character for vram read
NEWSYM nosprincr, db 0
NEWSYM poamaddrs, dw 0
NEWSYM ioportval, db 255
NEWSYM iohvlatch, db 0
NEWSYM ppustatus, db 0
NEWSYM hdmastartsc, db 0
NEWSYM hdmarestart, db 0
NEWSYM hdmadelay, db 0
NEWSYM nohdmaframe, db 0           ; no hdma for current frame

NEWSYM tempdat, times 474 db 0     ; expandable area

num2writeppureg equ $-sndrot
; pharos equ hack *sigh*
NEWSYM PHnum2writeppureg, dd num2writeppureg

NEWSYM scrndis,   db 0             ; which background is disabled

ALIGN32
NEWSYM oamaddr,   dd 0             ; oam address

NEWSYM bg1ptrx,   dd 0             ; pointer to background1
NEWSYM bg2ptrx,   dd 0             ; pointer to background2
NEWSYM bg3ptrx,   dd 0             ; pointer to background3
NEWSYM bg4ptrx,   dd 0             ; pointer to background4

NEWSYM bg1ptry,   dd 0             ; pointer to background1
NEWSYM bg2ptry,   dd 0             ; pointer to background2
NEWSYM bg3ptry,   dd 0             ; pointer to background3
NEWSYM bg4ptry,   dd 0             ; pointer to background4

NEWSYM Voice0Disable, db 1         ; Disable Voice 0
NEWSYM Voice1Disable, db 1         ; Disable Voice 1
NEWSYM Voice2Disable, db 1         ; Disable Voice 2
NEWSYM Voice3Disable, db 1         ; Disable Voice 3
NEWSYM Voice4Disable, db 1         ; Disable Voice 4
NEWSYM Voice5Disable, db 1         ; Disable Voice 5
NEWSYM Voice6Disable, db 1         ; Disable Voice 6
NEWSYM Voice7Disable, db 1         ; Disable Voice 7

NEWSYM BG116x16t, db 0
NEWSYM BG216x16t, db 0
NEWSYM BG316x16t, db 0
NEWSYM BG416x16t, db 0
NEWSYM SPC700read, dd 0
NEWSYM SPC700write, dd 0

NEWSYM JoyCRead, db 0

NEWSYM nssdip1, db 0
NEWSYM nssdip2, db 0
NEWSYM nssdip3, db 0
NEWSYM nssdip4, db 0
NEWSYM nssdip5, db 0
NEWSYM nssdip6, db 0

SECTION .text

NEWSYM ClearRegs
    mov word[VIRQLoc],0
    mov word[bg1ptr],0
    mov word[bg2ptr],0
    mov word[bg3ptr],0
    mov word[bg4ptr],0
    mov word[bg1ptrb],0
    mov word[bg2ptrb],0
    mov word[bg3ptrb],0
    mov word[bg4ptrb],0
    mov word[bg1ptrc],0
    mov word[bg2ptrc],0
    mov word[bg3ptrc],0
    mov word[bg4ptrc],0
    mov word[bg1ptrd],0
    mov word[bg2ptrd],0
    mov word[bg3ptrd],0
    mov word[bg4ptrd],0
    mov word[bg1objptr],0
    mov word[bg2objptr],0
    mov word[bg3objptr],0
    mov word[bg4objptr],0
    mov byte[ioportval],0FFh
    mov byte[hdmastartsc],0
    mov byte[hdmarestart],0
    mov byte[nohdmaframe],0
    mov byte[hdmadelay],0
    ret

reg2100r:
    ;Should be Openbus
    mov al,[vidbright]
    or al,[forceblnk]
    ret

; Multiply Result Low
reg2134r:
    checkmultchange
    mov al,[compmult]
    ret

; Multiply Result Middle
reg2135r:
    checkmultchange
    mov al,[compmult+1]
    ret

; Multiply Result High
reg2136r:
    checkmultchange
    mov al,[compmult+2]
    ret

; Software latch for horizontal/vertical counter
reg2137r:
;    mov byte[debstop3],1
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je .debugger
%endif
    cmp byte[iohvlatch],1
    je .dolatch
    test byte[ioportval],80h
    jz .nolatch
.dolatch
    xor ebx,ebx
    mov bl,[cycpl]
    sub bl,dh
    add bx,bx
    mov [latchx],bx
    mov bx,[curypos]
    mov [latchy],bx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq
    cmp word[HIRQLoc],0F0h
    ja .incry
    cmp dh,30
    jae .noincly
.incry
    inc word[latchy]
.novhirq
.noincly
.nolatch
    xor al,al
    mov byte[extlatch],0
    ret
.debugger
debblah:
    cmp byte[iohvlatch],1
    je .dolatch2
    test byte[ioportval],80h
    jz near .nolatch2
.dolatch2
    xor ebx,ebx
    mov bl,[cycpl]
    sub bl,[pdh]
    add bx,bx
    mov [latchx],bx
    mov bx,[curypos]
    mov [latchy],bx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq
    cmp word[HIRQLoc],0F0h
;    ja .incry
    cmp word[latchx],80
;    jb .noincly
.incry
    inc word[latchy]
.novhirq
.noincly
    xor al,al
    mov byte[extlatch],0
    ret

    push eax
    push edx
    xor eax,eax
    xor ebx,ebx
    mov al,[cycpl]
    sub al,[pdh]
    mov bx,339
    mul bx
    xor ebx,ebx
    mov bl,[cycpl]
    div bx
    mov [latchx],ax
    pop edx
    pop eax
    mov bx,[curypos]
    mov [latchy],bx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq2
    cmp word[HIRQLoc],0F0h
    ja .incry2
    cmp word[latchx],80
    jb .noincly2
.incry2
    inc word[latchy]
.novhirq2
.noincly2
.nolatch2
    xor al,al
    mov byte[extlatch],0
    ret

; Read OAM Data (Low, High)
reg2138r:
    xor ebx,ebx
    mov bx,[oamaddr]
    add ebx,oamram
    mov al,[ebx]
    inc word[oamaddr]
    cmp word[oamaddr],543
    ja .wrapoam
    ret
.wrapoam
    mov word[oamaddr],0
    ret

; Read VRAM Data (Low)
reg2139r:
    mov al,[vramread]

    xor ebx,ebx
    mov bx,[vramaddr]
    add ebx,[vram]
    mov bl,[ebx]
    mov [vramread],bl

    cmp byte[vramincr],0
    je near .noincr
    mov bx,[addrincr]
    add [vramaddr],bx
    cmp byte[vramincby8on],1
    jne .noincr
    dec byte[vramincby8left]
    jnz .noincr

    add word[vramaddr],2
    mov bl,[vramincby8totl]
    mov [vramincby8left],bl
    dec byte[vramincby8rowl]
    jz .nextrow
    mov bx,[vramincby8ptri]
    sub [vramaddr],bx
    jmp .noincr
.nextrow
    mov byte[vramincby8rowl],8
    sub word[vramaddr],16
.noincr
    ret

; Read VRAM Data (High)
reg213Ar:
    mov al,[vramread2]

    xor ebx,ebx
    mov bx,[vramaddr]
    add ebx,[vram]
    mov bl,[ebx+1]
    mov [vramread2],bl

    cmp byte[vramincr],1
    je near .noincr
    mov bx,[addrincr]
    add [vramaddr],bx
    cmp byte[vramincby8on],1
    jne .noincr
    dec byte[vramincby8left]
    jnz .noincr

    add word[vramaddr],2
    mov bl,[vramincby8totl]
    mov [vramincby8left],bl
    dec byte[vramincby8rowl]
    jz .nextrow
    mov bx,[vramincby8ptri]
    sub [vramaddr],bx
    jmp .noincr
.nextrow
    mov byte[vramincby8rowl],8
    sub word[vramaddr],16
.noincr
    ret

; Read CGRAM Data
reg213Br:
    xor ebx,ebx
    mov bx,[cgaddr]
    mov al,[cgram+ebx]
    inc word[cgaddr]
    and word[cgaddr],01FFh
    ret

; H counter data by external or software latch
reg213Cr:
    cmp byte[latchxr],1
    je .highv
    mov al,[latchx]
    mov byte[latchxr],1
    ret
.highv
    mov al,[latchx]
    and al,0FEh
    or  al,byte[latchx+1]
    mov byte[latchxr],0
    ret

; V counter data by external or software latch
reg213Dr:
    cmp byte[latchyr],1
    je .highv
    mov al,[latchy]
    mov byte[latchyr],1
    ret
.highv
    mov al,[latchy]
    and al,0FEh
    or  al,byte[latchy+1]
    mov byte[latchyr],0
    ret

; PPU Status Flag & Version number (OBJ over flags)
reg213Er:
    mov al,01h
    ret

; PPU Status Flag & Version number (NTSC/PAL/EXTRN Latch flag)
reg213Fr:
;    mov byte[debstop3],1
    mov al,[romispal]
    shl al,4
    or al,[ppustatus]
    or al,[cfield]
    mov byte[latchxr],0
    mov byte[latchyr],0
    or al,[extlatch]
    ret

; Sound Reg #1
reg2140r:
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg1read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    cmp word[ebx],0FB10h
    jne .noret
    mov word[ebx],0EAEAh
.noret
    mov al,5
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    inc byte[sndrot2]
    cmp byte[sndrot2],3
    jne .a
    mov byte[sndrot2],0
.a
    xor al,al
    test byte[sndrot2],01h
    jz .n
    mov al,[xa]
.n
    ret

; Sound Reg #2
reg2141r:
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg2read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    xor byte[sndrot],01h
    mov al,[xa+1]
    test byte[sndrot],01h
    jz .n
    mov al,[xa]
.n
    ret

; Sound Reg #3
reg2142r:
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg3read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    mov al,[xa]
    test byte[sndrot],01h
    jz .n
    mov al,[xa+1]
.n
    ret

; Sound Reg #4
reg2143r:
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg4read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    mov al,[xa+1]
    ret

reg2144r:
    xor al,al
    ret

; WRAM Read
reg2180r:
    mov ebx,[wramrwadr]
    add ebx,[wramdata]
    mov al,[ebx]
    inc dword[wramrwadr]
    and dword[wramrwadr],01FFFFh
    ret

; Unknown register, used by test cart

reg21C2r:
reg21C3r:
    mov al,21h
    ret

; Joystick Data for controller 1 and 2

reg4016r:
    xor al,al
    test dword[JoyANow],80000000h
    jz .noal
    mov al,1
.noal
    rol dword[JoyANow],1
    ret

SECTION .bss
NEWSYM MultiTap, resb 1
SECTION .text

; Joystick Data for controller 2 and 4
reg4017r:
    mov al,28
    cmp byte[device2],0
    jne .nomultitap
    cmp byte[MultiTap],1
    je .multitap
.nomultitap
;    cmp byte[device2],1
;    je .mouse2
;    test byte[JoyBNow],0FFh
;    jnz .reset
;.mouse2
    test dword[JoyBNow],80000000h
    jz .noal
;.reset
    or al,1
.noal
    rol dword[JoyBNow],1
    ret
.multitap
    test byte[MultiTapStat],1
    jz .no1
    or al,3
    ret
.no1
    test byte[MultiTapStat],80h
    jz .contr45
    test dword[JoyBNow],80000000h
    jz .nojb
    or al,1
.nojb
    test dword[JoyCNow],80000000h
    jz .nojc
    or al,2
.nojc
    rol dword[JoyBNow],1
    rol dword[JoyCNow],1
    ret
.contr45
    test dword[JoyDNow],80000000h
    jz .nojd
    or al,1
.nojd
    test dword[JoyENow],80000000h
    jz .noje
    or al,2
.noje
    rol dword[JoyDNow],1
    rol dword[JoyENow],1
    ret

reg4100r:
   xor al,al
   cmp byte[nssdip1],1
   jne .nodip1
   mov al,1
.nodip1
   cmp byte[nssdip2],1
   jne .nodip2
   or al,02h
.nodip2
   cmp byte[nssdip3],1
   jne .nodip3
   or al,04h
.nodip3
   cmp byte[nssdip4],1
   jne .nodip4
   or al,08h
.nodip4
   cmp byte[nssdip5],1
   jne .nodip5
   or al,10h
.nodip5
   cmp byte[nssdip6],1
   jne .nodip6
   or al,20h
.nodip6
   ret

reg420Ar:
reg420Br:
reg420Cr:
reg420Dr:
reg420Er:
reg420Fr:
    ;Should be Openbus
    xor al,al
    ret

; NMI Check Register
reg4210r:
    mov al,[NMIEnab]
    cmp byte[curnmi],0
    jne .nmi
    mov byte[NMIEnab],01h
.nmi
    mov byte[curnmi],0
    ret

; Video IRQ Register
reg4211r:
    mov al,[irqon]
    mov byte[irqon],0
    cmp dh,[cycphb]
    jae .nohblank
    or al,40h
.nohblank
    ret

SECTION .bss
NEWSYM hblank, resb 1
SECTION .text

; H/V Blank Flag & Joystick Controller Enable Flag
; bit 7 = vblank, 0=out,1=in, bit 6 = hblank, 0=out,1=in, bit 0 = joystick on
reg4212r:
    xor al,al
    cmp byte[joycontren],25
    jb .nojoyc
.yesjoy
    mov al,1
.nojoyc
    inc byte[joycontren]
    cmp byte[joycontren],50
    jne .n
    mov byte[joycontren],0
.n
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je .debugger
%endif
    mov bx,[resolutn]
    cmp word[curypos],bx
    jne .notres
    cmp byte[nmistatus],2
    je .vblank
.notres
    inc bx
    cmp word[curypos],bx
    jb .novbl
    mov bx,[totlines]
    dec bx
    cmp word[curypos],bx
    jae .novbl
.vblank
    or al,80h
;    jmp .nohblank
.novbl
    mov byte[hblank],0
    cmp dh,[cycphb]
    jae .nohblank
    mov byte[hblank],1
    or al,40h
.nohblank
    test byte[INTEnab],01h
    jz .nojoy
    mov bx,[resolutn]
    add bx,2
    cmp word[curypos],bx
    jne .nojoy
    cmp dh,5
    ja .nojoy
    ret
.nojoy
    ret
.debugger
debblah2:
    push edx
    mov dh,[pdh]
    mov bx,[resolutn]
    cmp word[curypos],bx
    jne .notres
    cmp byte[nmistatus],2
    je .vblank
.notres
    inc bx
    cmp word[curypos],bx
    jb .novbl
    mov bx,[totlines]
    dec bx
    cmp word[curypos],bx
    jae .novbl
.vblank
    or al,80h
;    jmp .nohblank
.novbl
    cmp dh,[cycphb]
    jae .nohblank
    or al,40h
.nohblank
    test byte[INTEnab],01h
    jz .nojoy
    mov bx,[resolutn]
    add bx,2
    cmp word[curypos],bx
    jne .nojoy
    cmp dh,5
    ja .nojoy
    pop edx
    ret
.nojoy
    pop edx
    ret

    mov bx,[resolutn]
    inc bx
    cmp word[curypos],bx
    jb .novbl2
    or al,80h
    jmp .nohblank2
.novbl2
    mov bl,[pdh]
    cmp bl,[cycphb]
    jae .nohblank2
    or al,40h
.nohblank2
    test byte[INTEnab],01h
    jz .nojoy2
    mov bx,[resolutn]
    add bx,2
    cmp word[curypos],bx
    jne .nojoy2
    cmp byte[pdh],5
    ja .nojoy
;    or al,01h
    ret
.nojoy2
    ret

; Programmable I/O port
reg4213r:
    mov al,[ioportval]
    ret

; Quotent of Divide Result (Low)
reg4214r:
    mov al,[divres]
    ret

; Quotent of Divide Result (High)
reg4215r:
    mov al,[divres+1]
    ret

; Product of Multiplication Result or Remainder of Divide Result (Low)
reg4216r:
    mov al,[multres]
    ret

; Product of Multiplication Result or Remainder of Divide Result (High)
reg4217r:
    mov al,[multres+1]
    ret

; Joystick 1 Low
; bit7=X,bit6=Y,bit5=L,bit4=R
reg4218r:
    mov al,[JoyAOrig+2]
    ret

; Joystick 1 High
; bit7=A,bit6=B,bit5=Sel,bit4=Start,bit3=up,bit2=down,bit1=left,bit0=right
reg4219r:
    mov al,[JoyAOrig+3]
    ret

; Joystick 2 Low
reg421Ar:
    mov al,[JoyBOrig+2]
    ret

; Joystick 2 High
reg421Br:
    mov al,[JoyBOrig+3]
    ret

; Joystick 3 Low
reg421Cr:
    mov al,[JoyDOrig+2]
    ret

; Joystick 3 High
reg421Dr:
    mov al,[JoyDOrig+3]
    ret

; Joystick 4 Low
reg421Er:
    mov al,[JoyCOrig+2]
    ret

; Joystick 4 High
reg421Fr:
    mov al,[JoyCOrig+3]
    ret

; DMA Reader
reg43XXr:
    xor ebx,ebx
    mov bx,cx
    sub bx,4300h
    mov al,[dmadata+ebx]
    ret

regINVALID:     ; Invalid Register
    cmp cx,2100h
    jb .cleared
    mov al,ch
    cmp byte[SPC7110Enable],0
    je .nospc7110
.cleared
    xor al,al
.nospc7110
;    mov byte[debstop3],1
    ret
