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

%macro GUIClickCButtonf 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%ifdef __WIN32__
  ccallv %4
%endif
%%noclick
%endmacro

%macro GUIClickCButtonC 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  mov dword[GUIccombviewloc],0
  mov dword[GUIccombcursloc],0
  mov dword[GUIccomblcursloc],0
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

%macro GUIClickCButton5 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
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

%macro GUIClickCButtonI 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
%ifndef __MSDOS__
  push ebx
  xor ebx,ebx
  mov bl,[cvidmode]
  cmp byte[GUIBIFIL+ebx],0
  pop ebx
  je %%skipinit
%ifdef __WIN32__
  ccallv initDirectDraw
%elifdef __OPENGL__
  ccallv initwinvideo
%endif
%%skipinit
%endif
  ccallv Clear2xSaIBuffer
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

%macro GUIWinControl2 13 ;Key Combination Editor Exclusive
  GUIClickArea %1,%2,%3,%4,{near %%noload}
  cmp dword[%7],0
  je near %%noload
  mov dword[GUIccomblcursloc],0FFFFFFFFh
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

%macro DGOptnsProcBoxc 5
  GUIClickArea %1,%2,%3,%4,%%skip
  push edi
  ccall guipresstest
  mov ebx, eax
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %5,ebx
  ret
%%cancelkey
  cmp ebx,1
  jne %%noesc
  mov byte[pressed+1],2
%%noesc
  xor ebx,ebx
  mov %5,ebx
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

%macro ComboBoxProc 5
  GUIClickArea %1,%2,%3,%4,%%nope
  cmp byte[GUINumCombo],42
  je %%nope
  xor ebx,ebx
  mov bl,[GUINumCombo]
  mov byte[GUIComboData+ebx],%5
  inc byte[GUINumCombo]
%%nope
%endmacro

%macro ComboBoxProcD 4
  GUIClickArea %1,%2,%3,%4,%%nope
  cmp byte[GUINumCombo],0
  je %%nope
  dec byte[GUINumCombo]
%%nope
%endmacro

; Mouse implementation

SECTION .bss
NEWSYM GUINumValue, resd 1
NEWSYM GUIBlankVar, resd 1
SECTION .data
NEWSYM NTSCPresetVar, dd 4
NEWSYM OGLCheckVar, db 0

SECTION .text

NEWSYM DisplayGUISoundClick
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+6*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+6*4]

  GUIClickCButton 11,21,byte[SPCDisable]
  GUIClickCButtonf 11,31,byte[soundon],reInitSound
  GUIClickCButton 11,41,byte[StereoSound]
  GUIClickCButton 11,51,byte[RevStereo]
  GUIClickCButton 11,61,byte[Surround]
%ifdef __MSDOS__
  GUIClickCButton 11,71,byte[Force8b]
%endif
%ifdef __WIN32__
  GUIClickCButton 11,71,byte[PrimaryBuffer]
%endif

  GUIPButtonHole 11,157,byte[SoundInterpType],0
  GUIPButtonHole 11,167,byte[SoundInterpType],1
  GUIPButtonHole 11,177,byte[SoundInterpType],2
  cmp byte[MMXSupport],0
  je near .no8ptintrp
  GUIPButtonHole 11,187,byte[SoundInterpType],3
.no8ptintrp

  GUIPButtonHole 111,157,byte[LowPassFilterType],0
  GUIPButtonHole 111,167,byte[LowPassFilterType],1
  GUIPButtonHole 111,177,byte[LowPassFilterType],2
  cmp byte[MMXSupport],0
  je near .nohiqualityfilter
  GUIPButtonHole 111,187,byte[LowPassFilterType],3
.nohiqualityfilter

  GUIClickArea 15,101,69,109,.nosrate
  mov eax,[SoundQuality]
  mov al,[sampratenext+eax]
  mov [SoundQuality],al
.nosrate
  GUIClickArea 15,129,115,133,{near .nomovebar}
  sub eax,15
  mov [MusicRelVol],al
  mov byte[GUIHold],5
  mov eax,[GUIwinposy+6*4]
  add eax,131
  mov [GUIHoldYlim],eax
  xor eax,eax
  xor edx,edx
  mov al,[MusicRelVol]
  shl eax,7
  mov ebx,100
  div ebx
  cmp al,127
  jb .noof
  mov al,127
.noof
  mov [MusicVol],al
  mov al,[DSPMem+0Ch]
  call WDSPReg0C
  mov al,[DSPMem+1Ch]
  call WDSPReg1C

  mov eax,[GUIwinposx+6*4]
  add eax,15
  mov [GUIHoldXlimL],eax
  add eax,100
  mov [GUIHoldXlimR],eax
.nomovebar
  ret

