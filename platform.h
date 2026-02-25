/*
 * platform.h — Canonical platform detection and backend selection
 *
 * Include this header instead of testing __UNIXSDL__/__WIN32__ directly.
 * New platform ports should add their detection here and provide an
 * implementation of the c_intrf.h interface.
 *
 * Platform defines (set by the Makefile via CFGDEFS):
 *   __UNIXSDL__                 — Unix with SDL (Linux, *BSD, macOS)
 *   __WIN32__                   — Windows (MinGW/MSVC)
 *
 * Fine-grained platform defines:
 *   __ZSNES_PLATFORM_LINUX__    — Linux
 *   __ZSNES_PLATFORM_FREEBSD__  — FreeBSD
 *   __ZSNES_PLATFORM_OPENBSD__  — OpenBSD
 *   __ZSNES_PLATFORM_NETBSD__   — NetBSD
 *   __ZSNES_PLATFORM_DARWIN__   — macOS / Darwin
 *   __ZSNES_PLATFORM_WINDOWS__  — Windows
 */
#ifndef PLATFORM_H
#define PLATFORM_H

/* ── Verify exactly one major platform is selected ── */

#if defined(__UNIXSDL__) && defined(__WIN32__)
#error "Both __UNIXSDL__ and __WIN32__ are defined; pick one."
#endif

#if !defined(__UNIXSDL__) && !defined(__WIN32__)
#error "No platform defined.  Build with ARCH=linux, ARCH=win, etc."
#endif

/* ── Platform category helpers ── */

#ifdef __UNIXSDL__
#define ZSNES_PLATFORM_UNIX 1
#define ZSNES_PLATFORM_WIN  0
#else
#define ZSNES_PLATFORM_UNIX 0
#define ZSNES_PLATFORM_WIN  1
#endif

#if defined(__ZSNES_PLATFORM_FREEBSD__) || defined(__ZSNES_PLATFORM_OPENBSD__) || defined(__ZSNES_PLATFORM_NETBSD__)
#define ZSNES_PLATFORM_BSD 1
#else
#define ZSNES_PLATFORM_BSD 0
#endif

/* ── Path separator ── */

#if ZSNES_PLATFORM_WIN
#define ZSNES_PATH_SEP '\\'
#define ZSNES_PATH_SEP_STR "\\"
#else
#define ZSNES_PATH_SEP '/'
#define ZSNES_PATH_SEP_STR "/"
#endif

#endif /* PLATFORM_H */
