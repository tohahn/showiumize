#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct conf_config {
	char* showrss;
	char* id;
	char* pin;
	char* series_folder;
} conf_config;

typedef struct rss_entry {
	int show_id;
	char* show_name;
	int episode_id;
	char* magnet;
} rss_entry;

typedef struct prem_restart {
	char* show_name;
	char* id;
	char* pin;
	char* magnet;
} prem_restart;

typedef struct prem_download {
	char* show_name;
	char* hash;
} prem_download;

#endif
