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

EXTSYM BGMA,DSPMem,INTEnab,V8Mode,antienab,cacheud,cbitmode
EXTSYM ccud,cfield,cgfxmod,cgram,coladdb,coladdg,coladdr,cpalval,curblank
EXTSYM curfps,cvidmode,delay,draw16bnng,extlatch,fnamest,fulladdtab,En2xSaI
EXTSYM gammalevel,hirestiledat,ignor512,latchx,latchy,maxbr,ForceNewGfxOff
EXTSYM newengen,nextframe,objptr,pressed,prevpal,res512switch,res640
EXTSYM resolutn,romispal,scaddtype,scadtng,scanlines
EXTSYM scbcong,selcA000,snesmouse,t1cc,vcache4b,vesa2_bpos,spritetablea
EXTSYM vesa2_clbit,vesa2_gpos,vesa2_rpos,vesa2red10,vesa2selec,vidbuffer
EXTSYM vram,vsyncon,vbufdptr,KeyStateSelct,forceblnk,soundon
EXTSYM Open_File,Read_File,Close_File,Create_File,Write_File,Get_File_Date
EXTSYM Triplebufen,makepal,changepal,saveselectpal,displayfpspal,superscopepal
EXTSYM DrawScreen,MMXSupport
EXTSYM Get_MouseData,Get_MousePositionDisplacement
EXTSYM GUIEnableTransp
EXTSYM GUIFontData
EXTSYM StopSound,StartSound
EXTSYM PrevPicture,File_Seek,File_Seek_End,nggposng
EXTSYM Palette0
EXTSYM GetTimeInSeconds
EXTSYM scaddset,scrnon,spcRam,nmiprevline,bgmode,ofsmcptr
EXTSYM interlval,bg3ptr,bg3scroly,bg3scrolx,C4Ram,SprValAdd,SA1IRQEn,SA1IRQV
EXTSYM winbg1en,winlogica,wincolen,winlogicb,dsp1ptr,dsp1array,bg3objptr
EXTSYM cnetptrhead,cnetptrtail,JoyBOrig,pl2neten,Voice6Ptr,HIRQLoc,SA1DoIRQ
EXTSYM mode7A,mode7B,mode7C,mode7D,mode7set,winbg3en,winl1,winr1,SA1DMAInfo
EXTSYM winl2,winr2,VIRQLoc,SA1Enable,mode7X0,mode7Y0,SA1Temp
EXTSYM SA1IRQTemp,SA1IRQEnable,SA1DMADest,SA1DMAChar,SA1DMASource,SA1DMACount
EXTSYM objptrn,nglogicval,bgtilesz,C4values
EXTSYM curexecstate,TempVidInfo,LatestBank,C4ObjSelec
EXTSYM BGMS1,scadsng,winenabm,winenabs,vidbright
EXTSYM genfulladdtab,genfulladdtabng
EXTSYM KeyQuickChat,CNetType,WritetochatBuffer,NetAddChar,TimerEnable,ShowTimer
EXTSYM ClearScreenSkip,debugdisble,cmovietimeint
EXTSYM ChatNick
EXTSYM StringLength
EXTSYM chatstrLt
EXTSYM GUIOn,FilteredGUI,HalfTrans
EXTSYM SmallMsgText
EXTSYM ClearScreen
EXTSYM Mode7HiRes,mosenng,mosszng,intrlng,mode7hr ;,VESAAddr
EXTSYM GUICPC, newgfx16b
EXTSYM vesa2_clbitng,vesa2_clbitng2,vesa2_clbitng3
EXTSYM granadd,CSStatus,CSStatus2,CSStatus3
EXTSYM SpecialLine
EXTSYM vidbufferofsb
;EXTSYM Super2xSaI
EXTSYM HalfTransB,HalfTransC

EXTSYM cur_zst_size,old_zst_size
EXTSYM MovieProcessing,mzt_chdir,UpChdir
EXTSYM MovieFrameStr,GetMovieFrameStr,MovieDisplayFrame

%ifdef __MSDOS__
EXTSYM SB_blank
%endif

SECTION .bss
NEWSYM ScreenScale, resb 1        ; If horizontal is scaled or not
NEWSYM TempDebugV, resw 1       ; Temporary Debugging variable
SECTION .text


%macro MMXStuff 0
%%1
    movq mm0,[esi]
    movq [es:edi],mm0
    movq mm1,[esi+8]
    movq [es:edi+8],mm1
    add esi,16
    add edi,16
    dec ecx
    jnz %%1
%endmacro

%macro FPUStuff 1
    FILD QWORD [ESI+%1*16]
    FILD QWORD [ESI+8+%1*16]
    FISTP QWORD [ES:EDI+8+%1*16]
    FISTP QWORD [ES:EDI+%1*16]
%endmacro

%MACRO CopyFPU 0
%ENDMACRO

%include "video/2xsaimmx.inc"
%include "video/copyvid.inc"

SECTION  .text
NEWSYM FPUZero
%if 0

; omg this is lame ;P
    mov [.Zero],eax
    mov [.Zero+4],eax
    mov [.Zero2],eax
    mov [.Zero2+4],eax
.TopOfLoop
    FILD QWORD [.Zero]
    FILD QWORD [.Zero2]
    FXCH
    FISTP QWORD [EDI]
    FISTP QWORD [EDI+8]
    ADD EDI,16
    DEC ECX
    JNZ .TopOfLoop
%else
    fld1
    fsub st0,st0
.TopOfLoop
    fst qword [edi]
    fst qword [edi+8]
    add edi,16
    dec ecx
    jnz .TopOfLoop
    fstp st0
%endif    
    ret

%if 0
SECTION .bss
.Zero resd 2
.Zero2 resd 2
SECTION .text
%endif

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
    mov bl,byte[ccud]
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



