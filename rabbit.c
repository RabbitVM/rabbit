#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "rabbit.h"

struct rabbit_s {
    rabbit_word regs[16];
    bool running;
    rabbit_word *mem;
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

rabbit_t rabbit_new (rabbit_word mem_size) {
    rabbit_t rb = calloc(1, sizeof *rb);
    if (rb == NULL) {
        return NULL;
    }

    rb->mem = calloc(mem_size, sizeof *(rb->mem));
    if (rb->mem == NULL) {
        free(rb);
        return NULL;
    }

    return rb;
}

void rabbit_free (rabbit_t *r) {
    assert(r != NULL);
    assert(*r != NULL);
    assert((*r)->mem != NULL);

    free((*r)->mem);
    free(*r);
}

rabbit_word rabbit_pack_reg (uint8_t opcode,
                             uint8_t modes,
                             uint8_t regc,
                             uint8_t regb,
                             uint8_t rega) {
    return 0U |
        ((opcode & 0xF) << 28) |
        ((modes & 0xF) << 24) |
        ((regc << 8) & 0xF) |
        ((regb << 4) & 0xF) |
        (rega & 0xF);
}

rabbit_word rabbit_pack_imm (uint8_t opcode,
                             uint8_t modes,
                             uint8_t regc,
                             uint8_t regb) {
    return rabbit_pack_reg(opcode, modes, regc, regb, 0U);
}

void rabbit_load_code (rabbit_t r, rabbit_word *code, rabbit_word len) {

}

#define fetch_immediate() r.mem[r.regs[RB_IP]++]
#define reg(R) r.regs[(R)]
#define deref(R) r.mem[(R)]

static inline void rabbit_instr_move (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *b_ptr = i.modes.regb_deref ? &deref(reg(i.regb)) : &reg(i.regb);
    rabbit_word c_val = i.modes.immediate ? fetch_immediate() : reg(i.regc);
    *b_ptr = i.modes.regc_deref ? deref(c_val) : reg(i.regc);
}

static inline void rabbit_instr_add (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val + c_val;
}

static inline void rabbit_instr_sub (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val - c_val;
}

static inline void rabbit_instr_div (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val / c_val;
}

static inline void rabbit_instr_mul (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val * c_val;
}

static inline void rabbit_instr_shr (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val >> c_val;
}

static inline void rabbit_instr_shl (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val << c_val;
}

static inline void rabbit_instr_nand (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = ~(b_val & c_val);
}

static inline void rabbit_instr_xor (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word *a_ptr = i.modes.rega_deref ? &deref(reg(i.rega)) : &reg(i.rega);
    rabbit_word b_val = i.modes.regb_deref ? deref(reg(i.regb)) : reg(i.regb);

    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    *a_ptr = b_val ^ c_val;
}

static inline void rabbit_instr_br (struct rabbit_s r, struct unpacked_s i) {
    rabbit_word c_val;
    if (i.modes.immediate) {
        c_val = fetch_immedate();
    }
    else {
        c_val = reg(i.regc);
    }

    if (i.modes.regc_deref) {
        c_val = deref(c_val);
    }

    r.regs[RB_IP] = c_val;
}

rabbit_status rabbit_run (rabbit_t mach) {
    assert(r != NULL);

    struct rabbit_s r = *mach;
    r.running = true;

    rabbit_word *regs = r.regs;
    rabbit_word *mem = r.mem;

    while (r.running) {
        rabbit_word instr = mem[regs[RB_IP]++];
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
            regb = (instr >> 4) & 0xF,
            rega = instr & 0xF,
        };

        switch (instr) {
            case RB_HALT:
                r.running = false;
                break;
            case RB_MOVE:
                rabbit_instr_move(r, instr_unpacked);
                break;
            case RB_ADD:
                rabbit_instr_add(r, instr_unpacked);
                break;
            case RB_SUB:
                rabbit_instr_sub(r, instr_unpacked);
                break;
            case RB_MUL:
                break;
            case RB_DIV:
                break;
            case RB_SHR:
                break;
            case RB_SHL:
                break;
            case RB_NAND:
                break;
            case RB_XOR:
                break;
            case RB_BR:
                break;
            case RB_BRZ:
                break;
            case RB_BRNZ:
                break;
            case RB_IN:
                break;
            case RB_OUT:
                break;
            default:
                return RB_ILLEGAL;
        }
    }

    return RB_SUCCESS;
}
