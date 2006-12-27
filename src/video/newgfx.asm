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
EXTSYM bgwinchange,res480,drawtileng2b,drawtileng4b,drawtileng8b,drawmode7win
EXTSYM drawtileng16x162b,drawtileng16x164b,drawtileng16x168b
EXTSYM osm2dis,drawlineng2b,drawlineng4b,drawlineng8b,processmode7hires
EXTSYM drawlineng16x162b,drawlineng16x164b,drawlineng16x168b,winboundary
EXTSYM winbg1enval,winbg2enval,winbg3enval,winbg4enval,winbgobjenval
EXTSYM winlogicaval,disableeffects,winenabs,scanlines,winl1,winbg1en,winobjen
EXTSYM winlogica,winenabm,bgallchange,bg1change,bg2change,bg3change,bg4change
EXTSYM hiresstuff,drawlineng16x84b,drawlineng16x82b,drawlinengom4b,WindowRedraw
EXTSYM winlogicb,ngwinptr,objwlrpos,objwen,objclineptr,CSprWinPtr
EXTSYM ofsmtptrs,ofsmcptr2,drawmode7ngextbg,drawmode7ngextbg2

%include "video/vidmacro.mac"
%include "video/newgfx2.mac"
%include "video/newgfx.mac"

; vidbufferofsmos pointer to mosaic buffer

%macro WinBGCheck 1
    mov bl,[winbg1en+%1]
    test bl,0Ah
    jz %%disable
    test byte[scrnon],1 << %1
    jnz %%nbgen
    test byte[winenabs],1 << %1
    jnz %%bg
    jmp %%disable
%%nbgen
    test byte[winenabm],1 << %1
    jnz %%bg
%%disable
    xor bl,bl
    jmp %%skip
%%bg
    mov cl,bl
    and cl,0Ah
    cmp cl,0Ah
    je %%skip
;    jne %%notskip
;    and bl,03h
;    and cl,03h
;%%notskip
    mov ch,bl
    mov edx,[winl1]
    cmp cl,02h
    je %%bg1
    shr ch,2
    shr edx,16
%%bg1
    test ch,01h
    jnz %%outside
    cmp dl,dh
    jbe %%skip
    xor bl,bl
    jmp %%skip
%%outside
    cmp dl,0
    ja %%skip
    cmp dh,255
    jb %%skip
    xor bl,bl
%%skip
    mov [winbg1enval+eax+%1*256],bl
%endmacro

SECTION .text

NEWSYM newengine8b
    ; store line by line data
    ; BGMode, BGxScrollX, BGxScrollY, both BGPtrs
    mov eax,[curypos]
    and eax,0FFh

    mov byte[bgallchange+eax],0
    mov byte[bg1change+eax],0
    mov byte[bg2change+eax],0
    mov byte[bg3change+eax],0
    mov byte[bg4change+eax],0

    ; BG3 Priority
    mov ebx,[bg3highst]
    mov [BG3PRI+eax],ebx
    ; Main/Sub Screen
    mov ebx,[scrnon]
    or [bgcmsung],ebx
    mov [BGMS1+eax*2],ebx
    cmp [BGMS1+eax*2-2],bx
    je .nosbgms1
    mov byte[bgallchange+eax],1
.nosbgms1

    ; Scroll Values
    mov ebx,[bg1scrolx]
    mov [BG1SXl+eax*2],ebx
    cmp [BG1SXl+eax*2-2],bx
    je .nosbgx1
    mov byte[bg1change+eax],1
.nosbgx1
    mov ebx,[bg2scrolx]
    mov [BG2SXl+eax*2],ebx
    cmp [BG2SXl+eax*2-2],bx
    je .nosbgx2
    mov byte[bg2change+eax],1
.nosbgx2
    mov ebx,[bg3scrolx]
    mov [BG3SXl+eax*2],ebx
    cmp [BG3SXl+eax*2-2],bx
    je .nosbgx3
    mov byte[bg3change+eax],1
.nosbgx3
    mov ebx,[bg4scrolx]
    mov [BG4SXl+eax*2],ebx
    cmp [BG4SXl+eax*2-2],bx
    je .nosbgx4
    mov byte[bg4change+eax],1
.nosbgx4

    mov ebx,[bg1scroly]
    mov [BG1SYl+eax*2],ebx
    cmp [BG1SYl+eax*2-2],bx
    je .nosbgy1
    mov byte[bg1change+eax],1
