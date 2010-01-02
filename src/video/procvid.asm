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



%include "macros.mac"

EXTSYM BGMA,V8Mode,antienab,cacheud,cbitmode,ccud,cfield,cgram,coladdb,coladdg
EXTSYM coladdr,curblank,cvidmode,delay,extlatch,En2xSaI
EXTSYM gammalevel,hirestiledat,ignor512,latchx,latchy,maxbr
EXTSYM newengen,nextframe,objptr,pressed,prevpal,res512switch,resolutn
EXTSYM romispal,scaddtype,scanlines,selcA000,t1cc,vcache4b,vesa2_bpos
EXTSYM spritetablea,vesa2_clbit,vesa2_gpos,vesa2_rpos,vesa2red10
EXTSYM vidbuffer,vram,KeyStateSelct,soundon
EXTSYM bg1objptr,DecompAPtr,HalfTransB,HalfTransC
EXTSYM DrawScreen,MMXSupport,SwapMouseButtons
EXTSYM Get_MouseData,Get_MousePositionDisplacement,GUIEnableTransp,GUIFontData
EXTSYM StopSound,StartSound,PrevPicture,nggposng,current_zst,newest_zst
EXTSYM bg3ptr,bg3scroly,bg3scrolx,C4Ram
EXTSYM genfulladdtab,TimerEnable,ShowTimer,debugdisble,GUIOn
EXTSYM FilteredGUI,HalfTrans,SmallMsgText,mosenng,mosszng
EXTSYM intrlng,mode7hr,newgfx16b,vesa2_clbitng,vesa2_clbitng2,CSStatus
EXTSYM CSStatus2,CSStatus3,CSStatus4,SpecialLine,Clear2xSaIBuffer,vidbufferofsb,bg1scroly
EXTSYM MovieProcessing,MovieFrameStr,GetMovieFrameStr,mouse1lh,mouse2lh
EXTSYM MovieDisplayFrame,SloMo,MouseCount,device2,LoadPicture
EXTSYM zst_determine_newest,zst_exists,SSAutoFire
EXTSYM outputhex,outputhex16,outputchar,outputchar16b
EXTSYM OutputGraphicString,OutputGraphicString16b
EXTSYM OutputGraphicString5x5,OutputGraphicString16b5x5,showfps,ClockOutput

%ifndef __MSDOS__
EXTSYM MouseMoveX,MouseMoveY,MouseButtons,MultiMouseProcess,mouse
%else
EXTSYM SB_blank,vsyncon,Triplebufen,granadd,Palette0,smallscreenon,ScreenScale,vesa2selec
EXTSYM displayfpspal,superscopepal,saveselectpal,dosmakepal,doschangepal
%endif

%ifdef __UNIXSDL__
EXTSYM numlockptr
%endif

%ifdef __MSDOS__
%include "video/copyvid.inc"
%endif

SECTION .data
NEWSYM ssautosw,     db 20h

NEWSYM mousexloc,    dw 128
NEWSYM mouseyloc,    dw 112

SECTION .bss
NEWSYM mousebuttons, resw 1
NEWSYM mousexpos,    resw 1
NEWSYM mousexdir,    resb 1
NEWSYM mouseypos,    resw 1
NEWSYM mouseydir,    resb 1
NEWSYM mousechan,    resb 1

