;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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


; Window Display Routines
; Last button value used = 65

GUIStringGreater:
    ; compares string at esi to edi and returns 1 to al if esi is >, else 0
    push esi
    cmp word[esi],'.'
    je .less
    cmp word[esi+32],'.'
    je .greater
    cmp word[esi],'..'
    je .less
    cmp word[esi+32],'..'
    je .greater
.nextchar
    cmp byte[esi],0
    je .less
    cmp byte[esi+32],0
    je .greater
    mov al,[esi]
    mov cl,[esi+32]
    cmp al,'a'
    jb .noucase1
    cmp al,'z'
    ja .noucase1
    sub al,'z'-'Z'
.noucase1
    cmp cl,'a'
    jb .noucase2
    cmp cl,'z'
    ja .noucase2
    sub cl,'z'-'Z'
.noucase2
    cmp al,cl
    jb .less
    ja .greater
    inc esi
    jmp .nextchar
.less
    mov al,0
    jmp .skip
.greater
    mov al,1
.skip
    pop esi
    ret

GUIStringGreater2:
    ; compares string at ebx to edx and returns 1 to al if esi is >, else 0
    push edx
    push ebx
    cmp word[ebx],'.'
    je .less
    cmp word[edx],'.'
    je .greater
    cmp word[ebx],'..'
    je .less
    cmp word[ebx],'..'
    je .greater
.nextchar
    cmp byte[ebx],0
    je .less
    cmp byte[edx],0
    je .greater
    mov al,[ebx]
    mov cl,[edx]
    cmp al,'a'
    jb .noucase1
    cmp al,'z'
    ja .noucase1
    sub al,'z'-'Z'
.noucase1
    cmp cl,'a'
    jb .noucase2
    cmp cl,'z'
    ja .noucase2
    sub cl,'z'-'Z'
.noucase2
    cmp al,cl
    jb .less
    ja .greater
    inc ebx
    inc edx
    jmp .nextchar
.less
    mov al,0
    jmp .skip
.greater
    mov al,1
.skip
    pop ebx
    pop edx
    ret

%Macro DrawGUILineSc 2
    mov dword[GUIcolscaleval],%2
    mov edx,%1
    push eax
    push ecx
    call GUIHLines
    pop ecx
    pop eax
    inc ebx
%Endmacro

%Macro DrawGUILine 0
    push eax
    push ecx
    call GUIHLine
    pop ecx
    pop eax
    inc ebx
%Endmacro

%macro DrawTitleBar 0
    mov edx,46+157+6
    sub edx,[GUIWincoladd]
    DrawGUILine
    mov edx,42+157+4+4
    sub edx,[GUIWincoladd]
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    dec edx
    DrawGUILine
    mov edx,38+157+4
    sub edx,[GUIWincoladd]
    DrawGUILine
    sub ebx,10
    mov edx,44+157+4
    sub edx,[GUIWincoladd]
    mov esi,9
    push ecx
    mov ecx,eax
%%loop2
    DrawGUILine
    dec esi
    jnz %%loop2
    pop ecx
    sub ebx,8
    mov edx,40+157+4
    sub edx,[GUIWincoladd]
    mov esi,9
    push eax
    mov eax,ecx
%%loop3
    DrawGUILine
    dec esi
    jnz %%loop3
    pop eax
%endmacro

%macro drawshadow2 3
    mov edx,[GUIwinposy+%1*4]
    cmp edx,223
    jg near .noshadow
    cmp edx,0
    jge .noshadclip
    mov edx,0
.noshadclip
    mov ebx,edx
    add ebx,%3
    add ebx,9
    cmp ebx,0
    jl near .noshadow
    cmp ebx,223
    jng .noshadclip2
    mov ebx,223
.noshadclip2
    sub ebx,edx
    inc ebx
    mov edi,edx
    shl edi,8
    mov eax,edx
    shl eax,5
    add edi,eax
    mov edx,[GUIwinposx+%1*4]
    mov ecx,edx
    cmp edx,255
    jg .noshadow
    cmp edx,-3
    jnl .noshadclip3
    mov edx,-3
.noshadclip3
    add ecx,%2
    cmp ecx,0
    jl .noshadow
    cmp ecx,255
    jng .noshadclip4
    mov ecx,255
.noshadclip4
    sub ecx,edx
    inc ecx
    add edi,edx
    add edi,[vidbuffer]
    add edi,288*3+3+16
    call GUIDrawShadow2
.noshadow
%endmacro

%macro GUIDrawWindowBox 2
    mov dword[GUIWincoladd],4
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone
    mov dword[GUIWincol],148+5
.noone
    cmp byte[cwindrawn],0
    jne .nozero
    mov dword[GUIWincoladd],0
    mov dword[GUIWincol],148
.nozero
    drawshadow2 %1,dword[GUIwinsizex+%1*4],dword[GUIwinsizey+%1*4]
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    mov ecx,eax
    add ecx,dword[GUIwinsizex+%1*4]
    DrawTitleBar
    mov esi,dword[GUIwinsizey+%1*4]
    dec esi
    mov edx,dword[GUIWincol]
    add edx,2
.loop
    DrawGUILine
    dec esi
    jnz .loop
    mov edx,dword[GUIWincol]
    DrawGUILine
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    add ebx,10
    mov ecx,eax
    mov esi,dword[GUIwinsizey+%1*4]
    mov edx,dword[GUIWincol]
    add edx,3
    dec esi
.loop2
    DrawGUILine
    dec esi
    jnz .loop2
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    add ebx,10
    add eax,dword[GUIwinsizex+%1*4]
    mov ecx,eax
    mov esi,dword[GUIwinsizey+%1*4]
    mov edx,dword[GUIWincol]
    add edx,1
.loop3
    DrawGUILine
    dec esi
    jnz .loop3
    mov ebx,dword[GUIwinposy+%1*4]
    add ebx,3
    mov edx,dword[GUIwinposx+%1*4]
    add edx,3
    mov byte[GUItextcolor],184
    GUIOuttextwin %2
    mov ebx,dword[GUIwinposy+%1*4]
    add ebx,2
    mov edx,dword[GUIwinposx+%1*4]
    add edx,2
    mov byte[GUItextcolor],220
    cmp byte[GUIWincoladd],0
    je .zero2
    sub byte[GUItextcolor],6
.zero2
    GUIOuttextwin %2
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    add eax,dword[GUIwinsizex+%1*4]
    sub eax,10
    GUIDisplayIcon GUIIconDataClose
%endmacro

%macro DrawGUIWinBox 6
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    add eax,%2
    add ebx,%3
    mov ecx,eax
    add ecx,%4-%2+1
    mov dl,%6
    mov esi,%5-%3+1
%%loop
    DrawGUILine
    dec esi
    jnz %%loop
%endmacro

%macro DrawGUIWinBox2 5
    mov eax,dword[GUIwinposx+%1*4]
    add eax,%2
    mov ecx,eax
    add ecx,%3-%2+1
    mov dl,%5
    cmp byte[GUIWincoladd],0
    je %%zero
    inc dl
%%zero
    mov esi,%4
    add ebx,dword[GUIwinposy+%1*4]
%%loop
    DrawGUILine
    dec esi
    jnz %%loop
%endmacro

%macro DrawGUIWinBox3 6
    mov ecx,%4
    mov eax,dword[GUIwinposx+%1*4]
    mov ebx,dword[GUIwinposy+%1*4]
    add eax,%2
    add ebx,%3
    add ecx,eax
    sub ecx,%2
    inc ecx
    mov dl,%6
    mov esi,%5-%3+1
%%loop
    DrawGUILine
    dec esi
    jnz %%loop
%endmacro

%macro DrawGUIButton 9
    mov dl,byte[GUItextcolor]
    sub dl,5
    cmp byte[GUICBHold],%7
    jne %%noa
    sub dl,13
%%noa
    DrawGUIWinBox %1,%2,%3,%4,%3,dl
    sub dl,3
    cmp byte[GUICBHold],%7
    jne %%nob
    add dl,5
%%nob
    DrawGUIWinBox %1,%2,%3,%2,%5,dl
    sub dl,3
    cmp byte[GUICBHold],%7
    jne %%noc
    add dl,5
%%noc
    DrawGUIWinBox %1,%2+1,%3+1,%4-2,%5-2,dl
    sub dl,3
    cmp byte[GUICBHold],%7
    jne %%nod
    add dl,5
%%nod
    DrawGUIWinBox %1,%4+1,%3+1,%4-2,%5-2,dl
    sub dl,3
    cmp byte[GUICBHold],%7
    jne %%noe
    add dl,5
%%noe
    DrawGUIWinBox %1,%2,%5,%4-1,%5,dl
    cmp byte[GUICBHold],%7
    je near %%no2
    sub byte[GUItextcolor],15
    GUIOuttextwin2 %1,%2+5+%8,%3+4+%9,%6
    add byte[GUItextcolor],15
    GUIOuttextwin2 %1,%2+4+%8,%3+3+%9,%6
    jmp %%yes
%%no2
    sub byte[GUItextcolor],18
    GUIOuttextwin2 %1,%2+6+%8,%3+5+%9,%6
    add byte[GUItextcolor],15
    GUIOuttextwin2 %1,%2+5+%8,%3+4+%9,%6
    add byte[GUItextcolor],3
%%yes
%endmacro

%macro GUIOuttextwin2load 3
    test dword[cloadnleft],80000000h
    jnz %%skip
    cmp byte[GUIloadfntype],0
    jne %%nottype0
    mov eax,[cloadnposb]
    mov eax,[spcRamcmp+eax*4]
    inc eax
    mov [cloadnpos],eax
    inc dword[cloadnposb]
%%nottype0
    GUIOuttextwin2l %1,%2,%3,[cloadnpos]
    add dword[cloadnpos],32
    dec dword[cloadnleft]
%%skip
%endmacro

%macro GUIDisplayButtonHole 5
    mov dword[GUITemp],GUIIconDataButtonHole
    cmp %4,%5
    jne %%nofill
    mov dword[GUITemp],GUIIconDataButtonFill
%%nofill
    GUIDisplayIconWin %1,%2,%3,[GUITemp]
%endmacro

; DGUIDisplay

DisplayGUILoad:
    GUIDrawWindowBox 1,GUILoadDisp
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
;    cmp byte[OSPort],2
;    jae near .noloadtypeb
%ifdef __MSDOS__
    GUIOuttextwin2 1,21,166,GUILoadText3
    GUIOuttextwin2 1,21,174,GUILoadText4
    GUIOuttextwin2 1,21,182,GUILoadText5
%endif
.noloadtypeb
    GUIOuttextwin2 1,21,192,GUILoadText6
    GUIOuttextwin2 1,6,16,GUILoadText7
    GUIOuttextwin2 1,161,16,GUILoadText8
    GUIOuttextwin2 1,6,138,GUIcurrentdir

    GUIOuttextwin2 1,146,172,GUILoadTextB
    GUIOuttextwin2 1,155,182,GUILoadTextC
    GUIOuttextwin2 1,155,192,GUILoadTextD
    GUIOuttextwin2 1,195,182,GUILoadTextE
    GUIOuttextwin2 1,195,192,GUILoadTextF

    mov dword[cloadmaxlen],39
    cmp dword[GUIcurrentfilewin],0
    jne .nofiles
;    cmp byte[OSPort],2
;    jb .nofiles
%ifndef __MSDOS__
    mov eax,[GUIcurrentcursloc]
    mov eax,[spcRamcmp+eax*4]
    inc eax
    mov [cloadnpos],eax
    GUIOuttextwin2l 1,6,158,[cloadnpos]
%endif
.nofiles

    add byte[GUItextcolor],15
;    cmp byte[OSPort],2
;    jae near .noloadtypec
%ifdef __MSDOS__
    GUIOuttextwin2 1,20,165,GUILoadText3
    GUIOuttextwin2 1,20,173,GUILoadText4
    GUIOuttextwin2 1,20,181,GUILoadText5
%endif
.noloadtypec
    GUIOuttextwin2 1,20,191,GUILoadText6
    GUIOuttextwin2 1,5,15,GUILoadText7
    GUIOuttextwin2 1,160,15,GUILoadText8
    GUIOuttextwin2 1,5,137,GUIcurrentdir

    GUIOuttextwin2 1,145,171,GUILoadTextB
    GUIOuttextwin2 1,154,181,GUILoadTextC
    GUIOuttextwin2 1,154,191,GUILoadTextD
    GUIOuttextwin2 1,194,181,GUILoadTextE
    GUIOuttextwin2 1,194,191,GUILoadTextF

    mov dword[cloadmaxlen],39
    cmp dword[GUIcurrentfilewin],0
    jne .nofilesb
;    cmp byte[OSPort],2
;    jb .nofilesb
%ifndef __MSDOS__
    mov eax,[GUIcurrentcursloc]
    mov eax,[spcRamcmp+eax*4]
    inc eax
    mov [cloadnpos],eax
    GUIOuttextwin2l 1,5,157,[cloadnpos]
%endif
.nofilesb

    DrawGUIButton 1,186,165,228,176,GUILoadText9,1,0,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
;    GUIOuttextwin2 1,6,16,GUILoadText1
;    cmp byte[OSPort],2
;    jae near .noloadtypee
%ifdef __MSDOS__
    GUIOuttextwin2 1,6,157,GUILoadText2
%endif
.noloadtypee
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
;    GUIOuttextwin2 1,5,15,GUILoadText1
;    cmp byte[OSPort],2
;    jae near .noloadtyped
%ifdef __MSDOS__
    GUIOuttextwin2 1,5,156,GUILoadText2
%endif
.noloadtyped
    DrawGUIWinBox 1,5,25,144,134,167    ; 126 = 6 * 21, 112 = 7 * 16
    DrawGUIWinBox 1,160,25,228,134,167  ; 78 =  6 * 13
    DrawGUIWinBox 1,5,145,228,152,167    ; 126 = 6 * 21, 112 = 7 * 16

    xor ebx,ebx
    mov bl,[GUILoadPos]
    mov byte[GUILoadTextA+ebx],0
    test byte[GUILDFlash],8
    jnz .nound
    mov byte[GUILoadTextA+ebx],'_'
    mov byte[GUILoadTextA+ebx+1],0
.nound

    mov ebx,[GUIcurrentcursloc]
    sub ebx,[GUIcurrentviewloc]
    cmp dword[GUIcurrentfilewin],0
    je .files
    mov ebx,[GUIcurrentdircursloc]
    sub ebx,[GUIcurrentdirviewloc]
.files

    mov eax,ebx
    lea ebx,[ebx*4]
    add ebx,eax
    add ebx,eax
    add ebx,eax
    add ebx,27
    cmp dword[GUIcurrentfilewin],0
    jne .dir
    DrawGUIWinBox2 1,5,144,7,224
    jmp .files2
.dir
    DrawGUIWinBox2 1,160,228,7,224
.files2

    mov byte[GUItextcolor],223
    GUIOuttextwin2 1,8,148, GUILoadTextA
    mov eax,[spcBuffera]
    inc eax
    mov dword[cloadnpos],eax
    mov eax,[GUIfileentries]
    sub eax,[GUIcurrentviewloc]
    dec eax
    mov dword[cloadnleft],eax
    mov eax,[GUIcurrentviewloc]
    mov [cloadnposb],eax
    shl eax,5
    add [cloadnpos],eax
    mov dword[cloadmaxlen],23

    GUIOuttextwin2load 1,8,29
    GUIOuttextwin2load 1,8,29+7
    GUIOuttextwin2load 1,8,29+7*2
    GUIOuttextwin2load 1,8,29+7*3
    GUIOuttextwin2load 1,8,29+7*4
    GUIOuttextwin2load 1,8,29+7*5
    GUIOuttextwin2load 1,8,29+7*6
    GUIOuttextwin2load 1,8,29+7*7
    GUIOuttextwin2load 1,8,29+7*8
    GUIOuttextwin2load 1,8,29+7*9
    GUIOuttextwin2load 1,8,29+7*10
    GUIOuttextwin2load 1,8,29+7*11
    GUIOuttextwin2load 1,8,29+7*12
    GUIOuttextwin2load 1,8,29+7*13
    GUIOuttextwin2load 1,8,29+7*14

    mov eax,[spcBuffera]
    inc eax
    mov dword[cloadnpos],eax
    mov eax,[GUIdirentries]
    sub eax,[GUIcurrentdirviewloc]
    dec eax
    mov dword[cloadnleft],eax
    mov eax,[GUIcurrentdirviewloc]
    add eax,[GUIfileentries]
    mov [cloadnposb],eax
    shl eax,5
    add [cloadnpos],eax
    mov dword[cloadmaxlen],11
    GUIOuttextwin2load 1,164,29
    GUIOuttextwin2load 1,164,29+7
    GUIOuttextwin2load 1,164,29+7*2
    GUIOuttextwin2load 1,164,29+7*3
    GUIOuttextwin2load 1,164,29+7*4
    GUIOuttextwin2load 1,164,29+7*5
    GUIOuttextwin2load 1,164,29+7*6
    GUIOuttextwin2load 1,164,29+7*7
    GUIOuttextwin2load 1,164,29+7*8
    GUIOuttextwin2load 1,164,29+7*9
    GUIOuttextwin2load 1,164,29+7*10
    GUIOuttextwin2load 1,164,29+7*11
    GUIOuttextwin2load 1,164,29+7*12
    GUIOuttextwin2load 1,164,29+7*13
    GUIOuttextwin2load 1,164,29+7*14

    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    GUIOuttextwin2 1,7,147, GUILoadTextA
    mov eax,[spcBuffera]
    inc eax
    mov dword[cloadnpos],eax
    mov eax,[GUIfileentries]
    sub eax,[GUIcurrentviewloc]
    dec eax
    mov dword[cloadnleft],eax
    mov eax,[GUIcurrentviewloc]
    mov [cloadnposb],eax
    shl eax,5
    add [cloadnpos],eax
    mov dword[cloadmaxlen],23
    GUIOuttextwin2load 1,7,28
    GUIOuttextwin2load 1,7,28+7
    GUIOuttextwin2load 1,7,28+7*2
    GUIOuttextwin2load 1,7,28+7*3
    GUIOuttextwin2load 1,7,28+7*4
    GUIOuttextwin2load 1,7,28+7*5
    GUIOuttextwin2load 1,7,28+7*6
    GUIOuttextwin2load 1,7,28+7*7
    GUIOuttextwin2load 1,7,28+7*8
    GUIOuttextwin2load 1,7,28+7*9
    GUIOuttextwin2load 1,7,28+7*10
    GUIOuttextwin2load 1,7,28+7*11
    GUIOuttextwin2load 1,7,28+7*12
    GUIOuttextwin2load 1,7,28+7*13
    GUIOuttextwin2load 1,7,28+7*14
    mov eax,[spcBuffera]
    inc eax
    mov dword[cloadnpos],eax
    mov eax,[GUIdirentries]
    sub eax,[GUIcurrentdirviewloc]
    dec eax
    mov dword[cloadnleft],eax
    mov eax,[GUIcurrentdirviewloc]
    add eax,[GUIfileentries]
    mov [cloadnposb],eax
    shl eax,5
    add [cloadnpos],eax
    mov dword[cloadmaxlen],11
    GUIOuttextwin2load 1,163,28
    GUIOuttextwin2load 1,163,28+7
    GUIOuttextwin2load 1,163,28+7*2
    GUIOuttextwin2load 1,163,28+7*3
    GUIOuttextwin2load 1,163,28+7*4
    GUIOuttextwin2load 1,163,28+7*5
    GUIOuttextwin2load 1,163,28+7*6
    GUIOuttextwin2load 1,163,28+7*7
    GUIOuttextwin2load 1,163,28+7*8
    GUIOuttextwin2load 1,163,28+7*9
    GUIOuttextwin2load 1,163,28+7*10
    GUIOuttextwin2load 1,163,28+7*11
    GUIOuttextwin2load 1,163,28+7*12
    GUIOuttextwin2load 1,163,28+7*13
    GUIOuttextwin2load 1,163,28+7*14

    xor ebx,ebx
    mov bl,[GUILoadPos]
    mov byte[GUILoadTextA+ebx],0

;    cmp byte[OSPort],2
;    jae near .noloadtype
%ifdef __MSDOS__
    GUIDisplayButtonHole 1,9,163,byte[GUIloadfntype],0
    GUIDisplayButtonHole 1,9,171,byte[GUIloadfntype],1
    GUIDisplayButtonHole 1,9,179,byte[GUIloadfntype],2
%endif
.noloadtype
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[showallext],1
    jne .noshowallext
    mov dword[GUITemp],GUIIconDataCheckBoxC
.noshowallext
    GUIDisplayIconWin 1,10,187,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[ForceROMTiming],1
    jne .noforcentsc
    mov dword[GUITemp],GUIIconDataCheckBoxC
.noforcentsc
    GUIDisplayIconWin 1,144,177,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[ForceROMTiming],2
    jne .noforcepal
    mov dword[GUITemp],GUIIconDataCheckBoxC
.noforcepal
    GUIDisplayIconWin 1,144,187,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[ForceHiLoROM],1
    jne .noforcelorom
    mov dword[GUITemp],GUIIconDataCheckBoxC
.noforcelorom
    GUIDisplayIconWin 1,184,177,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[ForceHiLoROM],2
    jne .noforcehirom
    mov dword[GUITemp],GUIIconDataCheckBoxC
.noforcehirom
    GUIDisplayIconWin 1,184,187,[GUITemp]

    ; win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
    DrawSlideBarWin 1,146,33,[GUIcurrentviewloc],[GUIfileentries],15,94,GUILStA
    cmp byte[GUICHold],1
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin 1,146,25,GUIIconDataUpArrow
    cmp byte[GUICHold],1
    jne .noholda2
    sub byte[GUIWincoladd],3
.noholda2
    cmp byte[GUICHold],2
    jne .noholdb
    add byte[GUIWincoladd],3
.noholdb
    GUIDisplayIconWin 1,146,127,GUIIconDataDownArrow
    cmp byte[GUICHold],2
    jne .noholdb2
    sub byte[GUIWincoladd],3
.noholdb2
    DrawSlideBarWin 1,230,33,[GUIcurrentdirviewloc],[GUIdirentries],15,94,GUILStB
    cmp byte[GUICHold],3
    jne .noholdc
    add byte[GUIWincoladd],3
.noholdc
    GUIDisplayIconWin 1,230,25,GUIIconDataUpArrow
    cmp byte[GUICHold],3
    jne .noholdc2
    sub byte[GUIWincoladd],3
.noholdc2
    cmp byte[GUICHold],4
    jne .noholdd
    add byte[GUIWincoladd],3
.noholdd
    GUIDisplayIconWin 1,230,127,GUIIconDataDownArrow
    cmp byte[GUICHold],4
    jne .noholdd2
    sub byte[GUIWincoladd],3
.noholdd2
    ; Draw borders
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2

    mov dl,[GUIWincol]
    DrawGUIWinBox 1,5,24,144,24,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 1,4,25,3,134,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 1,5,135,144,135,dl

    mov dl,[GUIWincol]
    DrawGUIWinBox 1,160,24,228,24,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 1,159,25,158,134,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 1,160,135,228,135,dl

    mov dl,[GUIWincol]
    DrawGUIWinBox 1,5,144,228,144,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 1,4,144,3,152,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 1,5,153,228,153,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 1,229,144,228,152,dl
    ret

SECTION .data
GUILoadText1 db 'SELECT FILE TO LOAD :',0
GUILoadText2 db 'DISPLAY TYPE :',0
GUILoadText3 db 'NORMAL 8.3 FORMAT',0
GUILoadText4 db 'SNES HEADER NAME',0
GUILoadText5 db 'WIN95 LONG FILENAME',0
GUILoadText6 db 'SHOW ALL EXTENSIONS',0
GUILoadText7 db 'FILENAME',0
GUILoadText8 db 'DIRECTORY',0
GUILoadText9 db 'LOAD',0
GUILoadTextA times 40 db 0
GUILoadTextB db 'FORCE',0
GUILoadTextC db 'NTSC',0
GUILoadTextD db 'PAL',0
GUILoadTextE db 'LOROM',0
GUILoadTextF db 'HIROM',0

SECTION .bss
GUILStA resd 3
GUILStB resd 3
GUILoadPos resb 1
SECTION .text

DisplayGUIReset:
    GUIDrawWindowBox 12,GUIResetDisp
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    mov dl,224
    cmp byte[GUIWincoladd],0
    je .zero4
    inc dl
.zero4
    cmp byte[GUICResetPos],0
    je near .resetboxa
    DrawGUIWinBox 12,79,29,117,42,dl
    jmp .resetboxb
.resetboxa
    DrawGUIWinBox 12,19,29,57,42,dl
.resetboxb
    DrawGUIButton 12,20,30,56,41,GUIResetText2,2,0,0
    DrawGUIButton 12,80,30,116,41,GUIResetText3,3,0,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 12,6,16,GUIResetText1
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 12,5,15,GUIResetText1
    ret

SECTION .data
GUIResetText1 db 'RESET : ARE YOU SURE ?',0
GUIResetText2 db 'YES',0
GUIResetText3 db 'NO',0
SECTION .text

DisplayGUIStates:
    GUIDrawWindowBox 14,GUIStatesDisp
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    mov dl,224
    cmp byte[GUIWincoladd],0
    je .zero4
    inc dl
.zero4
    cmp byte[GUICStatePos],0
    je near .resetboxa
    DrawGUIWinBox 14,79,29,117,42,dl
    jmp .resetboxb
.resetboxa
    DrawGUIWinBox 14,19,29,57,42,dl