NEWSYM processmouse
    push esi
    push edi
    push edx
    push ebx
    call Get_MouseData
    mov [mousebuttons],bx
    cmp byte[snesmouse],3
    jne .ss
    cmp byte[pressed+13],0
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
    call Get_MousePositionDisplacement
    cmp byte[snesmouse],4
    je .le
    cmp byte[snesmouse],3
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
    cmp byte[snesmouse],4
    je .le2
    cmp byte[snesmouse],3
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
         db 0Bh,13h,19h,1Fh,18h,18h,0Bh,19h,00h,00h,00h,00h,00h,00h,00h,00h
         db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
         db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
         db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
         db 00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
         db 00h,00h,00h,00h,00h,00h,00h,4Dh,4Ch,4Bh,4Ah,45h,46h,47h,48h,49h

NEWSYM FontData
         db 0,0,0,0,0,0,0,0
         db 01111100b,11000110b,11001110b,11010110b     ; 0, 1
         db 11100110b,11000110b,01111100b,00000000b
         db 00011000b,00111000b,01111000b,00011000b     ; 1, 2
         db 00011000b,00011000b,01111110b,00000000b
         db 01111100b,11000110b,00001100b,00011000b     ; 2, 3
         db 00110000b,01100110b,11111110b,00000000b
         db 01111100b,11000110b,00000110b,00111100b     ; 3, 4
         db 00000110b,11000110b,01111100b,00000000b
         db 00111100b,01101100b,11001100b,11111110b     ; 4, 5
         db 00001100b,00001100b,00001100b,00000000b
         db 11111110b,11000000b,11000000b,11111100b     ; 5, 6
         db 00000110b,11000110b,01111100b,00000000b
         db 00111100b,01100000b,11000000b,11111100b     ; 6, 7
         db 11000110b,11000110b,01111100b,00000000b
         db 11111110b,11000110b,00000110b,00001100b     ; 7, 8
         db 00011000b,00011000b,00011000b,00000000b
         db 01111100b,11000110b,11000110b,01111100b     ; 8, 9
         db 11000110b,11000110b,01111100b,00000000b
         db 01111100b,11000110b,11000110b,01111110b     ; 9, A
         db 00000110b,11000110b,01111100b,00000000b
         db 00111000b,01101100b,11000110b,11111110b     ; A, B
         db 11000110b,11000110b,11000110b,00000000b
         db 11111100b,11000110b,11000110b,11111100b     ; B, C
         db 11000110b,11000110b,11111100b,00000000b
         db 01111100b,11000110b,11000000b,11000000b     ; C, D
         db 11000000b,11000110b,01111100b,00000000b
         db 11111100b,11000110b,11000110b,11000110b     ; D, E
         db 11000110b,11000110b,11111100b,00000000b
         db 11111110b,11000000b,11000000b,11111000b     ; E, F
         db 11000000b,11000000b,11111110b,00000000b
         db 11111110b,11000000b,11000000b,11111000b     ; F, 10
         db 11000000b,11000000b,11000000b,00000000b
         db 01111100b,11000110b,11000000b,11000000b     ; G, 11
         db 11001110b,11000110b,01111100b,00000000b
         db 11000110b,11000110b,11000110b,11111110b     ; H, 12
         db 11000110b,11000110b,11000110b,00000000b     
         db 00111100b,00011000b,00011000b,00011000b     ; I, 13
         db 00011000b,00011000b,00111100b,00000000b     
         db 00011110b,00001100b,00001100b,00001100b     ; J, 14
         db 00001100b,11001100b,00111100b,00000000b     
         db 11001100b,11011000b,11110000b,11100000b     ; K, 15
         db 11110000b,11011000b,11001100b,00000000b
         db 11000000b,11000000b,11000000b,11000000b     ; L, 16
         db 11000000b,11000000b,11111110b,00000000b
         db 11000110b,11101110b,11111110b,11010110b     ; M, 17
         db 11000110b,11000110b,11000110b,00000000b
         db 11000110b,11100110b,11110110b,11011110b     ; N, 18
         db 11001110b,11000110b,11000110b,00000000b
         db 01111100b,11000110b,11000110b,11000110b     ; O, 19
         db 11000110b,11000110b,01111100b,00000000b
         db 11111100b,11000110b,11000110b,11111100b     ; P, 1A
         db 11000000b,11000000b,11000000b,00000000b
         db 01111100b,11000110b,11000110b,11000110b     ; Q, 1B
         db 11010110b,11001110b,01111110b,00000000b
         db 11111100b,11000110b,11000110b,11111100b     ; R, 1C
         db 11001100b,11000110b,11000110b,00000000b
         db 01111100b,11000110b,11000000b,01111100b     ; S, 1D
         db 00000110b,11000110b,01111100b,00000000b
         db 01111110b,00011000b,00011000b,00011000b     ; T, 1E
         db 00011000b,00011000b,00011000b,00000000b
         db 11000110b,11000110b,11000110b,11000110b     ; U, 1F
         db 11000110b,11000110b,01111100b,00000000b
         db 11000110b,11000110b,11000110b,11000110b     ; V, 20
         db 01101100b,00111000b,00010000b,00000000b
         db 11000110b,11000110b,11000110b,11010110b     ; W, 21
         db 11010110b,11111110b,01101100b,00000000b
         db 11000110b,01101100b,00111000b,00010000b     ; X, 22
         db 00111000b,01101100b,11000110b,00000000b
         db 11001100b,11001100b,01111000b,00110000b     ; Y, 23
         db 00110000b,00110000b,00110000b,00000000b
         db 11111100b,10001100b,00011000b,00110000b     ; Z, 24
         db 01100000b,11000100b,11111100b,00000000b
         db 00000000b,00000000b,00000000b,11111110b     ; -, 25
         db 00000000b,00000000b,00000000b,00000000b
         db 00000000b,00000000b,00000000b,00000000b     ; _, 26
         db 00000000b,00000000b,11111110b,00000000b
         db 01110000b,11011100b,00000110b,00000000b     ; ~, 27
         db 00000000b,00000000b,00000000b,00000000b
         db 00000000b,00000000b,00000000b,00000000b     ; ., 28
         db 00000000b,00110000b,00110000b,00000000b
         db 00000010b,00000100b,00001000b,00010000b     ; /, 29
         db 00100000b,01000000b,10000000b,00000000b
         db 00001100b,00011000b,00110000b,01100000b     ; <, 2A
         db 00110000b,00011000b,00001100b,00000000b
         db 01100000b,00110000b,00011000b,00001100b     ; >, 2B
         db 00011000b,00110000b,01100000b,00000000b
         db 00111000b,00100000b,00100000b,00100000b     ; [, 2C
         db 00100000b,00100000b,00111000b,00000000b
         db 00111000b,00001000b,00001000b,00001000b     ; ], 2D
         db 00001000b,00001000b,00111000b,00000000b
         db 00000000b,00011000b,00011000b,00000000b     ; :, 2E
         db 00011000b,00011000b,00000000b,00000000b
         db 00011000b
         db 00100100b
         db 00011000b
         db 00111010b    ; &, 2F
         db 01000100b
         db 01000110b
         db 00111010b
         db 00000000b
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
    mov byte[esi],al
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
    mov byte[esi],al
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
    mov word[esi],ax
    mov word[esi+75036*4],ax
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

