MAKEFLAGS := -sR

RM := rm
PRINTINFO := @echo CC -[M]

TARGET :=           bootloader
TARGET_ELF_FILE :=  $(TARGET).elf
TARGET_BIN_FILE :=  $(TARGET).bin

cur_objs := cpu_build_in.o dev_build_in.o net_build_in.o app_build_in.o lib.o

LIBPATH := -lgcc -L/home/zzy/arm-cortex_a8-linux-gnueabi-4.9.4/lib/gcc/arm-cortex_a8-linux-gnueabi/4.9.4
CFLAGS := -Wall -W -O2 -P -nostdlib -fno-builtin -fno-ident -ffreestanding -Wno-unused-parameter -Wno-sign-conversion -Wunused-variable
INCPATH := -I ../include
LDFLAGS := --strip-debug -Map $(TARGET).map		#去除调试信息

BUILDPATH := $(shell pwd)
export LIBPATH CFLAGS INCPATH PRINTINFO BUILDPATH

all: $(cur_objs)

	arm-linux-ld -T bootloader.lds $^ -o $(TARGET_ELF_FILE) $(LDFLAGS) $(LIBPATH)

	arm-linux-objcopy -O binary $(TARGET_ELF_FILE) $(TARGET_BIN_FILE)
	
	arm-linux-objdump -D -S $(TARGET_ELF_FILE) > $(TARGET).dis

	./addheader $(TARGET_BIN_FILE) $(TARGET)head.bin
	
	@echo "congratulation, compile completed!"

cpu_build_in.o :
	make -C ../cpu/ all

dev_build_in.o :
	make -C ../dev/ all

net_build_in.o :
	make -C ../net/ all

app_build_in.o :
	make -C ../app/ all

lib.o :
	make -C ../lib/ all

addheader :
	gcc addheader.c -o addheader

clean :
	$(RM) -f *.o .*.d *.dis *.bin *.elf
