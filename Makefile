
CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic
LFLAGS=
RABBIT_OBJS=rabbit.o rabbit_io.o rabbit_codewords.o
ASSEMBLER_OBJS=assembler.o rabbit_io.o
DISASSEMBLER_OBJS=disassembler.o rabbit_io.o rabbit_codewords.o

all: rabbit assembler disassembler

clean:
	rm -f rabbit rabbit-asm rabbit-dis
	rm -f $(RABBIT_OBJS) $(ASSEMBLER_OBJS) $(DISASSEMBLER_OBJS)

rabbit: $(RABBIT_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(RABBIT_OBJS) -o rabbit

assembler: $(ASSEMBLER_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(ASSEMBLER_OBJS) -o rabbit-asm

disassembler: $(DISASSEMBLER_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(DISASSEMBLER_OBJS) -o rabbit-dis

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
