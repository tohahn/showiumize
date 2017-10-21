/**INCLUDES **/
//STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
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
	change_config_dir();
	FILE* conf_file = open_file(FILE_CONFIG, FILE_READ_MODE);

	// reading config file
	conf_config* config = easy_malloc(sizeof(conf_config));
	char*** key_values = extract_lines(conf_file);

	config->showrss = extract_value(key_values, TOKEN_CONF_KEY_SHOWRSS);
	config->id = extract_value(key_values, TOKEN_CONF_KEY_PREM_ID);
	config->pin = extract_value(key_values, TOKEN_CONF_KEY_PREM_PIN);
	config->series_folder = extract_value(key_values, TOKEN_CONF_KEY_SERIES_FOLDER);

	// checking series folder for writability
	if (check_file_dir(config->series_folder, W_OK)) {
		make_dir(config->series_folder, DIR_READ_WRITE);
	}

	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		free(key_values[CONF_INDEX_KEYS][i]);
		free(key_values[CONF_INDEX_VALUES][i]);
	}
	free(key_values[CONF_INDEX_KEYS]);
	free(key_values[CONF_INDEX_VALUES]);
	free(key_values);

	return config;
}

unsigned char change_config_dir() {
	char* dir_name;

	char* conf_xdg_conf = getenv(TOKEN_ENV_XDG);
	if (conf_xdg_conf) {
		dir_name = easy_printf(FORMAT_CONF_XDG, conf_xdg_conf, DIR_CONF);
	} else {
		write_error(ERROR_XDG_CONFIG);

		char* conf_home = getenv(TOKEN_ENV_HOME);
		if (!conf_home) {
			write_error(ERROR_HOME);
			exit(EXIT_FAILURE);
		}

		dir_name = easy_printf(FORMAT_CONF_HOME, conf_home, DIR_CONF_ROOT, DIR_CONF);
	}

	if (check_file_dir(dir_name, S_IRWXU)) {
		change_dir(dir_name);
	} else {
		exit(EXIT_FAILURE);
	}

	free(dir_name);
	return TRUE;
}

char*** extract_lines(FILE* conf_file) {
	char*** key_value = easy_malloc(2 * sizeof(char**));

	char** keys = easy_malloc(CONF_VALUES_NUMBER * sizeof(char*));
	key_value[CONF_INDEX_KEYS] = keys;

	char** values = easy_malloc(CONF_VALUES_NUMBER * sizeof(char*));
	key_value[CONF_VALUES_NUMBER] = values;

	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		char* line = easy_readline_var(conf_file, ERROR_MALFORMED_CONFIG, FILE_CONFIG);
		char* token = strsep(&line, TOKEN_CONF_SEPARATOR);
		key_value[CONF_INDEX_KEYS][i] = strdup(token);

		token = strsep(&line, TOKEN_CONF_SEPARATOR);
		char* newline = strchr(token, '\n');
		*newline = '\0';
		key_value[CONF_VALUES_NUMBER][i] = strdup(token);

		if ((line = strsep(&line, TOKEN_CONF_SEPARATOR))) {
			write_error_var(ERROR_MALFORMED_CONFIG, FILE_CONFIG);
			free(line);
			exit(EXIT_FAILURE);
		}
		free(line);
	}

	return key_value;
}

char* extract_value(char*** key_value, const char* key) {
	for (int i = 0; i < CONF_VALUES_NUMBER; i++) {
		if (!strcmp(key, key_value[0][i])) {
			key_value[CONF_INDEX_KEYS][i][0] = '\0';
			return copy_string_to_heap(key_value[CONF_INDEX_VALUES][i]);
		}
	}
	write_error_var(ERROR_CONF_VALUE_NOT_FOUND, key);
	exit(EXIT_FAILURE);
}
