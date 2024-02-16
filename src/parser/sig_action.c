#include "term.h"

void sig_action(int sig_type) {
	term_t *state = glfwGetWindowUserPointer(window);

	int status;
	waitpid(-1, &status, WNOHANG);

	log_info("Child terminated with status %d", status);
	vterm_input_write(state->parser->vt, "[Process terminated]\n", 21);
}
