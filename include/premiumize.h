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
#define PREM_UNSUCCESS_DIR "./restart/"
#define PREM_SUCCESS_DIR "./download/"
#define CURR_DIR_FILE "./%s"
#define PREM_TEMP "prem_XXXXXX"
#define PREM_CREATE_LINK "https://www.premiumize.me/api/transfer/create"
#define PREM_CREATE_DATA "customer_id=%s&pin=%s&type=torrent&src=%s"
#define PREM_HASH_START "\"id\""
#define PREM_HASH_END "\""

typedef struct restart {
	char* show_name;
	char* pin;
	char* id;
	char* magnet;
} restart;

typedef struct download {
	char* show_name;
	char* hash;
} download;

void handle_premiumize(feed_entry** to_download, const char* premiumize_pin, const char* premiumize_id, const char* series_folder);
void handle_one_transfer(const char* show_name, const char* premiumize_pin, const char* premiumize_id, const char* magnet);
void curl_create_transfer(FILE* temp_file, char* post_data);
void check_existing(const char* premiumize_pin, const char* premiumize_id, const char* series_folder);
void add_one_download(download** downloads, size_t size, char* hash, char* show_name);
void add_one_restart(restart** restarts, size_t size, char* pin, char* id, char* show_name, char* magnet);
void read_restart_dir(download** downloads, size_t dcount, restart** restarts, size_t rcount);
char* copy_string_memory(char* tocopy);
void write_restart_dir(restart** restarts);

char* copy_string_memory(char* tocopy) {
	char* copy = malloc((strlen(tocopy) + 1) * sizeof(char));
	
	memcpy(copy, tocopy, strlen(tocopy, (strlen(tocopy) + 1) * sizeof(char)));
	copy[strlen(tocopy)] = '\0';
	
	return copy;
}

