# needs to be run in base directory
cd -- "$( dirname -- "${BASH_SOURCE[0]}" )"

# if these directories are not found, the dependency is downloaded
# and built automatically.
export ESP_QEMU_DIR="$(realpath qemu)"
export ESP_QEMU_GIT="https://github.com/max1220/qemu-esp32"

export ESP_IDF_DIR="$(realpath esp-idf)"
export ESP_IDF_GIT="https://github.com/espressif/esp-idf"

export ESP_LINUX_DIR="$(realpath linux-xtensa)"
export ESP_LINUX_GIT="https://github.com/max1220/linux-xtensa -b xtensa-6.0-esp32-max1220-experiments"

export ESP_BUILDROOT_DIR="$(realpath buildroot)"
export ESP_BUILDROOT_GIT="https://github.com/buildroot/buildroot"

export ESP_DOWNLOAD_DIR="$(realpath download)"


# build environment variables
export ARCH="xtensa"
#export CROSS_COMPILE="xtensa-esp32-elf-"
export CROSS_COMPILE="xtensa-buildroot-uclinux-uclibc-"

# default port for ESP is QEMU connection
export ESPPORT="socket://localhost:5555"

# esp-idf environment variables
if [ -z "${ESP_IDF_VERSION-}" ]; then
	[ -f "${ESP_IDF_DIR}/export.sh" ] && . "${ESP_IDF_DIR}/export.sh"
fi

# add buildroot tools to path
if [ -d "${ESP_BUILDROOT_DIR}" ]; then
	export PATH="${PATH}:${ESP_BUILDROOT_DIR}/output/host/bin:${ESP_BUILDROOT_DIR}/output/host/xtensa-buildroot-uclinux-uclibc/bin"
fi