.nosbgy1
    mov ebx,[bg2scroly]
    mov [BG2SYl+eax*2],ebx
    cmp [BG2SYl+eax*2-2],bx
    je .nosbgy2
    mov byte[bg2change+eax],1
.nosbgy2
    mov ebx,[bg3scroly]
    mov [BG3SYl+eax*2],ebx
    cmp [BG3SYl+eax*2-2],bx
    je .nosbgy3
    mov byte[bg3change+eax],1
.nosbgy3
    mov ebx,[bg4scroly]
    mov [BG4SYl+eax*2],ebx
    cmp [BG4SYl+eax*2-2],bx
    je .nosbgy4
    mov byte[bg1change+eax],1
.nosbgy4

    ; Background Mode
    mov bl,[bgmode]
    and ebx,07h
    mov [BGMA+eax],bl
    cmp bl,4
    je .changedmode4
    cmp [BGMA+eax-1],bl
    je .nobgma
.changedmode4
    mov byte[bgallchange+eax],1
.nobgma

    ; new graphics fix, thanks to TRAC
    and ebx,07h
    mov byte[modeused+ebx],1

    ; Pointer to OBJ tile data
    mov ebx,[bg1objptr]
    mov [BGOPT1+eax*2],ebx
    cmp [BGOPT1+eax*2-2],bx
    je .nosbgo1
    mov byte[bg1change+eax],1
.nosbgo1
    mov ebx,[bg2objptr]
    mov [BGOPT2+eax*2],ebx
    cmp [BGOPT2+eax*2-2],bx
    je .nosbgo2
    mov byte[bg2change+eax],1
.nosbgo2
    mov ebx,[bg3objptr]
    mov [BGOPT3+eax*2],ebx
    cmp [BGOPT3+eax*2-2],bx
    je .nosbgo3
    mov byte[bg3change+eax],1
.nosbgo3
    mov ebx,[bg4objptr]
    mov [BGOPT4+eax*2],ebx
    cmp [BGOPT4+eax*2-2],bx
    je .nosbgo4
    mov byte[bg4change+eax],1
.nosbgo4

    ; Pointer to tile status data
    mov ebx,[bg1ptr]
    mov [BGPT1+eax*2],ebx
    cmp [BGPT1+eax*2-2],bx
    je .nosbgp1
    mov byte[bg1change+eax],1
.nosbgp1
    mov ebx,[bg2ptr]
    mov [BGPT2+eax*2],ebx
    cmp [BGPT2+eax*2-2],bx
    je .nosbgp2
    mov byte[bg2change+eax],1
.nosbgp2
    mov ebx,[bg3ptr]
    mov [BGPT3+eax*2],ebx
    cmp [BGPT3+eax*2-2],bx
    je .nosbgp3
    mov byte[bg3change+eax],1
.nosbgp3
    mov ebx,[bg4ptr]
    mov [BGPT4+eax*2],ebx
    cmp [BGPT4+eax*2-2],bx
    je .nosbgp4
    mov byte[bg4change+eax],1
.nosbgp4

    mov ebx,[bg1ptrx]
    mov [BGPT1X+eax*2],ebx
    cmp [BGPT1X+eax*2-2],bx
    je .nosbgpx1
    mov byte[bg1change+eax],1
.nosbgpx1
    mov ebx,[bg2ptrx]
    mov [BGPT2X+eax*2],ebx
    cmp [BGPT2X+eax*2-2],bx
    je .nosbgpx2
    mov byte[bg2change+eax],1
.nosbgpx2
    mov ebx,[bg3ptrx]
    mov [BGPT3X+eax*2],ebx
    cmp [BGPT3X+eax*2-2],bx
    je .nosbgpx3
    mov byte[bg3change+eax],1
.nosbgpx3
    mov ebx,[bg4ptrx]
    mov [BGPT4X+eax*2],ebx
    cmp [BGPT4X+eax*2-2],bx
    je .nosbgpx4
    mov byte[bg4change+eax],1
.nosbgpx4

    mov ebx,[bg1ptry]
    mov [BGPT1Y+eax*2],ebx
    cmp [BGPT1Y+eax*2-2],bx
    je .nosbgpy1
    mov byte[bg1change+eax],1
.nosbgpy1
    mov ebx,[bg2ptry]
    mov [BGPT2Y+eax*2],ebx
    cmp [BGPT2Y+eax*2-2],bx
    je .nosbgpy2
    mov byte[bg2change+eax],1
