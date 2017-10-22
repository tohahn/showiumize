/**INCLUDES **/
//STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
//CUSTOM
#include "config.h"
#include "utils.h"
#include "definitions.h"
#include "messages.h"
#include "structs.h"
#include "log.h"

/** METHODS **/
conf_config* read_config_file() {
	char* config_folder = change_config_dir();
	FILE* conf_file = open_file(FILE_CONFIG, FILE_READ_MODE);

	// reading config file
	conf_config* config = easy_malloc(sizeof(conf_config));
	char*** key_values = extract_lines(conf_file);

	config->showrss = extract_value(key_values, TOKEN_CONF_KEY_SHOWRSS);
	config->id = extract_value(key_values, TOKEN_CONF_KEY_PREM_ID);
	config->pin = extract_value(key_values, TOKEN_CONF_KEY_PREM_PIN);
	config->series_folder = extract_value(key_values, TOKEN_CONF_KEY_SERIES_FOLDER);
	config->config_folder = config_folder;

	// checking series folder for writability
	if (!check_file_dir(config->series_folder, W_OK)) {
		make_dir(config->series_folder, FILE_PERMISSIONS);
	}

	change_dir(config->config_folder);

	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		free(key_values[CONF_INDEX_KEYS][i]);
		free(key_values[CONF_INDEX_VALUES][i]);
	}
	free(key_values[CONF_INDEX_KEYS]);
	free(key_values[CONF_INDEX_VALUES]);
	free(key_values);

	return config;
}

char* change_config_dir() {
	char* dir_name;

	char* conf_xdg_conf = getenv(TOKEN_ENV_XDG);
	if (conf_xdg_conf) {
		dir_name = easy_printf(FORMAT_CONF_XDG, strlen(FORMAT_CONF_XDG) + strlen(conf_xdg_conf) + strlen(DIR_CONF) + 1, conf_xdg_conf, DIR_CONF);
	} else {
		write_error(ERROR_XDG_CONFIG);

		char* conf_home = getenv(TOKEN_ENV_HOME);
		if (!conf_home) {
			write_error(ERROR_HOME);
			exit(EXIT_FAILURE);
		}

		dir_name = easy_printf(FORMAT_CONF_HOME, strlen(FORMAT_CONF_HOME) + strlen(conf_home) + strlen(DIR_CONF_ROOT) + strlen(DIR_CONF) + 1, conf_home, DIR_CONF_ROOT, DIR_CONF);
	}

	if (!check_file_dir(dir_name, W_OK | R_OK)) {
		make_dir(dir_name, FILE_PERMISSIONS);
	}
	change_dir(dir_name);
	return dir_name;
}

char*** extract_lines(FILE* conf_file) {
	char*** key_value = easy_malloc(2 * sizeof(char**));

	char** keys = easy_malloc(CONF_VALUES_NUMBER * sizeof(char*));
	key_value[CONF_INDEX_KEYS] = keys;

	char** values = easy_malloc(CONF_VALUES_NUMBER * sizeof(char*));
	key_value[CONF_INDEX_VALUES] = values;

	char* newline = NULL;
	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		char* temp = easy_readline_var(conf_file, ERROR_MALFORMED_CONFIG, FILE_CONFIG);
		char* line = temp;
		char* token = strsep(&line, TOKEN_CONF_SEPARATOR);
		key_value[CONF_INDEX_KEYS][i] = copy_string_to_heap(token);

		token = strsep(&line, TOKEN_CONF_SEPARATOR);
		newline = strchr(token, '\n');
		*newline = '\0';
		key_value[CONF_INDEX_VALUES][i] = copy_string_to_heap(token);

		if ((line = strsep(&line, TOKEN_CONF_SEPARATOR))) {
			write_error(ERROR_MALFORMED_CONFIG);
		}
		free(temp);
	}

	return key_value;
}

char* extract_value(char*** key_value, const char* key) {
	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		if (!strcmp(key, key_value[CONF_INDEX_KEYS][i])) {
			key_value[CONF_INDEX_KEYS][i][0] = '\0';
			return copy_string_to_heap(key_value[CONF_INDEX_VALUES][i]);
		}
	}
	size_t size = strlen(ERROR_CONF_VALUE_NOT_FOUND) + strlen(key) + 1;
	write_error_var(ERROR_CONF_VALUE_NOT_FOUND, size, key);
	exit(EXIT_FAILURE);
}
