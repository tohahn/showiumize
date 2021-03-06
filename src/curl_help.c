/** INCLUDES **/
//STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//CUSTOM
#include <curl/curl.h>
#include "utils.h"
#include "curl_help.h"
#include "log.h"
#include "utils.h"
#include "messages.h"

/** PROTOTYPES **/
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

/** METHODS **/
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    write_error(ERROR_MALLOC);
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

char* read_curl(char* url)
{
  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
    size_t size = strlen(CURL_FAILED) + strlen(curl_easy_strerror(res)) + 1;
    write_error_var(CURL_FAILED, size, curl_easy_strerror(res));
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  char* ret_val = copy_string_to_heap((char*) chunk.memory);
  free(chunk.memory);
  free(url);

  return ret_val;
}

char* read_curl_post(char* url, char* post_data)
{
  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
  curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_data);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
    size_t size = strlen(CURL_FAILED) + strlen(curl_easy_strerror(res)) + 1;
    write_error_var(CURL_FAILED, size, curl_easy_strerror(res));
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  char* ret_val = copy_string_to_heap((char*) chunk.memory);
  free(chunk.memory);
  free(url);
  free(post_data);

  return ret_val;
}