NEWSYM outputchar16b5x52
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
    mov word[esi-288*2],0
    mov word[esi+2-288*2],0
    mov word[esi+4-288*2],0
    mov word[esi+6-288*2],0
    mov word[esi+8-288*2],0
    mov word[esi+10-288*2],0
    mov word[esi-288*2+75036*4],0
    mov word[esi+2-288*2+75036*4],0
    mov word[esi+4-288*2+75036*4],0
    mov word[esi+6-288*2+75036*4],0
    mov word[esi+8-288*2+75036*4],0
    mov word[esi+10-288*2+75036*4],0
    mov cl,5
.loopa
    mov ah,[edi]
    mov ch,5
.loopb
    mov word[esi],0
    mov word[esi+2],0
    mov word[esi+75036*4],0
    mov word[esi+2+75036*4],0
    test ah,80h
    jz .nowrite
    push eax
    mov ax,[textcolor16b]
    mov word[esi],ax
    mov word[esi+75036*4],ax
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
    mov word[esi],0
    mov word[esi+2],0
    mov word[esi+4],0
    mov word[esi+6],0
    mov word[esi+8],0
    mov word[esi+10],0
    mov word[esi+75036*4],0
    mov word[esi+2+75036*4],0
    mov word[esi+4+75036*4],0
    mov word[esi+6+75036*4],0
    mov word[esi+8+75036*4],0
    mov word[esi+10+75036*4],0
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
    mov word[textcolor16b],ax
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
    mov word[textcolor16b],ax
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
    mov word[textcolor16b],ax
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
    mov word[textcolor16b],ax
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

NEWSYM OutputGraphicString16b5x52
    xor eax,eax
.nextstr
    mov al,[edi]
    cmp al,0
    je .nomore
    mov al,[ASCII2Font+eax]
    call outputchar16b5x52
    add esi,12
    inc edi
    jmp .nextstr
.nomore
    ret

NEWSYM OutputGraphicStringb
    cmp byte[cbitmode],1
    je near .do16b
    xor eax,eax
    cmp byte[edi-1],1
    je .dir
    cmp byte[edi-1],2
    je near .drive
    sub esi,8
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
.dir
    sub esi,8
    mov al,'<'
    mov al,[ASCII2Font+eax]
    call outputchar
    add esi,8
.nextstr2
    mov al,[edi]
    cmp al,0
    je .nomore2
    mov al,[ASCII2Font+eax]
    call outputchar
    add esi,8
    inc edi
    jmp .nextstr2
.nomore2
    mov al,'>'
    mov al,[ASCII2Font+eax]
    call outputchar
    ret
.drive
    sub esi,8
    mov al,'['
    mov al,[ASCII2Font+eax]
    call outputchar
    add esi,8
.nextstr3
    mov al,[edi]
    cmp al,0
    je .nomore3
    mov al,[ASCII2Font+eax]
    call outputchar
    add esi,8
    inc edi
    jmp .nextstr3
.nomore3
    mov al,']'
    mov al,[ASCII2Font+eax]
    call outputchar
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
    mov word[textcolor16b],ax
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
    mov word[textcolor16b],ax
.no131
NEWSYM OutputGraphicString16bb
    xor eax,eax
    cmp byte[edi-1],1
    je .dir
    cmp byte[edi-1],2
    je near .drive
    xor eax,eax
    sub esi,16
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
.dir
    sub esi,16
    mov al,'<'
    mov al,[ASCII2Font+eax]
    call outputchar16b
    add esi,16
.nextstr2
    mov al,[edi]
    cmp al,0
    je .nomore2
    mov al,[ASCII2Font+eax]
    call outputchar16b
    add esi,16
    inc edi
    jmp .nextstr2
.nomore2
    mov al,'>'
    mov al,[ASCII2Font+eax]
    call outputchar16b
    ret
.drive
    sub esi,16
    mov al,'['
    mov al,[ASCII2Font+eax]
    call outputchar16b
    add esi,16
.nextstr3
    mov al,[edi]
    cmp al,0
    je .nomore3
    mov al,[ASCII2Font+eax]
    call outputchar16b
    add esi,16
    inc edi
    jmp .nextstr3
