#ifndef ZSTATE_H
#define ZSTATE_H

#include <stdint.h>
#include <time.h>

void BackupSystemVars(void);
void LoadSecondState(void);
void RestoreSystemVars(void);
void SaveSecondState(void);
void SaveSramData(void);
void loadstate2(void);
void statesaver(void);
void zst_determine_newest(void);

extern time_t   newestfiledate;
extern uint32_t current_zst;

#endif
