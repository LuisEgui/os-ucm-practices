#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[])
{
	pid_t pid;
	int status;

	if (argc != 2){
		fprintf(stderr, "Usage: %s <command>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	pid = fork();

	if(pid == 0) {
		/* Child */
		execl("/bin/sh", "sh", "-c", argv[1], (char *) NULL);
		exit(EXIT_FAILURE);
	}

	/* Parent */
	while(waitpid(pid, &status, 0) == -1) {
		if (errno != ECHILD) {
			fprintf(stderr, "ERROR when waiting for children to finish\n");
			exit(EXIT_FAILURE);
		}
	}

	exit(0);
}
