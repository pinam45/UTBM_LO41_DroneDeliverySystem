#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void check(int result, const char* format, ...) {
	if (result < 0) {
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, ": %s/n",strerror(errno));
		exit(1);
	}
}
