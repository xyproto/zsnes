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

%include "macros.mac"


;/*---------------------------------------------------------------------*
; * The following (piece of) code, (part of) the 2xSaI engine,          *
; * copyright (c) 1999 by Derek Liauw Kie Fa.                           *
; * Non-Commercial use of the engine is allowed and is encouraged,      *
; * provided that appropriate credit be given and that this copyright   *
; * notice will not be removed under any circumstance.                  *
; * You may freely modify this code, but I request                      *
; * that any improvements to the engine be submitted to me, so          *
; * that I can implement these improvements in newer versions of        *
; * the engine.                                                         *
; * If you need more information, have any comments or suggestions,     *
; * you can e-mail me. My e-mail: derek-liauw@usa.net.                  *
; *---------------------------------------------------------------------*/

;----------------------
; 2xSaI version 0.59 WIP, soon to become version 0.60
;----------------------

	  BITS 32

	  SECTION .text ALIGN = 32

srcPtr        equ 8
deltaPtr      equ 12
srcPitch      equ 16
width         equ 20
dstOffset     equ 24
dstPitch      equ 28
dstSegment    equ 32


colorB0   equ -2
colorB1   equ 0
colorB2   equ 2
colorB3   equ 4

color7   equ -2
color8   equ 0
color9   equ 2

color4   equ -2
color5   equ 0
color6   equ 2
colorS2   equ 4

color1   equ -2
color2   equ 0
color3   equ 2
colorS1   equ 4

colorA0   equ -2
colorA1   equ 0
colorA2   equ 2
colorA3   equ 4



NEWSYM _2xSaISuperEagleLineW

; Store some stuff
	 push ebp
	 mov ebp, esp
         pushad

; Prepare the destination
%ifdef __DJGPP__
         ; Set the selector
         mov eax, [ebp+dstSegment]
         mov fs, ax
%endif
         mov edx, [ebp+dstOffset]         ; edx points to the screen
; Prepare the source
         ; eax points to colorA
         mov eax, [ebp+srcPtr]
         mov ebx, [ebp+srcPitch]
         mov ecx, [ebp+width]
         ; eax now points to colorB1
         sub eax, ebx

