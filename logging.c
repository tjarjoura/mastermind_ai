#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "logging.h"

char logfilename[20];
FILE *logfile = NULL;

void set_file(char *filename)
{
	strncpy(logfilename, filename, 20);
}

void write_log(char *msg, ...)
{
	char message_buffer[50];

	if (logfilename == NULL) {
		fprintf(stderr, "[ERROR] Set filename for logging: set_file(filename) \n");
		return;
	}
	logfile = fopen(logfilename, "a");
	va_list ap;
	va_start(ap, msg);
	vsnprintf(message_buffer, 50, msg, ap);
	va_end(ap);

	time_t t = time(NULL);
	char* time_str = ctime(&t);
	*(strchr(time_str, '\n')) = '\0';
	fprintf(logfile, "[%s]: %s\n", time_str, message_buffer);

	fclose(logfile);
}

