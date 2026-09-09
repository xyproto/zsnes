/* A .zip reader, written from the PKZIP APPNOTE.
 *
 * The shape of the format, for anyone changing this: an archive ends with an
 * "end of central directory" record naming where the central directory sits
 * and how many members it lists. The central directory holds one header per
 * member, with the offset of that member's local header. The local header is
 * what has to be read to find where the data actually begins, because its
 * name and extra fields may be sized differently from the copies in the
 * central directory - trusting the central directory's lengths is the classic
 * way to end up reading a few bytes off. */

#include "zipread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

enum { SIG_EOCD = 0x06054B50u,
    SIG_CENTRAL = 0x02014B50u,
    SIG_LOCAL = 0x04034B50u,
    CENTRAL_FIXED = 46,
    LOCAL_FIXED = 30,
    EOCD_FIXED = 22,
    /* The archive comment is a 16-bit length, so the record starts no further
       back than this. */
    EOCD_SEARCH = 22 + 65535,
    METHOD_STORED = 0,
    METHOD_DEFLATE = 8,
    ZIP_IN_CHUNK = 16384 };

typedef struct {
    char* name;
    uint32_t compressed;
    uint32_t uncompressed;
    uint32_t local_offset;
    uint16_t method;
} ZipEntry;

struct ZipFile {
    FILE* fp;
    ZipEntry* entries;
    unsigned count;
    unsigned cursor;

    /* Set between zip_open_entry and zip_close_entry. */
    int reading;
    uint32_t left_in; /* compressed bytes not yet taken off the file */
    uint32_t left_out; /* uncompressed bytes not yet handed back */
    z_stream stream;
    int inflating;
    unsigned char in[ZIP_IN_CHUNK];
};

static uint16_t rd16(unsigned char const* const p)
{
    return (uint16_t)(p[0] | (unsigned)p[1] << 8);
}

static uint32_t rd32(unsigned char const* const p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16
        | (uint32_t)p[3] << 24;
}

/* Where the end-of-central-directory record starts, or -1. */
static long find_eocd(FILE* const fp, long const size)
{
    long const window = size < (long)EOCD_SEARCH ? size : (long)EOCD_SEARCH;
    unsigned char* buf;
    long found = -1;
    long i;

    if (window < (long)EOCD_FIXED) {
        return -1;
    }
    buf = (unsigned char*)malloc((size_t)window);
    if (!buf) {
        return -1;
    }
    if (fseek(fp, size - window, SEEK_SET) != 0
        || fread(buf, 1, (size_t)window, fp) != (size_t)window) {
        free(buf);
        return -1;
    }
    /* Backwards: a comment could contain something that looks like the
       signature, and the real record is the last one. */
    for (i = window - (long)EOCD_FIXED; i >= 0; i--) {
        if (rd32(buf + i) == SIG_EOCD) {
            found = size - window + i;
            break;
        }
    }
    free(buf);
    return found;
}

static void free_entries(ZipFile* const z)
{
    unsigned i;

    for (i = 0; i < z->count; i++) {
        free(z->entries[i].name);
    }
    free(z->entries);
    z->entries = NULL;
    z->count = 0;
}