.nomore3
    mov al,']'
    mov al,[ASCII2Font+eax]
    call outputchar16b
    ret

;*******************************************************
; Save Select      Allows user to select save state slot
;*******************************************************
; For Save State

SECTION .bss
NEWSYM csounddisable, resb 1
NEWSYM statefileloc,  resd 1
newestfileloc resb 1
newestfiledate resd 1
f3menuen resb 1
PrevPictureVal resb 1
CurPictureVal resb 1
SECTION .text

NEWSYM drawhline
.loop
    mov byte[esi],al
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
    mov byte[esi],al
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

%macro determinenewhelp 2
    mov bl,%1
    mov byte[fnamest+eax],%2
    call DetermineNew
%endmacro

DetermineNew:
    push eax
    push ebx
    mov edx,fnamest+1
    call Open_File
    jc near .nodraw
    mov bx,ax
    mov edx,fnamest+1
    call Get_File_Date
%ifdef __MSDOS__
    shl edx,16
    mov dx,cx
%endif
    push edx
    call Close_File
    pop edx
    pop ebx
    pop eax
    ; date = edx, position = bl
    cmp edx,[newestfiledate]
    jbe .notlatest
    mov [newestfiledate],edx
    mov [newestfileloc],bl
.notlatest
    ret
.nodraw
    pop ebx
    pop eax
    ret

DetermineNewest:
    mov dword[newestfiledate],0
    mov byte[newestfileloc],0

    determinenewhelp 0,'t'
    determinenewhelp 1,'1'
    determinenewhelp 2,'2'
    determinenewhelp 3,'3'
    determinenewhelp 4,'4'
    determinenewhelp 5,'5'
    determinenewhelp 6,'6'
    determinenewhelp 7,'7'
    determinenewhelp 8,'8'
    determinenewhelp 9,'9'
    ret

GetPicture:
    mov cl,[CurPictureVal]
    cmp [PrevPictureVal],cl
    jne .notskip
    ret
.notskip
    mov [PrevPictureVal],cl
    mov edx,PrevPicture
    mov ecx,64*56*2
.loop
    mov byte[edx],0
    inc edx
    dec ecx
    jnz .loop
    mov edx,fnamest+1
    call Open_File
    jc near .nodraw2
    mov bx,ax
    xor cx,cx
    xor dx,dx
    call File_Seek_End
    shl edx,16
    mov dx,ax
    push eax
    sub edx,64*56*2 ;Size of thumbnail
    mov eax,[cur_zst_size]
    cmp edx,eax
    je .draw
    mov eax,[old_zst_size]
    cmp edx,eax
    je .draw
    pop eax
    jmp .nodraw
.draw
    pop eax
    mov ax,dx
    shr edx,16
    mov cx,dx
    mov dx,ax
    call File_Seek
    mov ecx,64*56*2
    mov edx,PrevPicture
    call Read_File
.nodraw
    call Close_File
.nodraw2
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
    mov word[esi],ax
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
    push eax
    push ebx
    mov edx,fnamest+1
    call Open_File
    jc near .nodraw
    mov bx,ax
    call Close_File
    pop ebx
    ; draws a 10x10 filled box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,76+94*288
    add esi,[vidbuffer]
    add esi,eax
    mov ecx,10
    mov al,176
    cmp [newestfileloc],bl
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
    ret
.nodraw
    pop ebx
    pop eax
    ret

NEWSYM drawfillboxsc16b
    push eax
    push ebx
    mov edx,fnamest+1
    call Open_File
    jc near .nodraw
    mov bx,ax
    call Close_File
    pop ebx
    ; draws a 10x10 filled box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,76*2+94*288*2
    add esi,[vidbuffer]
    add esi,eax
    add esi,eax
    mov ecx,10
    mov ax,[saveselect.allgrn]
    cmp [newestfileloc],bl
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
    ret
.nodraw
    pop ebx
    pop eax
    ret

NEWSYM drawbox
    ; draws a box according to position bl and color dl
    xor eax,eax
    mov al,11
    mul bl
    mov esi,75+93*288
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
    mov esi,75*2+93*288*2
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

%macro drawfillboxhelp 2
    mov bl,%1
    mov byte[fnamest+eax],%2
    call drawfillboxsc
%endmacro

%macro drawfillboxhelp16b 2
    mov bl,%1
    mov byte[fnamest+eax],%2
    call drawfillboxsc16b
%endmacro


NEWSYM saveselect
    cmp byte[MovieProcessing],0
    jz .nomovie
    pushad
    call mzt_chdir
    popad
.nomovie    
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
    call saveselectpal
    ; draw a small blue box with a white border
    mov esi,70+70*288
    add esi,[vidbuffer]
    mov ecx,150
    mov al,70
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
    mov eax,[statefileloc]
    mov bl,byte[fnamest+eax]
    push ebx
    call DetermineNewest
    drawfillboxhelp 0,'t'
    drawfillboxhelp 1,'1'
    drawfillboxhelp 2,'2'
    drawfillboxhelp 3,'3'
    drawfillboxhelp 4,'4'
    drawfillboxhelp 5,'5'
    drawfillboxhelp 6,'6'
    drawfillboxhelp 7,'7'
    drawfillboxhelp 8,'8'
    drawfillboxhelp 9,'9'
    pop ebx
    mov eax,[statefileloc]
    mov byte[fnamest+eax],bl

    mov esi,75+73*288
    add esi,[vidbuffer]
    mov edi,.stringa
    call OutputGraphicString
    mov esi,75+83*288
    add esi,[vidbuffer]
    mov edi,.stringb
    call OutputGraphicString
    mov esi,75+108*288
    add esi,[vidbuffer]
    mov edi,.stringc
    call OutputGraphicString
    mov esi,75+118*288
    add esi,[vidbuffer]
    mov edi,.stringd
    call OutputGraphicString
    mov esi,75+128*288
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
    mov ecx,70
    call drawvline
    mov esi,70+139*288
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline
    mov esi,219+70*288
    add esi,[vidbuffer]
    mov ecx,70
    call drawvline
    mov esi,75+93*288
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline
    mov esi,75+104*288
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline
    mov esi,75+94*288
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
    mov esi,78+96*288
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
    mov bl,0
    mov ebx,[statefileloc]
    mov al,byte[fnamest+ebx]
    cmp al,'t'
    jne .noT
    mov bl,0
    jmp .nexter
