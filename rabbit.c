
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef uint32_t rabbitw;

{{{declarations}}}

int main(int argc, char **argv) {
    {{{check_cli_arguments}}}

    {{{open_code_file}}}

    {{{stat_code_file}}}

    {{{allocate_memory}}}

    {{{read_code_into_memory}}}

    {{{fetch_decode_execute_loop}}}

    return 0;
}

#undef fetch_immediate
