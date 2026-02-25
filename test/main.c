/*
 * ZSNES2 headless test suite
 *
 * Tests pure C functions that have no dependency on graphics, audio, or
 * emulator state.  Compiled as plain 32-bit C; no SDL, no zlib needed here.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "zstest.h"

/* ── forward-declare only what we need from the project (no heavy headers) ── */

/* test_endmem.c / test_init.c */
void test_endmem(void);
void test_init(void);

/* zpath.c – pure string utilities */
void natify_slashes(char* str);
char* strcutslash(char* str);
char* strcatslash(char* str);
void setextension(char* base, const char* ext);
bool isextension(const char* fname, const char* ext);
void strdirname(char* str);
void strbasename(char* str);
char* strdupcat(const char* str1, const char* str2);

/* ── helpers ── */

/* Return a mutable copy of a string literal. Caller must not free. */
#define BUF(lit) ({ static char _b[] = lit; char _t[sizeof _b]; \
                    memcpy(_t, _b, sizeof _b); _t; })

/* ── test functions ── */

static void test_natify_slashes(void)
{
    ZT_SECTION("natify_slashes");

    {
        char s[] = "a\\b\\c";
        natify_slashes(s);
        ZT_CHECK_STR(s, "a/b/c");
    }
    {
        char s[] = "a/b/c";
        natify_slashes(s);
        ZT_CHECK_STR(s, "a/b/c");
    }
    {
        char s[] = "";
        natify_slashes(s);
        ZT_CHECK_STR(s, "");
    }
    {
        char s[] = "\\";
        natify_slashes(s);
        ZT_CHECK_STR(s, "/");
    }
    {
        char s[] = "foo\\bar\\baz";
        natify_slashes(s);
        ZT_CHECK_STR(s, "foo/bar/baz");
    }
    {
        char s[] = "a\\b/c\\d";
        natify_slashes(s);
        ZT_CHECK_STR(s, "a/b/c/d");
    }
}

static void test_strcutslash(void)
{
    ZT_SECTION("strcutslash");

    {
        char s[] = "/foo/bar/";
        strcutslash(s);
        ZT_CHECK_STR(s, "/foo/bar");
    }
    {
        char s[] = "/foo/bar";
        strcutslash(s);
        ZT_CHECK_STR(s, "/foo/bar");
    }
    {
        char s[] = "/";
        strcutslash(s);
        ZT_CHECK_STR(s, "");
    }
    {
        char s[] = "foo/";
        strcutslash(s);
        ZT_CHECK_STR(s, "foo");
    }
    {
        char s[] = "foo\\";
        strcutslash(s);
        ZT_CHECK_STR(s, "foo");
    }
}

static void test_strcatslash(void)
{
    ZT_SECTION("strcatslash");

    /* needs buffer large enough to append the slash */
    {
        char s[32] = "/foo/bar";
        strcatslash(s);
        ZT_CHECK_STR(s, "/foo/bar/");
    }
    {
        char s[32] = "/foo/bar/";
        strcatslash(s);
        ZT_CHECK_STR(s, "/foo/bar/");
    }
    {
        char s[32] = "foo";
        strcatslash(s);
        ZT_CHECK_STR(s, "foo/");
    }
    /* backslash input should be normalised then slash appended */
    {
        char s[32] = "foo\\bar";
        strcatslash(s);
        ZT_CHECK_STR(s, "foo/bar/");
    }
}

static void test_setextension(void)
{
    ZT_SECTION("setextension");

    {
        char s[32] = "file.txt";
        setextension(s, "c");
        ZT_CHECK_STR(s, "file.c");
    }
    {
        char s[32] = "file.txt";
        setextension(s, "zsn");
        ZT_CHECK_STR(s, "file.zsn");
    }
    {
        char s[32] = "file";
        setextension(s, "txt");
        ZT_CHECK_STR(s, "file.txt");
    }
    {
        char s[32] = "a.b.c";
        setextension(s, "d");
        ZT_CHECK_STR(s, "a.b.d");
    }
    {
        char s[32] = "ROM";
        setextension(s, "sfc");
        ZT_CHECK_STR(s, "ROM.sfc");
    }
    {
        char s[32] = "save.zst";
        setextension(s, "zst");
        ZT_CHECK_STR(s, "save.zst");
    }
}