.noT
    mov bl,al
    sub bl,48
.nexter
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
    call testpressed8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    mov ecx,2500
    call delay
    call testpressed8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    push ebx
    call copyvid
    pop ebx
    mov ecx,2500
    call delay
    call testpressed8b
    test byte[pressed+1],1
    jnz near .esc
    test byte[pressed+28],1
    jnz near .enter
    mov ecx,2500
    call delay
    call testpressed8b
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
    cmp bl,0
    jne .nozero
    mov al,'t'
    jmp .save
.nozero
    add bl,48
    mov al,bl
.save
    mov ebx,[statefileloc]
    mov byte[fnamest+ebx],al
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

    call makepal
    mov byte[f3menuen],0
    mov byte[ForceNonTransp],0
    cmp byte[MovieProcessing],0
    jz .nomovie2
    pushad
    call UpChdir
    popad
.nomovie2    
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
    mov esi,80*2+80*288*2
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
    mov al,70
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
    mov eax,[statefileloc]
    mov bl,byte[fnamest+eax]
    push ebx
    call DetermineNewest
    drawfillboxhelp16b 0,'t'
    drawfillboxhelp16b 1,'1'
    drawfillboxhelp16b 2,'2'
    drawfillboxhelp16b 3,'3'
    drawfillboxhelp16b 4,'4'
    drawfillboxhelp16b 5,'5'
    drawfillboxhelp16b 6,'6'
    drawfillboxhelp16b 7,'7'
    drawfillboxhelp16b 8,'8'
    drawfillboxhelp16b 9,'9'
    pop ebx
    mov eax,[statefileloc]
    mov byte[fnamest+eax],bl

    mov esi,75*2+73*288*2
    add esi,[vidbuffer]
    mov edi,.stringa
    call OutputGraphicString16b
    mov esi,75*2+83*288*2
    add esi,[vidbuffer]
    mov edi,.stringb
    call OutputGraphicString16b
    mov esi,75*2+108*288*2
    add esi,[vidbuffer]
    mov edi,.stringc
    call OutputGraphicString16b
    mov esi,75*2+118*288*2
    add esi,[vidbuffer]
    mov edi,.stringd
    call OutputGraphicString16b
    mov esi,75*2+128*288*2
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
    mov ecx,70
    call drawvline16b
    mov esi,70*2+139*288*2
    add esi,[vidbuffer]
    mov ecx,150
    call drawhline16b
    mov esi,219*2+70*288*2
    add esi,[vidbuffer]
    mov ecx,70
    call drawvline16b
    mov esi,75*2+93*288*2
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline16b
    mov esi,75*2+104*288*2
    add esi,[vidbuffer]
    mov ecx,111
    call drawhline16b
    mov esi,75*2+94*288*2
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
    mov esi,78*2+96*288*2
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
    mov bl,0
    mov ebx,[statefileloc]
    mov al,byte[fnamest+ebx]
    cmp al,'t'
    jne .noT16b
    mov bl,0
    jmp .nexter16b
.noT16b
    mov bl,al
    sub bl,48
.nexter16b
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
    ; wait until esc/enter is pressed

    mov byte[PrevPictureVal],0FFh
.noesc16b
    mov [CurPictureVal],bl
    pushad
    mov eax,[statefileloc]
    mov cl,byte[fnamest+eax]
    push ecx
    cmp bl,0
    jne .nozero16b2
    mov cl,'t'
    jmp .save16b2
.nozero16b2
    mov cl,bl
    add cl,48
.save16b2
    mov byte[fnamest+eax],cl
    call GetPicture
    pop ecx
    mov eax,[statefileloc]
    mov byte[fnamest+eax],cl
    popad

    mov dx,0FFFFh
    call drawbox16b
    mov ecx,2500
    call delay
    call testpressed8b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    mov ecx,2500
    call delay
    call testpressed8b
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
    call testpressed8b
    test byte[pressed+1],1
    jnz near .esc16b
    test byte[pressed+28],1
    jnz near .enter16b
    mov ecx,2500
    call delay
    call testpressed8b
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
    cmp bl,0
    jne .nozero16b
    mov al,'t'
    jmp .save16b
.nozero16b
    add bl,48
    mov al,bl
.save16b
    mov ebx,[statefileloc]
    mov byte[fnamest+ebx],al
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
    call Clear2xSaIBuffer
    cmp byte[MovieProcessing],0
    jz .nomovie3
    pushad
    call UpChdir
    popad
.nomovie3    
    ret

SECTION .data
.stringa db 'PLEASE SELECT',0
.stringb db 'SAVE STATE SLOT',0
.stringc db 'USE CURSOR KEYS',0
.stringd db 'TO MOVE AND',0
.stringe db 'ENTER TO SELECT',0

SECTION .bss
NEWSYM ForceNonTransp, resb 1

SECTION .text

NEWSYM testpressed8b
    test byte[pressed+2],1
    jz .no1
    mov bl,1
.no1
    test byte[pressed+3],1
    jz .no2
    mov bl,2
.no2
    test byte[pressed+4],1
    jz .no3
    mov bl,3
