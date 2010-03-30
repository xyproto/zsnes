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
