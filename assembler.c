#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  static char reverse_ops[][7] = {
      "halt", "move", "add", "sub", "mul",  "div", "shr", "shl",
      "nand", "xor",  "br",  "brz", "brnz", "in",  "out", "bif",
  };
  static int NUM_INSTRS = 16;

  for (int i = 0; i < NUM_INSTRS; i++) {
    if (strcmp(instr, reverse_ops[i]) == 0) {
      return i;
    }
  }

  return -1;
}

enum ops {
  HALT = 0,
  MOVE,
  ADD,
  SUB,
  MUL,
  DIV,
  SHR,
  SHL,
  NAND,
  XOR,
  BR,
  BRZ,
  BRNZ,
  IN,
  OUT,
  BIF,
};

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
  default:
    error("Could not parse instruction argument. Expecting: $x, rx.", NULL);
    break;
  }

  if (arg.is_deref == 1 && fgetc(input) != ')') {
    error("Could not parse instruction argument. Expecting: ).", NULL);
  }

  return arg;
}

instr_t read_three_register(FILE *input, unsigned instr) {
  struct instrarg a = read_arg(input);
  struct instrarg b = read_arg(input);
  struct instrarg c = read_arg(input);
  return three_register(instr, a, b, c);
}

instr_t read_two_register(FILE *input, unsigned instr) {
  struct instrarg b = read_arg(input);
  struct instrarg c = read_arg(input);
  return three_register(instr, emptyarg(), b, c);
}

instr_t read_one_register(FILE *input, unsigned instr) {
  struct instrarg c = read_arg(input);
  return three_register(instr, emptyarg(), emptyarg(), c);
}

typedef instr_t (*read_func)(FILE *, unsigned);

read_func read_arr[] = {
    [MOVE] = read_two_register,  [ADD] = read_three_register,
    [SUB] = read_three_register, [MUL] = read_three_register,
    [DIV] = read_three_register, [SHR] = read_three_register,
    [SHL] = read_three_register, [NAND] = read_three_register,
    [XOR] = read_three_register, [BR] = read_one_register,
    [BRNZ] = read_one_register,  [IN] = read_one_register,
    [OUT] = read_one_register,   [BIF] = read_one_register,
};

static instr_t read_instr(FILE *input, char *op_str) {
  int instr_num = instr_lookup(op_str);
  if (instr_num < 0) {
    error("Do not know instruction: `%s'.", op_str);
  }

  switch (instr_num) {
  case HALT:
    return three_register(HALT, emptyarg(), emptyarg(), emptyarg());

  case MOVE:
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case SHR:
  case SHL:
  case NAND:
  case XOR:
  case BR:
  case BRNZ:
  case IN:
  case OUT:
  case BIF:
    return read_arr[instr_num](input, instr_num);

  default:
    return read_three_register(input, instr_num);
  }
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
