OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	. = 0x20008000;
	__begin_kernel = .;
	.head.text ALIGN(4) : { *(.head.text) }

	.lmosem_hal.vector ALIGN(4) : { *(.lmosem_hal.vector) }

	.lmosem_hal.intvect ALIGN(4) : { *(.lmosem_hal.intvect) }
	
	__begin_text = .;
	.text ALIGN(4) : { *(.text) *(.rodata.str1.4)}
	__end_text = .;

	__begin_data = .;
	.data ALIGN(4) : { *(.data) }
	__end_data = .;

	__begin_rodata = .;
	.rodata ALIGN(4) : { *(.rodata) }
	__end_rodata = .;

	__begin_bss = .;

	.bss ALIGN(4) : 
	{ 	
		*(.shbss)
  		*(.bss .bss.* .gnu.linkonce.b.*)
  		*(COMMON)
  	}
	. = ALIGN(4);
	__end_bss = .;
	__end_kernel = .;
}