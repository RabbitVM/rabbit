#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
  Instruction format is as follows:

  add r0 r1 r2
  out r1

  cmov r1 r2 r3

  I am reasonably sure that blank lines are OK, and that there must be
  precisely one space between instruction and register, and register and
  register. No extra spaces ANYWHERE ELSE. Unchecked runtime error.

  MAKE SURE YOU HAVE A BLANK LINE AT THE END!!!
*/

typedef uint32_t word;

struct prog {
    word *prog;
    word len;
};

void error (char *msg, char *optional_arg);
void check_usage (int argc);
FILE *open_file (char *fn, char *mode);
word file_lines (FILE *fp);
struct prog process_file (FILE *input);
void write_file (FILE *output, struct prog asm_);

int main (int argc, char **argv) {
    check_usage(argc);

    FILE *input = open_file(argv[1], "r");
    struct prog asm_ = process_file(input);
    fclose(input);

    FILE *output = open_file(argv[2], "wb");
    write_file(output, asm_);
    fclose(output);

    free(asm_.prog);
}

void error (char *msg, char *optional_arg) {
    fprintf(stderr, "Error: ");

    if (optional_arg) {
        fprintf(stderr, msg, optional_arg);
        putchar('\n');
    }
    else {
        fprintf(stderr, "%s\n", msg);
    }

    exit(EXIT_FAILURE);
}

void check_usage (int argc) {
    if (argc != 3) {
        error("Invalid usage. Please run like: "
              "./asm <input_filename> <output_filename>",
              NULL);
    }
}

FILE *open_file (char *fn, char *mode) {
    FILE *fp = fopen(fn, mode);

    if (fp == NULL) {
        error("Could not open file `%s'.", fn);
    }

    return fp;
}

word file_lines (FILE *fp) {
    word lines = 0;

    /* Copied straight from http://stackoverflow.com/a/12733630/569183 */
    while (!feof(fp)) {
        if (fgetc(fp) == '\n') {
            lines++;
        }
    }

    rewind(fp);
    return lines;
}

static int instr_lookup (char *instr) {
    static char reverse_ops[][7] = {
        "halt", "move", "add", "sub", "mul", "div", "shr", "shl", "nand",
        "xor", "br", "brz", "brnz", "in", "out"
    };
    static int NUM_INSTRS = 15;

    for (int i = 0; i < NUM_INSTRS; i++) {
        if (strcmp(instr, reverse_ops[i]) == 0) {
            return i;
        }
    }

    return -1;
}

enum ops {
    HALT = 0, MOVE, ADD, SUB, MUL, DIV, SHR, SHL, NAND,
    XOR, BR, BRZ, BRNZ, IN, OUT
};

static const char OP_WIDTH = 4;
static const char OP_LSB = 32 - OP_WIDTH;

static const unsigned char OP_MASK = 0xF;
static const unsigned char REG_MASK = 0xF;

static word three_register (unsigned char op, int ra, int rb, int rc) {
    static const char RA_LSB = 0;
    static const char RB_LSB = 4;
    static const char RC_LSB = 8;

    word w = 0;
    w |= (op & OP_MASK) << OP_LSB;
    w |= (ra & REG_MASK) << RA_LSB;
    w |= (rb & REG_MASK) << RB_LSB;
    w |= (rc & REG_MASK) << RC_LSB;

    return w;
}

static unsigned read_reg (FILE *input) {
    unsigned reg = 0;
    fscanf(input, " r%d", &reg);
    return reg;
}

word read_three_register (FILE *input, unsigned instr) {
    unsigned char ra = read_reg(input);
    unsigned char rb = read_reg(input);
    unsigned char rc = read_reg(input);
    return three_register(instr, ra, rb, rc);
}

word read_two_register (FILE *input, unsigned instr) {
    unsigned char rb = read_reg(input);
    unsigned char rc = read_reg(input);
    return three_register(instr, 0, rb, rc);
}

word read_one_register (FILE *input, unsigned instr) {
    unsigned char rc = read_reg(input);
    return three_register(instr, 0, 0, rc);
}

static word read_val (FILE *input) {
    word val = 0;
    fscanf(input, " %u", &val);
    return val;
}

typedef word (* read_func)(FILE *, unsigned);

read_func read_arr[] = {
    [MOVE] = read_two_register,
    [ADD] = read_three_register,
    [SUB] = read_three_register,
    [MUL] = read_three_register,
    [DIV] = read_three_register,
    [SHR] = read_three_register,
    [SHL] = read_three_register,
    [NAND] = read_three_register,
    [XOR] = read_three_register,
    [BR] = read_one_register,
    [BRNZ] = read_one_register,
    [IN] = read_one_register,
    [OUT] = read_one_register,
};

static word read_instr (FILE *input, char *instr) {
    int instr_num = instr_lookup(instr);
    word w = 0;

    switch (instr_num) {
        case -1:
            error("Do not know instruction: `%s'.", instr);
            break;

        case HALT:
            w = three_register(HALT, 0, 0, 0);
            break;

        case MOVE: case ADD: case SUB: case MUL: case DIV: case SHR: case SHL:
        case NAND: case XOR: case BR: case BRNZ: case IN: case OUT:
            w = read_arr[instr_num](input, instr_num);
            break;

        default:
            w = read_three_register(input, instr_num);
            break;
    }

    return w;
}

struct prog process_file (FILE *input) {
    struct prog asm_ = { NULL, file_lines(input) };

    word *assembled = calloc(asm_.len, sizeof * assembled);

    if (assembled == NULL) {
        error("Could not allocate memory for file contents.", NULL);
    }

    for (word i = 0; i < asm_.len; i++) {
        char instr[7];
        fscanf(input, "%s", instr);
        //printf("%d\tinstr: %s\n", i, instr);
        assembled[i] = read_instr(input, instr);
    }

    asm_.prog = assembled;

    return asm_;
}

void write_file (FILE *output, struct prog asm_) {
    for (word i = 0; i < asm_.len; i++) {
        fwrite(&asm_.prog[i], sizeof(asm_.prog[i]), 1, output);
    }
}
