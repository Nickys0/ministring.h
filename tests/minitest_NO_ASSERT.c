/// TESTFILE: minitest_NO_ASSERT.c
///
/// Testing for ministring.h:
///   We ensure that when defining MS_NO_ASSERT everything works properly
///
/// COMMAND: cc -std=c99 -Wall -Wextra -ggdb -DMS_IMPLEMENTATION -o minitest_NO_ASSERT minitest_NO_ASSERT.c -fsanitize=address
///
#define MS_NO_ASSERT
#include "../ministring.h"

#include "common.h"

size_t sec_n = 0;

void signal_handler(int signal) {
    if(signal == SIGSEGV) {
        fprintf(stderr, "\n ----------------- Segmantation Fault ----------------- \n");
    } else if(signal == SIGABRT) {
        fprintf(stderr, "\n ----------------- ABORT ----------------- \n");
    }

    exit(1);
};

int main(void) {
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);

    mstring s1 = ms_new();
    mstring s2 = ms_new_len(31);
    mstring s3 = ms_new_from_l("Hello there!");

    SECTION("INIT");
    string_info(s1);
    string_info(s2);
    string_info(s3);

    SECTION("MODIFIERS");
    ms_strcatf(&s1, "Hello World from %s:%d", __FILE__, __LINE__);
    string_info(s1);

    ms_strcpy_c(&s2, "Hello World"); 
    string_info(s2);

    SECTION("CLEANERS");
    ms_free(&s1);
    ms_free(&s2);
    ms_free(&s3);

    return 0;
}