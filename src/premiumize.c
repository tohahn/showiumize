/** INCLUDE **/
//STDLIB
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
//CUSTOM
#include "premiumize.h"
#include "definitions.h"
#include "messages.h"
#include "structs.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "curl_help.h"

void handle_premiumize(rss_entry** to_download, char* pin, char* id, char* series_folder) {
	if (!check_file_dir(DIR_PREMIUMIZE_ROOT, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_ROOT, FILE_PERMISSIONS);
	}
	change_dir(DIR_PREMIUMIZE_ROOT);

	prem_restart*** restarts = easy_malloc(sizeof(prem_restart**));
	*restarts = easy_malloc(sizeof(prem_restart*));
	size_t* rcount = easy_malloc(sizeof(size_t));
	*rcount = 0;
	prem_download*** downloads = easy_malloc(sizeof(prem_download**));
	*downloads = easy_malloc(sizeof(prem_download*));
	size_t* dcount = easy_malloc(sizeof(size_t));
	*dcount = 0;

	create_transfers_for_feed_items(to_download, downloads, dcount, restarts, rcount, pin, id);
	read_restart_dir(downloads, dcount, restarts, rcount);
	*restarts = prem_restart_null_terminate(*restarts, *rcount);
	write_restart_dir(*restarts);
	read_download_dir(downloads, dcount);
	*downloads = prem_download_null_terminate(*downloads, *dcount);
	prem_download** unfinished = check_transfers(*downloads, id, pin, series_folder);
	if (unfinished) {
		write_download_dir(unfinished);
	}

	free(dcount);
	free(rcount);
	rss_entry_free(to_download);
	prem_download_free(unfinished);
	prem_download_free(*downloads);
	free(downloads);
	prem_restart_free(*restarts);
	free(restarts);
}

void create_transfers_for_feed_items(rss_entry** to_download, prem_download*** downloads_ptr, size_t* dc, prem_restart*** restarts_ptr, size_t* rc, char* pin, char* id) {
	if (!to_download) {
		return;
	}

	prem_download** downloads = *downloads_ptr;
	prem_restart** restarts = *restarts_ptr;

	size_t index = 0;
	rss_entry* curr;
	while ((curr = to_download[index++])) {
		char* response = handle_one_transfer(pin, id, curr->magnet);
		if (response) {
			++*dc;
			downloads = prem_download_add_one(downloads, dc, response, curr->show_name);
			free(response);
		} else {
			++*rc;
			restarts = prem_restart_add_one(restarts, rc, curr->show_name, id, pin, curr->magnet);
		}
		free(response);
	}
}

char* handle_one_transfer(const char* pin, const char* id, const char* magnet) {
	//FILE* temp_file = open_file(FILE_TEMP_NAME, FILE_WRITE_MODE);
	char* post_data = easy_printf(FORMAT_PREM_DATA_CREATE, id, pin, magnet);
	char* line = read_curl(copy_string_to_heap(LINK_PREM_CREATE), post_data);

	//fclose(temp_file);
	//open_file(FILE_TEMP_NAME, FILE_READ_MODE);

	//char* line = easy_readline(temp_file, ERROR_CREATING_TRANSFER_RESPONSE);
	char* ret_val = extract_string_from_line_without_exit(line, TOKEN_PREM_HASH_START, TOKEN_PREM_HASH_END);

	free(line);
	//close_file(temp_file);
	//remove(FILE_TEMP_NAME);

	return ret_val;
}

