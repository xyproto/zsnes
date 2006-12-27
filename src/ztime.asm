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

EXTSYM GetTime,GetDate,GetLocalTime
EXTSYM SystemTimewHour,SystemTimewMinute,SystemTimewSecond

SECTION .text

NEWSYM Get_Time
    pushad
    call GetTime
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    ret

NEWSYM Get_TimeDate
    pushad
    call GetDate
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    ret

NEWSYM Get_Date
    ; dl = day, dh = month, cx = year
    pushad
    call GetDate
    mov [TempVarSeek],eax
    popad
    mov eax,[TempVarSeek]
    movzx edx,al ;Move day into edx, day is in BCD
    shr edx,4    ;Chop off the second digit
    imul edx,10  ;Multiply first digit by 10, since we want decimal
    and al,0xF   ;Remove first BCD digit
    add dl,al    ;Add second digit to first*10
    mov dh,ah    ;Copy month
    ;Year Calculation
    shr eax,16
    movzx ecx,al
    shr ecx,4
    imul ecx,10
    and al,0xF
    add cl,al
    add cx,1900
    ret

NEWSYM GetTimeInSeconds
    call GetLocalTime
    movzx eax,word[SystemTimewHour]
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTimewMinute]
    add eax,ebx
    mov ebx,60
    mul ebx
    movzx ebx,word[SystemTimewSecond]
    add eax,ebx
    ret

SECTION .data
TempVarSeek dd 0
