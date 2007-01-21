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



%include "macros.mac"

EXTSYM BGMA,V8Mode,antienab,cacheud,cbitmode,ccud,cfield,cgram,coladdb,coladdg
EXTSYM coladdr,curblank,curfps,cvidmode,delay,extlatch,En2xSaI
EXTSYM gammalevel,hirestiledat,ignor512,latchx,latchy,maxbr,ForceNewGfxOff
EXTSYM newengen,nextframe,objptr,pressed,prevpal,res512switch,resolutn
EXTSYM romispal,scaddtype,scanlines,selcA000,t1cc,vcache4b,vesa2_bpos
EXTSYM spritetablea,vesa2_clbit,vesa2_gpos,vesa2_rpos,vesa2red10
EXTSYM vidbuffer,vram,KeyStateSelct,soundon
EXTSYM bg1objptr,DecompAPtr,HalfTransB,HalfTransC
EXTSYM DrawScreen,MMXSupport
EXTSYM Get_MouseData,Get_MousePositionDisplacement,GUIEnableTransp,GUIFontData
EXTSYM StopSound,StartSound,PrevPicture,nggposng,current_zst,newest_zst
EXTSYM GetTimeInSeconds,bg3ptr,bg3scroly,bg3scrolx,C4Ram
EXTSYM genfulladdtab,genfulladdtabng,TimerEnable,ShowTimer,debugdisble,GUIOn
EXTSYM FilteredGUI,HalfTrans,SmallMsgText,Mode7HiRes,mosenng,mosszng
EXTSYM intrlng,mode7hr,newgfx16b,vesa2_clbitng,vesa2_clbitng2,CSStatus
EXTSYM CSStatus2,CSStatus3,CSStatus4,SpecialLine,Clear2xSaIBuffer,vidbufferofsb,bg1scroly
EXTSYM MovieProcessing,MovieFrameStr,GetMovieFrameStr,mouse1lh,mouse2lh
EXTSYM MovieDisplayFrame,SloMo,MouseCount,device2,LoadPicture
EXTSYM zst_determine_newest,newestfiledate,zst_exists,ClockBox,SSAutoFire

%ifndef __MSDOS__
EXTSYM MouseMoveX,MouseMoveY,MouseButtons,MultiMouseProcess,mouse
%else
EXTSYM SB_blank,vsyncon,Triplebufen,granadd,Palette0,smallscreenon,ScreenScale,vesa2selec
EXTSYM displayfpspal,superscopepal,saveselectpal
%endif

%ifdef __UNIXSDL__
EXTSYM numlockptr
%endif

%ifdef __MSDOS__
%include "video/2xsaimmx.inc"
%endif
%include "video/copyvid.inc"

SECTION .text

;*******************************************************
; ShowVideo                   Processes & displays video
;*******************************************************

NEWSYM showvideo
    push esi
    push edi
    push edx
    push ebx
    push ebp
    inc byte[ccud]
    mov bl,[ccud]
    cmp byte[cacheud],bl
    je .noinc
    mov byte[ccud],0
.noinc
    call copyvid
    mov eax,[KeyStateSelct]
    test byte[pressed+eax],1
    jz .nosavesel
    call saveselect
.nosavesel
    xor ecx,ecx
    pop ebp
    pop ebx
    pop edx
    pop edi
    pop esi
    ret

NEWSYM SwapMouseButtons
    test bl,3
    jpe .noswap
    xor bl,3
.noswap
    ret

NEWSYM processmouse1
    push esi
    push edi
    push edx
    push ebx
%ifndef __MSDOS__
    cmp byte[MouseCount],1
    jle .nomultimouse
    pushad
    mov byte[mouse],0
    call MultiMouseProcess
    popad
    mov bx,[MouseButtons]
    cmp byte[mouse1lh],1
    jne .notlefthanded1
    call SwapMouseButtons
.notlefthanded1
    mov [mousebuttons],bx
    mov cx,[MouseMoveX]
    mov dx,[MouseMoveY]
    jmp .mousestuff
.nomultimouse
%endif
    call Get_MouseData
    cmp byte[mouse1lh],1
    jne .notlefthanded2
    call SwapMouseButtons
.notlefthanded2
    mov [mousebuttons],bx
    call Get_MousePositionDisplacement
.mousestuff
    mov word[mousexpos],0
    cmp cx,0
    je .noxchange
    mov byte[mousexdir],0
    cmp cx,0
    jge .noneg
    mov byte[mousexdir],1
    neg cx
.noneg
    mov [mousexpos],cx
.noxchange
    mov word[mouseypos],0
    cmp dx,0
    je .noychange
    mov byte[mouseydir],0
    cmp dx,0
    jge .noneg2
    mov byte[mouseydir],1
    neg dx
.noneg2
    mov [mouseypos],dx
.noychange
    xor ecx,ecx
    pop ebx
    pop edx
    pop edi
    pop esi
    ret

NEWSYM processmouse2
    push esi
    push edi
    push edx
    push ebx
%ifndef __MSDOS__
    cmp byte[MouseCount],1
    jle .nomultimouse
    pushad
    mov byte[mouse],1
    call MultiMouseProcess
    popad
    mov bx,[MouseButtons+2]
    jmp .mousestuff
.nomultimouse
%endif
    call Get_MouseData
.mousestuff
    cmp byte[mouse2lh],1
    jne .notlefthanded
    call SwapMouseButtons
.notlefthanded
    mov [mousebuttons],bx
    cmp byte[device2],2
    jne .ss
    push eax
    mov eax,[SSAutoFire]
    cmp byte[pressed+eax],0
    pop eax
    je .noautosw
    cmp byte[ssautoswb],1
    je .ss
    xor byte[ssautosw],20h
    mov byte[ssautoswb],1
    mov dword[Msgptr],.ssautoen
    cmp byte[ssautosw],0
    jne .nononauto
    mov dword[Msgptr],.ssautodi
.nononauto
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp .ss
.noautosw
    mov byte[ssautoswb],0
.ss
%ifndef __MSDOS__
    cmp byte[MouseCount],1
    jle .nomultimouse2
    mov cx,[MouseMoveX+2]
    mov dx,[MouseMoveY+2]
    jmp .mousestuff2
.nomultimouse2
%endif
    call Get_MousePositionDisplacement
.mousestuff2
    cmp byte[device2],3
    je .le
    cmp byte[device2],4
    je .le
    cmp byte[device2],2
    jne .ss2
.le
    add word[mousexloc],cx
    test word[mousexloc],8000h
    jz .nowrapleft
    mov word[mousexloc],0
.nowrapleft
    cmp word[mousexloc],255
    jbe .nowrapright
    mov word[mousexloc],255
.nowrapright
    mov ax,[mousexloc]
    add ax,40
    mov [latchx],ax
    mov byte[extlatch],40h
.ss2
    mov word[mousexpos],0
    cmp cx,0
    je .noxchange
    mov byte[mousexdir],0
    cmp cx,0
    jge .noneg
    mov byte[mousexdir],1
    neg cx
.noneg
    mov [mousexpos],cx
.noxchange
    cmp byte[device2],3
    je .le2
    cmp byte[device2],4
    je .le2
    cmp byte[device2],2
    jne .ss3
.le2
    add word[mouseyloc],dx
    test word[mouseyloc],8000h
    jz .nowrapup
    mov word[mouseyloc],0
.nowrapup
    cmp word[mouseyloc],223
    jbe .nowrapdown
    mov word[mouseyloc],223
.nowrapdown
    mov ax,[mouseyloc]
    mov [latchy],ax
.ss3
    mov word[mouseypos],0
    cmp dx,0
    je .noychange
    mov byte[mouseydir],0
    cmp dx,0
    jge .noneg2
    mov byte[mouseydir],1
    neg dx
.noneg2
    mov [mouseypos],dx
.noychange
    xor ecx,ecx
    pop ebx
    pop edx
    pop edi
    pop esi
    ret

