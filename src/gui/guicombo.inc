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



SECTION .text

ComboClip:
    movzx eax,byte[GUINumCombo]
.next
    cmp al,42
    jae .excess
    mov byte[GUIComboData+eax],0
    inc eax
    jmp .next
.excess
    ret

ComboAdder:
    cmp byte[romloadskip],0
    je .romloaded
    cmp byte[GUIComboGameSpec],0
    jne near .skipall
.romloaded
    pushad
    call ComboClip
    mov eax,[NumCombo]
    shl eax,6
    add eax,[NumCombo]
    add eax,[NumCombo]
    add eax,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespec
    sub eax,CombinDataGlob
    add eax,CombinDataLocl
.nogamespec
    ; copy data to eax
    mov ecx,20  ; copy name
    mov ebx,GUIComboTextH
.loop
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    dec ecx
    jnz .loop
    mov ecx,42  ; copy combination code
    mov ebx,GUIComboData
.loop2
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    dec ecx
    jnz .loop2
    mov dx,[GUIComboKey]
    mov [eax],dx
    mov dl,[GUIComboPNum]
    mov [eax+2],dl
    mov dl,[GUIComboLHorz]
    mov [eax+3],dl
    mov eax,[NumCombo]
    mov [GUIccombcursloc],eax
    sub eax,7
    cmp [GUIccombviewloc],eax
    jge .rangeokay
    mov [GUIccombviewloc],eax
.rangeokay
    inc dword[NumCombo]
    mov eax,[NumCombo]
    cmp byte[GUIComboGameSpec],0
    jne .local
    mov [NumComboGlob],eax
    jmp .global
.local
    mov [NumComboLocl],eax
.global
    popad
    mov byte[GUIComboTextH],0
    mov byte[GUINumCombo],0
    mov dword[GUIComboKey],0
.skipall
    ret

ComboReplace:
    pushad
    call ComboClip
    mov eax,[GUIccombcursloc]
    shl eax,6
    add eax,[GUIccombcursloc]
    add eax,[GUIccombcursloc]
    add eax,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespec
    sub eax,CombinDataGlob
    add eax,CombinDataLocl
.nogamespec
    ; copy data to eax
    mov ecx,20  ; copy name
    mov ebx,GUIComboTextH
.loop
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    dec ecx
    jnz .loop
    mov ecx,42  ; copy combination code
    mov ebx,GUIComboData
.loop2
    mov dl,[ebx]
    mov [eax],dl
    inc ebx
    inc eax
    dec ecx
    jnz .loop2
    mov dx,[GUIComboKey]
    mov [eax],dx
    mov dl,[GUIComboPNum]
    mov [eax+2],dl
    mov dl,[GUIComboLHorz]
    mov [eax+3],dl
    popad
    ret

ComboRemoval:
    pushad
    mov eax,[GUIccombcursloc]
    shl eax,6
    add eax,[GUIccombcursloc]
    add eax,[GUIccombcursloc]
    add eax,CombinDataGlob
    cmp byte[GUIComboGameSpec],0
    je .nogamespec
    sub eax,CombinDataGlob
    add eax,CombinDataLocl
.nogamespec
    mov ecx,[NumCombo]
    sub ecx,[GUIccombcursloc]
    dec ecx
    cmp ecx,0
    jle .nomove
.loop2
    mov ebx,66
.loop
    mov dl,[eax+66]
    mov [eax],dl
    inc eax
    dec ebx
    jnz .loop
    dec ecx
    jnz .loop2
.nomove
    dec dword[NumCombo]
    mov eax,[NumCombo]
    or eax,eax
    jz .yeszero
    dec eax
.yeszero
    cmp [GUIccombviewloc],eax
    jbe .notequal
    mov [GUIccombviewloc],eax
.notequal
    cmp [GUIccombcursloc],eax
    jbe .notequal2
    mov [GUIccombcursloc],eax
.notequal2
    mov eax,[NumCombo]
    cmp byte[GUIComboGameSpec],0
    jne .local
    mov [NumComboGlob],eax
    jmp .global
.local
    mov [NumComboLocl],eax
.global
    popad
    ret
