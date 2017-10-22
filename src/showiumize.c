#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "definitions.h"
#include "messages.h"
#include "structs.h"
#include "log.h"
#include "config.h"
#include "showrss.h"
#include "premiumize.h"
#include "curl/curl.h"

int main(void) {
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* For good PID, exit parent */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* u+rw,g+r,o+r */
	umask(0);

	/* open logs */
	open_log();

	open(DEV_NULL, O_RDONLY);
	open(DEV_NULL, O_RDWR);
	open(DEV_NULL, O_RDWR);

	/* New SID for child */
	sid = setsid();
	if (sid < 0) {
		write_error(ERROR_SID);
		exit(EXIT_FAILURE);
	}

	/* Change current working dir */
	if ((chdir(ROOT_DIR)) < 0) {
		write_error(ERROR_WORKING_DIR);
		exit(EXIT_FAILURE);
	}

	const conf_config* config = read_config_file();

	while(TRUE) {
		curl_global_init(CURL_GLOBAL_ALL);
		
		rss_entry** unread = handle_showrss(config->showrss);
		handle_premiumize(unread, config->pin, config->id, config->series_folder, config->config_folder);

		curl_global_cleanup();
		sleep(SLEEP_IN_SECONDS);
	}
}
