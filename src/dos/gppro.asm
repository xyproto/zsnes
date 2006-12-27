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

section .data
sample times 60 db 0
clock_mask db 0
data_mask db 0

section .text

NEWSYM read_gpp
   push ebx
   push ecx
   push edx
   push edi
   push esi

   cmp al,0
   jne GPP_L14
   mov byte[clock_mask],0x10
   mov byte[data_mask],0x20
   jmp GPP_L15
GPP_L14:
   mov byte[clock_mask],0x40
   mov byte[data_mask],0x80
GPP_L15:


   xor ebx,ebx
   xor edi,edi

   cli
   in al,dx
   mov ah,al

GPP_L4:
   xor ecx,ecx
GPP_L0:
   nop
   nop
   nop
   nop
   nop
   nop
   in al,dx
   cmp al,ah
   jne GPP_L1
   inc ecx
   cmp ecx,255
   jl GPP_L0
GPP_L1:
   cmp ecx,255
   je near GPP_ERR

   test [clock_mask],ah
   jz GPP_L2
   test [clock_mask],al
   jnz GPP_L2

   test [data_mask],al
   jz GPP_L3
   mov byte[sample+edi],1
   jmp GPP_L12
GPP_L3:
   mov byte[sample+edi],0
GPP_L12:
   inc edi

GPP_L2:
   mov ah,al
   cmp ebx,200
   je GPP_L13
   inc ebx
   cmp edi,50
   jl GPP_L4

GPP_L13:
   sti
   xor ecx,ecx
   mov esi,1
GPP_L7:
   cmp byte[sample+esi],1
   jg GPP_ERR
   jne GPP_L6
   inc ecx
   jmp GPP_L5
GPP_L6:
   xor ecx,ecx

GPP_L5:
   cmp ecx,5
   je GPP_L8
   cmp esi,edi
   je GPP_L8
   inc esi
   jmp GPP_L7

GPP_L8:
   cmp ecx,5
   jne GPP_ERR
   add esi,2
   xor eax,eax
   xor ebx,ebx
   xor ecx,ecx
   xor edx,edx

GPP_L10:
   inc ecx
   cmp ecx,5
   jne GPP_L11
   mov ecx,1
   inc esi
GPP_L11:
   mov dl,[sample+esi]
   or eax,edx
   shl eax,1
   cmp ebx,13
   je GPP_L9
   inc ebx
   inc esi
   jmp GPP_L10

GPP_L9:
   pop esi
   pop edi
   pop edx
   pop ecx
   pop ebx
   ret

GPP_ERR:
   sti
   pop esi
   pop edi
   pop edx
   pop ecx
   pop ebx
   mov eax,1
   ret


