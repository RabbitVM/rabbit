#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "rabbit_codewords.h"
#include "rabbit_io.h"

struct instr_s {
    const char *name;
    unsigned nargs;
};

static int NUM_INSTRS = 16;
static struct instr_s ops[] = {
    {"halt", 0},
    {"move", 2},
    {"add",  3},
    {"sub",  3},
    {"mul",  3},
    {"div",  3},
    {"shr",  3},
    {"shl",  3},
    {"nand", 3},
    {"xor",  3},
    {"br",   1},
    {"brz",  1},
    {"brnz", 1},
    {"in",   1},
    {"out",  1},
    {"bif",  1},
};

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Need to pass file to execute.\n");
        return 1;
    }

    char *fn = argv[1];
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open `%s'.\n", fn);
        return 1;
    }

    while (1) {
        uint32_t word = 0;
        int nread = read_word(fp, &word);
        if (nread == 0) {
            fclose(fp);
            return 0;
        }
        printf("%.8x:\t", word);

        struct unpacked_s i = decode(word);
        assert(i.opcode < NUM_INSTRS);
        const char *name = ops[i.opcode].name;
        unsigned nargs = ops[i.opcode].nargs;

        uint32_t immediate = 0;
        printf("%s", name);
        if (nargs == 1 || nargs == 2 || nargs == 3) {
            putchar(' ');
            if (nargs == 2 || nargs == 3) {
                if (nargs == 3) {
                    if (i.modes.rega_deref == 1) {
                        printf("(r%d), ", i.rega);
                    }
                    else {
                        printf("r%d, ", i.rega);
                    }
                }

                if (i.modes.regb_deref == 1) {
                    printf("(r%d), ", i.regb);
                }
                else {
                    printf("r%d, ", i.regb);
                }
            }

            if (i.modes.regc_deref == 1) {
                putchar('(');
            }

            if (i.modes.immediate == 1) {
                nread = read_word(fp, &immediate);
                if (nread == 0) {
                    return 1;
                }

                printf("$%d", immediate);
            }
            else {
                printf("r%d", i.regc);
            }

            if (i.modes.regc_deref) {
                putchar(')');
            }
        }
       putchar('\n');
    }
}
