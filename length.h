#ifndef C_LENGTH_H
#define C_LENGTH_H

#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif

#ifndef endof
#define endof(x) ((x) + lengthof(x))
#endif

#endif
