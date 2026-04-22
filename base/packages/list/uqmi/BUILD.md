# Building uqmi

## Dependencies

- `arm-linux-gnueabi-gcc` cross-compiler
- `libjson-c-dev:armel` (Debian package)
- [libubox](https://github.com/git-openwrt-org-mirror/libubox) (built from source)
- CMake, Perl

## Clone

```sh
git clone https://github.com/git-openwrt-org-mirror/uqmi.git
git clone https://github.com/git-openwrt-org-mirror/libubox.git
```

## Build libubox

```sh
cd libubox
mkdir build-arm && cd build-arm
cmake .. \
  -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=arm \
  -DCMAKE_C_FLAGS="-Os -march=armv5te -mtune=arm926ej-s -ffunction-sections -fdata-sections" \
  -DBUILD_LUA=OFF \
  -DBUILD_EXAMPLES=OFF
make -j$(nproc)
cd ../..
```

## Build uqmi

```sh
cd uqmi
mkdir build-arm && cd build-arm
cmake .. \
  -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=arm \
  -DCMAKE_C_FLAGS="-Os -march=armv5te -mtune=arm926ej-s -ffunction-sections -fdata-sections -Wno-maybe-uninitialized" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections" \
  -DBUILD_STATIC=ON \
  -Dubox_include_dir=../../ \
  -Dblobmsg_json_include_dir=../../ \
  -Djson_include_dir=/usr/include \
  -Dubox_library=../../libubox/build-arm/libubox.a \
  -Dblobmsg_json_library=../../libubox/build-arm/libblobmsg_json.a \
  -Djson_library=/usr/lib/arm-linux-gnueabi/libjson-c.a
make -j$(nproc)
```

## Strip

```sh
arm-linux-gnueabi-strip -o uqmi-stripped uqmi
```

The resulting binary is ~131K stripped, statically linking libubox, libblobmsg_json and json-c. Only glibc (`libc.so.6`) is dynamically linked.
