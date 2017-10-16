#ifndef CONFIG_H
#define CONFIG_H

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
#define CONF_KEY_VALUE_SEPARATOR ":"

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

conf_config* read_config_file(void);
char* extract_value(char*** key_value, const char* key);
char*** extract_lines(FILE* conf_file);
unsigned char change_config_dir(void);

conf_config* read_config_file() {
	change_config_dir();

	FILE* conf_file = fopen("./" CONF_FILE_NAME, "r");
	if (!conf_file) {
		write_error("Couldn't read configuration file. Exiting.");
		exit(EXIT_FAILURE);
	}

	// reading config file
	conf_config config;
	char*** key_values = extract_lines(conf_file);

	write_log("Reading configuration value for key " CONF_KEY_SHOWRSS ".");
	config.showrss = extract_value(key_values, CONF_KEY_SHOWRSS);
	
	write_log("Reading configuration value for key " CONF_KEY_PREM_ID ".");
	config.premiumize_id = extract_value(key_values, CONF_KEY_PREM_ID);
	
	write_log("Reading configuration value for key " CONF_KEY_PREM_PIN ".");
	config.premiumize_pin = extract_value(key_values, CONF_KEY_PREM_PIN);
	
	write_log("Reading configuration value for key " CONF_KEY_SERIES_FOLDER ".");
	config.series_folder = extract_value(key_values, CONF_KEY_SERIES_FOLDER);

	// checking series folder for writability
	if (access(config.series_folder, W_OK)) {
		write_error("Can't access folder for writing. Trying to create it.");

		if (mkdir(config.series_folder, 0755)) {
			write_error("Couldn't open or create series folder. Exiting.");
			exit(EXIT_FAILURE);
		}
		if (access(config.series_folder, W_OK)) {
			write_error("Made series folder, but can't open it. Exiting.");
			exit(EXIT_FAILURE);
		}
	}
	free(key_values);
	free(key_values[0]);
	free(key_values[1]);

	//copy generated config to heap
	conf_config* heapConf = malloc(sizeof(conf_config));
	memcpy(heapConf, &config, sizeof(conf_config));

	return heapConf;
}

char* extract_value(char*** key_value, const char* key) {
	for (int i = 0; i < CONF_NUMBER_VALUES; i++) {
		if (!strcmp(key, key_value[0][i])) {
			free(key_value[0][i]);
			return key_value[1][i];
		}
	}
	write_error("Configuration value couldn't be found. Exiting.");
	exit(EXIT_FAILURE);
}

char*** extract_lines(FILE* conf_file) {
	char*** key_value = (char***) malloc(2 * sizeof(char**));
	
	char** keys = (char**) malloc(CONF_NUMBER_VALUES * sizeof(char*));
	key_value[0] = keys;

	char** values = (char**) malloc(CONF_NUMBER_VALUES * sizeof(char*));
	key_value[1] = values;

	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	for (int i = 0; i < CONF_NUMBER_VALUES; i++) {
		char* token;

		read = getline(&line, &len, conf_file);
		if (read == -1) {
			write_error("Too few / malformed lines. Exiting.");
			exit(EXIT_FAILURE);
		}
		
		token = strsep(&line, CONF_KEY_VALUE_SEPARATOR);
		key_value[0][i] = strdup(token);

		token = strsep(&line, CONF_KEY_VALUE_SEPARATOR);
		char* newline = strchr(token, '\n');
		*newline = '\0';
		key_value[1][i] = strdup(token);
			
		if (strsep(&line, CONF_KEY_VALUE_SEPARATOR)) {
			write_error("Malformed line: Two colons in one line. Exiting.");
			exit(EXIT_FAILURE);
		}
		free(line);
	}
	if (getline(&line, &len, conf_file) > -1) {
		write_error("Too many lines in config file. Continuing.");
	}

	return key_value;
}

unsigned char change_config_dir() {
	char* temp_dir;
	char* conf_xdg_conf = getenv("XDG_CONFIG_HOME");
	if (conf_xdg_conf) {
		temp_dir = (char *) malloc(sizeof(conf_xdg_conf) + 1 + sizeof(CONF_CONF_FOLDER));
		sprintf(temp_dir, "%s/%s", conf_xdg_conf, CONF_CONF_FOLDER);
	} else {
		write_log("No XDG_CONFIG_HOME variable, using $HOME/.config instead.");

		char* conf_home = getenv("HOME");
		if (!conf_home) {
			write_error("No home variable set. Exiting.");
			exit(EXIT_FAILURE);
		}
		temp_dir = (char *) malloc(sizeof(conf_home) + sizeof(CONF_CONF_DIR) + 2 + sizeof(CONF_CONF_FOLDER));
		sprintf(temp_dir, "%s/%s/%s", conf_home, CONF_CONF_DIR, CONF_CONF_FOLDER);
	}
	
	DIR* conf_dir;
	if (!(conf_dir = opendir(temp_dir))) {
		write_log("Config dir does not exist. Trying to create...");
		if (mkdir(temp_dir, 0755)) {
			write_error("Couldn't open or create configuration directory. Exiting.");
			exit(EXIT_FAILURE);
		}
		if ((conf_dir = opendir(temp_dir))) {
			write_error("Made configuration directory, but can't open it. Exiting.");
			exit(EXIT_FAILURE);
		}
	}

	if (closedir(conf_dir)) {
		write_error("Couldn't close configuration directory. Continuing.");
	}

	if (chdir(temp_dir)) {
		write_error("Couldn't change to configuration directory. Exiting.");
		exit(EXIT_FAILURE);
	}

	free(temp_dir);
	return TRUE;
}

#endif