.resetboxb
    DrawGUIButton 14,20,30,56,41,GUIStatesText3,10,0,0
    DrawGUIButton 14,80,30,116,41,GUIStatesText4,11,0,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    cmp byte[GUIStatesText5],1
    je .load
    GUIOuttextwin2 14,6,16,GUIStatesText1
    jmp .save
.load
    GUIOuttextwin2 14,6,16,GUIStatesText2
.save
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    cmp byte[GUIStatesText5],1
    je .load2
    GUIOuttextwin2 14,5,15,GUIStatesText1
    jmp .save2
.load2
    GUIOuttextwin2 14,5,15,GUIStatesText2
.save2
    ret

SECTION .data
GUIStatesText1 db 'OKAY TO SAVE STATE?',0
GUIStatesText2 db 'OKAY TO LOAD STATE?',0
GUIStatesText3 db 'YES',0
GUIStatesText4 db 'NO',0
GUIStatesText5 db 0
SECTION .text

DisplayGUIChoseSave:
    GUIDrawWindowBox 2,GUIStateSelDisp
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    mov byte[GUIChoseSaveText2],'0'
    GUIOuttextwin2 2,21,31,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'1'
    GUIOuttextwin2 2,41,31,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'2'
    GUIOuttextwin2 2,61,31,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'3'
    GUIOuttextwin2 2,81,31,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'4'
    GUIOuttextwin2 2,101,31,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'5'
    GUIOuttextwin2 2,21,46,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'6'
    GUIOuttextwin2 2,41,46,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'7'
    GUIOuttextwin2 2,61,46,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'8'
    GUIOuttextwin2 2,81,46,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'9'
    GUIOuttextwin2 2,101,46,GUIChoseSaveText2
    add byte[GUItextcolor],15
    mov byte[GUIChoseSaveText2],'0'
    GUIOuttextwin2 2,20,30,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'1'
    GUIOuttextwin2 2,40,30,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'2'
    GUIOuttextwin2 2,60,30,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'3'
    GUIOuttextwin2 2,80,30,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'4'
    GUIOuttextwin2 2,100,30,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'5'
    GUIOuttextwin2 2,20,45,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'6'
    GUIOuttextwin2 2,40,45,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'7'
    GUIOuttextwin2 2,60,45,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'8'
    GUIOuttextwin2 2,80,45,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'9'
    GUIOuttextwin2 2,100,45,GUIChoseSaveText2
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 2,6,16,GUIChoseSaveText1
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 2,5,15,GUIChoseSaveText1
    mov dword[GUITemp],GUIIconDataButtonHole
    mov ebx,[statefileloc]
    mov al,byte[fnamest+ebx]
    mov [GUIChoseSaveText2],al
    GUIDisplayButtonHole 2,10,28,byte[GUIChoseSaveText2],'t'
    GUIDisplayButtonHole 2,30,28,byte[GUIChoseSaveText2],'1'
    GUIDisplayButtonHole 2,50,28,byte[GUIChoseSaveText2],'2'
    GUIDisplayButtonHole 2,70,28,byte[GUIChoseSaveText2],'3'
    GUIDisplayButtonHole 2,90,28,byte[GUIChoseSaveText2],'4'
    GUIDisplayButtonHole 2,10,43,byte[GUIChoseSaveText2],'5'
    GUIDisplayButtonHole 2,30,43,byte[GUIChoseSaveText2],'6'
    GUIDisplayButtonHole 2,50,43,byte[GUIChoseSaveText2],'7'
    GUIDisplayButtonHole 2,70,43,byte[GUIChoseSaveText2],'8'
    GUIDisplayButtonHole 2,90,43,byte[GUIChoseSaveText2],'9'
    ret

SECTION .data
GUIChoseSaveText1 db 'SELECT SAVE SLOT :',0
GUIChoseSaveText2 db '-',0
SECTION .text

%macro DGOptnsDrawBox 3
    mov eax,dword[GUIwinposx+9*4]
    mov ebx,dword[GUIwinposy+9*4]
    add eax,%1
    add ebx,%2
    mov ecx,eax
    add ecx,20
    mov dl,167
    mov esi,7
%%loop
    DrawGUILine
    dec esi
    jnz %%loop
    mov eax,%3
    mov ebx,[ScanCodeListing+eax*3]
    mov [GUIGameDisplayKy],ebx
    mov byte[GUIGameDisplayKy+3],0
    mov byte[GUItextcolor],223
    GUIOuttextwin2 9,%1+3,%2+2,GUIGameDisplayKy
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je %%zero
    mov byte[GUItextcolor],222
%%zero
    GUIOuttextwin2 9,%1+2,%2+1,GUIGameDisplayKy
%endmacro

%macro DGOptnsDrawBox2 3
    mov eax,dword[GUIwinposx+3*4]
    mov ebx,dword[GUIwinposy+3*4]
    add eax,%1
    add ebx,%2
    mov ecx,eax
    add ecx,20
    mov dl,167
    mov esi,7
%%loop
    DrawGUILine
    dec esi
    jnz %%loop
    mov eax,%3
    mov ebx,[ScanCodeListing+eax*3]
    mov [GUIGameDisplayKy],ebx
    mov byte[GUIGameDisplayKy+3],0
    mov byte[GUItextcolor],223
    GUIOuttextwin2 3,%1+3,%2+2,GUIGameDisplayKy
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je %%zero
    mov byte[GUItextcolor],222
%%zero
    GUIOuttextwin2 3,%1+2,%2+1,GUIGameDisplayKy
%endmacro

%macro GUIInputDispKey 2
    mov eax,[edi]
    mov [GUIInputTextQ],eax
    push edi
    DGOptnsDrawBox2 %1,%2,[GUIInputTextQ]
    pop edi
    add edi,4
%endmacro

DisplayGUIInput:
    ; Player Select, Joystick/keyboard Type List, Button Assignments
    GUIDrawWindowBox 3,GUIInputDisp
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    GUIOuttextwin2 3,6,102,GUIInputText3
    GUIOuttextwin2 3,6,112,GUIInputText4
    GUIOuttextwin2 3,6,122,GUIInputText5
    GUIOuttextwin2 3,6,132,GUIInputText6
    GUIOuttextwin2 3,6,142,GUIInputTextD
    GUIOuttextwin2 3,6,152,GUIInputTextE
    GUIOuttextwin2 3,76,102,GUIInputText7
    GUIOuttextwin2 3,76,112,GUIInputText8
    GUIOuttextwin2 3,76,122,GUIInputText9
    GUIOuttextwin2 3,76,132,GUIInputTextA
    GUIOuttextwin2 3,76,142,GUIInputTextB
    GUIOuttextwin2 3,76,152,GUIInputTextC
%ifdef __MSDOS__
    GUIOuttextwin2 3,20,167,GUIInputTextW
%endif
    GUIOuttextwin2 3,156,102,GUIInputText7
    GUIOuttextwin2 3,156,112,GUIInputText8
    GUIOuttextwin2 3,156,122,GUIInputTextC
    GUIOuttextwin2 3,116,102,GUIInputText9
    GUIOuttextwin2 3,116,112,GUIInputTextA
    GUIOuttextwin2 3,116,122,GUIInputTextB
    GUIOuttextwin2 3,153,142,GUIInputTextd
    GUIOuttextwin2 3,153,152,GUIInputTexte
    GUIOuttextwin2 3,113,142,GUIInputTextf
    GUIOuttextwin2 3,113,152,GUIInputTextg
    add byte[GUItextcolor],15
    GUIOuttextwin2 3,5,101,GUIInputText3
    GUIOuttextwin2 3,5,111,GUIInputText4
    GUIOuttextwin2 3,5,121,GUIInputText5
    GUIOuttextwin2 3,5,131,GUIInputText6
    GUIOuttextwin2 3,5,141,GUIInputTextD
    GUIOuttextwin2 3,5,151,GUIInputTextE
    GUIOuttextwin2 3,75,101,GUIInputText7
    GUIOuttextwin2 3,75,111,GUIInputText8
    GUIOuttextwin2 3,75,121,GUIInputText9
    GUIOuttextwin2 3,75,131,GUIInputTextA
    GUIOuttextwin2 3,75,141,GUIInputTextB
    GUIOuttextwin2 3,75,151,GUIInputTextC
%ifdef __MSDOS__
    GUIOuttextwin2 3,19,166,GUIInputTextW
%endif
    GUIOuttextwin2 3,155,101,GUIInputText7
    GUIOuttextwin2 3,155,111,GUIInputText8
    GUIOuttextwin2 3,155,121,GUIInputTextC
    GUIOuttextwin2 3,115,101,GUIInputText9
    GUIOuttextwin2 3,115,111,GUIInputTextA
    GUIOuttextwin2 3,115,121,GUIInputTextB
    GUIOuttextwin2 3,152,141,GUIInputTextd
    GUIOuttextwin2 3,152,151,GUIInputTexte
    GUIOuttextwin2 3,112,141,GUIInputTextf
    GUIOuttextwin2 3,112,151,GUIInputTextg
%ifdef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    xor eax,eax
    mov al,[cplayernum]
    cmp byte[pl1p209+eax],0
    je .nocheckbox2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox2
    GUIDisplayIconWin 3,5,162,[GUITemp]
%endif
    DrawGUIButton 3,125,34,155,45,GUIInputTextV,14,0,0
;    cmp byte[OSPort],3
;    je near .win32
%ifdef __MSDOS__
    DrawGUIButton 3,125,50,185,61,GUIInputTexta,15,0,0
%endif
.win32
    DrawGUIButton 3,125,66,179,77,GUIInputTexta2,15,0,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    mov al,[cplayernum]
    add al,'1'
    mov [GUIInputText1+8],al
    GUIOuttextwin2 3,6,16,GUIInputText1
    GUIOuttextwin2 3,6,26,GUIInputText2
    GUIOuttextwin2 3,6,92,GUIInputTextP
    GUIOuttextwin2 3,116,92,GUIInputTextb
    GUIOuttextwin2 3,116,131,GUIInputTextc
    xor eax,eax
    mov al,[cplayernum]
    mov ebx,[GUIInputRefP+eax*4]
    mov al,[ebx]
    mov al,[GUIInputRefB+eax]
    mov ebx,eax
    shl eax,4
    add eax,ebx
    add eax,GUIInputNames
    mov dword[GUITemp],eax
    GUIOuttextwin2 3,7+24,83,[GUITemp]
    GUIOuttextwin2 3,7,83,GUIInputTextT
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 3,5,15,GUIInputText1
    GUIOuttextwin2 3,5,25,GUIInputText2
    GUIOuttextwin2 3,5,91,GUIInputTextP
    GUIOuttextwin2 3,115,91,GUIInputTextb
    GUIOuttextwin2 3,115,130,GUIInputTextc
    GUIOuttextwin2 3,6+24,82,[GUITemp]
    GUIOuttextwin2 3,6,82,GUIInputTextT
    DrawGUIWinBox 3,5,34,107,77,167
    mov ebx,[GUIcurrentinputcursloc]
    sub ebx,[GUIcurrentinputviewloc]
    mov eax,ebx
    lea ebx,[ebx*8]
    add ebx,36
    DrawGUIWinBox2 3,5,107,7,224
    mov edi,pl1selk
    cmp byte[cplayernum],1
    jne .nopl2
    mov edi,pl2selk
.nopl2
    cmp byte[cplayernum],2
    jne .nopl3
    mov edi,pl3selk
.nopl3
    cmp byte[cplayernum],3
    jne .nopl4
    mov edi,pl4selk
.nopl4
    cmp byte[cplayernum],4
    jne .nopl5
    mov edi,pl5selk
.nopl5
    GUIInputDispKey 45,150      ; Select
    GUIInputDispKey 45,140      ; Start
    GUIInputDispKey 45,100      ; Up
    GUIInputDispKey 45,110      ; Down
    GUIInputDispKey 45,120      ; Left
    GUIInputDispKey 45,130      ; Right
    GUIInputDispKey 85,120      ; X
    GUIInputDispKey 85,100      ; A
    GUIInputDispKey 85,140      ; L
    GUIInputDispKey 85,130      ; Y
    GUIInputDispKey 85,110      ; B
    GUIInputDispKey 85,150      ; R
    mov edi,pl1Atk
    cmp byte[cplayernum],1
    jne .nopl2t
    mov edi,pl2Atk
.nopl2t
    cmp byte[cplayernum],2
    jne .nopl3t
    mov edi,pl3Atk
.nopl3t
    cmp byte[cplayernum],3
    jne .nopl4t
    mov edi,pl4Atk
.nopl4t
    cmp byte[cplayernum],4
    jne .nopl5t
    mov edi,pl5Atk
.nopl5t
    GUIInputDispKey 165,100      ; A Turbo
    GUIInputDispKey 165,110      ; B Turbo
    GUIInputDispKey 125,100      ; X Turbo
    GUIInputDispKey 125,110      ; Y Turbo
    mov edi,pl1Ltk
    cmp byte[cplayernum],1
    jne .nopl2tl
    mov edi,pl2Ltk
.nopl2tl
    cmp byte[cplayernum],2
    jne .nopl3tl
    mov edi,pl3Ltk
.nopl3tl
    cmp byte[cplayernum],3
    jne .nopl4tl
    mov edi,pl4Ltk
.nopl4tl
    cmp byte[cplayernum],4
    jne .nopl5tl
    mov edi,pl5Ltk
.nopl5tl
    GUIInputDispKey 125,120      ; L Turbo
    GUIInputDispKey 165,120      ; R Turbo

    mov edi,pl1ULk
    cmp byte[cplayernum],1
    jne .nopl2tb
    mov edi,pl2ULk
.nopl2tb
    cmp byte[cplayernum],2
    jne .nopl3tb
    mov edi,pl3ULk
.nopl3tb
    cmp byte[cplayernum],3
    jne .nopl4tb
    mov edi,pl4ULk
.nopl4tb
    cmp byte[cplayernum],4
    jne .nopl5tb
    mov edi,pl5ULk
.nopl5tb
    GUIInputDispKey 165,140      ; Up-Left
    GUIInputDispKey 165,150      ; Up-Right
    GUIInputDispKey 125,140      ; Down-Left
    GUIInputDispKey 125,150      ; Down-Right

    mov byte[GUItextcolor],223
    mov eax,[GUIcurrentinputviewloc]
    shl eax,4
    add eax,[GUIcurrentinputviewloc]
    add eax,GUIInputNames
    mov dword[GUITemp],eax
    GUIOuttextwin2 3,11,38,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,11,38+8,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,11,38+8*2,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,11,38+8*3,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,11,38+8*4,[GUITemp]

    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],222
.zero5
    mov eax,[GUIcurrentinputviewloc]
    shl eax,4
    add eax,[GUIcurrentinputviewloc]
    add eax,GUIInputNames
    mov dword[GUITemp],eax
    GUIOuttextwin2 3,10,37,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,10,37+8,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,10,37+8*2,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,10,37+8*3,[GUITemp]
    add dword[GUITemp],17
    GUIOuttextwin2 3,10,37+8*4,[GUITemp]

    DrawSlideBarWin 3,109,42,[GUIcurrentinputviewloc],[NumInputDevices],5,28,GUIIStA

    cmp byte[GUICHold],9
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin 3,109,34,GUIIconDataUpArrow
    cmp byte[GUICHold],9
    jne .noholda2
    sub byte[GUIWincoladd],3
.noholda2
    cmp byte[GUICHold],10
    jne .noholdb
    add byte[GUIWincoladd],3
.noholdb
    GUIDisplayIconWin 3,109,70,GUIIconDataDownArrow
    cmp byte[GUICHold],10
    jne .noholdb2
    sub byte[GUIWincoladd],3
.noholdb2

    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 3,5,33,107,33,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 3,4,34,3,77,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 3,5,78,107,78,dl
    ret

SECTION .data
GUIInputRefF db 0,1,2,3,5,18,6,7,8,9,10,11,14,15,16,17
GUIInputRefB db 0,1,2,3,3,4,6,7,8,9,10,11,11,11,12,13,14,15,5
GUIInputRefP dd pl1contrl,pl2contrl,pl3contrl,pl4contrl,pl5contrl
GUIInputText1 db 'PLAYER #  CONTROL :',0
GUIInputText2 db 'DEVICE :',0
GUIInputText3 db '    UP',0
GUIInputText4 db '  DOWN',0
GUIInputText5 db '  LEFT',0
GUIInputText6 db ' RIGHT',0
GUIInputText7 db 'A',0
GUIInputText8 db 'B',0
GUIInputText9 db 'X',0
GUIInputTextA db 'Y',0
GUIInputTextB db 'L',0
GUIInputTextC db 'R',0
GUIInputTextD db ' START',0
GUIInputTextE db 'SELECT',0
GUIInputTextP db 'KEYS :',0
GUIInputTextQ dd 0
GUIInputTextT db 'CDV:',0
GUIInputTextV db 'SET',0
GUIInputTextW db 'USE JOYSTICK PORT 209H',0
GUIInputTexta db 'CALIBRATE',0
GUIInputTexta2 db 'SET KEYS',0
GUIInputTextb db 'TURBO :',0
GUIInputTextc db 'DIAGONALS :',0
GUIInputTextd db 'UL',0
GUIInputTexte db 'UR',0
GUIInputTextf db 'DL',0
GUIInputTextg db 'DR',0

SECTION .bss
GUIIStA resd 3

GUIcurrentinputviewloc resd 1
GUIcurrentinputcursloc resd 1
SECTION .text

DisplayGUIOption:
    ; Frame Rate/Auto Frame Rate/Max Frame Skip/New Gfx Engine/Save Confirm
    GUIDrawWindowBox 4,GUIOptionDisp
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    mov dword[GUITemp],GUIOptionText1
    cmp byte[frameskip],0
    jne .noauto
    mov dword[GUITemp],GUIOptionText3
.noauto
    GUIOuttextwin2 4,6,16,[GUITemp]
    GUIOuttextwin2u 4,26,31,GUIOptionText2,0
    GUIOuttextwin2u 4,26,41,GUIOptionText4,0
    GUIOuttextwin2u 4,26,51,GUIOptionText5,2
    GUIOuttextwin2u 4,26,61,GUIOptionText6,0
    GUIOuttextwin2u 4,26,71,GUIOptionText7,0
%ifdef __MSDOS__
    GUIOuttextwin2u 4,26,81,GUIOptionTextA,0
%endif
%ifdef __WIN32__
    GUIOuttextwin2u 4,26,81,GUIOptionTextAb,0
%endif
    GUIOuttextwin2u 4,26,91,GUIOptionTextB,0
    GUIOuttextwin2u 4,26,101,GUIOptionTextC,2
    GUIOuttextwin2u 4,26,111,GUIOptionTextD,12
    GUIOuttextwin2u 4,26,121,GUIOptionTextE,2
    GUIOuttextwin2u 4,26,131,GUIOptionTextF,1
    GUIOuttextwin2u 4,26,141,GUIOptionTextG,0
    GUIOuttextwin2u 4,26,151,GUIOptionTextH,9
    GUIOuttextwin2u 4,26,161,GUIOptionTextI,15
    GUIOuttextwin2u 4,26,171,GUIOptionTextJ,0
;    GUIOuttextwin2u 4,26,181,GUIOptionTextK,1
%ifdef __WIN32__
    GUIOuttextwin2u 4,26,191,GUIOptionTextL,1
%endif
    add byte[GUItextcolor],15
    GUIOuttextwin2 4,5,15,[GUITemp]
    GUIOuttextwin2 4,25,30,GUIOptionText2
    GUIOuttextwin2 4,25,40,GUIOptionText4
    GUIOuttextwin2 4,25,50,GUIOptionText5
    GUIOuttextwin2 4,25,60,GUIOptionText6
    GUIOuttextwin2 4,25,70,GUIOptionText7
%ifdef __MSDOS__
    GUIOuttextwin2 4,25,80,GUIOptionTextA
%endif
%ifdef __WIN32__
    GUIOuttextwin2 4,25,80,GUIOptionTextAb
%endif
    GUIOuttextwin2 4,25,90,GUIOptionTextB
    GUIOuttextwin2 4,25,100,GUIOptionTextC
    GUIOuttextwin2 4,25,120,GUIOptionTextE
    GUIOuttextwin2 4,25,110,GUIOptionTextD
    GUIOuttextwin2 4,25,130,GUIOptionTextF
    GUIOuttextwin2 4,25,140,GUIOptionTextG
    GUIOuttextwin2 4,25,150,GUIOptionTextH
    GUIOuttextwin2 4,25,160,GUIOptionTextI
    GUIOuttextwin2 4,25,170,GUIOptionTextJ
;    GUIOuttextwin2 4,25,180,GUIOptionTextK
%ifdef __WIN32__
    GUIOuttextwin2 4,25,190,GUIOptionTextL
%endif
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[frameskip],0
    jne .nocheckbox
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox
    GUIDisplayIconWin 4,11,26,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[newengen],0
    je .nocheckbox1
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1
    GUIDisplayIconWin 4,11,36,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[AutoIncSaveSlot],0
    je .nocheckbox2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox2
    GUIDisplayIconWin 4,11,46,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[MMXSupport],1
    jne .nocheckbox3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox3
    GUIDisplayIconWin 4,11,56,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[pl12s34],0
    je .nocheckbox4
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox4
    GUIDisplayIconWin 4,11,66,[GUITemp]
%ifdef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SidewinderFix],0
    je .nocheckbox5
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox5
    GUIDisplayIconWin 4,11,76,[GUITemp]
%endif
%ifdef __WIN32__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[HighPriority],0
    je .nocheckbox5
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox5
    GUIDisplayIconWin 4,11,76,[GUITemp]
%endif
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[FPSAtStart],0
    je .nocheckbox6
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox6
    GUIDisplayIconWin 4,11,86,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[Turbo30hz],0
    je .nocheckbox7
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox7
    GUIDisplayIconWin 4,11,96,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[TimerEnable],0
    je .nocheckbox8
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox8
    GUIDisplayIconWin 4,11,106,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[FastFwdToggle],0
    je .nocheckbox9
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox9
    GUIDisplayIconWin 4,11,116,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SRAMSave5Sec],0
    je .nocheckbox10
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox10
    GUIDisplayIconWin 4,11,126,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[OldGfxMode2],0
    je .nocheckbox11
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox11
    GUIDisplayIconWin 4,11,136,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[LatestSave],0
    je .nocheckbox12
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox12
    GUIDisplayIconWin 4,11,146,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[AutoState],0
    je .nocheckbox13
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox13
    GUIDisplayIconWin 4,11,156,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[DontSavePath],0
    je .nocheckbox14
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox14
    GUIDisplayIconWin 4,11,166,[GUITemp]
%ifdef __WIN32__
;    mov dword[GUITemp],GUIIconDataCheckBoxUC
;    cmp byte[AlternateTimer],0
;    je .nocheckbox16
;    mov dword[GUITemp],GUIIconDataCheckBoxC
;.nocheckbox16
;    GUIDisplayIconWin 4,11,186,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[DisableScreenSaver],0
    je .nocheckbox15
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox15
    GUIDisplayIconWin 4,11,186,[GUITemp]
%endif
    cmp byte[frameskip],0
    je near .auto
    DrawGUIWinBox 4,75,13,93,21,167
    mov al,[frameskip]
    add al,47
    mov [GUIOptionText8],al
    mov byte[GUItextcolor],223
    GUIOuttextwin2 4,85,16,GUIOptionText8
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    GUIOuttextwin2 4,84,15,GUIOptionText8
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero6
    mov byte[GUItextcolor],211
.zero6
    DrawGUIButton 4,97,12,105,22,GUIOptionText9,12,-2,0
    DrawGUIButton 4,108,12,116,22,GUIOptionText10,13,-2,0
    ret
.auto
    DrawGUIWinBox 4,96,13,114,21,167
    mov al,[maxskip]
    add al,48
    mov [GUIOptionText8],al
    mov byte[GUItextcolor],223
    GUIOuttextwin2 4,107,16,GUIOptionText8
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],222
.zero5
    GUIOuttextwin2 4,106,15,GUIOptionText8
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero7
    mov byte[GUItextcolor],211
.zero7
    DrawGUIButton 4,118,12,126,22,GUIOptionText9,12,-2,0
    DrawGUIButton 4,129,12,137,22,GUIOptionText10,13,-2,0
    ret

SECTION .data
GUIOptionText1 db 'FRAME RATE',0
GUIOptionText2 db 'AUTO FRAME RATE',0
GUIOptionText3 db 'MAX FRAME SKIP',0
GUIOptionText4 db 'NEW GFX ENGINE',0
GUIOptionText5 db 'AUTO INCRMNT SAVE SLOT',0
GUIOptionText6 db 'MMX SUPPORT',0
GUIOptionText7 db 'USE PL3/4 AS PL1/2',0
GUIOptionTextA db 'SIDEWINDER FIX',0
GUIOptionTextAb db 'HIGH PRIORITY MODE',0
GUIOptionTextB db 'FPS AT PROGRAM START',0
GUIOptionTextC db 'USE 30HZ FOR TURBO',0
GUIOptionTextD db 'ENABLE GAME CLOCK',0
GUIOptionTextE db 'TOGGLED FAST FWD/SLWDWN',0
GUIOptionTextF db 'SRAM CHECK+SAVE',0
GUIOptionTextG db 'OLD GFX MODE 2',0
GUIOptionTextH db 'START AT LATEST SAVE',0
GUIOptionTextI db 'AUTO STATE SAVE/LOAD',0
GUIOptionTextJ db 'DON',39,'T SAVE CUR PATH',0
GUIOptionTextK db 'BLANK SPACE',0
GUIOptionTextL db 'DISABLE SCREENSAVER',0
GUIOptionText8 db '-',0
GUIOptionText9 db '+',0
GUIOptionText10 db '-',0
SECTION .text

