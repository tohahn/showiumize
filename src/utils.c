/** INCLUDES **/
//STDLIB
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
//CUSTOM
#include "utils.h"
#include "definitions.h"
#include "structs.h"
#include "messages.h"
#include "log.h"

/** METHODS **/
//FILES
FILE* open_file(const char* filename, const char* mode) {
  FILE* temp = fopen(filename, mode);
  if (temp) {
    write_error_with_error_number_var(ERROR_FILE_OPEN, filename, mode);
    exit(EXIT_FAILURE);
  }
  return temp;
}

FILE* open_temp_file() {
	char* temp_name = copy_string_to_heap(FILE_TEMP);
  int fd = mkstemp(temp_name);
  if (fd == -1) {
    write_error_with_error_number_var(ERROR_FILE_TEMP_OPEN, FILE_TEMP);
    exit(EXIT_FAILURE);
  }
	FILE* temp_file = fdopen(fd, TEMP_FILE_MODE);
	if (!temp_file) {
    write_error_with_error_number_var(ERROR_FILE_OPEN, temp_name, TEMP_FILE_MODE);
    exit(EXIT_FAILURE);
	}
  return temp_file;
}

unsigned char close_file(FILE* file) {
  if (fclose(file)) {
    write_error_with_error_number(ERROR_FILE_CLOSE);
    exit(EXIT_FAILURE);
  }
  return TRUE;
}

unsigned char remove_file(const char* filename) {
  if(remove(filename)) {
    write_error_with_error_number_var(ERROR_FILE_REMOVE, filename);
    exit(EXIT_FAILURE);
  }
  return TRUE;
}

//DIRECTORIES
unsigned char make_dir(const char* dir_name, mode_t mode) {
  if (!mkdir(dir_name, mode)) {
    write_error_with_error_number_var(ERROR_DIR_CREATE, dir_name, mode);
    exit(EXIT_FAILURE);
  }
  return TRUE;
}

DIR* open_dir(const char* dir_name) {
  DIR* temp = opendir(dir_name);
  if (temp) {
    write_error_with_error_number_var(ERROR_DIR_OPEN, dir_name);
    exit(EXIT_FAILURE);
  }
  return temp;
}

unsigned char change_dir(const char* dir_name) {
  int temp = chdir(dir_name);
  if (temp) {
    write_error_with_error_number_var(ERROR_DIR_CHANGE, dir_name);
    exit(EXIT_FAILURE);
  }
  return TRUE;
}

unsigned char close_dir(DIR* dir) {
  if (closedir(dir)) {
    write_error_with_error_number(ERROR_DIR_CLOSE);
    exit(EXIT_FAILURE);
  }
  return TRUE;
}

//BOTH
unsigned char check_file_dir(const char* path, mode_t mode) {
  if (access(path, mode)) {
    write_error_with_error_number_var(ERROR_PATH_NO_ACCESS, path, mode);
    return FALSE;
  } else {
    return TRUE;
  }
}

//FILE MANIPULATION
char* easy_readline(FILE* file, const char* error_message) {
  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  read = getline(&line, &len, file);
  if (read == -1) {
    write_error_with_error_number(error_message);
    free(line);
    exit(EXIT_FAILURE);
  }
  return line;
}

char* easy_readline_var(FILE* file, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  char* temp = easy_readline_var_helper(file, error_message, va);
  va_end(va);
  return temp;
}

char* easy_readline_var_helper(FILE* file, const char* error_message, va_list va) {
  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  read = getline(&line, &len, file);
  if (read == -1) {
    write_error_with_error_number_var_helper(error_message, va);
    free(line);
    exit(EXIT_FAILURE);
  }
  return line;
}

//MEMORY
void* easy_malloc(size_t size) {
  void* memory = malloc(size);
  if (memory) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  return memory;
}

char* easy_printf(const char* format, ...) {
  va_list va;
  va_start(va, format);
  char* temp = easy_printf_helper(format, va);
  va_end(va);
  return temp;
}

char* easy_printf_helper(const char* format, va_list va) {
  //finding out the size of the string
  size_t size = strlen(format) + VALUE_VARARG_SIZE + 1;
  char* string = easy_malloc(size * sizeof(char));
  if (vsprintf(string, format, va) < 1) {
    write_error(ERROR_EASY_SPRINTF);
    exit(EXIT_FAILURE);
  }
  return string;
}

char* copy_string_to_heap(const char* string) {
  size_t size = strlen(string);
  char* string_temp = easy_malloc(size + 1);
  memcpy(string_temp, string, size);
  string_temp[size] = '\0';
  return string_temp;
}

