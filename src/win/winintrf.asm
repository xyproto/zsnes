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

; NOTE: For timing, Game60hzcall should be called at 50hz or 60hz (depending
;   on romispal) after a call to InitPreGame and before DeInitPostGame are
;   made.  GUI36hzcall should be called at 36hz after a call GUIInit and
;   before GUIDeInit.

SECTION .data
NEWSYM CurKeyPos, dd 0
NEWSYM CurKeyReadPos, dd 0
NEWSYM KeyBuffer, times 16 dd 0
SECTION .text

; ****************************
; Video Stuff
; ****************************

; ** copy video mode functions **
SECTION .data
NEWSYM converta, dd 0

; ** Video Mode Variables **

; Video Mode Feature Availability (1 = Available, 0 = Not Available)
; Left side starts with Video Mode 0
;                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
NEWSYM GUIWFVID,  db 0,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,1,0,1,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,1,0 ; Fullscreen
NEWSYM GUIDSIZE,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; D Modes
NEWSYM GUISMODE,  db 0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; Win Stretched Modes
NEWSYM GUIDSMODE, db 0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1 ; Win D-Stretched Modes
NEWSYM GUIKEEP43, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1 ; Keep 4:3 Ratio
NEWSYM GUIM7VID,  db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; Hires Mode 7
NEWSYM GUIHQ2X,   db 0,0,0,1,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; Hq2x Filter
NEWSYM GUIHQ3X,   db 0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; Hq3x Filter
NEWSYM GUIHQ4X,   db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1 ; Hq4x Filter
NEWSYM GUINTVID,  db 0,0,0,0,0,1,1,1,0,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; NTSC Filter
NEWSYM GUIBIFIL,  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ; Bilinear Filter

SECTION .text

; ****************************
; Input Device Stuff
; ****************************

; Variables related to Input Device Routines:
;   pl1selk,pl1startk,pl1upk,pl1downk,pl1leftk,pl1rightk,pl1Xk,
;   pl1Ak,pl1Lk,pl1Yk,pl1Bk,pl1Rk
;     (Change 1 to 2,3,4 for other players)
;     Each of these variables contains the corresponding key pressed value
;       for the key data
;   pressed[]
;     - This is an array of pressed/released data (bytes) where the
;       corresponding key pressed value is used as the index.  The value
;       for each entry is 0 for released and 1 for pressed.  Also, when
;       writing keyboard data to this array, be sure to first check if
;       the value of the array entry is 2 or not.  If it is 2, do not write 1
;       to that array entry. (however, you can write 0 to it)
;   As an example, to access Player 1 L button press data, it is
;     done like : pressed[pl1Lk]
;   The 3 character key description of that array entry is accessed by the
;     GUI through ScanCodeListing[pl1Lk*3]

; Note: When storing the input device configuration of a dynamic input
;   device system (ie. Win9x) rather than a static system (ie. Dos), it
;   is best to store in the name of the device and relative button
;   assignments in the configuration file, then convert it to ZSNES'
;   numerical corresponding key format after reading from it. And then
;   convert it back when writing to it back.

SECTION .data

; Total Number of Input Devices
NEWSYM NumInputDevices, dd 2

; Input Device Names
NEWSYM GUIInputNames
db 'NONE            ',0
db 'KEYBOARD/GAMEPAD',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0
db '                ',0

