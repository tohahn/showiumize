#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "def.h"

#define LOG_DIR "/var/log/showiumize/"
#define LOG_FIRST_NAME "log"
#define LOG_SECOND_NAME "log.0"
#define LOG_MAX_LINES 254
#define LOG_MESSAGE "[LOG]"
#define LOG_ERROR "[ERROR]"
#define LOG_TIME_FORMAT "[%Y-%m-%d %H:%M:%S]"
#define LOG_TIME_BUFF_SIZE 22

unsigned char log_entries = 0, log_logging_enabled = TRUE;
FILE *log_first_p, *log_second_p;

unsigned char open_log(void);
unsigned char close_log(void);
unsigned char rotate_logs(void);
void get_log_time(char* log_time);
void write_message(const char* message, const char* type);
void write_log(const char* message);
void write_error(const char* message);

unsigned char open_log() {
	log_first_p = fopen(LOG_DIR LOG_FIRST_NAME, "w");
	if (log_first_p) {
		log_logging_enabled = TRUE;
		return TRUE;
	} else {
		printf("[ERROR] Log file could not be opened. Create directory %s with appropiate permissions. NO LOGGING FOR YOU!\n", LOG_DIR);
		log_logging_enabled = FALSE;
		return FALSE;
	}
}

unsigned char close_log() {
	unsigned char success = TRUE;
	
	if (log_first_p) {
		if (fclose(log_first_p)) {
			printf("[ERROR] First log file could not be closed.\n");
			success = FALSE;
		}
	}
	if (log_second_p) {
		if (fclose(log_second_p)) {
			printf("[ERROR] Second log file could not be closed.\n");
			success = FALSE;
		}
	}

	return success;
}

unsigned char rotate_logs() {
	int ch;
	
	log_entries = 0;

	log_first_p = freopen(LOG_DIR LOG_FIRST_NAME, "r", log_first_p);
	log_second_p = fopen(LOG_DIR LOG_SECOND_NAME, "w");

	if (!(log_first_p && log_second_p)) {
		printf("[ERROR] Couldn't open logs for rotating. Disabling logging.");
		close_log();
		log_logging_enabled = FALSE;
		return FALSE;
	}

	while (1) {
		ch = fgetc(log_first_p);
		if (ch == EOF) {
			break;
		} else {
			putc(ch, log_second_p);
		}
	}

	if (!close_log()) {
		printf("[ERROR] Logs could not be closed. Disabling logging.");
		log_logging_enabled = FALSE;
		return FALSE;
	}

	return open_log();
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
	if (log_logging_enabled) write_message(message, LOG_MESSAGE);
}

void write_error(const char* message) {
	fprintf(log_first_p, "ERROR NO: %d - ", errno);
	if (log_logging_enabled) write_message(message, LOG_ERROR);
}