//ARRAYS
//RSS_ENTRY
rss_entry** rss_entry_add_one(rss_entry** entries, size_t* size, int show_id, const char* show_name, int episode_id, const char* magnet_link) {
  size_t size_temp = *size;
  rss_entry** temp = realloc(entries, size_temp * sizeof(rss_entry*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  entries = temp;

  size_t i = size_temp - 1;
  entries[i] = easy_malloc(sizeof(rss_entry));

  entries[i]->show_id = show_id;
  entries[i]->show_name = copy_string_to_heap(show_name);
  entries[i]->episode_id = episode_id;
  entries[i]->magnet = copy_string_to_heap(magnet_link);

  return entries;
}

rss_entry** rss_entry_null_terminate(rss_entry** entries, size_t size) {
  if (!entries || size < 1) {
    return NULL;
  }

  rss_entry** temp = realloc(entries, (size + 1) * sizeof(rss_entry*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }

  entries = temp;
  entries[size] = NULL;

  return entries;
}

rss_entry** rss_entry_copy(rss_entry** tocopy) {
	rss_entry** copy = NULL;
	size_t centries = 0;

	size_t index = 0;
	rss_entry* cur;
	while ((cur = tocopy[index++])) {
    centries++;
		copy = rss_entry_add_one(copy, &centries, cur->show_id, cur->show_name, cur->episode_id, cur->magnet);
	}
	return rss_entry_null_terminate(copy, centries);
}

void rss_entry_free(rss_entry** tofree) {
  if (!tofree) return;
  size_t index = 0;
  rss_entry* curr;;
  while ((curr = tofree[index++])) {
    free(curr->show_name);
    free(curr->magnet);
    free(curr);
  }
  free(tofree);
}

//PREM RESTART
prem_restart** prem_restart_add_one(prem_restart** restarts, size_t* size, const char* show_name, const char* id, const char* pin, const char* magnet) {
  size_t size_temp = *size;
  prem_restart** temp = realloc(restarts, size_temp * sizeof(prem_restart*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  restarts = temp;

  size_t i = size_temp - 1;
  restarts[i] = easy_malloc(sizeof(prem_restart));

  restarts[i]->show_name = copy_string_to_heap(show_name);
  restarts[i]->id = copy_string_to_heap(id);
  restarts[i]->pin = copy_string_to_heap(pin);
  restarts[i]->magnet = copy_string_to_heap(magnet);

  return restarts;
}

prem_restart** prem_restart_null_terminate(prem_restart** restarts, size_t size) {
  if (!restarts || size < 1) {
    return NULL;
  }

  prem_restart** temp = realloc(restarts, (size + 1) * sizeof(prem_restart*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }

  restarts = temp;
  restarts[size] = NULL;

  return restarts;
}

void prem_restart_free(prem_restart** tofree) {
  if (!tofree) return;
  size_t index = 0;
  prem_restart* curr;;
  while ((curr = tofree[index++])) {
    free(curr->show_name);
    free(curr->magnet);
    free(curr);
  }
  free(tofree);
}

//PREM_DOWNLOAD
prem_download** prem_download_add_one(prem_download** downloads, size_t* size, const char* show_name, const char* hash) {
  size_t size_temp = *size;
  prem_download** temp = realloc(downloads, size_temp * sizeof(prem_download*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  downloads = temp;

  size_t i = size_temp - 1;
  downloads[i] = easy_malloc(sizeof(prem_download));

  downloads[i]->show_name = copy_string_to_heap(show_name);
  downloads[i]->hash = copy_string_to_heap(hash);

  return downloads;
}

prem_download** prem_download_null_terminate(prem_download** downloads, size_t size) {
  if (!downloads || size < 1) {
    return NULL;
  }

  prem_download** temp = realloc(downloads, (size + 1) * sizeof(prem_download*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }

  downloads = temp;
  downloads[size] = NULL;

  return downloads;
}

void prem_download_free(prem_download** tofree) {
  if (!tofree) return;
  size_t index = 0;
  prem_download* curr;
  while ((curr = tofree[index++])) {
    free(curr->show_name);
    free(curr->hash);
    free(curr);
  }
  free(tofree);
}

//VARIOUS
void write_error_with_error_number(const char* error_message) {
  char* error = easy_printf(FORMAT_ERROR_NUMBER, error_message, errno);
  write_error(error);
  free(error);
}

void write_error_with_error_number_var(const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
   write_error_with_error_number_var_helper(error_message, va);
  va_end(va);
}

void write_error_with_error_number_var_helper(const char* error_message, va_list va) {
  char* temp_error = easy_printf(error_message, va);
  char* error = easy_printf(FORMAT_ERROR_NUMBER, errno);
  free(temp_error);
  write_error(error);
  free(error);
}

int extract_number_from_line(char* line, const char* start_string, const char end_char) {
	if (!(line = strstr((const char*) line, start_string))) {
		write_error_var(ERROR_PREMATURE_END, start_string);
		exit(EXIT_FAILURE);
	}

	char* start = strstr(line, start_string) + strlen(start_string) + 1;
	char* end = strchr(start, end_char);
	char number_string[end - start + 1];
	memcpy(number_string, start, (size_t) end - (size_t) start);
	number_string[end - start] = '\0';

	return atoi((const char*) number_string);
}

char* extract_string_from_line(char* line, const char* start_string, const char end_char) {
	if (!(line = strstr((const char*) line, start_string))) {
		write_error_var(ERROR_PREMATURE_END, start_string);
		exit(EXIT_FAILURE);
	}

	char* start = strstr(line, start_string) + strlen(start_string) + 1;
	char* end = strchr(start, end_char);
	char* rss_string = easy_malloc((size_t)(end - start + 1) * sizeof(char));
	memcpy(rss_string, start, (size_t)(end - start) * sizeof(char));
	rss_string[end - start] = '\0';

	return rss_string;
}

char* extract_string_from_line_without_exit(char* line, const char* start_string, const char end_char) {
  if (!(line = strstr((const char*) line, start_string))) {
		return NULL;
	}

	char* start = strstr(line, start_string) + strlen(start_string) + 1;
	char* end = strchr(start, end_char);
	char* rss_string = easy_malloc((size_t)(end - start + 1) * sizeof(char));
	memcpy(rss_string, start, (size_t)(end - start) * sizeof(char));
	rss_string[end - start] = '\0';

	return rss_string;
}
