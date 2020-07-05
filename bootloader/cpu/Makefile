objs := start.o cpuinit.o

#$(filter-out start.o, $(objs))

all : $(objs)
	arm-linux-ld -r -o cpu_build_in.o $^ 
	mv *.o $(BUILDPATH)
	
%.o : %.S
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $<  
	$(PRINTINFO) $<

%.o : %.c
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<
