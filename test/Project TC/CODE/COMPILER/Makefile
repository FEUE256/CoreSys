# Makefile for compiling compiler.c

CC = gcc
CFLAGS = -O2 -Wall -std=c11
TARGET = tc2
SRC = compiler.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o *.asm *.s 
