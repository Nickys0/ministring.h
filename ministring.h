/*
 *  ministring.h - v0.2.0 - MIT

    MINISTRING LIBRARY (STB-STYLE)

    A simple C-compatible dynamic string library

    Inspired by the sds library by @antirez

    # Macro Interface

        All these macros are `#define`d by the user before including ministring.h

    ## Flags

        Enable or disable certain aspects of ministring.h

        - MA_IMPLEMENTATION     - Enable definitions of the functions. By default only declarations are included.
        - MS_NO_ASSERT          - If defined disables default assertions
        - MS_NO_DEFAULT_ZEROED  - If defined disables the defualt memset to 0 when allocating a new mstring

    ## Redefinable Macros

            Redefine default behaviors of ministring.h

        - #define MS_ASSERT         (default to libc)
        - #define MS_MALLOC         (default to libc)
        - #define MS_REALLOC        (default to libc)
        - #define MS_FREE           (default to libc)

        ! MS_MALLOC, MS_REALLOC, MS_FREE must be defined together !

   --------------------------------------------------------------------------------
   LICENSE: MIT

   Copyright (c) 2026 @Nickys0 <https://github.com/Nickys0>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#ifndef _MINISTRING_H
#define _MINISTRING_H

typedef char*       mstring;
typedef const char* cstring;

#include <stddef.h>     // size_t
#include <stdarg.h>     // va_arg
#include <stdbool.h>    // bool

#if defined(__GNUC__) || defined(__clang__)
    #define MA_PRINTF_FORMAT(arg) __attribute__((format(printf, (arg), (arg)+1)))
#else
    #define MA_PRINTF_FORMAT(arg)
#endif

#if !defined(MS_MALLOC) && !defined(MS_FREE) && !defined(MS_REALLOC)
    #include <stdlib.h>
    #define MS_MALLOC(bytes)        malloc((bytes))
    #define MS_REALLOC(old, nbytes) realloc((old), (nbytes))
    #define MS_FREE(ptr)            free((ptr))
#endif

#if !defined(MS_NO_ASSERT)
    #if !defined(MS_ASSERT)
        #include <assert.h>
        #define MS_ASSERT(expr) assert((expr))
    #endif
#else
    // We ensure that the expr is evaluated
    #define MS_ASSERT(expr) (void)(expr)
#endif // !MS_NO_ASSERT

struct ministring_header {
    size_t len;         // The lenght of the string

    // @TODO: change from 'size' to 'cap'
    size_t size;        // total number of character allocated (not counting the header itself)

    char   buffer[];    // The actual string
};

#define ms_get_header(ms)   (MS_ASSERT(ms != NULL), (((struct ministring_header*)ms)-1))

// Getting info of the string
static inline size_t ms_len(mstring ms) {  return ms_get_header(ms)->len;   }
static inline size_t ms_cap(mstring ms) {  return ms_get_header(ms)->size;  }

// (size - (len+1)) (returns how many character can be safely put before needing of reallocating)
#define ms_avail(ms)        ((MS_ASSERT(ms_cap(ms) >= (ms_len(ms)+1))), (ms_cap(ms))-(ms_len(ms)+1))

// returns the last character of the string
#define ms_last(ms)         (ms_len(ms)==0? '\0' : (ms)[ms_len(ms)-1])

// returns the last element + 1 (which should be the '\0') of the string.
// @NOTE: This can be useful in case you need to ensure that the string is '\0'
//        terminated by doing:
//        ms_end(s) = '\0'
#define ms_end(ms)          ((ms)[ms_len(ms)])

// Sets the <s>.len = new_len only if (new_len < <s>.size)
// returns true uppon success
bool ms_set_len(mstring s, size_t new_len);

// Creating a new ministring
mstring ms_new(void);               // allocates a 16 character string
mstring ms_new_len(size_t len);     // it ensures that you can append at least n'<len> characters

mstring ms_new_from(const char* cs /* NON-NULL */, size_t len);
#define ms_new_from_c(c) ms_new_from((c), (strlen(c)))
#define ms_new_from_l(l) ms_new_from((l), (sizeof(l)-1))

// dups ministring (aliases)
#define ms_dup(ms)  ms_new_from((ms), ms_len(ms))
#define ms_dup_c(c) ms_new_from_c((c))
#define ms_dup_l(l) ms_new_from_l((l))

// Increasing the size of the string
mstring ms_reserve(mstring* ms, size_t nelem); // It ensures that <ms> can contain at least <nelem>

