#ifndef SHOWRSS_H
#define SHOWRSS_H

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define RSS_FEED_TEMPLATE "https://showrss.info/user/%s.rss"
#define RSS_FILE_TEMPLATE "./%s.rss"
#define RSS_SHOW_ID "<tv:show_id>"
#define RSS_EPISODE_ID "<tv:external_id>"
#define RSS_MAGNET_LINK "<enclosure url=\""
#define RSS_START_CHAR '>'
#define RSS_END_CHAR '<'
#define RSS_MAGNET_CHAR '"'

typedef struct feed_entry {
	const char* magnet;
	int show_id;
	int episode_id;
} feed_entry;

void handle_showrss(const char* id);
FILE* download_feed(const char* id);
feed_entry** read_entries_from_feed(FILE* feed_file);

void handle_showrss(const char* id) {
	write_log("Downloading feed.");
	FILE* feed_file = download_feed(id);
	write_log("Reading it.");
	feed_entry** feed_items = read_entries_from_feed(feed_file);
	write_log(feed_items[sizeof(feed_items)]->magnet);
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
	char array_line[strlen(temp_line)];
	memcpy(array_line, temp_line, strlen(temp_line) * sizeof(char));
	free(temp_line);
	array_line[strlen(temp_line)] = '\0';
	char* line = &array_line[0];

	while ((line = strstr((const char*) line, RSS_SHOW_ID))) {
		//extract show id
		char* start = strchr(line + 1, RSS_START_CHAR) + 1;
		char* end = strchr(start, RSS_END_CHAR);
		char show_id_string[end - start + 1];
		memcpy(show_id_string, start, end - start);
		show_id_string[end - start] = '\0';	

		int show_id = atoi((const char*) show_id_string);

		if (!(line = strstr((const char*) line, RSS_EPISODE_ID))) {
			write_error("Premature end of feed file.");
			exit(EXIT_FAILURE);
		}			

		//extract episode id
		start = strchr(line, RSS_START_CHAR) + 1;
		end = strchr(start, RSS_END_CHAR);
		char episode_id_string[end - start + 1];
		memcpy(episode_id_string, start, end - start);
		episode_id_string[end - start] = '\0';

		int episode_id = atoi((const char*) episode_id_string);
		
		if (!(line = strstr((const char*) line, RSS_MAGNET_LINK))) {
			write_error("Premature end of feed file.");
			exit(EXIT_FAILURE);
		}

		//extract magnet link
		start = strchr(line, RSS_MAGNET_CHAR) + 1;
		end = strchr(start, RSS_MAGNET_CHAR);
		char* magnet_link = malloc((end - start + 1) * sizeof(char));
		memcpy(magnet_link, start, (end - start) * sizeof(char));
		magnet_link[end - start] = '\0';

		//create entry for feed entry
		feed_entries = realloc(feed_entries, (sizeof(feed_entries) + 1) * sizeof(feed_entry*));
		feed_entries[sizeof(feed_entries)] = malloc(sizeof(feed_entry*));
		feed_entries[sizeof(feed_entries)]->magnet = magnet_link;
		feed_entries[sizeof(feed_entries)]->show_id = show_id;
		feed_entries[sizeof(feed_entries)]->episode_id = episode_id;
	}

	if (fclose(feed_file)) {
		write_error("Feed file could not be closed. Exiting.");
		exit(EXIT_FAILURE);
	}

	return feed_entries;
}

#endif