; Main Loop
.Loop:   push ecx

         ;-----Check Delta------------------
         mov ecx, [ebp+deltaPtr]

         movq mm0, [eax+colorB0]
         movq mm1, [eax+colorB3]
         movq mm2, [eax+ebx+color4]
         movq mm3, [eax+ebx+colorS2]
         movq mm4, [eax+ebx+ebx+color1]
         movq mm5, [eax+ebx+ebx+colorS1]
         push eax
         add eax, ebx
         movq mm6, [eax+ebx+ebx+colorA0]
         movq mm7, [eax+ebx+ebx+colorA3]
         pop eax

         pcmpeqw mm0, [ecx+2+colorB0]
         pcmpeqw mm1, [ecx+2+colorB3]
         pcmpeqw mm2, [ecx+ebx+2+color4]
         pcmpeqw mm3, [ecx+ebx+2+colorS2]
         pcmpeqw mm4, [ecx+ebx+ebx+2+color1]
         pcmpeqw mm5, [ecx+ebx+ebx+2+colorS1]
         add ecx, ebx
         pcmpeqw mm6, [ecx+ebx+ebx+2+colorA0]
         pcmpeqw mm7, [ecx+ebx+ebx+2+colorA3]
         sub ecx, ebx


         pand mm0, mm1
         pand mm2, mm3
         pand mm4, mm5
         pand mm6, mm7
         pand mm0, mm2
         pand mm4, mm6
         pxor mm7, mm7
         pand mm0, mm4
         movq mm6, [eax+colorB0]
         pcmpeqw mm7, mm0

         movq [ecx+2+colorB0], mm6

         packsswb mm7, mm7
         movd ecx, mm7
         test ecx, ecx
         jz near .SKIP_PROCESS

         ;End Delta

         ;---------------------------------
         movq mm0, [eax+ebx+color5]
         movq mm1, [eax+ebx+color6]
         movq mm2, mm0
         movq mm3, mm1
         movq mm4, mm0
         movq mm5, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3                ;mm0 contains the interpolated values
         movq [I56Pixel], mm0
         movq mm7, mm0

         ;-------------------
         movq mm0, mm7
         movq mm1, mm4  ;5,5,5,6
         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3                ;mm0 contains the interpolated values
         movq [product1a], mm0
         ;--------------------

         movq mm0, mm7
         movq mm1, mm5  ;6,6,6,5
         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3
         movq [product1b], mm0

         ;-------------------------
         ;-------------------------
         movq mm0, [eax+ebx+ebx+color2]
         movq mm1, [eax+ebx+ebx+color3]
         movq mm2, mm0
         movq mm3, mm1
         movq mm4, mm0
         movq mm5, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3
         movq [I23Pixel], mm0
         movq mm7, mm0

         ;---------------------
         movq mm0, mm7
         movq mm1, mm4  ;2,2,2,3
         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3
         movq [product2a], mm0

         ;----------------------
         movq mm0, mm7
         movq mm1, mm5  ;3,3,3,2
         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3
         movq [product2b], mm0


         ;////////////////////////////////
         ; Decide which "branch" to take
         ;--------------------------------
         movq mm4, [eax+ebx+color5]
         movq mm5, [eax+ebx+color6]
         movq mm6, [eax+ebx+ebx+color3]
         movq mm7, [eax+ebx+ebx+color2]

         pxor mm3, mm3
         movq mm0, mm4
         movq mm1, mm5

         pcmpeqw mm0, mm6
         pcmpeqw mm1, mm7
         pcmpeqw mm1, mm3
         pand mm0, mm1
         movq [Mask35], mm0

         movq mm0, [eax+ebx+ebx+colorS1]
         movq mm1, [eax+ebx+color4]
         push eax
         add eax, ebx
         movq mm2, [eax+ebx+ebx+colorA2]
         pop eax
         movq mm3, [eax+colorB1]
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm4
         pcmpeqw mm3, mm4
         pand mm0, mm1
         pand mm2, mm3
         por mm0, mm2
         pand mm0, [Mask35]
         movq [Mask35b], mm0

         ;-----------
         pxor mm3, mm3
         movq mm0, mm4
         movq mm1, mm5

         pcmpeqw mm0, mm6
         pcmpeqw mm1, mm7
         pcmpeqw mm0, mm3
         pand mm0, mm1
         movq [Mask26], mm0

         movq mm0, [eax+ebx+ebx+color1]
         movq mm1, [eax+ebx+colorS2]
         push eax
         add eax, ebx
         movq mm2, [eax+ebx+ebx+colorA1]
         pop eax
         movq mm3, [eax+colorB2]
         pcmpeqw mm0, mm5
         pcmpeqw mm1, mm5
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm1
         pand mm2, mm3
         por mm0, mm2
         pand mm0, [Mask26]
         movq [Mask26b], mm0

         ;--------------------
         movq mm0, mm4
         movq mm1, mm5
         movq mm2, mm0

         pcmpeqw mm2, mm1
         pcmpeqw mm0, mm6
         pcmpeqw mm1, mm7
         pand mm0, mm1
         pand mm2, mm0
         pxor mm0, mm2
         movq mm7, mm0

         ;------------------
         packsswb mm7, mm7
         movd ecx, mm7
         test ecx, ecx
         jz near .SKIP_GUESS

