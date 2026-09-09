#ifndef _MSC_STDBOOL_H_
#define _MSC_STDBOOL_H_

#ifndef __cplusplus

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

typedef unsigned char bool;
#else
typedef bool _Bool;
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#define __bool_true_false_are_defined 1

#endif
