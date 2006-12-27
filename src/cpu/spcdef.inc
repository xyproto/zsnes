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



; move al at address ebx

; branch instructions
%macro spcbrancher 0
      inc ebp
      ret
.branch
      movsx ebx,byte[ebp]
      inc ebp
      add ebp,ebx
      ret
%endmacro

; tcall instruction
%macro spctcall 1
      mov ebx,ebp
      sub ebx,SPCRAM
      mov eax,[spcS]
      mov [SPCRAM+eax],bh
      dec byte[spcS]
      mov eax,[spcS]
      mov [SPCRAM+eax],bl
      dec byte[spcS]
      mov bx,[spcextraram+%1]
      test byte[SPCRAM+0F1h],80h
      jnz %%finished
      mov bx,[SPCRAM+0FFC0h+%1]
%%finished
      add ebx,SPCRAM
      mov ebp,ebx
      ret
%endmacro

; SET1 instruction
%macro set1 1
      mov bl,[ebp]
      add ebx,[spcRamDP]
      inc ebp
      push ebx
      ReadByte
      pop ebx
      or al,%1
      WriteByte
      ret
%endmacro

; CLR1 instruction
%macro clr1 1
      mov bl,[ebp]
      add ebx,[spcRamDP]
      inc ebp
      push ebx
      ReadByte
      pop ebx
      and al,%1
      WriteByte
      ret
%endmacro

; BBS instruction
%macro bbs 1
      mov bl,[ebp]
      add ebx,[spcRamDP]
      ReadByte
      test al,%1
      jnz .dp0jump
      add ebp,2
      ret
.dp0jump
      movsx ebx,byte[ebp+1]
      add ebp,ebx
      add ebp,2
      ret
%endmacro

; BBC instruction
%macro bbc 1
      mov bl,[ebp]
      add ebx,[spcRamDP]
      ReadByte
      test al,%1
      jz .dp0jump
      add ebp,2
      ret
.dp0jump
      movsx ebx,byte[ebp+1]
      add ebp,ebx
      add ebp,2
      ret
%endmacro

; OR A, instruction
%macro SPC_OR_A 0
      or byte[spcA], al
      mov al,[spcA]
      mov [spcNZ],al
      ret
%endmacro

; AND A, instruction
%macro SPC_AND_A 0
      and byte[spcA], al
      mov al,[spcA]
      mov [spcNZ],al
      ret
%endmacro

; EOR A, instruction
%macro SPC_EOR_A 0
      xor byte[spcA], al
      mov al,[spcA]
      mov [spcNZ],al
      ret
%endmacro

; CMP A, instruction
%macro SPC_CMP_A 0
      cmp byte[spcA], al
      cmc
      SPCSetFlagnzc
%endmacro

%macro SPC_ADC_A 0
      mov cl,[spcP]
      shr cl,1
      adc byte[spcA], al
      SPCSetFlagnvhzc
%endmacro

%macro SPC_SBC_A 0
      mov cl,[spcP]
      xor cl,1
      shr cl,1
      sbb byte[spcA], al
      cmc
      SPCSetFlagnvhzc
%endmacro

%macro SPC_MOV_A 0
      mov [spcA], al
      mov [spcNZ],al
      ret
%endmacro
