#!/usr/bin/env bash
set -euo pipefail

ARCH="${1:-x86_64}"
OUT="build/aerogel-${ARCH}.iso"
IMG="build/aerogel-${ARCH}.img"

if [ ! -f "$IMG" ]; then
    echo "Error: $IMG not found." >&2
    exit 1
fi

command -v xorriso >/dev/null || { echo "Error: xorriso not installed." >&2; exit 1; }

ISOROOT=$(mktemp -d)
trap 'rm -rf "$ISOROOT"' EXIT

cp "$IMG" "$ISOROOT/aerogel.img"

xorriso -as mkisofs \
  -o "$OUT" \
  -V "AEROGEL_${ARCH^^}" \
  "$ISOROOT"

echo "ISO written to $OUT"
