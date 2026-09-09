#ifndef ZLOADER_H
#define ZLOADER_H

struct backup_cmdline_vars {
#ifdef __WIN32__
    unsigned char _KitchenSync, _KitchenSyncPAL, _ForceRefreshRate, _SetRefreshRate;
#endif
    unsigned short _joy_sensitivity;
    unsigned char _guioff;
    unsigned char _per2exec;
    unsigned char _HacksDisable;
#ifndef __UNIXSDL__
    unsigned char _vsyncon;
#endif
#ifdef __WIN32__
    unsigned char _TripleBufferWin;
#endif
    unsigned char _soundon;
    unsigned char _antienab;
    unsigned char _StereoSound;
    unsigned char _cvidmode;
    unsigned int _SoundQuality;
};

extern struct backup_cmdline_vars saved_cmdline_vars;

void swap_backup_vars(void);

#endif
