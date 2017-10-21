#ifndef UTILS_H
#define UTILS_H

/** INCLUDES **/
//STDLIB
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
//CUSTOM
#include "definitions.h"
#include "structs.h"

/** DEFINITIONS **/
//FORMATS
#define FORMAT_ERROR_NUMBER "%s - ERROR NUMBER: %d"
//TOKENS
#define TOKEN_CHAR_FORMAT '%'
//VALUES
#define VALUE_VARARG_SIZE 1000
#define FILE_READ_MODE "r"
#define FILE_WRITE_MODE "w"
#define FILE_READ_WRITE "w+"
#define DIR_READ_WRITE 0755

/** METHODS **/
//FILES
FILE* open_file(const char* filename, const char* mode);
FILE* open_temp_file(void);
unsigned char close_file(FILE* file);
unsigned char remove_file(const char* filename);
//DIRECTORIES
unsigned char make_dir(const char* dir_name, mode_t mode);
DIR* open_dir(const char* dir_name);
unsigned char change_dir(const char* dir_name);
unsigned char close_dir(DIR* dir);
//BOTH
unsigned char check_file_dir(const char* path, mode_t mode);
//FILE MANIPULATION
char* easy_readline(FILE* file, const char* error_message);
char* easy_readline_var(FILE* file, const char* error_message, ...);
char* easy_readline_var_helper(FILE* file, const char* error_message, va_list va);
//MEMORY
void* easy_malloc(size_t size);
char* easy_printf(const char* format, ...);
char* easy_printf_helper(const char* format, va_list va);
char* copy_string_to_heap(const char* string);
//ARRAYS
//RSS_ENTRY
rss_entry** rss_entry_add_one(rss_entry** entries, size_t* size, int show_id, const char* show_name, int episode_id, const char* magnet_link);
rss_entry** rss_entry_null_terminate(rss_entry** entries, size_t size);
rss_entry** rss_entry_copy(rss_entry** tocopy);
void rss_entry_free(rss_entry** tofree);
//PREM_RESTART
prem_restart** prem_restart_add_one(prem_restart** restarts, size_t* size, const char* show_name, const char* id, const char* pin, const char* magnet);
prem_restart** prem_restart_null_terminate(prem_restart** restarts, size_t size);
void prem_restart_free(prem_restart** tofree);
//PREM_DOWNLOAD
prem_download** prem_download_add_one(prem_download** downloads, size_t* size, const char* show_name, const char* hash);
prem_download** prem_download_null_terminate(prem_download** downloads, size_t size);
void prem_download_free(prem_download** tofree);
//VARIOUS
void write_error_with_error_number(const char* error_message);
void write_error_with_error_number_var(const char* error_message, ...);
void write_error_with_error_number_var_helper(const char* error_message, va_list va);
int extract_number_from_line(char* line, const char* start_string, const char end_char);
char* extract_string_from_line(char* line, const char* start_string, const char end_char);
char* extract_string_from_line_without_exit(char* line, const char* start_string, const char end_char);

#endif
