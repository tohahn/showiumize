#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "definitions.h"
#include "messages.h"

unsigned char log_entries = 0;
FILE *log_first_p, *log_second_p;

void open_log() {
	log_first_p = fopen(DIR_LOG FILE_LOG_FIRST, "w");
	if (!log_first_p) {
		printf(ERROR_LOG_FILE_OPEN, DIR_LOG);
		exit(EXIT_FAILURE);
	}
}

void close_log() {
	if (fclose(log_first_p)) {
		write_error(ERROR_LOG_FILE_CLOSE_FIRST);
		exit(EXIT_FAILURE);
	}
	if (fclose(log_second_p)) {
		write_error(ERROR_LOG_FILE_CLOSE_SECOND);
		exit(EXIT_FAILURE);
	}
}

void rotate_logs() {
	log_first_p = freopen(DIR_LOG FILE_LOG_FIRST, "r", log_first_p);
	log_second_p = fopen(DIR_LOG FILE_LOG_SECOND, "w");

	if (!(log_first_p && log_second_p)) {
		write_error(ERROR_LOG_ROTATING);
		exit(EXIT_FAILURE);
	}

	char ch;
	while (1) {
		ch = fgetc(log_first_p);
		if (ch == EOF) {
			break;
		} else {
			putc(ch, log_second_p);
		}
	}

	close_log();
	open_log();
	log_entries = 0;
}

void get_log_time(char* log_time) {
	struct tm *sTm;

	time_t now = time(0);
	sTm = localtime(&now);

	strftime(log_time, LOG_TIME_BUFF_SIZE, LOG_TIME_FORMAT, sTm);
}

void write_message(const char* message, const char* type) {
	static char log_time_buff[LOG_TIME_BUFF_SIZE];
	get_log_time(log_time_buff);

	fprintf(log_first_p, "%s %s %s\n", log_time_buff, type, message);
	fflush(log_first_p);

	if (++log_entries > LOG_MAX_LINES) {
		rotate_logs();
	}
}

void write_log(const char* message) {
	write_message(message, LOG_MESSAGE);
}

void write_log(const char* message, ...) {
	va_list va;
	va_start(va, message);
	write_log(format, va);
	va_end(va);
}

void write_log(const char* message, va_list va) {
	char* msg = easy_printf(message, va);
	write_message(msg, LOG_MESSAGE);
	free(msg);
}

void write_error(const char* message) {
	write_message(message, LOG_ERROR);
}

void write_error(const char* message, ...) {
	va_list va;
	va_start(va, message);
	write_error(format, va);
	va_end(va);
}

void write_error(const char* message, va_list va) {
	char* msg = easy_printf(message, va);
	write_message(msg, LOG_ERROR);
	free(msg);
}
