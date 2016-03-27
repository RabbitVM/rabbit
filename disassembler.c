#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

struct instr_s {
    const char *name;
    unsigned nargs;
};

static int NUM_INSTRS = 15;
static struct instr_s ops[] = {
    {"halt", 0},
    {"move", 3},
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
};

struct modes_s {
    uint8_t immediate : 1;
    uint8_t regc_deref : 1;
    uint8_t regb_deref : 1;
    uint8_t rega_deref : 1;
};

struct unpacked_s {
    struct modes_s modes;
    uint8_t opcode : 4;
    uint8_t regc : 4;
    uint8_t regb : 4;
    uint8_t rega : 4;
};

struct unpacked_s decode(uint32_t instr) {
    static const unsigned char RB_ADDRA = 1U << 0,
        RB_ADDRB = 1U << 1,
        RB_ADDRC = 1U << 2,
        RB_IMMED = 1U << 3;
    uint8_t modes = (instr >> 24) & 0xF;
    struct unpacked_s instr_unpacked = {
        .modes = {
            .immediate = modes & RB_IMMED,
            .regc_deref = modes & RB_ADDRC,
            .regb_deref = modes & RB_ADDRB,
            .rega_deref = modes & RB_ADDRA,
        },
        .opcode = instr >> 28,
        .regc = (instr >> 8) & 0xF,
        .regb = (instr >> 4) & 0xF,
        .rega = instr & 0xF,
    };
    return instr_unpacked;
}

int main() {
    FILE *fp = stdin;
    while (1) {
        uint32_t word = 0;
        size_t nread = fread(&word, sizeof(word), 1, fp);
        if (nread == 0) {
            return 0;
        }
        printf("%.8x:\t", word);

        struct unpacked_s i = decode(word);
        assert(i.opcode < NUM_INSTRS);
        const char *name = ops[i.opcode].name;
        unsigned nargs = ops[i.opcode].nargs;

        uint32_t immediate = 0;
        printf("%s", name);
        switch(nargs) {
        case 0:
            break;
        case 1:
            putchar(' ');
            if (i.modes.regc_deref) {
                putchar('(');
            }

            if (i.modes.immediate) {
                nread = fread(&immediate, sizeof(immediate), 1, fp);
                if (nread == 0) {
                    return 1;
                }

                printf("%d", immediate);
            }
            else {
                printf("r%d", i.regc);
            }

            if (i.modes.regc_deref) {
                putchar(')');
            }
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
        putchar('\n');
    }
}