DisplayGUIVideo:
    cmp byte[TripBufAvail],0
    jne .notexttb
    mov byte[Triplebufen],0
.notexttb
    cmp byte[MMXSupport],1
    jne .2xSaIdis
    cmp byte[newgfx16b],0
    je .2xSaIdis
    jmp .no2xSaIdis
.2xSaIdis
    mov byte[En2xSaI],0
.no2xSaIdis
    cmp byte[En2xSaI],0
    je .no2xsaidis
    mov byte[Triplebufen],0
.no2xsaidis
    cmp byte[En2xSaI],0
    je .no2xsaien
    mov byte[scanlines],0
    mov byte[antienab],0
.no2xsaien
    ; Video Modes List/Scanlines/Interpolation/Full(Small) Screen
    GUIDrawWindowBox 5,GUIVideoDisp
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15

;    cmp byte[OSPort],3
;    jne near .nowin32pt2
%ifndef __MSDOS__
    GUIOuttextwin2 5,12,76,GUIVideoTextw0
    GUIOuttextwin2 5,12,83,GUIVideoTextw1
    GUIOuttextwin2 5,12,90,GUIVideoTextw2
    GUIOuttextwin2 5,12,97,GUIVideoTextw3
%endif
;.nowin32pt2

    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISLVID+eax],0
    je near .notext2
    GUIOuttextwin2 5,13,111,GUIVideoTextb
    GUIOuttextwin2u 5,36,119,GUIVideoTexta,0
.notext2
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIHSVID+eax],0
    je near .notexta
    GUIOuttextwin2 5,13,111,GUIVideoTextb
    GUIOuttextwin2u 5,76,119,GUIVideoTexta2,0
    GUIOuttextwin2u 5,116,119,GUIVideoTexta3,0
.notexta
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUITBWVID+eax],0
    je near .nottbw
    GUIOuttextwin2u 5,26,128,GUIVideoText7,0
.nottbw
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUI2xVID+eax],0
    je near .notextb
    GUIOuttextwin2u 5,26,138,GUIVideoTextc,1
    GUIOuttextwin2u 5,26,148,GUIVideoTextd,6
    GUIOuttextwin2u 5,26,158,GUIVideoTextd2,0
.notextb
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUII2VID+eax],0
    je near .notextb2
    GUIOuttextwin2u 5,26,168,GUIVideoText3,0
.notextb2
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIBIFIL+eax],0
    je near .notextbf2
    GUIOuttextwin2u 5,26,168,GUIVideoTextbf,0
.notextbf2
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIINVID+eax],0
    je .notext3
    GUIOuttextwin2u 5,26,83,GUIVideoText3,0
.notext3
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIEAVID+eax],0
    je .notext4
    GUIOuttextwin2u 5,26,83,GUIVideoText4,1
.notext4
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIFSVID+eax],0
    je near .notext5
    cmp byte[GUIWSVID+eax],0
    je .notext4a
    GUIOuttextwin2u 5,26,93,GUIVideoText5b,1
    jmp .notext5
.notext4a
    GUIOuttextwin2u 5,26,93,GUIVideoText5,1
.notext5
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISSVID+eax],0
    je .notext6
    GUIOuttextwin2u 5,26,103,GUIVideoText6,1
.notext6
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[TripBufAvail],0
    je near .notext7
    cmp byte[GUITBVID+eax],0
    je .notext7
    GUIOuttextwin2u 5,26,168,GUIVideoText7,0
.notext7
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIM7VID+eax],0
    je .notext8
    cmp byte[newengen],0
    je .notext8
    GUIOuttextwin2u 5,26,178,GUIVideoText8b,0
.notext8
    GUIOuttextwin2u 5,26,188,GUIVideoText8c,0
%ifndef __LINUX__
    GUIOuttextwin2u 5,130,188,GUIVideoText9,0
%endif
    add byte[GUItextcolor],15

;    cmp byte[OSPort],3
;    jne near .nowin32pt
%ifndef __MSDOS__
    GUIOuttextwin2 5,11,75,GUIVideoTextw0
    GUIOuttextwin2 5,11,82,GUIVideoTextw1
    GUIOuttextwin2 5,11,89,GUIVideoTextw2
    GUIOuttextwin2 5,11,96,GUIVideoTextw3
%endif
;.nowin32pt
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISLVID+eax],0
    je .notext2b
    GUIOuttextwin2 5,12,110,GUIVideoTextb
    GUIOuttextwin2 5,35,118,GUIVideoTexta
.notext2b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIHSVID+eax],0
    je near .notextab
    GUIOuttextwin2 5,12,110,GUIVideoTextb
    GUIOuttextwin2 5,75,118,GUIVideoTexta2
    GUIOuttextwin2 5,115,118,GUIVideoTexta3
.notextab
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUITBWVID+eax],0
    je near .nottbwb
    GUIOuttextwin2 5,25,127,GUIVideoText7
.nottbwb
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUI2xVID+eax],0
    je near .notextbb
    GUIOuttextwin2 5,25,137,GUIVideoTextc
    GUIOuttextwin2 5,25,147,GUIVideoTextd
    GUIOuttextwin2 5,25,157,GUIVideoTextd2
.notextbb
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUII2VID+eax],0
    je near .notextb2b
    GUIOuttextwin2 5,25,167,GUIVideoText3
.notextb2b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIBIFIL+eax],0
    je near .notextb2bf
    GUIOuttextwin2 5,25,167,GUIVideoTextbf
.notextb2bf
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIINVID+eax],0
    je .notext3b
    GUIOuttextwin2 5,25,82,GUIVideoText3
.notext3b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIEAVID+eax],0
    je .notext4b
    GUIOuttextwin2 5,25,82,GUIVideoText4
.notext4b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIFSVID+eax],0
    je near .notext5b
    cmp byte[GUIWSVID+eax],0
    je .notext4ba
    GUIOuttextwin2 5,25,92,GUIVideoText5b
    jmp .notext5b
.notext4ba
    GUIOuttextwin2 5,25,92,GUIVideoText5
.notext5b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISSVID+eax],0
    je .notext6b
    GUIOuttextwin2 5,25,102,GUIVideoText6
.notext6b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[TripBufAvail],0
    je near .notext7b
    cmp byte[GUITBVID+eax],0
    je .notext7b
    GUIOuttextwin2 5,25,167,GUIVideoText7
.notext7b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIM7VID+eax],0
    je .notext8b
    cmp byte[newengen],0
    je .notext8b
    GUIOuttextwin2 5,25,177,GUIVideoText8b
.notext8b
    GUIOuttextwin2 5,25,187,GUIVideoText8c
%ifndef __LINUX__
    GUIOuttextwin2 5,129,187,GUIVideoText9
%endif
    DrawGUIButton 5,128,60,164,71,GUIVideoText8,4,0,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 5,6,16,GUIVideoText1
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 5,5,15,GUIVideoText1
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISLVID+eax],0
    je near .nocheckboxb
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[scanlines],1
    jne .nocheckbox
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox
    GUIDisplayIconWin 5,21,114,[GUITemp]
.nocheckboxb
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIHSVID+eax],0
    je .nocheckboxw
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[scanlines],3
    jne .nocheckboxv
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxv
    GUIDisplayIconWin 5,61,114,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[scanlines],2
    jne .nocheckboxv2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxv2
    GUIDisplayIconWin 5,101,114,[GUITemp]
.nocheckboxw
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUITBWVID+eax],0
    je near .nocheckboxslw2
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[TripleBufferWin],1
    jne .nocheckboxslw
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxslw
    GUIDisplayIconWin 5,11,123,[GUITemp]
.nocheckboxslw2
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUI2xVID+eax],0
    je near .nocheckboxx
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[En2xSaI],1
    jne .nocheckboxx2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxx2
    GUIDisplayIconWin 5,11,133,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[En2xSaI],2
    jne .nocheckboxx3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxx3
    GUIDisplayIconWin 5,11,143,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[En2xSaI],3
    jne .nocheckboxx4
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxx4
    GUIDisplayIconWin 5,11,153,[GUITemp]
.nocheckboxx

    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUII2VID+eax],0
    je near .nocheckboxi2
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[antienab],0
    je .nocheckbox1i2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1i2
    GUIDisplayIconWin 5,11,163,[GUITemp]
.nocheckboxi2
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIBIFIL+eax],0
    je near .nocheckboxbf
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[BilinearFilter],0
    je .nocheckbox1bf
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1bf
    GUIDisplayIconWin 5,11,163,[GUITemp]
.nocheckboxbf
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUITBVID+eax],0
    je .nocheckboxtbb
    cmp byte[TripBufAvail],0
    je near .nocheckboxtbb
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[Triplebufen],1
    jne .nocheckboxtb
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxtb
    GUIDisplayIconWin 5,11,163,[GUITemp]
.nocheckboxtbb
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIM7VID+eax],0
    je near .nocheckboxm72
    cmp byte[newengen],0
    je near .nocheckboxm72
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[Mode7HiRes16b],0
    je .nocheckbox1m72
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1m72
    GUIDisplayIconWin 5,11,173,[GUITemp]
.nocheckboxm72
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GrayscaleMode],0
    je .nocheckboxm73
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxm73
    GUIDisplayIconWin 5,11,183,[GUITemp]
%ifndef __LINUX__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[vsyncon],0
    je .nocheckboxm74
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckboxm74
    GUIDisplayIconWin 5,115,183,[GUITemp]
%endif
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIIEVID+eax],0
    je .nocheckbox1b
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[antienab],0
    je .nocheckbox1
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1
    GUIDisplayIconWin 5,11,78,[GUITemp]
.nocheckbox1b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIFSVID+eax],0
    je .nocheckbox2b
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[ScreenScale],0
    je .nocheckbox2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox2
    GUIDisplayIconWin 5,11,88,[GUITemp]
.nocheckbox2b
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUISSVID+eax],0
    je .nocheckbox3b
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[smallscreenon],0
    je .nocheckbox3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox3
    GUIDisplayIconWin 5,11,98,[GUITemp]
.nocheckbox3b
    DrawGUIWinBox 5,5,26,115,69,167
    DrawSlideBarWin 5,117,34,[GUIcurrentvideoviewloc],[NumVideoModes],5,28,GUIVStA
    cmp byte[GUICHold],5
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin 5,117,26,GUIIconDataUpArrow
    cmp byte[GUICHold],5
    jne .noholdb
    sub byte[GUIWincoladd],3
.noholdb 
    cmp byte[GUICHold],6
    jne .noholdc
    add byte[GUIWincoladd],3
.noholdc
    GUIDisplayIconWin 5,117,62,GUIIconDataDownArrow
    cmp byte[GUICHold],6
    jne .noholdd
    sub byte[GUIWincoladd],3
.noholdd

    mov ebx,[GUIcurrentvideocursloc]
    sub ebx,[GUIcurrentvideoviewloc]
    lea ebx,[ebx*8]
    add ebx,28
    DrawGUIWinBox2 5,5,115,7,224

    mov byte[GUItextcolor],223
    mov eax,[GUIcurrentvideoviewloc]
    mov ebx,eax
    shl eax,4
    add eax,ebx
    add eax,ebx
    add eax,GUIVideoModeNames
    mov dword[GUITemp],eax
    GUIOuttextwin2 5,11,30,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,11,30+8,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,11,30+8*2,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,11,30+8*3,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,11,30+8*4,[GUITemp]

    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],222
.zero5
    mov eax,[GUIcurrentvideoviewloc]
    mov ebx,eax
    shl eax,4
    add eax,ebx
    add eax,ebx
    add eax,GUIVideoModeNames
    mov dword[GUITemp],eax
    GUIOuttextwin2 5,10,29,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,10,29+8,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,10,29+8*2,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,10,29+8*3,[GUITemp]
    add dword[GUITemp],18
    GUIOuttextwin2 5,10,29+8*4,[GUITemp]

    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 5,5,25,115,25,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 5,4,26,3,69,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 5,5,70,115,70,dl
    xor eax,eax
    mov al,[cvidmode]
    mov ebx,eax
    shl eax,4
    add eax,ebx
    add eax,ebx
    add eax,GUIVideoModeNames
    mov dword[GUITemp],eax
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 5,7,196,GUIVideoTextH
    GUIOuttextwin2 5,49,196,[GUITemp]
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero6
    mov byte[GUItextcolor],164
.zero6
    GUIOuttextwin2 5,6,195,GUIVideoTextH
    GUIOuttextwin2 5,48,195,[GUITemp]
    ret

SECTION .data
GUIVideoText1 db 'VIDEO MODES :',0
GUIVideoText2 db 'FULL SCANLINES',0
GUIVideoTextb db 'SCANLINES:',0
GUIVideoTexta db 'FULL',0
GUIVideoTexta2 db '50%',0
GUIVideoTexta3 db '25%',0
GUIVideoTextc db '2XSAI ENGINE',0
GUIVideoTextd db 'SUPER EAGLE ENGINE',0
GUIVideoTextd2 db 'SUPER 2XSAI ENGINE',0
GUIVideoTextbf db 'BILINEAR FILTER',0
GUIVideoText3 db 'INTERPOLATION',0      ; -y
GUIVideoText4 db 'EAGLE ENGINE',0       ; -y
GUIVideoText5 db 'FULL SCREEN',0        ; -c
GUIVideoText5b db 'WIDE SCREEN',0        ; -c
GUIVideoText6 db 'SMALL SCREEN',0       ; -c
GUIVideoText7 db 'TRIPLE BUFFERING',0       ; -c
GUIVideoText8b db 'HI-RESOLUTION MODE7',0       ; -c
GUIVideoText8c db 'GRAYSCALE MODE',0       ; -c
GUIVideoText8 db 'SET',0                ; set button
GUIVideoText9 db 'VSYNC',0       ; -c
GUIVideoTextH db 'MODE : ',0
GUIVideoTextw0 db 'LEGEND:',0
GUIVideoTextw1 db ' S = SCALED TO FIT SCREEN',0
GUIVideoTextw2 db ' R = MATCHED SCREEN RATIO',0
GUIVideoTextw3 db ' D = ALLOW 2XSAI,HIRES,ETC',0

SECTION .bss
GUIVStA resd 3

GUIcurrentvideoviewloc resd 1
GUIcurrentvideocursloc resd 1

SECTION .text


DisplayGUISound:
    ; Enable Sound/Sampling Rate/Stereo/Sound Buffering/Volume
    GUIDrawWindowBox 6,GUISoundDisp
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    ; E = Enable Sound, S = Stereo Sound, B = Sound Buffering
    ; R = Sample Rate
    GUIOuttextwin2u 6,26,26,GUISoundText1,0
    GUIOuttextwin2u 6,26,35,GUISoundText3,0
;    cmp byte[OSPort],3
;    je near .win32b
%ifdef __MSDOS__
    GUIOuttextwin2u 6,116,26,GUISoundText1b,0
    GUIOuttextwin2u 6,26,44,GUISoundText2b,1
%endif
;.win32b
;    GUIOuttextwin2u 6,26,106,GUISoundText4,8
    GUIOuttextwin2u 6,26,116,GUISoundTextH,2
;    cmp byte[OSPort],3
;    je near .win32s2
    GUIOuttextwin2u 6,26,126,GUISoundTextI,1
;.win32s2
    GUIOuttextwin2u 6,26,136,GUISoundTextJ,6
    GUIOuttextwin2u 6,26,146,GUISoundTextK,0
    GUIOuttextwin2u 6,26,168,GUISoundTextQ,0
    GUIOuttextwin2u 6,26,178,GUISoundTextR,0
%ifndef __MSDOS__
    GUIOuttextwin2u 6,26,188,GUISoundTextS,0
%endif
    GUIOuttextwin2u 6,126,168,GUISoundTextU,1
    GUIOuttextwin2u 6,126,178,GUISoundTextV,1
%ifndef __MSDOS__
    GUIOuttextwin2u 6,126,188,GUISoundTextW,0
%endif
    add byte[GUItextcolor],15
    GUIOuttextwin2 6,25,25,GUISoundText1
    GUIOuttextwin2 6,25,34,GUISoundText3
;    cmp byte[OSPort],3
;    je near .win32c
%ifdef __MSDOS__
    GUIOuttextwin2 6,115,25,GUISoundText1b
    GUIOuttextwin2 6,25,43,GUISoundText2b
%endif
;.win32c
;    GUIOuttextwin2 6,25,105,GUISoundText4
    GUIOuttextwin2 6,25,115,GUISoundTextH
;    cmp byte[OSPort],3
;    je near .win32s
    GUIOuttextwin2 6,25,125,GUISoundTextI
;.win32s
    GUIOuttextwin2 6,25,135,GUISoundTextJ
    GUIOuttextwin2 6,25,145,GUISoundTextK
    GUIOuttextwin2 6,25,167,GUISoundTextQ
    GUIOuttextwin2 6,25,177,GUISoundTextR
%ifndef __MSDOS__
    GUIOuttextwin2 6,25,187,GUISoundTextS
%endif
    GUIOuttextwin2 6,125,167,GUISoundTextU
    GUIOuttextwin2 6,125,177,GUISoundTextV
%ifndef __MSDOS__
    GUIOuttextwin2 6,125,187,GUISoundTextW
%endif
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 6,6,16,GUISoundText6
    GUIOuttextwin2u 6,6,53,GUISoundText2,9
    GUIOuttextwin2 6,6,76,GUISoundText5
    GUIOuttextwin2 6,6,158,GUISoundTextP
    GUIOuttextwin2 6,106,158,GUISoundTextT
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 6,5,15,GUISoundText6
    GUIOuttextwin2 6,5,52,GUISoundText2
    GUIOuttextwin2 6,5,75,GUISoundText5
    GUIOuttextwin2 6,5,157,GUISoundTextP
    GUIOuttextwin2 6,105,157,GUISoundTextT
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[cfgsoundon],0
    je .nocheckbox
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox
    GUIDisplayIconWin 6,11,21,[GUITemp]
%ifdef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[cfgforce8b],0
    je .nocheckbox0
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox0
    GUIDisplayIconWin 6,102,21,[GUITemp]
%endif
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[cfgStereoSound],0
    je .nocheckbox1
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1
    GUIDisplayIconWin 6,11,30,[GUITemp]
%ifdef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[RaisePitch],0
    je .nocheckbox1b
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox1b
;    cmp byte[OSPort],3
;    je near .win32
    GUIDisplayIconWin 6,11,39,[GUITemp]
%endif
;.win32
;    mov dword[GUITemp],GUIIconDataCheckBoxUC
;    cmp byte[SoundNoiseDis],0
;    je .nocheckbox2
;    mov dword[GUITemp],GUIIconDataCheckBoxC
;.nocheckbox2
;    GUIDisplayIconWin 6,11,101,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[RevStereo],0
    je .nocheckbox3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox3
    GUIDisplayIconWin 6,11,111,[GUITemp]
;    cmp byte[OSPort],3
;    je near .nocheckbox42
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[Surround],0
    je .nocheckbox4
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox4
    GUIDisplayIconWin 6,11,121,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SoundBufEn],0
    je .nocheckbox5
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox5
    GUIDisplayIconWin 6,11,131,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SPCDisable],0
    je .nocheckbox6
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox6
    GUIDisplayIconWin 6,11,141,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SoundInterpType],1
    jne .nocheckbox7
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox7
    GUIDisplayIconWin 6,11,163,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SoundInterpType],2
    jne .nocheckbox8
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox8
    GUIDisplayIconWin 6,11,173,[GUITemp]
%ifndef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SoundInterpType],3
    jne .nocheckbox9
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox9
    GUIDisplayIconWin 6,11,183,[GUITemp]
%endif
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[LowPassFilterType],1
    jne .nocheckbox10
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox10
    GUIDisplayIconWin 6,111,163,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[LowPassFilterType],2
    jne .nocheckbox11
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox11
    GUIDisplayIconWin 6,111,173,[GUITemp]
%ifndef __MSDOS__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[LowPassFilterType],3
    jne .nocheckbox12
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox12
    GUIDisplayIconWin 6,111,183,[GUITemp]
%endif
    DrawGUIWinBox 6,15,61,69,69,167
    mov dword[GUITemp],GUISoundText7
    xor eax,eax
    mov al,[cfgSoundQuality]
    shl eax,3
    add [GUITemp],eax
    mov byte[GUItextcolor],223
    GUIOuttextwin2 6,23,64,[GUITemp]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    GUIOuttextwin2 6,22,63,[GUITemp]
    mov eax,dword[GUIwinposx+6*4]
    mov ebx,dword[GUIwinposy+6*4]
    mov ecx,eax
    add eax,15
    add ecx,15+100
    add ebx,91
    xor edx,edx
    mov dl,215
    sub dl,[GUIWincoladd]
    DrawGUILine
    inc ecx
    inc eax
    sub dl,13
    DrawGUILine
    xor eax,eax
    mov al,[MusicRelVol]
    mov dword[GUITemp],11
    add [GUITemp],eax
    GUIDisplayIconWin 6,[GUITemp],87,GUIIconDataSlideBar
    mov dword[GUISoundTextD],' '
    mov byte[GUISoundTextD+3],'%'
    mov esi,GUISoundTextD+2
    xor eax,eax
    mov al,[MusicRelVol]
    mov ebx,10
    div bl
    add ah,48
    mov [esi],ah
    cmp al,0
    je .nomore
    dec esi
    xor ah,ah
    div bl
    add ah,48
    mov [esi],ah
    cmp al,0
    je .nomore
    dec esi
    xor ah,ah
    div bl
    add ah,48
    mov [esi],ah
.nomore
    mov [GUITemp],esi
    mov byte[GUItextcolor],223
    GUIOuttextwin2 6,119,89,[GUITemp]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],222
.zero5
    GUIOuttextwin2 6,118,88,[GUITemp]
    ret

SECTION .data
GUISoundText1 db 'ENABLE SOUND',0
GUISoundText1b db 'FORCE 8-BIT',0
GUISoundText2 db 'SAMPLING RATE :',0
GUISoundText2b db 'RAISE PITCH LEVEL',0
GUISoundText3 db 'STEREO SOUND',0
GUISoundText4 db '',0
GUISoundText5 db 'VOLUME LEVEL :',0
GUISoundText6 db 'SOUND SWITCHES :',0
GUISoundText7 db ' 8000HZ',0
GUISoundText8 db '11025HZ',0
GUISoundText9 db '22050HZ',0
GUISoundTextA db '44100HZ',0
GUISoundTextB db '16000HZ',0
GUISoundTextC db '32000HZ',0
GUISoundTextCb db '48000HZ',0
GUISoundTextD db '---%',0
GUISoundTextH db 'REVERSE STEREO',0
GUISoundTextI db 'SURROUND SOUND',0
GUISoundTextJ db 'SOUND BUFFERING',0
GUISoundTextK db 'DISABLE SPC EMULATION',0
GUISoundTextP db 'INTERPOLATION :',0
GUISoundTextQ db 'GAUSSIAN',0
GUISoundTextR db 'CUBIC SPLINE',0
GUISoundTextS db '8-POINT',0
GUISoundTextT db 'LOWPASS :',0
GUISoundTextU db 'SIMPLE',0
GUISoundTextV db 'DYNAMIC',0
GUISoundTextW db 'HI QUALITY',0
SECTION .text


DisplayGUICheatConv:
    mov eax,[ccheatnpos]
    add eax,4
    xor ebx,ebx
    mov ecx,3
    mov edx,GUICheatTextB
.loop
    mov bl,[eax]
    and bl,0Fh
    mov bl,[GUICheatTextC+ebx]
    mov [edx+1],bl
    mov bl,[eax]
    and bl,0F0h
    shr bl,4
    mov bl,[GUICheatTextC+ebx]
    mov [edx],bl
    add edx,2
    dec eax
    dec ecx
    jnz .loop
    mov eax,[ccheatnpos]
    inc eax
    mov edx,GUICheatTextB+7
    mov bl,[eax]
    and bl,0Fh
    mov bl,[GUICheatTextC+ebx]
    mov [edx+1],bl
    mov bl,[eax]
    and bl,0F0h
    shr bl,4
    mov bl,[GUICheatTextC+ebx]
    mov [edx],bl
    mov eax,[ccheatnpos]
    add eax,5
    mov edx,GUICheatTextB+10
    mov bl,[eax]
    and bl,0Fh
    mov bl,[GUICheatTextC+ebx]
    mov [edx+1],bl
    mov bl,[eax]
    and bl,0F0h
    shr bl,4
    mov bl,[GUICheatTextC+ebx]
    mov [edx],bl
    mov eax,[ccheatnpos]
    test byte[eax-28],80h
    jnz .src
    test byte[eax],04h
    jz .on
    mov byte[GUICheatTextB+13],'O'
    mov byte[GUICheatTextB+14],'F'
    mov byte[GUICheatTextB+15],'F'
    jmp .off
.on
    test byte[eax],80h
    jnz .repl
    mov byte[GUICheatTextB+13],'O'
    mov byte[GUICheatTextB+14],'N'
    mov byte[GUICheatTextB+15],' '
    jmp .off
.repl
    mov byte[GUICheatTextB+13],'R'
    mov byte[GUICheatTextB+14],'P'
    mov byte[GUICheatTextB+15],'L'
    jmp .off
.src
    mov byte[GUICheatTextB+13],'S'
    mov byte[GUICheatTextB+14],'R'
    mov byte[GUICheatTextB+15],'C'
.off
    mov ecx,20
    add eax,8
    mov edx,GUICheatTextB+17
.loop2
    mov bl,[eax]
    mov [edx],bl
    inc eax
    inc edx
    dec ecx
    jnz .loop2
    ret

