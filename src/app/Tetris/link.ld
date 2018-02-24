OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS
{
	. = 0x00;
	
	.text : 
	{
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
	}

	.data :
	{
		*(.data)
		*(.rodata*)
	}

	.bss  :
	{
		*(COMMON*)
		*(.bss*)
	}

}

