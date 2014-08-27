#==================================================================================================
#					Makefile for bzOS					  =
#==================================================================================================
#									2014-8-22	Bill Liang=
#==================================================================================================

ENTRYPOINT	= 0x30400
ENTRYOFFSET	= 0x400


ASM		= nasm
DASM		= ndisasm
LD		= ld
CC		= gcc

DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
LDFLAGS		= -s -Ttext $(ENTRYPOINT)
CFLAGS		= -I include/ -c -fno-builtin -fno-stack-protector

# 文件
BOOTINCLUDE	= boot/include/fat12hdr.inc boot/include/load.inc boot/include/pm.inc
OBJS		= kernel/kernel.o kernel/start.o kernel/i8259.o kernel/protect.o\
		  kernel/global.o kernel/main.o kernel/clock.o\
		  lib/klib.o lib/kliba.o lib/string.o

BZOSBOOT	= boot/boot.bin boot/loader.bin
BZOSKERNEL	= kernel/kernel.bin

DASMOUTPUT	= kernel.bin.asm

# 所有伪目标行为
.PHONY: everything clean realclean all final image disasm building

# 默认的make开始位置
everything: $(BZOSBOOT) $(BZOSKERNEL)

clean:
	rm -f $(OBJS)

realclean:
	rm -f $(OBJS) $(BZOSBOOT) $(BZOSKERNEL)

all: realclean everything

final: all clean

image: final building

disasm:
	$(DASM) $(DASMFLAGS) $(BZOSKERNEL) > $(DASMOUTPUT)

#假设当前目录存在“a.img”软盘镜像文件。
building:
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/
	sudo cp -fv boot/loader.bin kernel/kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy


boot/boot.bin: boot/boot.asm $(BOOTINCLUDE)
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: boot/loader.asm $(BOOTINCLUDE)
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(BZOSKERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

kernel/kernel.o: kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o: kernel/start.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c
	$(CC) $(CFLAGS) -o $@ $<

lib/klib.o: lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<	

lib/kliba.o: lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