SECTION .data
.ssautoen db 'AUTOFIRE ENABLED.',0
.ssautodi db 'AUTOFIRE DISABLED.',0

NEWSYM ssautosw,     db 20h

NEWSYM mousexloc,    dw 128
NEWSYM mouseyloc,    dw 112

SECTION .bss
NEWSYM ssautoswb,    resb 1
NEWSYM mousebuttons, resw 1
NEWSYM mousexpos,    resw 1
NEWSYM mousexdir,    resb 1
NEWSYM mouseypos,    resw 1
NEWSYM mouseydir,    resb 1
NEWSYM mousechan,    resb 1
SECTION .text

;*******************************************************
; Output Hex                 Outputs the hex in al @ esi
;*******************************************************

NEWSYM outputhex
    push edi
    push esi
    push eax
    push ebx
    push ecx
    push esi
    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    shr bl,4
    shl ebx,3
    add edi,ebx
    add edi,8
    mov cl,8
.loopa
    mov ah,[edi]
    mov ch,8
.loopb
    test ah,80h
    jz .nowrite
    mov byte[esi],128
    mov byte[esi+289],192
.nowrite
    shl ah,1
    inc esi
    dec ch
    jnz .loopb
    add esi,280
    inc edi
    dec cl
    jnz .loopa
    pop esi
    add esi,8
    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    and bl,0Fh
    shl ebx,3
    add edi,ebx
    add edi,8
    mov cl,8
.loopa2
    mov ah,[edi]
    mov ch,8
.loopb2
    test ah,80h
    jz .nowrite2
    mov byte[esi],128
    mov byte[esi+289],192
.nowrite2
    shl ah,1
    inc esi
    dec ch
    jnz .loopb2
    add esi,280
    inc edi
    dec cl
    jnz .loopa2
    pop ecx
    pop ebx
    pop eax
    pop esi
    pop edi
    ret

OutputText16b:
    cmp byte[ForceNonTransp],1
    je near OutText16bnt
    cmp byte[GUIEnableTransp],0
    je near OutText16bnt
    ; output text in edi to esi
    push ebx
    push eax
    mov cl,9
.loopa
    mov ch,9
    xor eax,eax
    cmp cl,1
    je .not1
    mov al,[edi]
    shl eax,1
.not1
    xor ebx,ebx
    cmp cl,9
    je .loopb
    mov bl,[edi-1]
.loopb
    test ax,100h
    jz .nowrite
    push eax
    and word[esi],dx
    shr word[esi],1
    and word[esi+75036*4],dx
    shr word[esi+75036*4],1
    ror edx,16
    add word[esi],dx
    add word[esi+75036*4],dx
    ror edx,16
    pop eax
    jmp .nowrite2
.nowrite
    test bx,100h
    jz .nowrite2
    and word[esi],dx
    shr word[esi],1
    and word[esi+75036*4],dx
    shr word[esi+75036*4],1
.nowrite2
    shl ax,1
    shl bx,1
    add esi,2
    dec ch
    jnz .loopb
    add esi,279*2
    inc edi
    dec cl
    jnz .loopa
    pop eax
    pop ebx
    ret

OutText16bnt:
    ; output text in edi to esi
    push ebx
    push eax
    mov cl,9
.loopa
    mov ch,9
    xor eax,eax
    cmp cl,1
    je .not1
    mov al,[edi]
    shl eax,1
.not1
    xor ebx,ebx
    cmp cl,9
    je .loopb
    mov bl,[edi-1]
.loopb
    test ax,100h
    jz .nowrite
    mov word[esi],0FFFFh
    mov word[esi+75036*4],0FFFFh
    jmp .nowrite2
.nowrite
    test bx,100h
    jz .nowrite2
    and word[esi],dx
    and word[esi+75036*4],dx
    shr word[esi],1
    shr word[esi+75036*4],1
.nowrite2
    shl ax,1
    shl bx,1
    add esi,2
    dec ch
    jnz .loopb
    add esi,279*2
    inc edi
    dec cl
    jnz .loopa
    pop eax
    pop ebx
    ret

NEWSYM outputhex16
    push edi
    push esi
    push eax
    push ebx
    push ecx
    push edx
    push esi
    mov dx,[vesa2_clbitng]
    ror edx,16
    mov dx,[vesa2_clbitng]
    shr dx,1
    ror edx,16

    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    shr bl,4
    shl ebx,3
    add edi,ebx
    add edi,8
    call OutputText16b
    pop esi
    add esi,16
    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    and bl,0Fh
    shl ebx,3
    add edi,ebx
    add edi,8
    call OutputText16b
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop esi
    pop edi
    ret

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

SECTION .text

;*******************************************************
; Output Char                   Outputs char in al @ esi
;*******************************************************

NEWSYM outputchar
    push edi
    push esi
    push eax
    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    shl ebx,3
    add edi,ebx
    mov cl,8
.loopa
    mov ah,[edi]
    mov ch,8
.loopb
    test ah,80h
    jz .nowrite
    mov al,[textcolor]
    mov [esi],al
    mov byte[esi+289],192
.nowrite
    shl ah,1
    inc esi
    dec ch
    jnz .loopb
    add esi,280
    inc edi
    dec cl
    jnz .loopa
    pop eax
    pop esi
    pop edi
    ret

NEWSYM outputchar16b
    push edi
    push esi
    push eax
    push edx
    push ecx
    mov dx,[vesa2_clbitng]
    ror edx,16
    mov dx,[vesa2_clbitng]
    shr dx,1
    ror edx,16

    mov edi,FontData
    xor ebx,ebx
    mov bl,al
    shl ebx,3
    add edi,ebx
    call OutputText16b
    pop ecx
    pop edx
    pop eax
    pop esi
    pop edi
    ret

SECTION .data
NEWSYM textcolor, db 128
NEWSYM textcolor16b, dw 0FFFFh
SECTION .text

NEWSYM outputchar5x5
    push edi
    push esi
    push eax
    mov edi,GUIFontData
    xor ebx,ebx
    mov bl,al
    shl ebx,2
    add edi,ebx
    xor ebx,ebx
    mov bl,al
    add edi,ebx
    mov cl,5
.loopa
    mov ah,[edi]
    mov ch,5
.loopb
    test ah,80h
    jz .nowrite
    mov al,[textcolor]
    mov [esi],al
.nowrite
    shl ah,1
    inc esi
    dec ch
    jnz .loopb
    add esi,283
    inc edi
    dec cl
    jnz .loopa
    pop eax
    pop esi
    pop edi
    ret

NEWSYM outputchar16b5x5
    push edi
    push esi
    push eax
    mov edi,GUIFontData
    xor ebx,ebx
    mov bl,al
    shl ebx,2
    add edi,ebx
    xor ebx,ebx
    mov bl,al
    add edi,ebx
    mov cl,5
.loopa
    mov ah,[edi]
    mov ch,5
.loopb
    test ah,80h
    jz .nowrite
    push eax
    mov ax,[textcolor16b]
    mov [esi],ax
    mov [esi+75036*4],ax
    pop eax
.nowrite
    shl ah,1
    add esi,2
    dec ch
    jnz .loopb
    add esi,283*2
    inc edi
    dec cl
    jnz .loopa
    pop eax
    pop esi
    pop edi
    ret

;*******************************************************
; Output Graphic String   Outputs String from edi to esi
;*******************************************************

NEWSYM OutputGraphicString
    cmp byte[cbitmode],1
    je .do16b
.no16bit
    xor eax,eax
.nextstr
    mov al,[edi]
    cmp al,0
    je .nomore
    mov al,[ASCII2Font+eax]
    call outputchar
    add esi,8
    inc edi
    jmp .nextstr
.nomore
    ret

.do16b
    sub esi,[vidbuffer]
    shl esi,1
    add esi,[vidbuffer]
    cmp byte[textcolor],128
    jne .no128
    mov word[textcolor16b],0FFFFh
.no128
    cmp byte[textcolor],129
    jne .no129
    mov word[textcolor16b],0
