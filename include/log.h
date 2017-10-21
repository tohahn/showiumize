#ifndef LOG_H
#define LOG_H

/** INCLUDES **/
#include <stdio.h>

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_LOG "/var/log/showiumize/"
//FILES
#define FILE_LOG_FIRST "log"
#define FILE_LOG_SECOND "log.0"
//VALUES
#define MAX_LOG_LINES 254
#define LOG_TIME_BUFF_SIZE 22
//STRINGS
#define LOG_MESSAGE "[LOG]"
#define LOG_ERROR "[ERROR]"
//FORMAT_STRINGS
#define FORMAT_LOG_TIME "[%Y-%m-%d %H:%M:%S]"

/** METHODS **/
//FILE HANDLING
void open_log(void);
void close_log(void);
void rotate_logs(void);
//MESSAGE HANDLING
void get_log_time(char* log_time);
void write_message(const char* message, const char* type);
void write_log(const char* message);
void write_log(const char* message, ...);
void write_log(const char* message, va_list va);
void write_error(const char* message);
void write_error(const char* message, ...);
void write_error(const char* message, va_list va);

#endif
