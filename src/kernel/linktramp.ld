OUTPUT_FORMAT("elf32-i386")
ENTRY(ini)

SECTIONS
{
	. = 0x1000;

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

	.bss :
	{
		*(COMMON*)
		*(.bss*)
	}
}

