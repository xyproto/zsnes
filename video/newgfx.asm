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

EXTSYM BG116x16t,BG1SXl,BG1SYl,BG216x16t,BG2SXl,BG2SYl,BG316x16t,BG3PRI,BG3SXl
EXTSYM BG3SYl,BG416x16t,BG4SXl,BG4SYl,BGFB,BGMA,BGMS1,BGOPT1,BGOPT2,BGOPT3
EXTSYM BGOPT4,BGPT1,BGPT1X,BGPT1Y,BGPT2,BGPT2X,BGPT2Y,BGPT3,BGPT3X,BGPT3Y,BGPT4
EXTSYM BGPT4X,BGPT4Y,StartDrawNewGfx16b,bg1objptr,bg1ptr,bg1ptrx,bg1ptry
EXTSYM bg1scrolx,bg1scroly,bg2objptr,bg2ptr,bg2ptrx,bg2ptry,bg2scrolx,bg2scroly
EXTSYM bg3highst,bg3objptr,bg3ptr,bg3ptrx,bg3ptry,bg3scrolx,bg3scroly,bg4objptr
EXTSYM bg4ptr,bg4ptrx,bg4ptry,bg4scrolx,bg4scroly,bgmode,bgtxad,cachesingle2bng
EXTSYM cachesingle8bng,cbitmode,cfield,colormodedef,csprbit,curmosaicsz
EXTSYM curvidoffset,curypos,forceblnk,interlval,intrlng,mode7A,m7starty
EXTSYM mode7C,mode7X0,mode7ab,mode7cd,mode7set,mode7st,mode7xy,mosaicon,mosaicsz
EXTSYM mosenng,mosszng,ngceax,ngcedi,ngpalcon2b,ngpalcon8b,ngptrdat,prdata
EXTSYM prdatb,prdatc,res640,resolutn,scrndis,scrnon,spritetablea,sprleftpr
EXTSYM sprlefttot,sprpriodata,sprtbng,sprtlng,t16x161,t16x162,t16x163,t16x164
EXTSYM tltype2b,tltype8b,vcache2b,vcache8b,vidbuffer,vidmemch2,ngptrdat2
EXTSYM vidmemch8,vram,vrama,winon,xtravbuf,ng16bbgval,ng16bprval,ofshvaladd
EXTSYM bgwinchange,res480
EXTSYM osm2dis
EXTSYM winboundary
EXTSYM winbg1enval,winbg2enval,winbg3enval,winbg4enval,winbgobjenval
EXTSYM winlogicaval,disableeffects,winenabs,scanlines,winl1,winbg1en,winobjen
EXTSYM winlogica,winenabm,bgallchange,bg1change,bg2change,bg3change,bg4change
EXTSYM hiresstuff,WindowRedraw
EXTSYM winlogicb,ngwinptr,objwlrpos,objwen,objclineptr,CSprWinPtr
EXTSYM ofsmtptrs,ofsmcptr2

%macro Process1DualWindow 0
    test ch,1
    jnz %%outside
    cmp edx,ebx
    jb %%noclip
    ; Process Inside window
    inc ebx
    mov [esi],ebx
    sub edx,ebx
    add edx,2
    mov [esi+4],edx
    mov dword[esi+8],0EE00h
    add esi,12
    jmp %%finwin
%%noclip
    mov dword[esi],0EE00h
    add esi,4
    jmp %%finwin
%%outside
    ; Process Outside window
    cmp edx,ebx
    jb %%completeclip
    or ebx,ebx
    jz %%skipclip
    inc ebx
    mov dword[esi],0
    mov [esi+4],ebx
    add esi,8
%%skipclip
    sub edx,ebx
    add edx,2
    mov [esi],edx
    mov dword[esi+4],0EE00h
    add esi,8
    jmp %%finwin
%%completeclip
    mov dword[esi],0
    mov dword[esi+4],0EE00h
    add esi,8
%%finwin
%endmacro

SECTION .text

NEWSYM BuildWindow2
    jmp BuildWindow.ns2
NEWSYM BuildWindow
    cmp byte[WindowRedraw],1
    je .ns2
    push edx
    push ecx
    mov edx,[nglogicval]
    shl edx,16
    mov dl,[winbg1enval+ebx]
    cmp edx,[pwinen]
    jne .notsimilar
    mov edx,[winboundary+eax*4]
    cmp edx,[pwinbound]
    jne .notsimilarb
    mov ecx,ngwintable
.loopc
    mov edx,[ecx+64]
    mov [ecx],edx
    add ecx,4
    cmp edx,0D000h
    ja .finloopc
    cmp ecx,ngwintable+64
    jne .loopc
.finloopc
    mov edx,[pngwinen]
    mov [ngwinen],edx
    pop ecx
    pop edx
    ret
.ns2
    mov byte[WindowRedraw],0
    push edx
    push ecx
    mov edx,[nglogicval]
    shl edx,16
    mov dl,[winbg1enval+ebx]
.notsimilar
    mov [pwinen],edx
    mov edx,[winboundary+eax*4]
