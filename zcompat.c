/* gz* file API on top of miniz.

   miniz implements DEFLATE and the zlib-named buffer helpers but not zlib's
   gzip file streams. ZSNES uses those for movies (.zmv), the SPC7110 graphics
   cache (.gfx), the MovieDump logo and gzip-compressed ROMs, so the small
   layer below reproduces the gz* entry points it relies on:

     gzopen gzdopen gzread gzwrite gzputc gzeof gzdirect gzclose

   Writing produces standard gzip framing (10-byte header + raw DEFLATE +
   CRC32/ISIZE trailer) so the output is readable by gzip and zlib alike.
   Reading parses the gzip header and inflates the body; input that is not in
   gzip format is passed through unchanged, matching zlib's transparent-read
   behaviour (see gzdirect and loadGZipFile). The gzip trailer is not checked
   against the data: mz_inflate already validates the DEFLATE bitstream, and
   skipping the CRC check keeps a slightly damaged movie or cache loadable
   rather than failing outright. */

#include "zcompat.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define ZC_BUF_SIZE 16384u
#define ZC_LEVEL_DEFAULT 6

struct zc_gzfile {
    int fd;
    int owns_fd; /* close fd in gzclose? */
    int writing; /* 1 = compress to fd, 0 = decompress from fd */
    int direct; /* read: input was not gzip, pass bytes through raw */
    int at_eof; /* read: decompressed input has been fully consumed */
    int failed; /* sticky error flag */
    int stream_done; /* read: inflate reported end of stream */
    int inited; /* mz_stream has been initialized */
    mz_stream strm;
    mz_ulong crc; /* write: running CRC32 of the uncompressed data */
    mz_ulong total; /* write: uncompressed byte count (low 32 bits = ISIZE) */
    unsigned char buf[ZC_BUF_SIZE];
    unsigned buf_pos; /* next unconsumed byte in buf (read path) */
    unsigned buf_len; /* valid bytes in buf (read path) */
};

/* Write exactly n bytes; returns 0 on success, -1 on error. */
static int zc_write_all(int fd, const unsigned char* p, unsigned n)
{
    unsigned done = 0u;
    while (done < n) {
        int w = (int)write(fd, p + done, n - done);
        if (w <= 0) {
            return -1;
        }
        done += (unsigned)w;
    }
    return 0;
}

/* Refill the compressed-input buffer. Returns bytes read (0 at EOF, -1 error). */
static int zc_fill(gzFile f)
{
    int n = (int)read(f->fd, f->buf, ZC_BUF_SIZE);
    if (n < 0) {
        f->failed = 1;
        return -1;
    }
    f->buf_pos = 0u;
    f->buf_len = (unsigned)n;
    return n;
}

/* Consume and return one byte from the input, or -1 at EOF/error. */
static int zc_getc(gzFile f)
{
    if (f->buf_pos >= f->buf_len) {
        int n = zc_fill(f);
        if (n <= 0) {
            return -1;
        }
    }
    return f->buf[f->buf_pos++];
}

/* Consume the variable-length gzip header (magic already peeked). 0 on ok. */
static int zc_skip_gzip_header(gzFile f)
{
    int flg;
    int i;

    for (i = 0; i < 3; i++) { /* magic (2) + CM */
        if (zc_getc(f) < 0) {
            return -1;
        }
    }
    flg = zc_getc(f);
    if (flg < 0) {
        return -1;
    }
    for (i = 0; i < 6; i++) { /* MTIME (4) + XFL + OS */
        if (zc_getc(f) < 0) {
            return -1;
        }
    }
    if (flg & 0x04) { /* FEXTRA */
        int lo = zc_getc(f);
        int hi = zc_getc(f);
        unsigned xlen;
        if (lo < 0 || hi < 0) {
            return -1;
        }
        xlen = (unsigned)lo | ((unsigned)hi << 8);
        while (xlen != 0u) {
            if (zc_getc(f) < 0) {
                return -1;
            }
            xlen--;
        }
    }
    if (flg & 0x08) { /* FNAME */
        int c;
        do {
            c = zc_getc(f);
            if (c < 0) {
                return -1;
            }
        } while (c != 0);
    }
    if (flg & 0x10) { /* FCOMMENT */
        int c;
        do {
            c = zc_getc(f);
            if (c < 0) {
                return -1;
            }
        } while (c != 0);
    }
    if (flg & 0x02) { /* FHCRC */
        if (zc_getc(f) < 0 || zc_getc(f) < 0) {
            return -1;
        }
    }
    return 0;
}