.no129
    cmp byte[textcolor],130
    jne .no130
    xor ax,ax
    xor bx,bx
    mov cl,[vesa2_rpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_gpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_bpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov [textcolor16b],ax
.no130
    ; Color #131, Red
    cmp byte[textcolor],131
    jne .no131
    xor ax,ax
    xor bx,bx
    mov cl,[vesa2_rpos]
    mov bx,22
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_gpos]
    mov bx,5
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_bpos]
    mov bx,5
    shl bx,cl
    add ax,bx
    mov [textcolor16b],ax
.no131
    jmp OutputGraphicString16b

NEWSYM OutputGraphicString16b
    xor eax,eax
.nextstr
    mov al,[edi]
    cmp al,0
    je .nomore
    mov al,[ASCII2Font+eax]
    call outputchar16b
    add esi,16
    inc edi
    jmp .nextstr
.nomore
    ret

NEWSYM OutputGraphicString5x5
    cmp byte[cbitmode],1
    je .do16b
.no16bit
    xor eax,eax
.nextstr
    mov al,[edi]
    cmp al,0
    je .nomore
    mov al,[ASCII2Font+eax]
    call outputchar5x5
    add esi,6
    inc edi
    jmp .nextstr
.nomore
    ret

.do16b
    sub esi,[vidbuffer]
    shl esi,1
    add esi,[vidbuffer]
    cmp byte[textcolor],128
    jne .no128
    mov word[textcolor16b],0FFFFh
.no128
    cmp byte[textcolor],129
    jne .no129
    mov word[textcolor16b],0
.no129
    cmp byte[textcolor],130
    jne .no130
    xor ax,ax
    xor bx,bx
    mov cl,[vesa2_rpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_gpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_bpos]
    mov bx,20
    shl bx,cl
    add ax,bx
    mov [textcolor16b],ax
.no130
    ; Color #131, Red
    cmp byte[textcolor],131
    jne .no131
    xor ax,ax
    xor bx,bx
    mov cl,[vesa2_rpos]
    mov bx,22
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_gpos]
    mov bx,5
    shl bx,cl
    add ax,bx
    mov cl,[vesa2_bpos]
    mov bx,5
    shl bx,cl
    add ax,bx
    mov [textcolor16b],ax
.no131
    jmp OutputGraphicString16b5x5

NEWSYM OutputGraphicString16b5x5
    xor eax,eax
.nextstr
    mov al,[edi]
    cmp al,0
    je .nomore
    mov al,[ASCII2Font+eax]
    call outputchar16b5x5
    add esi,12
    inc edi
    jmp .nextstr
.nomore
    ret

;*******************************************************
; Save Select      Allows user to select save state slot
;*******************************************************
; For Save State

SECTION .bss
NEWSYM csounddisable, resb 1
f3menuen resb 1
PrevPictureVal resb 1
CurPictureVal resb 1
SECTION .text

NEWSYM drawhline
.loop
    mov [esi],al
    inc esi
    dec ecx
    jnz .loop
    ret

NEWSYM drawhline16b
.loop
    mov [esi],ax
    add esi,2
    dec ecx
    jnz .loop
    ret

NEWSYM drawvline
.loop
    mov [esi],al
    add esi,288
    dec ecx
    jnz .loop
    ret

NEWSYM drawvline16b
.loop
    mov [esi],ax
    add esi,288*2
    dec ecx
    jnz .loop
    ret

DetermineNewest:
    pushad
    mov dword[newestfiledate],0
    mov eax,[current_zst]
    push eax
    mov bl,10
    div bl
    mul bl
    movzx ecx,al
    add al,bl
.again
    mov [current_zst],ecx
    pushad
    call zst_determine_newest
    popad
    inc cl
    cmp cl,al
    jne .again
    pop eax
    mov [current_zst],eax
    popad
    ret

GetPicture:
    mov cl,[CurPictureVal]
    cmp [PrevPictureVal],cl
    jne .notskip
    ret
.notskip
    mov [PrevPictureVal],cl

    pushad
    call LoadPicture
    popad

    ; convert to 1:5:5:5
    cmp byte[newengen],0
    je .noneweng
    cmp byte[nggposng],5
    jne .noneweng
    mov edx,PrevPicture
    mov ecx,64*56
.loop2
    mov ax,[edx]
    mov bx,ax
    and ax,1111111111000000b
    and bx,0000000000011111b
    shr ax,1
    or bx,ax
    mov [edx],bx
    add edx,2
    dec ecx
    jnz .loop2
.noneweng
    ; draw border
    mov esi,75*2+9*288*2
    add esi,[vidbuffer]
    mov edx,58
.ploopa
    mov ecx,66
    push esi
.ploopb
    mov word[esi],0FFFFh
    add esi,2
    dec ecx
    jnz .ploopb
    pop esi
    add esi,288*2
    dec edx
    jnz .ploopa
    ; draw picture
    mov esi,76*2+10*288*2
    add esi,[vidbuffer]
    mov edx,56
    mov edi,PrevPicture
.ploopa2
    mov ecx,64
    push esi
.ploopb2
    mov ax,[edi]
    mov [esi],ax
    add esi,2
    add edi,2
    dec ecx
    jnz .ploopb2
    pop esi
    add esi,288*2
    dec edx
    jnz .ploopa2
    ret

NEWSYM drawfillboxsc
    pushad
    call zst_exists
    cmp eax,1
    popad
    jne .nodraw

    push eax
    ; draws a 10x10 filled box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,76+104*288
    add esi,[vidbuffer]
    add esi,eax
    mov ecx,10
    mov al,176
    push eax
    mov eax,[current_zst]
    cmp eax,[newest_zst]
    pop eax
    jne .next
    mov al,208
.next
    push esi
    push ecx
    mov ecx,10
    call drawhline
    pop ecx
    pop esi
    add esi,288
    dec ecx
    jnz .next
    pop eax
.nodraw
    ret

NEWSYM drawfillboxsc16b
    pushad
    call zst_exists
    cmp eax,1
    popad
    jne .nodraw

    push eax
    ; draws a 10x10 filled box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,76*2+104*288*2
    add esi,[vidbuffer]
    add esi,eax
    add esi,eax
    mov ecx,10
    mov ax,[saveselect.allgrn]
    push eax
    mov eax,[current_zst]
    cmp eax,[newest_zst]
    pop eax
    jne .next
    mov ax,[saveselect.allgrnb]
.next
    push esi
    push ecx
    mov ecx,10
    call drawhline16b
    pop ecx
    pop esi
    add esi,288*2
    dec ecx
    jnz .next
    pop eax
.nodraw
    ret

NEWSYM drawbox
    ; draws a box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,75+103*288
    add esi,[vidbuffer]
    add esi,eax
    mov al,dl
    push esi
    mov ecx,12
    call drawhline
    pop esi
    push esi
    mov ecx,12
    call drawvline
    pop esi
    push esi
    add esi,11
    mov ecx,12
    call drawvline
    pop esi
    add esi,11*288
    mov ecx,12
    call drawhline
    ret

NEWSYM drawbox16b
    ; draws a box according to position bl and color dx
    xor eax,eax
    mov al,11
    mul bl
    mov esi,75*2+103*288*2
    add esi,[vidbuffer]
    add esi,eax
    add esi,eax
    mov ax,dx
    push esi
    mov ecx,12
    call drawhline16b
    pop esi
    push esi
    mov ecx,12
    call drawvline16b
    pop esi
    push esi
    add esi,11*2
    mov ecx,12
    call drawvline16b
    pop esi
    add esi,11*288*2
    mov ecx,12
    call drawhline16b
    ret

%macro drawfillboxbase 2
    push eax
    mov eax,[current_zst]
    mov bl,10
    div bl
    mul bl
    mov bl,%1
    add al,bl
    mov [current_zst],eax
    pop eax
    call %2
%endmacro

%macro drawfillboxhelp 1
    drawfillboxbase %1,drawfillboxsc
