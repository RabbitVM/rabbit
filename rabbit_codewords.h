
#ifndef RABBIT_CODEWORDS_H
#define RABBIT_CODEWORDS_H

#include "rabbit_types.h"

/* Options for operand use. Is space C an immediate value or a register? Should
 * any of spaces A, B, and C be dereferenced? */
struct modes_s {
    uint8_t immediate  : 1;
    uint8_t regc_deref : 1;
    uint8_t regb_deref : 1;
    uint8_t rega_deref : 1;
};

/* Holds an unpacked representation of an instruction (sans immediate value, if
 * it has one). */
struct unpacked_s {
    struct modes_s modes;
    uint8_t opcode : 4;
    uint8_t regc   : 4;
    uint8_t regb   : 4;
    uint8_t rega   : 4;
};

struct unpacked_s decode(rabbitw instr);

#endif