void read_restart_dir(prem_download*** downloads_ptr, size_t* dc, prem_restart*** restarts_ptr, size_t* rc) {
	prem_download** downloads = *downloads_ptr;
	prem_restart** restarts = *restarts_ptr;
	//char cwd[1024];
	//write_log_var("CWD: %s", getcwd(cwd, (cwd)));
	if (!check_file_dir(DIR_PREMIUMIZE_RESTART, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_RESTART, FILE_PERMISSIONS);
	}
	DIR* restart_dir = open_dir(DIR_PREMIUMIZE_RESTART);
	change_dir(DIR_PREMIUMIZE_RESTART);

	struct dirent* curr_entry;
	while ((curr_entry = readdir(restart_dir))) {
		if (!strcmp(curr_entry->d_name, CURRENT_DIR) || !strcmp(curr_entry->d_name, PREVIOUS_DIR)) {
			continue;
		}
		char* filename = easy_printf(FILE_CURR_DIR, curr_entry->d_name);
		FILE* curr_entry_file = open_file(filename, FILE_READ_MODE);

		char* line = easy_readline(curr_entry_file, ERROR_MALFORMED_RESTART_FILE);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_RESTART_FILE);
		*strchr(line, '\n') = '\0';
		char* pin = line;

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_RESTART_FILE);
		*strchr(line, '\n') = '\0';
		char* id = line;

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_RESTART_FILE);
		*strchr(line, '\n') = '\0';
		char* magnet = line;

		char* res = handle_one_transfer(pin, id, magnet);
		if (res) {
			++*dc;
			downloads = prem_download_add_one(downloads, dc, show_name, res);
		} else {
			++*rc;
			restarts = prem_restart_add_one(restarts, rc, show_name, id, pin, magnet);
		}
		free(show_name);
		free(res);
		free(id);
		free(pin);
		free(magnet);

		close_file(curr_entry_file);
		remove_file(filename);
		free(filename);
	}
	close_dir(restart_dir);
	change_dir(PREVIOUS_DIR);
}

void write_restart_dir(prem_restart** restarts) {
	char cwd[1024];
	write_log_var("CWD: %s", getcwd(cwd, sizeof(cwd)));
	if (!check_file_dir(DIR_PREMIUMIZE_RESTART, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_RESTART, FILE_PERMISSIONS);
	}
	change_dir(DIR_PREMIUMIZE_RESTART);

	size_t index = 0;
	prem_restart* curr;
	while ((curr = restarts[index++])) {
		FILE* temp_file = open_temp_file();

		fprintf(temp_file, FORMAT_STRING, curr->show_name);
		fprintf(temp_file, FORMAT_STRING, curr->pin);
		fprintf(temp_file, FORMAT_STRING, curr->id);
		fprintf(temp_file, FORMAT_STRING, curr->magnet);

		close_file(temp_file);
	}
	change_dir(PREVIOUS_DIR);
}

void read_download_dir(prem_download*** downloads_ptr, size_t* dc) {
	prem_download** downloads = *downloads_ptr;

	if (!check_file_dir(DIR_PREMIUMIZE_DOWNLOAD, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_DOWNLOAD, FILE_PERMISSIONS);
	}

	DIR* download_dir = open_dir(DIR_PREMIUMIZE_DOWNLOAD);

	struct dirent* curr_entry;
	while ((curr_entry = readdir(download_dir))) {
		if (!strcmp(curr_entry->d_name, CURRENT_DIR) || !strcmp(curr_entry->d_name, PREVIOUS_DIR)) {
			continue;
		}
		char* filename = easy_printf(FILE_CURR_DIR, curr_entry->d_name);
		FILE* curr_entry_file = open_file(filename, FILE_READ_MODE);

		char* line = easy_readline(curr_entry_file, ERROR_MALFORMED_DOWNLOAD_FILE);
		*strchr(line, '\n') = '\0';
		char* show_name = line;

		line = easy_readline(curr_entry_file, ERROR_MALFORMED_DOWNLOAD_FILE);
		*strchr(line, '\n') = '\0';
		char* hash = line;

		++*dc;
		downloads = prem_download_add_one(downloads, dc, show_name, hash);

		free(show_name);
		free(hash);

		close_file(curr_entry_file);
		remove_file(filename);
		free(filename);
	}
	close_dir(download_dir);
}