%endmacro

%macro drawfillboxhelp16b 1
    drawfillboxbase %1,drawfillboxsc16b
%endmacro

%macro testpressed 1
    push eax
    mov eax,[current_zst]
    mov bl,10
    div bl
    mov bl,ah
    mov ah,10
    mul ah

%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try1
    test byte[pressed+04Fh],1
    jz %%try1
    jmp %%yes1
%%try1
%endif
    test byte[pressed+2],1
    jz %%no1
%%yes1
    mov bl,1
%%no1
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try2
    test byte[pressed+050h],1
    jz %%try2
    jmp %%yes2
%%try2
%endif
    test byte[pressed+3],1
    jz %%no2
%%yes2
    mov bl,2
%%no2
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try3
    test byte[pressed+051h],1
    jz %%try3
    jmp %%yes3
%%try3
%endif
    test byte[pressed+4],1
    jz %%no3
%%yes3
    mov bl,3
%%no3
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try4
    test byte[pressed+04Bh],1
    jz %%try4
    jmp %%yes4
%%try4
%endif
    test byte[pressed+5],1
    jz %%no4
%%yes4
    mov bl,4
%%no4
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try5
    test byte[pressed+04Ch],1
    jz %%try5
    jmp %%yes5
%%try5
%endif
    test byte[pressed+6],1
    jz %%no5
%%yes5
    mov bl,5
%%no5
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try6
    test byte[pressed+04Dh],1
    jz %%try6
    jmp %%yes6
%%try6
%endif
    test byte[pressed+7],1
    jz %%no6
%%yes6
    mov bl,6
%%no6
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try7
    test byte[pressed+047h],1
    jz %%try7
    jmp %%yes7
%%try7
%endif
    test byte[pressed+8],1
    jz %%no7
%%yes7
    mov bl,7
%%no7
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try8
    test byte[pressed+048h],1
    jz %%try8
    jmp %%yes8
%%try8
%endif
    test byte[pressed+9],1
    jz %%no8
%%yes8
    mov bl,8
%%no8
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try9
    test byte[pressed+049h],1
    jz %%try9
    jmp %%yes9
%%try9
%endif
    test byte[pressed+10],1
    jz %%no9
%%yes9
    mov bl,9
%%no9
%ifdef __UNIXSDL__
    cmp dword[numlockptr],0 ; if numlock on, let's try this first
    je %%try0
    test byte[pressed+052h],1
    jz %%try0
    jmp %%yes0
%%try0
%endif
    test byte[pressed+11],1
    jz %%no0
%%yes0
    mov bl,0
%%no0
%ifdef __UNIXSDL__
    cmp dword[numlockptr],1 ; if numlock on, disregard numpad
    je %%noleft
%endif
    test byte[pressed+75],1
    jz %%noleft
    cmp bl,0
    jne %%plzsub
    add bl,9
    jmp %%donesub
%%plzsub
    dec bl
%%donesub
    mov byte[pressed+75],2
%%noleft
%ifdef __UNIXSDL__
    cmp dword[numlockptr],1 ; if numlock on, disregard numpad
    je %%noright
%endif
    test byte[pressed+77],1
    jz %%noright
    cmp bl,9
    jne %%plzadd
    sub bl,9
    jmp %%doneadd
%%plzadd
    inc bl
%%doneadd
    mov byte[pressed+77],2
%%noright
%ifdef __UNIXSDL__
    cmp dword[numlockptr],1 ; if numlock on, disregard numpad
    je %%noup
%endif
    test byte[pressed+72],1
    jz %%noup
    cmp al,0
    jne %%plzsubten
    add al,90
    jmp %%goneup
%%plzsubten
    sub al,10
%%goneup
    mov byte[pressed+72],2
    add al,bl
    mov [current_zst],eax
    pop eax
    jmp .updatescreen%1
%%noup
%ifdef __UNIXSDL__
    cmp dword[numlockptr],1 ; if numlock on, disregard numpad
    je %%nodown
%endif
    test byte[pressed+80],1
    jz %%nodown
    cmp al,90
    jne %%plzaddten
    sub al,90
    jmp %%gonedown
%%plzaddten
    add al,10
%%gonedown
    mov byte[pressed+80],2
    add al,bl
    mov [current_zst],eax
    pop eax
    jmp .updatescreen%1
%%nodown
%ifndef __MSDOS__
%ifdef __UNIXSDL__
    test byte[pressed+92],1
%else
    test byte[pressed+0CBh],1
%endif
    jz %%noleft2
    cmp bl,0
    jne %%plzsub2
    add bl,9
    jmp %%donesub2
%%plzsub2
    dec bl
%%donesub2
%ifdef __UNIXSDL__
    mov byte[pressed+92],2
%else
    mov byte[pressed+0CBh],2
%endif
%%noleft2
%ifdef __UNIXSDL__
    test byte[pressed+94],1
%else
    test byte[pressed+0CDh],1
%endif
    jz %%noright2
    cmp bl,9
    jne %%plzadd2
    sub bl,9
    jmp %%doneadd2
%%plzadd2
    inc bl
%%doneadd2
%ifdef __UNIXSDL__
    mov byte[pressed+94],2
%else
    mov byte[pressed+0CDh],2
%endif
%%noright2
%ifdef __UNIXSDL__
    test byte[pressed+90],1
%else
    test byte[pressed+0C8h],1
%endif
    jz %%noup2
    cmp al,0
    jne %%plzsubten2
    add al,90
    jmp %%goneup2
%%plzsubten2
    sub al,10
%%goneup2
%ifdef __UNIXSDL__
    mov byte[pressed+90],2
%else
    mov byte[pressed+0C8h],2
%endif
    add al,bl
    mov [current_zst],eax
    pop eax
    jmp .updatescreen%1
%%noup2
%ifdef __UNIXSDL__
    test byte[pressed+96],1
%else
    test byte[pressed+0D0h],1
%endif
    jz %%nodown2
    cmp al,90
    jne %%plzaddten2
    sub al,90
    jmp %%gonedown2
%%plzaddten2
    add al,10
%%gonedown2
%ifdef __UNIXSDL__
    mov byte[pressed+96],2
%else
    mov byte[pressed+0D0h],2
%endif
    add al,bl
    mov [current_zst],eax
    pop eax
    jmp .updatescreen%1
%%nodown2
%endif
    add al,bl
    mov [current_zst],eax
    pop eax
%endmacro

NEWSYM saveselect
    mov byte[f3menuen],1
    mov byte[ForceNonTransp],1
    cmp byte[ForceNewGfxOff],0
    jne .nong16b
    cmp byte[cbitmode],0
    je .nong16b
%ifdef __MSDOS__
    call GetScreen
%endif
.nong16b
    cmp dword[MessageOn],0
    je .nochangem
    mov dword[MessageOn],1
.nochangem
    mov al,[newengen]
    mov byte[newengen],0
    push eax
    call copyvid
    pop eax
    mov [newengen],al
    call StopSound
    cmp byte[soundon],0
    je .nosound
    mov byte[csounddisable],1
%ifdef __MSDOS__
    call SB_blank
%endif
.nosound
    cmp byte[cbitmode],1
    je near .16b
.updatescreen8b
%ifdef __MSDOS__
    call saveselectpal
%endif
    ; draw a small blue box with a white border
    mov esi,70+70*288
    add esi,[vidbuffer]
    mov ecx,150
    mov al,80
