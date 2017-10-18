#ifndef SHOWRSS_H
#define SHOWRSS_H

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define RSS_FEED_TEMPLATE "https://showrss.info/user/%s.rss"
#define RSS_FILE_TEMPLATE "./%s.rss"
#define RSS_FEED_FILE "./%s"
#define RSS_DIR "./showRSS/"
#define RSS_SHOW_ID "<tv:show_id>"
#define RSS_EPISODE_ID "<tv:external_id>"
#define RSS_SHOW_NAME "<tv:show_name>"
#define RSS_MAGNET_LINK "<enclosure url=\""
#define RSS_START_CHAR '>'
#define RSS_END_CHAR '<'
#define RSS_MAGNET_START_CHAR '"'
#define RSS_MAGNET_END_CHAR '&'

typedef struct feed_entry {
	int show_id;
	char* show_name;
	int episode_id;
	char* magnet;
} feed_entry;

feed_entry** handle_showrss(const char* id);
FILE* download_feed(const char* id);
feed_entry** read_entries_from_feed(FILE* feed_file);
feed_entry** read_entries_from_dir(void);
int rss_extract_number(char* line, const char* tag);
char* rss_extract_string(char* line, const char* tag, const char start_char, const char end_char);
feed_entry** rss_add_entry_to_array(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link);
char* readline(FILE* feed_file);
feed_entry** compare_entries(feed_entry** feed_items, feed_entry** dir_items);
void free_array(feed_entry** tofree);
void write_entries_to_dir(feed_entry** feed_items);
feed_entry** copy_entry_array(feed_entry** tocopy);
feed_entry** null_terminate_array(feed_entry** toterminate, size_t size);
feed_entry** copy_entry(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link);

feed_entry** null_terminate_array(feed_entry** toterminate, size_t size) {
	if (!toterminate || size < 1) {
		return NULL;
	}

	toterminate = realloc(toterminate, (size + 1) * sizeof(feed_entry*));
	toterminate[size] = NULL;

	return toterminate;
}

feed_entry** handle_showrss(const char* id) {
	write_log("Downloading feed.");
	FILE* feed_file = download_feed(id);
	
	write_log("Reading feed.");
	feed_entry** feed_items = read_entries_from_feed(feed_file);
	
	write_log("Reading dir.");
	feed_entry** dir_items = read_entries_from_dir();
	
	write_log("Check unread.");
	feed_entry** unread;
	unread = compare_entries(feed_items, dir_items);

	write_log("Write entries from feed.");
	write_entries_to_dir(feed_items);

	write_log("Free arrays.");
	free_array(feed_items);
	free_array(dir_items);

	return unread;
}	

void write_entries_to_dir(feed_entry** feed_items) {
	size_t index = 0;
	feed_entry* curr;
	while ((curr = feed_items[index++])) {
		char* filename = malloc(20 * sizeof(char));
		sprintf(filename, "./%d%d", curr->show_id, curr->episode_id);
		FILE* temp_file;
		
		if (!(temp_file = fopen(filename, "w"))) {
			write_error("Couldn't open feed entry file in showRSS directory.");
			exit(EXIT_FAILURE);
		}
		fprintf(temp_file, "%d\n", curr->show_id);
		fprintf(temp_file, "%s\n", curr->show_name);
		fprintf(temp_file, "%d\n", curr->episode_id);
		fprintf(temp_file, "%s\n", curr->magnet);
		
		if (fclose(temp_file)) {
			write_error("Could not close feed entry file in showRSS directory.");
			exit(EXIT_FAILURE);
		}
		
		free(filename);
	}
}

void free_array(feed_entry** tofree) {
	size_t index = 0;
	feed_entry* curr;;
	while ((curr = tofree[index++])) {
		free(curr->show_name);
		free(curr->magnet);
		free(curr);
	}
	free(tofree);
}

