
#ifndef RABBIT_CODEWORDS_H
#define RABBIT_CODEWORDS_H

#include "rabbit_types.h"

/* Options for operand use. Is space C an immediate value or a register? Should
 * any of spaces A, B, and C be dereferenced? */
struct modes_s {
    uint8_t immediate;
    uint8_t regc_deref;
    uint8_t regb_deref;
    uint8_t rega_deref;
};

/* Holds an unpacked representation of an instruction (sans immediate value, if
 * it has one). */
struct unpacked_s {
    struct modes_s modes;
    uint8_t opcode;
    uint8_t regc;
    uint8_t regb;
    uint8_t rega;
};

struct unpacked_s decode(rabbitw instr);

#endif
