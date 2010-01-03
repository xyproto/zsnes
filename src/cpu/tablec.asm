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



%include "macros.mac"

EXTSYM tableAc,tableBc,tableCc,tableDc,tableEc
EXTSYM tableFc,tableGc,tableHc,tableadc

%include "cpu/65816dc.inc"
%include "cpu/address.inc"
%include "cpu/addrni.inc"
%include "cpu/e65816c.inc"

; global variables

SECTION .text

eopINVALID
    ret

NEWSYM settableDm16
    mov dword[edi+61h*4],COp61m16d
    mov dword[edi+63h*4],COp63m16d
    mov dword[edi+65h*4],COp65m16d
    mov dword[edi+67h*4],COp67m16d
    mov dword[edi+69h*4],COp69m16d
    mov dword[edi+6Dh*4],COp6Dm16d
    mov dword[edi+6Fh*4],COp6Fm16d
    mov dword[edi+71h*4],COp71m16d
    mov dword[edi+72h*4],COp72m16d
    mov dword[edi+73h*4],COp73m16d
    mov dword[edi+75h*4],COp75m16d
    mov dword[edi+77h*4],COp77m16d
    mov dword[edi+79h*4],COp79m16d
    mov dword[edi+7Dh*4],COp7Dm16d
    mov dword[edi+7Fh*4],COp7Fm16d
    mov dword[edi+0E1h*4],COpE1m16d
    mov dword[edi+0E3h*4],COpE3m16d
    mov dword[edi+0E5h*4],COpE5m16d
    mov dword[edi+0E7h*4],COpE7m16d
    mov dword[edi+0E9h*4],COpE9m16d
    mov dword[edi+0EDh*4],COpEDm16d
    mov dword[edi+0EFh*4],COpEFm16d
    mov dword[edi+0F1h*4],COpF1m16d
    mov dword[edi+0F2h*4],COpF2m16d
    mov dword[edi+0F3h*4],COpF3m16d
    mov dword[edi+0F5h*4],COpF5m16d
    mov dword[edi+0F7h*4],COpF7m16d
    mov dword[edi+0F9h*4],COpF9m16d
    mov dword[edi+0FDh*4],COpFDm16d
    mov dword[edi+0FFh*4],COpFFm16d
    ret