void handle_premiumize(feed_entry** to_download, const char* premiumize_pin, const char* premiumize_id, const char* series_folder) {	
	curl_global_init(CURL_GLOBAL_ALL);
	
	if(access(PREM_DIR, R_OK | W_OK)) {
		write_log("./premiumize/ does not exist, creating it for further use.");
		if(mkdir(PREM_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./premiumize/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
		return NULL;
	}
	chdir(PREM_DIR);
	
	restart** restarts = NULL;
	size_t rcount = malloc(sizeof(size_t));
	rcount = 0;
	download** downloads = NULL;
	size_t dcount = malloc(sizeof(size_t));
	dcount = 0;

	if (to_download) {
		size_t index = 0;
		feed_entry* curr;
		
		while ((curr = to_download[index++])) {
			char* response = handle_one_transfer(curr->show_name, premiumize_pin, premiumize_id, curr->magnet);
			if (response) {
				add_one_download(downloads, ++dcount, response, copy_string_memory(curr->show_name));
			} else {
				add_one_restart(restarts, ++rcount, premiumize_pin, premiumize_id, copy_string_memory(curr->show_name), copy_string_memory(curr->magnet));
				free(response);
			}
		}
	}
	
	check_existing(downloads, &dcount, restarts, &rcount, premiumize_pin, premiumize_id, series_folder);

	curl_global_cleanup();
}

void check_existing(download** downloads, size_t* dc, restart** restarts, size_t* rc, char* pin, char* id, char* series_folder) {
	size_t dcount = *dc;
	size_t rcount = *rc;
	read_restart_dir(downloads, &dcount, restarts, &rcount);
	null_terminate_restart_array(restarts, &rcount);
	write_restart_dir(restarts);
	read_download_dir(downloads, &dcount);
	null_terminate_download_array(downloads, &dcount);
	char** check_transfers(downloads);
}

char** check_transfers(download** downloads) {
	
}

void read_download_dir(download** downloads, size_t* dc) {
	size_t dcount = *dc;
	if(access(PREM_SUCCESS_DIR, R_OK | W_OK)) {
		write_log("./download/ does not exist, creating it for further use.");
		if(mkdir(PREM_SUCCESS_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./download/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
		return NULL;
	}

	DIR* download_dir;
	if (!(download_dir = opendir(PREM_SUCCESS_DIR))) {
		write_error("Could not open ./download/. Exiting.");
		exit(EXIT_FAILURE);
	}
	chdir(PREM_SUCCESS_DIR);
	
	struct dirent* curr_entry;
	FILE* curr_entry_file;
	char* filename;
	size_t count = 0;
	while ((curr_entry = readdir(download_dir))) {
		if (!strcmp(curr_entry->d_name, ".") || !strcmp(curr_entry->d_name, "..")) {
			continue;
		}
		filename = malloc((strlen(curr_entry->d_name) + 3) * sizeof(char));
		sprintf(filename, CURR_DIR_FILE, curr_entry->d_name);
		if (!(curr_entry_file = fopen(filename, "r"))) {
			write_error("Could not open entry file for reading.");
			exit(EXIT_FAILURE);
		}
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* show_name = line;
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* hash = line;
		
		add_one_download(downloads, ++dcount, hash, show_name);

		if (fclose(curr_entry_file)) {
			write_error("Couldn't close entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
		if (remove(filename)) {
			write_error("Couldn't remove entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	closedir(download_dir);
}
void write_restart_dir(restart** restarts) {
	size_t index = 0;
	restart* curr;
	while ((curr = restarts[index++])) {
		char* temp_name = malloc((strlen(PREM_TEMP) + 1) * sizeof(char));
		memcpy(temp_name, PREM_TEMP, strlen(PREM_TEMP));
		temp_name[strlen(PREM_TEMP)] = '\0';
		FILE* temp_file = fdopen(mkstemp(temp_name), "w");
		if (!temp_file) {
			write_error("Couldn't open temp file for download.");
			free(temp_name);
		}
		
		fprintf(temp_file, "%s\n", curr->show_name);
		fprintf(temp_file, "%d\n", curr->pin);
		fprintf(temp_file, "%d\n", curr->id);
		fprintf(temp_file, "%s\n", curr->magnet);
		
		if (fclose(temp_file)) {
			write_error("Could not close feed entry file in showRSS directory.");
			exit(EXIT_FAILURE);
		}
		
		free(filename);
	}
	if (!chdir("..")) {
		write_error("Couldn't change back to premiumize dir. Exiting.");
		exit(EXIT_FAILURE);
	}
}

void read_restart_dir(download** downloads, size_t* dc, restart** restarts, size_t* rc) {
	size_t dcount = *dc;
	size_t rcount = *rc;
	if(access(PREM_UNSUCCESS_DIR, R_OK | W_OK)) {
		write_log("./restart/ does not exist, creating it for further use.");
		if(mkdir(PREM_UNSUCCESS_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./restart/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
		return NULL;
	}

	DIR* restart_dir;
	if (!(restart_dir = opendir(PREM_UNSUCCESS_DIR))) {
		write_error("Could not open ./restart/. Exiting.");
		exit(EXIT_FAILURE);
	}
	chdir(PREM_UNSUCCESS_DIR);
	
	struct dirent* curr_entry;
	FILE* curr_entry_file;
	char* filename;
	size_t count = 0;
	while ((curr_entry = readdir(restart_dir))) {
		if (!strcmp(curr_entry->d_name, ".") || !strcmp(curr_entry->d_name, "..")) {
			continue;
		}
		filename = malloc((strlen(curr_entry->d_name) + 3) * sizeof(char));
		sprintf(filename, CURR_DIR_FILE, curr_entry->d_name);
		if (!(curr_entry_file = fopen(filename, "r"))) {
			write_error("Could not open entry file for reading.");
			exit(EXIT_FAILURE);
		}
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* show_name = line;
		
		char* line = readline(curr_entry_file);
		int pin = atoi(line);
		free(line);

		line = readline(curr_entry_file);
		int id = atoi(line);
		free(line);
		
		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* magnet = line;
		
		char* res = handle_one_download(show_name, pin, id, magnet);
		if (res) {
			add_one_download(downloads, ++dcount, res, show_name);
			free(magnet);
		} else {
			add_one_restart(restarts, ++rcount, pin, id, show_name, magnet);
			free(response);
		}

		if (fclose(curr_entry_file)) {
			write_error("Couldn't close entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
		if (remove(filename)) {
			write_error("Couldn't remove entry file. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	closedir(restart_dir);
}

download** null_terminate_download_array(download** toterminate, size_t size) {
	if (!toterminate || size < 1) {
		return NULL;
	}

	toterminate = realloc(toterminate, (size + 1) * sizeof(download*));
	toterminate[size] = NULL;

	return toterminate;
}

restart** null_terminate_restart_array(restart** toterminate, size_t size) {
	if (!toterminate || size < 1) {
		return NULL;
	}

	toterminate = realloc(toterminate, (size + 1) * sizeof(restart*));
	toterminate[size] = NULL;

	return toterminate;
}

download** add_one_download(download** downloads, size_t size, char* res, char* show_name) {
	downloads = realloc(downloads, size * sizeof(download*));
	downloads[--size] = malloc(sizeof(download));
	
	downloads[size]->show_name = show_name;
	downloads[size]->hash = res;

	return downloads;
}

restart** add_one_restart(restart** restarts, size_t size, char* pin, char* id, char* show_name, char* magnet) {
	restarts = realloc(restarts, size * sizeof(restart*));
	restarts[--size] = malloc(sizeof(restart));
	
	restarts[size]->pin = pin;
	restarts[size]->id = id;
	restarts[size]->show_name = show_name;
	restarts[size]->magnet = magnet;

	return entries;
}

char* handle_one_transfer(const char* show_name, const char* premiumize_pin, const char* premiumize_id, const char* magnet) {
	char* temp_name = malloc((strlen(PREM_TEMP) + 1) * sizeof(char));
	memcpy(temp_name, PREM_TEMP, strlen(PREM_TEMP));
	temp_name[strlen(PREM_TEMP)] = '\0';
	FILE* temp_file = fdopen(mkstemp(temp_name), "r+");
	if (!temp_file) {
		write_error("Couldn't open temp file for download.");
		free(temp_name);
	}

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

	char* start;
	char* ret_val;
	if ((start = strstr(line, PREM_HASH_START))) {
		start += strlen(PREM_HASH_START) + 1;
		char* end = strchr(start, PREM_HASH_END);
		char* hash = malloc((size_t)(end - start + 1) * sizeof(char));
		memcpy(hash, start, (size_t)(end - start) * sizeof(char));
		hash[end - start] = '\0';
		ret_val = hash;
	} else {
		ret_val = NULL;
	}

	free(line);
	if (!fclose(temp_file)) {
		write_error("Couldn't close temp file. Exiting.");
		exit(EXIT_FAILURE);
	}
	remove(temp_name);
	free(temp_name);

	return ret_val;
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
