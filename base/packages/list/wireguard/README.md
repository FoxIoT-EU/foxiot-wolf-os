# wireguard

Ships `wg` (upstream wireguard-tools **v1.0.20260223**) and `wg_if` (local netlink helper, source in `wolf_wg_setup/`). Cross-compiled for NUC980 (armv5tej, soft-float, `arm-linux-gnueabi`).

## Rebuild

Host: `build-essential`, `git`, `gcc-arm-linux-gnueabi`.

### 1. Clone wireguard-tools

```sh
git clone https://git.zx2c4.com/wireguard-tools wireguard-tools-v1.0.20260223
cd wireguard-tools-v1.0.20260223
git checkout v1.0.20260223
```

### 2. Build `wg`

```sh
CFLAGS="-Os" make -C src CC=arm-linux-gnueabi-gcc wg
```

`CFLAGS` must be passed via environment (not `make CFLAGS=...`), otherwise a command-line override wipes the Makefile's `+=` appends and the build fails with undefined `RUNSTATEDIR` / `WGALLOWEDIP_A_FLAGS`.

### 3. Build `wg_if`

```sh
cd <this-package>/wolf_wg_setup
make CC=arm-linux-gnueabi-gcc
```

`-Os` is baked into `wolf_wg_setup/Makefile`.

When bumping wireguard-tools, re-sync `wolf_wg_setup/wireguard.{c,h}` from `wireguard-tools/contrib/embeddable-wg-library/` if they changed.

### 4. Strip and stage

```sh
arm-linux-gnueabi-strip <wireguard-tools>/src/wg wolf_wg_setup/wg_if
cp <wireguard-tools>/src/wg  root/bin/wg
cp wolf_wg_setup/wg_if       root/bin/wg_if
```

`root/etc/rc.wireguard` is local source code, not a build artifact — do not regenerate.
