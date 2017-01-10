
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_Reset)


SECTIONS
{
	/* =========== CODE section =========== */
	. = 0x00100000;

	. = ALIGN(4);
	.text :
	{
		__text_start = .;  	
		bootloader.o (.text*)
		*(.text*)
	}

	/* =========== RODATA section =========== */

	.rodata ALIGN(0x1000) :
	{
		*(.rodata)
		*(.roda)
		*(.rodata.*)
		*all.rodata*(*)
		*(.gnu.linkonce.r*)	
		. = ALIGN(4);
	}

	.ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
	__exidx_start = .;
	ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
	__exidx_end = .;

	/* =========== DATA section =========== */

	. = ALIGN(4);
	.data :
	{ 
		*(.data)
		. = ALIGN(4);
	}
	
	.tdata ALIGN(4) :
	{
		__tdata_lma = .;
		*(.tdata)
		*(.tdata.*)
		*(.gnu.linkonce.td.*)
		. = ALIGN(4);
		__tdata_lma_end = .;
	}

	.tbss ALIGN(4) :
	{
		*(.tbss)
		*(.tbss.*)
		*(.gnu.linkonce.tb.*)
		*(.tcommon)
		. = ALIGN(4);
	}
	. = ALIGN(4);

	.rel.plt ALIGN(4):
	{
		*(.__rel_dyn_start)
		*(.rel*)
		*(.rel.*)
		*(.rel.dyn)
		*(.__rel_dyn_end)
	}


	__code_end = .;



	. = ALIGN(4);	
	.bss :
	{ 
		*(.__bss_start)
		*(.bss COMMON) 
		*(.__bss_end)
		/* Reserve space for the TLS segment of the main thread */
		__tls_start = .;
		. += + SIZEOF(.tdata) + SIZEOF(.tbss);
		__tls_end = .;
	}
	__end__ = .;
}
