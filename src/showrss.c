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

rss_entry** handle_showrss(const char* id) {
	FILE* feed_file = download_feed(id);
	rss_entry** feed_items = read_entries_from_feed(feed_file);
	rss_entry** dir_items = read_entries_from_dir();
	rss_entry** unread = compare_entries(feed_items, dir_items);
	write_entries_to_dir(feed_items);
	rss_entry_free(feed_items);
	rss_entry_free(dir_items);

	change_dir(PREVIOUS_DIR);

	return unread;
}

FILE* download_feed(const char* id) {
	char* feed_url = easy_printf(LINK_RSS_FEED, id);
	char* feed_filename = easy_printf(TEMPLATE_FEED_FILE, id);
	FILE* feed_file = open_file(feed_filename, FILE_WRITE_MODE);

	CURL* curl;
	CURLcode res;

	res = curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, feed_url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, feed_file);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			write_error(ERROR_CURL_FEED);
			exit(EXIT_FAILURE);
		}

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	close_file(feed_file);
	open_file(feed_filename, FILE_READ_MODE);

	free(feed_url);
	free(feed_filename);
	return feed_file;
}

rss_entry** read_entries_from_feed(FILE* feed_file) {
	rss_entry** feed_entries = NULL;

	char* line = easy_readline(feed_file, ERROR_FEED_NO_ITEMS);

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
		++count;
		feed_entries = rss_entry_add_one(feed_entries, &count, show_id, show_name, episode_id, magnet);

		line += 1;
	}

	close_file(feed_file);

	return rss_entry_null_terminate(feed_entries, count);
}

rss_entry** read_entries_from_dir() {
	check_file_dir(DIR_SHOWRSS, R_OK | W_OK);
	make_dir(DIR_SHOWRSS, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH);

	DIR* show_dir = open_dir(DIR_SHOWRSS);
	change_dir(DIR_SHOWRSS);

	struct dirent* curr_entry;
	rss_entry** dir_entries = NULL;
	size_t count = 0;
	while ((curr_entry = readdir(show_dir))) {
		if (!strcmp(curr_entry->d_name, CURRENT_DIR) || !strcmp(curr_entry->d_name, PREVIOUS_DIR)) {
			continue;
		}

		char* filename = easy_printf(FILE_CURR_DIR, curr_entry->d_name);
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

		++count;
		dir_entries = rss_entry_add_one(dir_entries, &count, show_id, show_name, episode_id, magnet);

		close_file(curr_entry_file);
	}
	close_dir(show_dir);

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
			++count;
			unread = rss_entry_add_one(unread, &count, f_cur->show_id, f_cur->show_name, f_cur->episode_id, f_cur->magnet);
		}
	}

	return rss_entry_null_terminate(unread, count);
}

void write_entries_to_dir(rss_entry** feed_items) {
	size_t index = 0;
	rss_entry* curr;
	change_dir(DIR_SHOWRSS);
	while ((curr = feed_items[index++])) {
		FILE* temp_file = open_temp_file();

		fprintf(temp_file, FORMAT_NUMBER, curr->show_id);
		fprintf(temp_file, FORMAT_STRING, curr->show_name);
		fprintf(temp_file, FORMAT_NUMBER, curr->episode_id);
		fprintf(temp_file, FORMAT_STRING, curr->magnet);

		close_file(temp_file);
	}
}
