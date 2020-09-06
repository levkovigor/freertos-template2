#include "print.h"
#include <stdio.h>

void printChar(const char* character, bool errStream) {
	if(errStream) {
		putc(*character, stderr);
		return;
	}
	putc(*character, stdout);
}




