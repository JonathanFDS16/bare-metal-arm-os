-- MY SYSTEM MAP --
FLASH_START_ADDR: Main memory below
Page 1:    0x0800 0000 - 0x0800 03FF Size: 1k
Page 127:  0x0801 FC00 - 0x0801 FFFF Size: 1k

SRAM_START_ADDR:  0x2000 0000  The SRAM start address is 0x2000 0000.

-- PERIPHERAL ADDRESSES --
RCC_BASE_ADDR:    0x________ 0x4002 1000 - 0x4002 13FF Reset and clock control RCC
GPIOA_BASE_ADDR:  0x________ 0x4001 0800 - 0x4001 0BFF GPIO Port A
USART1_BASE_ADDR: 0x________ 0x4001 3800 - 0x4001 3BFF USART1

-- REGISTERS OFFSETS (Add to Base Addr) --
RCC_APB2ENR_OFFSET: 0x________ offset: 0x18
GPIOA_CRH_OFFSET:   0x________ offset: 0x04
USART1_DR_OFFSET:   0x________ offset: 0x04


NVIC register block is 0xE000E100
NVIC_ISER1 offset 0x004


Cool todos:
1. malloc and free
2. running two different processes, savings stack and etc
3. how would I create a syscall into this kernel so user programs can call and receive something back? Trap instruction
4. write syscall
