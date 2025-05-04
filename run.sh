#!/bin/bash

mkdir -p bin &&

nasm -f bin src/bootloader.asm -o bin/bootloader.bin &&

clang -target i386 -m16 -fno-stack-protector -fno-builtin -nostdlib -ffreestanding -c src/vdOS.c -o bin/c.o &&
nasm -f bin src/vdOS.asm -o bin/asmraw.bin &&
ld.lld -Ttext 0x8000 -o bin/elf.elf bin/c.o &&
llvm-objcopy -O binary bin/elf.elf bin/cc.bin &&
touch bin/vdOS.bin &&
truncate --size=19968 bin/vdOS.bin &&
dd if=bin/asmraw.bin of=bin/vdOS.bin bs=512 count=1 conv=notrunc &&
dd if=bin/cc.bin of=bin/vdOS.bin bs=512 seek=1 conv=notrunc &&

dd if=/dev/zero of=bin/vdOS.iso bs=512 count=20480 &&

dd if=bin/bootloader.bin of=bin/vdOS.iso bs=512 count=1 conv=notrunc &&

dd if=bin/vdOS.bin of=bin/vdOS.iso bs=512 seek=1 conv=notrunc &&

mkdir -p bin &&
qemu-img create vdOS.img 1G &&

dd if=bin/vdOS.iso of=vdOS.img bs=20480 seek=0 &&
qemu-system-x86_64 -drive file=vdOS.img,format=raw,media=disk &