.loop
    mov byte[esi],144
    inc esi
    dec ecx
    jnz .loop
    add esi,288-150
    dec al
    mov ecx,150
    jnz .loop
    ; draw filled boxes for existing files
    call DetermineNewest
    push ebx
    mov eax,[current_zst]
    push eax
    mov bl,10
    div bl
    add al,'0'
    mov [slotlevelnum],al
    drawfillboxhelp 0
    drawfillboxhelp 1
    drawfillboxhelp 2
    drawfillboxhelp 3
    drawfillboxhelp 4
    drawfillboxhelp 5
    drawfillboxhelp 6
    drawfillboxhelp 7
    drawfillboxhelp 8
    drawfillboxhelp 9
    pop eax
    mov [current_zst],eax
    pop ebx

    mov esi,75+73*288
    add esi,[vidbuffer]
    mov edi,.stringa
    call OutputGraphicString
    mov esi,75+83*288
    add esi,[vidbuffer]
    mov edi,.stringb
    call OutputGraphicString
    mov esi,75+93*288
    add esi,[vidbuffer]
    mov edi,.stringb2
    call OutputGraphicString
    mov esi,171+93*288
    add esi,[vidbuffer]
    mov edi,slotlevelnum
    call OutputGraphicString
    mov esi,75+118*288
    add esi,[vidbuffer]
    mov edi,.stringc
    call OutputGraphicString
    mov esi,75+128*288
    add esi,[vidbuffer]
    mov edi,.stringd
    call OutputGraphicString
    mov esi,75+138*288
    add esi,[vidbuffer]
    mov edi,.stringe
    call OutputGraphicString
    mov al,128
    mov esi,70+70*288
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline
    mov esi,70+70*288
    add esi,[vidbuffer]
    mov ecx,80
    call drawvline
    mov esi,70+149*288
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline
    mov esi,219+70*288
    add esi,[vidbuffer]
    mov ecx,80
    call drawvline
    mov esi,75+103*288
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline
    mov esi,75+114*288
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline
    mov esi,75+104*288
    add esi,[vidbuffer]
    mov bl,11
.nextvline
    mov ecx,10
    push esi
    push ebx
    call drawvline
    pop ebx
    pop esi
    add esi,11
    dec bl
    jnz .nextvline
    mov esi,78+106*288
    add esi,[vidbuffer]
    mov al,1
    call outputchar
    mov bl,9
.nextnumchar
    add esi,11
    inc al
    push ebx
    call outputchar
    pop ebx
    dec bl
    jnz .nextnumchar
    mov byte[curblank],0h

    mov dl,160
    call drawbox
    push ebx
    call copyvid
    pop ebx
    ; wait until esc/enter is pressed
.noesc
    mov dl,128
    call drawbox
    mov ecx,2500
    call delay
    testpressed 8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    mov ecx,2500
    call delay
    testpressed 8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    push ebx
    call copyvid
    pop ebx
    mov ecx,2500
    call delay
    testpressed 8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    mov ecx,2500
    call delay
    testpressed 8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    mov dl,160
    call drawbox
    push ebx
    call copyvid
    pop ebx
    jmp .noesc
.enter
    mov byte[pressed+28],2
.esc

    mov eax,pressed
    mov ecx,256
.looppr
    cmp byte[eax],1
    jne .notpr
    mov byte[eax],2
.notpr
    inc eax
    dec ecx
    jnz .looppr
%ifdef __MSDOS__
    mov byte[pressed+1],0
%endif

    mov word[t1cc],0
    mov byte[csounddisable],0
    call StartSound

%ifdef __MSDOS__
    call dosmakepal
%endif
    mov byte[f3menuen],0
    mov byte[ForceNonTransp],0
    ret

SECTION .bss
.allred resw 1
.allgrn resw 1
.allgrnb resw 1
.blue   resw 1
.stepb  resw 1
SECTION .text

; Start 16-bit stuff here
.16b
    cmp byte[newengen],0
    je .notng
    mov byte[GUIOn],1
.notng
    ; draw shadow behind box
    mov esi,80*2+90*288*2
    add esi,[vidbuffer]
    mov ecx,150
    mov al,70
    mov ah,5
.loop16b2
    mov dx,[esi]
    and dx,[vesa2_clbit]
    shr dx,1
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b2
    add esi,288*2-150*2
    dec al
    mov ecx,150
    jnz .loop16b2

.updatescreen16b
    mov ax,018h
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov [.allgrn],ax

    mov ax,25
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov [.allgrnb],ax
    mov ax,12
    mov cl,[vesa2_gpos]
    shl ax,cl
    or [.allgrnb],ax

    mov ax,01Fh
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov [.allred],ax
    mov ax,012h
    mov cl,[vesa2_bpos]
    shl ax,cl
    mov dx,ax
    mov ax,01h
    mov cl,[vesa2_gpos]
    shl ax,cl
    mov bx,ax
    mov ax,01h
    mov cl,[vesa2_rpos]
    shl ax,cl
    or bx,ax

    ; draw a small blue box with a white border
    mov esi,70*2+70*288*2
    add esi,[vidbuffer]
    mov ecx,150
    mov al,80
    mov ah,5
.loop16b
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b
    add esi,288*2-150*2
    dec ah
    jnz .nocolinc16b
    add dx,bx
    mov ah,5
.nocolinc16b
    dec al
    mov ecx,150
    jnz .loop16b

    ; draw filled boxes for existing files
    call DetermineNewest
    push ebx
    mov eax,[current_zst]
    push eax
    mov bl,10
    div bl
    add al,'0'
    mov [slotlevelnum],al
    drawfillboxhelp16b 0
    drawfillboxhelp16b 1
    drawfillboxhelp16b 2
    drawfillboxhelp16b 3
    drawfillboxhelp16b 4
    drawfillboxhelp16b 5
    drawfillboxhelp16b 6
    drawfillboxhelp16b 7
    drawfillboxhelp16b 8
    drawfillboxhelp16b 9
    pop eax
    mov [current_zst],eax
    pop ebx

    mov esi,75*2+73*288*2
    add esi,[vidbuffer]
    mov edi,.stringa
    call OutputGraphicString16b
    mov esi,75*2+83*288*2
    add esi,[vidbuffer]
    mov edi,.stringb
    call OutputGraphicString16b
    mov esi,75*2+93*288*2
    add esi,[vidbuffer]
    mov edi,.stringb2
    call OutputGraphicString16b
    mov esi,171*2+93*288*2
    add esi,[vidbuffer]
    mov edi,slotlevelnum
    call OutputGraphicString16b
    mov esi,75*2+118*288*2
    add esi,[vidbuffer]
    mov edi,.stringc
    call OutputGraphicString16b
    mov esi,75*2+128*288*2
    add esi,[vidbuffer]
    mov edi,.stringd
    call OutputGraphicString16b
    mov esi,75*2+138*288*2
    add esi,[vidbuffer]
    mov edi,.stringe
    call OutputGraphicString16b
    mov ax,0FFFFh
    mov esi,70*2+70*288*2
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline16b
    mov esi,70*2+70*288*2
    add esi,[vidbuffer]
    mov ecx,80
    call drawvline16b
    mov esi,70*2+149*288*2
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline16b
    mov esi,219*2+70*288*2
    add esi,[vidbuffer]
    mov ecx,80
    call drawvline16b
    mov esi,75*2+103*288*2
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline16b
    mov esi,75*2+114*288*2
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline16b
    mov esi,75*2+104*288*2
    add esi,[vidbuffer]
    mov bl,11
.nextvline16b
    mov ecx,10
    push esi
    push ebx
    call drawvline16b
    pop ebx
    pop esi
    add esi,22
    dec bl
    jnz .nextvline16b
    mov esi,78*2+106*288*2
    add esi,[vidbuffer]
    mov al,1
    call outputchar16b
    mov bl,9
.nextnumchar16b
    add esi,22
    inc al
    push ebx
    call outputchar16b
    pop ebx
    dec bl
    jnz .nextnumchar16b

    mov byte[curblank],0h
;    mov dx,[.allred]
;    call drawbox16b
    push ebx
    mov al,[newengen]
    mov byte[newengen],0
    push eax
    call copyvid
    pop eax
    mov [newengen],al
    pop ebx
    ; wait until esc/enter is pressed

    mov byte[PrevPictureVal],0FFh
