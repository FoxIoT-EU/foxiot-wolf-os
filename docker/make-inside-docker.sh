#!/bin/sh
set -e

if [ -z "$DISTRO" ]; then
  echo "Error: DISTRO environment variable not set"
  exit 1
fi

echo "📦 Building distro: $DISTRO"

# Build
/usr/bin/make -C "distro/$DISTRO" clean
/usr/bin/make -C "distro/$DISTRO"

mkdir -p build
cp distro/$DISTRO/root_*.itb build/.
