#include <stdarg.h>
#include <stdio.h>

#include "log.h"

extern int system_log_level;

void
syslog(int level, const char *fmt, ...) {
	va_list ap;

	if (system_log_level > level) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}
