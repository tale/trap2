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

	int draw_width, draw_height;
	glfwGetFramebufferSize(state.glfw_window, &draw_width, &draw_height);
	resize_term(&state, draw_width, draw_height);

	while (!glfwWindowShouldClose(state.glfw_window)) {
		glfwWaitEvents();
	}

	destroy_term(&state);
	return 0;
}
