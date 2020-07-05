objs := arp.o net.o

all : $(objs)
	arm-linux-ld -r -o net_build_in.o $^
	mv *.o $(BUILDPATH)
	
%.o : %.S
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $<  
	$(PRINTINFO) $<

%.o : %.c
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<
