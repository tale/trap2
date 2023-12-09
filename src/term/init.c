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
	// printf("damage: [%d, %d, %d, %d]\n", rect.start_col,
	//			rect.start_row, rect.end_col, rect.end_row);
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
	return 1;
}

int bell(void *user) {
	term_t *state = (term_t *)user;
	state->bell_active = true;
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
	.bell = bell,
	.damage = damage};

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
	term_t *state = glfwGetWindowUserPointer(window);
	pthread_mutex_lock(&state->global_lock);

	state->should_resize = true;
	state->should_render = true;

	pthread_cond_signal(&state->global_cond);
	pthread_mutex_unlock(&state->global_lock);
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
	state->window_active = focused;
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
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (mode == NULL) {
		fprintf(stderr, "Failed to get video mode\n");
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GL_FRAMEBUFFER_SRGB, GLFW_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	// glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_TRUE);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

	// Higher refresh rates allow overbuffered rendering
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate * 2);

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

	state->window_active = true;
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

	state->should_render = true;
	state->should_resize = true;
	return 1;
}
