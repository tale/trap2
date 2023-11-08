#include "log.h"

void log_internal(const char *prefix, const char *fmt, FILE *file, va_list args) {
	fprintf(file, "%s: ", prefix);
	vfprintf(file, fmt, args);
	fprintf(file, "\n");
}

void log_info(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_internal("INFO", fmt, stdout, args);
	va_end(args);
}

void log_warn(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_internal("WARN", fmt, stdout, args);
	va_end(args);
}

void log_error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_internal("ERRO", fmt, stdout, args);
	va_end(args);
}
