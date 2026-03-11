#include "utils.h"

void usart_send(char c) {
	while (!(USART1_SR & (1 << 7))) {} // While can't send
	USART1_DR = c;
	for (int i = 0; i < 100000; i++) asm("nop");
}

char ch;
char usart_read() {
	if (USART1_SR & (1 << 5)) {
		ch = USART1_DR;
		return ch;
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

void print_ptr(void *ptr)
{
	unsigned long value = (unsigned long)ptr;
	usart_send('0');
	usart_send('x');
	int shift = (sizeof(unsigned long) * 8) - 4;
	int started = 0;
	while (shift >= 0)
	{
		unsigned long digit = (value >> shift) & 0xF;
		if (digit != 0 || started || shift == 0)
		{
			started = 1;
			if (digit < 10)
				usart_send('0' + digit);
			else
				usart_send('a' + (digit - 10));
		}
		shift -= 4;
	}
	usart_send('\n');
}


void print_int(int value)
{
    char buffer[12];   // enough for -2147483648
    int i = 0;

    if (value == 0)
    {
        usart_send('0');
        usart_send('\n');
        return;
    }
    if (value < 0)
    {
        usart_send('-');
        value = -value;
    }
    while (value > 0)
    {
        int digit = value % 10;
        buffer[i++] = '0' + digit;
        value /= 10;
    }
    while (i > 0)
    {
        usart_send(buffer[--i]);
    }
    usart_send('\n');
}

// FIXME Big assumption that both strings are null terminated lol
int strcmp(const char *str, const char *str2) {
	while (*str && *str2) {
		if (*str != *str2) {
			return 0;
		}
		str++;
		str2++;
	}
	if (*str != *str2)
		return 0;
	return 1;
}

void shell_send(char c) {
	if (c) {
		usart_send(c);
		if (c == '\r') { // If enter then do this
			// Process command
			usart_send('\n');
			usart_print("> ");
		}
	}
}
