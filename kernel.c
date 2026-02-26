typedef unsigned int uint32_t;
// 1. RCC (Power)
#define RCC_BASE_ADDR       0x40021000
#define RCC_APB2ENR         (*((volatile uint32_t *)(RCC_BASE_ADDR + 0x18)))

// 2. GPIO (Wiring)
#define GPIOA_BASE_ADDR     0x40010800
#define GPIOA_CRH           (*((volatile uint32_t *)(GPIOA_BASE_ADDR + 0x04)))

// 3. USART (Protocol)
#define USART1_BASE_ADDR    0x40013800
#define USART1_SR           (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x00))) // Status Register
#define USART1_DR           (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x04))) // Data Register
#define USART1_BRR          (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x08))) // Baud Rate
#define USART1_CR1          (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x0C))) // Control

#define NVIC_BASE 			0xE000E100
#define NVIC_ISER1          (*((volatile uint32_t *)(NVIC_BASE + 0x004))) // Control


void usart_print(char* str);
void usart_send(char c);
void shell_send(char c);

void interrupt_init() {
	// Enable NVIC
	NVIC_ISER1 |= (1 << 5);
}

void Usart_IRQHandler() {
	if (USART1_SR & (1 << 5)) {
		char r = USART1_DR;
		shell_send(r);
	}
}

void usart_send(char c) {
	while (!(USART1_SR & (1 << 7))) {} // While can't send
	USART1_DR = c;
	for (int i = 0; i < 100000; i++) asm("nop");
}

char c;
char usart_read() {
	if (USART1_SR & (1 << 5)) {
		c = USART1_DR;
		return c;
	}
	return 0;
}

void usart_print(char* str) {
	while (*str) {
		usart_send(*str);
		if (*str == '\r') usart_send('\n');
		str++;
	}
}


void shell_send(char c) {
	if (c) {
		usart_send(c);
		if (c == '\r') {
			usart_send('\n');
			usart_print("> ");
		}
	}
}

int main(void) {
    // 1. Enable Clocks (RCC)
    // We need Bit 14 (USART1) and Bit 2 (GPIOA)
    RCC_APB2ENR |= (1 << 14) | (1 << 2);

    // 2. Configure Pin PA9 (GPIO)
    // PA9 lives in CRH bits [7:4].
    // RESET STATE: The bits are usually 0100 (Input Floating).
    // TARGET: We want 1011 (Alternate Function Push-Pull 50MHz).
    // A. CLEAR the bits first (Safety Step)
    // We create a mask 1111 (0xF) shifted to position 4, then invert it (~).
    GPIOA_CRH &= ~(0xF << 4); 
    // B. SET the new bits
    // We want 1011 (0xB) shifted to position 4.
    GPIOA_CRH |= (0xB << 4);

    // 3. Configure Baud Rate (USART)
    // Target: 9600 Baud @ 8MHz Clock.
    // Calculation: 8000000 / 9600 = 833 (Decimal) = 0x341 (Hex)
    USART1_BRR = 0x341;

    // 4. Enable UART (USART)
    // Bit 13: UE (USART Enable)
    // Bit 5: RXNEIE (Rx not empty interrup enable)
    // Bit 3:  TE (Transmitter Enable)
    // Bit 2:  RE (Receiver Enable)
    USART1_CR1 |= (1 << 13) | (1 << 5) | (1 << 3) | (1 << 2);

	usart_print("> ");
	interrupt_init();
    while (1) {
		//usart_print("Hello\n");
		//char c = usart_read();
		//shell_send(c);

		long count = 0;
        for (long i = 0; i < 1000000000; i++) {
			asm("nop");
			count++;
		}
		usart_print("MULTITASKING!!!!!\n");
    }

    return 0;
}
