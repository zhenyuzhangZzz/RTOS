MAKEFLAGS = -s

include buildfile.mkh 

TARGET :=          	emoskrnl
TARGET_ELF_FILE :=  $(TARGET).elf
TARGET_BIN_FILE :=  $(TARGET).bin

LDFLAGS := --strip-debug -Map $(TARGET).map					#去除调试信息

all :
	$(LD) -T lmosemlink.lds $(BUILD_MK_LINK_OBJS) -o $(TARGET_ELF_FILE) $(LDFLAGS)

	 @echo LD $(TARGET_ELF_FILE) from '$(BUILD_MK_LINK_OBJS)'

	$(OBJCOPY) -O binary $(TARGET_ELF_FILE) $(TARGET_BIN_FILE)

	@echo 'OBJCOPY $(TARGET_ELF_FILE) to $(TARGET_BIN_FILE)'

	$(OBJDUMP) -D -S $(TARGET_ELF_FILE) > $(TARGET).dis

	@echo 'OBJDUMP $(TARGET).dis'