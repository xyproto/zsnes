#include "saveload.h"

#include <string.h>

/* IEEE CRC32, poly 0xEDB88320 - the same value zlib's crc32 computes, but with
   no zlib dependency so the loader module builds and tests on its own. Runs
   once per save or load, where a table lookup is far more than enough. */

static u4 sl_crc32_table[256];
static int sl_crc32_ready;

static void sl_crc32_init(void)
{
    u4 i;

    for (i = 0; i < 256u; i++) {
        u4 c = i;
        int k;

        for (k = 0; k < 8; k++) {
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        sl_crc32_table[i] = c;
    }
    sl_crc32_ready = 1;
}

u4 sl_crc32(void const* buf, size_t len)
{
    u1 const* const p = (u1 const*)buf;
    u4 crc = 0xFFFFFFFFu;
    size_t i;

    if (!sl_crc32_ready) {
        sl_crc32_init();
    }
    for (i = 0; i < len; i++) {
        crc = sl_crc32_table[(crc ^ p[i]) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

static bool sl_put_u4(FILE* fp, u4 v)
{
    u1 b[4];

    b[0] = (u1)(v);
    b[1] = (u1)(v >> 8);
    b[2] = (u1)(v >> 16);
    b[3] = (u1)(v >> 24);
    return fwrite(b, 1, sizeof(b), fp) == sizeof(b);
}

static bool sl_get_u4(FILE* fp, u4* v)
{
    u1 b[4];

    if (fread(b, 1, sizeof(b), fp) != sizeof(b)) {
        return false;
    }
    *v = (u4)b[0] | ((u4)b[1] << 8) | ((u4)b[2] << 16) | ((u4)b[3] << 24);
    return true;
}

int sl_legacy_version(char const header[SL_V2_MAGIC_LEN])
{
    size_t const n = SL_V2_MAGIC_LEN - 2; /* the last two bytes vary by build */

    if (!memcmp(header, SL_HDR_V144, n)) {
        return 144;
    }
    if (!memcmp(header, SL_HDR_V143, n)) {
        return 143;
    }
    if (!memcmp(header, SL_HDR_V06, n)) {
        return 60;
    }
    return 0;
}

enum sl_format sl_detect(FILE* fp)
{
    char magic[SL_V2_MAGIC_LEN];
    long const pos = ftell(fp);
    size_t got;

    if (pos < 0) {
        return SL_FMT_UNKNOWN;
    }
    got = fread(magic, 1, sizeof(magic), fp);
    if (fseek(fp, pos, SEEK_SET) != 0) {
        return SL_FMT_UNKNOWN;
    }
    if (got == sizeof(magic) && memcmp(magic, SL_V2_MAGIC, sizeof(magic)) == 0) {
        return SL_FMT_V2;
    }
    return SL_FMT_LEGACY;
}

bool sl_v2_write_header(FILE* fp, u4 feature_mask, u4 body_len, u4 body_crc32)
{
    if (fwrite(SL_V2_MAGIC, 1, SL_V2_MAGIC_LEN, fp) != SL_V2_MAGIC_LEN) {
        return false;
    }
    return sl_put_u4(fp, SL_V2_VERSION)
        && sl_put_u4(fp, SL_V2_FLAG_LE)
        && sl_put_u4(fp, feature_mask)
        && sl_put_u4(fp, body_len)
        && sl_put_u4(fp, body_crc32);
}

bool sl_v2_read_header(FILE* fp, sl_v2_header* out)
{
    char magic[SL_V2_MAGIC_LEN];

    memset(out, 0, sizeof(*out));
    if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic)
        || memcmp(magic, SL_V2_MAGIC, sizeof(magic)) != 0) {
        return false;
    }
    if (!sl_get_u4(fp, &out->format_version)
        || !sl_get_u4(fp, &out->flags)
        || !sl_get_u4(fp, &out->feature_mask)
        || !sl_get_u4(fp, &out->body_len)
        || !sl_get_u4(fp, &out->body_crc32)) {
        memset(out, 0, sizeof(*out));
        return false;
    }
    /* This build stores and reads a little-endian body; a state that does not
       announce that byte order is one for a different build to load. */
    if (out->format_version != SL_V2_VERSION || !(out->flags & SL_V2_FLAG_LE)) {
        memset(out, 0, sizeof(*out));
        return false;
    }
    return true;
}
