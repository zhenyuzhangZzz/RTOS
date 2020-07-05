OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS {
	. = 0x30008000;

	.text ALIGN(4) : { cpu_build_in.o (.text) *(.text) *(.rodata.str1.4) }

	.data ALIGN(4) : { *(.data) }
	
	.rodata ALIGN(4) : { *(.rodata) }
	
	. = ALIGN(4);

	__begin_bss = .;
	.bss ALIGN(4) (NOLOAD): { *(.bss) }
	. = ALIGN(4);
	__end_bss = .;
}