; GUI Description codes for each corresponding key pressed value
NEWSYM ScanCodeListing
        db '---','ESC',' 1 ',' 2 ',' 3 ',' 4 ',' 5 ',' 6 '  ; 00h
        db ' 7 ',' 8 ',' 9 ',' 0 ',' - ',' = ','BKS','TAB'
        db ' Q ',' W ',' E ',' R ',' T ',' Y ',' U ',' I '  ; 10h
        db ' O ',' P ',' [ ',' ] ','RET','LCT',' A ',' S '
        db ' D ',' F ',' G ',' H ',' J ',' K ',' L ',' : '  ; 20h
        db ' " ',' ~ ','LSH',' \ ',' Z ',' X ',' C ',' V '
        db ' B ',' N ',' M ',' , ',' . ',' / ','RSH',' * '  ; 30h
        db 'LAL','SPC','CAP','F1 ','F2 ','F3 ','F4 ','F5 '
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','KP7'  ; 40h
        db 'KP8','KP9','KP-','KP4','KP5','KP6','KP+','KP1'
        db 'KP2','KP3','KP0','KP.','   ','   ','OEM','F11'  ; 50h
        db 'F12','59H','5AH','5BH','5CH','5DH','5EH','5FH'
        db '60H','61H','62H','63H','64H','65H','66H','67H'  ; 60h
        db '68H','69H','6AH','6BH','6CH','6DH','6EH','6FH'
        db '70H','71H','72H','73H','74H','75H','76H','77H'  ; 70h
        db '78H','79H','7AH','7BH','7CH','7DH','7EH','7FH'
        ; Keyboard continued (Direct Input)
        db '80H','81H','82H','83H','84H','85H','86H','87H'  ; 80h
        db '88H','89H','8AH','8BH','8CH','8DH','8EH','8FH'
        db '90H','91H','92H','93H','94H','95H','96H','97H'  ; 90h
        db '98H','99H','9AH','9BH','9CH','9DH','9EH','9FH'
        db 'A0H','A1H','A2H','A3H','A4H','A5H','A6H','A7H'  ; A0h
        db 'A8H','A9H','AAH','ABH','ACH','ADH','AEH','AFH'
        db 'B0H','B1H','B2H','B3H','B4H','B5H','B6H','B7H'  ; B0h
        db 'B8H','B9H','BAH','BBH','BCH','BDH','BEH','BFH'
        db 'C0H','C1H','C2H','C3H','C4H','C5H','C6H','C7H'  ; C0h
        db 'C8H','C9H','CAH','CBH','CCH','CDH','CEH','CFH'
        db 'D0H','D1H','D2H','D3H','D4H','D5H','D6H','D7H'  ; D0h
        db 'D8H','D9H','DAH','DBH','DCH','DDH','DEH','DFH'
        db 'E0H','E1H','E2H','E3H','E4H','E5H','E6H','E7H'  ; E0h
        db 'E8H','E9H','EAH','EBH','ECH','EDH','EEH','EFH'
        db 'F0H','F1H','F2H','F3H','F4H','F5H','F6H','F7H'  ; F0h
        db 'F8H','F9H','FAH','FBH','FCH','FDH','FEH','FFH'
        ; Joystick Stuff (Direct Input)
        db 'J00','J01','J02','J03','J04','J05','J06','J07'
        db 'J08','J09','J0A','J0B','J0C','J0D','J0E','J0F'
        db 'J10','J11','J12','J13','J14','J15','J16','J17'
        db 'J18','J19','J1A','J1B','J1C','J1D','J1E','J1F'
        db 'J20','J21','J22','J23','J24','J25','J26','J27'
        db 'J28','J29','J2A','J2B','J2C','J2D','J2E','J2F'
        db 'J30','J31','J32','J33','J34','J35','J36','J37'
        db 'J38','J39','J3A','J3B','J3C','J3D','J3E','J3F'
        db 'J40','J41','J42','J43','J44','J45','J46','J47'
        db 'J48','J49','J4A','J4B','J4C','J4D','J4E','J4F'
        db 'J50','J51','J52','J53','J54','J55','J56','J57'
        db 'J58','J59','J5A','J5B','J5C','J5D','J5E','J5F'
        db 'J60','J61','J62','J63','J64','J65','J66','J67'
        db 'J68','J69','J6A','J6B','J6C','J6D','J6E','J6F'
        db 'J70','J71','J72','J73','J74','J75','J76','J77'
        db 'J78','J79','J7A','J7B','J7C','J7D','J7E','J7F'
        ; Extra Stuff (180h) (Parallel Port)
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