%macro GUIOuttextwin2cheat 2
    test dword[ccheatnleft],80000000h
    jnz %%skip
    call DisplayGUICheatConv
    GUIOuttextwin2 7,%1,%2,GUICheatTextB
    add dword[ccheatnpos],28
    dec dword[ccheatnleft]
%%skip
%endmacro

DisplayGUICheat:
    GUIDrawWindowBox 7,GUICheatDisp
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    GUIOuttextwin2 7,6,132,GUICheatText1
    GUIOuttextwin2 7,6,143,GUICheatText2
    GUIOuttextwin2 7,6,13,GUICheatText3
    GUIOuttextwin2 7,11,154,GUICheatTextD
    GUIOuttextwin2 7,11,164,GUICheatTextE
    GUIOuttextwin2 7,11,172,GUICheatTextF
    GUIOuttextwin2 7,11,180,GUICheatTextG
    GUIOuttextwin2 7,26,191,GUICheatTextJ
    add byte[GUItextcolor],15
    GUIOuttextwin2 7,5,131,GUICheatText1
    GUIOuttextwin2 7,5,142,GUICheatText2
    GUIOuttextwin2 7,5,12,GUICheatText3
    GUIOuttextwin2 7,10,153,GUICheatTextD
    GUIOuttextwin2 7,10,163,GUICheatTextE
    GUIOuttextwin2 7,10,171,GUICheatTextF
    GUIOuttextwin2 7,10,179,GUICheatTextG
    GUIOuttextwin2 7,25,190,GUICheatTextJ
    DrawGUIWinBox 7,5,20,229,108,167

    DrawGUIButton 7,5,113,47,124,GUICheatText4,5,0,0
    DrawGUIButton 7,52,113,94,124,GUICheatText5,6,0,0
    DrawGUIButton 7,99,113,141,124,GUICheatText7,7,0,0
    DrawGUIButton 7,146,113,188,124,GUICheatText8,8,0,0
    DrawGUIButton 7,212,134,236,145,GUICheatText6,9,0,0
    DrawGUIButton 7,193,113,235,124,GUICheatTextI,33,0,0

    cmp dword[GUIcurrentcheatwin],0
    jne near .nowinbox
    mov ebx,[GUIcurrentcheatcursloc]
    sub ebx,[GUIcurrentcheatviewloc]
    mov eax,ebx
    lea ebx,[ebx*8]
    sub ebx,eax
    add ebx,22
    DrawGUIWinBox2 7,5,229,7,224
.nowinbox

    mov byte[GUItextcolor],223
    mov dword[ccheatnpos],cheatdata
    mov eax,[NumCheats]
    sub eax,[GUIcurrentcheatviewloc]
    dec eax
    mov dword[ccheatnleft],eax
    mov eax,[GUIcurrentcheatviewloc]
    mov ebx,eax
    shl eax,5
    sub eax,ebx
    sub eax,ebx
    sub eax,ebx
    sub eax,ebx
    add [ccheatnpos],eax
    GUIOuttextwin2cheat 12,24
    GUIOuttextwin2cheat 12,24+7
    GUIOuttextwin2cheat 12,24+7*2
    GUIOuttextwin2cheat 12,24+7*3
    GUIOuttextwin2cheat 12,24+7*4
    GUIOuttextwin2cheat 12,24+7*5
    GUIOuttextwin2cheat 12,24+7*6
    GUIOuttextwin2cheat 12,24+7*7
    GUIOuttextwin2cheat 12,24+7*8
    GUIOuttextwin2cheat 12,24+7*9
    GUIOuttextwin2cheat 12,24+7*10
    GUIOuttextwin2cheat 12,24+7*11

    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    mov dword[ccheatnpos],cheatdata
    mov eax,[NumCheats]
    sub eax,[GUIcurrentcheatviewloc]
    dec eax
    mov dword[ccheatnleft],eax
    mov eax,[GUIcurrentcheatviewloc]
    mov ebx,eax
    shl eax,5
    sub eax,ebx
    sub eax,ebx
    sub eax,ebx
    sub eax,ebx
    add [ccheatnpos],eax
    GUIOuttextwin2cheat 11,23
    GUIOuttextwin2cheat 11,23+7
    GUIOuttextwin2cheat 11,23+7*2
    GUIOuttextwin2cheat 11,23+7*3
    GUIOuttextwin2cheat 11,23+7*4
    GUIOuttextwin2cheat 11,23+7*5
    GUIOuttextwin2cheat 11,23+7*6
    GUIOuttextwin2cheat 11,23+7*7
    GUIOuttextwin2cheat 11,23+7*8
    GUIOuttextwin2cheat 11,23+7*9
    GUIOuttextwin2cheat 11,23+7*10
    GUIOuttextwin2cheat 11,23+7*11

    DrawSlideBarWin 7,231,28,[GUIcurrentcheatviewloc],[NumCheats],12,73,GUICStA
    cmp byte[GUICHold],7
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin 7,231,20,GUIIconDataUpArrow
    cmp byte[GUICHold],7
    jne .noholda2
    sub byte[GUIWincoladd],3
.noholda2
    cmp byte[GUICHold],8
    jne .noholdb
    add byte[GUIWincoladd],3
.noholdb
    GUIDisplayIconWin 7,231,101,GUIIconDataDownArrow
    cmp byte[GUICHold],8
    jne .noholdb2
    sub byte[GUIWincoladd],3
.noholdb2

    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 7,5,19,229,19,dl
    DrawGUIWinBox 7,82,128,172,128,dl
    DrawGUIWinBox 7,82,139,196,139,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 7,4,20,3,108,dl
    DrawGUIWinBox 7,81,129,80,136,dl
    DrawGUIWinBox 7,81,140,80,147,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 7,5,109,229,109,dl
    DrawGUIWinBox 7,82,137,172,137,dl
    DrawGUIWinBox 7,82,148,196,148,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 7,174,129,173,136,dl
    DrawGUIWinBox 7,156,140,197,147,dl

    ; Draw boxes
    mov ebx,[GUIcurrentcheatcursloc]
    sub ebx,[GUIcurrentcheatviewloc]
    mov eax,ebx
    lea ebx,[ebx*4]
    add ebx,eax
    add ebx,eax
    add ebx,eax
    add ebx,22
    mov dl,167
    cmp dword[GUIcurrentcheatwin],1
    jne .notopibox
    mov dl,226
    cmp byte[GUIWincoladd],0
    je .nocoladd
    inc dl
.nocoladd
.notopibox
    DrawGUIWinBox 7,82,129,172,136,dl
    mov dl,167
    cmp dword[GUIcurrentcheatwin],2
    jne .nobotibox
    mov dl,226
    cmp byte[GUIWincoladd],0
    je .nocoladd2
    inc dl
.nocoladd2
.nobotibox
    DrawGUIWinBox 7,82,140,196,147,dl
    mov byte[GUItextcolor],223
    GUIOuttextwin2 7,84,132,GUICheatText9
    GUIOuttextwin2 7,84,143,GUICheatTextA
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],222
.zero3
    GUIOuttextwin2 7,83,131,GUICheatText9
    GUIOuttextwin2 7,83,142,GUICheatTextA
    xor eax,eax
    mov al,[GUICheatPosA]
    mov byte[GUICheatText9+eax],0
    xor ebx,ebx
    mov bl,[GUICheatPosB]
    mov byte[GUICheatTextA+ebx],0
    test byte[GUICCFlash],8
    jnz .nound
    cmp dword[GUIcurrentcheatwin],1
    je .win1
    cmp dword[GUIcurrentcheatwin],2
    je .win2
    jmp .nound
.win1
    mov byte[GUICheatText9+eax],'_'
    jmp .nound
.win2
    mov byte[GUICheatTextA+ebx],'_'
.nound

    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[AutoLoadCht],1
    jne .nocheckbox
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox
    GUIDisplayIconWin 7,11,186,[GUITemp]
    ret

SECTION .data
GUICheatText1 db 'ENTER CODE :',0
GUICheatText2 db 'DESCRIPTION:',0
GUICheatText3 db 'ADDRESS CV PV TGL DESCRIPTION',0
GUICheatText4 db 'REMOVE',0
GUICheatText5 db 'TOGGLE',0
GUICheatText6 db 'ADD',0
GUICheatText7 db 'SAVE',0
GUICheatText8 db 'LOAD',0
GUICheatTextI db 'FIX',0
GUICheatText9 db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 14 digits
GUICheatTextA db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 11 digits
GUICheatTextB db '000000 00 00 OFF BLAHBLAH---',0,0,0,0,0,0,0,0,0,0,0,0
GUICheatTextC db '0123456789ABCDEF'
GUICheatTextD db 'VALID CODES : GAME GENIE, PAR, AND GF',0
GUICheatTextE db 'NOTE : YOU MAY HAVE TO RESET THE GAME',0
GUICheatTextF db ' AFTER ENTERING THE CODE. REMEMBER TO',0
GUICheatTextG db ' INSERT THE "-" FOR GAME GENIE CODES',0
GUICheatTextJ db 'AUTO-LOAD .CHT FILE AT GAME LOAD',0
SECTION .bss
GUICheatPosA resb 1
GUICheatPosB resb 1
GUICStA resd 3
GUIcurrentcheatviewloc resd 1
GUIcurrentcheatcursloc resd 1
GUIcurrentcheatwin     resd 1
ccheatnpos             resd 1
ccheatnleft            resd 1
SECTION .text

DrawWindowSearch:
    GUIDrawWindowBox 13,GUISearchDisp
    ret

%macro TextWindowMacro 4
    sub byte[GUItextcolor],15
    GUIOuttextwin2 %1,%2+1,%3+1,%4
    add byte[GUItextcolor],15
    GUIOuttextwin2 %1,%2,%3,%4
%endmacro

CSRemoveFlash:
.loop
    cmp byte[esi],'_'
    je .flash
    cmp byte[esi],0
    je .notflash
    inc esi
    jmp .loop
.flash
    mov byte[esi],0
.notflash
    ret

CSAddFlash:
.loop
    cmp byte[esi],'_'
    je .flash
    cmp byte[esi],0
    je .notflash
    inc esi
    jmp .loop
.notflash
    mov byte[esi],'_'
    mov byte[esi+1],0
.flash
    ret

DisplayGUISearch:
    jmp .startcheat
    call DrawWindowSearch
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    GUIOuttextwin2 13,6,16,GUISrcText1
    GUIOuttextwin2 13,6,26,GUISrcText2
    GUIOuttextwin2 13,6,36,GUISrcText3
    add byte[GUItextcolor],15
    GUIOuttextwin2 13,5,15,GUISrcText1
    GUIOuttextwin2 13,5,25,GUISrcText2
    GUIOuttextwin2 13,5,35,GUISrcText3
    ret
;    DrawGUIButton 13,197,30,239,41,GUICSrcText8+2,40,0,0
    ret
.startcheat
    cmp byte[CheatWinMode],1
    je near Incheatmode
    cmp byte[CheatWinMode],2
    je near Cheatmodeview
    cmp byte[CheatWinMode],3
    je near Cheatmodeadd

    mov dword[GUIwinsizex+13*4],170
    mov dword[GUIwinsizey+13*4],150
    call DrawWindowSearch
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 13,6,16,GUICSrcText1
    GUIOuttextwin2 13,6,101,GUICSrcText8
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 13,5,15,GUICSrcText1
    GUIOuttextwin2 13,5,100,GUICSrcText8

    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero2
    mov byte[GUItextcolor],211
.zero2
    TextWindowMacro 13,25,30,GUICSrcText2
    TextWindowMacro 13,25,40,GUICSrcText3
    TextWindowMacro 13,25,50,GUICSrcText4
    TextWindowMacro 13,25,60,GUICSrcText5
    TextWindowMacro 13,25,75,GUICSrcText6
    TextWindowMacro 13,25,85,GUICSrcText7
    TextWindowMacro 13,25,115,GUICSrcText9
    TextWindowMacro 13,25,125,GUICSrcTextA
    DrawGUIButton 13,95,140,140,152,GUICSrcTextB,50,0,1
    GUIDisplayButtonHole 13,11,28,byte[CheatSrcByteSize],0
    GUIDisplayButtonHole 13,11,38,byte[CheatSrcByteSize],1
    GUIDisplayButtonHole 13,11,48,byte[CheatSrcByteSize],2
    GUIDisplayButtonHole 13,11,58,byte[CheatSrcByteSize],3
    GUIDisplayButtonHole 13,11,73,byte[CheatSrcByteBase],0
    GUIDisplayButtonHole 13,11,83,byte[CheatSrcByteBase],1
    GUIDisplayButtonHole 13,11,113,byte[CheatSrcSearchType],0
    GUIDisplayButtonHole 13,11,123,byte[CheatSrcSearchType],1
    ret
Incheatmode:
    mov dword[GUIwinsizex+13*4],180
    mov dword[GUIwinsizey+13*4],150
    call DrawWindowSearch
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero2
    mov byte[GUItextcolor],211
.zero2
    cmp byte[CheatSearchStatus],1
    jne near CheatSearching
    TextWindowMacro 13,5,20,GUICSrcTextM
    TextWindowMacro 13,5,30,GUICSrcTextN
    TextWindowMacro 13,5,40,GUICSrcTextO
    TextWindowMacro 13,5,50,GUICSrcTextP
    jmp DisplayChtSrcRes.nosearch
CheatSearching:
    cmp byte[CheatSrcSearchType],1
    je near CheatSearchingComp

    ; Exact Value Search
    TextWindowMacro 13,5,20,GUICSrcTextD
    DrawGUIWinBox 13,10,40,80,47,167
    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 13,10,39,80,39,dl  ; 0,-1,0,-1
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 13,9,40,8,47,dl   ; -1,0,-2,0
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 13,10,48,80,48,dl  ; 0,1,0,1
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 13,82,40,81,47,dl ; 2,0,1,0

    test byte[GUICCFlash],8
    jnz .nound
    mov esi,CSInputDisplay
    call CSRemoveFlash
.nound

    mov byte[GUItextcolor],223
    cmp byte[CSOverValue],1
    jne .notdark
    mov byte[GUItextcolor],202
.notdark
    GUIOuttextwin2 13,13,42,CSInputDisplay
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    cmp byte[CSOverValue],1
    jne .notdark2
    mov byte[GUItextcolor],207
.notdark2
    GUIOuttextwin2 13,12,41,CSInputDisplay
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero2
    mov byte[GUItextcolor],211
.zero2
    TextWindowMacro 13,5,65,GUICSrcTextT

    mov esi,CSInputDisplay
    call CSAddFlash

    xor eax,eax
    mov al,[CheatSrcByteSize]
    mov eax,[SrcMask+eax*4]
    mov esi,GUICSrcTextQ
    cmp byte[CheatSrcByteBase],1
    je .hex
    call convertnum
    jmp .dec
.hex
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc ecx
    call converthex
.dec
    TextWindowMacro 13,71,65,GUICSrcTextQ
    jmp DisplayChtSrcRes

CheatSearchingComp:
    ; Comparative search
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 13,6,16,GUICSrcTextE
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],164
.zero3
    GUIOuttextwin2 13,5,15,GUICSrcTextE
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero2
    mov byte[GUItextcolor],211
.zero2
    TextWindowMacro 13,25,35,GUICSrcTextF
    TextWindowMacro 13,25,45,GUICSrcTextG
    TextWindowMacro 13,25,55,GUICSrcTextH
    TextWindowMacro 13,25,65,GUICSrcTextHb
    GUIDisplayButtonHole 13,11,33,byte[CheatCompareValue],0
    GUIDisplayButtonHole 13,11,43,byte[CheatCompareValue],1
    GUIDisplayButtonHole 13,11,53,byte[CheatCompareValue],2
    GUIDisplayButtonHole 13,11,63,byte[CheatCompareValue],3
    jmp DisplayChtSrcRes


FindChtSrcRes:
    inc edi
    ; Calculate search results
    mov eax,dword[vidbuffer]
    add eax,129600+65536*2
    mov ecx,16384
    xor esi,esi
    xor ebx,ebx
.searchloop
    mov dl,[eax]
    mov dh,8
.sloop2
    test dl,1
    jz .notset
    inc ebx
    dec edi
    jz .found
.notset
    inc esi
    shr dl,1
    dec dh
    jnz .sloop2
    inc eax
    dec ecx
    jnz .searchloop
    mov eax,ebx
    ret
.found
    mov [CSStartEntry],esi
    jmp .notset

DisplayChtSrcRes:
    DrawGUIButton 13,120,140,170,152,GUICSrcTextI,53,0,1
.nosearch
    DrawGUIButton 13,10,140,60,152,GUICSrcTextJ,51,0,1
    DrawGUIButton 13,70,140,110,152,GUICSrcTextL,52,0,1
    xor edi,edi
    call FindChtSrcRes
    mov esi,GUICSrcTextQ
    call convertnum
    TextWindowMacro 13,12,125,GUICSrcTextK
    TextWindowMacro 13,97,125,GUICSrcTextQ
    mov dword[GUIcurrentchtsrcviewloc],0
    mov dword[GUIcurrentchtsrccursloc],0
    ret

Cheatmodeview:
    mov byte[GUICSrcTextS+12],32
    cmp byte[CheatSrcByteSize],3
    jne .yesprev
    cmp byte[CheatSrcByteBase],0
    jne .yesprev
    mov byte[GUICSrcTextS+12],0
.yesprev

    mov dword[GUIwinsizex+13*4],185
    mov dword[GUIwinsizey+13*4],150
    call DrawWindowSearch
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    TextWindowMacro 13,10,12,GUICSrcTextS
    DrawGUIWinBox 13,5,20,171,108,167

    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 13,5,19,171,19,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 13,4,20,3,108,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 13,5,109,171,109,dl

    mov edi,[GUIcurrentchtsrcviewloc]
    call FindChtSrcRes
    mov [NumCheatSrc],eax
    mov dword[ccheatnpos],esi
    ; Display Window Contents

    mov byte[GUItextcolor],223
    mov eax,[NumCheatSrc]
    sub eax,[GUIcurrentchtsrcviewloc]
    cmp eax,12
    jbe .notof
    mov eax,12
.notof
    mov dword[ccheatnleft],eax
    mov dword[ccheatnleftb],eax

    mov dword[CheatSearchYPos],24
    mov dword[CheatSearchXPos],10
    mov eax,[CSStartEntry]
    mov [CSCurEntry],eax

    cmp dword[ccheatnleft],0
    je near .noentry
    mov byte[CheatLooped],0
    mov ebx,[GUIcurrentchtsrccursloc]
    sub ebx,[GUIcurrentchtsrcviewloc]
    mov [curentryleft],ebx
    mov eax,ebx
    lea ebx,[ebx*8]
    sub ebx,eax
    add ebx,22
    DrawGUIWinBox2 13,5,171,7,224
.nextdisplay
    cmp dword[curentryleft],0
    jne .notzero
    mov eax,[CSCurEntry]
    mov [curentryval],eax
.notzero
    dec dword[curentryleft]

    mov esi,GUICSrcTextQ
    mov ecx,3
    mov eax,[CSCurEntry]
    add eax,7E0000h
    call converthex
    GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextQ

    mov esi,GUICSrcTextQ
    mov ebx,[wramdata]
    add ebx,[CSCurEntry]
    mov eax,[ebx]
    cmp byte[CheatSrcByteBase],0
    je .dec
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc ecx
    call converthex
    jmp .hex
.dec
    xor ebx,ebx
    mov bl,[CheatSrcByteSize]
    and eax,[SrcMask+ebx*4]
    call convertnum
.hex
    add dword[CheatSearchXPos],42
    GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextQ

    add dword[CheatSearchXPos],60
    cmp byte[GUICSrcTextS+12],0
    je near .noprevious
    mov esi,GUICSrcTextQ
    mov ebx,dword[vidbuffer]
    add ebx,129600
    add ebx,[CSCurEntry]
    mov eax,[ebx]
    cmp byte[CheatSrcByteBase],0
    je .decb
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc ecx
    call converthex
    jmp .hexb
.decb
    xor ebx,ebx
    mov bl,[CheatSrcByteSize]
    and eax,[SrcMask+ebx*4]
    call convertnum
.hexb
    GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextQ
.noprevious
    sub dword[CheatSearchXPos],102
    add dword[CheatSearchYPos],7
    ; Search for next entry
.nextentry
    inc dword[CSCurEntry]
    mov eax,[CSCurEntry]
    mov ecx,eax
    and ecx,7
    mov edx,1
    shr eax,3
    shl edx,cl
    add eax,dword[vidbuffer]
    add eax,129600+65536*2
    test byte[eax],dl
    jz .nextentry
    dec dword[ccheatnleft]
    jnz near .nextdisplay
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    mov dword[CheatSearchYPos],23
    mov dword[CheatSearchXPos],11
    mov eax,[CSStartEntry]
    mov [CSCurEntry],eax
    mov eax,dword[ccheatnleftb]
    mov dword[ccheatnleft],eax
    inc byte[CheatLooped]
    cmp byte[CheatLooped],2
    jne near .nextdisplay
.noentry

    ; win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
    DrawSlideBarWin 13,173,28,[GUIcurrentchtsrcviewloc],[NumCheatSrc],12,73,GUICSStA
    cmp byte[GUICHold],11
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin 13,173,20,GUIIconDataUpArrow
    cmp byte[GUICHold],11
    jne .noholda2
    sub byte[GUIWincoladd],3
.noholda2
    cmp byte[GUICHold],12
    jne .noholdb
    add byte[GUIWincoladd],3
.noholdb
    GUIDisplayIconWin 13,173,101,GUIIconDataDownArrow
    cmp byte[GUICHold],12
    jne .noholdb2
    sub byte[GUIWincoladd],3
.noholdb2
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],211
.zero3
    DrawGUIButton 13,70,140,130,152,GUICSrcTextR,54,0,1
    DrawGUIButton 13,140,140,180,152,GUICSrcTextU,55,0,1
    ret

Cheatmodeadd:
    mov dword[GUIwinsizex+13*4],170
    mov dword[GUIwinsizey+13*4],165
    call DrawWindowSearch

    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],211
.zero3

    TextWindowMacro 13,5,20,GUICSrcTextV
    TextWindowMacro 13,5,45,GUICSrcTextW
    TextWindowMacro 13,5,70,GUICSrcTextX
    TextWindowMacro 13,22,143,GUICSrcTextY

    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[CheatUpperByteOnly],1
    jne .nocheck
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck
    GUIDisplayIconWin 13,8,139,[GUITemp]

    DrawGUIWinBox 13,10,30,80,37,167
    DrawGUIWinBox 13,10,55,126,62,167
    DrawGUIWinBox 13,10,80,80,120,167
    ; Draw border
    mov dword[GUIWincol],148+10
    cmp byte[cwindrawn],1
    jne .noone2
    mov dword[GUIWincol],148+5
.noone2
    cmp byte[cwindrawn],0
    jne .nozero2
    mov dword[GUIWincol],148
.nozero2
    mov dl,[GUIWincol]
    DrawGUIWinBox 13,10,29,80,29,dl  ; 0,-1,0,-1
    DrawGUIWinBox 13,10,54,126,54,dl  ; 0,-1,0,-1
    DrawGUIWinBox 13,10,79,80,79,dl  ; 0,-1,0,-1
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 13,9,30,8,37,dl   ; -1,0,-2,0
    DrawGUIWinBox 13,9,55,8,62,dl   ; -1,0,-2,0
    DrawGUIWinBox 13,9,80,8,120,dl   ; -1,0,-2,0
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 13,10,38,80,38,dl  ; 0,1,0,1
    DrawGUIWinBox 13,10,63,126,63,dl  ; 0,1,0,1
    DrawGUIWinBox 13,10,121,80,121,dl  ; 0,1,0,1
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 13,82,30,81,37,dl ; 2,0,1,0
    DrawGUIWinBox 13,128,55,127,62,dl ; 2,0,1,0
    DrawGUIWinBox 13,82,80,81,120,dl ; 2,0,1,0

    DrawGUIButton 13,60,155,120,167,GUICSrcTextR,56,0,1
    DrawGUIButton 13,130,155,160,167,GUICSrcTextU,57,0,1

    TextWindowMacro 13,5,130,GUICSrcTextT
    xor eax,eax
    mov al,[CheatSrcByteSize]
    mov eax,[SrcMask+eax*4]
    mov esi,GUICSrcTextQ
    cmp byte[CheatSrcByteBase],1
    je .hex
    call convertnum
    jmp .dec
.hex
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc ecx
    call converthex
.dec
    TextWindowMacro 13,71,130,GUICSrcTextQ

    cmp byte[CurCStextpos],0
    jne near .nofirstbox
    test byte[GUICCFlash],8
    jnz .nound
.nofirstbox
    mov esi,CSInputDisplay
    call CSRemoveFlash
.nound
    mov byte[GUItextcolor],223
    cmp byte[CSOverValue],1
    jne .notdark
    mov byte[GUItextcolor],202
.notdark
    GUIOuttextwin2 13,13,32,CSInputDisplay
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],222
.zero4
    cmp byte[CSOverValue],1
    jne .notdark2
    mov byte[GUItextcolor],207
.notdark2
    GUIOuttextwin2 13,12,31,CSInputDisplay
    mov esi,CSInputDisplay
    call CSAddFlash

    cmp byte[CurCStextpos],1
    jne .noundb
    test byte[GUICCFlash],8
    jnz .noundb
    mov esi,CSDescDisplay
    call CSAddFlash
.noundb
    mov byte[GUItextcolor],223
    GUIOuttextwin2 13,13,57,CSDescDisplay
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4b
    mov byte[GUItextcolor],222
