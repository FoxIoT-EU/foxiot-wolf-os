# libusb

libusb **1.0.29**. Cross-compiled for NUC980 (armv5tej, soft-float, `arm-linux-gnueabi`).

Add `#include "libusb.list"` to your distro's `rootfs.list` if your application needs libusb (the `example` distro includes it).

## Rebuild

Host: `build-essential`, `git`, `autoconf`, `automake`, `libtool`, `pkg-config`, `gcc-arm-linux-gnueabi`.

### 1. Clone and checkout

```sh
git clone https://github.com/libusb/libusb.git libusb-1.0.29
cd libusb-1.0.29
git checkout v1.0.29
```

### 2. Bootstrap

```sh
NOCONFIGURE=1 ./autogen.sh
```

### 3. Configure and build

```sh
mkdir build-armv5 && cd build-armv5
CFLAGS="-Os" ../configure --host=arm-linux-gnueabi \
    --disable-udev \
    --disable-examples-build \
    --disable-tests-build \
    --disable-static \
    --disable-log
make -j$(nproc)
arm-linux-gnueabi-strip --strip-unneeded libusb/.libs/libusb-1.0.so.0.*
```

### 4. Stage

```sh
cp libusb/.libs/libusb-1.0.so.0.5.0  <pkg>/root/lib/libusb-1.0.so.0.5.0
cd <pkg>/root/lib
ln -sf libusb-1.0.so.0.5.0 libusb-1.0.so.0
ln -sf libusb-1.0.so.0     libusb-1.0.so
```

If the SONAME minor bumps on a future version, update the filename in `libusb.list` and both symlink targets.