static int read_central(ZipFile* const z, uint32_t const offset,
    uint32_t const size, unsigned const expect)
{
    unsigned char* dir = (unsigned char*)malloc(size ? size : 1);
    unsigned pos = 0;
    unsigned n = 0;

    if (!dir) {
        return 0;
    }
    if (fseek(z->fp, (long)offset, SEEK_SET) != 0
        || fread(dir, 1, size, z->fp) != size) {
        free(dir);
        return 0;
    }
    z->entries = (ZipEntry*)calloc(expect ? expect : 1, sizeof(ZipEntry));
    if (!z->entries) {
        free(dir);
        return 0;
    }
    while (n < expect && pos + CENTRAL_FIXED <= size) {
        unsigned char const* const h = dir + pos;
        uint16_t name_len, extra_len, comment_len;
        ZipEntry* e;

        if (rd32(h) != SIG_CENTRAL) {
            break;
        }
        name_len = rd16(h + 28);
        extra_len = rd16(h + 30);
        comment_len = rd16(h + 32);
        if (pos + CENTRAL_FIXED + name_len + extra_len + comment_len > size) {
            break;
        }
        e = &z->entries[n];
        e->method = rd16(h + 10);
        e->compressed = rd32(h + 20);
        e->uncompressed = rd32(h + 24);
        e->local_offset = rd32(h + 42);
        e->name = (char*)malloc((size_t)name_len + 1);
        if (!e->name) {
            break;
        }
        memcpy(e->name, h + CENTRAL_FIXED, name_len);
        e->name[name_len] = '\0';
        n++;
        pos += (unsigned)CENTRAL_FIXED + name_len + extra_len + comment_len;
    }
    free(dir);
    z->count = n;
    return n > 0;
}

ZipFile* zip_open(char const* const path)
{
    ZipFile* z;
    unsigned char eocd[EOCD_FIXED];
    long size, at;
    uint32_t dir_offset, dir_size;
    unsigned entries;

    z = (ZipFile*)calloc(1, sizeof(*z));
    if (!z) {
        return NULL;
    }
    z->fp = fopen(path, "rb");
    if (!z->fp) {
        free(z);
        return NULL;
    }
    if (fseek(z->fp, 0, SEEK_END) != 0 || (size = ftell(z->fp)) < 0) {
        zip_close(z);
        return NULL;
    }
    at = find_eocd(z->fp, size);
    if (at < 0 || fseek(z->fp, at, SEEK_SET) != 0
        || fread(eocd, 1, sizeof(eocd), z->fp) != sizeof(eocd)) {
        zip_close(z);
        return NULL;
    }
    entries = rd16(eocd + 10);
    dir_size = rd32(eocd + 12);
    dir_offset = rd32(eocd + 16);
    /* ZIP64 puts sentinels here and the real values in an extra record. Say
       no rather than read the archive wrongly. */
    if (entries == 0xFFFFu || dir_size == 0xFFFFFFFFu
        || dir_offset == 0xFFFFFFFFu) {
        zip_close(z);
        return NULL;
    }
    if (!read_central(z, dir_offset, dir_size, entries)) {
        zip_close(z);
        return NULL;
    }
    z->cursor = 0;
    return z;
}

void zip_close(ZipFile* const z)
{
    if (!z) {
        return;
    }
    zip_close_entry(z);
    free_entries(z);
    if (z->fp) {
        fclose(z->fp);
    }
    free(z);
}

int zip_first(ZipFile* const z)
{
    if (!z || z->count == 0) {
        return ZIP_END;
    }
    z->cursor = 0;
    return ZIP_OK;
}

int zip_next(ZipFile* const z)
{
    if (!z || z->cursor + 1 >= z->count) {
        return ZIP_END;
    }
    z->cursor++;
    return ZIP_OK;
}

static int name_eq(char const* a, char const* b, int const case_sensitive)
{
    if (case_sensitive) {
        return strcmp(a, b) == 0;
    }
    for (; *a && *b; a++, b++) {
        int const ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
        int const cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;

        if (ca != cb) {
            return 0;
        }
    }
    return *a == *b;
}

int zip_locate(ZipFile* const z, char const* const name, int const case_sensitive)
{
    unsigned i;

    if (!z || !name) {
        return ZIP_ERR;
    }
    for (i = 0; i < z->count; i++) {
        if (name_eq(z->entries[i].name, name, case_sensitive)) {
            z->cursor = i;
            return ZIP_OK;
        }
    }
    return ZIP_END;
}

