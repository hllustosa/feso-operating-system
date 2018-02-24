OUTPUT_FORMAT("elf32-i386")
ENTRY(start)

SECTIONS
{
	. = 0x100000;

	.setup :
	{
		*(.setup)
	}

	. += 0xC0000000;

	.text : AT(ADDR(.text) - 0xC0000000)
	{
		code = .;
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
	}

	.data ALIGN (4096) : AT(ADDR(.data) - 0xC0000000)
	{
	    data = .;
		*(.data)
		*(.rodata*)
	}

	.bss ALIGN (4096) : AT(ADDR(.bss) - 0xC0000000)
	{
	    bss = .;
		*(COMMON*)
		*(.bss*)
	}
	
	end = . - 0xC0000000;
	
	
}