;---------------------------------------------
; Map of the pixels:                    I|E F|J
;                                       G|A B|K
;                                       H|C D|L
;                                       M|N O|P
         movq mm6, mm0
         movq mm4, [eax+ebx+color5]
         movq mm5, [eax+ebx+color6]
         pxor mm7, mm7
         pand mm6, [ONE]

         movq mm0, [eax+colorB1]
         movq mm1, [eax+ebx+color4]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         movq mm0, [eax+colorB2]
         movq mm1, [eax+ebx+colorS2]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         push eax
         add eax, ebx
         movq mm0, [eax+ebx+color1]
         movq mm1, [eax+ebx+ebx+colorA1]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         movq mm0, [eax+ebx+colorS1]
         movq mm1, [eax+ebx+ebx+colorA2]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         pop eax
         movq mm1, mm7
         pxor mm0, mm0
         pcmpgtw mm7, mm0
         pcmpgtw mm0, mm1

         por mm7, [Mask35]
         por mm1, [Mask26]
         movq [Mask35], mm7
         movq [Mask26], mm1

.SKIP_GUESS:
         ;Start the ASSEMBLY !!!

         movq mm4, [Mask35]
         movq mm5, [Mask26]
         movq mm6, [Mask35b]
         movq mm7, [Mask26b]

         movq mm0, [eax+ebx+color5]
         movq mm1, [eax+ebx+color6]
         movq mm2, [eax+ebx+ebx+color2]
         movq mm3, [eax+ebx+ebx+color3]
         pcmpeqw mm0, mm2
         pcmpeqw mm1, mm3
         movq mm2, mm4
         movq mm3, mm5
         por mm0, mm1
         por mm2, mm3
         pand mm2, mm0
         pxor mm0, mm2
         movq mm3, mm0

         movq mm2, mm0
         pxor mm0, mm0
         por mm2, mm4
         pxor mm4, mm6
         por mm2, mm5
         pxor mm5, mm7
         pcmpeqw mm2, mm0
         ;----------------

         movq mm0, [eax+ebx+color5]
         movq mm1, mm3
         por mm1, mm4
         por mm1, mm6
         pand mm0, mm1
         movq mm1, mm5
         pand mm1, [I56Pixel]
         por mm0, mm1
         movq mm1, mm7
         pand mm1, [product1b]
         por mm0, mm1
         movq mm1, mm2
         pand mm1, [product1a]
         por mm0, mm1
         movq [final1a], mm0

         movq mm0, [eax+ebx+color6]
         movq mm1, mm3
         por mm1, mm5
         por mm1, mm7
         pand mm0, mm1
         movq mm1, mm4
         pand mm1, [I56Pixel]
         por mm0, mm1
         movq mm1, mm6
         pand mm1, [product1a]
         por mm0, mm1
         movq mm1, mm2
         pand mm1, [product1b]
         por mm0, mm1
         movq [final1b], mm0

         movq mm0, [eax+ebx+ebx+color2]
         movq mm1, mm3
         por mm1, mm5
         por mm1, mm7
         pand mm0, mm1
         movq mm1, mm4
         pand mm1, [I23Pixel]
         por mm0, mm1
         movq mm1, mm6
         pand mm1, [product2b]
         por mm0, mm1
         movq mm1, mm2
         pand mm1, [product2a]
         por mm0, mm1
         movq [final2a], mm0

         movq mm0, [eax+ebx+ebx+color3]
         movq mm1, mm3
         por mm1, mm4
         por mm1, mm6
         pand mm0, mm1
         movq mm1, mm5
         pand mm1, [I23Pixel]
         por mm0, mm1
         movq mm1, mm7
         pand mm1, [product2a]
         por mm0, mm1
         movq mm1, mm2
         pand mm1, [product2b]
         por mm0, mm1
         movq [final2b], mm0


         movq mm0, [final1a]
         movq mm2, [final1b]
         movq mm1, mm0
         movq mm4, [final2a]
         movq mm6, [final2b]
         movq mm5, mm4
         punpcklwd mm0, mm2
         punpckhwd mm1, mm2
         punpcklwd mm4, mm6
         punpckhwd mm5, mm6




%ifdef __DJGPP__
         movq [fs:edx], mm0
         movq [fs:edx+8], mm1
         push edx
         add edx, [ebp+dstPitch]
         movq [fs:edx], mm4
         movq [fs:edx+8], mm5
         pop edx
