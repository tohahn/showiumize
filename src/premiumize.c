#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "definitions.h"
#include "showrss.h"
#include "premiumize.h"

char* copy_string_memory(const char* tocopy) {
	char* copy = malloc((strlen(tocopy) + 1) * sizeof(char));

	memcpy(copy, tocopy, (strlen(tocopy) + 1) * sizeof(char));
	copy[strlen(tocopy)] = '\0';

	return copy;
}

void handle_premiumize(feed_entry** to_download, char* premiumize_pin, char* premiumize_id, char* series_folder) {
	curl_global_init(CURL_GLOBAL_ALL);

	if(access(PREM_DIR, R_OK | W_OK)) {
		write_log("./premiumize/ does not exist, creating it for further use.");
		if(mkdir(PREM_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./premiumize/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
	}
	chdir(PREM_DIR);

	restart** restarts = NULL;
	size_t* rcount = malloc(sizeof(size_t));
	*rcount = 0;
	download** downloads = NULL;
	size_t* dcount = malloc(sizeof(size_t));
	*dcount = 0;

	if (to_download) {
		size_t index = 0;
		feed_entry* curr;

		while ((curr = to_download[index++])) {
			char* response = handle_one_download(premiumize_pin, premiumize_id, curr->magnet);
			if (response) {
				add_one_download(downloads, ++*dcount, response, copy_string_memory(curr->show_name));
				free(response);
			} else {
				add_one_restart(restarts, ++*rcount, premiumize_pin, premiumize_id, copy_string_memory(curr->show_name), copy_string_memory(curr->magnet));
			}
		}
	}

	check_existing(downloads, dcount, restarts, rcount, premiumize_pin, premiumize_id, series_folder);

	free(dcount);
	free(rcount);
	rss_free_array(to_download);
	free_download_array(downloads);
	free_restart_array(restarts);

	curl_global_cleanup();
}

void check_existing(download** downloads, size_t* dc, restart** restarts, size_t* rc, char* pin, char* id, char* series_folder) {
	size_t dcount = *dc;
	size_t rcount = *rc;
	read_restart_dir(downloads,dc, restarts, rc);
	null_terminate_restart_array(restarts, rcount);
	write_restart_dir(restarts);
	read_download_dir(downloads, &dcount);
	null_terminate_download_array(downloads, dcount);
	download** unfinished = check_transfers(downloads, id, pin, series_folder);
	if (unfinished) {
		write_download_dir(unfinished);
	}
	free_download_array(unfinished);
}

void free_download_array(download** tofree) {
	if (!tofree) return;
	size_t index = 0;
	download* curr;;
	while ((curr = tofree[index++])) {
		free(curr->show_name);
		free(curr->hash);
		free(curr);
	}
	free(tofree);
}

void free_restart_array(restart** tofree) {
	if (!tofree) return;
	size_t index = 0;
	restart* curr;;
	while ((curr = tofree[index++])) {
		free(curr->show_name);
		free(curr->pin);
		free(curr->id);
		free(curr->magnet);
		free(curr);
	}
	free(tofree);
}

void write_download_dir(download** unfinished) {
	size_t index = 0;
	download* curr;
	while ((curr = unfinished[index++])) {
		char* temp_name = malloc((strlen(PREM_TEMP) + 1) * sizeof(char));
		memcpy(temp_name, PREM_TEMP, strlen(PREM_TEMP));
		temp_name[strlen(PREM_TEMP)] = '\0';
		FILE* temp_file = fdopen(mkstemp(temp_name), "w");
		if (!temp_file) {
			write_error("Couldn't open temp file for download.");
			free(temp_name);
		}

		fprintf(temp_file, "%s\n", curr->show_name);
		fprintf(temp_file, "%s\n", curr->hash);

		if (fclose(temp_file)) {
			write_error("Could not close feed entry file in showRSS directory.");
			exit(EXIT_FAILURE);
		}

		free(temp_name);
	}
	if (!chdir("../..")) {
		write_error("Couldn't change back to premiumize dir. Exiting.");
		exit(EXIT_FAILURE);
	}
}
download** check_transfers(download** downloads, char* id, char* pin, char* series_folder) {
	download** unfinished = NULL;
	size_t ucount = 0;
	download* curr;
	size_t index = 0;
	while ((curr = downloads[index++])) {
		unsigned char res = handle_one_transfer(curr->show_name, curr->hash, id, pin, series_folder);
		if (!res) {
			add_one_download(unfinished, ++ucount, copy_string_memory(curr->hash), copy_string_memory(curr->show_name));
		}
	}
	if (unfinished) {
		null_terminate_download_array(unfinished, ucount);
	}

	return unfinished;
}

unsigned char handle_one_transfer(char* show_name, char* hash, char* id, char* pin, char* series_folder) {
	//download status
	char* data = malloc((strlen(PREM_STATUS_DATA) - 6 + strlen(hash) + strlen(id) + strlen(pin) + 1) * sizeof(char));
	char* temp_name = malloc((strlen(PREM_TEMP) + 1) * sizeof(char));
	memcpy(temp_name, PREM_TEMP, strlen(PREM_TEMP));
	temp_name[strlen(PREM_TEMP)] = '\0';
	FILE* temp_file = fdopen(mkstemp(temp_name), "w");
	if (!temp_file) {
		write_error("Couldn't open temp file for download.");
		free(temp_name);
	}
	curl_create_transfer(temp_file, copy_string_memory(PREM_STATUS_LINK), data);

	//read result into local storage
	temp_file = freopen(temp_name, "r", temp_file);
	char* temp_line = NULL;
	size_t len = 0;
	ssize_t read;
	read = getline(&temp_line, &len, temp_file);
	if (read == -1) {
		write_error("Couldn't read results from premiumize.");
		free(temp_line);
		exit(EXIT_FAILURE);
	}
	char* line = malloc((strlen(temp_line) + 1) * sizeof(char));
	memcpy(line, temp_line, strlen(temp_line));
	line[strlen(temp_line)] = '\0';

	//free memory, close file etc.
	fclose(temp_file);
	remove(temp_name);
	free(temp_name);
	free(temp_line);

	//handle result
	unsigned char success = FALSE;
	char* start = NULL;
	if ((start = strstr(line, PREM_SUCCESS_TOKEN))) {
		success = start_download(show_name, start, series_folder);
	}
	free(line);

	return success;
}

unsigned char start_download(char* show_name, char* line, char* series_folder) {
	char* end = NULL;
	while ((line = strstr(line, PREM_SUCCESS_TOKEN))) {
		line += strlen(PREM_SUCCESS_TOKEN);
		end = strchr(line, PREM_SUCCESS_END);
		char* size_string = malloc((size_t)(end - line + 2) * sizeof(char));
		memcpy(size_string, line, (size_t)(end - line + 1) * sizeof(char));
		size_string[end - line + 1] = '\0';
		int size = atoi(size_string);
		free(size_string);

		if (size > PREM_MIN_SIZE) {
			break;
		}
	}
	line = strstr(line, PREM_LINK_TOKEN);
	if (strstr(line, PREM_LINK_FALSE_TOKEN)) {
		line = strstr(line+1, PREM_LINK_TOKEN);
	}
	line += strlen(PREM_LINK_TOKEN);
	end = strchr(line, PREM_LINK_END);
	char* link_string = malloc((size_t)(end - line + 2) * sizeof(char));
	memcpy(link_string, line, (size_t)(end - line + 1) * sizeof(char));
	link_string[end - line + 1] = '\0';

	if(access(series_folder, R_OK | W_OK)) {
		write_log("series folder does not exist, creating it for further use.");
		if(mkdir(series_folder, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create series folder. Missing permissions?");
			exit(EXIT_FAILURE);
		}
	}

	size_t size = strlen(series_folder) + strlen(show_name) + 2;
	char* show_folder = malloc(size * sizeof(char));
	memcpy(show_folder, series_folder, strlen(series_folder));
	char* show_mid = show_folder+strlen(series_folder);
	memcpy(show_mid, show_name, strlen(show_name));
	show_folder[size-2] = '/';
	show_folder[size-1] = '\0';

	if(access(show_folder, R_OK | W_OK)) {
		write_log("Show folder does not exist, creating it for further use.");
		if(mkdir(show_folder, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create show folder. Missing permissions?");
			exit(EXIT_FAILURE);
		}
	}

	char* wget_command = malloc((strlen(WGET_STRING) - 4 + strlen(show_folder) + strlen(link_string)) * sizeof(char));
	sprintf(wget_command, WGET_STRING, show_folder, link_string);
	popen(wget_command, "r");

	free(wget_command);
	free(show_folder);
	free(link_string);

	return TRUE;
}

void read_download_dir(download** downloads, size_t* dc) {
	size_t dcount = *dc;
	if(access(PREM_SUCCESS_DIR, R_OK | W_OK)) {
		write_log("./download/ does not exist, creating it for further use.");
		if(mkdir(PREM_SUCCESS_DIR, S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH)) {
			write_error("Could not create ./download/. Missing permissions?");
			exit(EXIT_FAILURE);
		}
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

		char* line = readline(curr_entry_file);
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
		fprintf(temp_file, "%s\n", curr->pin);
		fprintf(temp_file, "%s\n", curr->id);
		fprintf(temp_file, "%s\n", curr->magnet);

		if (fclose(temp_file)) {
			write_error("Could not close feed entry file in showRSS directory.");
			exit(EXIT_FAILURE);
		}

		free(temp_name);
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

		char* line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* pin = line;

		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* id = line;

		line = readline(curr_entry_file);
		*strchr(line, '\n') = '\0';
		char* magnet = line;

		char* res = handle_one_download(pin, id, magnet);
		if (res) {
			add_one_download(downloads, ++dcount, res, show_name);
			free(magnet);
			free(pin);
			free(id);
		} else {
			add_one_restart(restarts, ++rcount, pin, id, show_name, magnet);
		}
		free(res);

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

	return restarts;
}

char* handle_one_download(const char* premiumize_pin, const char* premiumize_id, const char* magnet) {
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
	curl_create_transfer(temp_file, copy_string_memory(PREM_CREATE_LINK), post_data);

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

void curl_create_transfer(FILE* temp_file, char* link, char* post_data) {
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, link);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, temp_file);

		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			write_error("Curl failed to initiate transfer. Exiting.");
			exit(EXIT_FAILURE);
		}
		curl_easy_cleanup(curl);
		free(link);
		free(post_data);
	}
}
