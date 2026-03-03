#! /bin/zsh

arm-none-eabi-gcc \
    -mcpu=cortex-m3 \
    -mthumb \
    -g \
    -nostdlib \
    -T linker.ld \
    -o myos.elf \
    kernel.c startup.s malloc.c utils.c
