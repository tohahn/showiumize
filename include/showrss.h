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
#define RSS_MAGNET_CHAR '"'

typedef struct feed_entry {
	int show_id;
	int episode_id;
	char* show_name;
	char* magnet;
} feed_entry;

void handle_showrss(const char* id);
FILE* download_feed(const char* id);
feed_entry** read_entries_from_feed(FILE* feed_file);
feed_entry** read_entries_from_dir(void);
int rss_extract_number(char* line, const char* tag);
char* rss_extract_string(char* line, const char* tag, const char start_char, const char end_char);
feed_entry** rss_add_entry_to_array(feed_entry** entries, int show_id, int episode_id, char* show_name, char* magnet_link);
char* readline(FILE* feed_file);

void handle_showrss(const char* id) {
	write_log("Downloading feed.");
	FILE* feed_file = download_feed(id);
	write_log("Reading feed.");
	feed_entry** feed_items = read_entries_from_feed(feed_file);
	write_log("Reading dir.");
	feed_entry** dir_items = read_entries_from_dir();
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
	
	while ((line = strstr((const char*) line, RSS_SHOW_ID))) {
		int show_id = rss_extract_number(line, RSS_SHOW_ID);
		int episode_id = rss_extract_number(line, RSS_EPISODE_ID);
		char* show_name = rss_extract_string(line, RSS_SHOW_NAME, RSS_START_CHAR, RSS_END_CHAR);
		char* magnet_link = rss_extract_string(line, RSS_MAGNET_LINK, RSS_MAGNET_CHAR, RSS_MAGNET_CHAR);
		
		feed_entries = rss_add_entry_to_array(feed_entries, show_id, episode_id, show_name, magnet_link);
		
		line += 1;
	}

	if (fclose(feed_file)) {
		write_error("Feed file could not be closed. Exiting.");
		exit(EXIT_FAILURE);
	}

	return feed_entries;
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
		int episode_id = atoi(line);
		free(line);
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* magnet = line;
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		dir_entries = rss_add_entry_to_array(dir_entries, show_id, episode_id, magnet, show_name);
		
		free(filename);
		if (fclose(curr_entry_file)) {
			write_error("Couldn't close entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	closedir(show_dir);

	return dir_entries;
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
	memcpy(number_string, start, end - start);
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
	char* rss_string = malloc((end - start + 1) * sizeof(char));
	memcpy(rss_string, start, (end - start) * sizeof(char));
	rss_string[end - start] = '\0';

	return rss_string;
}

feed_entry** rss_add_entry_to_array(feed_entry** entries, int show_id, int episode_id, char* show_name, char* magnet_link) {
	size_t new_size;
	if (entries == NULL) {
		new_size = 1;
	} else {
		new_size = sizeof(entries) + 1;
	}

	entries = realloc(entries, (new_size * sizeof(feed_entry*)));
	entries[--new_size] = malloc(sizeof(feed_entry));
	
	entries[new_size]->show_id = show_id;
	entries[new_size]->episode_id = episode_id;
	entries[new_size]->show_name = show_name;
	entries[new_size]->magnet = magnet_link;

	return entries;
}
#endif
