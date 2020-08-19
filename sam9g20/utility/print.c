#include "print.h"
#include <stdint.h>
#include <stdio.h>

void print_uart(const char *s) {
	uint16_t i = 0;

	while (*s != '\0') { /* Loop until end of string */
		s++;
		i++;
	}

	char data[i];
	s = s - i;
	i = 0;

	while (*s != '\0') { /* Loop until end of string */
		data[i] = *s;
		s++;
		i++;
	}
	data[i] = '\0';
	printf("%s", data);
}

void printChar(const char* character, bool errStream) {
	printf("%c", *character);
}