.nosbgpy2
    mov ebx,[bg3ptry]
    mov [BGPT3Y+eax*2],ebx
    cmp [BGPT3Y+eax*2-2],bx
    je .nosbgpy3
    mov byte[bg3change+eax],1
.nosbgpy3
    mov ebx,[bg4ptry]
    mov [BGPT4Y+eax*2],ebx
    cmp [BGPT4Y+eax*2-2],bx
    je .nosbgpy4
    mov byte[bg4change+eax],1
.nosbgpy4
    mov ebx,[forceblnk]
    or bl,bl
    jne .dontdraw
    mov dword[scfbl],0
.dontdraw
    ; Variable size write error fix [TRAC]
    mov [BGFB+eax],bl
    cmp [BGFB+eax-1],bl
    je .nosbgfb
    mov byte[bgallchange+eax],1
.nosbgfb

    mov ebx,[mode7A]
    mov [mode7ab+eax*4],ebx
    mov ebx,[mode7C]
    mov [mode7cd+eax*4],ebx
    mov ebx,[mode7X0]
    mov [mode7xy+eax*4],ebx
    mov ebx,[mode7set]
    mov [mode7st+eax],ebx

    ; 16x16 tiles
    mov ebx,[BG116x16t]
    mov [t16x161+eax],ebx
    cmp [t16x161+eax-1],bl
    je .not16x161
    mov byte[bg1change+eax],1
.not16x161
    mov ebx,[BG216x16t]
    mov [t16x162+eax],ebx
    cmp [t16x162+eax-1],bl
    je .not16x162
    mov byte[bg2change+eax],1
.not16x162
    mov ebx,[BG316x16t]
    mov [t16x163+eax],ebx
    cmp [t16x163+eax-1],bl
    je .not16x163
    mov byte[bg3change+eax],1
.not16x163
    mov ebx,[BG416x16t]
    mov [t16x164+eax],ebx
    cmp [t16x164+eax-1],bl
    je .not16x164
    mov byte[bg4change+eax],1
.not16x164

;    mov byte[mode7hr+eax],0
    cmp byte[bgmode],7
    jne .noextbg
;    cmp byte[res640],0
;    je .nomode7512
;    mov byte[mode7hr+eax],1
;.nomode7512
    test byte[interlval],40h
    jz .noextbg
    mov byte[ngextbg],1
.noextbg

    ; mosaic
    mov ebx,[mosaicon]
    mov [mosenng+eax],ebx
    mov ebx,[mosaicsz]
    mov [mosszng+eax],ebx

    ; Interlaced
    mov ebx,[interlval]
    mov [intrlng+eax],ebx

; Windowing Stuff
;NEWSYM winl1,      0             ; window 1 left position
;NEWSYM winr1,      0             ; window 1 right position
;NEWSYM winl2,      0             ; window 2 left position
;NEWSYM winr2,      0             ; window 2 right position
;NEWSYM winbg1en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG1
;NEWSYM winbg2en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG2
;NEWSYM winbg3en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG3
;NEWSYM winbg4en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG4
;NEWSYM winobjen,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on sprites
;NEWSYM wincolen,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on backarea
;NEWSYM winlogica,  0             ; Window logic type for BG1 to 4
;NEWSYM winlogicb,  0             ; Window logic type for Sprites and Backarea
;NEWSYM winenabm,   0             ; Window logic enable for main screen
;NEWSYM winenabs,   0             ; Window logic enable for sub screen

    mov byte[bgwinchange+eax],0
    cmp byte[disableeffects],1
    je near .finishwin
    push ecx
    push edx
    WinBGCheck 0
    WinBGCheck 1
    WinBGCheck 2
    WinBGCheck 3
    WinBGCheck 4

    mov ebx,[winlogica]
    mov [winlogicaval+eax*2],ebx
    cmp [winlogicaval+eax*2-2],bx
    je .winnchangedb
    mov byte[bgwinchange+eax],1
.winnchangedb
    mov ebx,[winl1]
    mov [winboundary+eax*4],ebx
    cmp [winboundary+eax*4-4],ebx
    je .winnchanged
    mov byte[bgwinchange+eax],1
.winnchanged

    ; generate sprite window
    cmp byte[winbg1enval+eax+4*256],0
    je near .skipobjw

    mov ebx,[winl1]
    mov dl,[winobjen]
    mov dh,[winlogicb]
    and dh,03h
    ; Same as previous line?
    cmp dword[objwlrpos+eax*4-4],0FFFFFFFFh
    je .changed
    cmp [objwlrpos+eax*4-4],ebx
    jne .changedb
    cmp [objwen+eax*2-2],dx
    je near .notchanged
