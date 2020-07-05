MAKEFLAGS = -s
CCBUILDPATH	= ../lib/

HEADFILE_PATH = -I ../include -I ../include/commoninc -I ../include/libinc
CFLAGS += $(HEADFILE_PATH)
dep_file = .$@.d

include buildfile.mkh 

all: $(BUILD_MK_LIBS_OBJS)

dep_files := $(foreach f,$(BUILD_MK_LIBS_OBJS),.$(f).d)
dep_files := $(wildcard $(dep_files))

ifneq ($(dep_files),)
  include $(dep_files)
endif

%.o : $(CCBUILDPATH)%.c
	$(CC) $(CFLAGS) -Wp,-MD,$(dep_file) -c $< 
	$(PRINTINFO) $<

%.o : $(CCBUILDPATH)%.S
	$(CC) $(CFLAGS) -Wp,-MD,$(dep_file) -c $< 
	$(PRINTINFO) $<
