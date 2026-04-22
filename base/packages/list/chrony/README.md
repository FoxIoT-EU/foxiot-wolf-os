# chrony

[Chrony](https://chrony-project.org/) **4.8** — NTP client/server daemon (`chronyd`) and command-line interface (`chronyc`). Statically configured for minimal footprint (no external crypto library, internal MD5 only), cross-compiled for Nuvoton NUC980 (ARM926EJ-S, armv5tej, soft-float EABI, `arm-linux-gnueabi`).

This package **only ships the two binaries** — each distro provides its own `/etc/chrony.conf` via its own `rootfs.list`. The package is not opinionated about NTP sources, drift file location, or `rtcsync`.

## What gets installed on the target

| Path | Source | Size (stripped) |
|---|---|---|
| `/bin/chronyd` | `root/bin/chronyd` | 264 KB |
| `/bin/chronyc` | `root/bin/chronyc` | 133 KB |

Verify with `arm-linux-gnueabi-readelf -d root/bin/chronyd`:
```
NEEDED  libm.so.6
NEEDED  libc.so.6
NEEDED  ld-linux.so.3
```
No external crypto library (no libssl/libnettle/libgnutls/libnss/libtomcrypt). No libcap, no libseccomp, no libreadline/libedit.

## Enabled / disabled features

The version string printed by `chronyd --version` contains:
```
+CMDMON -REFCLOCK +RTC -PRIVDROP -SCFILTER -SIGND -NTS -SECHASH -IPV6 -DEBUG
```

| Feature | State | Rationale |
|---|---|---|
| NTP client/server | ✓ (implicit) | core |
| `chronyc` remote control (`CMDMON`) | ✓ | needed for `chronyc sources`, `tracking`, etc. |
| RTC support (`FEAT_RTC`) | ✓ | future-proof: NUC980 hardware supports a battery-backed RTC; keeps `rtcfile`/`rtcautotrim` directives available even though current `chrony.conf` only uses `rtcsync` |
| Async DNS | ✓ (always) | auto-enabled by configure when pthreads are present; no flag to disable |
| Reference clocks (`REFCLOCK`) | ✗ | no hardware refclocks |
| PHC (PTP hardware clock) | ✗ | no PTP hardware on NUC980 |
| PPS | ✗ | no PPS input wired |
| IPv6 | ✗ | kernel has `# CONFIG_IPV6 is not set` |
| NTS (RFC 8915) | ✗ | not used by any customer config; would require an external crypto library |
| SECHASH (SHA2 symmetric keys) | ✗ | not used; MD5 internal backend is sufficient |
| `privdrop` + libcap | ✗ | chronyd runs as root in wolf-os |
| seccomp filter | ✗ | not configured |
| Samba NTP signing (`SIGND`) | ✗ | not used |
| SW/HW timestamping | ✗ | not used |
| readline / editline | ✗ | interactive `chronyc` editing not needed; one-shot commands still work |

## How to rebuild from source

Host prerequisites (Debian 10): `build-essential`, `git`, `pkg-config`, `gcc-arm-linux-gnueabi`.

### 1. Clone and checkout

```sh
git clone https://gitlab.com/chrony/chrony.git chrony-4.8
cd chrony-4.8
git checkout 4.8

# CRITICAL: set the version string. Upstream release tarballs ship a
# version.txt file with the version in it; git checkouts do not, and
# chrony's configure falls through to CHRONY_VERSION="DEVELOPMENT" if
# version.txt is missing. Without this step, `chronyd --version` will
# report "version DEVELOPMENT" instead of "version 4.8".
echo "4.8" > version.txt
```

**Note on the upstream URL**: older clones (e.g. `/data/foxiot/testing_stuff/chrony/`) pointed at `https://git.tuxfamily.org/chrony/chrony.git`, which was decommissioned. GitLab is now the canonical upstream.

### 2. Cross-compile

Chrony's `./configure` is a hand-rolled shell script (not autoconf), so cross-compilation is specified via `CC` + explicit `--host-system`/`--host-release`/`--host-machine` rather than an autoconf `--host=...` triple.

```sh
CC=arm-linux-gnueabi-gcc \
AR=arm-linux-gnueabi-ar \
CFLAGS="-Os -D_FORTIFY_SOURCE=2 -fPIE -fstack-protector-strong --param=ssp-buffer-size=4" \
LDFLAGS="-pie -Wl,-z,relro,-z,now" \
./configure \
    --prefix=/usr \
    --sysconfdir=/etc \
    --localstatedir=/var \
    --host-system=Linux \
    --host-release=6.12.76 \
    --host-machine=armv5tel \
    --without-nettle \
    --without-nss \
    --without-gnutls \
    --without-tomcrypt \
    --without-editline \
    --without-libcap \
    --without-seccomp \
    --without-aes-gcm-siv \
    --disable-readline \
    --disable-sechash \
    --disable-nts \
    --disable-refclock \
    --disable-phc \
    --disable-pps \
    --disable-ipv6 \
    --disable-privdrop \
    --disable-timestamping

make -j$(nproc)
arm-linux-gnueabi-strip chronyd chronyc
```

Expected configure output at the end:
```
Features : +CMDMON -REFCLOCK +RTC -PRIVDROP -SCFILTER -SIGND -NTS -READLINE -SECHASH -IPV6 -DEBUG
```

### 3. Stage into this package

```sh
cp chrony-4.8/chronyd chrony/root/bin/chronyd
cp chrony-4.8/chronyc chrony/root/bin/chronyc
```

### 4. Verify

```sh
# dependency check — must be ONLY libm, libc, ld-linux
arm-linux-gnueabi-readelf -d chrony/root/bin/chronyd | grep NEEDED
arm-linux-gnueabi-readelf -d chrony/root/bin/chronyc | grep NEEDED

# feature string — must match expected +/- set
strings chrony/root/bin/chronyd | grep '^[+-]CMDMON'
```

Any extra `NEEDED` line means an optional library got linked in — chase it down in the configure output (usually a forgotten `--without-*` flag).

## On-target smoke tests

```sh
# version + feature flags
chronyd --version

# running as a one-shot NTP client against the existing config
chronyd -q -f /etc/chrony.conf
# -q = exit after setting the clock once; -f = config path
# useful to verify NTP sources, DNS resolution, and clock step without leaving a daemon running

# query a running chronyd (if started by rc.local)
chronyc tracking
chronyc sources
chronyc ntpdata
```

## Related

- Previous package: `chrony/` (chrony 4.1 + 48 commits from Mar 2025, built from the now-dead `git.tuxfamily.org` mirror)
- Old build workspace: `/data/foxiot/testing_stuff/chrony/` — preserved as reference; remote is dead, don't try to `git fetch` it
- Current build workspace: `/data/foxiot/testing_stuff/chrony-4.8/`