.changedb
    cmp [objwlrpos+eax*4],ebx
    jne .changed
    cmp [objwen+eax*2],dx
    jne .changed
    mov ecx,[CSprWinPtr]
    cmp [objclineptr+eax*4],ecx
    ja near .usecurrent
.changed
    mov [objwlrpos+eax*4],ebx
    mov [objwen+eax*2],dx

    mov bl,[winlogicb]
    and bl,03h
    mov [nglogicval],bl
    mov ebx,4*256
    add ebx,eax
    call BuildWindow
    mov ecx,[CSprWinPtr]
    add ecx,260
    mov [CSprWinPtr],ecx
    mov [objclineptr+eax*4],ecx
    add ecx,[ngwinptr]
    ; Construct Window in ecx
    push eax
    mov ebx,ngwintable
    dec ecx
    mov eax,256
.procnotempty
    mov edx,[ebx]
    add ebx,4
    or edx,edx
    jz .procempty
    dec edx
.swloop
    mov dword[ecx],0
    add ecx,4
    sub eax,4
    jc .done
    sub edx,4
    jnc .swloop
    sub eax,edx
    add ecx,edx
    dec eax
    inc ecx
.procempty
    mov edx,[ebx]
    dec edx
    add ebx,4
.swloop2
    mov dword[ecx],01010101h
    add ecx,4
    sub eax,4
    jc .done
    sub edx,4
    jnc .swloop2
    sub eax,edx
    add ecx,edx
    dec eax
    inc ecx
    jmp .procnotempty
.done
    pop eax
    jmp .skipobjw
.usecurrent
    mov ecx,[objclineptr+eax*4]
    mov [CSprWinPtr],ecx
    jmp .skipobjw
    ; copy over if it's the same
.notchanged
    mov [objwlrpos+eax*4],ebx
    mov [objwen+eax*2],dx
    mov ebx,[objclineptr+eax*4-4]
    mov [objclineptr+eax*4],ebx
.skipobjw
    pop edx
    pop ecx
    jmp .okaywin
.finishwin
    mov byte[winbg1enval+eax],0
    mov byte[winbg2enval+eax],0
    mov byte[winbg3enval+eax],0
    mov byte[winbg4enval+eax],0
    mov byte[winbgobjenval+eax],0
.okaywin

;    cmp byte[Mode7HiRes],1
;    jne .nomode7hires
;    cmp byte[bgmode],7
;    je .hires
.nomode7hires
    cmp byte[res640],0
    je near .no512
    cmp byte[bgmode],5
    jb .no512
    cmp byte[Mode7HiRes],1
    je .hires
    cmp byte[bgmode],7
    je .no512
.hires
    mov byte[hiresstuff],1
    push edi
    push eax
    push ecx
    mov ebx,eax
    mov edi,[vidbuffer]
    mov eax,ebx
    add edi,16+75036
    shl eax,8
    add edi,eax
    mov eax,ebx
    shl eax,5
    add edi,eax
    xor eax,eax
    mov ecx,64
    rep stosd
    pop ecx
    pop eax
    pop edi
.no512
    mov ebx,[sprleftpr+eax*4]
    cmp ebx,00000001h
    je .single
    cmp ebx,00000100h
    je .single
    cmp ebx,00010000h
    je .single
    cmp ebx,01000000h
    je .single
    or [sprprdrn],ebx
    xor ebx,ebx
    ret
.single
    or [sprprdrn],ebx
    or dword[sprleftpr+eax*4],80000000h
    xor ebx,ebx
    ret

%macro Process1DualWindow 0
    test ch,1
    jnz %%outside
    inc ebx
    cmp edx,ebx
    jb %%noclip
    ; Process Inside window
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

NEWSYM BuildWindow2
    jmp BuildWindow.ns2
NEWSYM BuildWindow
    cmp byte[WindowRedraw],1
    je .ns2
    mov dword[valtemp],0EE00h
    push edx
    push ecx
    mov edx,[winlogicaval+eax*2]
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
    jmp .ns
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
    inc ebx
    cmp edx,ebx
    jb .noclip
    ; Process Inside window
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
    sub edx,ebx
    add edx,2
    mov [esi],edx
    mov [esi+64],edx
    mov dword[esi+4],0EE00h
    mov dword[esi+4+64],0EE00h
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

