#!/bin/sh
rm disassembled.s
gcc -c level2_CI.s
objdump -d level2_CI.o > disassembled.s
