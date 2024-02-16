#include "mac.h"
#include "term.h"

GLFWwindow *window;

void sig_handler(int sig_type) {
	child_state = 0;

	// Reap the child process
	int status;
	waitpid(-1, &status, WNOHANG);

	log_info("Child process terminated with status %d", status);
	term_t *state = glfwGetWindowUserPointer(window);
	vterm_input_write(state->vterm, "[Process terminated]\n", 21);
}

int damage(VTermRect rect, void *user) {
	term_t *state = (term_t *)user;
	state->gl_state.damage = rect;

	// The idea is to append to the scissor, so that redraw is minimized
	// If the bounds of damage are past the scissor, increase scissor size
	if (rect.start_col < state->gl_state.scissor.start_col) {
		state->gl_state.scissor.start_col = rect.start_col;
	}

	if (rect.start_row < state->gl_state.scissor.start_row) {
		state->gl_state.scissor.start_row = rect.start_row;
	}

	if (rect.end_col > state->gl_state.scissor.end_col) {
		state->gl_state.scissor.end_col = rect.end_col;
	}

	if (rect.end_row > state->gl_state.scissor.end_row) {
		state->gl_state.scissor.end_row = rect.end_row;
	}

	glfwPostEmptyEvent();
	return 1;
}

int moverect(VTermRect dest, VTermRect src, void *user) {
	return 1;
}

int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
	term_t *state = (term_t *)user;
	state->cursor.x = pos.col;
	state->cursor.y = pos.row;

	if (visible == 0) {
		// Works great for 'top' but not for 'nano'. Nano should have a cursor!
		// state->cursor.active = false;
	} else
		state->cursor.active = true;
	return 1;
}

int settermprop(VTermProp prop, VTermValue *val, void *user) {
	term_t *state = (term_t *)user;

	switch (prop) {
	case VTERM_PROP_ALTSCREEN:
		break;

	case VTERM_PROP_CURSORVISIBLE:
		break;

	case VTERM_PROP_TITLE:
		state->title = malloc(val->string.len + 1);
		memcpy(state->title, val->string.str, val->string.len);
		state->title[val->string.len] = '\0';

		state->states.reprop = true;
		break;

	case VTERM_PROP_ICONNAME:
		break;

	case VTERM_PROP_REVERSE:
		break;

	case VTERM_PROP_CURSORSHAPE:
		break;

	case VTERM_PROP_MOUSE:
		break;

	case VTERM_PROP_CURSORBLINK:
		break;

	default:
		return 0;
	}

	return 1;
}

int bell(void *user) {
	term_t *state = (term_t *)user;
	glfwRequestWindowAttention(state->glfw_window);
	return 1;
}

int sb_pushline(int cols, const VTermScreenCell *cells, void *user) {
	return 1;
}

int sb_popline(int cols, VTermScreenCell *cells, void *user) {
	return 1;
}

VTermScreenCallbacks callbacks = {
	.movecursor = movecursor,
	.sb_pushline = sb_pushline,
	.settermprop = settermprop,
	.bell = bell,
	.damage = damage,
};

void vterm_output_callback(const char *bytes, size_t len, void *user) {
	int fd = *(int *)user;
	write(fd, bytes, len);
}

void glfw_error_callback(int error, const char *description) {
	log_error("GLFW Error %d: %s", error, description);
}

void glfw_char_callback(GLFWwindow *window, unsigned int codepoint) {
	term_t *state = glfwGetWindowUserPointer(window);
	vterm_keyboard_unichar(state->vterm, codepoint, VTERM_MOD_NONE);
}

void glfw_resize_callback(GLFWwindow *window, int width, int height) {
	// int draw_width, draw_height;
	// glfwGetFramebufferSize(window, &draw_width, &draw_height);
	//
	// term_t *state = glfwGetWindowUserPointer(window);
	// resize_term(state, draw_width, draw_height);
	// render_term(state);
	// glfwSwapBuffers(window);
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	term_t *state = glfwGetWindowUserPointer(window);

	if (action != GLFW_PRESS && action != GLFW_REPEAT) {
		return;
	}

	// Support Command + Q and Windows + Q to quit
	if (action == GLFW_PRESS && key == GLFW_KEY_Q && mods & GLFW_MOD_SUPER) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (action == GLFW_PRESS) {
		handle_key(state, key, mods);
	}
}