// Modifying the string
mstring ms_strcpy(mstring* ms, const char* cs, size_t n_cs);    // copies n_cs element of <cs> into <ms>
#define ms_strcpy_c(ms, cs) ms_strcpy((ms), (cs), strlen(cs))   // copies a run-time NULL-Terminating string into <ms>
#define ms_strcpy_l(ms, l)  ms_strcpy((ms), (l), sizeof(l)-1)   // copies a compile-time string into <ms>

mstring ms_strcat(mstring* ms, const char* cs, size_t n_cs);    // appends n_cs element of <cs> to <ms>
#define ms_strcat_c(ms, cs) ms_strcat((ms), (cs), strlen(cs))   // appends a run-time NULL-Terminating strings into <ms>
#define ms_strcat_l(ms, l)  ms_strcat((ms), (l), sizeof(l)-1)   // appends a compile-time strings into <ms>

mstring ms_strcatf(mstring* ms, cstring format, ...) MA_PRINTF_FORMAT(2);
mstring ms_strcatvf(mstring* ms, cstring cs, va_list va);

// Reset and Clear the string
void ms_reset(mstring ms); // Sets the string len to 0
void ms_clear(mstring ms); // Sets the string len to 0 and zeroed the entire string

// Freeing the string
void ms_free(mstring* ms);

//////////////////////////////////////////////////
// Basic example
#if 0
#define MS_IMPLEMENTATION
#include "ministring.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    mstring s = ms_new_from_l("Hello World"); 

    // From now on you can use <s> as it was a normal char*
    for(size_t i = 0; i < ms_len(s); ++i) {
        fprintf(stderr, "s[%zu]: %c\n", i, s[i]);
    }

    // Remember to free the string after you have finished
    ms_free(&s);
    return 0;
}
#endif // Basic example

#endif // !defined _MINISTRING_H

/////////////////////////////////////////////////////////////
// IMPLEMENTATION
//
#if defined(MS_IMPLEMENTATION) && !defined(__MS_IMPLEMENTED)
#define __MS_IMPLEMENTED

#include <string.h> // memset, memcpy
#include <stdio.h>  // vsnprintf
#include <stdint.h> // uintptr_t

#define MULTIPLE_OF(bytes) (bytes + (sizeof(uintptr_t) - 1)) & ~(sizeof(uintptr_t) - 1)

#if !defined(MS_NO_DEFAULT_ZEROED)
    #define __MS_SHOULD_ZEROED true
#else
    #define __MS_SHOULD_ZEROED false
#endif

#define __MS_INTERNAL_BUFFER_SIZE 4096

#define msh__header(ms) ((struct ministring_header*)ms-1)

static struct ministring_header* ms__make_new(size_t nelem, bool zeroed) {
    const size_t req_size =          sizeof(struct ministring_header) + (nelem+1);
    struct ministring_header* msh =  MS_MALLOC(MULTIPLE_OF(req_size));
    MS_ASSERT(msh != NULL);

    if(zeroed)  (void)memset(msh, 0, req_size);
    else        msh->len = 0; // we still need to ensure ensure that len is 0

    msh->size = nelem + 1;

    return msh;
}

// Makes enough room so that (<ms>.len + nelem + 1) < <ms>.size
static void ms__make_room_for(mstring* ms /* NON-NULL */, size_t nelem, bool zeroed) {
    const size_t new_len = ms_len(*ms) + nelem + 1;

    if(new_len >= ms_cap(*ms)) {
        const size_t new_size = ((ms_len(*ms) + new_len)) * 2;
        const size_t size_of = (sizeof(struct ministring_header) + new_size);

        struct ministring_header* h = MS_REALLOC(ms_get_header(*ms), MULTIPLE_OF(size_of));
        MS_ASSERT(h != NULL);
        h->size = new_size;

        *ms = h->buffer;

        // we init all the newly allocated space
        if(zeroed) memset(&ms_end(*ms), 0, ms_avail(*ms) + 1);
    }
}

bool ms_set_len(mstring ms, size_t new_len) {
    MS_ASSERT(ms != NULL);

    if(new_len < ms_cap(ms)) {
        ms_get_header(ms)->len = new_len;
        return true;
    }

    return false;
}

mstring ms_new_len(size_t len) {
    return ms__make_new(len, __MS_SHOULD_ZEROED)->buffer;
}

mstring ms_new(void) {
    return ms_new_len(16);
}

mstring ms_new_from(const char* cs, size_t len) {
    MS_ASSERT(cs != NULL);

    struct ministring_header* msh = ms__make_new(len, false);
    (void)memcpy(msh->buffer, cs, len);

    msh->len = len;
    (msh->buffer)[msh->len] = '\0'; // we ensure NULL terminating string

    return msh->buffer;
}

