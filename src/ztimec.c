/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <time.h>

short SystemTimewHour;
short SystemTimewMinute;
short SystemTimewSecond;

void GetLocalTime()
{
  time_t current;
  struct tm *timeptr;

  time(&current);
  timeptr = localtime(&current);
  SystemTimewHour = timeptr->tm_hour;
  SystemTimewMinute = timeptr->tm_min;
  SystemTimewSecond = timeptr->tm_sec;
}

unsigned int GetTime()
{
  unsigned int value;
  struct tm *newtime;
  time_t long_time;

  time(&long_time);
  newtime = localtime(&long_time);

  value = ((newtime->tm_sec)  % 10)+((newtime->tm_sec) /10)*16
         +((((newtime->tm_min)  % 10)+((newtime->tm_min) /10)*16) <<  8)
         +((((newtime->tm_hour) % 10)+((newtime->tm_hour)/10)*16) << 16);
  return(value);
}

unsigned int GetDate()
{
  unsigned int value;
  struct tm *newtime;
  time_t long_time;

  time(&long_time);
  newtime = localtime( &long_time );
  value = ((newtime->tm_mday) % 10)+((newtime->tm_mday)/10)*16
         +(((newtime->tm_mon)+1) << 8)
         +((((newtime->tm_year) % 10)+((newtime->tm_year)/10)*16) << 16)
         +((newtime->tm_wday) << 28);
  return(value);
}