.no3
    test byte[pressed+5],1
    jz .no4
    mov bl,4
.no4
    test byte[pressed+6],1
    jz .no5
    mov bl,5
.no5
    test byte[pressed+7],1
    jz .no6
    mov bl,6
.no6
    test byte[pressed+8],1
    jz .no7
    mov bl,7
.no7
    test byte[pressed+9],1
    jz .no8
    mov bl,8
.no8
    test byte[pressed+10],1
    jz .no9
    mov bl,9
.no9
    test byte[pressed+11],1
    jz .no0
    mov bl,0
.no0
    test byte[pressed+75],1
    jz .noleft
    cmp bl,0
    je .noleft
    dec bl
    mov byte[pressed+75],2
.noleft
    test byte[pressed+77],1
    jz .noright
    cmp bl,9
    je .noright
    inc bl
    mov byte[pressed+77],2
.noright
%ifndef __MSDOS__
%ifdef __LINUX__
    test byte[pressed+92],1
%else
    test byte[pressed+0CBh],1
%endif
    jz .noleft2
    cmp bl,0
    je .noleft2
    dec bl
%ifdef __LINUX__
    mov byte[pressed+92],2
%else
    mov byte[pressed+0CBh],2
%endif
.noleft2
%ifdef __LINUX__
    test byte[pressed+94],1
%else
    test byte[pressed+0CDh],1
%endif
    jz .noright2
    cmp bl,9
    je .noright2
    inc bl
%ifdef __LINUX__
    mov byte[pressed+94],2
%else
    mov byte[pressed+0CDh],2
%endif
.noright2
;.nowin32
%endif
    ret

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
    mov word[cgram+eax],bx
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
blahblahblah resw 1
SECTION .text

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

SECTION .bss
NEWSYM tempco0, resw 1
SECTION .text

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

SECTION .bss
NEWSYM prevbright, resb 1                 ; previous brightness
SECTION .text

;*******************************************************
; CopyVid                       Copies buffer into video
;*******************************************************

NEWSYM showfps
    mov cl,50
    cmp byte[romispal],0
    jne .nontsc
    mov cl,60
.nontsc
    inc byte[curfps]
    cmp byte[nextframe],cl
    jb .nofrc
    mov al,[curfps]
    mov [lastfps],al
    mov al,[curfps2]
    mov [lastfps2],al
    mov byte[curfps],0
    mov byte[curfps2],0
    sub byte[nextframe],cl
.nofrc

    cmp byte[cbitmode],1
    je near .do16b

    call displayfpspal

    mov al,[lastfps]
    mov bl,10
    xor ah,ah
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
    mov al,[lastfps]
    mov bl,10
    xor ah,ah
    div bl
    shl al,4
    add ah,al
    mov al,ah
    mov esi,208*288*2+32*2
    add esi,[vidbuffer]
    push ecx
    call outputhex16

    mov esi,208*288*2+48*2
    add esi,[vidbuffer]
    mov al,29h
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

.ng16b
    mov byte[ngfont],1

    mov al,[lastfps]
    mov bl,10
    xor ah,ah
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

SECTION .bss
NEWSYM spcdebugaddr, resd 1
NEWSYM tempoffset, resw 1

NEWSYM Testval, resd 1
SECTION .text


NEWSYM ClockOutput
    cmp byte[cbitmode],1
    je near .do16b3
.no16b3
    mov byte[ngfont],1
    call displayfpspal
    mov esi,215*288+32+192
    add esi,[vidbuffer]
    mov ebx,7
.loop2
    mov ecx,12
    mov byte[esi-1],0C0h
.loop
    mov dword[esi],0C0C0C0C0h
    add esi,4
    dec ecx
    jnz .loop
    add esi,288-48
    dec ebx
    jnz .loop2
    jmp .do8b
.do16b3
    mov esi,215*288*2+32*2+192*2
    add esi,[vidbuffer]
    mov ebx,7
.loop2b
    mov ecx,24
    mov word[esi-2],0
    mov word[esi-2+75036*4],0
.loopb
    mov dword[esi],0
    mov dword[esi+75036*4],0
    add esi,4
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

NEWSYM ClockOutputB
    cmp byte[cbitmode],1
    je near .do16b3
.no16b3
    mov byte[ngfont],1
    call displayfpspal
    mov esi,208*288+32+192
    add esi,[vidbuffer]
    mov ebx,7
.loop2
    mov ecx,12
    mov byte[esi-1],0C0h
.loop
    mov dword[esi],0C0C0C0C0h
    add esi,4
    dec ecx
    jnz .loop
    add esi,288-48
    dec ebx
    jnz .loop2
    jmp .do8b
.do16b3
    mov esi,208*288*2+32*2+192*2
    add esi,[vidbuffer]
    mov ebx,7
.loop2b
    mov ecx,24
    mov word[esi-2],0
    mov word[esi-2+75036*4],0
.loopb
    mov dword[esi],0
    mov dword[esi+75036*4],0
    add esi,4
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
    mov esi,209*288+32+228
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
    mov esi,209*288+32+210
    call .output
    pop eax
    ; eax = hours
    xor edx,edx
    mov ebx,10
    div ebx
    mov esi,209*288+32+192
    call .output
    cmp byte[cbitmode],1
    je .do16b2
.no16b4
    mov esi,209*288+32+222
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar5x5
    popad
    mov esi,209*288+32+204
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar5x5
    popad
    ret
.do16b2
    mov esi,209*288*2+32*2+222*2
    add esi,[vidbuffer]
    xor eax,eax
    add al,':'
    mov al,[ASCII2Font+eax]
    pushad
    call outputchar16b5x5
    popad
    mov esi,209*288*2+32*2+204*2
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

