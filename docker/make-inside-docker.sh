#!/bin/sh
set -e

if [ -z "$DISTRO" ]; then
  echo "Error: DISTRO environment variable not set"
  exit 1
fi

echo "📦 Building distro: $DISTRO"

# Build signing tools if not already built
if [ ! -x util/fw-sign/fw-sign ]; then
  echo "Building firmware signing tools..."
  /usr/bin/make -C util/fw-sign host
fi

# Build
/usr/bin/make -C "distro/$DISTRO" clean
/usr/bin/make -C "distro/$DISTRO"

mkdir -p build
cp distro/$DISTRO/root_*.itb build/.
