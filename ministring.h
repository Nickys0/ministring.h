/*
    ministring.h - v0.1.0 - MIT

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

        ! MS_MALLOC, MS_REALLOC, MS_FREE must be defined together!

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
    size_t size;        // Number of bytes allocated
    char   buffer[];    // The actual string
};

// @NOTE: Becouse we use macros these functions are not type safe

#define ms_get_header(ms)   (MS_ASSERT(ms != NULL), (((struct ministring_header*)ms)-1))

// Getting info of the string
#define ms_len(ms)          (ms_get_header(ms)->len)
#define ms_size(ms)         (ms_get_header(ms)->size)

// (size - len) (Is the available space of the string)
#define ms_avail(ms)        ((ms_size(ms)-1)-ms_len(ms))

// returns the last element + 1 (which should be the '\0') of the string
#define ms_end(ms)          ((ms)[ms_len(ms)])

// Sets the <s>.len = new_len only if (new_len < <s>.size)
// returns true uppon success
bool ms_set_len(mstring s, size_t new_len);
#define ms_set_len_raw(mstr, len) ms_len((mstr)) = (len)

// Creating a new ministring
mstring ms_new(void); // allocates a 16-byte string
mstring ms_new_len(size_t len); // allocates a <len> bytes string
mstring ms_init(const char* cs /* NON-NULL */, size_t len);

// dups ministring
#define ms_dup(ms)      ms_init((ms), ms_len(ms))
#define ms_dup_c(cstr)  ms_init((cstr), strlen(cstr))

// Increasing the size of the string
mstring ms_grow(mstring* ms, size_t nbytes);   // Increases <ms> with <nbytes>
mstring ms_reserve(mstring* ms, size_t nelem); // It ensures that <ms> can contain at least <nelem>

// Modifying the string
mstring ms_strcpy(mstring* ms, const char* cs, size_t n_cs); // copies n_cs element of <cs> into <ms>
#define ms_strcpy_c(ms, cs) ms_strcpy((ms), (cs), strlen(cs))

// @NOTE: should we add a space before appending <cs>?
mstring ms_strcat(mstring* ms, const char* cs, size_t n_cs); // appends n_cs element of <cs> to <ms>
#define ms_strcat_c(ms, cs) ms_strcat((ms), (cs), strlen(cs))

mstring ms_strcatf(mstring* ms, cstring format, ...) MA_PRINTF_FORMAT(2);
mstring ms_strcatvf(mstring* ms, cstring cs, va_list va);

// Reset and Clear the string
void ms_reset(mstring ms); // Sets the string len to 0
void ms_clear(mstring ms); // Sets the string len to 0 and zero initialized the entire string

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
#if 1
    mstring s = ms_new(); // empty string (16-bytes)
#else
    // which is the same as
    char* s = ms_new();
#endif

    ms_strcpy_c(&s, "Hello World");

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

#include <string.h> // memset, strncpy
#include <stdio.h>  // vsnprintf
#include <stdint.h> // uintptr_t

#define MULTIPLE_OF(bytes) (bytes + (sizeof(uintptr_t) - 1)) & ~(sizeof(uintptr_t) - 1)

// Makes enough room so that (<ms>.len + nbytes) < <ms>.size
static void ms__make_room_for(mstring* ms, size_t nbytes) {
    const size_t new_len = ms_len(*ms) + nbytes;
    if(new_len >= ms_size(*ms)) {
        const size_t new_ssize = ((ms_size(*ms) + new_len)) * 2;
        const size_t size_of = (sizeof(struct ministring_header) + new_ssize);

        struct ministring_header* h = MS_REALLOC(ms_get_header(*ms), size_of);
        MS_ASSERT(h != NULL);
        h->size = new_ssize;

        *ms = h->buffer;

        #if !defined(MS_NO_DEFAULT_ZEROED)
            memset(&ms_end(*ms), 0, ms_avail(*ms) + 1);
        #endif
    }
}

