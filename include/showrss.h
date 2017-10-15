#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define RSS_FEED_TEMPLATE "https://showrss.info/user/%s.rss"
#define RSS_FILE_TEMPLATE "./%s.rss"
#define RSS_LINK "<link>"
#define RSS_SHOW_ID "<tv:show_id>"
#define RSS_EPISODE_ID "<tv:external_id>"
#define RSS_START_CHAR '>'
#define RSS_END_CHAR '<'

typedef struct feed_entry {
	const char* magnet;
	int show_id;
	int episode_id;
} feed_entry;

void handle_showrss(const char* id);
FILE* download_feed(const char* id);
feed_entry** read_entries_from_feed(FILE* feed_file);

void handle_showrss(const char* id) {
	write_log("Downloading feed and reading it.");
	feed_entry** feed_items = read_entries_from_feed(download_feed(id));
	write_log(feed_items[sizeof(feed_items)]->magnet);
}

FILE* download_feed(const char* id) {
	char* feed_url = malloc(sizeof(RSS_FEED_TEMPLATE) + sizeof(id));
	sprintf(feed_url, RSS_FEED_TEMPLATE, id);

	char* feed_filename = malloc(sizeof(RSS_FILE_TEMPLATE) + sizeof(id));
	sprintf(feed_filename, RSS_FILE_TEMPLATE, id);
	
	FILE* feed_file;

	if(!(feed_file = fopen(feed_filename, "w"))) {
		write_error("Couldn't open feed file. Exiting.");
		exit(EXIT_FAILURE);
	}

	CURL* curl;
	CURLcode res;
	
	write_log("Came before curl global init");
	res = curl_global_init(CURL_GLOBAL_ALL);
	write_error("Finished global init");

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
	
	write_log("Curl cleanup.");

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
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	while (1) {
		read = getline(&line, &len, feed_file);
		if (read == -1) {
			free(line);
			break;
		}
		
		if (strstr(line, RSS_LINK)) {
			//extract magnet link
			char* start = strchr(line, RSS_START_CHAR) + 1;
			char* end = strchr(start, RSS_END_CHAR);
			char* magnet_link = malloc(end - start + 1);
			memcpy(magnet_link, start, end - start);
			magnet_link[end - start] = '\0';
			write_log(magnet_link);
			free(line);

			while (!strstr(line, RSS_SHOW_ID)) {
				if (getline(&line, &len, feed_file) == -1) {
					write_error("Premature end of feed file.");
					exit(EXIT_FAILURE);
				}
				free(line);
			}			
			//extract show id
			start = strchr(line, RSS_START_CHAR) + 1;
			end = strchr(start, RSS_END_CHAR);
			char* show_id_string[end - start + 1];
			memcpy(show_id_string, start, end - start);
			show_id_string[end - start] = '\0';
			
			write_log((const char*) show_id_string);
			int show_id = atoi((const char*) show_id_string);
			free(line);

			while (!strstr(line, RSS_EPISODE_ID)) {
				if (getline(&line, &len, feed_file) == -1) {
					write_error("Premature end of feed file.");
					exit(EXIT_FAILURE);
				}
				free(line);
			}			
			//extract episode id
			start = strchr(line, RSS_START_CHAR) + 1;
			end = strchr(start, RSS_END_CHAR);
			char episode_id_string[end - start + 1];
			memcpy(episode_id_string, start, end - start);
			episode_id_string[end - start] = '\0';
			
			write_log(episode_id_string);
			int episode_id = atoi((const char*) episode_id_string);
			
			//create entry for feed entry
			feed_entries = realloc(feed_entries, (sizeof(feed_entries) + 1) * sizeof(feed_entry*));
			feed_entries[sizeof(feed_entries)] = malloc(sizeof(feed_entry*));
			feed_entries[sizeof(feed_entries)]->magnet = magnet_link;
			feed_entries[sizeof(feed_entries)]->show_id = show_id;
			feed_entries[sizeof(feed_entries)]->episode_id = episode_id;
			free(line);
		}	
	}

	if (fclose(feed_file)) {
		write_error("Feed file could not be closed. Exiting.");
		exit(EXIT_FAILURE);
	}

	return feed_entries;
}