.zero4b
    GUIOuttextwin2 13,12,56,CSDescDisplay
    mov esi,CSDescDisplay
    call CSRemoveFlash

    cmp byte[CSOverValue],1
    je near .nodisplay
    cmp byte[CSInputDisplay],'_'
    je near .nodisplay

    mov dword[CheatSearchYPos],83
    mov eax,[curentryval]
    mov [curaddrvalcs],eax
    mov eax,[CSCurValue]
    mov [curvaluecs],eax
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc cl
    cmp byte[CheatUpperByteOnly],0
    je .cspardisploop
    mov cl,1
.cspardispagain
    cmp dword[curvaluecs],0FFh
    jbe .cspardisploop
    shr dword[curvaluecs],8
    inc dword[curaddrvalcs]
    jmp .cspardispagain
.cspardisploop
    push ecx
    mov esi,GUICSrcTextQ
    mov ecx,3
    mov eax,[curaddrvalcs]
    add eax,7E0000h
    call converthex
    mov esi,GUICSrcTextQ+6
    mov ecx,1
    mov al,[curvaluecs]
    call converthex
    shr dword[curvaluecs],8
    mov byte[GUItextcolor],223
    inc dword[CheatSearchYPos]
    GUIOuttextwin2 13,13,[CheatSearchYPos],GUICSrcTextQ
    dec dword[CheatSearchYPos]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero4c
    mov byte[GUItextcolor],222
.zero4c
    GUIOuttextwin2 13,12,[CheatSearchYPos],GUICSrcTextQ
    add dword[CheatSearchYPos],10
    inc dword[curaddrvalcs]
    pop ecx
    dec ecx
    jnz near .cspardisploop

.nodisplay
    ret

;CheatSrcByteSize db 0
;CheatSrcByteBase db 0
;CheatSrcSearchType db 0
SECTION .bss
CheatWinMode resb 1
CheatSearchStatus resb 1
CheatCompareValue resd 1
NumCheatSrc       resd 1
GUIcurrentchtsrcviewloc resd 1
GUIcurrentchtsrccursloc resd 1
CSStartEntry      resd 1
CSCurEntry      resd 1
CheatSearchXPos   resd 1
CheatSearchYPos   resd 1
CheatLooped       resd 1
ccheatnleftb      resd 1
CurCStextpos      resd 1

SECTION .data
SrcMask dd 0FFh,0FFFFh,0FFFFFFh,0FFFFFFFFh
CSInputDisplay db '_',0,'         ',0
CSDescDisplay  db '                  ',0,0

SECTION .bss
curentryval resd 1
curentryleft resd 1
curaddrvalcs resd 1
curvaluecs resd 1

GUICSStA resd 3

SECTION .data
GUISrcText1 db 'CHEAT SEARCH',0
GUISrcText2 db 'HAS NOT BEEN',0
GUISrcText3 db 'IMPLEMENTED YET',0
GUICSrcText1 db 'SELECT SIZE AND FORMAT:',0
GUICSrcText2 db '1 BYTE  [0..255]',0
GUICSrcText3 db '2 BYTES [0..65535]',0
GUICSrcText4 db '3 BYTES [0..16777215]',0
GUICSrcText5 db '4 BYTES [0..4294967295]',0
GUICSrcText6 db 'DEC (BASE 10)',0
GUICSrcText7 db 'HEX (BASE 16)',0
GUICSrcText8 db 'SELECT SEARCH TYPE:',0
GUICSrcText9 db 'EXACT VALUE SEARCH',0
GUICSrcTextA db 'COMPARATIVE SEARCH',0
GUICSrcTextB db 'START',0
GUICSrcTextD db 'ENTER VALUE:',0
GUICSrcTextE db 'SELECT COMPARISON:',0
GUICSrcTextF db 'NEW VALUE IS > OLD VALUE',0
GUICSrcTextG db 'NEW VALUE IS < OLD VALUE',0
GUICSrcTextH db 'NEW VALUE IS = OLD VALUE',0
GUICSrcTextHb db 'NEW VALUE IS != OLD VALUE',0
GUICSrcTextI db 'SEARCH',0
GUICSrcTextJ db 'RESTART',0
GUICSrcTextK db '# OF RESULTS:',0
GUICSrcTextL db 'VIEW',0
GUICSrcTextQ db '          ',0
GUICSrcTextM db 'NOW RETURN TO YOUR GAME',0
GUICSrcTextN db 'AND COME BACK WHEN ',0
GUICSrcTextO db 'THE NEXT SEARCH',0
GUICSrcTextP db 'SHOULD BE PROCESSED',0
GUICSrcTextR db 'RETURN',0
GUICSrcTextS db 'ADDR   VALUE     PVALUE',0
GUICSrcTextT db 'MAX VALUE:',0
GUICSrcTextU db 'ADD',0
GUICSrcTextV db 'ENTER NEW VALUE:',0
GUICSrcTextW db 'ENTER CHEAT DESCRIPTION:',0
GUICSrcTextX db 'PAR CODE EQUIVALENT:',0
GUICSrcTextY db 'USE ONLY UPPER BYTE',0
SECTION .text


NEWSYM NetAddChar             ; dl = color, dh = char
    mov eax,ViewBuffer
    add eax,[CCursLoc]
    mov ebx,[CCursLoc]
    sub ebx,[CViewLoc]
    cmp ebx,32*12
    jbe .okay
    mov ebx,[CCursLoc]
    sub ebx,32*11
    and ebx,0FFE0h
    mov [CViewLoc],ebx
.okay
    cmp dword[CCursLoc],32*48
    jb .notbottom
    call .movelistup
    mov ebx,[CCursLoc]
    sub ebx,32*12
    and ebx,0FFE0h
    mov [CViewLoc],ebx
    jmp NetAddChar
.notbottom
    test dword[CCursLoc],1Fh
    jnz .nobeginning
    cmp dl,' '
    je near .skipaddchar
    mov byte[eax],dh
    inc dword[CCursLoc]
    inc eax
.nobeginning
    mov ebx,[CCursLoc]
    and ebx,1Fh
    cmp ebx,1Eh
    jne near .noend
    add dword[CCursLoc],2
    cmp dword[CCursLoc],32*48
    jb .notbottom2
    call .movelistup
.notbottom2
    mov ebx,[CCursLoc]
    add ebx,ViewBuffer
    sub ebx,3
    mov eax,12
    cmp byte[ebx],' '
    je .notfoundspace
.loop3
    cmp byte[ebx],' '
    je .foundspace
    dec ebx
    dec eax
    jnz .loop3
    jmp .notfoundspace
.foundspace
    mov ecx,[CCursLoc]
    add ecx,ViewBuffer
    inc ebx
    cmp byte[ebx],0
    je .notfoundspace
    mov [ecx],dh
    inc ecx
    mov byte[ecx],' '
    inc ecx
    add dword[CCursLoc],2
.loop2
    cmp byte[ebx],0
    je .skipaddingspace
    mov al,[ebx]
    mov [ecx],al
    mov byte[ecx+1],0
    mov byte[ebx],' '
    inc ebx
    inc ecx
    inc dword[CCursLoc]
    jmp .loop2
    jmp .skipaddingspace
.notfoundspace
    mov ecx,[CCursLoc]
    add ecx,ViewBuffer
    mov [ecx],dh
    inc ecx
    mov byte[ecx],' '
    inc ecx
    add dword[CCursLoc],2
.skipaddingspace
    jmp NetAddChar
.noend
    cmp dl,13
    je .ret
    cmp dl,10
    je .next
    mov byte[eax],dl
    mov byte[eax+1],0
    inc dword[CCursLoc]
    ret
.next
    add dword[CCursLoc],32
.ret
    and dword[CCursLoc],0FFE0h
.skipaddchar
    mov ebx,[CViewLoc]
    shr ebx,5
    mov [NViewLoc],ebx
    ret

.movelistup
    ; move list up by 32 spaces
    mov ebx,ViewBuffer
    mov ecx,32*47
.loop
    mov al,[ebx+32]
    mov [ebx],al
    inc ebx
    dec ecx
    jnz .loop
    and dword[CCursLoc],0FFE0h
    mov dword[ViewBuffer+32*47],0
    sub dword[CCursLoc],32
    ret


section .bss
CCursLoc resd 1
CViewLoc resd 1
NViewLoc resd 1
TViewLoc resd 1
section .text


DisplayNetOptns:

    mov dword[GUIwinsizex+8*4],7*16
    mov dword[GUIwinsizey+8*4],57
    mov dword[GUINetDisp],'IPX '
    mov dword[GUINetDisp+4],'    '
    mov byte[GUINetDisp+8],0
    cmp byte[per2exec],99
    je .netokay
    mov dword[GUIwinsizex+8*4],7*16
    mov dword[GUIwinsizey+8*4],40
    mov dword[GUINetDisp],'ERRO'
    mov dword[GUINetDisp+4],'R!  '
    jmp .nomodem
.netokay
%ifdef __MSDOS__
    cmp byte[CNetType],8
    jne .noipxinvalid
    mov dword[GUIwinsizex+8*4],7*16
    mov dword[GUIwinsizey+8*4],30
    mov dword[GUINetDisp],'ERRO'
    mov dword[GUINetDisp+4],'R!  '
.noipxinvalid
%endif

    cmp byte[CNetType],22
    je .yesstate
    cmp byte[CNetType],21
    jne .nostate
.yesstate
    mov dword[GUIwinsizex+8*4],150
    mov dword[GUIwinsizey+8*4],59
    mov dword[GUINetDisp],'STAT'
    mov dword[GUINetDisp+4],'E TR'
    mov dword[GUINetDisp+8],'ANSF'
    mov dword[GUINetDisp+12],'ER  '
    mov dword[GUINetDisp+14],0
.nostate

    cmp byte[CNetType],20
    jne .nochat
    mov dword[GUIwinsizex+8*4],188
    mov dword[GUIwinsizey+8*4],179
    mov dword[GUINetDisp],'CHAT'
    mov dword[GUINetDisp+4],0
.nochat
    cmp byte[CNetType],15
    je .yesterm
    cmp byte[CNetType],12
    je .yesterm
    cmp byte[CNetType],11
    je .yesterm
    cmp byte[CNetType],10
    jne .noterm
.yesterm
    mov dword[GUIwinsizex+8*4],188
    mov dword[GUIwinsizey+8*4],144
    mov dword[GUINetDisp],'TERM'
    mov dword[GUINetDisp+4],'INAL'
    mov byte[GUINetDisp+8],0
.noterm
    cmp byte[CNetType],4
    jne .notcpip
    ; Your IP:
    ; Start as Server
    ; Client Options:
    ; IP  :
    ; Port:
    ; Connect
    mov dword[GUIwinsizex+8*4],201
    mov dword[GUIwinsizey+8*4],98
    mov dword[GUINetDisp],'INTE'
    mov dword[GUINetDisp+4],'RNET'
    mov byte[GUINetDisp+8],0
    cmp byte[NetPlayNoMore],1
    jne .notcpip
    mov dword[GUIwinsizex+8*4],201
    mov dword[GUIwinsizey+8*4],48
.notcpip
%ifdef __MSDOS__
    cmp byte[CNetType],3
    je .yesmodem
    cmp byte[CNetType],1
    jne .nomodem2
    jmp .skipmodem3
.yesmodem
    mov dword[GUIwinsizex+8*4],188
    mov dword[GUIwinsizey+8*4],45
    jmp .skipmodem2
.skipmodem3
    mov dword[GUIwinsizex+8*4],95
    mov dword[GUIwinsizey+8*4],70
.skipmodem2
    mov dword[GUINetDisp],'MODE'
    mov dword[GUINetDisp+4],'M   '
    mov byte[GUINetDisp+8],0
.nomodem2
    cmp byte[CNetType],2
    jne .nomodem
    mov dword[GUIwinsizex+8*4],188
    mov dword[GUIwinsizey+8*4],154
    mov dword[GUINetDisp],'MODE'
    mov dword[GUINetDisp+4],'M SE'
    mov dword[GUINetDisp+8],'TTIN'
    mov word[GUINetDisp+12],'GS'
    mov byte[GUINetDisp+14],0
%endif
.nomodem
    GUIDrawWindowBox 8,GUINetDisp
    cmp byte[per2exec],99
    jne near DisplayNetOptnsInvalid

    cmp byte[CNetType],22
    je near DisplayNetTCPIPState
    cmp byte[CNetType],21
    je near DisplayNetTCPIPState

%ifdef __MSDOS__
    cmp byte[CNetType],1
    je near DisplayNetOptnsModem
    cmp byte[CNetType],2
    je near DisplayNetOptnsModemConfig
    cmp byte[CNetType],3
    je near DisplayNetOptnsModemDial
%endif
    cmp byte[CNetType],4
    je near DisplayNetOptnsTCPIP
    cmp byte[CNetType],10
    je near DisplayNetOptnsModemTerm
    cmp byte[CNetType],11
    je near DisplayNetOptnsModemTerm
    cmp byte[CNetType],12
    je near DisplayNetOptnsModemTerm
    cmp byte[CNetType],15
    je near DisplayNetOptnsModemTerm
    cmp byte[CNetType],20
    je near DisplayNetOptnsChatWindow
%ifdef __MSDOS__
    cmp byte[CNetType],7
    je near DisplayNetIPXStart
    cmp byte[CNetType],8
    je near DisplayNetIPXInvalid

    jmp DisplayNetIPXStart

    cmp ax,0
    je .ipxokay
    mov byte[CNetType],8
    jmp DisplayNetIPXInvalid
.ipxokay
    cmp byte[CNetType],7
    jmp DisplayNetIPXStart

    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    mov dword[GUIEditStringcWin],0
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,16,GUINetText1
    GUIOuttextwin2 8,6,26,GUINetText2
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUINetText1
    GUIOuttextwin2 8,5,25,GUINetText2
    ret
%endif

DisplayNetTCPIPState:
    DrawGUIButton 8,73,48,127,59,GUITCPStateText3,64,0,0
    cmp byte[CNetType],22
    je near .receive
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,16,GUITCPStateText1
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUITCPStateText1
    jmp .send
.receive
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,16,GUITCPStateText2
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUITCPStateText2
.send
    DrawGUIWinBox 8,10,30,120,40,167
    cmp dword[NetStateTotal],0
    je near .nobox
    mov eax,[NetStateTotal]
    sub eax,[NetStateSize]
    mov ebx,110
    mul ebx
    mov ebx,[NetStateTotal]
    div ebx
    add eax,10
    cmp eax,10
    je near .nobox
    push eax
    inc eax
    DrawGUIWinBox3 8,10,30,eax,40,226
    pop eax
    DrawGUIWinBox3 8,10,30,eax,40,224
.nobox

    mov dl,[GUIWincol]
    DrawGUIWinBox 8,10,29,120,29,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 8,9,30,8,40,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 8,10,41,120,41,dl  ; 0,1,0,1
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 8,122,30,121,40,dl ; 2,0,1,0
    ret

SECTION .data
GUITCPStateText1 db 'SENDING STATUS',0
GUITCPStateText2 db 'RECEIVING STATUS',0
GUITCPStateText3 db 'CANCEL',0
SECTION .text

%ifdef __MSDOS__
DisplayNetIPXStart:
    DrawGUIButton 8,53,48,87,59,GUINetTextZ,34,0,0
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,16,GUINetText1c
    GUIOuttextwin2 8,6,26,GUINetText2c
    GUIOuttextwin2 8,6,36,GUINetText3c
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUINetText1c
    GUIOuttextwin2 8,5,25,GUINetText2c
    GUIOuttextwin2 8,5,35,GUINetText3c
    ret

DisplayNetIPXInvalid:
    ret
%endif

DisplayNetOptnsTCPIP:
    ; Your IP:
    ; Port:
    ; Start as Server
    ; Client Options:
    ; IP  :
    ; Connect

    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15

    cmp byte[NetPlayNoMore],1
    jne near .tcpip
    GUIOuttextwin2 8,6,16,GUINetTextr1
    GUIOuttextwin2 8,6,36,GUINetTextr2
    GUIOuttextwin2 8,6,46,GUINetTextr3
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUINetTextr1
    GUIOuttextwin2 8,5,35,GUINetTextr2
    GUIOuttextwin2 8,5,45,GUINetTextr3
    ret
.tcpip
    GUIOuttextwin2 8,6,16,GUINetTextf2
    GUIOuttextwin2 8,6,26,GUINetTextf3
    GUIOuttextwin2 8,6,51,GUINetTexth2
    GUIOuttextwin2 8,6,61,GUINetTexti2
    GUIOuttextwin2 8,18,98,GUINetTextp2
    GUIOuttextwin2 8,6,88,hostname
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUINetTextf2
    GUIOuttextwin2 8,5,25,GUINetTextf3
    DrawGUIButton 8,5,35,129,46,GUINetTextg2,25,0,0
    GUIOuttextwin2 8,5,50,GUINetTexth2
    GUIOuttextwin2 8,5,60,GUINetTexti2
    DrawGUIButton 8,5,70,129,81,GUINetTextj2,26,0,0
    DrawGUIWinBox 8,55,13,95,21,167
    DrawGUIWinBox 8,55,23,119,31,167
    DrawGUIWinBox 8,26,58,191,66,167
    GUIOuttextwin2 8,17,97,GUINetTextp2
    GUIOuttextwin2 8,5,87,hostname
   
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero2
    mov byte[GUItextcolor],222
.zero2
    GUIOuttextwin2 8,59,15,GUINetTextl2
    GUIOuttextwin2 8,30,60,TCPIPAddress
    GUIOuttextwin2 8,59,25,ChatNick
    mov byte[GUINetTextn2],'_'
    test byte[GUINetTextm2+2],8
    jz .yesus
    mov byte[GUINetTextn2],' '
.yesus
    xor eax,eax
    mov al,[GUINetTextm2+1]
    mov ebx,6
    mul ebx
    cmp byte[GUINetTextm2],1
    je near .cursor2
    cmp byte[GUINetTextm2],2
    je near .cursor3
    add eax,30
    GUIOuttextwin2 8,eax,61,GUINetTextn2
    jmp .cursor1
.cursor2
    xor eax,eax
    mov ebx,GUINetTextl2
.loopc
    cmp byte[ebx],0
    je .donec
    inc ebx
    add eax,6
    jmp .loopc
.donec
    add eax,59
    GUIOuttextwin2 8,eax,15,GUINetTextn2
    jmp .cursor1
.cursor3
    xor eax,eax
    mov ebx,ChatNick
.loopc2
    cmp byte[ebx],0
    je .donec2
    inc ebx
    add eax,6
    jmp .loopc2
.donec2
    add eax,59
    GUIOuttextwin2 8,eax,25,GUINetTextn2
.cursor1

    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[UDPConfig],1
    jne .nocheckm
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckm
    GUIDisplayIconWin 8,5,93,[GUITemp]
    ret

DisplayNetOptnsInvalid:
    mov byte[CNetType],255
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    mov dword[GUIEditStringcWin],0
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,16,GUINetText1b
    GUIOuttextwin2 8,6,26,GUINetText2b
    GUIOuttextwin2 8,6,36,GUINetText3b
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,15,GUINetText1b
    GUIOuttextwin2 8,5,25,GUINetText2b
    GUIOuttextwin2 8,5,35,GUINetText3b
    ret
%ifdef __MSDOS__
DisplayNetOptnsModem:
    mov dword[GUIEditStringcWin],0
    DrawGUIButton 8,5,15,89,26,GUINetTextG,21,0,0
    DrawGUIButton 8,5,30,89,41,GUINetTextH,22,0,0
    DrawGUIButton 8,5,45,89,56,GUINetTexth,23,0,0
    DrawGUIButton 8,5,60,89,71,GUINetTextI,24,0,0
    ret
DisplayNetOptnsModemConfig:
    cmp byte[ModemInitStat],0
    je .nodeinitmodem
;    call DeInitModem
    mov byte[ModemInitStat],0
.nodeinitmodem
    DrawGUIWinBox 8,31,70,50,78,167
    DrawGUIWinBox 8,21,125,175,133,167
    DrawGUIWinBox 8,21,136,175,144,167
    mov byte[GUINetText8],'0'
    mov al,[ComIRQ]
    cmp byte[ComIRQ],10
    jb .no10
    mov byte[GUINetText8],'1'
    sub al,10
.no10
    add al,'0'
    mov [GUINetText8+1],al
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,21,28,GUINetText3
    GUIOuttextwin2 8,21,38,GUINetText4
    GUIOuttextwin2 8,21,48,GUINetText5
    GUIOuttextwin2 8,21,58,GUINetText6
    GUIOuttextwin2 8,6,73,GUINetText7
    GUIOuttextwin2 8,21,88,GUINetTextC
    GUIOuttextwin2 8,21,98,GUINetTextD
    GUIOuttextwin2 8,106,28,GUINetTextJ
    GUIOuttextwin2 8,106,38,GUINetTextK
    GUIOuttextwin2 8,106,48,GUINetTextL
    GUIOuttextwin2 8,106,58,GUINetTextM
    GUIOuttextwin2 8,106,68,GUINetTextN
    GUIOuttextwin2 8,106,78,GUINetTextO
    GUIOuttextwin2 8,106,88,GUINetTextP
    GUIOuttextwin2 8,6,128,GUINetTexte
    GUIOuttextwin2 8,6,139,GUINetTextf
    GUIOuttextwin2 8,26,152,GUINetTextX
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,20,27,GUINetText3
    GUIOuttextwin2 8,20,37,GUINetText4
    GUIOuttextwin2 8,20,47,GUINetText5
    GUIOuttextwin2 8,20,57,GUINetText6
    GUIOuttextwin2 8,5,72,GUINetText7
    GUIOuttextwin2 8,20,87,GUINetTextC
    GUIOuttextwin2 8,20,97,GUINetTextD
    GUIOuttextwin2 8,105,27,GUINetTextJ
    GUIOuttextwin2 8,105,37,GUINetTextK
    GUIOuttextwin2 8,105,47,GUINetTextL
    GUIOuttextwin2 8,105,57,GUINetTextM
    GUIOuttextwin2 8,105,67,GUINetTextN
    GUIOuttextwin2 8,105,77,GUINetTextO
    GUIOuttextwin2 8,105,87,GUINetTextP
    GUIOuttextwin2 8,5,127,GUINetTexte
    GUIOuttextwin2 8,5,138,GUINetTextf
    GUIOuttextwin2 8,25,151,GUINetTextX
    DrawGUIButton 8,55,69,63,79,GUINetText9,25,-2,0
    DrawGUIButton 8,66,69,74,79,GUINetTextA,26,-2,0
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 8,6,16,GUINetTextR
    GUIOuttextwin2 8,6,113,GUINetTextE
    GUIOuttextwin2 8,91,16,GUINetTextQ
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],164
.zero
    GUIOuttextwin2 8,5,15,GUINetTextR
    GUIOuttextwin2 8,5,112,GUINetTextE
    GUIOuttextwin2 8,90,15,GUINetTextQ
    mov byte[GUItextcolor],223
    GUIOuttextwin2 8,36,73,GUINetText8
    GUIOuttextwin2 8,25,128,GUIInitSt1
    GUIOuttextwin2 8,25,139,GUIInitSt2
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],222
.zero3
    GUIOuttextwin2 8,35,72,GUINetText8
    GUIOuttextwin2 8,24,127,GUIInitSt1
    GUIOuttextwin2 8,24,138,GUIInitSt2
    GUIDisplayButtonHole 8,9,25,byte[ComNum],1
    GUIDisplayButtonHole 8,9,35,byte[ComNum],2
    GUIDisplayButtonHole 8,9,45,byte[ComNum],3
    GUIDisplayButtonHole 8,9,55,byte[ComNum],4
    GUIDisplayButtonHole 8,94,25,byte[BaudRate],0
    GUIDisplayButtonHole 8,94,35,byte[BaudRate],1
    GUIDisplayButtonHole 8,94,45,byte[BaudRate],2
    GUIDisplayButtonHole 8,94,55,byte[BaudRate],3
    GUIDisplayButtonHole 8,94,65,byte[BaudRate],4
    GUIDisplayButtonHole 8,94,75,byte[BaudRate],5
    GUIDisplayButtonHole 8,94,85,byte[BaudRate],6
    GUIDisplayButtonHole 8,9,85,byte[GUIDialSt+3],'T'
    GUIDisplayButtonHole 8,9,95,byte[GUIDialSt+3],'P'
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[FossilUse],1
    jne .nocheckm
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckm
    GUIDisplayIconWin 8,12,147,[GUITemp]
    ret
DisplayNetOptnsModemDial:
    DrawGUIWinBox 8,5,25,175,33,167
    DrawGUIButton 8,141,38,175,49,GUINetTextT,28,0,0
    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,6,14,GUINetTextF
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,5,13,GUINetTextF
    mov byte[GUItextcolor],223
    GUIOuttextwin2 8,9,28,DialNumber
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],222
.zero3
    GUIOuttextwin2 8,8,27,DialNumber
    mov dword[GUIEditString],DialNumber
    mov dword[GUIEditStringmLen],27
;    mov dword[GUIEditStringmLenM],27
    mov dword[GUIEditStringcWin],1
    ret
%endif
DisplayNetOptnsModemTerm:
    DrawGUIWinBox 8,6,15,181,127,167
    ; draw borders
    mov dl,[GUIWincol]
    DrawGUIWinBox 8,6,14,181,14,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 8,5,15,4,127,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 8,6,128,181,128,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 8,183,15,182,127,dl

    ; cancel button
    DrawGUIButton 8,120,135,181,146,GUINetTextS,27,0,0
    ; draw text
    mov dword[CurCStringPos],ViewBuffer+1
    mov ebx,[CViewLoc]
    add dword[CurCStringPos],ebx
    mov byte[GUItextcolor],223
    GUIOuttextwin2 8,9,28-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,36-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,44-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,52-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,60-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,68-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,76-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,84-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,92-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,100-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,108-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,116-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,124-5,[CurCStringPos]
    add dword[CurCStringPos],32
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],222
.zero3
    mov dword[CurCStringPos],ViewBuffer+1
    mov ebx,[CViewLoc]
    add dword[CurCStringPos],ebx
    GUIOuttextwin2 8,8,27-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,35-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,43-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,51-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,59-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,67-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,75-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,83-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,91-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,99-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,107-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,115-5,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,123-5,[CurCStringPos]
    add dword[CurCStringPos],32
    ret