bool ms_set_len(mstring ms, size_t new_len) {
    MS_ASSERT(ms != NULL);

    if(new_len < ms_size(ms)) {
        ms_get_header(ms)->len = new_len;
        return true;
    }

    return false;
}

mstring ms_new(void) {
    return ms_new_len(16);
}

mstring ms_new_len(size_t len) {
    const size_t size_of = sizeof(struct ministring_header) + (sizeof(char) * len);
    struct ministring_header* ms = MS_MALLOC(size_of);
    MS_ASSERT(ms != NULL);

    #if !defined(MS_NO_DEFAULT_ZEROED)
        (void)memset(ms, 0, size_of);
    #else
        ms->len = 0;
    #endif

    ms->size = len;

    return ms->buffer;
}

mstring ms_init(const char* cs, size_t len) {
    MS_ASSERT(cs != NULL);

    const size_t string_size = (len);
    const size_t size_of = sizeof(struct ministring_header) + string_size + 1;
    struct ministring_header* ms = MS_MALLOC(size_of);
    MS_ASSERT(ms != NULL);

    ms->size = string_size + 1;
    ms->len =  len;

    (void)memcpy(ms->buffer, cs, len);
    ms_end(ms->buffer) = '\0'; // we ensure NULL terminating string

    return ms->buffer;
}

mstring ms_grow(mstring* ms, size_t nbytes) {
    MS_ASSERT(ms != NULL);

    const size_t size_of_string = (ms_size(*ms) + nbytes);
    const size_t new_bytes = sizeof(struct ministring_header) + size_of_string;
    struct ministring_header* h = MS_REALLOC(ms_get_header(*ms), new_bytes);
    MS_ASSERT(h != NULL);
    *ms = h->buffer;
    h->size = size_of_string;

    return *ms;
}

mstring ms_reserve(mstring* ms, size_t nelem) {
    MS_ASSERT(ms != NULL);
    ms__make_room_for(ms, sizeof(char)*nelem);
    return *ms;
}

mstring ms_strcpy(mstring* ms, const char* cs, size_t n_cs) {
    MS_ASSERT(ms);

    (void)ms__make_room_for(ms, MULTIPLE_OF(n_cs));
    // @NOTE: Should we use strncpy or memcpy?
    (void)strncpy(*ms, cs, n_cs);
    ms_len(*ms) += n_cs;
    ms_end(*ms) = '\0'; // we ensure '\0' terminating

    return *ms;
}

mstring ms_strcat(mstring* ms, const char* cs, size_t n_cs) {
    (void)ms__make_room_for(ms, MULTIPLE_OF(n_cs));
    (void) strncpy(&ms_end(*ms), cs, n_cs);
    ms_len(*ms) += n_cs;
    ms_end(*ms) = '\0'; // we ensure '\0' terminating

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

    // @TEST: Test if we can safely pass sizeof()
    char buffer[16] = {0};
    int n = vsnprintf(buffer, sizeof(buffer), cs, list);

    if(n < 0 || n > (int)(sizeof(buffer))) {
        ms__make_room_for(ms, MULTIPLE_OF((size_t)n));
        va_list l2;
        va_copy(l2, va);
        vsnprintf(&(ms_end(*ms)), ms_avail(*ms), cs, l2);
        va_end(l2);

        ms_len(*ms) += (size_t)n;
        ms_end(*ms) = '\0';
    } else {
        ms_strcat_c(ms, buffer);
    }

    va_end(list);
    return NULL;
}

void ms_reset(mstring ms) {
    ms_set_len(ms, 0);
}

void ms_clear(mstring ms){
    memset(ms, 0, ms_size(ms));
    ms_set_len(ms, 0);
}

void ms_free(mstring* ms) {
    ms_clear(*ms);
    MS_FREE(ms_get_header(*ms));
    *ms = NULL;
}
#endif

/*
    Revision history:

    0.1.0 [2026-10-28] First minor release

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
      - Breaking backward compatibility in a MINOR release should be considered a bug and
        should be promptly fixed in the next PATCH release.
*/