feed_entry** compare_entries(feed_entry** feed_items, feed_entry** dir_items) {
	if (!feed_items) {
		return NULL;
	}
	feed_entry** unread = NULL;
	if (!dir_items) {
		return copy_entry_array(feed_items);
	}
	
	size_t count = 0;
	size_t index1 = 0;
	feed_entry* f_cur;
	while ((f_cur = feed_items[index1++])) {
		unsigned char is_unread = TRUE;

		size_t index2 = 0;
		feed_entry* d_cur;
		while ((d_cur = dir_items[index2++])) {
			write_log(f_cur->magnet);
			write_log(d_cur->magnet);
			if (!strcmp(f_cur->magnet, d_cur->magnet)) {
				is_unread = FALSE;
			}
		}
		if (is_unread) {
			unread = rss_add_entry_to_array(unread, ++count, f_cur->show_id, f_cur->show_name, f_cur->episode_id, f_cur->magnet);
		}
	}

	return null_terminate_array(unread, count);
}

feed_entry** copy_entry_array(feed_entry** tocopy) {
	feed_entry** copy = NULL;
	size_t centries = 0;

	size_t index = 0;
	feed_entry* cur;
	while ((cur = tocopy[index++])) {
		copy = copy_entry(copy, ++centries, cur->show_id, cur->show_name, cur->episode_id, cur->magnet);
	}
	return null_terminate_array(copy, centries);
}

feed_entry** copy_entry(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link) {
	entries = realloc(entries, size * sizeof(feed_entry*));
	entries[--size] = malloc(sizeof(feed_entry));
	
	entries[size]->show_id = show_id;

	size_t len = strlen(show_name);
	entries[size]->show_name = malloc((len + 1) * sizeof(char));
	memcpy(entries[size]->show_name, show_name, len * sizeof(char));
	entries[size]->show_name[len] = '\0';

	entries[size]->episode_id = episode_id;
	
	len = strlen(magnet_link);
	entries[size]->magnet = malloc((len + 1) * sizeof(char));
	memcpy(entries[size]->magnet, magnet_link, len * sizeof(char));
	entries[size]->magnet[len] = '\0';

	return entries;
}

FILE* download_feed(const char* id) {
	char* feed_url = malloc(sizeof(RSS_FEED_TEMPLATE) + sizeof(id) + 1);
	sprintf(feed_url, RSS_FEED_TEMPLATE, id);

	char* feed_filename = malloc(sizeof(RSS_FILE_TEMPLATE) + sizeof(id) + 1);
	sprintf(feed_filename, RSS_FILE_TEMPLATE, id);
	
	FILE* feed_file;

	if(!(feed_file = fopen(feed_filename, "w"))) {
		write_error("Couldn't open feed file. Exiting.");
		exit(EXIT_FAILURE);
	}

	CURL* curl;
	CURLcode res;
	
	res = curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, feed_url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, feed_file);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			write_error("Curl failed to get feed. Exiting.");
			exit(EXIT_FAILURE);
		}

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	if (!(feed_file = freopen(feed_filename, "r", feed_file))) {
		write_error("Feed file could not be reopenend for writing. Exiting.");
		exit(EXIT_FAILURE);
	}

	free(feed_url);
	free(feed_filename);
	return feed_file;
}

feed_entry** read_entries_from_feed(FILE* feed_file) {
	feed_entry** feed_entries = NULL;
	char* temp_line = NULL;
	size_t len = 0;
	ssize_t read;

	read = getline(&temp_line, &len, feed_file);
	if (read == -1) {
		write_error("Feed contains no items.");
		free(temp_line);
		exit(EXIT_FAILURE);
	}

	//copy line to local storage, so we don't have to worry about memory
	char array_line[strlen(temp_line) + 1];
	memcpy(array_line, temp_line, strlen(temp_line) * sizeof(char));
	array_line[strlen(temp_line)] = '\0';
	free(temp_line);
	char* line = &array_line[0];
	size_t count = 0;
	
	while ((line = strstr((const char*) line, RSS_SHOW_ID))) {
		int show_id = rss_extract_number(line, RSS_SHOW_ID);
		char* show_name = rss_extract_string(line, RSS_SHOW_NAME, RSS_START_CHAR, RSS_END_CHAR);
		int episode_id = rss_extract_number(line, RSS_EPISODE_ID);
		char* magnet_link = rss_extract_string(line, RSS_MAGNET_LINK, RSS_MAGNET_START_CHAR, RSS_MAGNET_END_CHAR);
		
		feed_entries = rss_add_entry_to_array(feed_entries, ++count, show_id, show_name, episode_id, magnet_link);

		line += 1;
	}
	

	if (fclose(feed_file)) {
		write_error("Feed file could not be closed. Exiting.");
		exit(EXIT_FAILURE);
	}

	return null_terminate_array(feed_entries, count);
}