%macro NetChatWindowHelp 3
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[%3],1
    jne %%nocheck
    mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheck
    cmp byte[%3],2
    jne %%nocheckb
    mov dword[GUITemp],GUIIconDataCheckBoxX
%%nocheckb
    GUIDisplayIconWin 8,%1,%2,[GUITemp]
%endmacro

DisplayNetOptnsChatWindow:
    mov byte[GUINetText8],'0'
    mov al,[Latency]
    cmp byte[Latency],10
    jb .no10b
    mov byte[GUINetText8],'1'
    sub al,10
.no10b
    add al,'0'
    mov [GUINetText8+1],al

    DrawGUIWinBox 8,55,157,74,165,167

    GUIDisplayButtonHole 8,111,137,byte[UseRemoteSRAMData],0
    GUIDisplayButtonHole 8,111,147,byte[UseRemoteSRAMData],1
    GUIDisplayButtonHole 8,111,157,byte[UseRemoteSRAMData],2
    NetChatWindowHelp 7,135,pl1neten
    NetChatWindowHelp 32,135,pl2neten
    NetChatWindowHelp 57,135,pl3neten
    NetChatWindowHelp 7,145,pl4neten
    NetChatWindowHelp 32,145,pl5neten
    NetChatWindowHelp 7,175,BackState

    sub byte[GUItextcolor],15
    GUIOuttextwin2 8,21,140,GUINetTextW
    GUIOuttextwin2 8,46,140,GUINetTextW+2
    GUIOuttextwin2 8,71,140,GUINetTextW+4
    GUIOuttextwin2 8,21,150,GUINetTextW+6
    GUIOuttextwin2 8,46,150,GUINetTextW+8
    GUIOuttextwin2 8,10,160,GUINetTextY
    GUIOuttextwin2 8,124,140,GUINetTextb
    GUIOuttextwin2 8,124,150,GUINetTextc
    GUIOuttextwin2 8,124,160,GUINetTextd
    GUIOuttextwin2 8,21,180,GUINetTexto2
    add byte[GUItextcolor],15
    GUIOuttextwin2 8,20,139,GUINetTextW
    GUIOuttextwin2 8,45,139,GUINetTextW+2
    GUIOuttextwin2 8,70,139,GUINetTextW+4
    GUIOuttextwin2 8,20,149,GUINetTextW+6
    GUIOuttextwin2 8,45,149,GUINetTextW+8
    GUIOuttextwin2 8,9,159,GUINetTextY
    GUIOuttextwin2 8,123,139,GUINetTextb
    GUIOuttextwin2 8,123,149,GUINetTextc
    GUIOuttextwin2 8,123,159,GUINetTextd
    GUIOuttextwin2 8,20,179,GUINetTexto2

    DrawGUIWinBox 8,6,15,181,112,167
    ; draw borders
    mov dl,[GUIWincol]
    DrawGUIWinBox 8,6,14,181,14,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox 8,5,15,4,112,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox 8,6,113,181,113,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox 8,183,15,182,112,dl
    DrawGUIWinBox 8,6,117,181,125,167
    ; disconnect button
    DrawGUIButton 8,111,169,181,180,GUINetTextU,27,0,0

    DrawGUIButton 8,79,156,87,166,GUINetText9,29,-2,0
    DrawGUIButton 8,90,156,98,166,GUINetTextA,30,-2,0


    mov byte[GUItextcolor],223
    mov dword[CurCStringPos],ViewBuffer+1
    mov ebx,[CViewLoc]
    add dword[CurCStringPos],ebx
    GUIOuttextwin2 8,9,36-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,44-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,52-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,60-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,68-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,76-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,84-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,92-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,100-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,108-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,116-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,9,124-18,[CurCStringPos]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3b
    mov byte[GUItextcolor],222
.zero3b
    mov dword[CurCStringPos],ViewBuffer+1
    mov ebx,[CViewLoc]
    add dword[CurCStringPos],ebx
    GUIOuttextwin2 8,8,35-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,43-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,51-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,59-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,67-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,75-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,83-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,91-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,99-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,107-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,115-18,[CurCStringPos]
    add dword[CurCStringPos],32
    GUIOuttextwin2 8,8,123-18,[CurCStringPos]

    ; Determine ChatString Position
    push ecx
    mov ebx,ChatString
    mov ecx,[GUIEditStringcLen]
    sub ecx,ebx
    cmp ecx,26
    jb .nostring
    cmp ecx,255
    ja .nostring
    sub ecx,26
    add ebx,ecx
.nostring
    mov [CurCStringPos],ebx
    pop ecx

    mov byte[GUItextcolor],223
    GUIOuttextwin2 8,10,120,[CurCStringPos]
    GUIOuttextwin2 8,60,160,GUINetText8
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],222
.zero3
    GUIOuttextwin2 8,9,119,[CurCStringPos]
    GUIOuttextwin2 8,59,159,GUINetText8
    mov dword[GUIEditString],ChatString
    mov dword[GUIEditStringmLen],200
;    mov dword[GUIEditStringmLenM],26
    mov dword[GUIEditStringcWin],1
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 8,7,131,GUINetTextV
    GUIOuttextwin2 8,111,131,GUINetTexta
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],164
.zero
    GUIOuttextwin2 8,6,130,GUINetTextV
    GUIOuttextwin2 8,110,130,GUINetTexta

    ; #############
;    mov eax,[CViewLoc]
;    shr eax,5
;    add eax,12
;    mov [TViewLoc],eax
;    181,112,167
;CViewLoc dd 0
;NViewLoc dd 0
;TViewLoc dd 0
;    15,181,112
;    DrawSlideBarWin 8,181,15,[NViewLoc],[TViewLoc],12,97,GUINCStA
    ret

SECTION .bss
NEWSYM GUINCStA, resd 3
NEWSYM CNetType, resb 1
SECTION .data
NEWSYM Latency, db 2
NEWSYM Latencytochange, db 2
NEWSYM LatencyLeft, db 2
SECTION .bss
NEWSYM NetSwap, resb 1
RemCNetType resb 1
UseRemoteSRAMData resb 1
ChatString resb 276
FileNameMod resb 512
CurCStringPos resd 1

SECTION .data
%ifdef __MSDOS__
GUINetText1 db 'IPX HAS NOT BEEN',0
GUINetText2 db 'IMPLEMENTED YET',0
%endif
GUINetText1b db 'PLEASE SET THE',0
GUINetText2b db 'PERCENTAGE OF',0
GUINetText3b db 'EXECUTION TO 100',0
%ifdef __MSDOS__
GUINetText1c db 'IPX IS IN BETA',0
GUINetText2c db 'STAGES.  IT MAY',0
GUINetText3c db 'NOT WORK RIGHT!',0
GUINetText3 db 'COM1 (3F8)',0
GUINetText4 db 'COM2 (2F8)',0
GUINetText5 db 'COM3 (3E8)',0
GUINetText6 db 'COM4 (2E8)',0
GUINetText7 db 'IRQ ',0
%endif
GUINetText8 db '00',0
GUINetText9 db '+',0
GUINetTextA db '-',0
%ifdef __MSDOS__
GUINetTextC db 'TONE (ATDT)',0
GUINetTextD db 'PULSE (ATDP)',0
GUINetTextE db 'INIT STRINGS :',0
GUINetTexte db '1.',0
GUINetTextf db '2.',0
GUINetTextF db 'ENTER NUMBER TO DIAL :',0
GUINetTextG db 'DIAL NUMBER',0
GUINetTextH db 'WAIT FOR RING',0
GUINetTexth db 'CONFIG MODEM',0
GUINetTextI db 'MANUAL',0
GUINetTextJ db '9600',0
GUINetTextK db '14400',0
GUINetTextL db '19200',0
GUINetTextM db '28800',0
GUINetTextN db '38400',0
GUINetTextO db '57600',0
GUINetTextP db '115200',0
GUINetTextQ db 'BAUDRATE (BPS) :',0
GUINetTextR db 'COMPORT :',0
%endif
GUINetTextS db 'CANCEL',0
%ifdef __MSDOS__
GUINetTextT db 'DIAL',0
%endif
GUINetTextU db 'DISCONNECT',0
GUINetTextV db 'PLAYER SELECT :',0
GUINetTextW db '1',0,'2',0,'3',0,'4',0,'5',0
%ifdef __MSDOS__
GUINetTextX db 'USE FOSSIL DRIVER',0
%endif
GUINetTextY db 'LATENCY',0
GUINetTextZ db 'OKAY',0
GUINetTexta db 'SAVE DATA :',0
GUINetTextb db 'NONE',0
GUINetTextc db 'LOCAL',0
GUINetTextd db 'REMOTE',0
GUINetTexte2 db 'YOUR IP:',0
GUINetTextf2 db 'PORT',0
GUINetTextf3 db 'NICKNAME',0
GUINetTextg2 db 'START AS SERVER',0
GUINetTexth2 db 'CLIENT OPTIONS:',0
GUINetTexti2 db 'IP:',0
GUINetTextj2 db 'CONNECT TO SERVER',0
GUINetTextl2 db '7845',0,0,0
GUINetTextm2 db 0,0,0
GUINetTextn2 db '_',0
GUINetTexto2 db 'BACK BUFFER',0
GUINetTextp2 db 'ALLOW UDP CONNECTION',0
GUINetTextr1 db 'PLEASE RESTART ZSNESW',0
GUINetTextr2 db 'BEFORE STARTING A NEW',0
GUINetTextr3 db 'NETPLAY SESSION',0

%ifdef __MSDOS__
BaudVal  dd 9600,14400,19200,28800,38400,57600,115200
%endif

SECTION .text

DisplayGameOptns:
    ; Key redefinition for :
    ; Sound Channels 0 .. 7, Save/Select/Load States, Fast Forward
    ; State Selection 0 .. 9
    ; Exit, Load, Reset, BG Disables, Reset, Windowing, New Gfx, OffsetMode
    GUIDrawWindowBox 9,GUIGameDisp

    ; Draw Shadows
    mov al,[GUIWincol]
    mov [GUIGameOptnsColA],al
    mov byte[GUIGameOptnsColB],202
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUIGameOptnsColB],196
.zero3
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,6,16,GUIGameOptnsText3
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText4+2],'1'
    GUIOuttextwin2 9,9,26,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'2'
    GUIOuttextwin2 9,9+45,26,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'3'
    GUIOuttextwin2 9,9+90,26,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'4'
    GUIOuttextwin2 9,9+135,26,GUIGameOptnsText4
    GUIOuttextwin2 9,9+180,26,GUIGameOptnsText5
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,6,36,GUIGameOptnsTextC
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,9,46,GUIGameOptnsText1A
    GUIOuttextwin2 9,9,54,GUIGameOptnsTextD
    GUIOuttextwin2 9,9,62,GUIGameOptnsTextL
    GUIOuttextwin2 9,9,70,GUIGameOptnsTextE
    GUIOuttextwin2 9,9,78,GUIGameOptnsTextF
    GUIOuttextwin2 9,9,86,GUIGameOptnsTextG
    GUIOuttextwin2 9,9,94,GUIGameOptnsTextT
;    cmp byte[OSPort],3
;    jne near .notwin32
%ifndef __MSDOS__
    GUIOuttextwin2 9,109,46,GUIGameOptnsTextS
%endif
;.notwin32
    GUIOuttextwin2 9,109,54,GUIGameOptnsTextH
    GUIOuttextwin2 9,109,62,GUIGameOptnsTextM
    GUIOuttextwin2 9,109,70,GUIGameOptnsTextN
    GUIOuttextwin2 9,109,78,GUIGameOptnsTextO
    GUIOuttextwin2 9,109,86,GUIGameOptnsTextP
    GUIOuttextwin2 9,109,94,GUIGameOptnsTextX
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,6,104,GUIGameOptnsText1
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText2+2],'1'
    GUIOuttextwin2 9,9,114,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'2'
    GUIOuttextwin2 9,9+45,114,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'3'
    GUIOuttextwin2 9,9+45*2,114,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'4'
    GUIOuttextwin2 9,9+45*3,114,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'5'
    GUIOuttextwin2 9,9,122,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'6'
    GUIOuttextwin2 9,9+45,122,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'7'
    GUIOuttextwin2 9,9+45*2,122,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'8'
    GUIOuttextwin2 9,9+45*3,122,GUIGameOptnsText2
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,6,132,GUIGameOptnsText6
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText7+2],'0'
    GUIOuttextwin2 9,9+45*2,134,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'1'
    GUIOuttextwin2 9,9+45*3,134,GUIGameOptnsText7
    GUIOuttextwin2 9,9+45*4,134,GUIGameOptnsTextY
    mov byte[GUIGameOptnsText7+2],'2'
    GUIOuttextwin2 9,9,141,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'3'
    GUIOuttextwin2 9,9+45,142,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'4'
    GUIOuttextwin2 9,9+45*2,142,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'5'
    GUIOuttextwin2 9,9+45*3,142,GUIGameOptnsText7
    GUIOuttextwin2 9,9+45*4,142,GUIGameOptnsTextZ
    mov byte[GUIGameOptnsText7+2],'6'
    GUIOuttextwin2 9,9,150,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'7'
    GUIOuttextwin2 9,9+45,150,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'8'
    GUIOuttextwin2 9,9+45*2,150,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'9'
    GUIOuttextwin2 9,9+45*3,150,GUIGameOptnsText7
    GUIOuttextwin2 9,9,160,GUIGameOptnsTextI
    GUIOuttextwin2 9,9+57,160,GUIGameOptnsTextJ
    GUIOuttextwin2 9,9+114,160,GUIGameOptnsTextK
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,6,170,GUIGameOptnsText8
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,9,180,GUIGameOptnsText9
    GUIOuttextwin2 9,9+52,180,GUIGameOptnsTextA
    GUIOuttextwin2 9,9+109,180,GUIGameOptnsTextB
    GUIOuttextwin2 9,9,190,GUIGameOptnsTextQ
    GUIOuttextwin2 9,9+52,190,GUIGameOptnsTextR
    GUIOuttextwin2 9,9+160,180,GUIGameOptnsTextU
    GUIOuttextwin2 9,137,190,GUIGameOptnsTextV

    mov byte[GUIGameOptnsColA],163
    mov byte[GUIGameOptnsColB],217
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUIGameOptnsColA],164
    mov byte[GUIGameOptnsColB],211
.zero
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,5,15,GUIGameOptnsText3
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText4+2],'1'
    GUIOuttextwin2 9,8,25,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'2'
    GUIOuttextwin2 9,8+45,25,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'3'
    GUIOuttextwin2 9,8+90,25,GUIGameOptnsText4
    mov byte[GUIGameOptnsText4+2],'4'
    GUIOuttextwin2 9,8+135,25,GUIGameOptnsText4
    GUIOuttextwin2 9,8+180,25,GUIGameOptnsText5
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,5,35,GUIGameOptnsTextC
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,8,45,GUIGameOptnsText1A
    GUIOuttextwin2 9,8,53,GUIGameOptnsTextD
    GUIOuttextwin2 9,8,61,GUIGameOptnsTextL
    GUIOuttextwin2 9,8,69,GUIGameOptnsTextE
    GUIOuttextwin2 9,8,77,GUIGameOptnsTextF
    GUIOuttextwin2 9,8,85,GUIGameOptnsTextG
    GUIOuttextwin2 9,8,93,GUIGameOptnsTextT
;    cmp byte[OSPort],3
;    jne near .notwin32b
%ifndef __MSDOS__
    GUIOuttextwin2 9,108,45,GUIGameOptnsTextS
%endif
;.notwin32b
    GUIOuttextwin2 9,108,53,GUIGameOptnsTextH
    GUIOuttextwin2 9,108,61,GUIGameOptnsTextM
    GUIOuttextwin2 9,108,69,GUIGameOptnsTextN
    GUIOuttextwin2 9,108,77,GUIGameOptnsTextO
    GUIOuttextwin2 9,108,85,GUIGameOptnsTextP
    GUIOuttextwin2 9,108,93,GUIGameOptnsTextX
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,5,103,GUIGameOptnsText1
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText2+2],'1'
    GUIOuttextwin2 9,8,113,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'2'
    GUIOuttextwin2 9,8+45,113,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'3'
    GUIOuttextwin2 9,8+45*2,113,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'4'
    GUIOuttextwin2 9,8+45*3,113,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'5'
    GUIOuttextwin2 9,8,121,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'6'
    GUIOuttextwin2 9,8+45,121,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'7'
    GUIOuttextwin2 9,8+45*2,121,GUIGameOptnsText2
    mov byte[GUIGameOptnsText2+2],'8'
    GUIOuttextwin2 9,8+45*3,121,GUIGameOptnsText2
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,5,131,GUIGameOptnsText6
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    mov byte[GUIGameOptnsText7+2],'0'
    GUIOuttextwin2 9,8+45*2,133,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'1'
    GUIOuttextwin2 9,8+45*3,133,GUIGameOptnsText7
    GUIOuttextwin2 9,8+45*4,133,GUIGameOptnsTextY
    mov byte[GUIGameOptnsText7+2],'2'
    GUIOuttextwin2 9,8,141,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'3'
    GUIOuttextwin2 9,8+45,141,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'4'
    GUIOuttextwin2 9,8+45*2,141,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'5'
    GUIOuttextwin2 9,8+45*3,141,GUIGameOptnsText7
    GUIOuttextwin2 9,8+45*4,141,GUIGameOptnsTextZ
    mov byte[GUIGameOptnsText7+2],'6'
    GUIOuttextwin2 9,8,149,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'7'
    GUIOuttextwin2 9,8+45,149,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'8'
    GUIOuttextwin2 9,8+45*2,149,GUIGameOptnsText7
    mov byte[GUIGameOptnsText7+2],'9'
    GUIOuttextwin2 9,8+45*3,149,GUIGameOptnsText7
    GUIOuttextwin2 9,8,159,GUIGameOptnsTextI
    GUIOuttextwin2 9,8+57,159,GUIGameOptnsTextJ
    GUIOuttextwin2 9,8+114,159,GUIGameOptnsTextK
    mov al,byte[GUIGameOptnsColA]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,5,169,GUIGameOptnsText8
    mov al,byte[GUIGameOptnsColB]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 9,8,179,GUIGameOptnsText9
    GUIOuttextwin2 9,8+52,179,GUIGameOptnsTextA
    GUIOuttextwin2 9,8+109,179,GUIGameOptnsTextB
    GUIOuttextwin2 9,8,189,GUIGameOptnsTextQ
    GUIOuttextwin2 9,8+52,189,GUIGameOptnsTextR
    GUIOuttextwin2 9,8+160,179,GUIGameOptnsTextU
    GUIOuttextwin2 9,136,189,GUIGameOptnsTextV

    ; Draw black boxes
    DGOptnsDrawBox 27,24,[KeyBGDisble0]
    DGOptnsDrawBox 27+45,24,[KeyBGDisble1]
    DGOptnsDrawBox 27+45*2,24,[KeyBGDisble2]
    DGOptnsDrawBox 27+45*3,24,[KeyBGDisble3]
    DGOptnsDrawBox 27+45*4,24,[KeySprDisble]
    DGOptnsDrawBox 78,44,[KeyUsePlayer1234]
    DGOptnsDrawBox 78,52,[KeyResetAll]
    DGOptnsDrawBox 78,60,[KeyExtraEnab]
    DGOptnsDrawBox 78,68,[KeyNewGfxSwt]
    DGOptnsDrawBox 78,76,[KeyWinDisble]
    DGOptnsDrawBox 78,84,[KeyOffsetMSw]
    DGOptnsDrawBox 78,92,[KeySlowDown]
;    cmp byte[OSPort],3
;    jne near .notwin32c
%ifndef __MSDOS__
    DGOptnsDrawBox 178,44,[KeyRewind]
%endif
;.notwin32c
    DGOptnsDrawBox 178,52,[KeyFastFrwrd]
    DGOptnsDrawBox 178,60,[KeyVolUp]
    DGOptnsDrawBox 178,68,[KeyVolDown]
    DGOptnsDrawBox 178,76,[KeyFRateUp]
    DGOptnsDrawBox 178,84,[KeyFRateDown]
    DGOptnsDrawBox 178,92,[KeyDisplayFPS]
    DGOptnsDrawBox 27,112,[KeyDisableSC0]
    DGOptnsDrawBox 27+45,112,[KeyDisableSC1]
    DGOptnsDrawBox 27+45*2,112,[KeyDisableSC2]
    DGOptnsDrawBox 27+45*3,112,[KeyDisableSC3]
    DGOptnsDrawBox 27,120,[KeyDisableSC4]
    DGOptnsDrawBox 27+45,120,[KeyDisableSC5]
    DGOptnsDrawBox 27+45*2,120,[KeyDisableSC6]
    DGOptnsDrawBox 27+45*3,120,[KeyDisableSC7]
    DGOptnsDrawBox 27+45*2,132,[KeyStateSlc0]
    DGOptnsDrawBox 27+45*3,132,[KeyStateSlc1]
    DGOptnsDrawBox 27+45*4,132,[KeyIncStateSlot]
    DGOptnsDrawBox 27,140,[KeyStateSlc2]
    DGOptnsDrawBox 27+45,140,[KeyStateSlc3]
    DGOptnsDrawBox 27+45*2,140,[KeyStateSlc4]
    DGOptnsDrawBox 27+45*3,140,[KeyStateSlc5]
    DGOptnsDrawBox 27+45*4,140,[KeyDecStateSlot]
    DGOptnsDrawBox 27,148,[KeyStateSlc6]
    DGOptnsDrawBox 27+45,148,[KeyStateSlc7]
    DGOptnsDrawBox 27+45*2,148,[KeyStateSlc8]
    DGOptnsDrawBox 27+45*3,148,[KeyStateSlc9]
    DGOptnsDrawBox 8+25,158,[KeySaveState]
    DGOptnsDrawBox 8+57+25,158,[KeyLoadState]
    DGOptnsDrawBox 8+114+25,158,[KeyStateSelct]
    DGOptnsDrawBox 8+25,178,[KeyQuickLoad]
    DGOptnsDrawBox 8+57+26,178,[KeyQuickRst]
    DGOptnsDrawBox 8+114+20,178,[KeyQuickExit]
    DGOptnsDrawBox 8+114+78,178,[KeyQuickClock]
    DGOptnsDrawBox 8+25,188,[KeyQuickChat]
    DGOptnsDrawBox 8+57+45,188,[KeyQuickSnapShot]
    DGOptnsDrawBox 8+108+70,188,[KeyQuickSaveSPC]
    ret

SECTION .bss
GUIGameOptnsColA resb 1
GUIGameOptnsColB resb 1
GUIGameDisplayKy resd 1

SECTION .data
GUIGameOptnsText1 db 'SOUND DISABLES :',0
GUIGameOptnsText2 db 'CH#',0
GUIGameOptnsText3 db 'BG DISABLES :',0
GUIGameOptnsText4 db 'BG#',0
GUIGameOptnsText5 db 'OBJ ',0
GUIGameOptnsText6 db 'STATE SELECTS :',0
GUIGameOptnsText7 db 'ST#',0
GUIGameOptnsText8 db 'QUICK KEYS :',0
GUIGameOptnsText9 db 'LOAD',0
GUIGameOptnsTextA db 'RESET',0
GUIGameOptnsTextB db 'EXIT',0
GUIGameOptnsTextC db 'MISC TOGGLES :',0
GUIGameOptnsTextD db 'CLRSWITCHES',0
GUIGameOptnsTextE db 'NEW GFX ENG',0
GUIGameOptnsTextF db 'BG WINDOW',0
GUIGameOptnsTextG db 'OFFSET MODE',0
GUIGameOptnsTextH db 'FASTFWD KEY',0
GUIGameOptnsTextI db 'SAVE',0
GUIGameOptnsTextJ db 'LOAD',0
GUIGameOptnsTextK db 'PICK',0
GUIGameOptnsTextL db 'EXTRADEVICE',0
GUIGameOptnsTextM db '+VOL',0
GUIGameOptnsTextN db '-VOL',0
GUIGameOptnsTextO db '+FRAME RATE',0
GUIGameOptnsTextP db '-FRAME RATE',0
GUIGameOptnsTextQ db 'CHAT',0
GUIGameOptnsTextR db 'SNAPSHOT',0
GUIGameOptnsTextS db 'REWIND KEY',0
GUIGameOptnsTextT db 'SLOWDWN KEY',0
GUIGameOptnsTextU db 'CLOCK',0
GUIGameOptnsTextV db 'SAVE SPC',0
GUIGameOptnsTextX db 'DISPLAY FPS',0
GUIGameOptnsTextY db 'ST+',0
GUIGameOptnsTextZ db 'ST-',0
GUIGameOptnsText1A db 'USE PL 12/34',0
SECTION .text

DisplayGUIOptns:
%ifdef __WIN32__
    mov dword[GUIwinsizey+10*4],192
%endif
    GUIDrawWindowBox 10,GUIGUIDisp

    push eax
    cmp byte[CurPalSelect],1
    je .yespal2b
    cmp byte[CurPalSelect],2
    je .yespal3b
    mov al,[GUIRAdd]
    mov [TRVal2],al
    mov al,[GUIGAdd]
    mov [TGVal2],al
    mov al,[GUIBAdd]
    mov [TBVal2],al
    jmp .nopal1b
