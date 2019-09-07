CC=ccache gcc
CCFLAGS=-Wall -ggdb
OBJ=execute.o novapy.o opcodes.o prints.o

all: novapy

%.o: %.c
	$(CC) -c -o $@ $^ $(CCFLAGS)

novapy: $(OBJ)
	$(CC) -o novapy $^ $(CCFLAGS)

