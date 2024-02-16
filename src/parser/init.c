#include "parser.h"

parser_t *parser_create(config_t *config) {
	parser_t *parser = malloc(sizeof(parser_t));
	if (parser == NULL) {
		log_error("Failed to allocate parser");
		return NULL;
	}

	if (!child_create(parser, config->shell, config->argc, config->argv)) {
		log_error("Failed to create child");
		vterm_free(parser->vt);
		free(parser);
		return NULL;
	}

	if (!vt_create(parser, config->rows, config->cols)) {
		log_error("Failed to create vterm");
		free(parser);
		return NULL;
	}

	return parser;
}

int vt_create(parser_t *parser, int rows, int cols) {
	VTerm *vt = vterm_new(rows, cols);
	if (vt == NULL) {
		log_error("Failed to create vt");
		return 0;
	}

	VTermScreen *vt_screen = vterm_obtain_screen(vt);
	if (vt_screen == NULL) {
		log_error("Failed to obtain vt screen");
		vterm_free(vt);
		return 0;
	}

	VTermState *vt_state = vterm_obtain_state(vt);
	if (vt_state == NULL) {
		log_error("Failed to obtain vt state");
		vterm_free(vt);
		return 0;
	}

	VTermScreenCallbacks *cbs = malloc(sizeof(VTermScreenCallbacks));
	if (cbs == NULL) {
		log_error("Failed to allocate vt screen callbacks");
		vterm_free(vt);
		return 0;
	}

	cbs->bell = vt_bell;
	cbs->damage = vt_damage;
	cbs->movecursor = vt_movecursor;
	cbs->settermprop = vt_setprop;

	parser->vt = vt;
	parser->vt_screen = vt_screen;
	parser->vt_state = vt_state;
	parser->vt_callbacks = cbs;

	vterm_set_utf8(vt, 1);
	vterm_screen_enable_altscreen(vt_screen, 1);
	vterm_screen_enable_reflow(vt_screen, 1);
	vterm_screen_reset(vt_screen, 1);

	vterm_screen_set_callbacks(vt_screen, parser->vt_callbacks, parser);
	vterm_output_set_callback(vt, vt_output, &parser->child_fd);

	return 1;
}

int child_create(parser_t *parser, char *shell, int argc, char **argv) {
	parser->child_pid = forkpty(&parser->child_fd, NULL, NULL, NULL);
	if (parser->child_pid < 0) {
		log_error("Failed to forkpty");
		return 0;
	}

	char *exec_argv[64] = {shell};
	for (int i = 0; i < argc; i++) {
		exec_argv[i + 1] = argv[i];
	}

	exec_argv[argc + 1] = NULL;
	if (parser->child_pid == 0) {
		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);
		execv(shell, exec_argv);
		return 1; // Never hit anyways
	}

	struct sigaction sa;
	sa.sa_handler = sig_action;
	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);
	child_status = CHILD_STATUS_RUNNING;
	return 1;
}