.yespal2b
    mov al,[GUITRAdd]
    mov [TRVal2],al
    mov al,[GUITGAdd]
    mov [TGVal2],al
    mov al,[GUITBAdd]
    mov [TBVal2],al
    jmp .nopal1b
.yespal3b
    mov al,[GUIWRAdd]
    mov [TRVal2],al
    mov al,[GUIWGAdd]
    mov [TGVal2],al
    mov al,[GUIWBAdd]
    mov [TBVal2],al
.nopal1b
    pop eax

    mov eax,dword[GUIwinposx+10*4]
    mov ebx,dword[GUIwinposy+10*4]
    mov ecx,eax
    add eax,25
    add ecx,25+32*4
    add ebx,124
    xor edx,edx
    mov dl,215
    sub dl,[GUIWincoladd]
    DrawGUILine
    add ebx,11
    DrawGUILine
    add ebx,11
    DrawGUILine
    inc ecx
    mov ebx,dword[GUIwinposy+10*4]
    inc eax
    add ebx,125
    sub dl,13
    DrawGUILine
    add ebx,11
    DrawGUILine
    add ebx,11
    DrawGUILine
    xor eax,eax
    mov al,[TRVal2]
    shl eax,2
    mov dword[GUITemp],22
    add [GUITemp],eax
    GUIDisplayIconWin 10,[GUITemp],120,GUIIconDataSlideBar
    xor eax,eax
    mov al,[TGVal2]
    shl eax,2
    mov dword[GUITemp],22
    add [GUITemp],eax
    GUIDisplayIconWin 10,[GUITemp],132,GUIIconDataSlideBar
    xor eax,eax
    mov al,[TBVal2]
    shl eax,2
    mov dword[GUITemp],22
    add [GUITemp],eax
    GUIDisplayIconWin 10,[GUITemp],144,GUIIconDataSlideBar
    mov dword[GUITemp],GUIIconDataCheckBoxUC

    cmp byte[GUIEnableTransp],1
    jne .nocheckt
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckt
    GUIDisplayIconWin 10,124,23,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[FilteredGUI],1
    jne .nocheckfg
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckfg
    GUIDisplayIconWin 10,124,43,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[mousewrap],1
    jne .nocheck
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck
    GUIDisplayIconWin 10,12,23,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[mouseshad],1
    jne .nocheck2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck2
    GUIDisplayIconWin 10,12,33,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[lastcursres],1
    jne .nocheck3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck3
    GUIDisplayIconWin 10,12,43,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[resetposn],1
    je .nocheck4
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck4
    GUIDisplayIconWin 10,12,53,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GUIClick],1
    jne .nocheck5
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck5
    GUIDisplayIconWin 10,12,63,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[JoyPad1Move],1
    jne .nocheck6
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck6
    GUIDisplayIconWin 10,12,73,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GUIEffect],1
    jne .nocheck6b
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck6b
    GUIDisplayIconWin 10,12,83,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GUIEffect],4
    jne .nocheck7
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck7
    GUIDisplayIconWin 10,124,83,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GUIEffect],2
    jne .nocheck7b
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck7b
    GUIDisplayIconWin 10,12,93,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[GUIEffect],3
    jne .nocheck8
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck8
    GUIDisplayIconWin 10,124,93,[GUITemp]
%ifdef __WIN32__
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[MouseWheel],1
    jne .nocheck9
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck9
    GUIDisplayIconWin 10,124,33,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[AlwaysOnTop],1
    jne .nocheck10
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck10
    GUIDisplayIconWin 10,12,168,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[SaveMainWindowPos],1
    jne .nocheck11
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck11
    GUIDisplayIconWin 10,12,178,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[AllowMultipleInst],1
    jne .nocheck12
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck12
    GUIDisplayIconWin 10,12,188,[GUITemp]
%endif
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 10,6,111,GUIGUIOptnsText1
    GUIOuttextwin2 10,6,16,GUIGUIOptnsText5
%ifdef __WIN32__
    GUIOuttextwin2 10,6,160,GUIGUIOptnsTextD
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[TrapMouseCursor],1
    jne .nocheck13
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheck13
    GUIDisplayIconWin 10,124,53,[GUITemp]
%endif
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],164
.zero
    GUIOuttextwin2 10,5,110,GUIGUIOptnsText1
    GUIOuttextwin2 10,5,15,GUIGUIOptnsText5
%ifdef __WIN32__
    GUIOuttextwin2 10,5,160,GUIGUIOptnsTextD
%endif
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],211
.zero3
    sub byte[GUItextcolor],15
    GUIOuttextwin2u 10,26,28,GUIGUIOptnsText2,0
    GUIOuttextwin2u 10,26,38,GUIGUIOptnsText3,6
    GUIOuttextwin2u 10,26,48,GUIGUIOptnsText4,0
    GUIOuttextwin2u 10,26,58,GUIGUIOptnsText7,5
    GUIOuttextwin2u 10,26,68,GUIGUIOptnsText8,0
    GUIOuttextwin2u 10,26,78,GUIGUIOptnsTextA,0
    GUIOuttextwin2 10,60,111,GUIGUIOptnsTextG
    GUIOuttextwin2 10,100,111,GUIGUIOptnsTextH
    GUIOuttextwin2 10,147,111,GUIGUIOptnsTextI
    GUIOuttextwin2u 10,26,88,GUIGUIOptnsTextB,1
    GUIOuttextwin2u 10,138,88,GUIGUIOptnsTextBb,2
    GUIOuttextwin2u 10,26,98,GUIGUIOptnsTextC,3
    GUIOuttextwin2u 10,138,98,GUIGUIOptnsTextCb,13
    GUIOuttextwin2u 10,138,28,GUIGUIOptnsTextJ,0
    GUIOuttextwin2u 10,138,48,GUIGUIOptnsTextM,0
%ifdef __WIN32__
    GUIOuttextwin2u 10,26,173,GUIGUIOptnsTextE,0
    GUIOuttextwin2u 10,26,183,GUIGUIOptnsTextF,2
    GUIOuttextwin2u 10,138,38,GUIGUIOptnsTextK,1
    GUIOuttextwin2u 10,26,193,GUIGUIOptnsTextL,1
    GUIOuttextwin2u 10,138,58,GUIGUIOptnsTextN,1
%endif
    mov byte[GUIGUIOptnsText6],'R'
    GUIOuttextwin2 10,16,123,GUIGUIOptnsText6
    mov byte[GUIGUIOptnsText6],'G'
    GUIOuttextwin2 10,16,135,GUIGUIOptnsText6
    mov byte[GUIGUIOptnsText6],'B'
    GUIOuttextwin2 10,16,147,GUIGUIOptnsText6
    add byte[GUItextcolor],15
    GUIOuttextwin2 10,25,27,GUIGUIOptnsText2
    GUIOuttextwin2 10,25,37,GUIGUIOptnsText3
    GUIOuttextwin2 10,25,47,GUIGUIOptnsText4
    GUIOuttextwin2 10,25,57,GUIGUIOptnsText7
    GUIOuttextwin2 10,25,67,GUIGUIOptnsText8
    GUIOuttextwin2 10,25,77,GUIGUIOptnsTextA
    GUIOuttextwin2 10,59,110,GUIGUIOptnsTextG
    GUIOuttextwin2 10,99,110,GUIGUIOptnsTextH
    GUIOuttextwin2 10,146,110,GUIGUIOptnsTextI
    GUIOuttextwin2 10,25,87,GUIGUIOptnsTextB
    GUIOuttextwin2 10,137,87,GUIGUIOptnsTextBb
    GUIOuttextwin2 10,25,97,GUIGUIOptnsTextC
    GUIOuttextwin2 10,137,97,GUIGUIOptnsTextCb
    GUIOuttextwin2 10,137,27,GUIGUIOptnsTextJ
    GUIOuttextwin2 10,137,47,GUIGUIOptnsTextM
%ifdef __WIN32__
    GUIOuttextwin2 10,25,172,GUIGUIOptnsTextE
    GUIOuttextwin2 10,25,182,GUIGUIOptnsTextF
    GUIOuttextwin2 10,137,37,GUIGUIOptnsTextK
    GUIOuttextwin2 10,25,192,GUIGUIOptnsTextL
    GUIOuttextwin2 10,137,57,GUIGUIOptnsTextN
%endif
    mov byte[GUIGUIOptnsText6],'R'
    GUIOuttextwin2 10,15,122,GUIGUIOptnsText6
    mov byte[GUIGUIOptnsText6],'G'
    GUIOuttextwin2 10,15,134,GUIGUIOptnsText6
    mov byte[GUIGUIOptnsText6],'B'
    GUIOuttextwin2 10,15,146,GUIGUIOptnsText6

    GUIDisplayButtonHole 10,48,108,byte[CurPalSelect],0
    GUIDisplayButtonHole 10,88,108,byte[CurPalSelect],1
    GUIDisplayButtonHole 10,135,108,byte[CurPalSelect],2

    mov dword[GUIGUIOptnsText9],0
    mov esi,GUIGUIOptnsText9+2
    xor eax,eax
    mov al,[TRVal2]
    call .processnum
    GUIOuttextwin2 10,160,123,[GUITemp]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],222
.zero5
    GUIOuttextwin2 10,159,122,[GUITemp]
    mov dword[GUIGUIOptnsText9],0
    mov esi,GUIGUIOptnsText9+2
    xor eax,eax
    mov al,[TGVal2]
    call .processnum
    GUIOuttextwin2 10,160,135,[GUITemp]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero6
    mov byte[GUItextcolor],222
.zero6
    GUIOuttextwin2 10,159,134,[GUITemp]
    mov dword[GUIGUIOptnsText9],0
    mov esi,GUIGUIOptnsText9+2
    xor eax,eax
    mov al,[TBVal2]
    call .processnum
    GUIOuttextwin2 10,160,147,[GUITemp]
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zero7
    mov byte[GUItextcolor],222
.zero7
    GUIOuttextwin2 10,159,146,[GUITemp]
    ret

.processnum
    mov ebx,10
    div bl
    add ah,48
    mov [esi],ah
    cmp al,0
    je .nomore
    dec esi
    xor ah,ah
    div bl
    add ah,48
    mov [esi],ah
.nomore
    mov [GUITemp],esi
    mov byte[GUItextcolor],223
    ret

;mousewrap db 0          ; 0 = mouse boundries, 1 = mouse wrap
;mouseshad db 1          ; 0 = no mouse shadow, 1 = mouse shadow
;lastcursres db 0        ; 0 = go to load, 1 = go to previous menu, 2 = no menu
;resetposn   db 1        ; 0 = no window reset, 1 = window reset

SECTION .data
GUIGUIOptnsText1 db 'COLOR:',0
GUIGUIOptnsText2 db 'MOUSE WRAP',0
GUIGUIOptnsText3 db 'MOUSE SHADOW',0
GUIGUIOptnsText4 db 'KEEP MENU POSITN',0
GUIGUIOptnsText5 db 'GUI SWITCHES :',0
GUIGUIOptnsText6 db '@',0
GUIGUIOptnsText7 db 'SAVE WIN POSITNS',0
GUIGUIOptnsText8 db 'CLICK ENTERS GUI',0
GUIGUIOptnsText9 dd 0
GUIGUIOptnsTextA db 'USE JOYPAD1',0
GUIGUIOptnsTextB dd 'SNOW EFFECT',0
GUIGUIOptnsTextBb dd 'BURNING EFFECT',0
GUIGUIOptnsTextC dd 'WATER EFFECT A',0
GUIGUIOptnsTextCb dd 'WATER EFFECT B',0
GUIGUIOptnsTextD dd 'MAIN WINDOW OPTIONS :',0
GUIGUIOptnsTextE dd 'ALWAYS ON TOP',0
GUIGUIOptnsTextF db 'SAVE MAIN WINDOW POSITION',0
GUIGUIOptnsTextG db 'BACK',0
GUIGUIOptnsTextH db 'TITLE',0
GUIGUIOptnsTextI db 'WIN',0
GUIGUIOptnsTextJ db 'TRANSPARENT MSGS',0
GUIGUIOptnsTextK db 'WHEEL MOUSE',0
GUIGUIOptnsTextL db 'ALLOW MULTIPLE INSTANCES',0
GUIGUIOptnsTextM db 'FILTERED GUI',0
GUIGUIOptnsTextN db 'TRAP MOUSE CURSOR',0
SECTION .text

DisplayGUIAbout:
    GUIDrawWindowBox 11,GUIAboutDisp
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero3
    mov byte[GUItextcolor],211
.zero3
%ifndef __LINUX__
    DrawGUIButton 11,90,50,175,60,GUIGUIAboutText8,65,0,0
%endif
    sub byte[GUItextcolor],15
    GUIOuttextwin2 11,6,16,GUIGUIAboutText1
    GUIOuttextwin2 11,6,26,GUIGUIAboutText7
    GUIOuttextwin2 11,6,46,GUIGUIAboutText3
    GUIOuttextwin2 11,6,56,GUIGUIAboutText4
    GUIOuttextwin2 11,6,66,GUIGUIAboutText6c
    GUIOuttextwin2 11,6,86,GUIGUIAboutText6
    GUIOuttextwin2 11,6,96,GUIGUIAboutText6b
    GUIOuttextwin2 11,6,106,GUIGUIAboutText6d
    GUIOuttextwin2 11,6,116,GUIGUIAboutText6e
    GUIOuttextwin2 11,6,131,GUIGUIAboutTextA
    GUIOuttextwin2 11,6,141,GUIGUIAboutTextB
    GUIOuttextwin2 11,6,151,GUIGUIAboutTextC
    GUIOuttextwin2 11,6,161,GUIGUIAboutTextD
    GUIOuttextwin2 11,6,171,GUIGUIAboutTextE
    GUIOuttextwin2 11,6,181,GUIGUIAboutTextF
    GUIOuttextwin2 11,6,191,GUIGUIAboutTextG
    add byte[GUItextcolor],15
    GUIOuttextwin2 11,5,15,GUIGUIAboutText1
    GUIOuttextwin2 11,5,25,GUIGUIAboutText7
    GUIOuttextwin2 11,5,45,GUIGUIAboutText3
    GUIOuttextwin2 11,5,55,GUIGUIAboutText4
    GUIOuttextwin2 11,5,65,GUIGUIAboutText6c
    GUIOuttextwin2 11,5,85,GUIGUIAboutText6
    GUIOuttextwin2 11,5,95,GUIGUIAboutText6b
    GUIOuttextwin2 11,5,105,GUIGUIAboutText6d
    GUIOuttextwin2 11,5,115,GUIGUIAboutText6e
    GUIOuttextwin2 11,5,130,GUIGUIAboutTextA
    GUIOuttextwin2 11,5,140,GUIGUIAboutTextB
    GUIOuttextwin2 11,5,150,GUIGUIAboutTextC
    GUIOuttextwin2 11,5,160,GUIGUIAboutTextD
    GUIOuttextwin2 11,5,170,GUIGUIAboutTextE
    GUIOuttextwin2 11,5,180,GUIGUIAboutTextF
    GUIOuttextwin2 11,5,190,GUIGUIAboutTextG
    mov al,[GUIWincol]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 11,6,36,GUIGUIAboutText2
    GUIOuttextwin2 11,6,76,GUIGUIAboutText5
    mov byte[GUItextcolor],163
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],164
.zero
    GUIOuttextwin2 11,5,35,GUIGUIAboutText2
    GUIOuttextwin2 11,5,75,GUIGUIAboutText5
    ret

SECTION .data
GUIGUIAboutText1 db 'ZSNES V',ZVERSION,0
GUIGUIAboutText2 db 'CODED BY :  ',0
GUIGUIAboutText3 db '    ZSKNIGHT',0
GUIGUIAboutText4 db '    _DEMO_',0
GUIGUIAboutText6c db '    PAGEFAULT',0
GUIGUIAboutText5 db 'ASSISTANT CODERS :  ',0
GUIGUIAboutText6 db '    PHAROS',0
GUIGUIAboutText6b db '    TEUF',0
GUIGUIAboutText6d db '    THEODDONE33',0
GUIGUIAboutText6e db '    STAINLESS',0

%ifdef __MSDOS__
GUIGUIAboutText7 db 'DOS VERSION',0
%elifdef __WIN32__
GUIGUIAboutText7 db 'WIN VERSION',0
%elifdef __LINUX__
GUIGUIAboutText7 db 'LINUX VERSION',0
%endif

GUIGUIAboutText8 db 'WWW.ZSNES.COM',0
GUIGUIAboutTextA db 'ZSNES comes with ABSOLUTELY',0
GUIGUIAboutTextB db 'NO WARRANTY.  This is free',0
GUIGUIAboutTextC db 'software, and you are welcome',0
GUIGUIAboutTextD db 'to redistribute it under',0
GUIGUIAboutTextE db 'certain conditions; please',0
GUIGUIAboutTextF db 'please read ',39,'LICENSE',39,0
GUIGUIAboutTextG db 'thoroughly before using it.',0

SECTION .text

DisplayGUIMovies:
    GUIDrawWindowBox 15,GUIMovieDisp
    cmp byte[MovieRecordWinVal],0
    je near .displayboxa
    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],211
.zero4
    sub byte[GUItextcolor],15
    GUIOuttextwin2 15,9,16,GUIMovieText5
    GUIOuttextwin2 15,9,26,GUIMovieText6
    GUIOuttextwin2 15,9,41,GUIMovieText7
    add byte[GUItextcolor],15
    GUIOuttextwin2 15,8,15,GUIMovieText5
    GUIOuttextwin2 15,8,25,GUIMovieText6
    GUIOuttextwin2 15,8,40,GUIMovieText7
    DrawGUIButton 15,17,55,59,66,GUIMovieText8,19,0,0
    DrawGUIButton 15,70,55,112,66,GUIMovieText9,20,0,0
    ret
.displayboxa
    mov dword[GUIMovieTextA+9],'INAC'
    mov dword[GUIMovieTextA+13],'TIVE'
    mov byte[GUIMovieTextA+17],' '
    cmp byte[MovieProcessing],1
    jne .noplay
    mov dword[GUIMovieTextA+9],'PLAY'
    mov dword[GUIMovieTextA+13],'ING '
    mov byte[GUIMovieTextA+17],' '
.noplay
    cmp byte[MovieProcessing],2
    jne .norec
    mov dword[GUIMovieTextA+9],'RECO'
    mov dword[GUIMovieTextA+13],'RDIN'
    mov byte[GUIMovieTextA+17],'G'
.norec
    DrawGUIButton 15,7,17,49,28,GUIMovieText1,16,0,0
    DrawGUIButton 15,7,33,49,44,GUIMovieText2,17,0,0
    DrawGUIButton 15,7,49,49,60,GUIMovieText3,18,0,0
    cmp byte[GUIWincoladd],0
    je .zero
    mov byte[GUItextcolor],211
.zero
    sub byte[GUItextcolor],15
    GUIOuttextwin2 15,61,16,GUIMovieText4
    GUIOuttextwin2 15,7,70,GUIMovieTextA
    mov byte[GUIChoseSaveText2],'0'
    GUIOuttextwin2 15,71,28,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'1'
    GUIOuttextwin2 15,91,28,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'2'
    GUIOuttextwin2 15,111,28,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'3'
    GUIOuttextwin2 15,131,28,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'4'
    GUIOuttextwin2 15,71,43,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'5'
    GUIOuttextwin2 15,91,43,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'6'
    GUIOuttextwin2 15,111,43,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'7'
    GUIOuttextwin2 15,131,43,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'8'
    GUIOuttextwin2 15,71,58,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'9'
    GUIOuttextwin2 15,91,58,GUIChoseSaveText2
    add byte[GUItextcolor],15
    GUIOuttextwin2 15,60,15,GUIMovieText4
    GUIOuttextwin2 15,6,69,GUIMovieTextA
    mov byte[GUIChoseSaveText2],'0'
    GUIOuttextwin2 15,70,27,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'1'
    GUIOuttextwin2 15,90,27,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'2'
    GUIOuttextwin2 15,110,27,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'3'
    GUIOuttextwin2 15,130,27,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'4'
    GUIOuttextwin2 15,70,42,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'5'
    GUIOuttextwin2 15,90,42,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'6'
    GUIOuttextwin2 15,110,42,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'7'
    GUIOuttextwin2 15,130,42,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'8'
    GUIOuttextwin2 15,70,57,GUIChoseSaveText2
    mov byte[GUIChoseSaveText2],'9'
    GUIOuttextwin2 15,90,57,GUIChoseSaveText2
    GUIDisplayButtonHole 15,60,25,byte[CMovieExt],'v'
    GUIDisplayButtonHole 15,80,25,byte[CMovieExt],'1'
    GUIDisplayButtonHole 15,100,25,byte[CMovieExt],'2'
    GUIDisplayButtonHole 15,120,25,byte[CMovieExt],'3'
    GUIDisplayButtonHole 15,60,40,byte[CMovieExt],'4'
    GUIDisplayButtonHole 15,80,40,byte[CMovieExt],'5'
    GUIDisplayButtonHole 15,100,40,byte[CMovieExt],'6'
    GUIDisplayButtonHole 15,120,40,byte[CMovieExt],'7'
    GUIDisplayButtonHole 15,60,55,byte[CMovieExt],'8'
    GUIDisplayButtonHole 15,80,55,byte[CMovieExt],'9'
    ret

SECTION .data
GUIMovieText1 db 'PLAY',0
GUIMovieText2 db 'RECORD',0
GUIMovieText3 db 'STOP',0
GUIMovieText4 db 'SELECT :',0
GUIMovieText5 db 'WARNING :  THIS MOVIE',0
GUIMovieText6 db '  FILE ALREADY EXISTS',0
GUIMovieText7 db 'OKAY TO OVERWRITE?',0
GUIMovieText8 db 'YES',0
GUIMovieText9 db 'NO',0
GUIMovieTextA db 'STATUS :           ',0
CMovieExt db 'v'

SECTION .text

; Key types: Up, Down, Left, Right, A, B, X, Y, L, R (Press/Relase/P+R)
; Frame delays: 1 frame, 2, 3, 4, 5, 1 sec., 2, 3 ,4 ,5

%macro DrawBorderedBox 6
    ; draw borders
    mov dl,[GUIWincol]
    DrawGUIWinBox %1,%2,%6,%4,%3,dl   ; For some reason, this freezes if I
                                      ; replace %6 with %3-1
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
    DrawGUIWinBox %1,%2,%3,%4,%5,167
%endmacro

%macro DrawBorderedBoxB 7       ; Special function for combo displays
    ; draw borders
    mov dl,[GUIWincol]
    DrawGUIWinBox %1,%2,%6,%4,%3,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
    DrawGUIWinBox %1,%2,%3,%4,%5,167
    mov byte[GUIComboText3],%7
    mov byte[GUItextcolor],223
    GUIOuttextwin2 %1,%2+5,%3+2,GUIComboText3
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je %%zero
    mov byte[GUItextcolor],222
%%zero
    GUIOuttextwin2 %1,%2+4,%3+1,GUIComboText3
%endmacro

%macro DrawBorderedBoxB2 7       ; Special function for combo displays
    ; draw borders
    mov dl,[GUIWincol]
    DrawGUIWinBox %1,%2,%6,%4,%3,dl
    mov dl,[GUIWincol]
    add dl,1
    DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
    mov dl,[GUIWincol]
    add dl,4
    DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
    mov dl,[GUIWincol]
    add dl,3
    DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
    DrawGUIWinBox %1,%2,%3,%4,%5,167
    mov byte[GUIComboText3b+1],%7
    mov byte[GUItextcolor],223
    GUIOuttextwin2 %1,%2+2,%3+2,GUIComboText3b
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je %%zero
    mov byte[GUItextcolor],222
%%zero
    GUIOuttextwin2 %1,%2+1,%3+1,GUIComboText3b
%endmacro

%macro InitTextColor 0
    mov al,217
    cmp byte[GUIWincoladd],0
    je .zero3
    mov al,211
.zero3
    sub al,15
    mov byte[TextColorSp+2],al
    mov al,[GUIWincol]
    mov byte[TextColorSp],al
    mov al,163
    cmp byte[GUIWincoladd],0
    je .zero
    mov al,164
    mov byte[GUItextcolor],164
.zero
    mov byte[TextColorSp+1],al
%endmacro

%macro GUIOuttextwin2b 4        ; Special, yellow text
    mov al,[TextColorSp]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 %1,%2,%3,%4
    mov al,[TextColorSp+1]
    mov byte[GUItextcolor],al
    GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIOuttextwin2c 4        ; Boxed, green text
    mov byte[GUItextcolor],223
    GUIOuttextwin2 %1,%2,%3,%4
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je %%zero
    mov byte[GUItextcolor],222
%%zero
    GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro DrawSlideBar 10
    DrawSlideBarWin %1,%2,%3+8,%4,%5,%6,%7-16,%8
    mov al,[TextColorSp+2]
    mov byte[GUItextcolor],al
    cmp byte[GUICHold],%9
    jne .noholda
    add byte[GUIWincoladd],3
.noholda
    GUIDisplayIconWin %1,%2,%3,GUIIconDataUpArrow
    cmp byte[GUICHold],%9
    jne .noholda2
    sub byte[GUIWincoladd],3
.noholda2
    mov al,[TextColorSp+2]
    mov byte[GUItextcolor],al
    cmp byte[GUICHold],%10
    jne .noholdb
    add byte[GUIWincoladd],3
