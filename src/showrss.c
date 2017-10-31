/** INCLUDES **/
//STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//CUSTOM
#include "log.h"
#include "messages.h"
#include "definitions.h"
#include "showrss.h"
#include "structs.h"
#include "utils.h"
#include "curl_help.h"

rss_entry** handle_showrss(const char* id) {
	rss_entry** feed_items = read_entries_from_feed(id);
	rss_entry** dir_items = read_entries_from_dir();
	rss_entry** unread = compare_entries(feed_items, dir_items);
	write_entries_to_dir(feed_items);
	rss_entry_free(feed_items);
	rss_entry_free(dir_items);

	return unread;
}

rss_entry** read_entries_from_feed(const char* id) {
	rss_entry** feed_entries = NULL;
	size_t size = strlen(LINK_RSS_FEED) + strlen(id) + 1;
	char* feed_url = easy_printf(LINK_RSS_FEED, size, id);
	char* line = read_curl(feed_url);

	//copy line to local storage, so we don't have to worry about memory
	char array_line[strlen(line) + 1];
	memcpy(array_line, line, strlen(line) * sizeof(char));
	array_line[strlen(line)] = '\0';
	free(line);
	line = &array_line[0];
	size_t count = 0;

	while ((line = strstr((const char*) line, TOKEN_RSS_SHOW_ID))) {
		int show_id = extract_number_from_line(line, TOKEN_RSS_SHOW_ID, TOKEN_RSS_END_CHAR);
		char* show_name = extract_string_from_line(line, TOKEN_RSS_SHOW_NAME, TOKEN_RSS_END_CHAR);
		int episode_id = extract_number_from_line(line, TOKEN_RSS_EPISODE_ID, TOKEN_RSS_END_CHAR);
		char* magnet = extract_string_from_line(line, TOKEN_RSS_MAGNET_LINK, TOKEN_RSS_MAGNET_END_CHAR);
		feed_entries = rss_entry_add_one(feed_entries, ++count, show_id, show_name, episode_id, magnet);
		free(show_name);
		free(magnet);

		line += 1;
	}

	return rss_entry_null_terminate(feed_entries, count);
}

rss_entry** read_entries_from_dir() {
	if (!check_file_dir(DIR_SHOWRSS, R_OK | W_OK)) {
		make_dir(DIR_SHOWRSS, FILE_PERMISSIONS);
	}

	DIR* show_dir = open_dir(DIR_SHOWRSS);
	change_dir(DIR_SHOWRSS);

	struct dirent* curr_entry;
	rss_entry** dir_entries = NULL;
	size_t count = 0;
	while ((curr_entry = readdir(show_dir))) {
		if (!strcmp(curr_entry->d_name, CURRENT_DIR) || !strcmp(curr_entry->d_name, PREVIOUS_DIR)) {
			continue;
		}
		size_t size = strlen(FILE_CURR_DIR) + strlen(curr_entry->d_name) + 1;
		char* filename = easy_printf(FILE_CURR_DIR, size, curr_entry->d_name);
		FILE* curr_entry_file = open_file(filename, FILE_READ_MODE);

		char* line = easy_readline(curr_entry_file, ERROR_MALFORMED_ENTRY_FILE);
		int show_id = atoi(line);
		free(line);

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_ENTRY_FILE);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_ENTRY_FILE);
		int episode_id = atoi(line);
		free(line);

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_ENTRY_FILE);
		*strchr(line, '\n') = '\0';
		char* magnet = line;

		dir_entries = rss_entry_add_one(dir_entries, ++count, show_id, show_name, episode_id, magnet);
		free(show_name);
		free(magnet);

		close_file(curr_entry_file);
		remove(filename);
		free(filename);
	}
	close_dir(show_dir);
	change_dir(PREVIOUS_DIR);

	return rss_entry_null_terminate(dir_entries, count);
}

rss_entry** compare_entries(rss_entry** feed_items, rss_entry** dir_items) {
	if (!feed_items) {
		return NULL;
	}
	rss_entry** unread = NULL;
	if (!dir_items) {
		return rss_entry_copy(feed_items);
	}

	size_t count = 0;
	size_t index1 = 0;
	rss_entry* f_cur;
	while ((f_cur = feed_items[index1++])) {
		unsigned char is_unread = TRUE;
		size_t index2 = 0;
		rss_entry* d_cur;
		while ((d_cur = dir_items[index2++])) {
			if (f_cur->show_id == d_cur->show_id && f_cur->episode_id == d_cur->episode_id) {
				is_unread = FALSE;
			}
		}
		if (is_unread) {
			unread = rss_entry_add_one(unread, ++count, f_cur->show_id, f_cur->show_name, f_cur->episode_id, f_cur->magnet);
		}
	}

	return rss_entry_null_terminate(unread, count);
}

void write_entries_to_dir(rss_entry** feed_items) {
	change_dir(DIR_SHOWRSS);

	size_t index = 0;
	rss_entry* curr;
	while ((curr = feed_items[index++])) {
		FILE* temp_file = open_temp_file();

		fprintf(temp_file, FORMAT_NUMBER, curr->show_id);
		fprintf(temp_file, FORMAT_STRING, curr->show_name);
		fprintf(temp_file, FORMAT_NUMBER, curr->episode_id);
		fprintf(temp_file, FORMAT_STRING, curr->magnet);

		close_file(temp_file);
	}

	change_dir(PREVIOUS_DIR);
}
