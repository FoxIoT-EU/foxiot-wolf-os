# qfirehose

Ships `QFirehose` (Quectel's firehose/sahara flashing tool, upstream **V1.4.17**, source <https://github.com/nippynetworks/qfirehose>, commit fbbb4fe) with one local patch. Used to update the firmware of the Quectel EC25-EUX cellular module through the gateway — procedure in `docs/modem-firmware-update.md`. Cross-compiled for NUC980 (armv5tej, soft-float), dynamically linked against the image's glibc only, stripped, about 93 KB.

## Local patch

`0001-rndis-diag-interface.patch` (16 lines in `usb_linux.c`): upstream assumes the module's DIAG/DM port on USB interface 0, which only holds in the module's default composition. Wolf-OS runs the EC25 in RNDIS composition (`AT+QCFG="usbnet",3`), where interface 0 is the RNDIS control interface and DIAG is interface 2. The patch uses the first `Cls=ff Sub=ff Prot=ff` interface when interface 0 is not vendor-specific. Without it the switch to emergency-download mode fails with `USBDEVFS_SUBMITURB … No such file or directory`.

## Rebuild

Host: Docker with the `dockcross/linux-armv5` image (same as `docs/kernel-build.md`), `git`, `patch`.

### 1. Clone upstream

```sh
git clone https://github.com/nippynetworks/qfirehose
git -C qfirehose checkout fbbb4fe
```

### 2. Build, patch and stage

```sh
cd <this-package>
./build.sh /path/to/qfirehose
```

Note: the upstream source embeds the build time (`__DATE__`/`__TIME__` in the version banner), so two builds of the same source differ in a few bytes and have different hashes. The shipped binary is the one tested on 2026-09-23 (SHA-256 `fbf4ff841aebebbe1ba21480a9f96ae48a19f76b3474c128427c5ecf84277b08`); a rebuild is functionally identical.

`build.sh` applies the patch if `usb_linux.c` does not already contain it, runs the compiler inside dockcross with
`-Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables`, and copies the stripped binary to `root/bin/QFirehose`. It prints the SHA-256 of the result; record it in the release notes.

Manual equivalent:

```sh
cd qfirehose
patch -p1 < <this-package>/0001-rndis-diag-interface.patch
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/work -w /work dockcross/linux-armv5 bash -c \
  '$CC -Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wall \
   firehose_protocol.c qfirehose.c sahara.c usb_linux.c stream_download_protocol.c md5.c usb2tcp.c \
   -o QFirehose-armv5 -lpthread -ldl'
cp QFirehose-armv5 <this-package>/root/bin/QFirehose
```

### 3. PC-side binary

The operator's PC needs the same tool built natively (no patch required there, but harmless):

```sh
cd qfirehose && make linux     # or: gcc -Os -s <same sources> -o QFirehose -lpthread -ldl
```

## Check

`file root/bin/QFirehose` → `ELF 32-bit LSB executable, ARM, EABI5 … soft-float ABI … stripped`; `readelf -d` must list only `libc.so.6`. On a unit: `QFirehose -h` prints the version banner.
