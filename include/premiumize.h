#ifndef PREMIUMIZE_H
#define PREMIUMIZE_H

/** INCLUDE **/
#include <stdio.h>
#include "structs.h"

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_PREMIUMIZE_ROOT "./premiumize/"
#define DIR_PREMIUMIZE_RESTART "./restart/"
#define DIR_PREMIUMIZE_DOWNLOAD "./download/"
//FILES
#define FILE_TEMP_NAME "./PREM_DOWNLOAD_TEMP"
//TOKENS
#define TOKEN_PREM_SIZE_START "\"size\":"
#define TOKEN_PREM_SIZE_END ','
#define TOKEN_PREM_LINK_START "\"url\":\""
#define TOKEN_PREM_LINK_END '\"'
#define TOKEN_PREM_LINK_FALSE ".stream"
#define TOKEN_PREM_HASH_START "\"id\":\""
#define TOKEN_PREM_HASH_END '\"'
//VALUES
#define PREM_MIN_SIZE 200000000
//LINKS
#define LINK_PREM_CREATE "https://www.premiumize.me/api/transfer/create"
#define LINK_PREM_STATUS "https://www.premiumize.me/api/torrent/browse"
//FORMAT_STRINGS
#define FORMAT_PREM_DATA_CREATE "customer_id=%s&pin=%s&type=torrent&src=%s"
#define FORMAT_PREM_DATA_STATUS "customer_id=%s&pin=%s&hash=%s"
#define FORMAT_CURL "curl -O -J -L -s %s &"
#define FORMAT_SHOW_FOLDER "%s%s/"

/** METHODS **/
void handle_premiumize(rss_entry** to_download, char* pin, char* id, char* series_folder);
void create_transfers_for_feed_items(rss_entry** to_download, prem_download*** downloads_ptr, size_t* dc, prem_restart*** restarts_ptr, size_t* rc, char* pin, char* id);
char* handle_one_transfer(const char* pin, const char* id, const char* magnet);
void read_restart_dir(prem_download*** downloads_ptr, size_t* dc, prem_restart*** restarts_ptr, size_t* rc);
void write_restart_dir(prem_restart** restarts);
void read_download_dir(prem_download*** downloads_ptr, size_t* dc);
prem_download** check_transfers(prem_download** downloads, char* id, char* pin, char* series_folder);
unsigned char handle_one_download(char* series_folder, char* show_name, char* hash, char* id, char* pin);
unsigned char start_download(char* line, char* series_folder, char* show_name);
void write_download_dir(prem_download** unfinished);
void curl_create_transfer(FILE* temp_file, char* link, char* post_data);

#endif