SECTION .data
NEWSYM firstdrawn, db 0

NEWSYM bgusedng
         dd 01010101h,00010101h,00000101h,00000101h,00000101h,00000101h
         dd 00000001h,00000001h

SECTION .bss
NEWSYM bgcmsung, resd 1
NEWSYM modeused, resd 2
NEWSYM reslbyl,  resd 1
NEWSYM sprprdrn, resd 1
NEWSYM csprival, resd 1
NEWSYM pesimpng2, resd 1
NEWSYM cfieldad, resd 1
NEWSYM ignor512, resd 1
NEWSYM ofsmcptr, resd 1
NEWSYM ofsmtptr, resd 1
NEWSYM ofsmmptr, resd 1
NEWSYM ofsmcyps, resd 1
NEWSYM ofsmady,  resd 1
NEWSYM ofsmadx,  resd 1
NEWSYM mosoldtab, resd 15

SECTION .data
ALIGN32

NEWSYM ngwintable, times 16 dd 0EE00h
NEWSYM ngwintableb, times 16 dd 0EE00h
NEWSYM ngwintablec, times 16 dd 0EE00h
NEWSYM ngwintabled, times 16 dd 0EE00h
NEWSYM valtemp, dd 0EE00h, 0EE00h
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
NEWSYM cbgval, resd 1
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
NEWSYM pnglogicval, resd 1
NEWSYM mosjmptab, resd 15
NEWSYM Mode7HiRes, resd 1
NEWSYM pesimpng, resd 1
NEWSYM bgtxadd2, resd 1
SECTION .text

NEWSYM StartDrawNewGfx
    mov byte[WindowRedraw],1
    mov dword[ignor512],1
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
    cmp byte[cbitmode],1
    je near StartDrawNewGfx16b
    push edx
    push esi
    push edi
    push ebp

    ; Clear video memory
    mov edi,[vidbuffer]
    xor eax,eax
    add edi,16
    mov dl,[resolutn]
.loopa
    mov ecx,64
    rep stosd
    add edi,32
    dec dl
    jnz .loopa

    cmp dword[scfbl],0
    jne near .dontdraw

    ; Sprite Layering :
    ; Mode 0/1 - BG4/BG3pr0 * BG4/BG3pr1 * BG2/BG1pr0 * BG2/BG1pr0 * BG3pr1
    ; Mode 2-6 - BG2pr0 * BG1pr0 * BG2pr1 * BG1pr1 *
    ; Mode 7 - * BG1pr0 * BG1pr1 * *

    ; Copy data to sprite table
    mov ecx,256
    mov eax,[spritetablea]
    mov ebx,sprlefttot
    mov edi,sprtbng
.loop
    mov [edi],eax
    add eax,512
    inc ebx
    add edi,4
    dec ecx
    jnz .loop

    xor eax,eax
    mov edi,sprtlng
    mov ecx,64
    rep stosd

    mov byte[firstdrawn],1
    mov dword[bg1totng],0
    mov dword[bg2totng],0
    mov dword[bg3totng],0
    mov dword[bg4totng],0
    mov dword[bg1drwng],0
    mov dword[bg2drwng],0
    mov dword[bg3drwng],0
    mov dword[bg4drwng],0

    ; Draw SubScreens
    test byte[scrndis],8h
    jnz near .nodobg4s
    test dword[bgcmsung],800h
    jz near .nodobg4s
    mov dword[mode0ads],60606060h
    Procbgpr0 3, drawbg4line, drawbg4tile, 800h, 8h
.nodobg4s
    test byte[scrndis],4h
    jnz near .nodobg3s
    test dword[bgcmsung],400h
    jz near .nodobg3s
    mov dword[mode0ads],40404040h
    Procbg3pr0 2, drawbg3line, drawbg3tile, 400h, 4h
.nodobg3s

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprpra
    test dword[modeused],00000101h
    jz near .nosprpra
    test dword[bgcmsung],1000h
    jz near .nosprpra
    Procsprng01 1000h, 10h
.nosprpra

    test byte[scrndis],8h
    jnz near .nodobg4sb
    test dword[bgcmsung],800h
    jz near .nodobg4sb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4sb
    mov dword[mode0ads],60606060h
    Procbgpr1 3, drawbg4linepr1, drawbg4tilepr1, prdata, 800h, 8h
.nodobg4sb
    test byte[scrndis],4h
    jnz near .nodobg3sb
    test dword[bgcmsung],400h
    jz near .nodobg3sb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3sb
    mov dword[mode0ads],40404040h
    Procbg3pr1 2, drawbg3linepr1, drawbg3tilepr1, prdatc, 400h, 4h
