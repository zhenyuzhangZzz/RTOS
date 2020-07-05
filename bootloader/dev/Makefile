objs := dm9000.o led.o nand.o uart.o interrupt.o timer.o

all : $(objs)
	arm-linux-ld -r -o dev_build_in.o $^ $(LIBPATH)
	mv *.o $(BUILDPATH)
	
%.o : %.S
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $<  
	$(PRINTINFO) $<

%.o : %.c
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<
