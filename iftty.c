#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

int pipethru(int fd_in, int fd_out) {
	int r;
	int w;
	int w2;

	char buffer[BUFSIZ];

	for (;;) {
		r = read(fd_in, buffer, sizeof(buffer));
		if (r == 0) {
			break;
		} else if (r == -1) {
			perror("iftty, read");
			return 1;
		} else {
			w2 = 0;
			while (w2 < r) {
				w = write(fd_out, buffer + w2, r - w2);
				if (w == -1) {
					perror("iftty, write");
					return 1;
				}
				w2 += w;
			}
		}
	}

	return 0;
}

int pipize(char **argv) {
	execvp(argv[0], argv);

	perror("iftty, execvp");
	return 1;
}

int paginate(char **argv) {
	int r;
	int fildes[2];
	char *pager;
	pid_t pid;

	pager = getenv("PAGER");
	if (pager == NULL) pager = "less";

	r = pipe(fildes);
	if (r == -1) {
		perror("iftty, pipe");
		return 1;
	}

	pid = fork();
	if (pid == -1) {
		perror("iftty, fork");
		return 1;
	} else if (pid == 0) {
		/* child
		 * iftty
		 */
		close(fildes[0]);
		r = dup2(fildes[1], STDOUT_FILENO);
		if (r == -1) {
			perror("iftty, dup2");
			return 1;
		}

		return pipize(argv);

	} else {
		/* parent
		 * pager
		 */
		close(fildes[1]);
		r = dup2(fildes[0], STDIN_FILENO);
		if (r == -1) {
			perror("iftty, dup2");
			return 1;
		}

		char* args[] = { pager, NULL };
		return pipize(args);
	}
}

int usage() {
	printf("iffty version 2.0-rc1\n");
	printf("  -h  Print this message\n");
	printf("  -p  Paginate output with default pager ($PAGER)\n");
	return 0;
}

int main(int argc, char **argv) {
	int pager = 0;
	char c;

	while ((c = getopt(argc, argv, "hp")) != -1) {
		switch (c) {
			case 'p':
				pager = 1;
				break;

			case '?':
			case 'h':
			default:
				return usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0 && isatty(STDOUT_FILENO)) {
		if (pager == 0) {
			return pipize(argv);
		} else {
			return paginate(argv);
		}
	} else {
		return pipethru(STDIN_FILENO, STDOUT_FILENO);
	}
}
