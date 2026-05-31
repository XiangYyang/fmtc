// SPDX-License-Identifier: Apache-2.0
#include <stdio.h>
#include <stdlib.h>
#include "fmt.h"

#define BUFF_SZ 64

#define demo_section(name) \
    do {                   \
        puts("");          \
    } while (0)

#define example_block(stat)   \
    do {                      \
        size_t len = BUFF_SZ; \
        stat;                 \
    } while (0)

int main(void)
{
    char buff[BUFF_SZ];
    size_t len;

    demo_section("simple");

    // step1. set len to max_buff_len
    len = BUFF_SZ;

    // step2. fmt
    fmt_error_t err = fmt_tobuff("{}", buff, &len, 42);

    // step3. check error
    if (err == FmtError_Ok) {
        printf("fmt(\"{}\", 42)        -> %s\n", buff);
    } else {
        printf("fmt(\"{}\", 42)        -> error [%d]\n", (int)err);
    }

    // integer fmt
    demo_section("integer");

    example_block({
        fmt_tobuff("{:d}", buff, &len, 42);
        printf("fmt(\"{:d}\", 42)      -> %s\n", buff);
    });

    example_block({
        fmt_tobuff("{:h}", buff, &len, 42);
        printf("fmt(\"{:h}\", 42)      -> %s\n", buff);
    });

    example_block({
        fmt_tobuff("{:x}", buff, &len, 42);
        printf("fmt(\"{:x}\", 42)      -> %s\n", buff);
    });

    // align
    demo_section("align");

    example_block({
        fmt_tobuff("{:>8d}", buff, &len, 42);
        printf("fmt(\"{:>8d}\", 42)    -> |%s|\n", buff);
    });

    example_block({
        fmt_tobuff("{:<8d}", buff, &len, 42);
        printf("fmt(\"{:<8d}\", 42)    -> |%s|\n", buff);
    });

    example_block({
        fmt_tobuff("{:^8d}", buff, &len, 42);
        printf("fmt(\"{:^8d}\", 42)    -> |%s|\n", buff);
    });

    // pointer/string
    demo_section("pointer/string");

    const char* pstr = "QwQ";

    example_block({
        fmt_tobuff("{}", buff, &len, pstr);
        printf("fmt(\"{}\", \"QwQ\")     -> %s\n", buff);
    });

    example_block({
        fmt_tobuff("{:p}", buff, &len, pstr);
        printf("fmt(\"{:p}\", \"QwQ\")   -> %s\n", buff);
    });

    // order
    demo_section("arguments order");

    example_block({
        fmt_tobuff("0:{} 1:{} 1:{1} 0:{0}", buff, &len, 42, pstr);
        printf("fmt(\"0:{} 1:{} 1:{1} 0:{0}\", 42, \"QwQ\")   -> %s\n", buff);
    });

    return 0;
}
