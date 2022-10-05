#!/bin/bash

# normal boot is 0x12, flash mode is 0x0f
strap_mode="0x12"
if [ "${1}" = "flash" ]; then
	echo "=== FLASH MODE ==="
	strap_mode="0x0f"
else
	echo "=== QEMU ==="
fi

# start QEMU with a serial port listening on TCP :5555
qemu/build/qemu-system-xtensa -nographic \
	-machine esp32 \
	-m 4M \
	-serial tcp::5555,server,wait \
	-global driver=esp32.gpio,property=strap_mode,value=${strap_mode} \
	-drive file=flash_image.bin,if=mtd,format=raw \
	-drive file=qemu_efuse.bin,if=none,format=raw,id=efuse \
	-drive file=sdcard.img,if=sd,format=raw \
	-global driver=nvram.esp32.efuse,property=drive,value=efuse
