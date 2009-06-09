#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char **argv) {
	int r;
	int w;
	int w2;

	if (argc <= 1) {
		fprintf(stderr, "iftty: no command\n");

		return 1;
	}

	if (argc > 1 && isatty(STDOUT_FILENO)) {
		execvp(argv[1], argv+1);
		
		perror("iftty, execvp");
		return 1;
	} else {
		char buffer[BUFSIZ];

		for (;;) {
			r = read(STDIN_FILENO, buffer, sizeof(buffer));
			if (r == 0) {
				break;
			} else if (r == -1) {
				perror("iftty, read");
				return 1;
			} else {
				w2 = 0;
				while (w2 < r) {
					w = write(STDOUT_FILENO, buffer + w2, r - w2);
					if (w == -1) {
						perror("iftty, write");
						return 1;
					}
					w2 += w;
				}
			}
		}
	}

	return 0;
}
