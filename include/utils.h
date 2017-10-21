#ifndef UTILS_H
#define UTILS_H

/** INCLUDES **/
//STDLIB
#include <stdio.h>
#include <dirent.h>
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

/** METHODS **/
//FILES
FILE* open_file(const char* filename, const char* mode, const char* error_message, ...);
FILE* open_file(const char* filename, const char* mode, const char* error_message, va_list va);
FILE* open_temp_file(char* filename, const char* error_message, ...);
FILE* open_temp_file(char* filename, const char* error_message, va_list va);
unsigned char close_file(FILE* file, const char* error_message, ...);
unsigned char close_file(FILE* file, const char* error_message, va_list va);
unsigned char remove_file(const char* filename, const char* error_message, ...);
unsigned char remove_file(const char* filename, const char* error_message, va_list va);
//DIRECTORIES
DIR* open_dir(const char* dir_name, const char* error_message, ...);
DIR* open_dir(const char* dir_name, const char* error_message, va_list va);
unsigned char close_dir(DIR* dir, const char* error_message, ...);
unsigned char close_dir(DIR* dir, const char* error_message, va_list va);
//BOTH
unsigned char check_file_dir(const char* path, const char* mode, const char* error_message, ...);
unsigned char check_file_dir(const char* path, const char* mode, const char* error_message, va_list va);
//MEMORY
void* easy_malloc(size_t size);
char* easy_printf(const char* format, ...);
char* easy_printf(const char* format, va_list va);
char* copy_string_to_heap(const char* string, const char* error_message, ...);
char* copy_string_to_heap(const char* string, const char* error_message, va_list va);
//ARRAYS
//RSS_ENTRY
rss_entry** rss_entry_add_one(rss_entry** entries, size_t* size, int show_id, const char* show_name, int episode_id, const char* magnet_link);
rss_entry** rss_entry_null_terminate(rss_entry** entries, size_t size);
//PREM_RESTART
prem_restart** prem_restart_add_one(prem_restart** restarts, size_t* size, const char* show_name, const char* id, const char* pin, const char* magnet);
prem_restart** prem_restart_null_terminate(prem_restart** restarts, size_t size);
//PREM_DOWNLOAD
prem_download** prem_download_add_one(prem_download** downloads, size_t* size, const char* show_name, const char* hash);
prem_download** prem_download_null_terminate(prem_download** downloads, size_t size);
//VARIOUS
void write_error_with_error_number(const char* error_message, va_list va);

#endif
