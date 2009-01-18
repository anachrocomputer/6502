# Makefile for 6502 assembler

all: as6502 tests

as6502: as6502.o
	gcc -o as6502 as6502.o

as6502.o: as6502.c as6502.h
	gcc -c -o as6502.o as6502.c

tests: as6502
	as6502 testok.asm testok.hex testok.lst
	exectest
