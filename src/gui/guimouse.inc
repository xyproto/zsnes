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

%macro GUITextBoxInputNach 7
  GUIClickArea %1,%2,%3,%4,%%noclick
  ccallv %7
  mov dword[GUIInputBox],%5+1
  mov dword[GUIInputLimit],%6-1
%%noclick
%endmacro

%macro GUIPButtonHole 4
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
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

%macro GUIPTabClick 4-* ; minX, maxX, value, var, vars to zero
  GUIClickArea %1+1,11,%2-1,22,%%noclick
  mov dword[GUIInputBox],0
  mov dword[%4],%3
%if (%0>4)
  %rotate 4
%rep (%0-4)
  mov dword[%1],0
  %rotate 1
%endrep
%endif
  mov byte[GUIFreshInputSelect],1
%%noclick
%endmacro

; Mouse implementation

SECTION .bss
NEWSYM GUINumValue, resd 1
NEWSYM GUIBlankVar, resd 1
SECTION .data
NEWSYM NTSCPresetVar, dd 4

SECTION .text

NEWSYM DisplayGUIAddOnClick
  GUIPButtonHole 9,23,byte[device1],0
  GUIPButtonHole 9,33,byte[device1],1

  GUIPButtonHole 100,23,byte[device2],0
  GUIPButtonHole 100,33,byte[device2],1
  GUIPButtonHole 100,43,byte[device2],2
  GUIPButtonHole 100,53,byte[device2],3
  GUIPButtonHole 100,63,byte[device2],4

  DGOptnsProcBox 74,76,[KeyExtraEnab1]
  DGOptnsProcBox 165,76,[KeyExtraEnab2]

  GUIClickCButton 9,83,byte[mouse1lh]
  GUIClickCButton 100,83,byte[mouse2lh]

  DGOptnsProcBox 74,108,[SSAutoFire]
  DGOptnsProcBox 165,108,[SSPause]
  ret

NEWSYM DisplayGUIChipClick
  GUIClickCButton 9,43,byte[nssdip1]
  GUIClickCButton 59,43,byte[nssdip2]
  GUIClickCButton 109,43,byte[nssdip3]
  GUIClickCButton 9,53,byte[nssdip4]
  GUIClickCButton 59,53,byte[nssdip5]
  GUIClickCButton 109,53,byte[nssdip6]
  ret

%ifdef __MSDOS__
%define path_length 256
%else
%define path_length 1024
%endif

NEWSYM DisplayGUIPathsClick
  GUIPTabClick 0,51,1,GUIPathTabs
  GUIPTabClick 52,86,2,GUIPathTabs
  GUIPTabClick 87,157,3,GUIPathTabs

  cmp dword[GUIPathTabs],1
  jne near .notgeneral

  GUITextBoxInputNach 8,41,237,51,0,path_length,init_save_paths        ;SRAMPath
  GUITextBoxInputNach 8,76,237,86,1,path_length,init_save_paths        ;SStatePath
  GUITextBoxInputNach 8,111,237,121,2,path_length,init_save_paths      ;MoviePath
  GUITextBoxInputNach 8,146,237,156,3,path_length,init_save_paths      ;IPSPath

  GUIPButtonHole 8,178,byte[RelPathBase],0
  GUIPButtonHole 88,178,byte[RelPathBase],1
.notgeneral

  cmp dword[GUIPathTabs],2
  jne near .notmorepaths

  GUITextBoxInputNach 8,41,237,51,0,path_length,init_save_paths        ;SnapPath
  GUITextBoxInputNach 8,76,237,86,1,path_length,init_save_paths        ;SPCPath
  GUITextBoxInputNach 8,111,237,121,2,path_length,init_save_paths      ;CHTPath
  GUITextBoxInputNach 8,146,237,156,3,path_length,init_save_paths      ;ComboPath
  GUITextBoxInputNach 8,181,237,191,4,path_length,init_save_paths      ;INPPath
.notmorepaths

  cmp dword[GUIPathTabs],3
  jne near .notbc

  GUITextBoxInputNach 8,41,237,51,0,path_length,init_save_paths        ;BSXPath
  GUITextBoxInputNach 8,76,237,86,1,path_length,init_save_paths        ;STPath
  GUITextBoxInputNach 8,111,237,121,2,path_length,init_save_paths      ;GNextPath
  GUITextBoxInputNach 8,146,237,156,3,path_length,init_save_paths      ;SGPath
.notbc
  ret

NEWSYM DisplayGUISaveClick
  GUIClickCButton 11,38,byte[nosaveSRAM]
  GUIClickCButton 11,48,byte[SRAMSave5Sec]
  GUIClickCButton 11,58,byte[SRAMState]
  GUIClickCButton 11,68,byte[LatestSave]
  GUIClickCButton 11,78,byte[AutoIncSaveSlot]
  GUIClickCButton 11,88,byte[AutoState]
  GUIClickCButton 11,98,byte[PauseLoad]
  GUIClickCButton 11,108,byte[PauseRewind]
  GUIPHoldbutton2 173,17,181,24,70,byte[RewindStates],1,99
  GUIPHoldbutton2 184,17,192,24,71,byte[RewindStates],-1,0
  GUIPHoldbutton2 173,29,181,36,72,byte[RewindFrames],1,99
  GUIPHoldbutton2 184,29,192,36,73,byte[RewindFrames],-1,1
  DGOptnsProcBox 27,130,[KeyStateSlc0]
  DGOptnsProcBox 27+45,130,[KeyStateSlc1]
  DGOptnsProcBox 27+45*2,130,[KeyStateSlc2]
  DGOptnsProcBox 27+45*3,130,[KeyStateSlc3]
  DGOptnsProcBox 27,139,[KeyStateSlc4]
  DGOptnsProcBox 27+45,139,[KeyStateSlc5]
  DGOptnsProcBox 27+45*2,139,[KeyStateSlc6]
  DGOptnsProcBox 27+45*3,139,[KeyStateSlc7]
  DGOptnsProcBox 27,148,[KeyStateSlc8]
  DGOptnsProcBox 27+45,148,[KeyStateSlc9]
  DGOptnsProcBox 27+45*2,148,[KeyIncStateSlot]
  DGOptnsProcBox 27+45*3,148,[KeyDecStateSlot]
  DGOptnsProcBox 8+25,157,[KeySaveState]
  DGOptnsProcBox 8+57+25,157,[KeyLoadState]
  DGOptnsProcBox 8+114+25,157,[KeyStateSelct]
  DGOptnsProcBox 8+38,166,[KeyRewind]
  ret

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
