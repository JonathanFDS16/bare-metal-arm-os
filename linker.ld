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

	_sdata_flash = LOADADDR(.data);

    .data : { 
		. = ALIGN(4);
		_sdata = .;
		*(.data)
		. = ALIGN(4);
		_edata = .;
	} > RAM AT> FLASH

    .bss :{ 
		. = ALIGN(4);
		_sbss = .;
		*(.bss)
		. = ALIGN(4);
		_ebss = .;
	} > RAM


    _estack = ORIGIN(RAM) + LENGTH(RAM);
	_startheap = _ebss;
}
