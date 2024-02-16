#include "mac.h"
#include "term.h"

void glfw_error_callback(int error, const char *description) {
	log_error("GLFW Error %d: %s", error, description);
}

void glfw_char_callback(GLFWwindow *window, unsigned int codepoint) {
	term_t *state = glfwGetWindowUserPointer(window);
	vterm_keyboard_unichar(state->parser->vt, codepoint, VTERM_MOD_NONE);
}

void glfw_resize_callback(GLFWwindow *window, int width, int height) {
	int draw_width, draw_height;
	glfwGetWindowSize(window, &draw_width, &draw_height);

	term_t *state = glfwGetWindowUserPointer(window);
	resize_term(state, draw_width, draw_height);
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

	state->states.focused = true;
	state->glyph_cache = cache_ht_create();
	return 1;
}