.noesc16b
    push eax
    mov eax,[current_zst]
    mov [CurPictureVal],al
    pop eax
    pushad
    call GetPicture
    popad

    mov dx,0FFFFh
    call drawbox16b
    mov ecx,2500
    call delay
    testpressed 16b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    mov ecx,2500
    call delay
    testpressed 16b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    push ebx
    mov al,[newengen]
    mov byte[newengen],0
    push eax
    call copyvid
    pop eax
    mov [newengen],al
    pop ebx
    mov ecx,2500
    call delay
    testpressed 16b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    mov ecx,2500
    call delay
    testpressed 16b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    mov dx,[.allred]
    call drawbox16b
    push ebx
    mov al,[newengen]
    mov byte[newengen],0
    push eax
    call copyvid
    pop eax
    mov [newengen],al
    pop ebx
    jmp .noesc16b
.enter16b
    mov byte[pressed+28],2
.esc16b
    mov eax,pressed
    mov ecx,256
.looppr2
    cmp byte[eax],1
    jne .notpr2
    mov byte[eax],2
.notpr2
    inc eax
    dec ecx
    jnz .looppr2
.prwin
%ifdef __MSDOS__
    mov byte[pressed+1],0
%endif
    mov word[t1cc],0
    mov byte[csounddisable],0
    call StartSound
    mov byte[f3menuen],0
    mov byte[ForceNonTransp],0
    mov byte[GUIOn],0
    pushad
    call Clear2xSaIBuffer
    popad
    ret

SECTION .data
.stringa db 'PLEASE SELECT',0
.stringb db 'SAVE STATE SLOT',0
.stringb2 db 'SLOT LEVEL:',0
.stringc db 'USE CURSOR KEYS',0
.stringd db 'TO MOVE AND',0
.stringe db 'ENTER TO SELECT',0
slotlevelnum db '-',0

SECTION .bss
NEWSYM ForceNonTransp, resb 1

;*******************************************************
; MakePal                     Changes the entire palette
;*******************************************************
; set the brightness with [maxbr]
SECTION .bss
NEWSYM cgramback, resw 256
SECTION .text

NEWSYM doveg
    pushad
    ; backup cgram
    mov ecx,128
    xor ebx,ebx
.loop
    mov eax,[cgram+ebx]
    mov [cgramback+ebx],eax
    add ebx,4
    dec ecx
    jnz .loop
    xor eax,eax
    mov al,[coladdr]
    add al,[coladdg]
    add al,[coladdb]
    xor dx,dx
    mov bx,3
    div bx
    and ax,011111b
    mov [coladdr],al
    mov [coladdg],al
    mov [coladdb],al
    xor eax,eax
.next
    push eax
    mov ax,[cgram+eax]
    mov bx,ax
    mov cx,ax
    and bx,011111b
    and cx,1111100000b
    shr cx,5
    add bx,cx
    mov cx,ax
    and cx,111110000000000b
    shr cx,10
    add bx,cx
    mov ax,bx
    xor dx,dx
    mov bx,3
    div bx
    and ax,011111b
    mov cx,ax
    mov bx,ax
    shl bx,5
    or ax,bx
    shl cx,10
    or ax,cx
    mov bx,ax
    pop eax
    mov [cgram+eax],bx
    add eax,2
    cmp eax,200h
    jne .next
    popad
    ret

NEWSYM dovegrest
    pushad
    ; backup cgram
    mov ecx,128
    xor ebx,ebx
.loop
    mov eax,[cgramback+ebx]
    mov [cgram+ebx],eax
    add ebx,4
    dec ecx
    jnz .loop
    popad
    ret

SECTION .bss
NEWSYM tempco0, resw 1
NEWSYM prevbright, resb 1                 ; previous brightness
SECTION .text

%ifdef __MSDOS__
NEWSYM dosmakepal
    cmp byte[V8Mode],1
    jne .noveg
    call doveg
.noveg
    mov ax,[cgram]
    mov [tempco0],ax
    test byte[scaddtype],00100000b
    jz near .noaddition
    test byte[scaddtype],10000000b
    jnz near .noaddition
    mov cx,[cgram]
    mov ax,cx
    and ax,001Fh
    add al,[coladdr]
    cmp al,01Fh
    jb .noadd
    mov al,01Fh
.noadd
    mov bx,ax
    mov ax,cx
    shr ax,5
    and ax,001Fh
    add al,[coladdg]
    cmp al,01Fh
    jb .noaddb
    mov al,01Fh
.noaddb
    shl ax,5
    add bx,ax
    mov ax,cx
    shr ax,10
    and ax,001Fh
    add al,[coladdb]
    cmp al,01Fh
    jb .noaddc
    mov al,01Fh
.noaddc
    shl ax,10
    add bx,ax
    mov [cgram],bx
.noaddition
    cmp byte[Palette0],0
    je .nocol0mod
    mov word[cgram],0
.nocol0mod
NEWSYM makepalb
    mov edi,cgram
    mov ebx,prevpal
    xor ah,ah
.loopa
    mov cx,[edi]
    push eax
    push ebx
    mov [ebx],cx
    mov al,ah
    mov dx,03C8h
    out dx,al
    mov ax,cx
    and al,01Fh
    mov bh,[maxbr]
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    mov dx,03C9h
    add al,[gammalevel]
    cmp al,63
    jbe .nor
    mov al,63
.nor
    out dx,al
    mov ax,cx
    shr ax,5
    and al,01Fh
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    add al,[gammalevel]
    cmp al,63
    jbe .nog
    mov al,63
.nog
    out dx,al
    mov ax,cx
    shr ax,10
    and al,01Fh
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    add al,[gammalevel]
    cmp al,63
    jbe .nob
    mov al,63
.nob
    out dx,al
    pop ebx
    pop eax
    add edi,2
    add ebx,2
    inc ah
    jnz near .loopa
    mov al,[maxbr]
    mov [prevbright],al
    mov ax,[tempco0]
    mov [cgram],ax
    cmp byte[MessageOn],0
    je .nochange128
    mov dx,03C8h
    mov al,128
    out dx,al
    mov al,63
    inc dx
    out dx,al
    out dx,al
    out dx,al
    mov dx,03C8h
    mov al,128+64
    out dx,al
    mov al,0
    inc dx
    out dx,al
    out dx,al
    out dx,al
.nochange128
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret

;*******************************************************
; ChangePal                          Sets up the palette
;*******************************************************

NEWSYM doschangepal
    cmp byte[V8Mode],1
    jne .noveg
    call doveg
.noveg
    mov ax,[cgram]
    mov [tempco0],ax
    test byte[scaddtype],00100000b
    jz near .noaddition
    test byte[scaddtype],10000000b
    jnz near .noaddition
    mov cx,[cgram]
    mov ax,cx
    and ax,001Fh
    add al,[coladdr]
    cmp al,01Fh
    jb .noadd
    mov al,01Fh
.noadd
    mov bx,ax
    mov ax,cx
    shr ax,5
    and ax,001Fh
    add al,[coladdg]
    cmp al,01Fh
    jb .noaddb
    mov al,01Fh
.noaddb
    shl ax,5
    add bx,ax
    mov ax,cx
    shr ax,10
    and ax,001Fh
    add al,[coladdb]
    cmp al,01Fh
    jb .noaddc
    mov al,01Fh
.noaddc
    shl ax,10
    add bx,ax
    mov [cgram],bx
.noaddition
    cmp byte[Palette0],0
    je .nocol0mod
    mov word[cgram],0
.nocol0mod
    ; check if brightness differs
    mov al,[maxbr]
    cmp al,[prevbright]
    jne near makepalb
    ; check for duplicate palette (Compare prevpal with cgram)
    mov ebx,prevpal
    mov edi,cgram
    xor ah,ah
.loopa
    mov cx,[edi]
    cmp cx,[ebx]
    je .nochange
    push eax
    push ebx
    push eax
    pop eax
    mov [ebx],cx
    mov al,ah
    mov dx,03C8h
    out dx,al
    mov ax,cx
    and al,01Fh
    mov bh,[maxbr]
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    add al,[gammalevel]
    cmp al,63
    jbe .nor
    mov al,63
