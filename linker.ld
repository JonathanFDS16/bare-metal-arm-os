ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000 , LENGTH = 128K 

    RAM   (rwx) : ORIGIN = 0x20000000 , LENGTH = 8K
}

SECTIONS
{
    .text :
    {
        *(.isr_vector) 
        *(.text)
        *(.rodata)
	} > FLASH  

    .data : { 
		. = ALIGN(4);
		*(.data)
		. = ALIGN(4);
		_edata = .;
	} > RAM   

    .bss :{ 
		. = ALIGN(4);
		*(.bss)
		. = ALIGN(4);
		_ebss = .;
	} > RAM


    _estack = ORIGIN(RAM) + LENGTH(RAM);
	_startheap = _ebss;
}
