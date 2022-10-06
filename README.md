# xtensa-fun

My test repository for having fun with the ESP32 and Linux.

All porting work was done by other people.

Repositories that make this possible:

 * [buildroot/buildroot](https://github.com/buildroot/buildroot) (generate the rootfs)
 * [crosstool-ng/crosstool-ng](https://github.com/crosstool-ng/crosstool-ng) (used to build FDPIC toolchain)
 * [espressif/esp-idf](https://github.com/espressif/esp-idf) (ESP32 toolchain)
 * [espressif/qemu](https://github.com/espressif/qemu) (ESP32 emulator)
 * [jcmvbkbc/crosstool-NG](https://github.com/jcmvbkbc/crosstool-NG) (For the ESP32 overlay, not downloaded)
 * [jcmvbkbc/esp-idf](https://github.com/jcmvbkbc/esp-idf) (Just the Linux bootloader, not downloaded)
 * [jcmvbkbc/linux-xtensa](https://github.com/jcmvbkbc/linux-xtensa) (Linux kernel port)
 * [ESP32-Bare-Metal-AppCPU](https://github.com/Winkelkatze/ESP32-Bare-Metal-AppCPU) (Bootloader support functions)



# Files and directories



## setup.sh

This script is used to download and build a toolchain, Linux kernel,
and buildroot rootfs for the ESP32.

Supported commands:

```
build_all, build_boot, build_buildroot, build_idf, build_linux, build_qemu, clean, download, flash_qemu
```



## env.sh

Envirioment variables, mostly for the setup.sh script, but can also be
used to build projects manually.

Adds ESP-IDF and buildroot toolchains to $PATH, among some other things.



## qemu.sh

This script is used to launch the QEMU emulator for testing.

If the first argument is "flash" then QEMU starts the emulated ESP32
in flash mode(bootstrap pin set to 0x0f).



## boot_linux/

This is the simple bootloader for the Linux image.
It simply sets up the flash cache and jumps to the Linux CONFIG_KERNEL_LOAD_ADDRESS(0x400d0000).

Mostly copied from [jcmvbkbc Linux bootloader](https://github.com/jcmvbkbc/esp-idf/blob/linux/examples/get-started/linux_boot/main/linux_boot_main.c)



## boot_linux_duo/

This is an experimental bootloader that tries to boot Linux on the
APP CPU, while leaving FreeRTOS running on the PRO CPU(Not working yet,
Currently FreeRTOS still stops).

Mostly copied from [jcmvbkbc Linux bootloader](https://github.com/jcmvbkbc/esp-idf/blob/linux/examples/get-started/linux_boot/main/linux_boot_main.c)



## hello_world/

This is an example Linux C application that can be compiled with the
buildroot toolchain. Dumps some registers.



## Flash process

```
./qemu.sh flash &

pushd boot_linux
idf.py flash
popd

pushd linux-xtensa
time parttool.py -p "${ESPPORT}" write_partition --partition-name linux --input arch/xtensa/boot/xipImage
popd

pushd buildroot
time parttool.py -p "${ESPPORT}" write_partition --partition-name rootfs --input output/images/rootfs.cramfs
popd

kill %1
```



# TODO

There is a bunch of stuff still missing, both in the Linux port, and in
the related tooling.

 * Figure out a way to build a working FDPIC toolchain to save RAM -
   Currently, the bFLT binary format is used, which is not well suited
   to a binary like BusyBox.
 * It would be great to keep FreeRTOS running on the PRO CPU.
   In *theory* the two cores *should* be independent enough to support
   running side by side(separate mapping for MMU, PID, Timers, IRQ, etc.).
   This would possibly enable using the FreeRTOS drivers from Linux,
   and could provide important drivers like WiFi or Bluetooth
   to Linux that would otherwise be difficult or impossible provide due
   to closed-source binary blob drivers specifically for FreeRTOS.
 * Make a more RAM-friendly kernel that could fit entirely in the internal
   SRAM of the ESP32(520K).
   The current 6.0-based mostly stock kernel probably still has some
   needlessly large buffers etc. that could be cut down.
   For inspiration, see the article series on [lwn.net](https://lwn.net/Articles/741494/).
 * Figure out a way to make more than 4MB Flash available for Linux.
