# ministring.h - A simple C-compatible dynamic string library
This library is inspired from the [sds](https://github.com/antirez/sds) library by @antirez

## Basic example
```c
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
```

## How to use the library in your own project

The only file you need from here is [ministring.h](https://github.com/Nickys0/ministring.h/blob/main/ministring.h). Just copy-paste it to your project and start using it

Aftery copy-paste just include "ministring.h" in your project and define MS_IMPLEMENTATION to include the implementation.

## Configuration

The library lets you define some MACROS before including the library to change its behaviour:

You can define this to use different allocators/assertions which are compatible with the libc related functions
```c
#define MS_ASSERT(expr) // (default to libc's assert macro)
#define MS_MALLOC(nbytes) // (default to libc's malloc function)
#define MS_REALLOC(old_ptr, new_nbytes) // (default to libc's realloc function)
#define MS_FREE(ptr) // (default to libc's free function)
```

To complately disable certain aspect of the library
```c
#define MS_NO_ASSERT          // If defined disables default assertions
#define MS_NO_DEFAULT_ZEROED  // If defined disables the defualt memset to 0 when allocating a new mstring
```

## How to run tests
To run tests under ./tests just compile run_tests like this:
```bash
gcc -o tester run_tests.c -Wall -Wextra
```
and then run it:
```bash
./tester
```
(If you are on Windows you may need to use a different compiler)

## Compatibility 
The library is tested on Linux and it should work also on Windows but it is not tested yet.