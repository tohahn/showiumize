#ifndef CURL_HELP_H
#define CURL_HELP_H

/** INCLUDES **/
#include <stdio.h>

/** STRUCTS **/
struct MemoryStruct {
  char *memory;
  size_t size;
};

/** METHODS **/
char* read_curl(char* url);
char* read_curl_post(char* url, char* post_data);

#endif
