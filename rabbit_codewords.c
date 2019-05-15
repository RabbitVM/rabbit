
#include "rabbit_codewords.h"

/* Transform the packed representation of an instruction into the unpacked
 * representation. */
struct unpacked_s decode(rabbitw instr) {
  /* Fetch the space modes from the instruction. */
  uint8_t modes = (instr >> 24) & 0xF;

  /* Offsets of the space modes in the mode nibble. */
  static const uint8_t RB_ADDRA_LSB = 0, RB_ADDRB_LSB = 1, RB_ADDRC_LSB = 2,
                       RB_IMMED_LSB = 3;

  return (struct unpacked_s){
      .modes =
          {
              .immediate = (modes >> RB_IMMED_LSB) & 0x1,
              .regc_deref = (modes >> RB_ADDRC_LSB) & 0x1,
              .regb_deref = (modes >> RB_ADDRB_LSB) & 0x1,
              .rega_deref = (modes >> RB_ADDRA_LSB) & 0x1,
          },
      .opcode = instr >> 28,
      .regc = (instr >> 8) & 0xF,
      .regb = (instr >> 4) & 0xF,
      .rega = instr & 0xF,
  };
}
