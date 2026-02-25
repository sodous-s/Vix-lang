#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"
TARGET="i386-unknown-none"
KERNEL="kernel.vix"
OBJ="kernel.o"
ELF="kernel.elf"
ISO="vixos.iso"
./vixc "$KERNEL" -obj kernel --target="$TARGET"
ld.lld -m elf_i386 -T linker.ld "$OBJ" -o "$ELF"
cp "$ELF" iso/boot/kernel.elf
grub-mkrescue -o "$ISO" iso
qemu-system-i386 -cdrom "$ISO"