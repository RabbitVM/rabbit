#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rabbit_bif.h"
#include "rabbit_codewords.h"

/*
  Instruction format is as follows:

  add r0, r1, r2
  out r1

  cmov r1, r2, r3

  MAKE SURE YOU HAVE A BLANK LINE AT THE END!!!
*/

typedef uint32_t word;

void error(const char *msg, const char *optional_arg);
void check_usage(int argc);
FILE *open_file(const char *fn, const char *mode);
word file_lines(FILE *fp);
void assemble_file(FILE *input, FILE *output);

int main(int argc, char **argv) {
  check_usage(argc);

  const char *inputfn = argv[1];
  size_t inputfnlen = strlen(inputfn);
  char *outputfn = malloc(inputfnlen + 2 + 1);
  if (outputfn == NULL) {
    error("Could not allocate space for filename.", NULL);
    return 1;
  }

  strncpy(outputfn, inputfn, inputfnlen);
  outputfn[inputfnlen] = '.';
  outputfn[inputfnlen + 1] = 'o';
  outputfn[inputfnlen + 2] = 0;

  FILE *input = open_file(inputfn, "r");
  FILE *output = open_file(outputfn, "wb+");
  assemble_file(input, output);
  fclose(input);
  fclose(output);
}

void error(const char *msg, const char *optional_arg) {
  fprintf(stderr, "Error: ");

  if (optional_arg) {
    fprintf(stderr, msg, optional_arg);
    putchar('\n');
  } else {
    fprintf(stderr, "%s\n", msg);
  }

  exit(EXIT_FAILURE);
}

void check_usage(int argc) {
  if (argc != 2) {
    error("Invalid usage. Please run like: "
          "./asm <input_filename>",
          NULL);
  }
}

FILE *open_file(const char *fn, const char *mode) {
  FILE *fp = fopen(fn, mode);

  if (fp == NULL) {
    error("Could not open file `%s'.", fn);
  }

  return fp;
}

word file_lines(FILE *fp) {
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

static int instr_lookup(char *instr) {
  for (int i = 0; i < kNumInstrs; i++) {
    if (strcmp(instr, instr_nargs[i].name) == 0) {
      return i;
    }
  }
  return kInvalidInstr;
}

#define OP_WIDTH 4
static const char OP_LSB = 32 - OP_WIDTH;

static const unsigned char OP_MASK = 0xF;
static const unsigned char REG_MASK = 0xF;

enum { REG, IMMINT, IMMSTR };

struct instrarg {
  int argtype;
  int is_deref;
  word i;
};

struct instrarg emptyarg() {
  return (struct instrarg){.argtype = REG, .is_deref = 0, .i = 0};
}

typedef struct {
  int is_imm;
  word instr;
  word imm;
} instr_t;

static instr_t three_register(unsigned char op, struct instrarg a,
                              struct instrarg b, struct instrarg c) {
  static const unsigned char RA_LSB = 0;
  static const unsigned char RB_LSB = 4;
  static const unsigned char RC_LSB = 8;
  static const unsigned char MODES_LSB = 24;
  static const unsigned char RB_ADDRA = 1U << 0, RB_ADDRB = 1U << 1,
                             RB_ADDRC = 1U << 2, RB_IMMED = 1U << 3;

  uint8_t modes = 0;
  if (a.is_deref == 1) {
    modes |= RB_ADDRA;
  }
  if (b.is_deref == 1) {
    modes |= RB_ADDRB;
  }
  if (c.is_deref == 1) {
    modes |= RB_ADDRC;
  }
  if (c.argtype == IMMINT) {
    modes |= RB_IMMED;
  }

  word w = 0;
  w |= (op & OP_MASK) << OP_LSB;
  w |= (a.i & REG_MASK) << RA_LSB;
  w |= (b.i & REG_MASK) << RB_LSB;
  w |= (c.i & REG_MASK) << RC_LSB;
  w |= (modes & 0xF) << MODES_LSB;

  instr_t instr = {.is_imm = 0, .instr = w, .imm = 0};
  if (c.argtype == IMMINT) {
    instr.is_imm = 1;
    instr.imm = c.i;
  }

  return instr;
}

int read_int(FILE *input) {
  int val;
  if (fscanf(input, "%d", &val) != 1) {
    error("Could not parse instruction argument. "
          "Was expecting integer.",
          NULL);
  }
  return val;
}

int read_whitespace(FILE *input) {
  int c;
  do {
    c = fgetc(input);
  } while (c != EOF && isspace(c));
  return c;
}

int bif_lookup(char *bif_name) {
  for (int i = 0; i < kNumBifs; i++) {
    if (strcmp(bif_name, bif_names[i]) == 0) {
      return i;
    }
  }
  return kInvalidBif;
}

int read_bif(FILE *input) {
  char bif_name[12] = {0};
  if (fscanf(input, "%11s", bif_name) != 1) {
    error("Could not parse bif argument. "
          "Was expecting bif name.",
          NULL);
  }
  int bif_id = bif_lookup(bif_name);
  if (bif_id == kInvalidBif) {
    error("Invalid bif with name `%s'.", bif_name);
  }
  return bif_id;
}

static struct instrarg read_arg(FILE *input) {
  int c = read_whitespace(input);
  if (c == EOF) {
    error("Reached end of file while reading an instruction.", NULL);
  }

  struct instrarg arg = emptyarg();
  if (c == '(') {
    arg.is_deref = 1;
    c = fgetc(input);
  }

  switch (c) {
  case '$':
    arg.i = read_int(input);
    arg.argtype = IMMINT;
    break;
  case 'r':
    arg.i = read_int(input);
    arg.argtype = REG;
    break;
  case '@':
    arg.i = read_bif(input);
    arg.argtype = IMMINT;
    break;
  default:
    error("Could not parse instruction argument. Expecting: $x, rx.", NULL);
    break;
  }

  if (arg.is_deref == 1 && fgetc(input) != ')') {
    error("Could not parse instruction argument. Expecting: ).", NULL);
  }

  return arg;
}

static instr_t read_instr(FILE *input, char *op_str) {
  unsigned instr = instr_lookup(op_str);
  if (instr == kInvalidInstr) {
    error("Do not know instruction: `%s'.", op_str);
  }
  unsigned nargs = instr_nargs[instr].nargs;
  if (nargs == 0) {
    return three_register(instr, emptyarg(), emptyarg(), emptyarg());
  }
  if (nargs == 1) {
    struct instrarg c = read_arg(input);
    return three_register(instr, emptyarg(), emptyarg(), c);
  }
  if (nargs == 2) {
    struct instrarg b = read_arg(input);
    struct instrarg c = read_arg(input);
    return three_register(instr, emptyarg(), b, c);
  }
  if (nargs == 3) {
    struct instrarg a = read_arg(input);
    struct instrarg b = read_arg(input);
    struct instrarg c = read_arg(input);
    return three_register(instr, a, b, c);
  }
  error("Unreachable (op `%s').", op_str);
  assert(0);
}

void write_word(FILE *fp, word *w) {
  unsigned char *wordp = (unsigned char *)w;
  for (int i = 3; i >= 0; i--) {
    fwrite(&wordp[i], sizeof wordp[i], 1, fp);
  }
}

void assemble_file(FILE *input, FILE *output) {
  word lines = file_lines(input);

  for (word i = 0; i < lines; i++) {
    char instr_string[7];
    fscanf(input, "%s", instr_string);
    instr_t instr = read_instr(input, instr_string);
    write_word(output, &instr.instr);
    if (instr.is_imm == 1) {
      write_word(output, &instr.imm);
    }
  }
}
