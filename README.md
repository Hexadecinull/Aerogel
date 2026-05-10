# Aerogel OS

An open-source operating system built from scratch, targeting x86 (BIOS/MBR) and x86_64 (UEFI/GPT).

**License:** GPL-3.0  
**Status:** Early development

## Architecture

| Target | Boot | Partition |
|--------|------|-----------|
| x86    | BIOS | MBR       |
| x86_64 | UEFI | GPT       |

## Building

Requires: `nasm`, `i686-elf-gcc` (x86) or `x86_64-elf-gcc` (x86_64), `cmake >= 3.20`, `python3`

```sh
# x86
cmake -B build/x86 -DAERGEL_ARCH=x86 --toolchain tools/cmake/toolchain-x86.cmake
cmake --build build/x86

# x86_64
cmake -B build/x86_64 -DAERGEL_ARCH=x86_64 --toolchain tools/cmake/toolchain-x86_64.cmake
cmake --build build/x86_64
```

## Running (QEMU)

```sh
./tools/scripts/run-qemu.sh x86
./tools/scripts/run-qemu.sh x86_64
```

## Structure

```
arch/       Architecture-specific boot, CPU init, and memory management
kernel/     Architecture-agnostic kernel subsystems
drivers/    Device drivers
libs/       Freestanding libc and kernel utility library
userspace/  Init, shell, setup wizard
rust/       Rust workspace for safety-critical components
tools/      Build toolchains and scripts
docs/       Documentation
```

## License

GPL-3.0 — see [LICENSE](LICENSE)
