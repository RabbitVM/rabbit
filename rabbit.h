#ifndef RABBIT_H
#define RABBIT_H

struct rabbit_s;
typedef struct rabbit_s *rabbit_t;
typedef uint32_t rabbit_word;

typedef enum {
    RB_SUCCESS,
    RB_FAIL,
    RB_OVERFLOW,
    RB_ILLEGAL,
} rabbit_status;

enum {
    RB_ZERO,
    RB_R1,
    RB_R2,
    RB_R3,
    RB_R4,
    RB_R5,
    RB_R6,
    RB_R7,
    RB_R8,
    RB_R9,
    RB_IP,
    RB_SP,
    RB_RET,
    RB_TMP,
    RB_FLAGS,
} rabbit_reg;

enum {
    RB_HALT,
    RB_MOVE,
    RB_ADD,
    RB_SUB,
    RB_MUL,
    RB_DIV,
    RB_SHR,
    RB_SHL,
    RB_NAND,
    RB_XOR,
    RB_BR,
    RB_BRZ,
    RB_BRNZ,
    RB_IN,
    RB_OUT,
} rabbit_instr;

const unsigned char RB_ADDRA = 1U << 0,
    RB_ADDRB = 1U << 1,
    RB_ADDRC = 1U << 2,
    RB_IMMED = 1U << 3;

rabbit_t rabbit_new (rabbit_word mem_size);
void rabbit_free (rabbit_t *r);

rabbit_word rabbit_pack_reg (unsigned char opcode,
                             unsigned char modes,
                             unsigned char regc,
                             unsigned char regb,
                             unsigned char rega);

rabbit_word rabbit_pack_imm (unsigned char opcode,
                             unsigned char modes,
                             unsigned char regc,
                             unsigned char regb);

void rabbit_load_code (rabbit_t r, rabbit_word *code, rabbit_word len);
rabbit_status rabbit_run (rabbit_t r);

#endif
