#include "parser.h"

void *child_read(void *user) {
	parser_t *parser = (parser_t *)user;
	int fd = parser->child_fd;
	struct timeval timeout = {0};
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;

	fd_set readfds;
	FD_ZERO(&readfds);

	while (parser->child_alive) {
		FD_SET(fd, &readfds);
		int size = select(fd + 1, &readfds, NULL, NULL, &timeout);

		if (size > 0) {
			// Page size is the most efficient buffer size in this case
			// It doesn't make sense to read more than a page at a time
			size_t page_size = sysconf(_SC_PAGESIZE);
			char line[page_size];

			size_t len = read(fd, line, sizeof(line));
			if (len < 0) {
				log_error("Failed to read from pty: %s", strerror(errno));
				break;
			}

			if (len == 0) {
				break;
			}

			vterm_input_write(parser->vt, line, len);
		}
	}

	pthread_exit(NULL);
}
