/*
 * V2 save-state envelope tests (saveload.c)
 *
 * Covers the self-describing header the 2.4.0+ format wraps its body in:
 *   - CRC32 matches the standard IEEE value (interoperable with zlib)
 *   - write then read round-trips every descriptor field
 *   - detection leaves the file offset untouched
 *   - a foreign, truncated, or wrong-version file is rejected, not misread
 */

#include <stdio.h>
#include <string.h>

#include "../saveload.h"
#include "zstest.h"

/* A file-backed scratch stream, since the envelope I/O works on FILE*. */
static FILE* scratch(void)
{
    FILE* f = tmpfile();
    ZT_CHECK(f != 0);
    return f;
}

int main(void)
{
    printf("V2 save-state envelope tests\n");

    ZT_SECTION("crc32 known answers");
    /* The canonical CRC32 check vector. */
    ZT_CHECK(sl_crc32("123456789", 9) == 0xCBF43926u);
    ZT_CHECK(sl_crc32("", 0) == 0x00000000u);
    ZT_CHECK(sl_crc32("a", 1) == 0xE8B7BE43u);
    /* CRC is order sensitive. */
    ZT_CHECK(sl_crc32("ab", 2) != sl_crc32("ba", 2));

    ZT_SECTION("legacy version registry");
    ZT_CHECK(sl_legacy_version(SL_HDR_V144) == 144);
    ZT_CHECK(sl_legacy_version(SL_HDR_V143) == 143);
    ZT_CHECK(sl_legacy_version(SL_HDR_V06) == 60);
    /* The two trailing bytes are ignored, so a build-specific variant matches. */
    ZT_CHECK(sl_legacy_version("ZSNES Save State File V144\x00\x00") == 144);
    /* A foreign or V2 header is not a legacy version. */
    ZT_CHECK(sl_legacy_version(SL_V2_MAGIC) == 0);
    ZT_CHECK(sl_legacy_version("ZSNES Save State File V999\x1a\x8f") == 0);

    ZT_SECTION("header round-trip");
    {
        FILE* f = scratch();
        sl_v2_header h;

        ZT_CHECK(sl_v2_write_header(f, 0x0000ABCDu, 275325u, 0xDEADBEEFu));
        rewind(f);
        ZT_CHECK(sl_v2_read_header(f, &h));
        ZT_CHECK(h.format_version == SL_V2_VERSION);
        ZT_CHECK((h.flags & SL_V2_FLAG_LE) != 0);
        ZT_CHECK(h.feature_mask == 0x0000ABCDu);
        ZT_CHECK(h.body_len == 275325u);
        ZT_CHECK(h.body_crc32 == 0xDEADBEEFu);
        /* The read leaves the stream at the body: magic + five u4. */
        ZT_CHECK(ftell(f) == (long)(SL_V2_MAGIC_LEN + 5u * 4u));
        fclose(f);
    }

    ZT_SECTION("on-disk layout is little-endian");
    {
        FILE* f = scratch();
        unsigned char raw[SL_V2_MAGIC_LEN + 20];

        ZT_CHECK(sl_v2_write_header(f, 0x04030201u, 0x08070605u, 0x0C0B0A09u));
        rewind(f);
        ZT_CHECK(fread(raw, 1, sizeof(raw), f) == sizeof(raw));
        ZT_CHECK(memcmp(raw, SL_V2_MAGIC, SL_V2_MAGIC_LEN) == 0);
        /* format_version = 200 = 0xC8, then flags = 1. */
        ZT_CHECK(raw[SL_V2_MAGIC_LEN] == 0xC8 && raw[SL_V2_MAGIC_LEN + 1] == 0);
        /* feature_mask 0x04030201 stored low byte first. */
        ZT_CHECK(raw[SL_V2_MAGIC_LEN + 8] == 0x01
            && raw[SL_V2_MAGIC_LEN + 9] == 0x02
            && raw[SL_V2_MAGIC_LEN + 10] == 0x03
            && raw[SL_V2_MAGIC_LEN + 11] == 0x04);
        fclose(f);
    }

    ZT_SECTION("detect leaves the offset untouched");
    {
        FILE* f = scratch();

        ZT_CHECK(sl_v2_write_header(f, 0, 1, 2));
        rewind(f);
        ZT_CHECK(sl_detect(f) == SL_FMT_V2);
        ZT_CHECK(ftell(f) == 0);
        /* From a non-zero offset it still restores that offset. */
        ZT_CHECK(fseek(f, 3, SEEK_SET) == 0);
        ZT_CHECK(sl_detect(f) == SL_FMT_LEGACY); /* no magic at offset 3 */
        ZT_CHECK(ftell(f) == 3);
        fclose(f);
    }

    ZT_SECTION("foreign and broken files are rejected");
    {
        FILE* f = scratch();
        sl_v2_header h;

        /* A pre-2.4.0 header is not V2. */
        fputs("ZSNES Save State File V144\x1a\x8f", f);
        rewind(f);
        ZT_CHECK(sl_detect(f) == SL_FMT_LEGACY);
        rewind(f);
        ZT_CHECK(!sl_v2_read_header(f, &h));
        fclose(f);

        /* Truncated after the magic: the descriptor cannot be read. */
        f = scratch();
        ZT_CHECK(fwrite(SL_V2_MAGIC, 1, SL_V2_MAGIC_LEN, f) == SL_V2_MAGIC_LEN);
        ZT_CHECK(fwrite("\xC8\x00\x00", 1, 3, f) == 3); /* partial version */
        rewind(f);
        ZT_CHECK(!sl_v2_read_header(f, &h));
        ZT_CHECK(h.body_len == 0); /* zeroed on failure */
        fclose(f);

        /* Right magic, wrong version. */
        f = scratch();
        ZT_CHECK(fwrite(SL_V2_MAGIC, 1, SL_V2_MAGIC_LEN, f) == SL_V2_MAGIC_LEN);
        {
            unsigned char ver[20] = { 0 };
            ver[0] = 0xC9; /* 201, not 200 */
            ver[4] = SL_V2_FLAG_LE;
            ZT_CHECK(fwrite(ver, 1, sizeof(ver), f) == sizeof(ver));
        }
        rewind(f);
        ZT_CHECK(!sl_v2_read_header(f, &h));
        fclose(f);
    }

    ZT_RESULTS();
}