SECTION .data
sampratenext db 1,4,5,6,2,3,0,0
SECTION .text

NEWSYM DisplayGUICheatClick2
  GUISlidebarPostImpl 231,28,238,100,.scrollup,.scrolldown,7,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,GUIBlankVar,1
  GUIPostWinBoxMove 5,22,229,22+12*7,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,.scrollup,.scrolldown,DisplayGUICheatClick.skipscrol,12

NEWSYM DisplayGUICheatClick
  GUIBoxVar 5,20,181,110,GUIcurrentcheatwin,0      ; Main Box
  GUIBoxVar 82,129,173,136,GUIcurrentcheatwin,1    ; Enter Code Box
  GUIBoxVar 82,140,196,147,GUIcurrentcheatwin,2    ; Description Box
  ; SlideBar Implementation
  GUISlidebarImpl 231,28,238,100,GUICStA,12,GUIcurrentcheatviewloc,GUIcurrentcheatcursloc,NumCheats,7
.skipscrol
  cmp byte[GUIcurrentcheatwin],0
  jne near .nowin
  GUIWinControl 5,23,229,21+12*7,GUIBlankVar,GUIcurrentcheatviewloc,NumCheats,22,7,GUIcurrentcheatcursloc,3,7,30
.nowin
  GUIPHoldbutton 5,113,47,124,5                  ; Buttons
  GUIPHoldbutton 52,113,94,124,6
  GUIPHoldbutton 99,113,141,124,7
  GUIPHoldbutton 146,113,188,124,8
  GUIPHoldbutton 212,134,236,145,9
  GUIPHoldbutton 193,113,235,124,33
  GUIClickCButton5 11,186,byte[AutoLoadCht],1    ; Checkbox
  ret

NEWSYM DisplayNetOptnsClick
  ret

NEWSYM DisplayGameOptnsClick
  mov dword[keycontrolval],0                  ; Shortcut Boxes
  DGOptnsProcBox 27,23,[KeyBGDisble0]
  DGOptnsProcBox 27+45,23,[KeyBGDisble1]
  DGOptnsProcBox 27+45*2,23,[KeyBGDisble2]
  DGOptnsProcBox 27+45*3,23,[KeyBGDisble3]
  DGOptnsProcBox 27+45*4,23,[KeySprDisble]
  DGOptnsProcBox 27,41,[KeyDisableSC0]
  DGOptnsProcBox 27+45,41,[KeyDisableSC1]
  DGOptnsProcBox 27+45*2,41,[KeyDisableSC2]
  DGOptnsProcBox 27+45*3,41,[KeyDisableSC3]
  DGOptnsProcBox 34+45*4,41,[KeyVolUp]
  DGOptnsProcBox 27,50,[KeyDisableSC4]
  DGOptnsProcBox 27+45,50,[KeyDisableSC5]
  DGOptnsProcBox 27+45*2,50,[KeyDisableSC6]
  DGOptnsProcBox 27+45*3,50,[KeyDisableSC7]
  DGOptnsProcBox 34+45*4,50,[KeyVolDown]

  DGOptnsProcBox 8+25,70,[KeyQuickLoad]
  DGOptnsProcBox 8+57+26,70,[KeyQuickRst]
  DGOptnsProcBox 8+114+20,70,[KeyQuickExit]
  DGOptnsProcBox 8+114+78,70,[KeyQuickClock]
  DGOptnsProcBox 8+25,80,[KeyQuickChat]
  DGOptnsProcBox 8+57+45,80,[KeyQuickSnapShot]
  DGOptnsProcBox 8+108+70,80,[KeyQuickSaveSPC]

  DGOptnsProcBox 78,100,[KeyUsePlayer1234]
  DGOptnsProcBox 78,110,[KeyResetAll]
  DGOptnsProcBox 78,120,[KeyDisplayFPS]
%ifndef __MSDOS__
  DGOptnsProcBox 78,130,[KeyDisplayBatt]
%endif

  DGOptnsProcBox 191,100,[KeyNewGfxSwt]
  DGOptnsProcBox 191,110,[KeyWinDisble]
  DGOptnsProcBox 191,120,[KeyOffsetMSw]
  DGOptnsProcBox 191,130,[KeyIncreaseGamma]
  DGOptnsProcBox 191,140,[KeyDecreaseGamma]
  ret

NEWSYM DisplayGUIOptnsClick
  mov eax,[GUImouseposx]
  sub eax,[GUIwinposx+10*4]
  mov edx,[GUImouseposy]
  sub edx,[GUIwinposy+10*4]
  GUIClickCButton 12,23,byte[GUIRClick]      ; Checkboxes
  GUIClickCButton 12,33,byte[lhguimouse]
  GUIClickCButton 12,43,byte[mouseshad]
  GUIClickCButton 12,53,byte[mousewrap]

  GUIClickCButton 129,23,byte[esctomenu]
  GUIClickCButton 129,33,byte[JoyPad1Move]
  GUIClickCButtonI 129,43,byte[FilteredGUI]
  GUIClickCButton 129,53,byte[newfont]
  GUIClickCButton 129,63,byte[savewinpos]

