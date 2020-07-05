objs := main.o

all : $(objs)
	arm-linux-ld -r -o app_build_in.o $^ $(LIBPATH)
	mv *.o $(BUILDPATH)
	
%.o : %.S
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $<  
	$(PRINTINFO) $<

%.o : %.c
	arm-linux-gcc $(CFLAGS) $(INCPATH) -c $< 
	$(PRINTINFO) $<