.nodobg3sb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprb
    test dword[modeused],00000101h
    jz near .nosprprb
    test dword[bgcmsung],1000h
    jz near .nosprprb
    Procsprng01 1000h, 10h
.nosprprb

    test byte[scrndis],2h
    jnz near .nodobg2s
    test dword[bgcmsung],200h
    jz near .nodobg2s
    mov dword[mode0ads],20202020h
    Procbgpr0 1, drawbg2line, drawbg2tile, 200h, 2h
.nodobg2s

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgsc
    test dword[bgcmsung],300h
    jz near .noextbgsc
    ProcMode7ngextbg 300h, 3h
.noextbgsc

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprc
    test dword[modeused],01010000h
    jnz near .yessprprc
    test dword[modeused+4],01010101h
    jz near .nosprprc
.yessprprc
    test dword[bgcmsung],1000h
    jz near .nosprprc
    Procsprng234567 1000h, 10h
.nosprprc

    test byte[scrndis],1h
    jnz near .nodobg1s
    test dword[bgcmsung],100h
    jz near .nodobg1s
    mov dword[mode0ads],00000000h
    Procbgpr0 0, drawbg1line, drawbg1tile, 100h, 1h
.nodobg1s

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7
    test dword[bgcmsung],300h
    jz near .nomode7
    ProcMode7ng 100h, 1h
.nomode7

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprd
    test dword[bgcmsung],1000h
    jz near .nosprprd
    Procsprng 1000h, 10h
.nosprprd

    test byte[scrndis],2h
    jnz near .nodobg2sb
    test dword[bgcmsung],200h
    jz near .nodobg2sb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2sb
    mov dword[mode0ads],20202020h
    Procbgpr1 1, drawbg2linepr1, drawbg2tilepr1, prdata, 200h, 2h
.nodobg2sb

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgscb
    test dword[bgcmsung],300h
    jz near .noextbgscb
    ProcMode7ngextbg2 200h, 2h
.noextbgscb

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprpre
    test dword[modeused],01010000h
    jnz near .yessprpre
    test dword[modeused+4],01010101h
    jz near .nosprpre
.yessprpre
    test dword[bgcmsung],1000h
    jz near .nosprpre
    Procsprng234567 1000h, 10h
.nosprpre

    test byte[scrndis],1h
    jnz near .nodobg1sb
    test dword[bgcmsung],100h
    jz near .nodobg1sb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1sb
    mov dword[mode0ads],00000000h
    Procbgpr1 0, drawbg1linepr1, drawbg1tilepr1, prdatb, 100h, 1h
.nodobg1sb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprf
    test dword[bgcmsung],1000h
    jz near .nosprprf
    Procsprng 1000h, 10h
.nosprprf

    test byte[scrndis],4h
    jnz near .nodobg3sb2
    cmp byte[modeused+1],0
    je near .nodobg3sb2
    test dword[bgcmsung],400h
    jz near .nodobg3sb2
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3sb2
    mov dword[mode0ads],40404040h
    Procbg3pr1b 2, drawbg3linepr1, drawbg3tilepr1, prdatc, 400h, 4h
.nodobg3sb2

    mov dword[bg1totng],0
    mov dword[bg2totng],0
    mov dword[bg3totng],0
    mov dword[bg4totng],0
    mov dword[bg1drwng],0
    mov dword[bg2drwng],0
    mov dword[bg3drwng],0
    mov dword[bg4drwng],0

    ; Draw MainScreens
    test byte[scrndis],8h
    jnz near .nodobg4m
    test dword[bgcmsung],8h
    jz near .nodobg4m
    mov dword[mode0ads],60606060h
    Procbgpr0 3, drawbg4line, drawbg4tile, 8h, 0
.nodobg4m
    test byte[scrndis],4h
    jnz near .nodobg3m
    test dword[bgcmsung],4h
    jz near .nodobg3m
    mov dword[mode0ads],40404040h
    Procbg3pr0 2, drawbg3line, drawbg3tile, 4h, 0
.nodobg3m

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprma
    test dword[modeused],00000101h
    jz near .nosprprma
    test dword[bgcmsung],10h
    jz near .nosprprma
    Procsprng01 10h, 0
