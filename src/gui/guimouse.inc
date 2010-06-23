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



EXTSYM guipresstest, CheatCodeToggle

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

%macro GUISlidebarPostImpl 13 ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  cmp dword[%11],0
  je near %%nolower
  cmp dword[GUIdispmode],1
  je near %%slidebar
  GUIClickArea %1,%2-7,%3,%2-1,%%noupper
  mov byte[%12],%13
  mov byte[GUICHold],%7
  jmp %5
%%noupper
  GUIClickArea %1,%4+1,%3,%4+7,{near %%nolower}
  mov byte[%12],%13
  mov byte[GUICHold],%7+1
  jmp %6
%%slidebar
  GUIClickArea %1,%2,%3,%4,{near %%nolower}
  mov byte[%12],%13
  ; displacement = (GUIdispmode * pixeldisp. / (listsize-scsize))
  mov eax,[%11]
  sub eax,%8
  mov ebx,edx
  sub ebx,[GUIlastypos]
  imul ebx
  mov ebx,[GUIlastdispval]
  or ebx,ebx
  jz near %%zerodiv
  test ebx,80000000h
  jnz near %%zerodiv
  idiv ebx
  mov ebx,[GUIlastvpos]
  add ebx,eax
  mov [%9],ebx
  mov ebx,[GUIlastcpos]
  add ebx,eax
  mov [%10],ebx
  test dword[%9],8000000h
  jz %%noneg
  mov dword[%9],0
%%noneg
  test dword[%10],8000000h
  jz %%noneg2
  mov dword[%10],0
%%noneg2
  mov eax,[%11]
  dec eax
  cmp [%10],eax
  jb %%notover
  mov [%10],eax
%%notover
  sub eax,%8-1
  cmp [%9],eax
  jb %%notover2
  mov [%9],eax
%%notover2
%%zerodiv
  ret
%%nolower
%endmacro

SECTION .bss
NEWSYM GUIlastdispval, resd 1
NEWSYM GUIdispmode,    resd 1
NEWSYM GUIlastypos,    resd 1
NEWSYM GUIlastcpos,    resd 1
NEWSYM GUIlastvpos,    resd 1
SECTION .text

%macro GUISlidebarImpl 10 ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  mov dword[GUIdispmode],0
  GUIClickArea %1,%2,%3,%4,{near %%nobar}
  sub edx,%2
  cmp [%5+4],edx
  ja near %%above
  cmp [%5+8],edx
  jb near %%below
  add edx,%2
  mov [GUIlastypos],edx
  mov dword[GUIdispmode],1
  mov eax,[GUIwinposy+%10*4]
  add eax,%2
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%4
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov eax,[%5]
  mov [GUIlastdispval],eax
  mov eax,[%8]
  mov [GUIlastcpos],eax
  mov eax,[%7]
  mov [GUIlastvpos],eax
  mov byte[GUIHold],3
  ret
%%above
  sub dword[%7],%6
  sub dword[%8],%6
  test dword[%7],8000000h
  jz %%noneg
  mov dword[%7],0
  mov dword[%8],0
%%noneg
  ret
%%below
  add dword[%7],%6
  add dword[%8],%6
  mov ebx,[%9]
  dec ebx
  cmp [%8],ebx
  jb %%notoverflow
  mov [%8],ebx
  sub ebx,%6-1
  mov [%7],ebx
%%notoverflow
  mov ebx,[%9]
  sub ebx,%6
  cmp [%7],ebx
  jb %%notoverflow2
  mov [%7],ebx
%%notoverflow2
  ret
%%nobar
  ; upper arrow
  GUIClickArea %1,%2-7,%3,%2-1,%%noupper
  mov eax,[GUIwinposy+%10*4]
  add eax,%2-7
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%2-1
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%noupper
  ; lower arrow
  GUIClickArea %1,%4+1,%3,%4+7,%%nolower
  mov eax,[GUIwinposy+%10*4]
  add eax,%4+1
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%10*4]
  add eax,%4+7
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%10*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%nolower
%endmacro

%macro GUIPostWinBoxMove 11 ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  cmp eax,%1
  jl near %%noload
  cmp eax,%3
  jg near %%noload
  cmp edx,%2
  je near %%scrollup
  cmp edx,%4
  jne near %%noload
  ; Scroll Down
  cmp dword[%7],%11
  jbe %9
  mov ebx,[%5]
  add ebx,%11-1
  mov [%6],ebx
%9
  cmp dword[GUIScrolTim1],0
  jne near %%donescrol
  mov ebx,[%7]
  dec ebx
  cmp ebx,[%6]
  jbe %%donescrolldown
  inc dword[%6]
  cmp dword[%7],%11
  jbe near %%donescrol
  inc dword[%5]
  mov ebx,[%7]
  sub ebx,%11
  cmp ebx,[%5]
  ja near %%donescrolldown
  mov [%5],ebx
  jmp %%donescrolldown
%%scrollup
  mov ebx,[%5]
  mov [%6],ebx
%8
  cmp dword[GUIScrolTim1],0
  jne near %%donescrol
  cmp dword[%6],0
  je %%donescrolldown
  dec dword[%6]
  cmp dword[%5],0
  je %%donescrolldown
  dec dword[%5]
%%donescrolldown
  mov dword[GUIScrolTim1],1
  cmp dword[GUIScrolTim2],4
  jb %%noscrol1
  inc dword[GUIScrolTim1]
%%noscrol1
  cmp dword[GUIScrolTim2],0
  je %%noscrol2
  inc dword[GUIScrolTim1]
  dec dword[GUIScrolTim2]
%%noscrol2
%%donescrol
  jmp %10
%%noload
%endmacro

