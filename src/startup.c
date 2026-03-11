typedef unsigned int uint32_t;
extern uint32_t _sdata_flash; // Start of .data in Flash
extern uint32_t _sdata;  // Start of .data in RAM
extern uint32_t _edata;  // End of .data in RAM
extern uint32_t _sbss;   // Start of .bss in RAM
extern uint32_t _ebss;   // End of .bss in RAM

extern int _start(void *heap_start);

void Reset_Handler(void) {
    uint32_t *src;
    uint32_t *dest;

    // 1. Copy the .data section from Flash to RAM
    src = &_sdata_flash;
    dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }

    // 2. Clear the .bss section in RAM to 0
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    void *heap_start_addr = (void *)&_ebss;
    _start(heap_start_addr);

    while (1);
}
