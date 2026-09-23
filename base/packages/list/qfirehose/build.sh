#!/bin/sh
# Cross-build Quectel QFirehose for the Wolf Gateway (ARMv5, soft-float, glibc).
# Uses the same dockcross image as docs/kernel-build.md.
#
# Usage: ./build.sh [path-to-qfirehose-source]
#   Source: https://github.com/nippynetworks/qfirehose  (tested at upstream commit fbbb4fe)
#   The FoxIoT patch 0001-rndis-diag-interface.patch is applied if not already present.
# Output:  QFirehose (stripped) copied to base/packages/list/qfirehose/root/bin/QFirehose
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$(cd "$HERE" && cd "${1:-../../../../../qfirehose}" && pwd)"
PKG_BIN="$HERE/root/bin"
SOURCES="firehose_protocol.c qfirehose.c sahara.c usb_linux.c stream_download_protocol.c md5.c usb2tcp.c"
CFLAGS="-Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wall"
if ! grep -q "diag_candidate" "$SRC_DIR/usb_linux.c"; then
  echo "applying FoxIoT patch"
  patch -d "$SRC_DIR" -p1 < "$HERE/0001-rndis-diag-interface.patch"
fi
docker run --rm -u "$(id -u):$(id -g)" -v "$SRC_DIR":/work -w /work dockcross/linux-armv5 \
  bash -c "\$CC $CFLAGS $SOURCES -o QFirehose-armv5 -lpthread -ldl"
mkdir -p "$PKG_BIN"
cp "$SRC_DIR/QFirehose-armv5" "$PKG_BIN/QFirehose"
ls -l "$PKG_BIN/QFirehose"
sha256sum "$PKG_BIN/QFirehose"
