#ifndef ZCOMPAT_H
#define ZCOMPAT_H

/* Bundled compression shim.

   ZSNES used to link against the system zlib. It is now built against the
   vendored miniz (miniz.c / miniz.h) instead, so there is no external
   compression dependency on any target - in particular the 32-bit Windows
   cross build no longer needs a hand-built mingw zlib.

   miniz already exposes the zlib-compatible spellings the emulator relies on
   (compress2, uncompress, crc32, z_stream, inflate*, the Z_* constants and
   the Bytef/uInt/... typedefs), so callers that used those need only include
   this header in place of <zlib.h>. What miniz does NOT provide is zlib's
   gz* stdio-style file API, so that thin layer is implemented on top of
   miniz's raw DEFLATE in zcompat.c and declared below. */

#include "miniz.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque gzip file handle, matching zlib's gzFile shape (a pointer). */
typedef struct zc_gzfile* gzFile;

/* Open a file whose contents are (de)compressed with the gzip framing.
   mode is a zlib-style string: it must contain 'r' (read) or 'w' (write),
   may contain a compression level digit '1'..'9' for writing, and any 'b'
   is accepted and ignored. Returns NULL on failure. */
gzFile gzopen(const char* path, const char* mode);

/* Like gzopen but wrapping an already-open descriptor. Unlike zlib, the
   descriptor is NOT closed by gzclose here, so an existing FILE* over the
   same fd stays valid for the caller to fclose (see loadGZipFile). */
gzFile gzdopen(int fd, const char* mode);

/* Read up to len uncompressed bytes. Transparently passes through data that
   is not in gzip format, mirroring zlib. Returns the byte count, 0 at end of
   input, or -1 on error. */
int gzread(gzFile file, void* buf, unsigned len);

/* Write len bytes, compressing them. Returns len, or 0 on error. */
int gzwrite(gzFile file, const void* buf, unsigned len);

/* Write a single byte. Returns the byte written, or -1 on error. */
int gzputc(gzFile file, int c);

/* Non-zero once reading has reached the end of the (decompressed) input. */
int gzeof(gzFile file);

/* Non-zero when the stream is being read/written without compression
   (i.e. the input was not gzip data). Matches zlib's gzdirect. */
int gzdirect(gzFile file);

/* Flush/finish and release the handle. Returns 0 (Z_OK) on success. */
int gzclose(gzFile file);

#ifdef __cplusplus
}
#endif

#endif /* ZCOMPAT_H */
