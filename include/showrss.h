#ifndef SHOWRSS_H
#define SHOWRSS_H

/** INCLUDES **/
#include <stdio.h>
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
//TEMPLATES
#define TEMPLATE_FEED_FILE "./%s.rss"

/** METHODS **/
rss_entry** handle_showrss(const char* id);
FILE* download_feed(const char* id);
rss_entry** read_entries_from_feed(FILE* feed_file);
rss_entry** read_entries_from_dir(void);
rss_entry** compare_entries(rss_entry** feed_items, rss_entry** dir_items);
void write_entries_to_dir(rss_entry** feed_items);

#endif
