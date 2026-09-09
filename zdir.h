#ifndef ZDIR_H
#define ZDIR_H

struct dirent_info {
    char* name;
    mode_t mode;
    off_t size;
#ifdef __UNIXSDL__
    uid_t uid;
    gid_t gid;
#endif
};

#ifndef __UNIXSDL__
#include <stdint.h>

#include <io.h>
#include <windows.h>

// Avoid clashing with DJGPP and MinGW extras

struct z_dirent {
    char d_name[256];
};

typedef struct
{
    intptr_t find_first_handle;
    struct _finddata_t fileinfo;
    struct z_dirent entry;
} z_DIR;

z_DIR* z_opendir(const char* path);
struct z_dirent* z_readdir(z_DIR* dir);
int z_closedir(z_DIR* dir);

#ifndef NO_ZDIR_TYPEDEF
#define dirent z_dirent
typedef z_DIR DIR;
#define opendir z_opendir
#define readdir z_readdir
#define closedir z_closedir
#endif

#else
#include <dirent.h>
typedef DIR z_DIR;
#endif

struct dirent_info* readdir_info(z_DIR* dir);
int dirent_access(struct dirent_info* entry, int mode);

#endif