%else
         movq [edx], mm0
         movq [edx+8], mm1
         push edx
         add edx, [ebp+dstPitch]
         movq [edx], mm4
         movq [edx+8], mm5
         pop edx
%endif
.SKIP_PROCESS:
         mov ecx, [ebp+deltaPtr]
         add ecx, 8
         mov [ebp+deltaPtr], ecx
         add edx, 16
         add eax, 8

         pop ecx
         sub ecx, 4
         cmp ecx, 0
         jg  near .Loop

; Restore some stuff
         popad
         mov esp, ebp
         pop ebp
         emms
         ret


;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------


;This is version 0.50
colorI   equ -2
colorE   equ 0
colorF   equ 2
colorJ   equ 4

colorG   equ -2
colorA   equ 0
colorB   equ 2
colorK   equ 4

colorH   equ -2
colorC   equ 0
colorD   equ 2
colorL   equ 4

colorM   equ -2
colorN   equ 0
colorO   equ 2
colorP   equ 4

NEWSYM _2xSaILineW
; Store some stuff
	 push ebp
	 mov ebp, esp
         pushad

; Prepare the destination
%ifdef __DJGPP__
         ; Set the selector
         mov eax, [ebp+dstSegment]
         mov fs, ax
%endif
         mov edx, [ebp+dstOffset]         ; edx points to the screen
; Prepare the source
         ; eax points to colorA
         mov eax, [ebp+srcPtr]
         mov ebx, [ebp+srcPitch]
         mov ecx, [ebp+width]
         ; eax now points to colorE
         sub eax, ebx


; Main Loop
.Loop:   push ecx

         ;-----Check Delta------------------
         mov ecx, [ebp+deltaPtr]

         movq mm0, [eax+colorI]
         movq mm1, [eax+colorJ]
         movq mm2, [eax+ebx+colorG]
         movq mm3, [eax+ebx+colorK]
         movq mm4, [eax+ebx+ebx+colorH]
         movq mm5, [eax+ebx+ebx+colorL]
         push eax
         add eax, ebx
         movq mm6, [eax+ebx+ebx+colorM]
         movq mm7, [eax+ebx+ebx+colorP]
         pop eax

         pcmpeqw mm0, [ecx+2+colorI]
         pcmpeqw mm1, [ecx+2+colorK]
         pcmpeqw mm2, [ecx+ebx+2+colorG]
         pcmpeqw mm3, [ecx+ebx+2+colorK]
         pcmpeqw mm4, [ecx+ebx+ebx+2+colorH]
         pcmpeqw mm5, [ecx+ebx+ebx+2+colorL]
         add ecx, ebx
         pcmpeqw mm6, [ecx+ebx+ebx+2+colorM]
         pcmpeqw mm7, [ecx+ebx+ebx+2+colorP]
         sub ecx, ebx


         pand mm0, mm1
         pand mm2, mm3
         pand mm4, mm5
         pand mm6, mm7
         pand mm0, mm2
         pand mm4, mm6
         pxor mm7, mm7
         pand mm0, mm4
         movq mm6, [eax+colorI]
         pcmpeqw mm7, mm0

         movq [ecx+2+colorI], mm6

         packsswb mm7, mm7
         movd ecx, mm7
         test ecx, ecx
         jz near .SKIP_PROCESS

         ;End Delta

         ;---------------------------------