SECTION .data
hextestfilen db 'DSP1DUMP.DAT',0

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
    cmp byte[pressed+25],1
    jne .nopress25
    jmp .nopress25
    pushad
    mov edx,hextestfilen
    call Create_File
    jc .failed
    mov bx,ax
    mov edx,dsp1array
    mov ecx,4096
    call Write_File
    call Close_File
.failed
    popad
    mov byte[pressed+25],2
.nopress25
    call displayfpspal

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
    EXTSYM Op14Zr,Op14Xr,Op14Yr,Op14U,Op14F,Op14L
    EXTSYM Op02CX,Op02CY,bg1scrolx,bg1scroly
    EXTSYM TValDebug,TValDebug2,curhdma,bg1ptr,bg1objptr,DecompAPtr
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
NEWSYM ngfont,       resb 1
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
    call displayfpspal

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

SECTION .bss
NEWSYM chaton, resb 1
NEWSYM chatstrL, resb 140
NEWSYM chatLpos, resd 1
NEWSYM chatstrR, resb 140
NEWSYM chatstrR2, resb 140
NEWSYM chatstrR3, resb 140
NEWSYM chatstrR4, resb 140
NEWSYM chatstrR5, resb 140
NEWSYM chatRTL, resd 1
NEWSYM chatRTL2, resd 1
NEWSYM chatRTL3, resd 1
NEWSYM chatRTL4, resd 1
NEWSYM chatRTL5, resd 1
NEWSYM chatTL, resd 1

SECTION .data
NEWSYM chatreqtable
    db 0  ,2  ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,0 
    db 'Q','W','E','R','T','Y','U','I','O','P','[',']',13 ,0  ,'A','S'
    db 'D','F','G','H','J','K','L',';',27h,'`',1  ,'\','Z','X','C','V'
    db 'B','N','M',',','.','/',1  ,0  ,0  ,' ',0  ,0  ,0  ,0  ,0  ,0
    ; Shift Key Presses
    db 0  ,2  ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,0 
    db 'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S'
    db 'D','F','G','H','J','K','L',':','"','~',1  ,'|','Z','X','C','V'
    db 'B','N','M','<','>','?',1  ,0  ,0  ,' ',0  ,0  ,0  ,0  ,0  ,0

SECTION .text

; Letters transferred & string cleared when chatstrL[0]!=0 and chaton=0
; Disable all transfers when chaton=1 (except joysticks and upper keyboard
;   in input devices)

NEWSYM ChatType2
    jmp ChatType.skipchatdisp
NEWSYM ChatType
    cmp dword[chatTL],0
    jne near .chatdisplayed
    cmp byte[chatstrL],0
    jne near .processchat
    jmp .skipchatdisp
.chatdisplayed
    dec dword[chatTL]
    cmp dword[chatTL],0
    jne .skipchatdisp
    mov dword[chatstrL],0
.skipchatdisp

    mov eax,[KeyQuickChat]
    cmp byte[chaton],0
    jne .nosendchats
    cmp byte[chatstrL],0
    je .nosendchats
    cmp dword[chatTL],0
    jne .nosendchats
    mov byte[pressed+eax],0
.nosendchats

    cmp byte[pressed+eax],1
    jne .noqchat

    mov dword[chatTL],0
    mov byte[pressed+eax],2
    mov byte[chaton],1
    mov dword[chatLpos],0
    mov byte[chatstrL],'>'
    mov byte[chatstrL+1],'_'
    mov byte[chatstrL+2],0
.noqchat
    jmp .skipall
.processchat
    cmp byte[chaton],0
    je near .skipall
    mov eax,63
.notof
    cmp byte[chatreqtable+eax],1
    jbe near .skipthis
    cmp byte[pressed+eax],1
    jne near .skipthis
    mov byte[pressed+eax],2
    push eax
    cmp byte[pressed+42],1
    je .shift
    cmp byte[pressed+54],1
    je .shift
    mov al,[chatreqtable+eax]
    jmp .noshift
.shift
    mov al,[chatreqtable+eax+64]
.noshift
    cmp al,8
    je .delete
    cmp al,13
    je .enter
    cmp al,2
    je near .cancel
    cmp dword[chatLpos],32+30
    jae near .skipoutput
    mov ebx,[chatLpos]
    mov [chatstrL+ebx+1],al
    mov byte[chatstrL+ebx+2],'_'
    mov byte[chatstrL+ebx+3],0
    inc dword[chatLpos]
    jmp .skipoutput
.delete
    mov byte[pressed+0Eh],2
    cmp dword[chatLpos],0
    je near .skipoutput
    dec dword[chatLpos]
    mov ebx,[chatLpos]
    mov byte[chatstrL+ebx+1],'_'
    mov byte[chatstrL+ebx+2],0
    jmp .skipoutput
.enter
    mov byte[pressed+1Ch],2
    mov byte[chaton],0
    cmp dword[chatLpos],0
    jne .noskipoutput
    mov byte[chatstrL],0
    jmp .skipoutput
.noskipoutput
    mov ebx,[chatLpos]
    mov byte[chatstrL+ebx+1],0
    pushad
    mov dl,'L'
%ifndef __MSDOS__
    cmp dword[chatstrL+1],'/ME '
    je .action
%endif
;.dos
    mov esi,ChatNick
    call WritetochatBuffer
    mov esi,chatstrL
    call WritetochatBuffer
    jmp .noaction
.action
    mov dl,'*'
    call NetAddChar
    mov esi,ChatNick
    call WritetochatBuffer
    mov dl,' '
    call NetAddChar
    mov esi,chatstrL+5
    call WritetochatBuffer
.noaction
    mov dl,13
    call NetAddChar
    mov dl,10
    call NetAddChar
    popad
    jmp .skipoutput
.cancel
    mov byte[chatstrL],0
    mov byte[chaton],0
    mov byte[pressed+1],2
