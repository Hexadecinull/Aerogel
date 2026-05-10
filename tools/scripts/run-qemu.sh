#!/usr/bin/env bash
set -euo pipefail

ARCH="${1:-x86_64}"
IMG="build/aerogel-${ARCH}.img"

if [ ! -f "$IMG" ]; then
    echo "Error: $IMG not found. Run the build first." >&2
    exit 1
fi

case "$ARCH" in
  x86)
    exec qemu-system-i386 \
      -drive file="$IMG",format=raw,if=ide \
      -m 256M \
      -serial stdio \
      -no-reboot \
      -no-shutdown
    ;;
  x86_64)
    OVMF=""
    for path in \
      /usr/share/OVMF/OVMF_CODE.fd \
      /usr/share/ovmf/OVMF.fd \
      /usr/share/edk2/ovmf/OVMF_CODE.fd; do
      [ -f "$path" ] && OVMF="$path" && break
    done

    if [ -z "$OVMF" ]; then
      echo "Error: OVMF firmware not found. Install ovmf package." >&2
      exit 1
    fi

    exec qemu-system-x86_64 \
      -M q35 \
      -drive file="$IMG",format=raw,if=ide \
      -drive if=pflash,format=raw,readonly=on,file="$OVMF" \
      -m 512M \
      -serial stdio \
      -no-reboot \
      -no-shutdown
    ;;
  *)
    echo "Usage: $0 [x86|x86_64]" >&2
    exit 1
    ;;
esac