SECTION .data
NEWSYM ASCII2Font
  db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
  db 00h,00h,00h,00h,00h,00h,00h,00h,00h,30h,00h,00h,00h,00h,00h,00h
  db 00h,3Eh,33h,31h,3Fh,37h,2Fh,3Dh,3Ah,3Bh,35h,38h,39h,25h,28h,29h
  db 01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,2Eh,40h,2Ah,32h,2Bh,36h
  db 3Ch,0Bh,0Ch,0Dh,0Eh,0Fh,10h,11h,12h,13h,14h,15h,16h,17h,18h,19h
  db 1Ah,1Bh,1Ch,1Dh,1Eh,1Fh,20h,21h,22h,23h,24h,2Ch,34h,2Dh,42h,26h
  db 41h,0Bh,0Ch,0Dh,0Eh,0Fh,10h,11h,12h,13h,14h,15h,16h,17h,18h,19h
  db 1Ah,1Bh,1Ch,1Dh,1Eh,1Fh,20h,21h,22h,23h,24h,43h,00h,44h,27h,00h
  db 0Dh,1Fh,0Fh,0Bh,0Bh,0Bh,0Bh,0Dh,0Fh,0Fh,0Fh,13h,13h,13h,0Bh,0Bh
  db 0Fh,0Bh,0Bh,19h,19h,19h,1Fh,1Fh,23h,19h,1Fh,0Dh,10h,23h,1Ah,10h
  db 0Bh,4Eh,4Fh,50h,51h,52h,53h,54h,55h,56h,57h,58h,59h,5Ah,5Bh,5Ch
  db 5Dh,5Eh,5Fh,60h,61h,62h,63h,64h,65h,66h,67h,68h,69h,6Ah,6Bh,6Ch
  db 6Dh,6Eh,6Fh,70h,71h,72h,73h,74h,75h,76h,77h,78h,79h,7Ah,7Bh,7Ch
  db 7Dh,7Eh,7Fh,80h,81h,82h,83h,84h,85h,86h,87h,88h,89h,8Ah,8Bh,8Ch
  db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
  db 00h,00h,00h,00h,00h,00h,00h,4Dh,4Ch,4Bh,4Ah,45h,46h,47h,48h,49h

