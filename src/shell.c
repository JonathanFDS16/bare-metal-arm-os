enum TOKEN_TYPE {
	STRING
};

typedef struct Commands {
	enum TOKEN_TYPE type;
	char *value;
} Commands ;


void lex_str(char *str) {
	while (*str) {
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
				while (*str) {

					if (*str == ' ') {
						
					}
				}
				break;
			default:
				//usart_print("Can't accept numbers \n");
		}
	}
}

void parse_cmd(char *str) {
	while (*str) {
		
	}
}