.nor
    inc dx
    out dx,al
    mov ax,cx
    shr ax,5
    and al,01Fh
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    add al,[gammalevel]
    cmp al,63
    jbe .nog
    mov al,63
.nog
    out dx,al
    mov ax,cx
    shr ax,10
    and al,01Fh
    mov bl,bh
    mul bl
    mov bl,15
    div bl
    shl al,1
    add al,[gammalevel]
    cmp al,63
    jbe .nob
    mov al,63
.nob
    out dx,al
    pop ebx
    pop eax
.nochange
    add edi,2
    add ebx,2
    inc ah
    jnz near .loopa
    mov ax,[tempco0]
    mov [cgram],ax
    cmp byte[MessageOn],0
    je .nochange128
    mov dx,03C8h
    mov al,128
    out dx,al
    mov al,63
    inc dx
    out dx,al
    out dx,al
    out dx,al
.nochange128
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret
%endif

;*******************************************************
; CopyVid                       Copies buffer into video
;*******************************************************

NEWSYM showfps
    mov ax,60
    cmp byte[romispal],0
    je .ntsc
    mov ax,50
.ntsc
    inc byte[curfps]
    cmp byte[nextframe],al
    jb .nofrc
    mov cl,[curfps]
    mov [lastfps],cl
    mov cl,[curfps2]
    mov [lastfps2],cl
    mov byte[curfps],0
    mov byte[curfps2],0
    sub byte[nextframe],al
.nofrc
    mov cl,[SloMo]
    or cl,cl
    jz .noslw
    inc cl
    div cl
.noslw
    mov cl,al

    cmp byte[cbitmode],1
    je near .do16b

%ifdef __MSDOS__
    call displayfpspal
%endif

    movzx ax,byte[lastfps]
    mov bl,10
    div bl
    shl al,4
    add ah,al
    mov al,ah
    mov esi,208*288+32
    add esi,[vidbuffer]
    push ecx
    call outputhex

    mov esi,208*288+48
    add esi,[vidbuffer]
    mov al,29h
    call outputchar
    pop ecx

    mov al,cl
    mov bl,10
    xor ah,ah
    div bl
    shl al,4
    add ah,al
    mov al,ah
    mov esi,208*288+56
    add esi,[vidbuffer]
    call outputhex
    ret

.do16b
  mov esi,208*288*2+48*2
  add esi,[vidbuffer]
  mov al,[lastfps]
  push ecx
  xor ecx,ecx
.strloop
  xor ah,ah
  add al,48
  sub esi,8*2
.asciiloop16b
  cmp al,58
  jb .h2adone16b
  inc ah
  sub al,10
  jmp .asciiloop16b
.h2adone16b
  mov cl,al
  mov al,[ASCII2Font+ecx]
  call outputchar16b
  mov al,ah
  or al,al
  jnz .strloop

  mov esi,208*288*2+48*2
  add esi,[vidbuffer]
  mov al,41 ; '/'
  call outputchar16b
  pop ecx

  mov al,cl
  mov bl,10
  xor ah,ah
  div bl
  shl al,4
  add ah,al
  mov al,ah
  mov esi,208*288*2+56*2
  add esi,[vidbuffer]
  call outputhex16
  ret

SECTION .bss
NEWSYM tempoffset, resw 1
NEWSYM Testval, resd 1
SECTION .text

EXTSYM TwelveHourClock

NEWSYM ClockOutput
    cmp byte[cbitmode],1
    je near .do16b3
.no16b3
%ifdef __MSDOS__
    call displayfpspal
%endif
    mov esi,215*288+32+192
    add esi,[vidbuffer]
    cmp byte[ForceNonTransp],1
    je .menuon8
    cmp byte[ClockBox],1
    jne near .do8b
.menuon8
    mov ebx,7
.loop2
    mov ecx,12
    mov byte[esi-1],0C0h
.loop
    mov dword[esi],0C0C0C0C0h
    add esi,4
.nobox8
    dec ecx
    jnz .loop
    add esi,288-48
    dec ebx
    jnz .loop2
    jmp .do8b
.do16b3
    mov esi,215*288*2+32*2+192*2
    add esi,[vidbuffer]
    cmp byte[ForceNonTransp],1
    je .menuon16
    cmp byte[ClockBox],1
    jne .do8b
.menuon16
    mov ebx,7
.loop2b
    mov ecx,24
    mov word[esi-2],0
    mov word[esi-2+75036*4],0
.loopb
    mov dword[esi],0
    mov dword[esi+75036*4],0
    add esi,4
.nobox16
    dec ecx
    jnz .loopb
    add esi,288*2-48*2
    dec ebx
    jnz .loop2b
.do8b
    call GetTimeInSeconds
    xor edx,edx
    mov ebx,60
    div ebx
    push eax
    ; edx = seconds
    mov eax,edx
    xor edx,edx
    mov ebx,10
    div ebx
    mov esi,216*288+32+228
    call .output
    pop eax
    mov ebx,60
    xor edx,edx
    div ebx
    push eax
    ; edx = minutes
    mov eax,edx
    xor edx,edx
    mov ebx,10
    div ebx
    mov esi,216*288+32+210
    call .output
    pop eax
    ; eax = hours
    cmp byte[TwelveHourClock],1
    jne .no12hour
    ; check to see if it's 12 PM
    cmp eax,12
    jbe .not12pm
    sub eax,12
.not12pm
    cmp eax,0
    jne .no12hour
    add eax,12
.no12hour
    xor edx,edx
    mov ebx,10
    div ebx
    mov esi,216*288+32+192
    call .output
    cmp byte[cbitmode],1
    je .do16b2
.no16b4
    mov esi,216*288+32+222
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar5x5
    popad
    mov esi,216*288+32+204
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar5x5
    popad
    ret

.do16b2
    mov esi,216*288*2+32*2+222*2
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar16b5x5
    popad
    mov esi,216*288*2+32*2+204*2
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar16b5x5
    popad
    ret
.output
    ; output char value at al and dl
    cmp byte[cbitmode],1
    je .do16b
.no16b5
    add esi,[vidbuffer]
    and eax,0FFh
    add al,48
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar5x5
    popad
    xor eax,eax
    mov al,dl
    add al,48
    mov al,[ASCII2Font+eax]
    add esi,6
    pushad
    call outputchar5x5
    popad
    ret
.do16b
    add esi,esi
    add esi,[vidbuffer]
    and eax,0FFh
    add al,48
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar16b5x5
    popad
    xor eax,eax
    mov al,dl
    add al,48
    mov al,[ASCII2Font+eax]
    add esi,12
    pushad
    call outputchar16b5x5
    popad
    ret

SECTION .bss
NEWSYM SoundTest, resb 1
blahrnr resw 1

SECTION .text

NEWSYM hextestoutput

    mov dx,[bg3scroly]
;    and dx,0F8h
    shr edx,3
    shl edx,6
    xor eax,eax
    mov ax,[bg3ptr]
    add ax,dx
    xor edx,edx
    mov dx,[bg3scrolx]
    and dx,0F8h
    shr edx,3
    shl edx,1
    add ax,dx
    mov dx,[bg3scrolx]
    test dx,8000h
    jz .nooma
    and dx,0F000h
    shr dx,5
    add ax,dx
.nooma
    add eax,40h
    mov edx,eax
    mov [Testval],edx
%ifdef __MSDOS__
    call displayfpspal
%endif

    mov esi,[vram]
    mov ax,0
    mov ecx,400h
