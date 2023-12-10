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
	pthread_create(&state.threads.pty_thread, NULL, pty_read_thread, &state);
	pthread_create(&state.threads.draw_thread, NULL, draw_thread, &state);

	pthread_detach(state.threads.pty_thread);
	pthread_detach(state.threads.draw_thread);

	pthread_mutex_init(&state.states.lock, NULL);
	pthread_cond_init(&state.states.cond, NULL);

	while (!glfwWindowShouldClose(state.glfw_window)) {
		glfwWaitEvents();
	}

	destroy_term(&state);
	destroy_config(&config);
	return 0;
}
