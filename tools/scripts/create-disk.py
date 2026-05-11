#!/usr/bin/env python3
import argparse, os, struct, subprocess, sys, tempfile, uuid

SECTOR = 512

def make_mbr_image(kernel_bin, stage2_bin, mbr_bin, out):
    if len(mbr_bin) != SECTOR:
        sys.exit(f"mbr.bin must be exactly {SECTOR} bytes")

    DISK_MB  = 64
    DATA_LBA = 2048
    disk = bytearray(DISK_MB * 1024 * 1024)

    disk[0:SECTOR] = mbr_bin

    DATA_MB    = 32
    data_sects = DATA_MB * 1024 * 1024 // SECTOR
    part = struct.pack("<BBBBBBBBII",
        0x00, 0x00, 0x20, 0x21,
        0x0C,
        0x00, 0xFE, 0xFF,
        DATA_LBA, data_sects)
    disk[446:462] = part
    disk[510] = 0x55
    disk[511] = 0xAA

    disk[SECTOR : SECTOR + len(stage2_bin)] = stage2_bin

    kernel_lba = 17
    disk[SECTOR * kernel_lba : SECTOR * kernel_lba + len(kernel_bin)] = kernel_bin

    import shutil
    have_tools = all(shutil.which(t) for t in ("mkfs.fat", "mmd", "mcopy"))

    if have_tools:
        esp_bytes = data_sects * SECTOR
        with tempfile.TemporaryDirectory() as td:
            img = os.path.join(td, "data.img")
            with open(img, "wb") as f:
                f.write(b"\x00" * esp_bytes)
            subprocess.run(["mkfs.fat", "-F", "32", "-n", "AEROGEL_DAT", img],
                           check=True, capture_output=True)
            subprocess.run(["mmd", "-i", img, "::aerogel"],
                           check=True, capture_output=True)
            subprocess.run(["mcopy", "-i", img, "-",
                            "::aerogel/readme.txt"],
                           input=b"Aerogel OS data partition\n",
                           check=True, capture_output=True)
            with open(img, "rb") as f:
                fat = f.read()
        disk[DATA_LBA * SECTOR : DATA_LBA * SECTOR + len(fat)] = fat

    with open(out, "wb") as f:
        f.write(disk)

    print(f"[x86]  {out}")
    print(f"       mbr@LBA0  stage2@LBA1  kernel@LBA17  FAT32@LBA{DATA_LBA}")


def make_gpt_image(efi_bin, kernel_bin, out):
    for cmd in ("mkfs.fat", "mmd", "mcopy"):
        import shutil
        if not shutil.which(cmd):
            sys.exit(f"Required: {cmd}  (install dosfstools + mtools)")

    IMG_MB    = 128
    ESP_MB    = 32
    ESP_START = 2048
    ESP_SECTS = ESP_MB * 1024 * 1024 // SECTOR
    disk = bytearray(IMG_MB * 1024 * 1024)

    DISK_GUID = uuid.uuid4().bytes_le
    PART_GUID = uuid.uuid4().bytes_le
    EFI_TYPE  = bytes.fromhex("28732AC11FF8D211BA4B00A0C93EC93B")
    esp_end   = ESP_START + ESP_SECTS - 1
    last_lba  = len(disk) // SECTOR - 1

    part = (EFI_TYPE + PART_GUID +
            struct.pack("<QQ", ESP_START, esp_end) +
            struct.pack("<Q", 0x04) +
            "EFI System Partition".encode("utf-16-le").ljust(72, b"\x00"))

    import binascii
    crc_part = binascii.crc32(part.ljust(128, b"\x00")) & 0xFFFFFFFF

    def gpt_header(primary, part_lba, other_lba):
        hdr = struct.pack("<8sIIIIQQQQ16sQIII",
            b"EFI PART", 0x00010000, 92, 0, 0,
            1 if primary else last_lba,
            last_lba if primary else 1,
            ESP_START, last_lba - 1,
            DISK_GUID, part_lba, 1, 128, crc_part)
        crc = binascii.crc32(hdr) & 0xFFFFFFFF
        return hdr[:16] + struct.pack("<I", crc) + hdr[20:]

    disk[SECTOR:SECTOR*2]   = gpt_header(True, 2, last_lba)
    disk[SECTOR*2:SECTOR*3] = part.ljust(128, b"\x00")
    bkp = last_lba - 1
    disk[bkp*SECTOR:(bkp+1)*SECTOR]         = part.ljust(128, b"\x00")
    disk[last_lba*SECTOR:(last_lba+1)*SECTOR] = gpt_header(False, bkp, 1)

    with tempfile.TemporaryDirectory() as td:
        img = os.path.join(td, "esp.img")
        esp_bytes = ESP_SECTS * SECTOR
        with open(img, "wb") as f:
            f.write(b"\x00" * esp_bytes)
        subprocess.run(["mkfs.fat", "-F", "32", "-n", "AEROGEL_EFI", img],
                       check=True, capture_output=True)
        subprocess.run(["mmd", "-i", img, "::EFI"],       check=True, capture_output=True)
        subprocess.run(["mmd", "-i", img, "::EFI/BOOT"],  check=True, capture_output=True)
        subprocess.run(["mmd", "-i", img, "::aerogel"],   check=True, capture_output=True)
        if efi_bin:
            subprocess.run(["mcopy", "-i", img, efi_bin,
                            "::EFI/BOOT/BOOTX64.EFI"], check=True, capture_output=True)
        if kernel_bin:
            subprocess.run(["mcopy", "-i", img, kernel_bin,
                            "::aerogel/kernel.bin"], check=True, capture_output=True)
        with open(img, "rb") as f:
            esp = f.read()

    disk[ESP_START * SECTOR : ESP_START * SECTOR + len(esp)] = esp

    os.makedirs(os.path.dirname(os.path.abspath(out)) or ".", exist_ok=True)
    with open(out, "wb") as f:
        f.write(disk)
    print(f"[x86_64] {out}  ESP@LBA{ESP_START} ({ESP_MB} MiB FAT32)")


def load(path):
    if path and os.path.exists(path):
        with open(path, "rb") as f:
            return f.read()
    return None


def main():
    ap = argparse.ArgumentParser(description="Aerogel disk image builder")
    ap.add_argument("--arch",   required=True, choices=["x86", "x86_64"])
    ap.add_argument("--output", required=True)
    ap.add_argument("--mbr",    default="arch/x86/boot/mbr.bin")
    ap.add_argument("--stage2", default="arch/x86/boot/stage2.bin")
    ap.add_argument("--efi",    default=None)
    ap.add_argument("--kernel", default=None)
    args = ap.parse_args()

    if args.arch == "x86":
        mbr = load(args.mbr)
        if not mbr:
            sys.exit("mbr.bin not found")
        stage2 = load(args.stage2) or b""
        kernel = load(args.kernel) or b""
        make_mbr_image(kernel, stage2, mbr, args.output)
    else:
        make_gpt_image(args.efi, args.kernel, args.output)


if __name__ == "__main__":
    main()
