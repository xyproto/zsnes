/* Reading .zip archives: enough of the format to pull a ROM or a patch out of
   one, written from the PKZIP APPNOTE against zlib's inflate.

   Only the two compression methods that appear in practice are handled -
   stored and deflate - and archives that need the ZIP64 extensions are
   refused rather than half read. */

#ifndef ZSNES_ZIP_ZIPREAD_H
#define ZSNES_ZIP_ZIPREAD_H

#include <stddef.h>
#include <stdint.h>

typedef struct ZipFile ZipFile;

enum { ZIP_OK = 0,
    ZIP_END = 1, /* the cursor has run off the end of the archive */
    ZIP_ERR = -1 };

/* NULL if the file is missing, unreadable, or not an archive. */
ZipFile* zip_open(char const* path);
void zip_close(ZipFile* z);

/* Move the cursor. ZIP_END once there is nothing left. */
int zip_first(ZipFile* z);
int zip_next(ZipFile* z);

/* Put the cursor on a named member. */
int zip_locate(ZipFile* z, char const* name, int case_sensitive);

/* The member under the cursor. Either pointer may be NULL. `name` is always
   NUL terminated when there is room. */
int zip_entry(ZipFile* z, char* name, size_t name_size, uint32_t* uncompressed);

/* Start reading the member under the cursor, then take it in as many pieces
   as suits the caller. zip_read returns bytes read, 0 at the end, and -1 on a
   corrupt archive, so it reads like fread. */
int zip_open_entry(ZipFile* z);
long zip_read(ZipFile* z, void* buf, size_t len);
void zip_close_entry(ZipFile* z);

#endif
