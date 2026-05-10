AS      := nasm
ASFLAGS := -f bin -i $(CURDIR)/

.PHONY: all clean

all: mbr.bin stage2.bin

mbr.bin: mbr.asm
	$(AS) $(ASFLAGS) -o $@ $<

stage2.bin: stage2.asm a20.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f mbr.bin stage2.bin
