#include "utils.h"
#include "syscalls.h"

#define MAX_TOKEN_AMOUNT 10
#define MAX_BUFF_AMOUNT 256

#define CARRIAGE_RETURN 127

enum TOKEN_TYPE {
	HELP,
	REBOOT,
	ARG,
	CR
};

typedef struct Token {
	enum TOKEN_TYPE type;
	char *value;
} Token ;

Token tokens[MAX_TOKEN_AMOUNT] = {0};

//FORWARD DECLARATIONS
void lex_line(char *str, int max);
void parse_tokens(Token *tokens);
void help();
void reboot();
void invalid_input();

void run_shell() {
	usart_print("> ");
	char buf[MAX_BUFF_AMOUNT];
	int i = 0;
	while (1) {
		char c = poll_usart();
		if (c == 0)
			continue;
		if (c == CARRIAGE_RETURN) {
			usart_print("\033[1D"); // move back
			usart_print("\033[J");  // delete char
			if (i) {
				buf[--i] = '\0';
			}
			continue;
		}


		usart_send(c);
		buf[i++] = c;
		if (c == '\r') {
			buf[i++] = '\0';
			usart_print("\n");
			lex_line(buf, MAX_BUFF_AMOUNT);
			parse_tokens(tokens);
			i = 0;
			usart_print("> ");
		}
	}
}


Token get_token(char *token) {
	if (strcmp(token, "help")) {
		return (Token){HELP, 0};
	}
	else if (strcmp(token, "reboot")) {
		return (Token){REBOOT, 0};
	}
	else if (*token == '\r') {
		return (Token){CR, 0};
	}
	return (Token){ARG, 0}; //FIXME the arg should contain a value for future args in shell
}

void lex_line(char *str, int max) {
	int token_i = 0;
	int buf_i = 0;
	char buf[64];
	while (*str) {
		//print_int(*str);
		switch (*str) {
			// letter accumulate until whitespace
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'x':
			case 'y':
			case 'z':
				{
					while (*str) {
						if (*str == ' ' || *str == '\r') {
							buf[buf_i] = '\0';
							tokens[token_i++] = get_token(buf);
						}
						else {
							buf[buf_i++] = *str;
						}
						str++;
					}
				}
				break;
			default:
				str++;
				break;
		}
	}
}

void parse_tokens(Token *tokens) {
	// Let's assume one token for now
	switch (tokens->type) {
		case REBOOT:
			reboot();
			break;
		case HELP:
			// gather all tokens [cmd] [args...]
			help();
			break;
		default:
			invalid_input();
			break;
	}
}

void help() {
	usart_print("HELPING YOU IS A PLEASURE (Not Implemented) :)\n");
}

void reboot() {
	usart_print("REBOOTING THIS MACHINE IS CRAZY WORK :D (Not Implemented)\n");
}

void invalid_input() {
	usart_print("Invalid Input :(\n");
}
