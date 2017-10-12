#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "def.h"
#include "log.h"

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
	write_log("This is my message.");

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

	/* Close standard file descriptors */
	/*
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	*/

	while(1) {
		write_log("Writing a log entry.");
		write_error("Writing an error.");
		sleep(1);
	}
}
