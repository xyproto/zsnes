/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



extern unsigned int newengen, nggposng[2];
extern unsigned short PrevPicture[64*56], *vidbuffer;

void CapturePicture()
{
    unsigned short work1, work2, filter;
    unsigned int i, j, offset, pppos=0;

    if ((newengen & 0xFF) && ((nggposng[0] & 0xFF) == 5))
    {
	filter = 0x7BDE;	// 0111 1011 1101 1110
    }
    else
    {
	filter = 0xF7DE;	// 1111 0111 1101 1110
    }

    for (j=0 ; j<56 ; j++)
    {
	offset = 288+16+j*288*4;

	for (i=0 ; i<64 ; i++)
	{
	    work1 = ((vidbuffer[offset] & filter)>>1) + ((vidbuffer[offset+2] & filter)>>1);
	    work2 = ((vidbuffer[offset+288] & filter)>>1) + ((vidbuffer[offset+288+2] & filter)>>1);
	    PrevPicture[pppos] = ((work1 & filter)>>1) + ((work2 & filter)>>1);
	    offset += 4;
	    pppos++;
	}
    }

    if ((newengen & 0xFF) && ((nggposng[0] & 0xFF) == 5))
    {
	for (pppos=0 ; pppos<64*56 ; pppos++)
	{
	    PrevPicture[pppos] = ((PrevPicture[pppos] & 0x7FE0)<<1)|(PrevPicture[pppos] & 0x001F);
	} // 0111 1111 1110 0000 and 0000 0000 0001 1111
    }
}
