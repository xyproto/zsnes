#ifndef MACROS_H
#define MACROS_H

#define lengthof(x) (sizeof(x) / sizeof *(x))
#define endof(x)    ((x) + lengthof(x))

#define ROL(x, n) ((x) << (n) | (x) >> (sizeof(x) * 8 - (n)))

#endif
