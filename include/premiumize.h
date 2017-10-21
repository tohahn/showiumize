#ifndef PREMIUMIZE_H
#define PREMIUMIZE_H

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_PREMIUMIZE_ROOT "./premiumize/"
#define DIR_PREMIUMIZE_RESTART "./restart/"
#define DIR_PREMIUMIZE_DOWNLOAD "./download/"
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

/** METHODS **/
char* copy_string_memory(const char* tocopy);
void handle_premiumize(feed_entry** to_download, char* premiumize_pin, char* premiumize_id, char* series_folder);
void check_existing(download** downloads, size_t* dc, restart** restarts, size_t* rc, char* pin, char* id, char* series_folder);
void free_download_array(download** tofree);
void free_restart_array(restart** tofree);
void write_download_dir(download** unfinished);
download** check_transfers(download** downloads, char* id, char* pin, char* series_folder);
unsigned char handle_one_transfer(char* show_name, char* hash, char* id, char* pin, char* series_folder);
unsigned char start_download(char* show_name, char* line, char* series_folder);
void read_download_dir(download** downloads, size_t* dc);
void write_restart_dir(restart** restarts);
void read_restart_dir(download** downloads, size_t* dc, restart** restarts, size_t* rc);
download** null_terminate_download_array(download** toterminate, size_t size);
restart** null_terminate_restart_array(restart** toterminate, size_t size);
download** add_one_download(download** downloads, size_t size, char* res, char* show_name);
restart** add_one_restart(restart** restarts, size_t size, char* pin, char* id, char* show_name, char* magnet);
char* handle_one_download(const char* premiumize_pin, const char* premiumize_id, const char* magnet);
void curl_create_transfer(FILE* temp_file, char* link, char* post_data);

#endif