.nosprprma

    test byte[scrndis],8h
    jnz near .nodobg4mb
    test dword[bgcmsung],8h
    jz near .nodobg4mb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4mb
    mov dword[mode0ads],60606060h
    Procbgpr1 3, drawbg4linepr1, drawbg4tilepr1, prdata, 8h, 0
.nodobg4mb
    test byte[scrndis],4h
    jnz near .nodobg3mb
    test dword[bgcmsung],4h
    jz near .nodobg3mb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3mb
    mov dword[mode0ads],40404040h
    Procbg3pr1 2, drawbg3linepr1, drawbg3tilepr1, prdatc, 4h, 0
.nodobg3mb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprmb
    test dword[modeused],00000101h
    jz near .nosprprmb
    test dword[bgcmsung],10h
    jz near .nosprprmb
    Procsprng01 10h, 0
.nosprprmb

    test byte[scrndis],2h
    jnz near .nodobg2m
    test dword[bgcmsung],2h
    jz near .nodobg2m
    mov dword[mode0ads],20202020h
    Procbgpr0 1, drawbg2line, drawbg2tile, 2h, 0
.nodobg2m

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgmn
    test dword[bgcmsung],3h
    jz near .noextbgmn
    ProcMode7ngextbg 3h, 0
.noextbgmn

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprmc
    test dword[modeused],01010000h
    jnz near .yessprprmc
    test dword[modeused+4],01010101h
    jz near .nosprprmc
.yessprprmc
    test dword[bgcmsung],10h
    jz near .nosprprmc
    Procsprng234567 10h, 0h
.nosprprmc

    test byte[scrndis],1h
    jnz near .nodobg1m
    test dword[bgcmsung],1h
    jz near .nodobg1m
    mov dword[mode0ads],00000000h
    Procbgpr0 0, drawbg1line, drawbg1tile, 1h, 0
.nodobg1m

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7m
    test dword[bgcmsung],1h
    jz near .nomode7m
    ProcMode7ng 1h, 0
.nomode7m

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmd
    test dword[bgcmsung],10h
    jz near .nosprprmd
    Procsprng 10h, 0
.nosprprmd

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgmn2
    test dword[bgcmsung],3h
    jz near .noextbgmn2
    ProcMode7ngextbg2 2h, 0
.noextbgmn2

    test byte[scrndis],2h
    jnz near .nodobg2mb
    test dword[bgcmsung],2h
    jz near .nodobg2mb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2mb
    mov dword[mode0ads],20202020h
    Procbgpr1 1, drawbg2linepr1, drawbg2tilepr1, prdata, 2h, 0
.nodobg2mb

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprme
    test dword[modeused],01010000h
    jnz near .yessprprme
    test dword[modeused+4],01010101h
    jz near .nosprprme
.yessprprme
    test dword[bgcmsung],10h
    jz near .nosprprme
    Procsprng234567 10h, 0
.nosprprme

    test byte[scrndis],1h
    jnz near .nodobg1mb
    test dword[bgcmsung],1h
    jz near .nodobg1mb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1mb
    mov dword[mode0ads],00000000h
    Procbgpr1 0, drawbg1linepr1, drawbg1tilepr1, prdatb, 1h, 0
.nodobg1mb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmf
    test dword[bgcmsung],10h
    jz near .nosprprmf
    Procsprng 10h, 0
.nosprprmf

    test byte[scrndis],4h
    jnz near .nodobg3mb2
    cmp byte[modeused+1],0
    je near .nodobg3mb2
    test dword[bgcmsung],4h
    jz near .nodobg3mb2
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3mb2
    mov dword[mode0ads],40404040h
    Procbg3pr1b 2, drawbg3linepr1, drawbg3tilepr1, prdatc, 4h, 0
.nodobg3mb2

.dontdraw
    xor ebx,ebx
    xor ecx,ecx
    pop ebp
    pop edi
    pop esi
    pop edx
    ret

NEWSYM drawbg1tile
    mov byte[prdatb+ebx],1
    drawbgtileng 0,0
    ret

NEWSYM drawbg2tile
    mov byte[prdata+ebx],1
    drawbgtileng 1,1
    ret

NEWSYM drawbg3tile
    mov byte[prdatc+ebx],1
    drawbgtileng 2,2
    ret

NEWSYM drawbg4tile
    mov byte[prdata+ebx],1
    drawbgtileng 3,1
    ret

NEWSYM drawbg1tilepr1
    drawbgtilengpr1 0,0
    ret

NEWSYM drawbg2tilepr1
    drawbgtilengpr1 1,1
    ret