;1
         ;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorE) && (colorB == colorL)
         movq mm0, [eax+ebx+colorA]        ;mm0 and mm1 contain colorA
         movq mm2, [eax+ebx+colorB]        ;mm2 and mm3 contain colorB

         movq mm1, mm0
         movq mm3, mm2

         pcmpeqw mm0, [eax+ebx+ebx+colorD]
         pcmpeqw mm1, [eax+colorE]
         pcmpeqw mm2, [eax+ebx+ebx+colorL]
         pcmpeqw mm3, [eax+ebx+ebx+colorC]

         pand mm0, mm1
         pxor mm1, mm1
         pand mm0, mm2
         pcmpeqw mm3, mm1
         pand mm0, mm3                 ;result in mm0

         ;if ((colorA == colorC) && (colorB != colorE) && (colorA == colorF) && (colorB == colorJ)
         movq mm4, [eax+ebx+colorA]        ;mm4 and mm5 contain colorA
         movq mm6, [eax+ebx+colorB]        ;mm6 and mm7 contain colorB
         movq mm5, mm4
         movq mm7, mm6

         pcmpeqw mm4, [eax+ebx+ebx+colorC]
         pcmpeqw mm5, [eax+colorF]
         pcmpeqw mm6, [eax+colorJ]
         pcmpeqw mm7, [eax+colorE]

         pand mm4, mm5
         pxor mm5, mm5
         pand mm4, mm6
         pcmpeqw mm7, mm5
         pand mm4, mm7                 ;result in mm4

         por mm0, mm4                  ;combine the masks
         movq [Mask1], mm0

         ;--------------------------------------------

;2
         ;if ((colorB == colorC) && (colorA != colorD) && (colorB == colorF) && (colorA == colorH)
         movq mm0, [eax+ebx+colorB]        ;mm0 and mm1 contain colorB
         movq mm2, [eax+ebx+colorA]        ;mm2 and mm3 contain colorA
         movq mm1, mm0
         movq mm3, mm2

         pcmpeqw mm0, [eax+ebx+ebx+colorC]
         pcmpeqw mm1, [eax+colorF]
         pcmpeqw mm2, [eax+ebx+ebx+colorH]
         pcmpeqw mm3, [eax+ebx+ebx+colorD]

         pand mm0, mm1
         pxor mm1, mm1
         pand mm0, mm2
         pcmpeqw mm3, mm1
         pand mm0, mm3                 ;result in mm0

         ;if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)
         movq mm4, [eax+ebx+colorB]        ;mm4 and mm5 contain colorB
         movq mm6, [eax+ebx+colorA]        ;mm6 and mm7 contain colorA
         movq mm5, mm4
         movq mm7, mm6

         pcmpeqw mm4, [eax+ebx+ebx+colorD]
         pcmpeqw mm5, [eax+colorE]
         pcmpeqw mm6, [eax+colorI]
         pcmpeqw mm7, [eax+colorF]

         pand mm4, mm5
         pxor mm5, mm5
         pand mm4, mm6
         pcmpeqw mm7, mm5
         pand mm4, mm7                 ;result in mm4

         por mm0, mm4                  ;combine the masks
         movq [Mask2], mm0


;interpolate colorA and colorB
         movq mm0, [eax+ebx+colorA]
         movq mm1, [eax+ebx+colorB]

         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3                ;mm0 contains the interpolated values

         ;assemble the pixels
         movq mm1, [eax+ebx+colorA]
         movq mm2, [eax+ebx+colorB]

         movq mm3, [Mask1]
         movq mm5, mm1
         movq mm4, [Mask2]
         movq mm6, mm1

         pand mm1, mm3
         por mm3, mm4
         pxor mm7, mm7
         pand mm2, mm4

         pcmpeqw mm3, mm7
         por mm1, mm2
         pand mm0, mm3

         por mm0, mm1

         punpcklwd mm5, mm0
         punpckhwd mm6, mm0

%ifdef __DJGPP__
         movq [fs:edx], mm5
         movq [fs:edx+8], mm6
%else
         movq [edx], mm5
         movq [edx+8], mm6
%endif

;------------------------------------------------
;        Create the Nextline
;------------------------------------------------
;3       ;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorG) && (colorC == colorO)
         movq mm0, [eax+ebx+colorA]        ;mm0 and mm1 contain colorA
         movq mm2, [eax+ebx+ebx+colorC]        ;mm2 and mm3 contain colorC
         movq mm1, mm0
         movq mm3, mm2

         push eax
         add eax, ebx
         pcmpeqw mm0, [eax+ebx+colorD]
         pcmpeqw mm1, [eax+colorG]
         pcmpeqw mm2, [eax+ebx+ebx+colorO]
         pcmpeqw mm3, [eax+colorB]
         pop eax

         pand mm0, mm1
         pxor mm1, mm1
         pand mm0, mm2
         pcmpeqw mm3, mm1
         pand mm0, mm3                 ;result in mm0

         ;if ((colorA == colorB) && (colorG != colorC) && (colorA == colorH) && (colorC == colorM)
         movq mm4, [eax+ebx+colorA]        ;mm4 and mm5 contain colorA
         movq mm6, [eax+ebx+ebx+colorC]        ;mm6 and mm7 contain colorC
         movq mm5, mm4
         movq mm7, mm6

         push eax
         add eax, ebx
         pcmpeqw mm4, [eax+ebx+colorH]
         pcmpeqw mm5, [eax+colorB]
         pcmpeqw mm6, [eax+ebx+ebx+colorM]
         pcmpeqw mm7, [eax+colorG]
         pop eax

         pand mm4, mm5
         pxor mm5, mm5
         pand mm4, mm6
         pcmpeqw mm7, mm5
         pand mm4, mm7                 ;result in mm4

         por mm0, mm4                  ;combine the masks
         movq [Mask1], mm0
         ;--------------------------------------------

;4
         ;if ((colorB == colorC) && (colorA != colorD) && (colorC == colorH) && (colorA == colorF)
         movq mm0, [eax+ebx+ebx+colorC]        ;mm0 and mm1 contain colorC
         movq mm2, [eax+ebx+colorA]        ;mm2 and mm3 contain colorA
         movq mm1, mm0
         movq mm3, mm2

         pcmpeqw mm0, [eax+ebx+colorB]
         pcmpeqw mm1, [eax+ebx+ebx+colorH]
         pcmpeqw mm2, [eax+colorF]
         pcmpeqw mm3, [eax+ebx+ebx+colorD]

         pand mm0, mm1
         pxor mm1, mm1
         pand mm0, mm2
         pcmpeqw mm3, mm1
         pand mm0, mm3                 ;result in mm0

         ;if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)
         movq mm4, [eax+ebx+ebx+colorC]        ;mm4 and mm5 contain colorC
         movq mm6, [eax+ebx+colorA]        ;mm6 and mm7 contain colorA
         movq mm5, mm4
         movq mm7, mm6

         pcmpeqw mm4, [eax+ebx+ebx+colorD]
         pcmpeqw mm5, [eax+ebx+colorG]
         pcmpeqw mm6, [eax+colorI]
         pcmpeqw mm7, [eax+ebx+ebx+colorH]

         pand mm4, mm5
         pxor mm5, mm5
         pand mm4, mm6
         pcmpeqw mm7, mm5
         pand mm4, mm7                 ;result in mm4

         por mm0, mm4                  ;combine the masks
         movq [Mask2], mm0
         ;----------------------------------------------

;interpolate colorA and colorC
         movq mm0, [eax+ebx+colorA]
         movq mm1, [eax+ebx+ebx+colorC]

         movq mm2, mm0
         movq mm3, mm1

         pand mm0, [colorMask]
         pand mm1, [colorMask]

         psrlw mm0, 1
         psrlw mm1, 1

         pand mm3, [lowPixelMask]
         paddw mm0, mm1

         pand mm3, mm2
         paddw mm0, mm3                ;mm0 contains the interpolated values
         ;-------------

         ;assemble the pixels
         movq mm1, [eax+ebx+colorA]
         movq mm2, [eax+ebx+ebx+colorC]

         movq mm3, [Mask1]
         movq mm4, [Mask2]

         pand mm1, mm3
         pand mm2, mm4

         por mm3, mm4
         pxor mm7, mm7
         por mm1, mm2

         pcmpeqw mm3, mm7
         pand mm0, mm3
         por mm0, mm1
         movq [ACPixel], mm0

;////////////////////////////////
; Decide which "branch" to take
;--------------------------------
         movq mm0, [eax+ebx+colorA]
         movq mm1, [eax+ebx+colorB]
         movq mm6, mm0
         movq mm7, mm1
         pcmpeqw mm0, [eax+ebx+ebx+colorD]
         pcmpeqw mm1, [eax+ebx+ebx+colorC]
         pcmpeqw mm6, mm7

         movq mm2, mm0
         movq mm3, mm0

         pand mm0, mm1       ;colorA == colorD && colorB == colorC
         pxor mm7, mm7

         pcmpeqw mm2, mm7
         pand mm6, mm0
         pand mm2, mm1       ;colorA != colorD && colorB == colorC

         pcmpeqw mm1, mm7

         pand mm1, mm3       ;colorA == colorD && colorB != colorC
         pxor mm0, mm6
         por mm1, mm6
         movq mm7, mm0
         movq [Mask2], mm2
         packsswb mm7, mm7
         movq [Mask1], mm1

         movd ecx, mm7
         test ecx, ecx
         jz near .SKIP_GUESS
;---------------------------------------------
; Map of the pixels:                    I|E F|J
;                                       G|A B|K
;                                       H|C D|L
;                                       M|N O|P
         movq mm6, mm0
         movq mm4, [eax+ebx+colorA]
         movq mm5, [eax+ebx+colorB]
         pxor mm7, mm7
         pand mm6, [ONE]

         movq mm0, [eax+colorE]
         movq mm1, [eax+ebx+colorG]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         movq mm0, [eax+colorF]
         movq mm1, [eax+ebx+colorK]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         push eax
         add eax, ebx
         movq mm0, [eax+ebx+colorH]
         movq mm1, [eax+ebx+ebx+colorN]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         movq mm0, [eax+ebx+colorL]
         movq mm1, [eax+ebx+ebx+colorO]
         movq mm2, mm0
         movq mm3, mm1
         pcmpeqw mm0, mm4
         pcmpeqw mm1, mm4
         pcmpeqw mm2, mm5
         pcmpeqw mm3, mm5
         pand mm0, mm6
         pand mm1, mm6
         pand mm2, mm6
         pand mm3, mm6
         paddw mm0, mm1
         paddw mm2, mm3

         pxor mm3, mm3
         pcmpgtw mm0, mm6
         pcmpgtw mm2, mm6
         pcmpeqw mm0, mm3
         pcmpeqw mm2, mm3
         pand mm0, mm6
         pand mm2, mm6
         paddw mm7, mm0
         psubw mm7, mm2

         pop eax
         movq mm1, mm7
         pxor mm0, mm0
         pcmpgtw mm7, mm0
         pcmpgtw mm0, mm1

         por mm7, [Mask1]
         por mm1, [Mask2]
         movq [Mask1], mm7
         movq [Mask2], mm1

.SKIP_GUESS:
         ;----------------------------
         ;interpolate A, B, C and D
         movq mm0, [eax+ebx+colorA]
         movq mm1, [eax+ebx+colorB]
         movq mm4, mm0
         movq mm2, [eax+ebx+ebx+colorC]
         movq mm5, mm1
         movq mm3, [qcolorMask]
         movq mm6, mm2
         movq mm7, [qlowpixelMask]

         pand mm0, mm3
         pand mm1, mm3
         pand mm2, mm3
         pand mm3, [eax+ebx+ebx+colorD]

         psrlw mm0, 2
         pand mm4, mm7
         psrlw mm1, 2
         pand mm5, mm7
         psrlw mm2, 2
         pand mm6, mm7
         psrlw mm3, 2
         pand mm7, [eax+ebx+ebx+colorD]

         paddw mm0, mm1
         paddw mm2, mm3

         paddw mm4, mm5
         paddw mm6, mm7

         paddw mm4, mm6
         paddw mm0, mm2
         psrlw mm4, 2
         pand mm4, [qlowpixelMask]
         paddw mm0, mm4      ;mm0 contains the interpolated value of A, B, C and D

;\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
         ;assemble the pixels
         movq mm1, [Mask1]
         movq mm2, [Mask2]
         movq mm4, [eax+ebx+colorA]
         movq mm5, [eax+ebx+colorB]
         pand mm4, mm1
         pand mm5, mm2

         pxor mm7, mm7
         por mm1, mm2
         por mm4, mm5
         pcmpeqw mm1, mm7
         pand mm0, mm1
         por mm4, mm0        ;mm4 contains the diagonal pixels

         movq mm0, [ACPixel]
         movq mm1, mm0
         punpcklwd mm0, mm4
         punpckhwd mm1, mm4

         push edx
         add edx, [ebp+dstPitch]

%ifdef __DJGPP__
         movq [fs:edx], mm0
         movq [fs:edx+8], mm1
%else
         movq [edx], mm0
         movq [edx+8], mm1
%endif
         pop edx

.SKIP_PROCESS:
         mov ecx, [ebp+deltaPtr]
         add ecx, 8
         mov [ebp+deltaPtr], ecx
         add edx, 16
         add eax, 8

         pop ecx
         sub ecx, 4
         cmp ecx, 0
         jg  near .Loop

; Restore some stuff
         popad
         mov esp, ebp
         pop ebp
         emms
         ret

;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------

NEWSYM Init_2xSaIMMXW
; Store some stuff
	 push ebp
	 mov ebp, esp
         push edx


;Damn thing doesn't work
;	 mov eax,1
;	 cpuid
;	 test edx, 0x00800000     ;test bit 23
;	 jz end2 ;bit not set => no MMX detected

	 mov eax, [ebp+8]         ;PixelFormat
	 cmp eax, 555
	 jz Bits555
	 cmp eax, 565
	 jz Bits565
end2:
	 mov eax, 1
	 jmp end
Bits555:
         mov edx, 0x7BDE7BDE
         mov eax, colorMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0x04210421
         mov eax, lowPixelMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0x739C739C
         mov eax, qcolorMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0x0C630C63
         mov eax, qlowpixelMask
         mov [eax], edx
         mov [eax+4], edx
         mov eax, 0
         jmp end
Bits565:
         mov edx, 0xF7DEF7DE
         mov eax, colorMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0x08210821
         mov eax, lowPixelMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0xE79CE79C
         mov eax, qcolorMask
         mov [eax], edx
         mov [eax+4], edx
         mov edx, 0x18631863
         mov eax, qlowpixelMask
         mov [eax], edx
         mov [eax+4], edx
         mov eax, 0
         jmp end
end:	
         pop edx
	 mov esp, ebp
	 pop ebp
	 ret


;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------
;-------------------------------------------------------------------------

	SECTION .data ALIGN = 32
;Some constants
colorMask     dd 0xF7DEF7DE, 0xF7DEF7DE
lowPixelMask  dd 0x08210821, 0x08210821

qcolorMask    dd 0xE79CE79C, 0xE79CE79C
qlowpixelMask dd 0x18631863, 0x18631863

FALSE         dd 0x00000000, 0x00000000
TRUE          dd 0xffffffff, 0xffffffff
ONE           dd 0x00010001, 0x00010001

ACPixel       times 8 db 0
Mask1         times 8 db 0
Mask2         times 8 db 0

I56Pixel      times 8 db 0
I23Pixel      times 8 db 0
Mask26        times 8 db 0
Mask35        times 8 db 0
Mask26b       times 8 db 0
Mask35b       times 8 db 0
product1a     times 8 db 0
product1b     times 8 db 0
product2a     times 8 db 0
product2b     times 8 db 0
final1a       times 8 db 0
final1b       times 8 db 0
final2a       times 8 db 0
final2b       times 8 db 0