mstring ms_reserve(mstring* ms, size_t nelem) {
    MS_ASSERT(ms != NULL);
    ms__make_room_for(ms, nelem, __MS_SHOULD_ZEROED);

    return *ms;
}

mstring ms_strcpy(mstring* ms, const char* cs, size_t n_cs) {
    MS_ASSERT(ms);
    MS_ASSERT(cs);

    ms__make_room_for(ms, n_cs, false);
    struct ministring_header* msh = msh__header(*ms);
    memcpy(*ms, cs, n_cs);

    msh->len += n_cs;
    msh->buffer[msh->len] = '\0'; // ensuring NULL terminating string

    return *ms;
}

mstring ms_strcat(mstring* ms, const char* cs, size_t n_cs) {
    MS_ASSERT(ms);
    MS_ASSERT(cs);

    ms__make_room_for(ms, n_cs, false);

    struct ministring_header* msh = msh__header(*ms);
    (void)memcpy(&msh->buffer[msh->len], cs, n_cs);

    msh->len += n_cs;
    msh->buffer[msh->len] = '\0'; // we ensure '\0' terminating

    return *ms;
}

mstring ms_strcatf(mstring* ms, cstring format, ...) {
    MS_ASSERT(ms != NULL);

    va_list list;
    va_start(list, format);
    mstring ret = ms_strcatvf(ms, format, list);
    va_end(list);

    return ret;
}

mstring ms_strcatvf(mstring* ms, cstring cs, va_list va) {
    MS_ASSERT(ms != NULL);

    va_list list;
    va_copy(list, va);

    char buffer[__MS_INTERNAL_BUFFER_SIZE] = {0};
    int n = vsnprintf(buffer, sizeof(buffer), cs, list);

    if(n < 0 || n > (int)(sizeof(buffer))) {
        ms__make_room_for(ms, (size_t)n, false);
        struct ministring_header* msh = msh__header(*ms);

        va_list l2;
        va_copy(l2, va);
        vsnprintf(&msh->buffer[msh->len], ms_avail(*ms), cs, l2);
        va_end(l2);

        msh->len += (size_t)n;
        msh->buffer[msh->len] = '\0'; // ensuring NULL terminating string
    } else {
        ms_strcat(ms, buffer, n);
    }

    va_end(list);
    return NULL;
}

void ms_reset(mstring ms) {
    MS_ASSERT(ms != NULL);

    msh__header(ms)->len = (0);
}

void ms_clear(mstring ms){
    MS_ASSERT(ms != NULL);

    memset(ms, 0, ms_cap(ms));
    msh__header(ms)->len = (0);
}

void ms_free(mstring* ms) {
    MS_ASSERT(ms != NULL);

    ms_clear(*ms);
    MS_FREE(msh__header(*ms));

    *ms = NULL;
}

#undef __BUFFER_SIZE
#undef msh__header

#endif

/*
    Revision history:

 !  IMPORTANT: UNTIL RELEASE 1.0.0 WE DO NOT GARANTEE BACKWARD COMPATABILITY
               AS THIS ARE STILL AN UNSTABLE API

    0.2.0   [2026-11-18] Fixes on how the size and lenght were calculated
                        (+) 'ms_strcpy_l'      (wrapper of 'ms_strcpy')
                        (+) 'ms_strcat_l'      (wrapper of 'ms_strcat')
                        (+) 'ms_new_from_c'    (wrapper of 'ms_new_from')
                        (+) 'ms_new_from_l'    (wrapper of 'ms_new_from')
                        (+) 'ms_last'
                        (>) 'ms_size' -> 'ms_cap'
                        (>) 'ms_grow' -> 'ms_reserve'
                        (-) 'ms_grow'
                        (-) 'ms_set_len_raw'

    0.1.1   [2026-11-11] Some internal changes/patches

    0.1.0   [2026-10-28] First minor release

    We are following https://semver.org/ so the version has a format MAJOR.MINOR.PATCH:
      - Modifying comments does not update the version.

      - PATCH is incremented in case of a bug fix or refactoring without touching the API.

      - MINOR is incremented when new functions and/or types are added in a way that does
        not break any existing user code. We want to do this in the majority of the situation.
        If we want to delete a certain function or type in favor of another one we should
        just add the new function/type and deprecate the old one in a backward compatible way
        and let them co-exist for a while.

      - MAJOR update should be just a periodic cleanup of the DEPRECATED functions and types
        without really modifying any existing functionality.
*/