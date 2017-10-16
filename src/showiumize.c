#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "def.h"
#include "log.h"
#include "config.h"
#include "showrss.h"

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

	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
	
	/* New SID for child */
	sid = setsid();
	if (sid < 0) {
		write_error("Invalid SID.");
		exit(EXIT_FAILURE);
	}

	/* Change current working dir */
	if ((chdir("/")) < 0) {
		write_error("Couldn't change working dir.");
		exit(EXIT_FAILURE);
	}
	
	const conf_config* config = read_config_file();
	
	unsigned char blub = TRUE;
	while(blub) {
		handle_showrss(config->showrss);
		blub = FALSE;
	}
}
