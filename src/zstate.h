#ifndef ZSTATE_H
#define ZSTATE_H

void BackupSystemVars(void);
void LoadSecondState(void);
void RestoreSystemVars(void);
void SaveSecondState(void);
void loadstate2(void);

extern uint32_t current_zst;

#endif
