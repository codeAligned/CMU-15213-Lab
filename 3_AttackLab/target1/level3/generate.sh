#!/bin/sh
rm disassembled.s
gcc -c level3_CI.s
objdump -d level3_CI.o > disassembled.s
