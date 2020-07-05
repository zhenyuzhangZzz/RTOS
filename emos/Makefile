MAKEFLAGS =-sR

CROSS_COMPILE = arm-linux-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP

MKDIR = mkdir
RMDIR = rmdir
CP = cp
CD = cd
RM = rm
MKAE = make 
PRINTINFO := @echo CC 
export MKDIR RMDIR CP CD RM MAKE PRINTINFO

CFLAGS := -O2 -P -fno-builtin -std=c99 -ffreestanding -Wall -Wshadow -W -Wconversion -Wno-sign-conversion -fno-stack-protector \
		  -fomit-frame-pointer -fno-ident -Wno-unused-parameter -Wunused-variable -nostdlib -Winline #-save-temps

export CFLAGS PRINTINFO CFLAGS

BUILD_PATH := ./build 

ARMHALYMFLGS = -C $(BUILD_PATH) -f lmosemhal.mk
ARMKRNLMFLGS = -C $(BUILD_PATH) -f lmosemcore.mk
ARMDRIVMFLGS = -C $(BUILD_PATH) -f lmosemdrv.mk
ARMLIBSMFLGS = -C $(BUILD_PATH) -f lmosemlib.mk
ARMTASKMFLGS = -C $(BUILD_PATH) -f lmosemtask.mk
ARMLINKMFLGS = -C $(BUILD_PATH) -f lmosemlink.mk

.PHONY: all clean

all:
	$(MAKE) $(ARMHALYMFLGS)
	$(MAKE) $(ARMKRNLMFLGS)
	$(MAKE) $(ARMDRIVMFLGS)
	$(MAKE) $(ARMLIBSMFLGS)
	#$(MAKE) $(ARMTASKMFLGS)
	$(MAKE) $(ARMLINKMFLGS)
	@echo 'congratulation, compile completed!'

clean:
	$(CD) $(BUILD_PATH); $(RM) -f *.o *.bin *.i *.elf *.map *.lib *.dis .*.d *.s *.i 
	@echo "target clean completed."