int zip_entry(ZipFile* const z, char* const name, size_t const name_size,
    uint32_t* const uncompressed)
{
    ZipEntry const* e;

    if (!z || z->cursor >= z->count) {
        return ZIP_ERR;
    }
    e = &z->entries[z->cursor];
    if (name && name_size) {
        size_t const len = strlen(e->name);
        size_t const fit = len < name_size - 1 ? len : name_size - 1;

        memcpy(name, e->name, fit);
        name[fit] = '\0';
    }
    if (uncompressed) {
        *uncompressed = e->uncompressed;
    }
    return ZIP_OK;
}

int zip_open_entry(ZipFile* const z)
{
    unsigned char local[LOCAL_FIXED];
    ZipEntry const* e;
    long data_at;

    if (!z || z->cursor >= z->count) {
        return ZIP_ERR;
    }
    zip_close_entry(z);
    e = &z->entries[z->cursor];
    if (e->method != METHOD_STORED && e->method != METHOD_DEFLATE) {
        return ZIP_ERR;
    }
    if (fseek(z->fp, (long)e->local_offset, SEEK_SET) != 0
        || fread(local, 1, sizeof(local), z->fp) != sizeof(local)
        || rd32(local) != SIG_LOCAL) {
        return ZIP_ERR;
    }
    /* The local header's own name and extra lengths, not the ones in the
       central directory: they are allowed to differ. */
    data_at = (long)e->local_offset + LOCAL_FIXED + rd16(local + 26)
        + rd16(local + 28);
    if (fseek(z->fp, data_at, SEEK_SET) != 0) {
        return ZIP_ERR;
    }
    z->left_in = e->compressed;
    z->left_out = e->uncompressed;
    if (e->method == METHOD_DEFLATE) {
        memset(&z->stream, 0, sizeof(z->stream));
        /* Negative window bits: the member is a raw deflate stream with no
           zlib wrapper around it. */
        if (inflateInit2(&z->stream, -MAX_WBITS) != Z_OK) {
            return ZIP_ERR;
        }
        z->inflating = 1;
    }
    z->reading = 1;
    return ZIP_OK;
}

void zip_close_entry(ZipFile* const z)
{
    if (!z) {
        return;
    }
    if (z->inflating) {
        inflateEnd(&z->stream);
        z->inflating = 0;
    }
    z->reading = 0;
    z->left_in = 0;
    z->left_out = 0;
}

long zip_read(ZipFile* const z, void* const buf, size_t const len)
{
    if (!z || !z->reading || !buf) {
        return -1;
    }
    if (len == 0 || z->left_out == 0) {
        return 0;
    }

    if (!z->inflating) { /* stored */
        size_t want = len < z->left_out ? len : z->left_out;
        size_t got = fread(buf, 1, want, z->fp);

        z->left_out -= (uint32_t)got;
        if (z->left_in >= got) {
            z->left_in -= (uint32_t)got;
        }
        return (long)got;
    }

    z->stream.next_out = (Bytef*)buf;
    z->stream.avail_out = (uInt)(len < z->left_out ? len : z->left_out);
    while (z->stream.avail_out != 0) {
        int rc;

        if (z->stream.avail_in == 0 && z->left_in != 0) {
            size_t const want = z->left_in < ZIP_IN_CHUNK ? z->left_in : ZIP_IN_CHUNK;
            size_t const got = fread(z->in, 1, want, z->fp);

            if (got == 0) {
                break;
            }
            z->left_in -= (uint32_t)got;
            z->stream.next_in = z->in;
            z->stream.avail_in = (uInt)got;
        }
        rc = inflate(&z->stream, Z_NO_FLUSH);
        if (rc == Z_STREAM_END) {
            break;
        }
        if (rc != Z_OK) {
            return -1;
        }
        if (z->stream.avail_in == 0 && z->left_in == 0) {
            break; /* nothing left to feed it */
        }
    }
    {
        long const produced = (long)((unsigned char*)z->stream.next_out
            - (unsigned char*)buf);

        z->left_out -= (uint32_t)produced;
        return produced;
    }
}