/* Release the handle and its stream. Does not touch the fd. */
static void zc_free(gzFile f)
{
    if (!f) {
        return;
    }
    if (f->inited) {
        if (f->writing) {
            mz_deflateEnd(&f->strm);
        } else {
            mz_inflateEnd(&f->strm);
        }
    }
    free(f);
}

/* Split a zlib-style mode string. Returns 0 on success. */
static int zc_parse_mode(const char* mode, int* writing, int* level)
{
    *writing = -1;
    *level = ZC_LEVEL_DEFAULT;
    for (; mode && *mode; mode++) {
        char c = *mode;
        if (c == 'r') {
            *writing = 0;
        } else if (c == 'w' || c == 'a') {
            *writing = 1;
        } else if (c >= '1' && c <= '9') {
            *level = c - '0';
        }
        /* 'b', '+', 'T' and the like are accepted and ignored. */
    }
    return (*writing >= 0) ? 0 : -1;
}

static gzFile zc_open_fd(int fd, int owns_fd, const char* mode)
{
    int writing;
    int level;
    gzFile f;

    if (fd < 0 || zc_parse_mode(mode, &writing, &level) != 0) {
        return NULL;
    }
    f = (gzFile)calloc(1, sizeof(*f));
    if (!f) {
        return NULL;
    }
    f->fd = fd;
    f->owns_fd = owns_fd;
    f->writing = writing;

    if (writing) {
        static const unsigned char hdr[10] = {
            0x1fu, 0x8bu, 0x08u, 0x00u, /* magic, CM=deflate, FLG=0 */
            0x00u, 0x00u, 0x00u, 0x00u, /* MTIME = 0 */
            0x00u, 0xffu /* XFL = 0, OS = unknown */
        };
        if (zc_write_all(f->fd, hdr, sizeof(hdr)) != 0
            || mz_deflateInit2(&f->strm, level, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS,
                   9, MZ_DEFAULT_STRATEGY)
                != MZ_OK) {
            zc_free(f);
            return NULL;
        }
        f->inited = 1;
    } else {
        int n = zc_fill(f);
        if (n < 0) {
            zc_free(f);
            return NULL;
        }
        if (f->buf_len >= 2u && f->buf[0] == 0x1fu && f->buf[1] == 0x8bu) {
            if (zc_skip_gzip_header(f) != 0
                || mz_inflateInit2(&f->strm, -MZ_DEFAULT_WINDOW_BITS) != MZ_OK) {
                zc_free(f);
                return NULL;
            }
            f->inited = 1;
        } else {
            f->direct = 1; /* not gzip: hand back the bytes verbatim */
        }
    }
    return f;
}

gzFile gzopen(const char* path, const char* mode)
{
    int writing;
    int level;
    int fd;
    int flags;
    gzFile f;

    if (zc_parse_mode(mode, &writing, &level) != 0) {
        return NULL;
    }
    flags = writing ? (O_WRONLY | O_CREAT | O_TRUNC | O_BINARY) : (O_RDONLY | O_BINARY);
    fd = open(path, flags, 0644);
    if (fd < 0) {
        return NULL;
    }
    f = zc_open_fd(fd, 1, mode);
    if (!f) {
        close(fd);
        return NULL;
    }
    return f;
}

gzFile gzdopen(int fd, const char* mode)
{
    /* The caller keeps ownership of fd (see loadGZipFile, which fcloses its
       FILE* over the same descriptor after gzclose). */
    return zc_open_fd(fd, 0, mode);
}

