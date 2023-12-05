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

	// Yay we are doing multithreading now to make stuff fast
	// Note: Needs to be called after init_term because of context
	state.threads.active = true;
	pthread_create(&state.threads.pty_thread, NULL, pty_read_thread, &state);
	pthread_create(&state.threads.draw_thread, NULL, draw_thread, &state);

	int draw_width, draw_height;
	glfwGetFramebufferSize(state.glfw_window, &draw_width, &draw_height);
	resize_term(&state, draw_width, draw_height);

	while (!glfwWindowShouldClose(state.glfw_window)) {
		glfwWaitEvents();
	}

	destroy_term(&state);
	// TODO: Segfaulting in the end on quit, need to fix
	destroy_config(&config);
	return 0;
}