prem_download** check_transfers(prem_download** downloads, char* id, char* pin, char* series_folder) {
	prem_download** unfinished = NULL;
	size_t ucount = 0;
	prem_download* curr;
	size_t index = 0;
	while ((curr = downloads[index++])) {
		unsigned char res = handle_one_download(series_folder, curr->show_name, curr->hash, id, pin);
		if (!res) {
			++ucount;
			unfinished = prem_download_add_one(unfinished, &ucount, curr->hash, curr->show_name);
		}
	}
	if (unfinished) {
		unfinished = prem_download_null_terminate(unfinished, ucount);
	}

	return unfinished;
}

unsigned char handle_one_download(char* series_folder, char* show_name, char* hash, char* id, char* pin) {
	//download status
	char* data = easy_printf(FORMAT_PREM_DATA_STATUS, hash, id, pin);
	FILE* temp_file = open_file(FILE_TEMP_NAME, FILE_WRITE_MODE);
	curl_create_transfer(temp_file, copy_string_to_heap(LINK_PREM_STATUS), data);

	//read result into local storage
	close_file(temp_file);
	temp_file = open_file(FILE_TEMP_NAME, FILE_READ_MODE);
	char* line = easy_readline(temp_file, ERROR_CHECKING_STATUS_RESPONSE);

	//free memory, close file etc.
	close_file(temp_file);
	remove(FILE_TEMP_NAME);

	//handle result
	unsigned char success = FALSE;
	char* start = NULL;
	if ((start = strstr(line, TOKEN_PREM_SIZE_START))) {
		success = start_download(show_name, start, series_folder);
	}
	free(line);

	return success;
}

unsigned char start_download(char* line, char* series_folder, char* show_name) {
	while ((line = strstr(line, TOKEN_PREM_SIZE_START))) {
		int size = extract_number_from_line(line, TOKEN_PREM_SIZE_START, TOKEN_PREM_SIZE_END);
		line++;

		if (size > PREM_MIN_SIZE) {
			break;
		}
	}
	char* link = extract_string_from_line(line, TOKEN_PREM_LINK_START, TOKEN_PREM_LINK_END);

	if (!check_file_dir(series_folder, R_OK | W_OK)) {
		make_dir(series_folder, FILE_PERMISSIONS);
	}
	char* show_folder = easy_printf(FORMAT_SHOW_FOLDER, series_folder, show_name);
	if (!check_file_dir(show_folder, R_OK | W_OK)) {
		make_dir(show_folder, FILE_PERMISSIONS);
	}
	change_dir(show_folder);

	char* curl_command = easy_printf(FORMAT_CURL, link);
	popen(curl_command, FILE_READ_MODE);

	free(curl_command);
	free(show_folder);
	free(link);

	change_config_dir();

	return TRUE;
}

void write_download_dir(prem_download** unfinished) {
	if (!check_file_dir(DIR_PREMIUMIZE_ROOT, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_ROOT, FILE_PERMISSIONS);
	}
	change_dir(DIR_PREMIUMIZE_ROOT);
	if (!check_file_dir(DIR_PREMIUMIZE_DOWNLOAD, R_OK | W_OK)) {
		make_dir(DIR_PREMIUMIZE_DOWNLOAD, FILE_PERMISSIONS);
	}
	change_dir(DIR_PREMIUMIZE_DOWNLOAD);

	size_t index = 0;
	prem_download* curr;
	while ((curr = unfinished[index++])) {
		FILE* temp_file = open_temp_file();

		fprintf(temp_file, FORMAT_STRING, curr->show_name);
		fprintf(temp_file, FORMAT_STRING, curr->hash);

		close_file(temp_file);
	}
	change_dir(DOUBLE_PREVIOUS_DIR);
}

void curl_create_transfer(FILE* temp_file, char* link, char* post_data) {
	curl_global_init(CURL_GLOBAL_ALL);
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
			write_error(ERROR_CURL_TRANSFER);
			free(link);
			free(post_data);
			exit(EXIT_FAILURE);
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		free(link);
		free(post_data);
	}
}
