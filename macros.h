#ifndef MACROS_H
#define MACROS_H

#define STATIC_ASSERT(x) extern int STATIC_ASSERT[((x) != 0) * 2 - 1]

#define lengthof(x) (sizeof(x) / sizeof *(x))
#define endof(x) ((x) + lengthof(x))

#define ROL(x, n) ((x) << (n) | (x) >> (sizeof(x) * 8 - (n)))

#endif
