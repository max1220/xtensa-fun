#!/bin/bash
set -eu

# build an elf-fdpic binary
function build_fdpic() {
	# TODO: I have not yet succeeded in building a xtensa fdpic toolchain,
	#       Would this work?
	"${CROSS_COMPILE}gcc" -c -Wall -Werror -fPIC -o main_fdpic.o main.c | tee main_fdpic.out
	"${CROSS_COMPILE}gcc" -fPIE -shared -o main_fdpic main_fdpic.o | tee -a main_fdpic.out
}

# build a bFLT(FLAT) binary.
function build_flat() {
	"${CROSS_COMPILE}gcc" -v -Wl,-elf2flt="-v -k" -o main_flat main.c | tee main_flat.out
	# print FLAT headers
	flthdr -p main_flat
	flthdr -P main_flat
	# set some FLAT headers
	flthdr main_flat -Z -R
}

# could also use buildroot BR2_ROOTFS_OVERLAY
function build_rootfs() {
	mkdir -p rootfs/bin
	[ -f "main_flat" ] && cp main_flat rootfs/bin
	[ -f "main_fdpic" ] && cp main_fdpic rootfs/bin
	mkcramfs -q -v -X -L rootfs/ rootfs.cramfs
}

# delete build artifacts
function clean() {
	rm -rf *.o *.out *.gdb
	rm -rf main_fdpic main_flat rootfs rootfs.cramfs
}

# flash using parttool(slow, use the top-level build script to flash using dd)
function flash() {
	[ ! -f "rootfs.cramfs" ] && build_rootfs

	# start qemu in flash mode
	pushd ..
	./qemu.sh flash &
	popd

	# wait for qemu to become ready
	sleep 1

	# write to rootfs partition
	parttool.py -p "${ESPPORT}" write_partition --partition-name rootfs --input rootfs.cramfs

	echo -e "  >>> QEMU still running! <<<"
}




if [ -n "${1-}" ]; then
	eval "${1}" "${@:2}"
else
	echo "Supported commands: build_fdpic, build_flat, build_rootfs, clean, flash"
fi

echo -e "\n\n >>> DONE <<<\n\n"
wait
