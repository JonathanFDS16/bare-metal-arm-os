#! /bin/zsh

arm-none-eabi-gcc \
    -mcpu=cortex-m3 \
    -mthumb \
    -g \
    -nostdlib \
    -T linker.ld \
    -o myos.elf \
    src/kernel.c src/scheduler.c src/syscalls.c src/shell.c src/startup.s src/startup.c src/malloc.c src/utils.c
