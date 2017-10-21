#ifndef CONFIG_H
#define CONFIG_H

/** DEFINITIONS **/
//DIRECTORIES
#define DIR_CONF_ROOT ".config/"
#define DIR_CONF "showiumize/"
//FILES
#define FILE_CONFIG "./config"
#define FILE_FEED "./%s.rss"
//TOKENS
#define TOKEN_CONF_SEPARATOR ":"
#define TOKEN_CONF_KEY_SHOWRSS "showrss"
#define TOKEN_CONF_KEY_PREM_ID "premiumize_id"
#define TOKEN_CONF_KEY_PREM_PIN "premiumize_pin"
#define TOKEN_CONF_KEY_SERIES_FOLDER "series_folder"
//VALUES
#define CONF_VALUES_NUMBER 4

/** METHODS **/
conf_config* read_config_file(void);
char* extract_value(char*** key_value, const char* key);
char*** extract_lines(FILE* conf_file);
unsigned char change_config_dir(void);

#endif