%ifdef __WIN32__
  GUIClickCButton 12,63,byte[TrapMouseCursor]
  GUIClickCButton 12,73,byte[MouseWheel]
  GUIClickCButton 12,168,byte[AlwaysOnTop]
  GUIClickCButton 12,178,byte[SaveMainWindowPos]
  GUIClickCButton 12,188,byte[AllowMultipleInst]
%endif

  GUIPButtonHole 72,88,byte[GUIEffect],0    ; Radio Buttons
  GUIPButtonHole 122,88,byte[GUIEffect],1
  GUIPButtonHole 182,88,byte[GUIEffect],4
  GUIPButtonHole 72,98,byte[GUIEffect],5
  GUIPButtonHole 122,98,byte[GUIEffect],2
  GUIPButtonHole 182,98,byte[GUIEffect],3

  GUIPButtonHole 48,108,byte[CurPalSelect],0
  GUIPButtonHole 88,108,byte[CurPalSelect],1
  GUIPButtonHole 133,108,byte[CurPalSelect],2

  push eax                          ; Sliders
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

  cmp eax,25
  jl near .nomovebar
  cmp eax,25+127
  jg near .nomovebar
  sub eax,25
  shr eax,2
  cmp edx,122
  jl .nomovebar1
  cmp edx,126
  jg .nomovebar1
  cmp byte[TRVal2],al
  je .nomovebar1b
  mov [TRVal2],al
  mov byte[TRVal2+1],1
.nomovebar1b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,124
  mov [GUIHoldYlim],eax
  jmp .nomovebar
.nomovebar1
  cmp edx,134
  jl .nomovebar2
  cmp edx,138
  jg .nomovebar2
  cmp byte[TGVal2],al
  je .nomovebar2b
  mov [TGVal2],al
  mov byte[TRVal2+1],1
.nomovebar2b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,136
  mov [GUIHoldYlim],eax
  jmp .nomovebar
.nomovebar2
  cmp edx,146
  jl .nomovebar3
  cmp edx,150
  jg .nomovebar3
  cmp byte[TBVal2],al
  je .nomovebar3b
  mov [TBVal2],al
  mov byte[TRVal2+1],1
.nomovebar3b
  mov byte[GUIHold],2
  mov eax,[GUIwinposy+10*4]
  add eax,148
  mov [GUIHoldYlim],eax
.nomovebar3
.nomovebar

  push eax
  cmp byte[CurPalSelect],1
  je .yespal2
  cmp byte[CurPalSelect],2
  je .yespal3
  mov al,[TRVal2]
  mov [GUIRAdd],al
  mov al,[TGVal2]
  mov [GUIGAdd],al
  mov al,[TBVal2]
  mov [GUIBAdd],al
  jmp .nopal1
.yespal2
  mov al,[TRVal2]
  mov [GUITRAdd],al
  mov al,[TGVal2]
  mov [GUITGAdd],al
  mov al,[TBVal2]
  mov [GUITBAdd],al
  jmp .nopal1
.yespal3
  mov al,[TRVal2]
  mov [GUIWRAdd],al
  mov al,[TGVal2]
  mov [GUIWGAdd],al
  mov al,[TBVal2]
  mov [GUIWBAdd],al
.nopal1
  pop eax

  cmp byte[TRVal2+1],1
  jne .nopalchange
  ccallv GUISetPal
  mov byte[TRVal2+1],0
.nopalchange

  cmp byte[GUIHold],2
  jne .nohold
  mov eax,[GUIwinposx+10*4]
  add eax,25
  mov [GUIHoldXlimL],eax
  mov eax,[GUIwinposx+10*4]
  add eax,25+127
  mov [GUIHoldXlimR],eax
.nohold
  ret

NEWSYM DisplayGUIAboutClick
%ifndef __MSDOS__
  cmp byte[EEgg],1
  je near .eegg
  GUIPHoldbutton 90,22,175,32,65
  GUIPHoldbutton 90,33,175,43,66
.eegg
%endif
  ret

; DrawSlideBar 16,192,20,[GUIccombviewloc],[NumCombo],8,61,GUICSStC,13,14
;    DrawBorderedBox 16,10,20,190,80,19

;    ccallv DrawSlideBarWin, 3, 109, 42, [GUIcurrentinputviewloc], 16, 5, 28, GUIIStA