static void test_isextension(void)
{
    ZT_SECTION("isextension");

    ZT_CHECK(isextension("file.txt", "txt"));
    ZT_CHECK(isextension("file.TXT", "txt")); /* case-insensitive */
    ZT_CHECK(isextension("file.txt", "TXT")); /* case-insensitive */
    ZT_CHECK(isextension("ROM.sfc", "sfc"));
    ZT_CHECK(isextension("ROM.SFC", "sfc"));
    ZT_CHECK(!isextension("file.txt", "c"));
    ZT_CHECK(!isextension("file.txt", "txtt")); /* longer ext */
    ZT_CHECK(!isextension("filetxt", "txt")); /* no dot     */
}

static void test_strdirname(void)
{
    ZT_SECTION("strdirname");

    {
        char s[64] = "/foo/bar/baz";
        strdirname(s);
        ZT_CHECK_STR(s, "/foo/bar");
    }
    {
        char s[64] = "/foo/bar/baz/";
        strdirname(s);
        ZT_CHECK_STR(s, "/foo/bar");
    }
    {
        char s[64] = "/foo";
        strdirname(s);
        ZT_CHECK_STR(s, "/");
    }
    {
        char s[64] = "/";
        strdirname(s);
        ZT_CHECK_STR(s, "/");
    }
    /* no slash – element with no directory component is left unchanged */
    {
        char s[64] = "baz";
        strdirname(s);
        ZT_CHECK_STR(s, "baz");
    }
    /* Windows-style separators normalised first */
    {
        char s[64] = "foo\\bar\\baz";
        strdirname(s);
        ZT_CHECK_STR(s, "foo/bar");
    }
}

static void test_strbasename(void)
{
    ZT_SECTION("strbasename");

    {
        char s[64] = "/foo/bar/baz";
        strbasename(s);
        ZT_CHECK_STR(s, "baz");
    }
    {
        char s[64] = "/baz";
        strbasename(s);
        ZT_CHECK_STR(s, "baz");
    }
    {
        char s[64] = "baz";
        strbasename(s);
        ZT_CHECK_STR(s, "baz");
    }
    {
        char s[64] = "foo\\bar";
        strbasename(s);
        ZT_CHECK_STR(s, "bar");
    }
}

static void test_strdupcat(void)
{
    ZT_SECTION("strdupcat");

    char* r;

    r = strdupcat("hello", "world");
    ZT_CHECK(r != NULL);
    ZT_CHECK_STR(r, "helloworld");
    free(r);
    r = strdupcat("", "abc");
    ZT_CHECK(r != NULL);
    ZT_CHECK_STR(r, "abc");
    free(r);
    r = strdupcat("abc", "");
    ZT_CHECK(r != NULL);
    ZT_CHECK_STR(r, "abc");
    free(r);
    r = strdupcat("", "");
    ZT_CHECK(r != NULL);
    ZT_CHECK_STR(r, "");
    free(r);
    r = strdupcat("/foo/", "bar");
    ZT_CHECK(r != NULL);
    ZT_CHECK_STR(r, "/foo/bar");
    free(r);
    /* result length */
    r = strdupcat("ab", "cd");
    ZT_CHECK(r && strlen(r) == 4);
    free(r);
}

/* ── entry point ── */

int main(void)
{
    printf("ZSNES2 headless tests\n");
    printf("─────────────────────\n");

    test_natify_slashes();
    test_strcutslash();
    test_strcatslash();
    test_setextension();
    test_isextension();
    test_strdirname();
    test_strbasename();
    test_strdupcat();

    test_endmem();
    test_init();

    ZT_RESULTS();
}
