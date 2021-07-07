#ifndef ZSTATE_H
#define ZSTATE_H

#include <stdint.h>
#include <time.h>

int zst_exists(void);
void BackupSystemVars(void);
void LoadSecondState(void);
void RestoreSystemVars(void);
void SaveSecondState(void);
void SaveSramData(void);
void loadstate2(void);
void statesaver(void);
void zst_determine_newest(void);

void statesaver(void);

void loadstate(void);

void SaveSramData(void);

void savespcdata(void);

extern time_t newestfiledate;
extern uint32_t current_zst;
extern uint32_t newest_zst;

extern char spcsaved[16];
extern u4 Totalbyteloaded;

#endif
