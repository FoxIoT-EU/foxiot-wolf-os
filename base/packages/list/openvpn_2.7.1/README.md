# openvpn_2.7.1

OpenVPN **2.7.1** statically configured for minimal footprint, dynamically linked against **OpenSSL 3.5.6 LTS** (supported upstream until 2030-04-08), cross-compiled for the Nuvoton NUC980 (ARM926EJ-S, armv5tej, soft-float EABI, `arm-linux-gnueabi`).

This package supersedes `openvpn/` (2.4.7 + mbedTLS, 2020) and `openvpn_3.3.0/` (2.7_git snapshot from Feb 2024 + OpenSSL 3.3.0-dev). Both of those should be removed once this package is in use.

## What gets installed on the target

| Path | Source | Note |
|---|---|---|
| `/bin/openvpn` | `root/bin/openvpn` | main daemon, stripped |
| `/lib/libssl.so.3` | `root/lib/libssl.so.3` | OpenSSL 3.5.6 LTS |
| `/lib/libcrypto.so.3` | `root/lib/libcrypto.so.3` | OpenSSL 3.5.6 LTS |
| `/lib/libcap-ng.so.0` | `root/lib/libcap-ng.so.0` | required by OpenVPN 2.7 on Linux (capability dropping); cannot be disabled at configure time |
| `/sbin/ifconfig` → `/bin/busybox` | symlink | used by OpenVPN's default up/down scripts |
| `/sbin/route` → `/bin/busybox` | symlink | used by OpenVPN's default up/down scripts |

Verify with `arm-linux-gnueabi-readelf -d root/bin/openvpn`:
```
NEEDED  libcap-ng.so.0
NEEDED  libssl.so.3
NEEDED  libcrypto.so.3
NEEDED  libc.so.6
```

## How to rebuild from source

Host prerequisites (Debian 10 was used): `build-essential`, `git`, `autoconf`, `automake`, `libtool`, `pkg-config`, `perl`, `python3`, `gcc-arm-linux-gnueabi`, `libcap-ng-dev:armel` (for the ARM sysroot `libcap-ng.so.0`).

### 1. OpenSSL 3.5.6

```sh
git clone https://github.com/openssl/openssl.git
cd openssl
git checkout openssl-3.5.6

# clean any previous build
git clean -fdx

CFLAGS="-Os" ./Configure linux-armv4 \
    no-asm no-ssl3 no-tls1 no-tls1_1 no-comp no-dso no-async \
    --cross-compile-prefix=arm-linux-gnueabi- \
    --prefix=/tmp/ssl --openssldir=/tmp/ssl \
    -Wl,-rpath,'$(LIBRPATH)'

make -j$(nproc)
arm-linux-gnueabi-strip libcrypto.so.3 libssl.so.3
```

Notes:
- `linux-armv4` is OpenSSL's baseline ARM target — works on armv5.
- `no-asm` keeps it portable across armv5 sub-variants.
- `no-ssl3 no-tls1 no-tls1_1 no-comp no-dso no-async` removes obsolete protocols and features we don't use.
- The `--prefix`/`--openssldir` values are build-host paths; the libraries are relocatable and do not encode these at runtime for the parts we ship.

### 2. OpenVPN 2.7.1

```sh
git clone https://github.com/OpenVPN/openvpn.git
cd openvpn
git checkout v2.7.1

# clean any previous build
git clean -fdx

autoreconf -i

CFLAGS="-Os -I/path/to/openssl/include" \
LDFLAGS="-L/path/to/openssl" \
OPENSSL_LIBS="-lssl -lcrypto" \
OPENSSL_CFLAGS="-I/path/to/openssl/include" \
./configure --host=arm-linux-gnueabi \
    --disable-plugins \
    --disable-management \
    --disable-debug \
    --disable-plugin-auth-pam \
    --disable-dco \
    --disable-lzo \
    --disable-lz4 \
    --disable-pkcs11 \
    --disable-systemd

make -j$(nproc)
arm-linux-gnueabi-strip src/openvpn/openvpn
```

Replace `/path/to/openssl` with the absolute path to the built OpenSSL tree from step 1.

### Minimal flag rationale

| Flag | Reason |
|---|---|
| `--disable-plugins` | no plugin loading, smaller binary |
| `--disable-management` | no runtime management socket (not used in wolf-os) |
| `--disable-debug` | strip debug scaffolding |
| `--disable-plugin-auth-pam` | no PAM |
| `--disable-dco` | kernel 6.12.76 has no `ovpn`/`ovpn-dco` module (upstream `ovpn` landed only in 6.16); also avoids pulling in `libnl-3` + `libnl-genl-3` |
| `--disable-lzo` | deprecated compression (VORACLE); not used by any customer config |
| `--disable-lz4` | not used by any customer config |
| `--disable-pkcs11` | no hardware tokens (wolf-os uses file-based PKI) |
| `--disable-systemd` | wolf-os uses a plain sysvinit `rc.openvpn` script |

Cannot disable:
- **IPv6**: OpenVPN removed `--disable-ipv6` around 2.6; 2.7 always compiles IPv6 support. Harmless because the nuc980 kernel has `# CONFIG_IPV6 is not set`, so the code paths are dead at runtime.
- **libcap-ng**: OpenVPN 2.7 hard-depends on libcap-ng on Linux hosts (`configure.ac:776-788`, unconditional `PKG_CHECK_MODULES` that errors out if missing). Ship the 22 KB library.

### 3. Stage into this package

```sh
cp openvpn/src/openvpn/openvpn            openvpn_2.7.1/root/bin/openvpn
cp openssl/libssl.so.3                    openvpn_2.7.1/root/lib/libssl.so.3
cp openssl/libcrypto.so.3                 openvpn_2.7.1/root/lib/libcrypto.so.3
cp /usr/lib/arm-linux-gnueabi/libcap-ng.so.0.0.0 \
                                           openvpn_2.7.1/root/lib/libcap-ng.so.0
```

### 4. Verify

```sh
# version check
strings openvpn_2.7.1/root/bin/openvpn | grep '^OpenVPN 2\.'
# expected: OpenVPN 2.7.1 [git:...] arm-unknown-linux-gnueabi [SSL (OpenSSL)] [EPOLL] [MH/PKTINFO] [AEAD]

# dynamic dependencies
arm-linux-gnueabi-readelf -d openvpn_2.7.1/root/bin/openvpn | grep NEEDED
# expected exactly:
#   libcap-ng.so.0
#   libssl.so.3
#   libcrypto.so.3
#   libc.so.6
```

Any extra `NEEDED` entry means an optional feature snuck back in and needs to be chased down in `configure` output.
