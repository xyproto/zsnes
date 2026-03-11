/*
Copyright (C) 2005-2008 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )
Copyright (C) 2001-4 Igor Pavlov ( http://www.7-zip.org )

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef IIOSTRM_H
#define IIOSTRM_H

#include "portable.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Abstract sequential streams using function pointers.
 * Each concrete stream type provides a context struct plus the callbacks.
 */

typedef HRESULT (*InStreamReadFn)(void* ctx, void* data, UINT32 size, UINT32* processed);
typedef struct {
    InStreamReadFn read;
    void* ctx;
} InStream;

typedef HRESULT (*OutStreamWriteFn)(void* ctx, const void* data, UINT32 size, UINT32* processed);
typedef struct {
    OutStreamWriteFn write;
    void* ctx;
} OutStream;

/* ------------------------------------------------------------------ */
/* Concrete input stream: read from a byte array                       */
/* ------------------------------------------------------------------ */
typedef struct {
    const char* data;
    UINT32 size;
} InStreamArrayCtx;

HRESULT InStreamArray_Read(void* ctx, void* data, UINT32 size, UINT32* processed);

/* ------------------------------------------------------------------ */
/* Concrete input stream: read from a FILE                             */
/* ------------------------------------------------------------------ */
typedef struct {
    FILE* file;
} InStreamFileCtx;

HRESULT InStreamFile_Read(void* ctx, void* data, UINT32 size, UINT32* processed);

/* ------------------------------------------------------------------ */
/* Concrete output stream: write to a byte array (tracks overflow)    */
/* ------------------------------------------------------------------ */
typedef struct {
    char* data;
    UINT32 size;
    int overflow;
    UINT32 total;
} OutStreamArrayCtx;

HRESULT OutStreamArray_Write(void* ctx, const void* data, UINT32 size, UINT32* processed);

/* ------------------------------------------------------------------ */
/* Memory buffer: growable, used where stringstream was used.         */
/* Doubles as both an OutStream context (write) and a readable buffer.*/
/* ------------------------------------------------------------------ */
typedef struct {
    char* data;
    UINT32 capacity;
    UINT32 write_pos;
    UINT32 read_pos;
} MemBuf;

HRESULT MemBuf_Write(void* ctx, const void* data, UINT32 size, UINT32* processed);
int MemBuf_GetByte(MemBuf* buf, char* out); /* returns 0 on EOF, 1 on success */
UINT32 MemBuf_Read(MemBuf* buf, void* out, UINT32 size);
void MemBuf_SeekToStart(MemBuf* buf);
void MemBuf_Free(MemBuf* buf);

/* Convenience macros to fill InStream / OutStream from a context. */
#define INSTREAM_ARRAY(is, ctx_ptr)     \
    do {                                \
        (is).read = InStreamArray_Read; \
        (is).ctx = (ctx_ptr);           \
    } while (0)
#define INSTREAM_FILE(is, ctx_ptr)     \
    do {                               \
        (is).read = InStreamFile_Read; \
        (is).ctx = (ctx_ptr);          \
    } while (0)
#define OUTSTREAM_ARRAY(os, ctx_ptr)       \
    do {                                   \
        (os).write = OutStreamArray_Write; \
        (os).ctx = (ctx_ptr);              \
    } while (0)
#define OUTSTREAM_MEMBUF(os, ctx_ptr) \
    do {                              \
        (os).write = MemBuf_Write;    \
        (os).ctx = (ctx_ptr);         \
    } while (0)

#endif /* IIOSTRM_H */
