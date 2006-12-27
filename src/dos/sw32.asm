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








;32-bit DOS-Mode driver for the Microsoft Sidewinder Gamepad
;Multi-SW Version 1.5
;(C) 1997, 1998 Robert William Grubbs, All Rights Reserved
;Latest revision 1/20/98
;  Driver Source code Include file

;C-linkable, rewrote decoder -sardu

;Flat memory mode (Protected mode extender required!  Tested with DOS32)
;Tested with TASM 4.0+

SECTION .data
SW1 dd 0                ;SW #1's button status
SW2 dd 0                ;SW #2's button status
SW3 dd 0                ;SW #3's button status
SW4 dd 0                ;SW #4's button status
SWCount dd 1            ;Tell the driver how many sidewinders are present
SWSetup dd 0            ;Tell the driver what polling mode to use
                        ;  0=Interrupts disabled, Multiple OUT statements
                        ;  1=Interrupts disabled, Single OUT statement
                        ;  2=Interrupts enabled, Multiple OUT statements
                        ;  3=Interrupts enabled, Single OUT statement

gDump times 100h db 0   ;SW Status dump buffer (space for 256 bytes, uses 200)
bDump times 80h db 0    ;buffer to hold button data (Modes A and B, all SW)
SECTION .text

;This macro calculates parity for the buttons and compares it to the SW's
; parity bit.  If they don't match, the button data is discarded.
%macro ParityCheckSW 1
  mov ecx,ebx                    ;duplicate button status
  xor cl,ch                      ;
  jpe %%ParChkSW
  mov [%1],ebx              ;update button status for SW #n
%%ParChkSW                      ;done
%endmacro

;The main subroutine;  this is the important one;  bow down before it
;IN: None
;Out: SWx=buttons (bit 0=null 1=up 2=dn 3=rt 4=lt 5=A 6=B 7=C 8=X)
;                 (9=Y 10=Z 11=L 12=R 13=St 14=M 15=Parity)
;No registers destroyed