.notsimilarb
    mov [pwinbound],edx
.ns
    push esi
    mov cl,[winbg1enval+ebx]
    xor edx,edx
    mov ch,cl
    and cl,0Ah
    mov esi,ngwintable
    cmp cl,0Ah
    je near .dualwin
    xor ebx,ebx
    cmp cl,2
    jne .winb
    mov bl,[winboundary+eax*4]
    mov dl,[winboundary+eax*4+1]
    jmp .wina
.winb
    mov bl,[winboundary+eax*4+2]
    mov dl,[winboundary+eax*4+3]
    shr ch,2
.wina
    test ch,1
    jnz .outside
    cmp edx,ebx
    jb .noclip
    ; Process Inside window
    inc ebx
    mov [esi],ebx
    mov [esi+64],ebx
    sub edx,ebx
    add edx,2
    mov [esi+4],edx
    mov dword[esi+8],0EE00h
    mov [esi+4+64],edx
    mov dword[esi+8+64],0EE00h
    mov dword[ngwinen],1
.noclip
    pop esi
    mov edx,[ngwinen]
    mov [pngwinen],edx
    pop ecx
    pop edx
    ret
.outside
    ; Process Outside window
    cmp edx,ebx
    jb .completeclip
    or ebx,ebx
    jz .skipclip
    inc ebx
    mov dword[esi],0
    mov [esi+4],ebx
    mov dword[esi+64],0
    mov [esi+4+64],ebx
    add esi,8
.skipclip
    cmp edx,255
    jne .clip
    or ebx,ebx
    jz .noclip
    mov dword[esi],0EE00h
    mov dword[esi+64],0EE00h
    jmp .finwin
.clip
    sub edx,ebx
    add edx,2
    mov [esi],edx
    mov [esi+64],edx
    mov dword[esi+4],0EE00h
    mov dword[esi+4+64],0EE00h
.finwin
    mov dword[ngwinen],1
    pop esi
    mov edx,[ngwinen]
    mov [pngwinen],edx
    pop ecx
    pop edx
    ret
.completeclip
    mov dword[esi],0
    mov dword[esi+4],0EE00h
    mov dword[esi+64],0
    mov dword[esi+4+64],0EE00h
    mov dword[ngwinen],1
    pop esi
    mov edx,[ngwinen]
    mov [pngwinen],edx
    pop ecx
    pop edx
    ret
.dualwin
    push ebx
    xor ebx,ebx
    mov dl,[winboundary+eax*4+1]
    mov bl,[winboundary+eax*4]
    Process1DualWindow
    pop ebx
    mov [WinPtrAPos],esi

    mov ch,[winbg1enval+ebx]
    xor ebx,ebx
    xor edx,edx
    mov bl,[winboundary+eax*4+2]
    mov dl,[winboundary+eax*4+3]
    shr ch,2
    Process1DualWindow
    mov [WinPtrBPos],esi

    ; Convert displacement table to cumulative table
    mov esi,ngwintable
    xor ebx,ebx
.loopapos
    add ebx,[esi]
    mov [esi],ebx
    add esi,4
    cmp esi,[WinPtrAPos]
    jne .loopapos
    xor ebx,ebx
.loopbpos
    add ebx,[esi]
    mov [esi],ebx
    add esi,4
    cmp esi,[WinPtrBPos]
    jne .loopbpos

    ; Combine both lists
    push edi
    mov ecx,0101h
    mov edx,ngwintablec
    mov esi,ngwintable
    mov edi,[WinPtrAPos]
    mov dword[edx],0
    mov dword[edx+64],0
    cmp dword[esi],0
    jne .notzeroa
    add esi,4
    inc dword[edx+64]
    neg cl
.notzeroa
    cmp dword[edi],0
    jne .notzerob
    add edi,4
    inc dword[edx+64]
    neg ch
.notzerob
    add edx,4
.loop
    mov ebx,[esi]
    cmp ebx,0EE00h
    jae .edi
.noedi
    cmp ebx,[edi]
    je .equal
    jb .esi
.edi
    mov ebx,[edi]
    cmp ebx,0EE00h
    jae .esib
    mov [edx],ebx
    mov [edx+64],ch
    add edx,4
    add edi,4
    neg ch
    jmp .loop
.esib
    mov ebx,[esi]
    cmp ebx,0EE00h
    jae .fin
.esi
    mov [edx],ebx
    mov [edx+64],cl
    add edx,4
    add esi,4
    neg cl
    jmp .loop
.equal
    mov [edx],ebx
    mov [edx+64],cl
    add [edx+64],ch
    neg cl
    neg ch
    add edx,4
    add esi,4
    add edi,4
    cmp ebx,0EE00h
    jb .loop
    jmp .finb
.fin
    mov dword[edx],0EE00h
    add edx,4
.finb

    mov [WinPtrAPos],edx

;    jmp .c

    ; Convert list to proper on/off format
    mov edi,[nglogicval]
    and edi,3
    shl edi,2
    add edi,OrLogicTable
    mov edx,ngwintablec
    xor ecx,ecx