NEWSYM drawbg3tilepr1
    drawbgtilengpr1 2,2
    ret

NEWSYM drawbg4tilepr1
    drawbgtilengpr1 3,1
    ret

NEWSYM drawbg1line
    mov byte[prdatb+ebx],0
    drawbglineng 0,0

NEWSYM drawbg2line
    mov byte[prdata+ebx],0
    drawbglineng 1,1

NEWSYM drawbg3line
    mov byte[prdatc+ebx],0
    drawbglineng 2,2

NEWSYM drawbg4line
    mov byte[prdata+ebx],0
    drawbglineng 3,1

NEWSYM domosaicng
    mov esi,[pesimpng]
    xor eax,eax
    mov edi,xtravbuf+16
    mov al,[curmosaicsz]
    cmp al,16
    ja .notokay
    cmp al,1
    jbe .notokay
    push esi
    jmp [mosjmptab+eax*4-8]
.notokay
    ret

NEWSYM mosdraw2
    mov ecx,128
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mosender 2

NEWSYM mosdraw3
    mov ecx,86
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mosender 3

NEWSYM mosdraw4
    mov ecx,64
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mosender 4

NEWSYM mosdraw5
    mov ecx,52
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mosender 5

NEWSYM mosdraw6
    mov ecx,43
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mosender 6

NEWSYM mosdraw7
    mov ecx,37
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mosender 7

NEWSYM mosdraw8
    mov ecx,32
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mosender 8

NEWSYM mosdraw9
    mov ecx,29
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mosender 9

NEWSYM mosdraw10
    mov ecx,26
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mosender 10

NEWSYM mosdraw11
    mov ecx,24
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mosender 11

NEWSYM mosdraw12
    mov ecx,22
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mov [esi+11],al
    mosender 12

NEWSYM mosdraw13
    mov ecx,20
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mov [esi+11],al
    mov [esi+12],al
    mosender 13

NEWSYM mosdraw14
    mov ecx,19
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mov [esi+11],al
    mov [esi+12],al
    mov [esi+13],al
    mosender 14

NEWSYM mosdraw15
    mov ecx,18
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mov [esi+11],al
    mov [esi+12],al
    mov [esi+13],al
    mov [esi+14],al
    mosender 15

NEWSYM mosdraw16
    mov ecx,16
.next
    mov al,[edi]
    or al,al
    jz .nodraw
    mov [esi],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    mov [esi+8],al
    mov [esi+9],al
    mov [esi+10],al
    mov [esi+11],al
    mov [esi+12],al
    mov [esi+13],al
    mov [esi+14],al
    mov [esi+15],al
    mosender 16

NEWSYM drawbg1linepr1
    drawbglinengpr1 0,0

NEWSYM drawbg2linepr1
    drawbglinengpr1 1,1

NEWSYM drawbg3linepr1
    drawbglinengpr1 2,2

NEWSYM drawbg4linepr1
    drawbglinengpr1 3,1


SECTION .bss
NEWSYM bgtxadd,  resd 1
NEWSYM bgcyval,  resd 1
NEWSYM bgcxval,  resd 1
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
NEWSYM sprcurng, resd 1
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

NEWSYM drawsprng
    cmp byte[winbg1enval+ebx+4*256],0
    jne near drawsprngw
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,ecx
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    pop ebx
    pop esi
    ret
.drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    pop ebx
    pop esi
    ret

NEWSYM drawsprngm7h
    cmp byte[winbg1enval+ebx+4*256],0
    jne near drawsprngm7w
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,ecx
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    pop ebx
    pop esi
    ret
.drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    pop ebx
    pop esi
    ret

SECTION .bss
NEWSYM NGNumSpr, resb 1
SECTION .text

NEWSYM drawsprngw
    mov [NGNumSpr],cl
    mov ecx,[objclineptr+ebx*4]
    add ecx,[ngwinptr]
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawprawb
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawprawb
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    xor ecx,ecx
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    xor edx,edx
    mov dl,[NGNumSpr]
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprbwb
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprbwb
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret

NEWSYM drawsprngm7w
    mov [NGNumSpr],cl
    mov ecx,[objclineptr+ebx*4]
    add ecx,[ngwinptr]
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawprawb
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawprawb
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    xor edx,edx
    mov dl,[NGNumSpr]
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprbwb
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprbwb
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret

;*******************************************************
; Prepare Sprite Priorities
;*******************************************************

NEWSYM makesprprtable
    ret

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
