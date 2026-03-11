#ifndef UTILS_H
#define UTILS_H

#define USART1_BASE_ADDR    0x40013800
#define USART1_SR           (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x00))) // Status Register
#define USART1_DR           (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x04))) // Data Register
#define USART1_BRR          (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x08))) // Baud Rate
#define USART1_CR1          (*((volatile uint32_t *)(USART1_BASE_ADDR + 0x0C))) 

typedef unsigned int size_t;
typedef unsigned int uint32_t;

void usart_send(char c);
char usart_read();

void usart_print(char* str);

void print_ptr(void *ptr);
void print_int(int value);

int strcmp(const char *str, const char *str2);

void shell_send(char c);
#endif
