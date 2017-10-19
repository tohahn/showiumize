#ifndef PREMIUMIZE_H
#define PREMIUMIZE_H

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "showrss.h"

#define PREM_DIR "./premiumize/"
#define PREM_TEMP "prem_XXXXXX"
#define PREM_CREATE_LINK "https://www.premiumize.me/api/transfer/create"
#define PREM_CREATE_DATA "customer_id=%s&pin=%s&type=torrent&src=%s"

void handle_premiumize(feed_entry** to_download, const char* premiumize_pin, const char* premiumize_id, const char* series_folder);
void handle_one_transfer(const char* show_name, const char* premiumize_pin, const char* premiumize_id, const char* magnet);
void curl_create_transfer(FILE* temp_file, char* post_data);
void check_existing(const char* premiumize_pin, const char* premiumize_id, const char* series_folder);

void handle_premiumize(feed_entry** to_download, const char* premiumize_pin, const char* premiumize_id, const char* series_folder) {	
	curl_global_init(CURL_GLOBAL_ALL);
	
	if (!to_download) {
		check_existing(premiumize_pin, premiumize_id, series_folder);
	}

	size_t index = 0;
	feed_entry* curr;
	while ((curr = to_download[index++])) {
		handle_one_transfer(curr->show_name, premiumize_pin, premiumize_id, curr->magnet);
	}

	curl_global_cleanup();
}

void check_existing(const char* premiumize_pin, const char* premiumize_id, const char* series_folder) {
	return;
}

void handle_one_transfer(const char* show_name, const char* premiumize_pin, const char* premiumize_id, const char* magnet) {
	char* temp_name = malloc((strlen(PREM_TEMP) + 1) * sizeof(char));
	memcpy(temp_name, PREM_TEMP, strlen(PREM_TEMP));
	temp_name[strlen(PREM_TEMP)] = '\0';
	FILE* temp_file = fdopen(mkstemp(temp_name), "r+");
	if (!temp_file) {
		write_error("Couldn't open temp file for download.");
		free(temp_name);
	}
	free(temp_name);

	char* post_data = malloc((strlen(PREM_CREATE_DATA) - 6 + strlen(premiumize_pin) + strlen(premiumize_id) + strlen(magnet) + 1) * sizeof(char));
	sprintf(post_data, PREM_CREATE_DATA, premiumize_id, premiumize_pin, magnet);
	curl_create_transfer(temp_file, post_data);
	free(post_data);

	rewind(temp_file);
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	read = getline(&line, &len, temp_file);
	if (read == -1) {
		write_error("No response from premiumize.");
		free(line);
		exit(EXIT_FAILURE);
	}
	write_log(line);
	free(line);
}

void curl_create_transfer(FILE* temp_file, char* post_data) {
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, PREM_CREATE_LINK);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, temp_file);

		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			write_error("Curl failed to initiate transfer. Exiting.");
			exit(EXIT_FAILURE);
		}
		curl_easy_cleanup(curl);
	}
}

#endif
