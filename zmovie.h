#ifndef ZMOVIE_H
#define ZMOVIE_H

#include "gblvars.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>

void mzt_chdir_up(void);
void mzt_chdir_down(void);
bool mzt_save(int, bool, bool);
bool mzt_load(int, bool);

void GetMovieFrameStr(void);
void MovieRecord(void);
void MoviePlay(void);
void MovieStop(void);
void MovieAppend(void);
void MovieDumpRaw(void);
bool MovieInProgress(void);
void MovieInsertChapter(void);
void MovieSeekAhead(void);
void MovieSeekBehind(void);
void ResetDuringMovie(void);
void SkipMovie(void);

extern uint8_t MovieProcessing;
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
extern uint8_t MoviePassWaiting;

#endif
