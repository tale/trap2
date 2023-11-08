#include "term.h"

int main(void) {
	term_t state;
	char *argv[] = {"zsh", "-l", NULL};

	config_t config = {
		.width = 800,
		.height = 600,
		.font = "iosevka.ttf",
		.font_size = 15,
		.shell = "/etc/profiles/per-user/tale/bin/zsh",
		.argv = argv,
		.cols = 80,
		.rows = 24,
		.dpi = 1,
	};

	if (!init_term(&state, &config)) {
		fprintf(stderr, "Failed to initialize terminal\n");
		return -1;
	}

	while (!handle_term(&state)) {
		render_term(&state);
		SDL_GL_SwapWindow(state.window);
	}

	destroy_term(&state);
	return 0;
}