readSideWinder:
  pushad

  mov ecx,200                 ;dump buffer fill size
  mov ebx,gDump               ;initial dump pointer
  mov edx,0201h               ;joystick port

  cmp dword[SWSetup],0
  jne NotSW0
  cli                         ;Disable interrupts (required to avoid jitter)
  GetSWDataLoop:              ;
  out dx,al                   ;trigger joystick port
  in al,dx                    ;read SW status byte
  mov [ebx],al                ;dump status byte
  inc ebx                     ;increment dump pointer
  dec ecx
  jnz GetSWDataLoop
  sti                         ;Re-enable interrupts
  jmp SWPollDone

  NotSW0:
  cmp dword[SWSetup],1
  jne NotSW1
  cli                         ;Disable interrupts (required to avoid jitter)
  out dx,al                   ;trigger joystick port
  GetSWDataLoop1:             ;
  in al,dx                    ;read SW status byte
  mov [ebx],al                ;dump status byte
  inc ebx                     ;increment dump pointer
  dec ecx
  jnz GetSWDataLoop1
  sti                         ;Re-enable interrupts
  jmp SWPollDone

  NotSW1:
  cmp dword[SWSetup],2
  jne NotSW2
  GetSWDataLoop2:             ;
  out dx,al                   ;trigger joystick port
  in al,dx                    ;read SW status byte
  mov [ebx],al                ;dump status byte
  inc ebx                     ;increment dump pointer
  dec ecx
  jnz GetSWDataLoop2
  jmp SWPollDone

  NotSW2:
  ;default all others to SWStatus=3
  out dx,al                   ;trigger joystick port
  GetSWDataLoop3:             ;
  in al,dx                    ;read SW status byte
  mov [ebx],al                ;dump status byte
  inc ebx                     ;increment dump pointer
  dec ecx
  jnz GetSWDataLoop3

  SWPollDone:

  mov ecx,0                   ;tick count
  mov esi,1                   ;initialize output mask
  mov ebx,0                   ;initialize output
  mov edi,0                   ;initialize input pointer

  ;My current method of cycle detection is to look for 15 highs in a row on
  ; the strobe line.  Cycle ends is detected by 15 lows in a row.
  ;Mode A has 15 strobes in a cycle, Mode B has 5.
  ; Note that the 15 highs/lows for cycle detection may be too high for slow
  ; machines.  I havn't seen a problem yet, but it may exist...
  ;Multiple Sidewinder data complicates things.  Each additional SW tags
  ; another set of strobes to the cycle, 5 more in mode B, 15 more in mode A.
  ; Detecting extra SW gamepad data is fairly simple: count the number of
  ; strobes. If it is a multiple of 5, you're in mode B and can divide by 5
  ; to get the total number of gamepads.  If it's divisible by 15, use mode A.
  ; However, this method cannot distinguish between mode A for one SW and mode
  ; B for three SW.  In that case, the SWCount variable must be set correctly.

  FindCycle:
  mov al,[gDump+edi]          ;get next status byte
  inc edi                     ;increment input pointer
  cmp edi,200                 ;test for end of status block
  je SWNoFind                 ;if it's the end, quit sub with error
  test al,00010000b           ;Check for nonzero bits
  jnz WMFCS1                  ;
  xor ecx,ecx                 ;if zero, reset tick count
  jmp FindCycle               ;can't be pre-cycle
  WMFCS1:                     ;Possibly pre-cycle
  inc ecx                     ;increment tick count
  cmp ecx,15                  ;test for sufficient ticks for cycle start
  jne FindCycle               ;if insufficient, get next status byte
                              ;Yippie! it found a (probable) cycle!

  mov ebp,0                   ;initialize bDump index (strobe count)

  FindStrobeLow:              ;Search for leading edge of data strobe
  mov al,[gDump+edi]          ;get next status byte
  inc edi                     ;increment input pointer
  cmp edi,200                 ;test for end of status block
  je SWNoFind                 ;if it's the end, quit sub with error
  test al,00010000b           ;get "strobe" bit
  jnz SHORT FindStrobeLow     ;if it isn't zero, we're not there yet
  xor ecx,ecx                 ;initialize cycle end test count

  FindStrobeHigh:
  inc ecx                     ;increment zero count
  cmp ecx,0fh                 ;is it 15?
  je SWModeCheck              ;if so, goto mode check
  mov al,[gDump+edi]          ;get next status byte
  inc edi                     ;increment input pointer
  cmp edi,200                 ;test for end of status block
  je SWNoFind                 ;if it's the end, quit sub with error
  test al,00010000b           ;get "strobe" bit
  jz FindStrobeHigh           ;if it is zero, we're not there yet
                              ;if not, we're there!  data bit is valid (probably)
  mov [bDump+ebp],al          ;preserve data for button decoding
  inc ebp                     ;increment strobe count/bDump index
  jmp FindStrobeLow           ;wait for the next button

 SMWDone:
 SWNoFind:
  popad
  ret                         ;return to calling procedure

 SWModeCheck:                ;Check strobe count to identify mode and # of SW
  cmp ebp,15                  ;Is it Mode A with 1 Sidewinder or B with 3?
  je ModeA1
  cmp ebp,5                   ;Is it Mode B with 1 Sidewinders?
  je ModeB1
  cmp ebp,30                  ;Is it Mode A with 2 Sidewinders?
  je ModeA2
  cmp ebp,10                  ;Is it Mode B with 2 Sidewinders?
  je near ModeB2
  cmp ebp,45                  ;Is it Mode A with 3 Sidewinders?
  je near ModeA3
  cmp ebp,60                  ;Is it Mode A with 4 Sidewinders?
  je near ModeA4
  cmp ebp,20                  ;Is it Mode B with 4 Sidewinders?
  je near ModeB4
  jmp SHORT SWNoFind          ;Any other # of strobes is invalid data

  ModeB1:
  xor ebp,ebp
  call DoModeB
  ParityCheckSW SW1
  jmp SMWDone

  ModeA1:
  cmp dword[SWCount],3
  je near ModeB3
  xor ebp,ebp
  call DoModeA
  ParityCheckSW SW1
  jmp SMWDone

  ModeA2:
  xor ebp,ebp
  call DoModeA
  ParityCheckSW SW1
  mov ebp,15
  call DoModeA
  ParityCheckSW SW2
  jmp SMWDone

  ModeA3:
  xor ebp,ebp
  call DoModeA
  ParityCheckSW SW1
  mov ebp,15
  call DoModeA
  ParityCheckSW SW2
  mov ebp,30
  call DoModeA
  ParityCheckSW SW3
  jmp SMWDone

  ModeA4:
  xor ebp,ebp
  call DoModeA
  ParityCheckSW SW1
  mov ebp,15
  call DoModeA
  ParityCheckSW SW2
  mov ebp,30
  call DoModeA
  ParityCheckSW SW3
  mov ebp,45
  call DoModeA
  ParityCheckSW SW4
  jmp SMWDone

  ModeB2:
  xor ebp,ebp
  call DoModeB
  ParityCheckSW SW1
  mov ebp,5
  call DoModeB
  ParityCheckSW SW2
  jmp SMWDone

  ModeB3:
  xor ebp,ebp
  call DoModeB
  ParityCheckSW SW1
  mov ebp,5
  call DoModeB
  ParityCheckSW SW2
  mov ebp,10
  call DoModeB
  ParityCheckSW SW3
  jmp SMWDone

  ModeB4:
  xor ebp,ebp
  call DoModeB
  ParityCheckSW SW1
  mov ebp,5
  call DoModeB
  ParityCheckSW SW2
  mov ebp,10
  call DoModeB
  ParityCheckSW SW3
  mov ebp,15
  call DoModeB
  ParityCheckSW SW4
  jmp SMWDone
ENDP

%macro SWRepeat 1
  mov al,[bDump+ebp+%1]
  shr al,5       ;get upper 3 bits
  shl eax,1+3*%1  ;shift into place
  or  ebx,eax    ;or into output
%endmacro

DoModeB:
  xor ebx,ebx                 ;Initialize output
  xor eax,eax

  SWRepeat 0
  SWRepeat 1
  SWRepeat 2
  SWRepeat 3
  SWRepeat 4

  xor ebx,0FFFEh
  ret

DoModeA:
  xor ebx,ebx                 ;Clear output
  mov ecx,15 ;bit count

ALP:
  mov al,[bDump+ebp]
  inc ebp
  shl al,3
  rcr ebx,1
  dec ecx
  jg  ALP

  shr ebx,16
  xor ebx,0FFFEh
  ret