feed_entry** read_entries_from_dir() {
	if(access(RSS_DIR, R_OK | W_OK)) {
		write_log("./showRSS/ does not exist, creating it for further use.");
		if(mkdir(RSS_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./showRSS/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
		return NULL;
	}

	DIR* show_dir;
	if (!(show_dir = opendir(RSS_DIR))) {
		write_error("Could not open ./showRSS/. Exiting.");
		exit(EXIT_FAILURE);
	}

	chdir(RSS_DIR);
	
	struct dirent* curr_entry;
	feed_entry** dir_entries = NULL;
	FILE* curr_entry_file;
	char* filename;
	size_t count = 0;
	while ((curr_entry = readdir(show_dir))) {
		if (!strcmp(curr_entry->d_name, ".") || !strcmp(curr_entry->d_name, "..")) {
			continue;
		}
		filename = malloc((strlen(curr_entry->d_name) + 3) * sizeof(char));
		sprintf(filename, RSS_FEED_FILE, curr_entry->d_name);
		if (!(curr_entry_file = fopen(filename, "r"))) {
			write_error("Could not open entry file for reading.");
			exit(EXIT_FAILURE);
		}
		
		char* line = readline(curr_entry_file);
		int show_id = atoi(line);
		free(line);

		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		line = readline(curr_entry_file);
		int episode_id = atoi(line);
		free(line);
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* magnet = line;
		
		dir_entries = rss_add_entry_to_array(dir_entries, ++count, show_id, show_name, episode_id, magnet);

		if (fclose(curr_entry_file)) {
			write_error("Couldn't close entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	closedir(show_dir);

	return null_terminate_array(dir_entries, count);
}

char* readline(FILE* feed_file) {
	size_t len = 0;
	char* line = NULL;
	ssize_t read = getline(&line, &len, feed_file);
	
	if (read == -1) {
		write_error("File malformed.");
		free(line);
		exit(EXIT_FAILURE);
	}

	return line;
}

int rss_extract_number(char* line, const char* tag) {
	if (!(line = strstr((const char*) line, tag))) {
		write_error("Premature end of feed file.");
		exit(EXIT_FAILURE);
	}			

	char* start = strchr(line, RSS_START_CHAR) + 1;
	char* end = strchr(start, RSS_END_CHAR);
	char number_string[end - start + 1];
	memcpy(number_string, start, (size_t) end - (size_t) start);
	number_string[end - start] = '\0';

	return atoi((const char*) number_string);
}

char* rss_extract_string(char* line, const char* tag, const char start_char, const char end_char) {
	if (!(line = strstr((const char*) line, tag))) {
		write_error("Premature end of feed file.");
		exit(EXIT_FAILURE);
	}

	char* start = strchr(line, start_char) + 1;
	char* end = strchr(start, end_char);
	char* rss_string = malloc((size_t)(end - start + 1) * sizeof(char));
	memcpy(rss_string, start, (size_t)(end - start) * sizeof(char));
	rss_string[end - start] = '\0';

	return rss_string;
}

feed_entry** rss_add_entry_to_array(feed_entry** entries, size_t size, int show_id, char* show_name, int episode_id, char* magnet_link) {
	entries = realloc(entries, size * sizeof(feed_entry*));
	entries[--size] = malloc(sizeof(feed_entry));
	
	entries[size]->show_id = show_id;
	entries[size]->show_name = show_name;
	entries[size]->episode_id = episode_id;
	entries[size]->magnet = magnet_link;

	return entries;
}
#endif
