# Building the Kernel

## Overview

Wolf OS ships with a pre-built kernel (`base/kernel/zImage`). Building from source is optional and intended for advanced users who need to modify the kernel.

The kernel is based on **Linux 6.12.y LTS**, maintained as a patched fork for the NUC980 SoC.

---

## Prerequisites

- `arm-linux-gnueabi-gcc` cross-compiler toolchain
- `make`, `bc`, `bison`, `flex`, `libssl-dev`
- Git

---

## Clone the Kernel Source

```sh
git clone https://github.com/FoxIoT-EU/linux-kernel-nuc980.git
cd linux-kernel-nuc980
```

---

## Build

1. Copy the defconfig from the Wolf OS repo into the kernel source tree:

```sh
cp /path/to/foxiot-wolf-os/base/kernel/foxiot_wolf_defconfig arch/arm/configs/
```

2. Apply the configuration:

```sh
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- foxiot_wolf_defconfig
```

3. Build the kernel:

```sh
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -j$(nproc)
```

---

## Install

Copy the built kernel image into the Wolf OS build tree:

```sh
cp arch/arm/boot/zImage /path/to/foxiot-wolf-os/base/kernel/zImage
```

Then rebuild the firmware as usual — the device tree is compiled separately during the OS build.

---

## Defconfig Reference

A copy of the kernel defconfig is stored at `base/kernel/foxiot_wolf_defconfig` for reference. This matches the configuration used to build the shipped kernel.

---

⬅️ [Back to main page](../README.md)