int gzread(gzFile f, void* buf, unsigned len)
{
    if (!f || f->writing || f->failed) {
        return -1;
    }
    if (len == 0u) {
        return 0;
    }

    if (f->direct) {
        unsigned char* dst = (unsigned char*)buf;
        unsigned produced = 0u;
        while (produced < len) {
            if (f->buf_pos < f->buf_len) {
                unsigned avail = f->buf_len - f->buf_pos;
                unsigned want = len - produced;
                unsigned take = (avail < want) ? avail : want;
                memcpy(dst + produced, f->buf + f->buf_pos, take);
                f->buf_pos += take;
                produced += take;
            } else {
                int n = (int)read(f->fd, dst + produced, len - produced);
                if (n < 0) {
                    f->failed = 1;
                    return -1;
                }
                if (n == 0) {
                    f->at_eof = 1;
                    break;
                }
                produced += (unsigned)n;
            }
        }
        return (int)produced;
    }

    f->strm.next_out = (unsigned char*)buf;
    f->strm.avail_out = len;
    while (f->strm.avail_out != 0u && !f->stream_done) {
        unsigned in0;
        int r;
        if (f->strm.avail_in == 0u) {
            if (f->buf_pos >= f->buf_len) {
                int n = zc_fill(f);
                if (n < 0) {
                    return -1;
                }
                if (n == 0) {
                    break; /* truncated input */
                }
            }
            f->strm.next_in = f->buf + f->buf_pos;
            f->strm.avail_in = f->buf_len - f->buf_pos;
        }
        in0 = f->strm.avail_in;
        r = mz_inflate(&f->strm, MZ_NO_FLUSH);
        f->buf_pos += in0 - f->strm.avail_in;
        if (r == MZ_STREAM_END) {
            f->stream_done = 1;
        } else if (r != MZ_OK && r != MZ_BUF_ERROR) {
            f->failed = 1;
            return -1;
        }
        if (r == MZ_BUF_ERROR && in0 == f->strm.avail_in && f->buf_pos < f->buf_len) {
            break; /* stalled with input left: avoid spinning */
        }
    }
    if (f->stream_done) {
        f->at_eof = 1;
    }
    return (int)(len - f->strm.avail_out);
}

int gzwrite(gzFile f, const void* buf, unsigned len)
{
    const unsigned char* src = (const unsigned char*)buf;

    if (!f || !f->writing || f->failed) {
        return 0;
    }
    if (len == 0u) {
        return 0;
    }
    f->crc = mz_crc32(f->crc, src, len);
    f->total += len;
    f->strm.next_in = src;
    f->strm.avail_in = len;
    do {
        int r;
        unsigned have;
        f->strm.next_out = f->buf;
        f->strm.avail_out = ZC_BUF_SIZE;
        r = mz_deflate(&f->strm, MZ_NO_FLUSH);
        if (r != MZ_OK && r != MZ_BUF_ERROR) {
            f->failed = 1;
            return 0;
        }
        have = ZC_BUF_SIZE - f->strm.avail_out;
        if (have != 0u && zc_write_all(f->fd, f->buf, have) != 0) {
            f->failed = 1;
            return 0;
        }
    } while (f->strm.avail_out == 0u);
    return (int)len;
}

int gzputc(gzFile f, int c)
{
    unsigned char b = (unsigned char)c;
    if (gzwrite(f, &b, 1u) != 1) {
        return -1;
    }
    return (int)b;
}

int gzeof(gzFile f)
{
    return (f && f->at_eof) ? 1 : 0;
}

int gzdirect(gzFile f)
{
    return (f && f->direct) ? 1 : 0;
}

int gzclose(gzFile f)
{
    int rc = 0;

    if (!f) {
        return MZ_STREAM_ERROR;
    }
    if (f->writing && f->inited && !f->failed) {
        int r;
        f->strm.next_in = NULL;
        f->strm.avail_in = 0u;
        do {
            unsigned have;
            f->strm.next_out = f->buf;
            f->strm.avail_out = ZC_BUF_SIZE;
            r = mz_deflate(&f->strm, MZ_FINISH);
            if (r != MZ_OK && r != MZ_STREAM_END) {
                f->failed = 1;
                break;
            }
            have = ZC_BUF_SIZE - f->strm.avail_out;
            if (have != 0u && zc_write_all(f->fd, f->buf, have) != 0) {
                f->failed = 1;
                break;
            }
        } while (r != MZ_STREAM_END);

        if (!f->failed) {
            mz_uint32 crc = (mz_uint32)f->crc;
            mz_uint32 isize = (mz_uint32)(f->total & 0xffffffffu);
            unsigned char trailer[8];
            trailer[0] = (unsigned char)(crc & 0xffu);
            trailer[1] = (unsigned char)((crc >> 8) & 0xffu);
            trailer[2] = (unsigned char)((crc >> 16) & 0xffu);
            trailer[3] = (unsigned char)((crc >> 24) & 0xffu);
            trailer[4] = (unsigned char)(isize & 0xffu);
            trailer[5] = (unsigned char)((isize >> 8) & 0xffu);
            trailer[6] = (unsigned char)((isize >> 16) & 0xffu);
            trailer[7] = (unsigned char)((isize >> 24) & 0xffu);
            if (zc_write_all(f->fd, trailer, sizeof(trailer)) != 0) {
                f->failed = 1;
            }
        }
    }

    if (f->failed) {
        rc = MZ_ERRNO;
    }
    if (f->owns_fd && f->fd >= 0) {
        close(f->fd);
    }
    zc_free(f);
    return rc;
}