%macro GUIPHoldbutton 5
  GUIClickArea %1,%2,%3,%4,%%nohold
  mov dword[GUIHoldXlimL],%1
  mov dword[GUIHoldXlimR],%3
  mov dword[GUIHoldYlim],%2
  mov dword[GUIHoldYlimR],%4
  mov byte[GUICBHold2],%5
  mov byte[GUIHold],4
%%nohold
%endmacro

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

%macro GUIWinControl 13 ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIClickArea %1,%2,%3,%4,{near %%noload}
  cmp dword[%7],0
  je near %%noload
  mov dword[%5],%11
  mov eax,edx
  sub eax,%8
  xor edx,edx
  mov ebx,%9
  div ebx
  add eax,[%6]
  mov ebx,[%7]
  dec ebx
  cmp eax,ebx
  jg %%nomod
  mov [%10],eax
%%nomod
  cmp byte[GUIHold],0
  jne %%held
  cmp dword[GUIDClickTL],0
  je %%noprevhold
  cmp byte[GUIDClCWin],%11
  jne %%noprevhold
  cmp [GUIDClCEntry],eax
  jne %%noprevhold
  mov dword[GUIDClickTL],0
  mov eax,%12
  cmp eax,1
  jne %%cheat
  ccallv GUILoadData
  ret
%%cheat
  ccallv CheatCodeToggle
  ret
%%noprevhold
  mov dword[GUIDClickTL],%13
  mov byte[GUIDClCWin],%11
  mov [GUIDClCEntry],eax
%%held
  mov eax,[GUIwinposy+%12*4]
  add eax,%2-1
  mov [GUIHoldYlim],eax
  mov eax,[GUIwinposy+%12*4]
  add eax,%4+1
  mov [GUIHoldYlimR],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%1
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+%12*4]
  add eax,%3
  mov [GUIHoldXlimR],eax
  mov byte[GUIHold],3
  ret
%%noload
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

%macro GUIBoxVar 6
  GUIClickArea %1,%2,%3,%4,%%nope
  mov dword[%5],%6
%%nope
%endmacro

; Mouse implementation

SECTION .bss
NEWSYM GUINumValue, resd 1
NEWSYM GUIBlankVar, resd 1
SECTION .data
NEWSYM NTSCPresetVar, dd 4

SECTION .text

NEWSYM DisplayGUICheatSearchClick2
  cmp byte[CheatWinMode],2
  je near DisplayGUICheatSearchClick.preview
NEWSYM DisplayGUICheatSearchClick
  cmp byte[CheatWinMode],1
  je near .incheatmode
  cmp byte[CheatWinMode],2
  je near .view
  cmp byte[CheatWinMode],3
  je near .add

  GUIPHoldbutton 95,140,140,152,50         ; Main Menu
  GUIPButtonHole 11,28,byte[CheatSrcByteSize],0
  GUIPButtonHole 11,38,byte[CheatSrcByteSize],1
  GUIPButtonHole 11,48,byte[CheatSrcByteSize],2
  GUIPButtonHole 11,58,byte[CheatSrcByteSize],3
  GUIPButtonHole 11,73,byte[CheatSrcByteBase],0
  GUIPButtonHole 11,83,byte[CheatSrcByteBase],1
  GUIPButtonHole 11,113,byte[CheatSrcSearchType],0
  GUIPButtonHole 11,123,byte[CheatSrcSearchType],1
  ret

.incheatmode                            ; Exact Value
  GUIPHoldbutton 30,140,80,152,51
  cmp byte[CheatSearchStatus],1
  jne near .cheatsearching
  jmp .nosearch

.cheatsearching                         ; Return and Re-Search
  cmp byte[CheatSrcSearchType],1
  je near .cheatsearchingcomp
  jmp .searched

.cheatsearchingcomp                     ; Select Comparison
  GUIPButtonHole 11,33,byte[CheatCompareValue],0
  GUIPButtonHole 11,43,byte[CheatCompareValue],1
  GUIPButtonHole 11,53,byte[CheatCompareValue],2
  GUIPButtonHole 11,63,byte[CheatCompareValue],3

.searched                               ; Restart/View/Search Buttons
  GUIPHoldbutton 120,140,170,152,53
.nosearch
  GUIPHoldbutton 10,140,60,152,51
  GUIPHoldbutton 70,140,110,152,52
  ret

; View Cheat
.preview                     ; Preview Box
  ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  GUISlidebarPostImpl 173,28,180,100,.scrollup,.scrolldown,11,12,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,GUIBlankVar,1
  ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  GUIPostWinBoxMove 5,22,171,22+12*7,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,.scrollup,.scrolldown,DisplayGUICheatSearchClick.skipscrol,12
.view
  ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  GUISlidebarImpl 173,28,180,100,GUICSStA,12,GUIcurrentchtsrcviewloc,GUIcurrentchtsrccursloc,NumCheatSrc,13
.skipscrol
  ; x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIWinControl 5,23,171,21+12*7,GUIBlankVar,GUIcurrentchtsrcviewloc,NumCheatSrc,22,7,GUIcurrentchtsrccursloc,5,13,30

  GUIPHoldbutton 70,140,130,152,54         ; Return/Add Buttons
  GUIPHoldbutton 140,140,180,152,55
  ret

;    ccallv DrawSlideBarWin, 13, 173, 28, [GUIcurrentchtsrcviewloc], [NumCheatSrc], 12, 73, GUICSStA

.add                         ; Add Cheat
  GUIPHoldbutton 60,155,120,167,56                 ; Return/Add Buttons
  GUIPHoldbutton 130,155,160,167,57
  GUIClickCButton 8,139,byte[CheatUpperByteOnly]   ; Checkbox
  GUIBoxVar 10,30,80,37,CurCStextpos,0             ; Input Boxes
  GUIBoxVar 10,55,126,62,CurCStextpos,1
  ret

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
