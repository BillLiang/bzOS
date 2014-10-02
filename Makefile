#==================================================================================================
#			Makefile for bzOS		BillLiang	2014-8-22
#==================================================================================================

ENTRYPOINT	= 0x30400
ENTRYOFFSET	= 0x400


ASM		= nasm
DASM		= ndisasm
LD		= ld
CC		= gcc

DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -I include/sys/ -f elf
LDFLAGS		= -s -Ttext $(ENTRYPOINT)
CFLAGS		= -I include/ -I include/sys/ -c -fno-builtin -fno-stack-protector

# 文件
BOOTINCLUDE	= boot/include/fat12hdr.inc boot/include/load.inc boot/include/pm.inc
OBJS		= kernel/kernel.o kernel/start.o kernel/i8259.o kernel/protect.o \
		  kernel/global.o kernel/main.o kernel/clock.o kernel/proc.o kernel/syscall.o \
		  kernel/keyboard.o kernel/tty.o kernel/console.o kernel/printf.o kernel/systask.o \
		  kernel/hd.o \
		  fs/main.o  fs/misc.o fs/open.o fs/read_write.o \
		  lib/klib.o lib/kliba.o lib/string.o lib/misc.o \
		  lib/open.o lib/close.o lib/read.o lib/write.o

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

kernel/kernel.o: kernel/kernel.asm
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

kernel/proc.o: kernel/proc.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/syscall.o: kernel/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/systask.o: kernel/systask.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/hd.o: kernel/hd.c
	$(CC) $(CFLAGS) -o $@ $<

fs/main.o: fs/main.c
	$(CC) $(CFLAGS) -o $@ $<

fs/misc.o: fs/misc.c
	$(CC) $(CFLAGS) -o $@ $<

fs/open.o: fs/open.c
	$(CC) $(CFLAGS) -o $@ $<

fs/read_write.o: fs/read_write.c
	$(CC) $(CFLAGS) -o $@ $<

lib/klib.o: lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<	

lib/kliba.o: lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/misc.o: lib/misc.c
	$(CC) $(CFLAGS) -o $@ $<

lib/open.o: lib/open.c
	$(CC) $(CFLAGS) -o $@ $<

lib/close.o: lib/close.c
	$(CC) $(CFLAGS) -o $@ $<

lib/read.o: lib/read.c
	$(CC) $(CFLAGS) -o $@ $<

lib/write.o: lib/write.c
	$(CC) $(CFLAGS) -o $@ $<
