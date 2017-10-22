#ifndef CONFIG_H
#define CONFIG_H

/** INCLUDES **/
#include <stdio.h>
#include "structs.h"

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_CONF_ROOT ".config/"
#define DIR_CONF "showiumize/"
//FILES
#define FILE_CONFIG "./config"
#define FILE_FEED "./%s.rss"
//TOKENS
#define TOKEN_ENV_XDG "XDG_CONFIG_HOME"
#define TOKEN_ENV_HOME "HOME"
#define TOKEN_CONF_SEPARATOR ":"
#define TOKEN_CONF_KEY_SHOWRSS "showrss"
#define TOKEN_CONF_KEY_PREM_ID "premiumize_id"
#define TOKEN_CONF_KEY_PREM_PIN "premiumize_pin"
#define TOKEN_CONF_KEY_SERIES_FOLDER "series_folder"
//VALUES
#define CONF_VALUES_NUMBER 4
#define CONF_INDEX_KEYS 0
#define CONF_INDEX_VALUES 1
//FORMATS
#define FORMAT_CONF_XDG "%s/%s"
#define FORMAT_CONF_HOME "%s/%s%s"

/** METHODS **/
conf_config* read_config_file(void);
char* change_config_dir(void);
char*** extract_lines(FILE* conf_file);
char* extract_value(char*** key_value, const char* key);

#endif
