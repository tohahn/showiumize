#ifndef SHOWRSS_H
#define SHOWRSS_H

#include "structs.h"

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_SHOWRSS "./showRSS/"
//TOKENS
#define TOKEN_RSS_SHOW_ID "<tv:show_id>"
#define TOKEN_RSS_EPISODE_ID "<tv:external_id>"
#define TOKEN_RSS_SHOW_NAME "<tv:show_name>"
#define TOKEN_RSS_MAGNET_LINK "<enclosure url=\""
#define TOKEN_RSS_START_CHAR '>'
#define TOKEN_RSS_END_CHAR '<'
#define TOKEN_RSS_MAGNET_START_CHAR '"'
#define TOKEN_RSS_MAGNET_END_CHAR '&'
//LINKS
#define LINK_RSS_FEED "https://showrss.info/user/%s.rss"

/** METHODS **/
feed_entry** handle_showrss(const char* id);
FILE* download_feed(const char* id);
feed_entry** read_entries_from_feed(FILE* feed_file);
feed_entry** read_entries_from_dir(void);
int rss_extract_number(char* line, const char* tag);
char* rss_extract_string(char* line, const char* tag, const char start_char, const char end_char);
feed_entry** rss_add_entry_to_array(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link);
char* readline(FILE* feed_file);
feed_entry** compare_entries(feed_entry** feed_items, feed_entry** dir_items);
void rss_free_array(feed_entry** tofree);
void write_entries_to_dir(feed_entry** feed_items);
feed_entry** copy_entry_array(feed_entry** tocopy);
feed_entry** null_terminate_array(feed_entry** toterminate, size_t size);
feed_entry** copy_entry(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link);

#endif
