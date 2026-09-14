#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "types.h"

/* Self-describing envelope for the V2 (2.4.0+) save-state format.

   The pre-2.4.0 formats named themselves only by a header string and were
   told apart by file length; a length that matched the wrong format loaded a
   state at the wrong layout, and a state restored at the wrong layout runs off
   into random memory. V2 records its own body length, a CRC over that body,
   and a byte-order flag, so a truncated, foreign, or mismatched file is
   rejected rather than misread.

   The magic keeps the old textual convention, so a pre-2.4.0 build sees a
   header it does not recognise and declines the file cleanly. */

#define SL_V2_MAGIC "ZSNES Save State File V200\x1a\x8f"
#define SL_V2_MAGIC_LEN 28u /* sizeof(SL_V2_MAGIC) - 1 */
#define SL_V2_VERSION 200u
#define SL_V2_FLAG_LE 1u /* body scalars are stored little-endian */

enum sl_format { SL_FMT_UNKNOWN,
    SL_FMT_V2,
    SL_FMT_LEGACY };

typedef struct {
    u4 format_version; /* SL_V2_VERSION */
    u4 flags; /* SL_V2_FLAG_* */
    u4 feature_mask; /* cartridge/chip fingerprint, a wrong-file guard */
    u4 body_len; /* uncompressed body length in bytes */
    u4 body_crc32; /* IEEE CRC32 over the uncompressed body */
} sl_v2_header;

/* IEEE CRC32 (zlib-compatible) over len bytes of buf. */
u4 sl_crc32(void const* buf, size_t len);

/* Whether the file at its current offset opens with the V2 magic; the offset
   is left unchanged. A file that is not V2 is reported SL_FMT_LEGACY, to be
   handed to the older loader. */
enum sl_format sl_detect(FILE* fp);

/* Write / read the magic and the 20-byte descriptor that follows it. The
   descriptor is little-endian on disk regardless of host. Both leave the file
   positioned at the body. Read returns false, and zeroes *out, when the magic,
   version, or byte order is not one this build loads. */
bool sl_v2_write_header(FILE* fp, u4 feature_mask, u4 body_len, u4 body_crc32);
bool sl_v2_read_header(FILE* fp, sl_v2_header* out);

#endif
