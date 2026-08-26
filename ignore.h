/* Deliberately ignoring a result the compiler wants checked.
 *
 * The call still happens; this only records that nothing here acts on a short
 * read or a failed write. Most of these are loaders that validate the data
 * afterwards, or best-effort writes to a pipe. Grep for IGNORE_RESULT to find
 * the places that should grow real error handling.
 */
#ifndef IGNORE_H
#define IGNORE_H

#define IGNORE_RESULT(call) \
    do {                    \
        if (call) { }       \
    } while (0)

#endif