.loopp
    add cl,[edx+64]
    mov bl,[edi+ecx]
    mov [edx+64],bl
    add edx,4
    cmp edx,[WinPtrAPos]
    jne .loopp

    ; Shorten & Convert back to displacement format
    mov edx,ngwintablec
    mov esi,ngwintable
    mov cl,1
    xor ebx,ebx
.loops
    cmp byte[edx+64],cl
    jne .noadd
    mov edi,[edx]
    sub edi,ebx
    add ebx,edi
    mov [esi],edi
    mov [esi+64],edi
    add esi,4
    xor cl,1
.noadd
    add edx,4
    cmp edx,[WinPtrAPos]
    jne .loops
    mov dword[esi],0EE00h
    mov dword[esi+64],0EE00h

.c

    pop edi
    pop esi
    mov dword[ngwinen],1
    mov dword[pngwinen],1
    pop ecx
    pop edx
    ret

SECTION .bss
NEWSYM bgcmsung, resd 1
NEWSYM modeused, resd 2
NEWSYM reslbyl,  resd 1
NEWSYM csprival, resd 1
NEWSYM cfieldad, resd 1
NEWSYM ofsmcptr, resd 1
NEWSYM ofsmtptr, resd 1
NEWSYM ofsmmptr, resd 1
NEWSYM ofsmcyps, resd 1
NEWSYM ofsmady,  resd 1
NEWSYM ofsmadx,  resd 1

SECTION .data
ALIGN32

NEWSYM ngwintable, times 32 dd 0EE00h
NEWSYM ngwintablec, times 32 dd 0EE00h
NEWSYM ngcwinptr, dd ngwintable

SECTION .bss
NEWSYM ngwinen, resd 1
NEWSYM ngcwinmode, resd 1
NEWSYM ngcpixleft, resd 1
NEWSYM Mode7BackA, resd 1
NEWSYM Mode7BackC, resd 1
NEWSYM Mode7BackX0, resd 1
NEWSYM Mode7BackSet, resd 1
NEWSYM ngextbg, resd 1
NEWSYM ofsmval, resd 1
NEWSYM ofsmvalh, resd 1

SECTION .data
NEWSYM pwinen, dd 0FFFFh
NEWSYM pngwinen, dd 0FFFFh

SECTION .bss
NEWSYM pwinbound, resd 1
NEWSYM WinPtrAPos, resd 1
NEWSYM WinPtrBPos, resd 1

SECTION .data
NEWSYM OrLogicTable, db 0,1,1,0
NEWSYM AndLogicTable, db 0,0,1,0
NEWSYM XorLogicTable, db 0,1,0,0
NEWSYM XNorLogicTable, db 1,0,1,0

SECTION .bss
NEWSYM nglogicval, resd 1
NEWSYM mosjmptab, resd 15
NEWSYM Mode7HiRes, resb 1
NEWSYM pesimpng, resd 1
NEWSYM bgtxadd2, resd 1
SECTION .text

NEWSYM StartDrawNewGfx
    mov byte[WindowRedraw],1
    mov dword[cfieldad],0
    cmp byte[res480],1
    jne .scan2
    cmp byte[scanlines],0
    jne .scan2
    mov al,[cfield]
    mov [cfieldad],al
.scan2
    mov ax,[resolutn]
    sub ax,8
    mov [reslbyl],ax
    jmp StartDrawNewGfx16b

SECTION .bss
NEWSYM bgtxadd,  resd 1
NEWSYM tleftn,   resd 1
NEWSYM tleftnb,  resd 1
NEWSYM bg1totng, resd 1
NEWSYM bg2totng, resd 1
NEWSYM bg3totng, resd 1
NEWSYM bg4totng, resd 1
NEWSYM bg1drwng, resd 1
NEWSYM bg2drwng, resd 1
NEWSYM bg3drwng, resd 1
NEWSYM bg4drwng, resd 1
NEWSYM scfbl,    resd 1
NEWSYM mode0ads, resd 1
NEWSYM mode0add, resd 1
NEWSYM taddnfy16x16, resd 1
NEWSYM taddfy16x16, resd 1
NEWSYM switch16x16, resd 1
NEWSYM yposng,     resd 1
NEWSYM flipyposng, resd 1
NEWSYM yposngom,     resd 1
NEWSYM flipyposngom, resd 1
SECTION .text

SECTION .bss
NEWSYM NGNumSpr, resb 1
SECTION .text

;*******************************************************
; Prepare Sprite Priorities
;*******************************************************

NEWSYM preparesprpr
    xor ebx,ebx
    mov bl,[curypos]
    mov eax,[sprleftpr+ebx*4]
    mov [sprclprio],eax
    cmp eax,00000001h
    je .single
    cmp eax,00000100h
    je .single
    cmp eax,00010000h
    je .single
    cmp eax,01000000h
    je .single
    mov dword[sprsingle],0
    ret
.single
    mov dword[sprsingle],1
    ret

SECTION .bss
NEWSYM sprclprio,  resd 1
NEWSYM sprsingle,  resd 1
SECTION .text
