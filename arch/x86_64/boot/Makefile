CC      := x86_64-w64-mingw32-gcc
CFLAGS  := -ffreestanding -fno-stack-protector -fno-builtin -fshort-wchar \
           -mno-red-zone -O2 -Wall -Wextra -I.
LD      := x86_64-w64-mingw32-ld
LDFLAGS := --subsystem 10 --dll --image-base 0 -e efi_main

.PHONY: all clean

all: bootx64.efi

bootx64.efi: efi_main.o
	$(LD) $(LDFLAGS) -o $@ $^

efi_main.o: efi_main.c efi.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f efi_main.o bootx64.efi
