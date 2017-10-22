#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>

typedef struct conf_config {
	char* showrss;
	char* id;
	char* pin;
	char* series_folder;
	char* config_folder;
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

typedef struct prem_collection {
	prem_restart** restarts;
	size_t rcount;
	prem_download** downloads;
	size_t dcount;
} prem_collection;

#endif
