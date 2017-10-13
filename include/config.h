#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "def.h"

#define CONF_CONF_DIR ".config/"
#define CONF_CONF_FOLDER "showiumize/"
#define CONF_FILE_NAME "config"
#define CONF_KEY_VALUE_SEPERATOR ":"

#define CONF_NUMBER_VALUES 4
#define CONF_KEY_SHOWRSS "showrss"
#define CONF_KEY_PREM_ID "premiumize_id"
#define CONF_KEY_PREM_PIN "premiumize_pin"
#define CONF_KEY_SERIES_FOLDER "series_folder"

typedef struct conf_config {
	const char* showrss;
	const char* premiumize_id;
	const char* premiumize_pin;
	const char* series_folder;
} conf_config;

const char* conf_dir_path;

conf_config* read_config_file() {
	// opening config file
	FILE* conf_file = fopen(conf_dir_path CONF_FILE_NAME, "r");
	if (conf_file) {
		write_error("Couldn't read configuration file in %s. Exiting.", conf_dir_path CONF_FILE_NAME);
		exit(EXIT_FAILURE);
	}
	
	// reading config file
	conf_config config;
	const char*** key_values = extract_lines();
	config->showrss = extract_value(key_value, CONF_KEY_SHOWRSS);
	config->premiumize_id = extract_value(key_value, CONF_KEY_PREM_ID);
	config->premiumize_pin = extract_value(key_value, CONF_KEY_PREM_PIN);
	config->series_folder = extract_value(key_value, CONF_KEY_SERIES_FOLDER);

	// checking series folder for writability
	if (access(config->series_folder, W_OK)) {
		write_error("Can't access folder for writing. Trying to create it.");

		if (mkdir(config->series_folder, 0755)) {
			write_error("Couldn't open or create series folder. Exiting.");
			exit(EXIT_FAILURE);
		}
		if (access(config->series_folder, W_OK) {
			write_error("Made series folder, but can't open it. Exiting.");
			exit(EXIT_FAILURE);
		}
	}

	return config;
}

const char* extract_value(const char*** key_value, const char* key) {
	for (int i = 0; i < CONF_NUMBER_VALUES; i++) {
		if (!strcmp(key, key_value[0][i])) {
			return key_value[1][i];
		}
	}
	write_error("Configuration value for %s couldn't be found. Exiting.", key);
	exit(EXIT_FAILURE);
}

const char*** extract_lines() {
	const char** key_values[2];
	key_values[0] = keys[CONF_NUMBER_VALUES];
	key_values[1] = values[CONF_NUMBER_VALUES];
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	for (int i = 0; i < CONF_NUMBER_VALUES; i++) {
		read = getline(&line, &len, conf_file);
		if (read == -1) {
			write_error("Too few / malformed lines. Exiting.");
			exit(EXIT_FAILURE);
		}
		key_values[0][i] = strsep(line, CONF_KEY_VALUE_SEPARATOR);
		key_values[1][i] = strsep(line, CONF_KEY_VALUE_SEPARATOR);
		if (strsep(line, COF_KEY_VALUE_SEPARATOR)) {
			write_error("Malformed line: Two colons in one line. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	if (getline(&line, &len, conf_file) > -1) {
		write_error("Too many lines in config file. Continuing.");
	}

	free(line);
	return key_values;
}

unsigned char read_config_dir() {
	char* temp_dir;
	const char* conf_xdg_conf = getenv("XDG_CONFIG_HOME");
	if (conf_xdg_conf) {
		temp_dir = conf_xdg_conf;
	} else {
		write_log("No XDG_CONFIG_HOME variable, using $HOME/.config instead.");
		
		const char* conf_home = getenv("HOME");
		if (!conf_home) {
			write_error("No home variable set. Exiting.");
			exit(EXIT_FAILURE);
		}
		temp_dir = conf_home CONF_CONF_DIR;
	}

	conf_dir_path = temp_dir;

	DIR* conf_dir;

	if (!(dir = opendir(conf_dir_path))) {
		write_log("Config dir does not exist. Trying to create...");
		if (mkdir(conf_dir_path, 0755)) {
			write_error("Couldn't open or create configuration directory. Exiting.");
			exit(EXIT_FAILURE);
		}
		if (!(dir = opendir(conf_dir_path))) {
			write_error("Made configuration directory, but can't open it. Exiting.");
			exit(EXIT_FAILURE);
		}
	}

	if (closedir(conf_dir)) {
		write_error("Couldn't close configuration directory. Continuing.");
	}
	return TRUE;
}
