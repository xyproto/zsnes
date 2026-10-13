#include "guiarena.h"

#include <stdlib.h>
#include <string.h>

enum { ARENA_BLOCK = 64 * 1024 };

typedef struct ArenaBlock {
    struct ArenaBlock* next;
    size_t size;
    size_t used;
    /* The payload follows, aligned by the union below. */
} ArenaBlock;

/* What the payload has to start on. */
typedef union {
    long double ld;
    void* p;
    long long ll;
} ArenaAlign;

enum { ARENA_HEAD = (sizeof(ArenaBlock) + sizeof(ArenaAlign) - 1)
        / sizeof(ArenaAlign) * sizeof(ArenaAlign) };

static ArenaBlock* arena_head = NULL;
static ArenaBlock* arena_cur = NULL;

static unsigned char* block_at(ArenaBlock* const b, size_t const off)
{
    return (unsigned char*)b + ARENA_HEAD + off;
}

static ArenaBlock* new_block(size_t const want)
{
    size_t const size = want > ARENA_BLOCK ? want : ARENA_BLOCK;
    ArenaBlock* const b = (ArenaBlock*)malloc(ARENA_HEAD + size);

    if (!b) {
        return NULL;
    }
    b->next = NULL;
    b->size = size;
    b->used = 0;
    return b;
}

void* GUIAlloc(size_t const bytes)
{
    size_t const n = (bytes + sizeof(ArenaAlign) - 1) / sizeof(ArenaAlign)
        * sizeof(ArenaAlign);
    void* out;

    if (n == 0) {
        return NULL;
    }
    while (arena_cur && arena_cur->used + n > arena_cur->size) {
        if (!arena_cur->next) {
            arena_cur->next = new_block(n);
            if (!arena_cur->next) {
                return NULL;
            }
        }
        arena_cur = arena_cur->next;
    }
    if (!arena_cur) {
        arena_head = arena_cur = new_block(n);
        if (!arena_cur) {
            return NULL;
        }
    }
    out = block_at(arena_cur, arena_cur->used);
    arena_cur->used += n;
    memset(out, 0, n);
    return out;
}

char* GUIStrdup(char const* const s)
{
    size_t const n = strlen(s) + 1;
    char* const out = (char*)GUIAlloc(n);

    if (out) {
        memcpy(out, s, n);
    }
    return out;
}

void GUIArenaReset(void)
{
    ArenaBlock* b;

    for (b = arena_head; b; b = b->next) {
        b->used = 0;
    }
    arena_cur = arena_head;
}

void GUIArenaFree(void)
{
    ArenaBlock* b = arena_head;

    while (b) {
        ArenaBlock* const next = b->next;

        free(b);
        b = next;
    }
    arena_head = arena_cur = NULL;
}
