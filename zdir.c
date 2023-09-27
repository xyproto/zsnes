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

#define _ATFILE_SOURCE

#include <sys/stat.h>

#include "zdir.h"
#include "zpath.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#define FIND_GOOD(handle) ((handle) != -1)
#define FIND_FAIL(handle) ((handle) == -1)
#define ff_name name
#define ff_fsize size
#define ff_attrib attrib
#define WILD_ALL "*"

// Note, these are faster than the built in DJGPP/MinGW ones
z_DIR *z_opendir(const char *path) {
	z_DIR *dir = 0;
	if (path && *path) {
		char search[PATH_SIZE];
		strcpy(search, path);
		strcatslash(search);
		strcat(search, WILD_ALL);

		dir = malloc(sizeof(z_DIR));
		if (dir) {
			dir->find_first_handle = _findfirst(search, &dir->fileinfo);
			if (FIND_FAIL(dir->find_first_handle)) {
				// ENOENT set by findfirst already
				free(dir);
				dir = 0;
			}
		} else {
			errno = ENOMEM;
		}
	} else {
		errno = EINVAL;
	}

	return (dir);
}

struct z_dirent *z_readdir(z_DIR *dir) {
	struct z_dirent *entry = 0;
	if (FIND_GOOD(dir->find_first_handle)) {
		entry = &dir->entry;
		strcpy(entry->d_name, dir->fileinfo.ff_name);
		if (FIND_FAIL(_findnext(dir->find_first_handle, &dir->fileinfo))) {
			_findclose(dir->find_first_handle);
			dir->find_first_handle = -1;
		}
	} else {
		errno = EBADF;
	}

	return (entry);
}

int z_closedir(z_DIR *dir) {
	int result = 0;

	if (dir) {
		if (FIND_GOOD(dir->find_first_handle)) {
			_findclose(dir->find_first_handle);
		}
		free(dir);
	} else {
		result = -1;
		errno = EBADF;
	}

	return (result);
}

struct dirent_info *readdir_info(z_DIR *dir) {
	static struct dirent_info info;
	struct dirent_info *infop = 0;

	if (FIND_GOOD(dir->find_first_handle)) {
		strcpy(dir->entry.d_name, dir->fileinfo.ff_name);

		info.name = dir->entry.d_name;
		info.size = dir->fileinfo.ff_fsize;
		info.mode = S_IREAD;
		if (!(dir->fileinfo.ff_attrib & _A_RDONLY)) {
			info.mode |= S_IWRITE;
		}
		if (dir->fileinfo.ff_attrib & _A_SUBDIR) {
			info.mode |= S_IFDIR;
		} else {
			info.mode |= S_IFREG;
		}
		infop = &info;

		if (FIND_FAIL(_findnext(dir->find_first_handle, &dir->fileinfo))) {
			_findclose(dir->find_first_handle);
			dir->find_first_handle = -1;
		}
	} else {
		errno = EBADF;
	}

	return (infop);
}

#else
#include <unistd.h>
#include "linux/lib.h"

struct dirent_info *readdir_info(z_DIR *dir) {
	static struct dirent_info info;
	struct dirent_info *infop = 0;

	struct dirent *entry = readdir(dir);
	if (entry) {
		struct stat stat_buffer;
		if (!fstatat(dirfd(dir), entry->d_name, &stat_buffer, 0)) {
			info.name = entry->d_name;
			info.size = stat_buffer.st_size;
			info.mode = stat_buffer.st_mode;
			info.uid = stat_buffer.st_uid;
			info.gid = stat_buffer.st_gid;
			infop = &info;
		} else {
			infop = readdir_info(dir);
		}
	}
	return (infop);
}

int dirent_access(struct dirent_info *entry, int mode) {
	int accessable = 0; // This is accessable, non access is -1

	if (!entry) {
		accessable = -1;
		errno = EACCES;
	} else if (mode) {
		uid_t uid = geteuid();
		gid_t gid = getegid();

		if (!(
				(!(mode & R_OK) || ((entry->mode & S_IROTH) || ((gid == entry->gid) && (entry->mode & S_IRGRP)) || ((uid == entry->uid) && (entry->mode & S_IRUSR)))) && (!(mode & W_OK) || ((entry->mode & S_IWOTH) || ((gid == entry->gid) && (entry->mode & S_IWGRP)) || ((uid == entry->uid) && (entry->mode & S_IWUSR)))) && (!(mode & X_OK) || ((entry->mode & S_IXOTH) || ((gid == entry->gid) && (entry->mode & S_IXGRP)) || ((uid == entry->uid) && (entry->mode & S_IXUSR)))))) {
			accessable = -1;
			errno = EACCES;
		}
	}

	return (accessable);
}

#endif