.noholdb
    GUIDisplayIconWin %1,%2,%3+%7-8,GUIIconDataDownArrow
    cmp byte[GUICHold],%10
    jne .noholdb2
    sub byte[GUIWincoladd],3
.noholdb2
%endmacro

%macro DrawCheckBox 4
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[%4],0
    je %%nocheck4
    mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheck4
    GUIDisplayIconWin %1,%2,%3,[GUITemp]
%endmacro


%macro DetermineChar 2
    cmp al,%1
    jne %%control
    mov al,%2
%%control
%endmacro

DisplayGUICombo:
    mov eax,[GUIccombcursloc]
    cmp [GUIccomblcursloc],eax
    je .nolastcurschange
    mov [GUIccomblcursloc],eax
    ; copy contents into temporary variables
    mov esi,[GUIccombcursloc]
    shl esi,6
    add esi,[GUIccombcursloc]
    add esi,[GUIccombcursloc]
    add esi,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespecb
    sub esi,CombinDataGlob
    add esi,CombinDataLocl
.nogamespecb
    ; copy 20 bytes to GUIComboTextH
    mov ecx,20
    mov edi,GUIComboTextH
    rep movsb
    ; copy 42 bytes to GUIComboData
    mov ecx,42
    mov edi,GUIComboData
    rep movsb
    mov ax,[esi]
    mov [GUIComboKey],ax
    mov al,[esi+2]
    mov [GUIComboPNum],al
    mov al,[esi+3]
    mov [GUIComboLHorz],al
    ; determine length of combo data
    mov eax,GUIComboData
    xor ecx,ecx
.notfin
    cmp byte[eax],0
    je .fin
    inc eax
    inc ecx
    jmp .notfin
.fin
    mov [GUINumCombo],cl
.nolastcurschange

    ; copy into data if description equal
    mov esi,[GUIccombcursloc]
    shl esi,6
    add esi,[GUIccombcursloc]
    add esi,[GUIccombcursloc]
    add esi,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespecc
    sub esi,CombinDataGlob
    add esi,CombinDataLocl
.nogamespecc
    mov ecx,20
    mov edi,GUIComboTextH
.comploop
    mov al,[esi]
    cmp al,[edi]
    jne near .notequal
    or al,al
    jz .donecomp
    inc esi
    inc edi
    dec ecx
    jnz .comploop
.donecomp
    call ComboClip
    mov edi,[GUIccombcursloc]
    shl edi,6
    add edi,[GUIccombcursloc]
    add edi,[GUIccombcursloc]
    add edi,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespecd
    sub edi,CombinDataGlob
    add edi,CombinDataLocl
.nogamespecd
    mov ecx,20
    mov esi,GUIComboTextH
    rep movsb
    ; copy 42 bytes to GUIComboData
    mov ecx,42
    mov esi,GUIComboData
    rep movsb
    mov ax,[GUIComboKey]
    mov [edi],ax
    mov al,[GUIComboPNum]
    mov [edi+2],al
    mov al,[GUIComboLHorz]
    mov [edi+3],al
.notequal
    InitTextColor

    cmp byte[GUIWincoladd],0
    je .zero4
    mov byte[GUItextcolor],211
.zero4

    GUIDrawWindowBox 16,GUIComboDisp

    DrawBorderedBox 16,10,20,190,80,19
    DrawBorderedBox 16,10,91,130,99,90
    DrawBorderedBox 16,135,91,157,99,90
    DrawBorderedBox 16,10,110,220,146,109

    ; Draw SlideBar
    ; win#,X,Y start,List Loc,List size,# Lines,Bar Size(Y),UpArrowResource#,DownArrowRes#
    DrawSlideBar 16,192,20,[GUIccombviewloc],[NumCombo],8,61,GUICSStC,13,14

    ; Draw control boxes
    DrawBorderedBoxB 16,75,150,85,157,149,251
    DrawBorderedBoxB 16,89,150,99,157,149,252
    DrawBorderedBoxB 16,103,150,113,157,149,253
    DrawBorderedBoxB 16,117,150,127,157,149,254
    DrawBorderedBoxB 16,131,150,141,157,149,'A'
    DrawBorderedBoxB 16,145,150,155,157,149,'B'
    DrawBorderedBoxB 16,159,150,169,157,149,'X'
    DrawBorderedBoxB 16,173,150,183,157,149,'Y'
    DrawBorderedBoxB 16,187,150,197,157,149,'L'
    DrawBorderedBoxB 16,201,150,211,157,149,'R'
    DrawBorderedBoxB2 16,215,150,227,157,149,'T'
    DrawBorderedBoxB2 16,231,150,243,157,149,'L'

    DrawBorderedBoxB 16,75,160,85,167,159,251
    DrawBorderedBoxB 16,89,160,99,167,159,252
    DrawBorderedBoxB 16,103,160,113,167,159,253
    DrawBorderedBoxB 16,117,160,127,167,159,254
    DrawBorderedBoxB 16,131,160,141,167,159,'A'
    DrawBorderedBoxB 16,145,160,155,167,159,'B'
    DrawBorderedBoxB 16,159,160,169,167,159,'X'
    DrawBorderedBoxB 16,173,160,183,167,159,'Y'
    DrawBorderedBoxB 16,187,160,197,167,159,'L'
    DrawBorderedBoxB 16,201,160,211,167,159,'R'
    DrawBorderedBoxB2 16,215,160,227,167,159,'T'
    DrawBorderedBoxB2 16,231,160,243,167,159,'L'

    DrawBorderedBoxB 16,75,170,85,177,169,251
    DrawBorderedBoxB 16,89,170,99,177,169,252
    DrawBorderedBoxB 16,103,170,113,177,169,253
    DrawBorderedBoxB 16,117,170,127,177,169,254
    DrawBorderedBoxB 16,131,170,141,177,169,'A'
    DrawBorderedBoxB 16,145,170,155,177,169,'B'
    DrawBorderedBoxB 16,159,170,169,177,169,'X'
    DrawBorderedBoxB 16,173,170,183,177,169,'Y'
    DrawBorderedBoxB 16,187,170,197,177,169,'L'
    DrawBorderedBoxB 16,201,170,211,177,169,'R'
    DrawBorderedBoxB2 16,215,170,227,177,169,'T'
    DrawBorderedBoxB2 16,231,170,243,177,169,'L'

    DrawBorderedBoxB 16,10,189,20,196,188,'1'
    DrawBorderedBoxB 16,24,189,34,196,188,'2'
    DrawBorderedBoxB 16,38,189,48,196,188,'3'
    DrawBorderedBoxB 16,52,189,62,196,188,'4'
    DrawBorderedBoxB 16,66,189,76,196,188,'5'
    DrawBorderedBoxB 16,80,189,90,196,188,'9'

    DrawBorderedBoxB 16,107,189,117,196,188,250
    DrawBorderedBoxB 16,121,189,131,196,188,'1'
    DrawBorderedBoxB 16,135,189,145,196,188,'2'
    DrawBorderedBoxB 16,149,189,159,196,188,'3'
    DrawBorderedBoxB 16,163,189,173,196,188,'4'
    DrawBorderedBoxB 16,177,189,187,196,188,'5'

    DrawBorderedBoxB 16,204,189,218,196,188,255

    ; Normal Text
    mov byte[GUItextcolor],217
    cmp byte[GUIWincoladd],0
    je .zero5
    mov byte[GUItextcolor],211
.zero5
    sub byte[GUItextcolor],15
    GUIOuttextwin2 16,10,13,GUIComboText1
    GUIOuttextwin2 16,138,13,GUIComboText2
    GUIOuttextwin2 16,10,84,GUIComboTextE
    GUIOuttextwin2 16,138,84,GUIComboTextF
    GUIOuttextwin2 16,10,103,GUIComboText4
    GUIOuttextwin2 16,10,152,GUIComboText5
    GUIOuttextwin2 16,10,162,GUIComboText6
    GUIOuttextwin2 16,10,172,GUIComboText7
    GUIOuttextwin2 16,10,182,GUIComboText8
    GUIOuttextwin2 16,114,182,GUIComboText9
    GUIOuttextwin2 16,204,182,GUIComboTextG
    GUIOuttextwin2 16,165,85,GUIComboTextI
    GUIOuttextwin2 16,176,93,GUIComboTextJ
    GUIOuttextwin2 16,176,101,GUIComboTextK

    add byte[GUItextcolor],15
    GUIOuttextwin2 16,9,12,GUIComboText1
    GUIOuttextwin2 16,137,12,GUIComboText2
    GUIOuttextwin2 16,9,83,GUIComboTextE
    GUIOuttextwin2 16,137,83,GUIComboTextF
    GUIOuttextwin2 16,9,102,GUIComboText4
    GUIOuttextwin2 16,9,151,GUIComboText5
    GUIOuttextwin2 16,9,161,GUIComboText6
    GUIOuttextwin2 16,9,171,GUIComboText7
    GUIOuttextwin2 16,9,181,GUIComboText8
    GUIOuttextwin2 16,113,181,GUIComboText9
    GUIOuttextwin2 16,203,181,GUIComboTextG
    GUIOuttextwin2 16,164,84,GUIComboTextI
    GUIOuttextwin2 16,175,92,GUIComboTextJ
    GUIOuttextwin2 16,175,100,GUIComboTextK

    ; Buttons
    DrawGUIButton 16,202,20,246,31,GUIComboTextA,60,-1,0
    DrawGUIButton 16,202,35,246,46,GUIComboTextB,61,-1,0
    DrawGUIButton 16,202,50,246,61,GUIComboTextC,62,-1,0
    DrawGUIButton 16,202,65,246,76,GUIComboTextD,63,-1,0

    ; Calculate Text Cursor Position / Draw Box Text
    xor eax,eax
.nozerocp
    cmp byte[GUIComboTextH+eax],0
    je .zerocp
    inc eax
    jmp .nozerocp
.zerocp
    mov [GUIComboPos],al
    test byte[GUICCFlash],8
    jnz .nound
    mov byte[GUIComboTextH+eax],'_'
    mov byte[GUIComboTextH+eax+1],0
.nound
    GUIOuttextwin2c 16,12,94,GUIComboTextH
    xor eax,eax
    mov al,[GUIComboPos]
    mov byte[GUIComboTextH+eax],0

    ; Display Current Combo Key
    mov eax,[GUIComboKey]
    mov ebx,[ScanCodeListing+eax*3]
    mov [GUIGameDisplayKy],ebx
    mov byte[GUIGameDisplayKy+3],0
    GUIOuttextwin2c 16,139,94,GUIGameDisplayKy

    ; Buttons
    mov al,[TextColorSp+2]
    add al,15
    mov byte[GUItextcolor],al
    DrawGUIButton 16,202,20,246,31,GUIComboTextA,60,-1,0
    DrawGUIButton 16,202,35,246,46,GUIComboTextB,61,-1,0
    DrawGUIButton 16,202,50,246,61,GUIComboTextC,62,-1,0
    DrawGUIButton 16,202,65,246,76,GUIComboTextD,63,-1,0

    ; Hole Switches
    GUIDisplayButtonHole 16,178,82,byte[GUIComboPNum],0
    GUIDisplayButtonHole 16,196,82,byte[GUIComboPNum],1
    GUIDisplayButtonHole 16,214,82,byte[GUIComboPNum],2
    GUIDisplayButtonHole 16,232,82,byte[GUIComboPNum],3

    ; Check Box
    DrawCheckBox 16,163,88,GUIComboLHorz
    DrawCheckBox 16,163,96,GUIComboGameSpec

    ; Draw Combination Keys (Each 15x11 -> 210x36)
    xor eax,eax
    mov al,[GUINumCombo]
    or eax,eax
    jz near .comboend
    xor ebx,ebx
    mov ecx,11
    mov edx,112
.comboiconloop
    push eax
    mov al,[GUIComboData+ebx]
    dec al
    mov esi,GUIIconDataComboPressRelease
    cmp al,12
    jb .doneicon
    mov esi,GUIIconDataComboPress
    sub al,12
    cmp al,12
    jb .doneicon
    mov esi,GUIIconDataComboRelease
    sub al,12
    cmp al,12
    jb .doneicon
    mov esi,GUIIconDataComboFrame
    cmp al,17
    jne .not9
    mov al,20
    jmp .doneicon
.not9
    cmp al,18
    jb .doneicon
    sub al,7
    mov esi,GUIIconDataComboSecond
    cmp al,11
    jne .notback
    mov al,250-37
    jmp .doneicon
.notback
.doneicon
    cmp al,11
    jbe .control
    add al,37
.control
    DetermineChar 0,251
    DetermineChar 1,252
    DetermineChar 2,253
    DetermineChar 3,254
    DetermineChar 4,'A'
    DetermineChar 5,'B'
    DetermineChar 6,'X'
    DetermineChar 7,'Y'
    DetermineChar 8,'L'
    DetermineChar 9,'R'
    DetermineChar 10,'T'
    DetermineChar 11,'E'
    mov [GUIComboText3],al
    pushad
    mov eax,edx
    add ecx,8
    add eax,5

    mov byte[GUItextcolor],223
    push ecx
    push eax
    GUIOuttextwin2 16,ecx,eax,GUIComboText3
    pop eax
    pop ecx
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zerocol
    mov byte[GUItextcolor],222
.zerocol
    dec ecx
    dec eax
    GUIOuttextwin2 16,ecx,eax,GUIComboText3

    popad
    push ecx
    push edx
    push ebx
    GUIDisplayIconWin 16,ecx,edx,esi
    pop ebx
    pop edx
    pop ecx
    pop eax
    add ecx,15
    cmp ecx,11+15*14
    jne .notoverflowx
    sub ecx,15*14
    add edx,11
.notoverflowx
    inc ebx
    dec eax
    jnz near .comboiconloop
.comboend

    ; Display Bordered Box
    cmp dword[NumCombo],0
    je near .nowinbox
    mov ebx,[GUIccombcursloc]
    sub ebx,[GUIccombviewloc]
    mov eax,ebx
    lea ebx,[ebx*8]
    sub ebx,eax
    add ebx,23
    DrawGUIWinBox2 16,10,190,7,224
.nowinbox

    ; Display Scroll Lines
    ; Copy Description to GUIScrolBufA, Others to GUIScrolBufB
    mov edi,[GUIccombviewloc]
    shl edi,6
    add edi,[GUIccombviewloc]
    add edi,[GUIccombviewloc]
    add edi,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespec
    sub edi,CombinDataGlob
    add edi,CombinDataLocl
.nogamespec

    mov ecx,12
    mov eax,25
    mov ebx,[NumCombo]
    sub ebx,[GUIccombviewloc]
    cmp ebx,0
    jle near .noscrolldisplay
    cmp ebx,8
    jbe .scrolloop
    mov ebx,8
.scrolloop
    push ebx
    push edi
    push eax

    xor eax,eax
    mov ax,[edi+62]
    mov eax,[ScanCodeListing+eax*3]
    mov [GUIScrolBufB],eax
    mov byte[GUIScrolBufB+3],32
    mov dword[GUIScrolBufB+4],20202020h
    mov al,[edi+64]
    add al,49
    mov [GUIScrolBufB+4],al
    mov byte[GUIScrolBufB+7],'N'
    cmp byte[edi+65],0
    je .yesnoff
    mov byte[GUIScrolBufB+7],'Y'
.yesnoff
    mov bl,20
    mov eax,GUIScrolBufA
.scloopb
    mov bh,[edi]
    mov [eax],bh
    inc edi
    inc eax
    dec bl
    jnz .scloopb
    pop eax

    mov byte[GUItextcolor],223
    push ecx
    push eax
    GUIOuttextwin2 16,ecx,eax,GUIScrolBufA
    pop eax
    pop ecx
    push ecx
    push eax
    add ecx,128
    GUIOuttextwin2 16,ecx,eax,GUIScrolBufB
    pop eax
    pop ecx
    mov byte[GUItextcolor],221
    cmp byte[GUIWincoladd],0
    je .zerocolb
    mov byte[GUItextcolor],222
.zerocolb
    push ecx
    push eax
    dec ecx
    dec eax
    push ecx
    push eax
    GUIOuttextwin2 16,ecx,eax,GUIScrolBufA
    pop eax
    pop ecx
    add ecx,128
    GUIOuttextwin2 16,ecx,eax,GUIScrolBufB
    pop eax
    pop ecx
    pop edi
    pop ebx
    add eax,7
    add edi,66
    dec ebx
    jnz near .scrolloop
.noscrolldisplay
    ret

SECTION .bss
GUIccombviewloc resd 1
GUIccombcursloc resd 1
GUIccomblcursloc resd 1
NEWSYM NumCombo,     resd 1
NumComboGlob    resd 1
NEWSYM NumComboLocl, resd 1
GUICSStC        resd 3
GUIComboKey     resd 1
GUIComboPos     resb 1
GUIComboPNum    resb 1
GUIComboLHorz   resb 1
NEWSYM GUINumCombo, resb 1
GUIComboData resb 50
GUIScrolBufA resb 21
GUIScrolBufB resb 10


;CombinDataGlob times 3300 db 0 ; 20-name, 42-combo, 2-key#, 1-P#, 1-ff
;CombinDataLocl times 3300 db 0

SECTION .data
GUIComboText1 db 'DESCRIPTION',0
GUIComboText2 db 'KEY P# LH',0
GUIComboText3 db ' ',0
GUIComboText3b db 'S ',0
GUIComboText4 db 'COMBINATION KEYS:',0
GUIComboText5 db 'PRESS+REL',0
GUIComboText6 db 'PRESS ONLY',0
GUIComboText7 db 'REL ONLY',0
GUIComboText8 db 'FRAME DELAY',0
GUIComboText9 db 'SECOND DELAY',0
GUIComboTextA db 'CLEAR',0
GUIComboTextB db 'ADD',0
GUIComboTextC db 'REPLACE',0
GUIComboTextD db 'DELETE',0
GUIComboTextE db 'DESCRIPTION:',0
GUIComboTextF db 'KEY:',0
GUIComboTextG db 'DEL',0
GUIComboTextH db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
GUIComboTextI db 'P#  1  2  3  4',0
GUIComboTextJ db 254,' = LAST ',253,'/',254,0
GUIComboTextK db 'GAME SPECFIC',0

GUIIconDataComboPressRelease:
    db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
    db 0  ,166,0  ,166,166,166,0  ,0  ,0  ,0
    db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
    db 166,166,166,0  ,166,0  ,0  ,0  ,0  ,0
    db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
    db 0  ,234,234,234,234,0  ,0  ,0  ,0  ,0
    db 234,234,234,234,234,234,0  ,0  ,0  ,0
    db 234,234,234,234,234,234,0  ,0  ,0  ,0
    db 235,234,234,234,234,235,0  ,0  ,0  ,0
    db 0  ,235,235,235,235,0  ,0  ,0  ,0  ,0

GUIIconDataComboPress:
    db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,166,166,166,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,236,236,236,236,0  ,0  ,0  ,0  ,0
    db 236,236,236,236,236,236,0  ,0  ,0  ,0
    db 236,236,236,236,236,236,0  ,0  ,0  ,0
    db 237,236,236,236,236,237,0  ,0  ,0  ,0
    db 0  ,237,237,237,237,0  ,0  ,0  ,0  ,0

GUIIconDataComboRelease:
    db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,166,166,166,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,238,238,238,238,0  ,0  ,0  ,0  ,0
    db 238,238,238,238,238,238,0  ,0  ,0  ,0
    db 238,238,238,238,238,238,0  ,0  ,0  ,0
    db 239,238,238,238,238,239,0  ,0  ,0  ,0
    db 0  ,239,239,239,239,0  ,0  ,0  ,0  ,0

GUIIconDataComboFrame:
    db 0  ,0  ,0  ,0  ,58 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,0  ,56 ,46 ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,0  ,54 ,44 ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,52 ,0  ,42 ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,50 ,40 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,48 ,38 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,46 ,36 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,44 ,0  ,34 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,42 ,32 ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,40 ,32 ,0  ,0  ,0  ,0  ,0  ,0

GUIIconDataComboSecond:
    db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,51 ,45 ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,51 ,45 ,0  ,0  ,0  ,0  ,0  ,0
    db 0  ,54 ,52 ,48 ,46 ,0  ,0  ,0  ,0  ,0
    db 53 ,46 ,50 ,50 ,48 ,45 ,0  ,0  ,0  ,0
    db 52 ,50 ,45 ,48 ,46 ,44 ,0  ,0  ,0  ,0
    db 51 ,50 ,50 ,46 ,50 ,43 ,0  ,0  ,0  ,0
    db 50 ,50 ,50 ,50 ,50 ,42 ,0  ,0  ,0  ,0
    db 0  ,48 ,46 ,44 ,42 ,0  ,0  ,0  ,0  ,0
    db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

SECTION .text

DisplayGUIAddOns:
    GUIDrawWindowBox 17,GUIAddOnsDisp
    sub byte[GUItextcolor],15
    GUIOuttextwin2 17,21,18,GUIAddOnText1
    GUIOuttextwin2 17,21,28,GUIAddOnText2
    GUIOuttextwin2 17,21,38,GUIAddOnText3
    GUIOuttextwin2 17,21,48,GUIAddOnText4
    GUIOuttextwin2 17,21,58,GUIAddOnText5
    GUIOuttextwin2 17,10,78,GUIAddOnText6
    GUIOuttextwin2 17,21,88,GUIAddOnText7
    GUIOuttextwin2 17,71,88,GUIAddOnText8
    GUIOuttextwin2 17,121,88,GUIAddOnText9
    GUIOuttextwin2 17,21,98,GUIAddOnTextA
    GUIOuttextwin2 17,71,98,GUIAddOnTextB
    GUIOuttextwin2 17,121,98,GUIAddOnTextC
    add byte[GUItextcolor],15
    GUIOuttextwin2 17,20,17,GUIAddOnText1
    GUIOuttextwin2 17,20,27,GUIAddOnText2
    GUIOuttextwin2 17,20,37,GUIAddOnText3
    GUIOuttextwin2 17,20,47,GUIAddOnText4
    GUIOuttextwin2 17,20,57,GUIAddOnText5
    GUIOuttextwin2 17,9,77,GUIAddOnText6
    GUIOuttextwin2 17,20,87,GUIAddOnText7
    GUIOuttextwin2 17,70,87,GUIAddOnText8
    GUIOuttextwin2 17,120,87,GUIAddOnText9
    GUIOuttextwin2 17,20,97,GUIAddOnTextA
    GUIOuttextwin2 17,70,97,GUIAddOnTextB
    GUIOuttextwin2 17,120,97,GUIAddOnTextC
    GUIDisplayButtonHole 17,9,15,byte[snesmouse],0
    GUIDisplayButtonHole 17,9,25,byte[snesmouse],1
    GUIDisplayButtonHole 17,9,35,byte[snesmouse],2
    GUIDisplayButtonHole 17,9,45,byte[snesmouse],3
    GUIDisplayButtonHole 17,9,55,byte[snesmouse],4
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip1],0
    je .nodip1
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip1
    GUIDisplayIconWin 17,9,83,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip2],0
    je .nodip2
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip2
    GUIDisplayIconWin 17,59,83,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip3],0
    je .nodip3
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip3
    GUIDisplayIconWin 17,109,83,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip4],0
    je .nodip4
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip4
    GUIDisplayIconWin 17,9,93,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip5],0
    je .nodip5
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip5
    GUIDisplayIconWin 17,59,93,[GUITemp]
    mov dword[GUITemp],GUIIconDataCheckBoxUC
    cmp byte[nssdip6],0
    je .nodip6
    mov dword[GUITemp],GUIIconDataCheckBoxC
.nodip6
    GUIDisplayIconWin 17,109,93,[GUITemp]
    ret

SECTION .data
GUIAddOnText1 db 'NONE',0
GUIAddOnText2 db 'SNES MOUSE AT PORT #1',0
GUIAddOnText3 db 'SNES MOUSE AT PORT #2',0
GUIAddOnText4 db 'SUPER SCOPE',0
GUIAddOnText5 db 'LETHAL ENFORCER GUN',0
GUIAddOnText6 db 'SUPER SYSTEM',0
GUIAddOnText7 db 'DIP 1',0
GUIAddOnText8 db 'DIP 2',0
GUIAddOnText9 db 'DIP 3',0
GUIAddOnTextA db 'DIP 4',0
GUIAddOnTextB db 'DIP 5',0
GUIAddOnTextC db 'DIP 6',0

GUILoadDisp     db 'LOAD GAME',0
GUIResetDisp    db 'RESET GAME',0
GUIStateSelDisp db 'STATE SELECT',0
GUIInputDisp db 'INPUT DEVICE',0
GUIOptionDisp db 'OPTIONS',0
GUISoundDisp db 'SOUND CONFIG',0
GUIVideoDisp db 'VIDEO CONFIG',0
GUICheatDisp db 'CHEAT',0
GUISearchDisp db 'CHEAT SEARCH',0
GUINetDisp   db 'REMOTE',0,0,0,0,0,0,0,0,0
GUIGameDisp  db 'GAME KEYS',0
GUIGUIDisp   db 'GUI OPTIONS',0
GUIAboutDisp db 'ABOUT',0
GUIMovieDisp db 'MOVIE OPTIONS',0
GUIStatesDisp db 'STATE CONFIRM',0
GUIComboDisp db 'KEY COMBINATION EDITOR',0
GUIAddOnsDisp db 'ADD-ON SELECTOR',0

SECTION .bss
GUIItemBoxColor resd 1
TextColorSp resd 1

SECTION .data
NEWSYM GUIMenuItem, db 'GAME  ',0
            db 'CONFIG',0
            db 'CHEAT ',0
            db 'NETPLAY',0
            db 'MISC  ',0
            db 25,0

SECTION .text
