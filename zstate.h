#ifndef ZSTATE_H
#define ZSTATE_H

#include <stdint.h>
#include <time.h>

#include "types.h"

int zst_exists(void);
void BackupSystemVars(void);
void LoadSecondState(void);
void RestoreSystemVars(void);
void SaveSecondState(void);
void SaveSramData(void);
void loadstate(void);
void loadstate2(void);
void savespcdata(void);
void statesaver(void);
void zst_determine_newest(void);

extern time_t newestfiledate;
extern uint32_t current_zst;
extern uint32_t newest_zst;

extern char spcsaved[16];
extern u4 Totalbyteloaded;

uint64_t zst_state_hash(void);

/* Non-zero if any cartridge would produce two state formats of the same
   length, which the loader tells apart by length alone. */
int zst_format_check(void);

#endif
