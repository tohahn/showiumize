/** INCLUDES **/
//STDLIB
#include <unistd.h>
//CUSTOM
#include "utils.h"
#include "definitions.h"

/** METHODS **/
//FILES
FILE* open_file(const char* filename, const char* mode, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  FILE* temp = open_file(filename, mode, error_message, va);
  va_end(va);
  return temp;
}

FILE* open_file(const char* filename, const char* mode, const char* error_message, va_list va) {
  FILE* temp = fopen(filename, mode);
  if (temp) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  } else {
    return temp;
  }
}

FILE* open_temp_file(const char* filename, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  FILE* temp = open_temp_file(filename, mode, error_message, va);
  va_end(va);
  return temp;
}

FILE* open_temp_file(const char* filename, const char* error_message, va_list va) {
	char* temp_name = easy_malloc((strlen(FILE_TEMP) + 1) * sizeof(char));
	memcpy(temp_name, FILE_TEMP, strlen(PREM_TEMP));
	temp_name[strlen(PREM_TEMP)] = '\0';

  int fd = mkstemp(temp_name);
  if (fd == -1) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  }
	FILE* temp_file = fdopen(fd, TEMP_FILE_MODE);
	if (!temp_file) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
	}

  return temp_file;
}

unsigned char close_file(FILE* file const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  unsigned char temp = open_temp_file(file, error_message, va);
  va_end(va);
  return temp;
}

unsigned char close_file(FILE* file, const char* error_message, va_list va) {
  if (close(file)) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  } else {
    return TRUE;
  }
}

unsigned char remove_file(const char* filename, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  unsigned char temp = remove_file(filename, error_message, va);
  va_end(va);
  return temp;
}

unsigned char remove_file(const char* filename, const char* error_message, va_list va) {
  if(remove(filename)) {
    unsigned char temp = add_error_number_to_message(error_message, va, errorno);
    write_error(temp);
    free(temp);
    exit(EXIT_FAILURE);
  } else {
    return TRUE;
  }
}

//DIRECTORIES
DIR* open_dir(const char* dir_name, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  DIR* temp = open_dir(dir_name, error_message, va);
  va_end(va);
  return temp;
}

DIR* open_dir(const char* dir_name, const char* error_message, va_list va) {
  DIR* temp = opendir(filename);
  if (temp) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  } else {
    return temp;
  }
}

unsigned char close_dir(DIR* dir, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  unsigned char temp = close_dir(dir_name, error_message, va);
  va_end(va);
  return temp;
}

unsigned char close_dir(DIR* dir, const char* error_message, va_list va) {
  if (closedir(dir_name)) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  } else {
    return TRUE;
  }
}

//BOTH
unsigned char check_file_dir(const char* path, const char* mode, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  unsigned char temp = check_file_dir(path, mode, error_message, va);
  va_end(va);
  return temp;
}

unsigned char check_file_dir(const char* path, const char* mode, const char* error_message, va_list va) {
  if (access(filename, mode)) {
    write_error_with_error_number(error_message, va);
    exit(EXIT_FAILURE);
  } else {
    return TRUE;
  }
}

//MEMORY
void* easy_malloc(size_t size) {
  void* memory = malloc(size);
  if (memory) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  } else {
    return memory;
  }
}

char* easy_printf(const char* format, ...) {
  va_list va;
  va_start(va, format);
  char* temp = easy_printf(format, va);
  va_end(va);
  return temp;
}

char* easy_printf(const char* format, va_list va) {
  //finding out the size of the string
  size_t size = strlen(format) + VALUE_VARARG_SIZE + 1;
  char* string = easy_malloc(size * sizeof(char));
  if (vsprintf(string, format, va) < 1) {
    write_error(ERROR_EASY_SPRINTF);
    exit(EXIT_FAILURE);
  } else {
    return string;
  }
}

char* copy_string_to_heap(const char* string, const char* error_message, ...) {
  va_list va;
  va_start(va, error_message);
  char* temp = copy_string_to_heap(format, va);
  va_end(va);
  return temp;
}

char* copy_string_to_heap(const char* string, const char* error_message, va_list va) {
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

  size_t i = size - 1;
  entries[i] = malloc(sizeof(feed_entry));

  entries[i]->show_id = show_id;
  entries[i]->show_name = show_name;
  entries[i]->episode_id = episode_id;
  entries[i]->magnet = magnet_link;

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

//PREM_download
prem_download** prem_download_add_one(prem_download** downloads, size_t* size, const char* show_name, const char* id, const char* pin, const char* magnet) {
  size_t size_temp = *size;
  prem_download** temp = realloc(downloads, size_temp * sizeof(prem_download*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  downloads = temp;

  size_t i = size - 1;
  downloads[i] = malloc(sizeof(prem_download));

  downloads[i]->show_name = show_name;
  downloads[i]->id = id;
  downloads[i]->pin = pin:
  downloads[i]->magnet = magnet;

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

//PREM_DOWNLOAD
prem_download** prem_download_add_one(prem_download** downloads, size_t* size, const char* show_name, const char* hash) {
  size_t size_temp = *size;
  prem_download** temp = realloc(downloads, size_temp * sizeof(prem_download*));
  if (!temp) {
    write_error(ERROR_MALLOC);
    exit(EXIT_FAILURE);
  }
  downloads = temp;

  size_t i = size - 1;
  downloads[i] = malloc(sizeof(prem_download));

  downloads[i]->show_name = show_name;
  downloads[i]->hash = hash;

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

//VARIOUS
void write_error_with_error_number(const char* error_message, va_list va) {
  char* temp_error = easy_printf(error_message, va);
  size_t size = strlen(temp_error) + sizeof(FORMAT_ERROR_NUMBER) - 4 + sizeof(errorno) + 1;
  char* error = easy_malloc(size);
  sprintf(error, FORMAT_ERROR_NUMBER, errorno);
  free(temp_error);
  write_error(error);
  free(error);
}
