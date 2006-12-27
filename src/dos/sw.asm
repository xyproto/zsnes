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

EXTSYM SidewinderFix

%macro ParityCheckSW 1
   mov ecx,ebx
   xor cl,ch
   jpe %%ParChkSW
   mov [%1],ebx
%%ParChkSW:
%endmacro

section .data
gDump times 256 db 0
bDump times 128 db 0
NEWSYM _SW1, dd 0
NEWSYM _SW2, dd 0
NEWSYM _SW3, dd 0
NEWSYM _SW4, dd 0
NEWSYM _SWCount, dd 0

section .text
NEWSYM _readSideWinder
   pushad

   mov ecx,200
   mov ebx,gDump

   cli
   cmp byte[SidewinderFix],0
   je .write
   out dx,al
.write
GetSWDataLoop:
   cmp byte[SidewinderFix],0
   jne .nowrite
   out dx,al
.nowrite
   nop
   nop
   nop
   nop
   nop
   nop
   in al,dx
   mov [ebx],al
   inc ebx
   dec ecx
   jnz GetSWDataLoop
   sti

   xor ebx,ebx
   xor ecx,ecx
   xor edi,edi
   mov esi,1

FindCycle:
   mov al,[gDump+edi]
   inc edi
   cmp edi,200
   je SMWError
   test al,00010000b
   jnz WMFCS1
   xor ecx,ecx
   jmp FindCycle
WMFCS1:
   inc ecx
   cmp ecx,15
   jne FindCycle

   xor ebp,ebp

FindStrobeLow:
   mov al,[gDump+edi]
   inc edi
   cmp edi,200
   je SMWError
   test al,00010000b
   jnz FindStrobeLow
   xor ecx,ecx

FindStrobeHigh:
   inc ecx
   cmp ecx,15
   je SWModeCheck
   mov al,[gDump+edi]
   inc edi
   cmp edi,200
   je SMWError
   test al,00010000b
   jz FindStrobeHigh

   mov [bDump+ebp],al
   inc ebp
   jmp FindStrobeLow

SMWDone:
   popad
   mov eax,0
   ret

SMWError:
   popad
   mov eax,1
   ret

SWModeCheck:
   cmp ebp,5
   je near ModeB1
   cmp ebp,15
   je near ModeA1
   cmp ebp,10
   je near ModeB2
   cmp ebp,30
   je near ModeA2
   cmp ebp,45
   je near ModeA3
   cmp ebp,20
   je near ModeB4
   cmp ebp,60
   je near ModeA4
   jmp short SMWError

ModeA1:
   cmp dword[_SWCount],3
   je near ModeB3
   xor ebp,ebp
   call DoModeA
   ParityCheckSW _SW1
   jmp SMWDone

ModeA4:
   mov ebp,45
   call DoModeA
   ParityCheckSW _SW4
ModeA3:
   mov ebp,30
   call DoModeA
   ParityCheckSW _SW3
ModeA2:
   mov ebp,15
   call DoModeA
   ParityCheckSW _SW2
   xor ebp,ebp
   call DoModeA
   ParityCheckSW _SW1
   jmp SMWDone

ModeB4:
   mov ebp,15
   call DoModeB
   ParityCheckSW _SW4
ModeB3:
   mov ebp,10
   call DoModeB
   ParityCheckSW _SW3
ModeB2:
   mov ebp,5
   call DoModeB
   ParityCheckSW _SW2
ModeB1:
   xor ebp,ebp
   call DoModeB
   ParityCheckSW _SW1
   jmp SMWDone

DoModeB:
   xor ebx,ebx
   mov eax,2
   mov ecx,5
   add ebp,bDump
ModeBLoop:
   test byte[ebp],00100000b
   jnz $+4
   or ebx,eax
   shl eax,1
   test byte[ebp],01000000b
   jnz $+4
   or ebx,eax
   shl eax,1
   test byte[ebp],10000000b
   jnz $+4
   or ebx,eax
   shl eax,1
   inc ebp
   dec ecx
   jnz ModeBLoop
   ret

DoModeA:
   xor ebx,ebx
   mov eax,2
   mov ecx,15
   add ebp,bDump
ModeALoop:
   test byte[ebp],00100000b
   jnz $+4
   or ebx,eax
   shl eax,1
   inc ebp
   dec ecx
   jnz ModeALoop
   ret


