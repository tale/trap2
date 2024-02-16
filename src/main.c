#include "term.h"

int main(void) {
	term_t state;
	config_t config;

	if (!init_config(&config)) {
		log_error("Failed to initialize config");
		return -1;
	}

	if (!load_config(&config)) {
		log_warn("Unable to load config, falling back to defaults");
	}

	if (!init_term(&state, &config)) {
		log_error("Failed to initialize term");
		return -1;
	}

	state.parser = parser_create(&config);
	if (state.parser == NULL) {
		log_error("Failed to initialize parser");
		return -1;
	}

	state.states.active = true;

	// Yay we are doing multithreading now to make stuff fast
	pthread_create(&state.threads.pty_thread, NULL, pty_read_thread, &state);
	pthread_detach(state.threads.pty_thread);

	if (!init_gl_context(&state)) {
		log_error("Failed to initialize OpenGL context");
		return 0;
	}

	if (get_gl_error()) {
		log_error("Failed to initialize OpenGL context 2");
		return 0;
	}

	int font_size = state.config->font_size * state.config->dpi;
	if (!init_font(&state.font, state.config->font, font_size)) {
		log_error("Failed to initialize font");
		return 0;
	}

	while (!glfwWindowShouldClose(state.glfw_window)) {
		if (state.states.reprop) {
			// glfwSetWindowTitle(state.glfw_window, state.title);
		}

		render_term(&state);
		glfwSwapBuffers(state.glfw_window);
		glfwWaitEvents();
	}

	destroy_term(&state);
	destroy_config(&config);
	destroy_gl_context(&state);
	return 0;
}