NEWSYM DisplayGUIComboClick2
  mov dword[keycontrolval],0
  ; x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
  GUISlidebarPostImpl 192,28,199,72,.scrollup,.scrolldown,13,8,GUIccombviewloc,GUIccombcursloc,NumCombo,GUIBlankVar,1
  ;x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
  GUIPostWinBoxMove 10,22,190,23+8*7,GUIccombviewloc,GUIccombcursloc,NumCombo,.scrollup,.scrolldown,DisplayGUIComboClick.skipscrol,8
NEWSYM DisplayGUIComboClick
  ; SlideBar Implementation
  ; x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
  GUISlidebarImpl 192,28,199,72,GUICSStC,8,GUIccombviewloc,GUIccombviewloc,NumCombo,16
.skipscrol
  ;x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
  GUIWinControl2 10,23,190,22+8*7,GUIBlankVar,GUIccombviewloc,NumCombo,23,7,GUIccombcursloc,6,16,0

  GUIPButtonHole 158,82,byte[GUIComboPNum],0
  GUIPButtonHole 176,82,byte[GUIComboPNum],1
  GUIPButtonHole 194,82,byte[GUIComboPNum],2
  GUIPButtonHole 212,82,byte[GUIComboPNum],3
  GUIPButtonHole 230,82,byte[GUIComboPNum],4
  GUIClickCButton 163,88,byte[GUIComboLHorz]
  GUIClickCButtonC 163,96,byte[GUIComboGameSpec]
  GUIPHoldbutton 202,20,246,31,60
  GUIPHoldbutton 202,35,246,46,61
  GUIPHoldbutton 202,50,246,61,62
  GUIPHoldbutton 202,65,246,76,63
  DGOptnsProcBoxc 10,91,32,99,[GUIComboKey]

  ComboBoxProc 75,150,85,157,1
  ComboBoxProc 89,150,99,157,2
  ComboBoxProc 103,150,113,157,3
  ComboBoxProc 117,150,127,157,4
  ComboBoxProc 131,150,141,157,5
  ComboBoxProc 145,150,155,157,6
  ComboBoxProc 159,150,169,157,7
  ComboBoxProc 173,150,183,157,8
  ComboBoxProc 187,150,197,157,9
  ComboBoxProc 201,150,211,157,10
  ComboBoxProc 215,150,227,157,11
  ComboBoxProc 231,150,243,157,12
  ComboBoxProc 75,160,85,167,13
  ComboBoxProc 89,160,99,167,14
  ComboBoxProc 103,160,113,167,15
  ComboBoxProc 117,160,127,167,16
  ComboBoxProc 131,160,141,167,17
  ComboBoxProc 145,160,155,167,18
  ComboBoxProc 159,160,169,167,19
  ComboBoxProc 173,160,183,167,20
  ComboBoxProc 187,160,197,167,21
  ComboBoxProc 201,160,211,167,22
  ComboBoxProc 215,160,227,167,23
  ComboBoxProc 231,160,243,167,24
  ComboBoxProc 75,170,85,177,25
  ComboBoxProc 89,170,99,177,26
  ComboBoxProc 103,170,113,177,27
  ComboBoxProc 117,170,127,177,28
  ComboBoxProc 131,170,141,177,29
  ComboBoxProc 145,170,155,177,30
  ComboBoxProc 159,170,169,177,31
  ComboBoxProc 173,170,183,177,32
  ComboBoxProc 187,170,197,177,33
  ComboBoxProc 201,170,211,177,34
  ComboBoxProc 215,170,227,177,35
  ComboBoxProc 231,170,243,177,36

  ComboBoxProc 10,189,20,196,37
  ComboBoxProc 24,189,34,196,38
  ComboBoxProc 38,189,48,196,39
  ComboBoxProc 52,189,62,196,40
  ComboBoxProc 66,189,76,196,41
  ComboBoxProc 80,189,90,196,42
  ComboBoxProc 107,189,117,196,43
  ComboBoxProc 121,189,131,196,44
  ComboBoxProc 135,189,145,196,45
  ComboBoxProc 149,189,159,196,46
  ComboBoxProc 163,189,173,196,47
  ComboBoxProc 177,189,187,196,48
  ComboBoxProcD 204,189,218,196
  mov ebx,[NumComboLocl]
  cmp byte[GUIComboGameSpec],0
  jne .local
  mov ebx,[NumComboGlob]
.local
  mov [NumCombo],ebx
  xor ebx,ebx
  ret

NEWSYM DisplayGUIResetClick
  GUIPHoldbutton 20,30,56,41,2
  GUIPHoldbutton 80,30,116,41,3
  ret

NEWSYM DisplayGUIStatesClick
  GUIPHoldbutton 20,30,56,41,10
  GUIPHoldbutton 80,30,116,41,11
  ret

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
