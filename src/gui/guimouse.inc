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



EXTSYM guipresstest

SECTION .text

%macro GUIClickArea 5
  cmp eax,%1
  jl %5
  cmp edx,%2
  jl %5
  cmp eax,%3
  jg %5
  cmp edx,%4
  jg %5
%endmacro

%macro GUIClickCButton 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%%noclick
%endmacro

%ifdef __MSDOS__
%macro GUIClickCButton3 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  DOSClearScreenMouse
  ret
%%noclick
%endmacro

%macro DOSClearScreenMouse 0
  call DOSClearScreen
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
%%loop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz %%loop
  pop ecx
  pop eax
  ret
%endmacro
%endif

%macro GUIClickCButtonfr 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  mov byte[FPSOn],0
  cmp %3,0
  je %%zero
  mov %3,0
  jmp %%notzero
%%zero
  mov %3,1
%%notzero
%%noclick
%endmacro

%macro GUIClickCButton5MMXC 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp byte[MMXSupport],0
  je %%clear
  cmp %3,%4
  jne %%set
  xor %3,%4
  jmp %%clear
%%set
  mov %3,%4
%%clear
  ret
%%noclick
%endmacro

SECTION .bss
NEWSYM GUIlastdispval, resd 1
NEWSYM GUIdispmode,    resd 1
NEWSYM GUIlastypos,    resd 1
NEWSYM GUIlastcpos,    resd 1
NEWSYM GUIlastvpos,    resd 1
SECTION .text

%macro GUIPHoldbutton2 8
  GUIClickArea %1,%2,%3,%4,%%nohold
  cmp byte[GUIHold],0
  jne %%nohold
  mov dword[GUIHoldXlimL],%1
  mov dword[GUIHoldXlimR],%3
  mov dword[GUIHoldYlim],%2
  mov dword[GUIHoldYlimR],%4
  mov byte[GUICBHold2],%5
  mov byte[GUIHold],4
  cmp %6,%8
  je %%nohold
  add %6,%7
%%nohold
%endmacro

%macro GUITextBoxInputB 5
  GUIClickArea %1,%2,%3,%4,%%noclick
  mov byte[GUINetTextm2],%5
%%noclick
%endmacro

%macro GUIPButtonHole2 6
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
  mov %5,%6
%%noclick
%endmacro

%macro DGOptnsProcBox 3
  GUIClickArea %1,%2,%1+19,%2+6,%%skip
  push edi
  ccall guipresstest
  mov ebx, eax
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %3,ebx
%ifndef __MSDOS__
  cmp dword[keycontrolval],0
  je %%notwin32
  push ebx
  mov ebx,[keycontrolval]
  mov byte[ebx],1
  pop ebx
%%notwin32
%endif
  ret
%%cancelkey
  xor ebx,ebx
  mov %3,ebx
  ret
%%skip
%endmacro

%macro DGOptnsProcBoxb 3
  GUIClickArea %1,%2,%1+19,%2+6,%%skip
  push edi
  ccall guipresstest
  mov ebx, eax
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %3,ebx
  jmp %%donekey
%%cancelkey
  cmp ebx,1
  jne %%noesc
  mov byte[pressed+1],2
%%noesc
  xor ebx,ebx
  mov %3,ebx
%%donekey
  ret
%%skip
%endmacro

; Mouse implementation

SECTION .bss
NEWSYM GUINumValue, resd 1
NEWSYM GUIBlankVar, resd 1
SECTION .data
NEWSYM NTSCPresetVar, dd 4

SECTION .text

NEWSYM DisplayGUISpeedClick
  mov eax,[GUImouseposx]    ; this updates the mouse location
  sub eax,[GUIwinposx+21*4] ; effectively 'grabbing' the cursor !
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+21*4]

  GUIClickCButton 11,135,byte[FastFwdToggle]     ; Checkboxes
  GUIClickCButtonfr 11,145,byte[frameskip]

  cmp byte[frameskip],0
  jne near .frate
  DGOptnsProcBox 12,78,[KeyEmuSpeedUp]
  DGOptnsProcBox 12,98,[KeyEmuSpeedDown]
  jmp near .endsc
.frate
  DGOptnsProcBox 12,78,[KeyFRateUp]
  DGOptnsProcBox 12,98,[KeyFRateDown]
.endsc

  DGOptnsProcBox 12,58,[KeyFastFrwrd]        ; Shortcut Boxes
  DGOptnsProcBox 12,68,[KeySlowDown]
  DGOptnsProcBox 12,88,[KeyResetSpeed]
  DGOptnsProcBox 12,108,[EMUPauseKey]
  DGOptnsProcBox 12,118,[INCRFrameKey]

  GUIPHoldbutton2 118,24,126,32,74,byte[FFRatio],1,28
  GUIPHoldbutton2 129,24,137,32,75,byte[FFRatio],-1,0
  GUIPHoldbutton2 118,35,126,43,76,byte[SDRatio],1,28
  GUIPHoldbutton2 129,35,137,43,77,byte[SDRatio],-1,0
  cmp byte[frameskip],0
  je near .autopress
  GUIPHoldbutton2 118,13,126,21,78,byte[frameskip],1,10
  GUIPHoldbutton2 129,13,137,21,79,byte[frameskip],-1,1
  ret

.autopress
  GUIPHoldbutton2 118,13,126,21,78,byte[maxskip],1,9
  GUIPHoldbutton2 129,13,137,21,79,byte[maxskip],-1,0

  ; Speed Slider
  cmp eax,7                      ; X-Range for click-area
  jl near .nomovebar
  cmp eax,7+116
  jg near .nomovebar
  sub eax,7
  shr eax,1
  cmp edx,173                    ; Y-Range for click-area
  jl near .nomovebar
  cmp edx,177
  jg near .nomovebar
  mov [EmuSpeed],al
  mov byte[GUIHold],6            ; Lock mouse to bar when clicked
  mov eax,[GUIwinposy+21*4]
  add eax,175
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposx+21*4]
  add eax,7
  mov [GUIHoldXlimL],eax
  add eax,116
  mov [GUIHoldXlimR],eax
.nomovebar
  ret
