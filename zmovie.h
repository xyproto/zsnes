/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#ifndef ZMOVIE_H
#define ZMOVIE_H

#include "gblvars.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>

void mzt_chdir_up();
void mzt_chdir_down();
bool mzt_save(int, bool, bool);
bool mzt_load(int, bool);

void GetMovieFrameStr(void);
void MovieRecord();
void MoviePlay();
void MovieStop();
void MovieAppend();
void MovieDumpRaw(void);
bool MovieInProgress();
void MovieInsertChapter(void);
void MovieSeekAhead(void);
void MovieSeekBehind(void);
void ResetDuringMovie(void);
void SkipMovie(void);

extern unsigned char MovieProcessing;
extern uint8_t MovieRecordWinVal;

enum MovieStatus { MOVIE_OFF = 0,
    MOVIE_PLAYBACK,
    MOVIE_RECORD,
    MOVIE_OLD_PLAY,
    MOVIE_ENDING_DUMPING,
    MOVIE_DUMPING_NEW,
    MOVIE_DUMPING_OLD };
#define SetMovieMode(mode) (MovieProcessing = (unsigned char)mode)

enum MZT_FORCE_MODE_SWITCH { RTR_OFF = 0,
    RTR_REPLAY_TO_RECORD,
    RTR_RECORD_TO_REPLAY };

void MovieInsertChapter(void);

void MovieSeekAhead(void);

void MovieSeekBehind(void);

void ResetDuringMovie(void);

void MovieDumpRaw(void);

extern u1 MovieForcedLengthEnabled;
extern u1 lameExists;
extern u1 mencoderExists;
extern bool RawDumpInProgress;
extern char MovieFrameStr[10];
extern unsigned char MoviePassWaiting;

#endif
