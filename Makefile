CC = clang
AS = nasm
LINK = ld.ldd

all:
  nasm -f bin bootloader.asm -o asm.bin
  clang -target i386-pc-none-elf -ffreestanding -fno-builtin -fomit-frame-pointer -fno-stack-protector -fno-pic -fno-pie -m16 -mno-red-zone -mno-stackrealign -nostdlib -Wall -Wextra -c kernel.c -o o.o
  ld.lld -Ttext 0x7E00 --oformat binary o.o -o c.bin
  truncate --size=5120 c.bin
  dd if=/dev/zero of=vdOS.img bs=512 count=20480; # 10MB blank disk
  dd if=asm.bin of=vdOS.img bs=512 count=1 conv=notrunc;
  dd if=c.bin of=vdOS.img bs=512 seek=1 conv=notrunc;
run:
  qemu-system-x86_64 -drive file=vdOS.img,format=raw,media=disk
