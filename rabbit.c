
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rabbit_bif.h"
#include "rabbit_codewords.h"
#include "rabbit_io.h"
#include "rabbit_types.h"

/* Exit/error codes and statuses. */
typedef enum rabbits {
  RB_SUCCESS = 0,
  RB_FAIL,
  RB_OVERFLOW,
  RB_ILLEGAL,
} rabbits;

/* Registers available for use. */
typedef enum rabbitr {
  RB_ZERO = 0,
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
  RB_NUMREGS,
} rabbitr;

const unsigned kZeroFlag = 0x2U;

#define fetch_immediate() mem[regs[RB_IP]++]

/* Most instructions have a destination address and two operands. This struct
 * holds those. */
struct abc_s {
  rabbitw *dst, b, c;
};

/* Using the options/modes given by the user, fetch the destination address and
 * operands. */
static struct abc_s getabc(rabbitw *regs, rabbitw *mem, struct unpacked_s i) {
  rabbitw *dst = i.modes.rega_deref ? &mem[regs[i.rega]] : &regs[i.rega];
  rabbitw bval = i.modes.regb_deref ? mem[regs[i.regb]] : regs[i.regb];
  rabbitw cval = i.modes.immediate ? fetch_immediate() : regs[i.regc];
  cval = i.modes.regc_deref ? mem[cval] : cval;
  return (struct abc_s){.dst = dst, .b = bval, .c = cval};
}

rabbitw hello(rabbitw *regs, rabbitw *mem) {
  (void)regs;
  (void)mem;
  fprintf(stdout, "hello\n");
  return 0;
}

typedef rabbitw (*bif)(rabbitw *regs, rabbitw *mem);

#define FN_ENTRY(id, fn_name) fn_name,

static const bif bif_functions[] = {BIF_TABLE(FN_ENTRY)};

#undef FN_ENTRY

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Need to pass file to execute.\n");
    return RB_FAIL;
  }

  char *fn = argv[1];
  FILE *fp = fopen(fn, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Can't open `%s'.\n", fn);
    return RB_FAIL;
  }

  /* Get the size of the file so that we can allocate memory for it. */
  struct stat st;
  if (fstat(fileno(fp), &st) != 0) {
    fprintf(stderr, "Can't stat `%s'.\n", fn);
    fclose(fp);
    return RB_FAIL;
  }

  /* Allocate memory with program first, stack second. */
  size_t stacksize = 1000;
  off_t size = st.st_size;
  rabbitw *mem = malloc(stacksize + size * sizeof *mem);
  if (mem == NULL) {
    fprintf(stderr,
            "Not enough memory. Could not allocate stack of size"
            "%zu + program of size %ld.\n",
            stacksize, size);
    return RB_FAIL;
  }

  /* Read the file into memory. */
  size_t i = 0;
  rabbitw word = 0;
  /* We cannot use fread because of endian-ness issues. */
  while (read_word(fp, &word) != 0) {
    mem[i++] = word;
  }
  fclose(fp);

  struct abc_s abc;
  rabbitw regs[RB_NUMREGS] = {0};
  regs[RB_SP] = i;

  /* Main fetch-decode-execute loop. */
  while (1) {
    /* Fetch the current instruction word. */
    rabbitw word = mem[regs[RB_IP]++];

    /* Decode it. */
    struct unpacked_s i = decode(word);

    /* Execute it. */
    switch (i.opcode) {
    case Halt:
      free(mem);
      return RB_SUCCESS;
      break;
    case Move: {
      /* Move is special because it has one source instead of two
       * operands. */
      rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
      rabbitw *dst = i.modes.regb_deref ? &mem[regs[i.regb]] : &regs[i.regb];
      *dst = i.modes.regc_deref ? mem[src] : src;
      break;
    }
    case Add:
      abc = getabc(regs, mem, i);
      *abc.dst = abc.b + abc.c;
      break;
    case Sub:
      abc = getabc(regs, mem, i);
      rabbitw res = abc.b - abc.c;
      *abc.dst = res;
      if (res == 0) {
        regs[RB_FLAGS] |= kZeroFlag;
      }
      break;
    case Mul:
      abc = getabc(regs, mem, i);
      *abc.dst = abc.b * abc.c;
      break;
    case Div:
      abc = getabc(regs, mem, i);
      if (abc.c == 0) {
        free(mem);
        return RB_ILLEGAL;
      }

      *abc.dst = abc.b / abc.c;
      break;
    case Shr:
      abc = getabc(regs, mem, i);
      *abc.dst = abc.b >> abc.c;
      break;
    case Shl:
      abc = getabc(regs, mem, i);
      *abc.dst = abc.b << abc.c;
      break;
    case Nand:
      abc = getabc(regs, mem, i);
      *abc.dst = ~(abc.b & abc.c);
      break;
    case Xor:
      abc = getabc(regs, mem, i);
      *abc.dst = abc.b ^ abc.c;
      break;
    case Br: {
      /* Branch is special because it only has one argument. */
      rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
      regs[RB_IP] = i.modes.regc_deref ? mem[src] : src;
      break;
    }
    case Brz:
      /* Branch if zero is special because it only has one argument. */
      if ((regs[RB_FLAGS] & kZeroFlag) == 0) {
        rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
        regs[RB_IP] = i.modes.regc_deref ? mem[src] : src;
      }
      break;
    case Brnz:
      /* Branch not zero is special because it only has one argument. */
      if ((regs[RB_FLAGS] & kZeroFlag) != 0) {
        rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
        regs[RB_IP] = i.modes.regc_deref ? mem[src] : src;
      }
      break;
    case In: {
      /* Input is special because it does not have an argument. */
      rabbitw *dstp = i.modes.regc_deref ? &mem[regs[i.regc]] : &regs[i.regc];
      *dstp = getchar();
      break;
    }
    case Out: {
      /* Output is special because it has one argument and no
       * destination. */
      rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
      src = i.modes.regc_deref ? mem[src] : src;
      putchar(src);
      break;
    }
    case Bif: {
      rabbitw src = i.modes.immediate ? fetch_immediate() : regs[i.regc];
      src = i.modes.regc_deref ? mem[src] : src;
      if (src > kNumBifs) {
        fprintf(stderr, "Invalid bif: `%u'.\n", src);
        return RB_FAIL;
      }
      bif f = bif_functions[src];
      f(regs, mem);
      break;
    }
      /*    case RB_CFF:
              break;
      */
    default:
      free(mem);
      return RB_ILLEGAL;
      break;
    }
  }

  return 0;
}

#undef fetch_immediate
