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
NEWSYM dssel, dw 0

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

; GUI Description codes for each corresponding key pressed value
NEWSYM ScanCodeListing
        db '---','ESC',' 1 ',' 2 ',' 3 ',' 4 ',' 5 ',' 6 '
        db ' 7 ',' 8 ',' 9 ',' 0 ',' - ',' = ','BKS','TAB'
        db ' Q ',' W ',' E ',' R ',' T ',' Y ',' U ',' I '
        db ' O ',' P ',' [ ',' ] ','RET','CTL',' A ',' S '
        db ' D ',' F ',' G ',' H ',' J ',' K ',' L ',' : '
        db ' " ',' ~ ','LSH',' \ ',' Z ',' X ',' C ',' V '
        db ' B ',' N ',' M ',' , ',' . ',' / ','RSH',' * '
        db 'ALT','SPC','CAP','F1 ','F2 ','F3 ','F4 ','F5 '
        db 'F6 ','F7 ','F8 ','F9 ','F10','NUM','SCR','HOM'
        db 'UP ','PGU',' - ','LFT',' 5 ','RGT',' + ','END'
        db 'DWN','PGD','INS','DEL','   ','   ','   ','F11'
        db 'F12','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        db '   ','   ','   ','   ','   ','   ','   ','   '
        ; Joystick Stuff, Port 201h (80h)
        db 'JB7','JB8','JB1','JB2','JB3','JB4','JB5','JB6'
        db 'SWA','SWB','SWC','SWX','SWY','SWZ','SWL','SWR'
        db 'S2A','S2B','S2C','S2X','S2Y','S2Z','S2L','S2R'
        db 'S3A','S3B','S3C','S3X','S3Y','S3Z','S3L','S3R'
        db 'S4A','S4B','S4C','S4X','S4Y','S4Z','S4L','S4R'
        db 'GRR','GRB','GRY','GRG','GL1','GL2','GR1','GR2'
        db 'G2R','G2B','G2Y','G2G','2L1','2L2','2R1','2R2'
        db 'G3R','G3B','G3Y','G3G','3L1','3L2','3R1','3R2'
        db 'G4R','G4B','G4Y','G4G','4L1','4L2','4R1','4R2'
        db 'SWS','SWM','GSL','GST','JUP','JDN','JLF','JRG'
        db 'S2S','S2M','2SL','2ST','SWU','SWD','SWL','SWR'
        db 'S3S','S3M','3SL','3ST','S2U','S2D','S2L','S2R'
        db 'S4S','S4M','4SL','4ST','S3U','S3D','S3L','S3R'
        db 'J2U','J2D','J2L','J2R','S4U','S4D','S4L','S4R'
        db 'GRU','GRD','GRL','GRR','G2U','G2D','G2L','G2R'
        db 'G3U','G3D','G3L','G3R','G4U','G4D','G4L','G4R'
        ; Joystick Stuff, Port 209h (100h)
        db 'JB7','JB8','JB1','JB2','JB3','JB4','JB5','JB6'
        db 'SWA','SWB','SWC','SWX','SWY','SWZ','SWL','SWR'
        db 'S2A','S2B','S2C','S2X','S2Y','S2Z','S2L','S2R'
        db 'S3A','S3B','S3C','S3X','S3Y','S3Z','S3L','S3R'
        db 'S4A','S4B','S4C','S4X','S4Y','S4Z','S4L','S4R'
        db 'GRR','GRB','GRY','GRG','GL1','GL2','GR1','GR2'
        db 'G2R','G2B','G2Y','G2G','2L1','2L2','2R1','2R2'
        db 'G3R','G3B','G3Y','G3G','3L1','3L2','3R1','3R2'
        db 'G4R','G4B','G4Y','G4G','4L1','4L2','4R1','4R2'
        db 'SWS','SWM','GSL','GST','JUP','JDN','JLF','JRG'
        db 'S2S','S2M','2SL','2ST','SWU','SWD','SWL','SWR'
        db 'S3S','S3M','3SL','3ST','S2U','S2D','S2L','S2R'
        db 'S4S','S4M','4SL','4ST','S3U','S3D','S3L','S3R'
        db 'J2U','J2D','J2L','J2R','S4U','S4D','S4L','S4R'
        db 'GRU','GRD','GRL','GRR','G2U','G2D','G2L','G2R'
        db 'G3U','G3D','G3L','G3R','G4U','G4D','G4L','G4R'
        ; Extra Stuff (180h) (Parallel Port)
        db 'PPB','PPY','PSL','PST','PUP','PDN','PLT','PRT'
        db 'PPA','PPX','PPL','PPR','   ','   ','   ','   '
        db 'P2B','P2Y','P2S','P2T','P2U','P2D','P2L','P2R'
        db 'P2A','P2X','P2L','P2R','   ','   ','   ','   '
        db 'P3B','P3Y','P3S','P3T','P3U','P3D','P3L','P3R'
        db 'P3A','P3X','P3L','P3R','   ','   ','   ','   '
        db 'P4B','P4Y','P4S','P4T','P4U','P4D','P4L','P4R'
        db 'P4A','P4X','P4L','P4R','   ','   ','   ','   '
        db 'P5B','P5Y','P5S','P5T','P5U','P5D','P5L','P5R'
        db 'P5A','P5X','P5L','P5R','   ','   ','   ','   '