.loop
;    mov word[esi],ax
    add esi,2
    dec ecx
    jnz .loop
    inc word[blahrnr]
    mov esi,216*288+32
    add esi,[vidbuffer]
    xor eax,eax
    ; 4F00h
    mov ebx,[C4Ram]
    mov ebx,[vram]
    mov al,[DecompAPtr+1h]
    call outputhex
    mov esi,216*288+32+16
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov ebx,[vram]
    mov al,[DecompAPtr]
    call outputhex
    mov esi,216*288+70
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov al,[bg1objptr+1]
    call outputhex
    mov esi,216*288+70+16
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov al,[bg1objptr]
    call outputhex
    mov esi,216*288+108
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov al,[ebx+4]
    call outputhex
    mov esi,216*288+108+16
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov al,[ebx+9]
    call outputhex
    mov esi,216*288+146
    add esi,[vidbuffer]
    xor eax,eax
    mov ebx,[C4Ram]
    mov al,[ebx+10]
    call outputhex
    xor eax,eax
    mov esi,216*288+146+16
    add esi,[vidbuffer]
    or al,[bg1scroly]
    mov ebx,[C4Ram]
    mov al,[ebx+11]
    call outputhex
    ret

SECTION .bss
NEWSYM SoundPlayed0, resb 1
NEWSYM SoundPlayed1, resb 1
NEWSYM SoundPlayed2, resb 1
NEWSYM SoundPlayed3, resb 1
NEWSYM SoundPlayed4, resb 1
NEWSYM SoundPlayed5, resb 1
NEWSYM SoundPlayed6, resb 1
NEWSYM SoundPlayed7, resb 1
SECTION .text

NEWSYM ShowSound
    add esi,[vidbuffer]
.next
    mov [esi],ebx
    mov [esi+4],ebx
    mov [esi+8],bx
    sub esi,288
    dec al
    jnz .next
    ret

NEWSYM sounddisplay
%ifdef __MSDOS__
    call displayfpspal
%endif

    push esi
    push ebx
    push eax
    mov ebx,80808080h
    cmp byte[SoundPlayed0],0
    je .nosnd0
    mov al,[SoundPlayed0]
    mov esi,223*288+16
    call ShowSound
    sub byte[SoundPlayed0],2
.nosnd0
    cmp byte[SoundPlayed1],0
    je .nosnd1
    mov al,[SoundPlayed1]
    mov esi,223*288+28
    call ShowSound
    sub byte[SoundPlayed1],2
.nosnd1
    cmp byte[SoundPlayed2],0
    je .nosnd2
    mov al,[SoundPlayed2]
    mov esi,223*288+40
    call ShowSound
    sub byte[SoundPlayed2],2
.nosnd2
    cmp byte[SoundPlayed3],0
    je .nosnd3
    mov al,[SoundPlayed3]
    mov esi,223*288+52
    call ShowSound
    sub byte[SoundPlayed3],2
.nosnd3
    cmp byte[SoundPlayed4],0
    je .nosnd4
    mov al,[SoundPlayed4]
    mov esi,223*288+64
    call ShowSound
    sub byte[SoundPlayed4],2
.nosnd4
    cmp byte[SoundPlayed5],0
    je .nosnd5
    mov al,[SoundPlayed5]
    mov esi,223*288+76
    call ShowSound
    sub byte[SoundPlayed5],2
.nosnd5
    cmp byte[SoundPlayed6],0
    je .nosnd6
    mov al,[SoundPlayed6]
    mov esi,223*288+88
    call ShowSound
    sub byte[SoundPlayed6],2
.nosnd6
    cmp byte[SoundPlayed7],0
    je .nosnd7
    mov al,[SoundPlayed7]
    mov esi,223*288+100
    call ShowSound
    sub byte[SoundPlayed7],2
.nosnd7
    pop eax
    pop esi
    pop ebx
    ret

NEWSYM waitvsync
    mov dx,3DAh             ;VGA status port
;.loop
;    in al,dx
;    test al,8               ;check VR bit
;    jnz .loop               ;in middle of VR, better wait for next one
.loop2
    in al,dx
    test al,8
    jz .loop2               ;updating the screen
    ret

SECTION .data
NEWSYM prevengval, db 10


SECTION .text

NEWSYM copyvid
    mov dword[.sdrawptr],0
    ; Test if add table needs updating
    cmp byte[cbitmode],0
    je .notatud
    cmp byte[vesa2red10],0
    je .notatud
    mov al,[newengen]
    cmp [prevengval],al
    je .notatud
    mov [prevengval],al
    call genfulladdtab
    jmp .notatud
.redadd
    call genfulladdtabng
.notatud
    cmp dword[MessageOn],0
    je near .nomsg
    cmp byte[cbitmode],1
    je near .do16b
.no16b
    mov edi,[Msgptr]
    mov esi,192*288+32
    add esi,[vidbuffer]
    cmp edi,CSStatus
    je .fivex5b
    cmp byte[SmallMsgText],1
    je .smallmsgtext
    call OutputGraphicString.no16bit
    jmp .nfivex5b
.fivex5b
    call OutputGraphicString5x5
    mov edi,CSStatus2
    mov esi,200*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
    mov edi,CSStatus3
    mov esi,208*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
    mov edi,CSStatus4
    mov esi,216*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
    jmp .nfivex5b
.smallmsgtext
    call OutputGraphicString5x5
.nfivex5b
    dec dword[MessageOn]
    jnz near .nomsg
%ifdef __MSDOS__
    cmp byte[cbitmode],1
    je near .nomsg
    call dosmakepal
%endif
    jmp .nomsg
.do16b
    mov edi,[Msgptr]
    mov esi,192*288*2+32*2
    add esi,[vidbuffer]
    cmp edi,CSStatus
    je .fivex5
    cmp byte[SmallMsgText],1
    je .smallmsgtext2
    call OutputGraphicString16b
    jmp .nfivex5
.fivex5
    call OutputGraphicString16b5x5
    mov edi,CSStatus2
    mov esi,200*288*2+32*2
    add esi,[vidbuffer]
    call OutputGraphicString16b5x5
    mov edi,CSStatus3
    mov esi,208*288*2+32*2
    add esi,[vidbuffer]
    call OutputGraphicString16b5x5
    mov edi,CSStatus4
    mov esi,216*288*2+32*2
    add esi,[vidbuffer]
    call OutputGraphicString16b5x5
    jmp .nfivex5
.smallmsgtext2
    call OutputGraphicString16b5x5
.nfivex5
    dec dword[MessageOn]
.nomsg
    cmp byte[MovieProcessing],0
    jz .nomovie4
    cmp byte[MovieDisplayFrame],0
    jz .nomovie4
    pushad
    call GetMovieFrameStr
    popad
    mov edi,MovieFrameStr
    cmp byte[cbitmode],1
    jne .not16bframe
    mov esi,216*288*2+32*2
    add esi,[vidbuffer]
    call OutputGraphicString16b5x5
    jmp .nomovie4
.not16bframe
    mov esi,216*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
.nomovie4
    jmp vidpaste
SECTION .bss
.sdrawptr resd 1
SECTION .text

NEWSYM vidpaste
%ifdef __MSDOS__
    cmp byte[vsyncon],0
    je .novsync
    cmp byte[Triplebufen],0
    jne .novsync
    cmp byte[curblank],0h
    jne .novsync
    call waitvsync
.novsync
    cmp byte[cbitmode],1
    je .nopal
    cmp byte[curblank],0
    jne .nopal
    call doschangepal
.nopal
%endif
    cmp byte[FPSOn],0
    je .nofps
    cmp byte[curblank],0
    jne .nofps
    call showfps
.nofps
    cmp byte[TimerEnable],0
    je .noclock
    cmp byte[ShowTimer],0
    je .noclock
    call ClockOutput
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
    jmp DrawScreen

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
NEWSYM MsgCount,  dd 120                ; How long message will stay (PAL = 100)

SECTION .bss
NEWSYM lastfps,   resb 1                  ; stores the last fps encountered
NEWSYM lastfps2,  resb 1                  ; stores the last fps encountered
NEWSYM curfps2,   resb 1                  ; current video refresh fps
NEWSYM Msgptr,    resd 1                  ; Pointer to message
NEWSYM MessageOn, resd 1                  ; Message On Countdown
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