void glfw_focus_callback(GLFWwindow *window, int focused) {
	term_t *state = glfwGetWindowUserPointer(window);
	state->states.focused = focused;
}

void glfw_window_refresh_callback(GLFWwindow *window) {
	term_t *state = glfwGetWindowUserPointer(window);
	render_term(state);
	glfwSwapBuffers(window);
}

int init_term(term_t *state, config_t *config) {
	if (config == NULL) {
		fprintf(stderr, "Config is NULL\n");
		return 0;
	}

	state->config = config;

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 0;
	}

	log_info("Loaded GLFW: %s", glfwGetVersionString());

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GL_FRAMEBUFFER_SRGB, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_ALPHA_BITS, 2);

	glfwWindowHint(GLFW_RED_BITS, 10);
	glfwWindowHint(GLFW_GREEN_BITS, 10);
	glfwWindowHint(GLFW_BLUE_BITS, 10);

	state->glfw_window = glfwCreateWindow(
		config->width,
		config->height,
		"trap2 Terminal",
		NULL,
		NULL);

	if (state->glfw_window == NULL) {
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		return 0;
	}

#ifdef __APPLE__
	darkenTitlebar();
#endif

	// The sigaction needs this for printing
	window = state->glfw_window;

	glfwSetWindowUserPointer(state->glfw_window, state);
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetInputMode(state->glfw_window, GLFW_STICKY_KEYS, GLFW_TRUE);

	glfwSetCharCallback(state->glfw_window, glfw_char_callback);
	glfwSetKeyCallback(state->glfw_window, glfw_key_callback);
	glfwSetFramebufferSizeCallback(state->glfw_window, glfw_resize_callback);
	glfwSetWindowFocusCallback(state->glfw_window, glfw_focus_callback);
	glfwSetWindowRefreshCallback(state->glfw_window, glfw_window_refresh_callback);

	state->vterm = vterm_new(state->config->rows, state->config->cols);
	if (state->vterm == NULL) {
		fprintf(stderr, "Failed to initialize vterm\n");
		return 0;
	}

	vterm_set_utf8(state->vterm, 1);

	state->vterm_screen = vterm_obtain_screen(state->vterm);
	if (state->vterm_screen == NULL) {
		fprintf(stderr, "Failed to obtain vterm screen\n");
		return 0;
	}

	vterm_screen_enable_altscreen(state->vterm_screen, 1);
	vterm_screen_enable_reflow(state->vterm_screen, 1);
	vterm_screen_reset(state->vterm_screen, 1);

	state->vterm_state = vterm_obtain_state(state->vterm);
	if (state->vterm_state == NULL) {
		fprintf(stderr, "Failed to obtain vterm state\n");
		return 0;
	}

	vterm_screen_set_callbacks(state->vterm_screen, &callbacks, state);
	vterm_output_set_callback(state->vterm, vterm_output_callback, &state->child_fd);

	state->child_pty = forkpty(&state->child_fd, NULL, NULL, NULL);
	if (state->child_pty < 0) {
		fprintf(stderr, "Failed to forkpty\n");
		return 0;
	}

	char *exec_argv[64] = {config->shell};
	for (int i = 0; i < config->argc; i++) {
		exec_argv[i + 1] = config->argv[i];
	}

	// If running in child
	if (state->child_pty == 0) {
		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);
		execv(config->shell, exec_argv);
	} else {
		// Block the sigaction for SIGCHLD
		// Because we terminate it in the end
		struct sigaction sa;
		sa.sa_handler = sig_handler;
		sa.sa_flags = 0;

		sigemptyset(&sa.sa_mask);
		sigaction(SIGCHLD, &sa, NULL);
		child_state = 1;
	}

	state->states.focused = true;
	state->states.active = true;
	state->glyph_cache = cache_ht_create();
	return 1;
}
