#pragma once

#include <stdio.h>
#include <stddef.h>
#include <signal.h>

#define string_info(ms)                                                             \
        do{                                                                         \
            fprintf(stderr, "string:\n");                                           \
            fprintf(stderr, "  \\_ len:   %zu\n",   ms_len(ms));                    \
            fprintf(stderr, "  \\_ size:  %zu\n",   ms_cap(ms));                    \
            fprintf(stderr, "  \\_ avail: %zu\n",   ms_avail(ms));                  \
            fprintf(stderr, "  \\_ cont:  '%s'\n",  ms_len(ms) == 0? "EMPTY" : ms); \
        } while(0)

extern size_t sec_n;

#define SECTION(header)\
        do{                                                                             \
            fprintf(stderr, "\n");                                                      \
            fprintf(stderr, " -- SECTION(%zu) '%s' ----------\n", sec_n++, header);    \
            fprintf(stderr, "\n");                                                      \
        } while(0)
