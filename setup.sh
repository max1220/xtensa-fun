#!/bin/bash
# hackey build/download script for ESP32 Linux

# source environment variables needed
. "env.sh"



# helper to download a single dependency into it's download folder using git
function _download_git() {
	[ ! -d "${1}" ] && git clone --depth=1 --recursive "${2}" "$(basename "${1}")"
}



# build all needed dependencies
function build_all() {
	build_qemu
	build_idf
	build_linux
	build_boot
	build_buildroot
}

# build the simple linux boot loader
function build_boot() {
	pushd "boot_linux"

	idf.py build

	popd
}

# build the rootfs using buildroot
function build_buildroot() {
	[ ! -d "${ESP_BUILDROOT_DIR}" ] && cp -r "${ESP_DOWNLOAD_DIR}/$(basename "${ESP_BUILDROOT_DIR}")" "${ESP_BUILDROOT_DIR}"
	pushd "${ESP_BUILDROOT_DIR}"

	# install configuration
	cp "${ESP_DOWNLOAD_DIR}/xtensa_esp32.tar" .
	cp ../buildroot_config .config

	# patch the sources
	# patch -r - --forward --strip=1 < buildroot_.patch

	make -j13

	popd
}

# build esp-idf
function build_idf() {
	[ ! -d "${ESP_IDF_DIR}" ] && cp -r "${ESP_DOWNLOAD_DIR}/$(basename "${ESP_IDF_DIR}")" "${ESP_IDF_DIR}"
	pushd "${ESP_IDF_DIR}"

	./install.sh esp32

	# patch the sources
	patch -r - --forward --strip=1 < ../idf_parttool_hack.patch

	. export.sh

	popd
}

# build linux kernel
function build_linux() {
	[ ! -d "${ESP_LINUX_DIR}" ] && cp -r "${ESP_DOWNLOAD_DIR}/$(basename "${ESP_LINUX_DIR}")" "${ESP_LINUX_DIR}"
	pushd "${ESP_LINUX_DIR}"

	# install configuration
	cp ../linux_config .config

	# patch the sources
	patch -r - --forward --strip=1 < ../linux_binfmt_flat_endianess.patch
	patch -r - --forward --strip=1 < ../linux_dts_disable_bootargs.patch
	patch -r - --forward --strip=1 < ../linux_esp32_serial_rx.patch

	# build linux
	make -j13

	popd
}

# build qemu
function build_qemu() {
	[ ! -d "${ESP_QEMU_DIR}" ] && cp -r "${ESP_DOWNLOAD_DIR}/$(basename "${ESP_QEMU_DIR}")" "${ESP_QEMU_DIR}"
	pushd "${ESP_QEMU_DIR}"

	./configure --target-list=xtensa-softmmu \
		--enable-gcrypt \
		--enable-debug --enable-sanitizers \
		--disable-strip --disable-user \
		--disable-capstone --disable-vnc \
		--disable-sdl --disable-gtk
	ninja -C build

	popd
}

# remove build artifacts
function clean() {
	rm -rf "${ESP_QEMU_DIR}" "${ESP_IDF_DIR}" "${ESP_LINUX_DIR}" "${ESP_BUILDROOT_DIR}"
}

# download all required dependencies
function download() {
	# make sure ESP_DOWNLOAD_DIR exists and enter it
	mkdir -p "${ESP_DOWNLOAD_DIR}"
	pushd "${ESP_DOWNLOAD_DIR}"

	# download required tool sources if not found
	_download_git "${ESP_QEMU_DIR}" ${ESP_QEMU_GIT}
	_download_git "${ESP_IDF_DIR}" ${ESP_IDF_GIT}
	_download_git "${ESP_LINUX_DIR}" ${ESP_LINUX_GIT}
	_download_git "${ESP_BUILDROOT_DIR}" ${ESP_BUILDROOT_GIT}

	if [ ! -f "xtensa_esp32.tar" ]; then
		wget "https://raw.githubusercontent.com/jcmvbkbc/crosstool-NG/xtensa-1.22.x/overlays/xtensa_esp32.tar" -O "xtensa_esp32.tar"
	fi

	popd
}

# flash application, linux and rootfs
function flash() {
	# write boot_linux application
	pushd boot_linux
	idf.py flash -p "${ESPPORT}"
	popd

	# write linux partition
	parttool.py -p "${ESPPORT}" write_partition --partition-name linux --input "${ESP_LINUX_DIR}/arch/xtensa/boot/xipImage"

	# write rootfs partition
	parttool.py -p "${ESPPORT}" write_partition --partition-name rootfs --input "${ESP_BUILDROOT_DIR}/output/images/rootfs.cramfs"
	#parttool.py -p "${ESPPORT}" write_partition --partition-name rootfs --input "busybox/rootfs.cramfs"
}

# flash the specified file at the specified address
function flash_image_at() {
	addr="${1}"
	file="${2}"
	count="$(wc -c < "${file}")"
	seek="$(( addr ))"
	echo "Flashing ${file} at offset ${seek}, ${count} bytes"
	dd of=flash_image.bin bs=1 conv=notrunc seek="${seek}" count="${count}" if="${file}"
}

# generate a new flash_image.bin by writing
# the bootloader, partition table, Linux boot loader, kernel and rootfs
function build_flash_image() {
	# generate empty files
	dd if=/dev/zero bs=1024 count=16384 of=flash_image.bin
	dd if=/dev/zero bs=1 count=124 of=qemu_efuse.bin
	dd if=/dev/zero bs=1024 count=65536 of=sdcard.img

	# flash bootloader, partition table and Linux boot loader
	flash_image_at 0x1000 "boot_linux_duo/build/bootloader/bootloader.bin"
	flash_image_at 0x8000 "boot_linux_duo/build/partition_table/partition-table.bin"
	flash_image_at 0x10000 "boot_linux_duo/build/linux_boot_duo.bin"

	# flash kernel and rootfs partitions
	flash_image_at 0x40000 "${ESP_LINUX_DIR}/arch/xtensa/boot/xipImage"
	flash_image_at 0x200000 "${ESP_BUILDROOT_DIR}/output/images/rootfs.cramfs"
	#flash_image_at 0x400000 "${ESP_BUILDROOT_DIR}/output/images/rootfs.cramfs"
}



if [ -n "${1}" ]; then
	eval "${1}" "${@:2}"
else
	echo "Supported commands: build_all, build_boot, build_buildroot, build_idf, build_linux, build_qemu, clean, download, flash_qemu"
fi



# let user flash other things, or abort using ctrl-c
echo -e  "\n\n>>> DONE! <<<"
wait