.skipoutput
    pop eax
.skipthis
    dec eax
    jns near .notof
.skipall
    ret

%macro RemoteStringPrint 1
    cmp byte[%1],0
    je %%nostringremote
    mov edi,%1
    mov esi,[vidbuffer]
    cmp byte[cbitmode],1
    je %%do16b3
    mov byte[ngfont],1
    add esi,194*288+20
    add esi,[.sdrawptr]
    cmp byte[%1],' '
    jne %%nospace
    inc edi
    add esi,6
%%nospace
    add dword[.sdrawptr],288*6
    call OutputGraphicString5x5.no16bit
    jmp %%nostringremote
%%do16b3
    add esi,194*288*2+20*2
    add esi,[.sdrawptr]
    cmp byte[%1],' '
    jne %%nospaceb
    inc edi
    add esi,6
%%nospaceb
    add dword[.sdrawptr],288*6*2
    call OutputGraphicString16b5x52
%%nostringremote
%endmacro


NEWSYM copyvid
    cmp byte[CNetType],20
    jne .nonet
    call ChatType
.nonet
    mov byte[ngfont],0
    cmp byte[chatstrL],0
    je .nostringlocal
    mov eax,chatstrL
    call StringLength
    sub ecx,42
    cmp ecx,0
    jge .notover
    xor ecx,ecx
.notover
    mov edi,chatstrL
    add edi,ecx
    mov esi,[vidbuffer]
    cmp byte[cbitmode],1
    je .do16b2
.no16b2
    mov byte[ngfont],1
    add esi,188*288+20
    call OutputGraphicString5x5.no16bit
    jmp .nostringlocal
.do16b2
    add esi,188*288*2+20*2
    call OutputGraphicString16b5x52
.nostringlocal

    cmp byte[chatstrLt],0
    je .nostringtemp
    mov edi,chatstrLt
    mov esi,[vidbuffer]
    cmp byte[cbitmode],1
    je .do16b2temp
.no16b2temp
    mov byte[ngfont],1
    add esi,182*288+20
    call OutputGraphicString5x5.no16bit
    jmp .nostringtemp
.do16b2temp
    add esi,182*288*2+20*2
    call OutputGraphicString16b5x52
.nostringtemp

    mov dword[.sdrawptr],0
    RemoteStringPrint chatstrR5
    RemoteStringPrint chatstrR4
    RemoteStringPrint chatstrR3
    RemoteStringPrint chatstrR2
    RemoteStringPrint chatstrR

    cmp dword[chatRTL],0
    je .nortl
    dec dword[chatRTL]
    cmp dword[chatRTL],0
    jne .nortl
    mov dword[chatstrR],0
.nortl
    cmp dword[chatRTL2],0
    je .nortl2
    dec dword[chatRTL2]
    cmp dword[chatRTL2],0
    jne .nortl2
    mov dword[chatstrR2],0
.nortl2
    cmp dword[chatRTL3],0
    je .nortl3
    dec dword[chatRTL3]
    cmp dword[chatRTL3],0
    jne .nortl3
    mov dword[chatstrR3],0
.nortl3
    cmp dword[chatRTL4],0
    je .nortl4
    dec dword[chatRTL4]
    cmp dword[chatRTL4],0
    jne .nortl4
    mov dword[chatstrR4],0
.nortl4
    cmp dword[chatRTL5],0
    je .nortl5
    dec dword[chatRTL5]
    cmp dword[chatRTL5],0
    jne .nortl5
    mov dword[chatstrR5],0
.nortl5

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
    mov byte[ngfont],1
    mov edi,[Msgptr]
    mov esi,200*288+32
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
    mov esi,208*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
    mov edi,CSStatus3
    mov esi,216*288+32
    add esi,[vidbuffer]
    call OutputGraphicString5x5
    jmp .nfivex5b
.smallmsgtext
    call OutputGraphicString5x5
.nfivex5b
    dec dword[MessageOn]
    jnz .nomsg
    cmp byte[cbitmode],1
    je .nomsg
    call makepal
    jmp .nomsg
.do16b
    mov edi,[Msgptr]
    mov esi,200*288*2+32*2
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
    mov esi,208*288*2+32*2
    add esi,[vidbuffer]
    call OutputGraphicString16b5x5
    mov edi,CSStatus3
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
    cmp byte[vsyncon],0
    je .novsync
    cmp byte[Triplebufen],0
    jne .novsync
    cmp byte[curblank],0h
    jne .novsync
%ifdef __MSDOS__
    call waitvsync
%endif
.novsync
    cmp byte[cbitmode],1
    je .nopal
    cmp byte[curblank],0
    jne .nopal
    call changepal
.nopal
    cmp byte[FPSOn],0
    je .nofps
    cmp byte[curblank],0
    jne .nofps
    call showfps
.nofps
;    call ClockOutputB

    cmp byte[TimerEnable],0
    je .noclock
    cmp byte[ShowTimer],0
    je .noclock
    call ClockOutput
.noclock
    cmp byte[snesmouse],3
    je near .drawss
;    cmp byte[snesmouse],4
;    je near .drawss
.returnfromdraw
    mov ax,[resolutn]
    cmp [prevresolutn],ax
    je .noclear
    mov [prevresolutn],ax
    call ClearScreen
.noclear
    jmp DrawScreen

.drawss
    cmp byte[cbitmode],1
    je .noss8b
    call superscopepal
.noss8b
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
SECTION .text


NEWSYM Clear2xSaIBuffer
    mov ebx,[vidbufferofsb]
    add ebx,288*2
    mov ecx,144*239
.nextb
    mov dword[ebx],0FFFFFFFFh
    add ebx,4
    dec ecx
    jnz .nextb
    ret

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


