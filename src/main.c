#include "term.h"

int main(void) {
	term_t state;
	char *argv[] = {"bash", "-l", "-i", NULL};

	config_t config = {
		.width = 800,
		.height = 600,
		.font = "iosevka.ttf",
		.font_size = 15,
		.shell = "/run/current-system/sw/bin/bash",
		.argv = argv,
		.cols = 80,
		.rows = 24,
		.dpi = 1,
	};

	if (!init_term(&state, &config)) {
		fprintf(stderr, "Failed to initialize terminal\n");
		return -1;
	}

	// Yay we are doing multithreading now to make stuff fast
	// Note: Needs to be called after init_term because of context
	state.threads.active = true;
	pthread_mutex_init(&state.threads.mutex, NULL);
	pthread_create(&state.threads.pty_thread, NULL, pty_read_thread, &state);
	pthread_create(&state.threads.draw_thread, NULL, draw_thread, &state);

	int draw_width, draw_height;
	glfwGetFramebufferSize(state.glfw_window, &draw_width, &draw_height);
	resize_term(&state, draw_width, draw_height);

	while (!glfwWindowShouldClose(state.glfw_window)) {
		glfwWaitEvents();
	}

	destroy_term(&state);
	return 0;
}
