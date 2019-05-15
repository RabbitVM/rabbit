
#ifndef RABBIT_CODEWORDS_H
#define RABBIT_CODEWORDS_H

#include "rabbit_types.h"

#define INSTR_TABLE(V)                                                         \
  V(Halt, halt, 0)                                                             \
  V(Move, move, 2)                                                             \
  V(Add, add, 3)                                                               \
  V(Sub, sub, 3)                                                               \
  V(Mul, mul, 3)                                                               \
  V(Div, div, 3)                                                               \
  V(Shr, shr, 3)                                                               \
  V(Shl, shl, 3)                                                               \
  V(Nand, nand, 3)                                                             \
  V(Xor, xor, 3)                                                               \
  V(Br, br, 1)                                                                 \
  V(Brz, brz, 1)                                                               \
  V(Brnz, brnz, 1)                                                             \
  V(In, in, 1)                                                                 \
  V(Out, out, 1)                                                               \
  V(Bif, bif, 1)

#define INSTR_ID(id, name, nargs) id,

// clang-format off
enum InstrId {
  INSTR_TABLE(INSTR_ID)
  kNumInstrs,
  kInvalidInstr,
};
// clang-format on

#undef INSTR_ID

struct instr_s {
  const char *name;
  unsigned nargs;
};

#define INSTR_NARGS(id, name, nargs) {#name, nargs},

static const struct instr_s instr_nargs[] = {INSTR_TABLE(INSTR_NARGS)};

#undef INSTR_NARGS

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