NEWSYM FontData
; bitmap 8x8 font ; char, offset for ASCII2Font
         db 0,0,0,0,0,0,0,0                       ; ' ', 00
         db 01111100b,11000110b,11001110b,11010110b ; 0, 01
         db 11100110b,11000110b,01111100b,00000000b
         db 00011000b,00111000b,01111000b,00011000b ; 1, 02
         db 00011000b,00011000b,01111110b,00000000b
         db 01111100b,11000110b,00001100b,00011000b ; 2, 03
         db 00110000b,01100110b,11111110b,00000000b
         db 01111100b,11000110b,00000110b,00111100b ; 3, 04
         db 00000110b,11000110b,01111100b,00000000b
         db 00111100b,01101100b,11001100b,11111110b ; 4, 05
         db 00001100b,00001100b,00001100b,00000000b
         db 11111110b,11000000b,11000000b,11111100b ; 5, 06
         db 00000110b,11000110b,01111100b,00000000b
         db 00111100b,01100000b,11000000b,11111100b ; 6, 07
         db 11000110b,11000110b,01111100b,00000000b
         db 11111110b,11000110b,00000110b,00001100b ; 7, 08
         db 00011000b,00011000b,00011000b,00000000b
         db 01111100b,11000110b,11000110b,01111100b ; 8, 09
         db 11000110b,11000110b,01111100b,00000000b
         db 01111100b,11000110b,11000110b,01111110b ; 9, 0A
         db 00000110b,11000110b,01111100b,00000000b
         db 00111000b,01101100b,11000110b,11111110b ; A, 0B
         db 11000110b,11000110b,11000110b,00000000b
         db 11111100b,11000110b,11000110b,11111100b ; B, 0C
         db 11000110b,11000110b,11111100b,00000000b
         db 01111100b,11000110b,11000000b,11000000b ; C, 0D
         db 11000000b,11000110b,01111100b,00000000b
         db 11111100b,11000110b,11000110b,11000110b ; D, 0E
         db 11000110b,11000110b,11111100b,00000000b
         db 11111110b,11000000b,11000000b,11111000b ; E, 0F
         db 11000000b,11000000b,11111110b,00000000b
         db 11111110b,11000000b,11000000b,11111000b ; F, 10
         db 11000000b,11000000b,11000000b,00000000b
         db 01111100b,11000110b,11000000b,11000000b ; G, 11
         db 11001110b,11000110b,01111100b,00000000b
         db 11000110b,11000110b,11000110b,11111110b ; H, 12
         db 11000110b,11000110b,11000110b,00000000b
         db 00111100b,00011000b,00011000b,00011000b ; I, 13
         db 00011000b,00011000b,00111100b,00000000b
         db 00011110b,00001100b,00001100b,00001100b ; J, 14
         db 00001100b,11001100b,00111100b,00000000b
         db 11001100b,11011000b,11110000b,11100000b ; K, 15
         db 11110000b,11011000b,11001100b,00000000b
         db 11000000b,11000000b,11000000b,11000000b ; L, 16
         db 11000000b,11000000b,11111110b,00000000b
         db 11000110b,11101110b,11111110b,11010110b ; M, 17
         db 11000110b,11000110b,11000110b,00000000b
         db 11000110b,11100110b,11110110b,11011110b ; N, 18
         db 11001110b,11000110b,11000110b,00000000b
         db 01111100b,11000110b,11000110b,11000110b ; O, 19
         db 11000110b,11000110b,01111100b,00000000b
         db 11111100b,11000110b,11000110b,11111100b ; P, 1A
         db 11000000b,11000000b,11000000b,00000000b
         db 01111100b,11000110b,11000110b,11000110b ; Q, 1B
         db 11010110b,11001110b,01111110b,00000000b
         db 11111100b,11000110b,11000110b,11111100b ; R, 1C
         db 11001100b,11000110b,11000110b,00000000b
         db 01111100b,11000110b,11000000b,01111100b ; S, 1D
         db 00000110b,11000110b,01111100b,00000000b
         db 01111110b,00011000b,00011000b,00011000b ; T, 1E
         db 00011000b,00011000b,00011000b,00000000b
         db 11000110b,11000110b,11000110b,11000110b ; U, 1F
         db 11000110b,11000110b,01111100b,00000000b
         db 11000110b,11000110b,11000110b,11000110b ; V, 20
         db 01101100b,00111000b,00010000b,00000000b
         db 11000110b,11000110b,11000110b,11010110b ; W, 21
         db 11010110b,11111110b,01101100b,00000000b
         db 11000110b,01101100b,00111000b,00010000b ; X, 22
         db 00111000b,01101100b,11000110b,00000000b
         db 11001100b,11001100b,01111000b,00110000b ; Y, 23
         db 00110000b,00110000b,00110000b,00000000b
         db 11111100b,10001100b,00011000b,00110000b ; Z, 24
         db 01100000b,11000100b,11111100b,00000000b
         db 00000000b,00000000b,00000000b,11111110b ; -, 25
         db 00000000b,00000000b,00000000b,00000000b
         db 00000000b,00000000b,00000000b,00000000b ; _, 26
         db 00000000b,00000000b,11111110b,00000000b
         db 01110000b,11011100b,00000110b,00000000b ; ~, 27
         db 00000000b,00000000b,00000000b,00000000b
         db 00000000b,00000000b,00000000b,00000000b ; ., 28
         db 00000000b,00110000b,00110000b,00000000b
         db 00000010b,00000100b,00001000b,00010000b ; /, 29
         db 00100000b,01000000b,10000000b,00000000b
         db 00001100b,00011000b,00110000b,01100000b ; <, 2A
         db 00110000b,00011000b,00001100b,00000000b
         db 01100000b,00110000b,00011000b,00001100b ; >, 2B
         db 00011000b,00110000b,01100000b,00000000b
         db 00111000b,00100000b,00100000b,00100000b ; [, 2C
         db 00100000b,00100000b,00111000b,00000000b
         db 00111000b,00001000b,00001000b,00001000b ; ], 2D
         db 00001000b,00001000b,00111000b,00000000b
         db 00000000b,00011000b,00011000b,00000000b ; :, 2E
         db 00011000b,00011000b,00000000b,00000000b
         db 00011000b,00100100b,00011000b,00111010b ; &, 2F
         db 01000100b,01000110b,00111010b,00000000b
         ; Arrow, 30
         ; #, 31  (, 3A  {, 43
         ; =, 32  ), 3B  }, 44
         ; ", 33  @, 3C  Up,45
         ; \, 34  ', 3D  Dn,46
         ; *, 35  !, 3E  Lt,47
         ; ?, 36  $, 3F  Rt,48
         ; %, 37  ;, 40  Bk,49
         ; +, 38  `, 41  .5,4A
         ; ,, 39  ^, 42

SECTION .bss
NEWSYM csounddisable, resb 1
NEWSYM f3menuen, resb 1

NEWSYM ForceNonTransp, resb 1

SECTION .bss
NEWSYM tempco0, resw 1
NEWSYM prevbright, resb 1

SECTION .text

NEWSYM vidpaste
%ifdef __MSDOS__
    cmp byte[vsyncon],0
    je .novsync
    cmp byte[Triplebufen],0
    jne .novsync
    cmp byte[curblank],0h
    jne .novsync
    ccallv waitvsync
.novsync
    cmp byte[cbitmode],1
    je .nopal
    cmp byte[curblank],0
    jne .nopal
    ccallv doschangepal
.nopal
%endif
    cmp byte[FPSOn],0
    je .nofps
    cmp byte[curblank],0
    jne .nofps
    ccallv showfps
.nofps
    cmp byte[TimerEnable],0
    je .noclock
    cmp byte[ShowTimer],0
    je .noclock
    ccallv ClockOutput
.noclock
    cmp byte[device2],2
    je near .drawss
.returnfromdraw
    mov ax,[resolutn]
    cmp [prevresolutn],ax
    je .noclear
    mov [prevresolutn],ax
%ifdef __MSDOS__
    call DOSClearScreen
%endif
.noclear
    ccallv DrawScreen
    ret

.drawss
%ifdef __MSDOS__
    cmp byte[cbitmode],1
    je .noss8b
    call superscopepal
.noss8b
%endif
    xor eax,eax
    mov al,[mouseyloc]
    mov ebx,288
    mul ebx
    mov esi,[vidbuffer]
    mov edi,SScopeCursor
    xor ebx,ebx
    mov bl,[mousexloc]
    add ebx,6
%ifdef __MSDOS__
    cmp byte[cbitmode],1
    je near .ss16b
    mov cl,20
.ssloop
    mov ch,20
.ssloop2
    cmp byte[edi],0
    je .nodraw
    mov edx,eax
    add edx,ebx
    sub edx,288*10
    jb .nodraw
    mov byte[esi+edx],128+16
.nodraw
    inc ebx
    inc edi
    dec ch
    jnz .ssloop2
    sub ebx,20
    add eax,288
    dec cl
    jnz .ssloop
    jmp .returnfromdraw

.ss16b
%endif
    push ebx
    mov cl,[vesa2_rpos]
    mov bx,31
    shl bx,cl
    mov [.SSRedCo],bx
    pop ebx
    shl eax,1
    shl ebx,1
    mov cl,20
.ssloopb
    mov ch,20
.ssloopb2
    cmp byte[edi],0
    je .nodrawb
    mov edx,eax
    add edx,ebx
    sub edx,288*10*2
    jb .nodrawb
    push eax
    mov ax,[.SSRedCo]
    mov [esi+edx],ax
    pop eax
.nodrawb
    add ebx,2
    inc edi
    dec ch
    jnz .ssloopb2
    sub ebx,40
    add eax,288*2
    dec cl
    jnz .ssloopb
    jmp .returnfromdraw

SECTION .bss
.SSRedCo resw 1

SECTION .data
NEWSYM MsgCount,  dd 120

SECTION .bss
NEWSYM Msgptr,    resd 1
NEWSYM MessageOn, resd 1
NEWSYM FPSOn,     resb 1

SECTION .data
prevresolutn dd 224

NEWSYM SScopeCursor
db 0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,0,0,0,0,1,1,0,0,1,0,0,1,1,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0
db 0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0
db 0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0
db 0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0
db 0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
db 0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
db 0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
db 0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
db 0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0
db 0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0
db 0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0
db 0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0
db 0,0,0,0,0,1,1,0,0,1,0,0,1,1,0,0,0,0,0,0
db 0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
