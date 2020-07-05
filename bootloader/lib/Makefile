objs := lib.o

LIBPATH := -lgcc -L/home/mark/work/tool/tool_chain/arm-cortex_a8-linux-gnueabi-4.9/lib/gcc/arm-cortex_a8-linux-gnueabi/4.9.4

all : $(objs)
	mv *.o $(BUILDPATH)
	
%.o : %.S
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<

%.o : %.c
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<
