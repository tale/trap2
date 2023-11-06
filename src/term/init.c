#include "term.h"

void glfw_error_callback(int error, const char *description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void sig_handler(int sig_type) {
	child_state = 0;
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

int init_term(term_t *state, config_t *config) {
	if (config == NULL) {
		fprintf(stderr, "Config is NULL\n");
		return 0;
	}

	state->config = config;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to initialize SDL\n");
		fprintf(stderr, "%s\n", SDL_GetError());
		return 0;
	}

	state->window = SDL_CreateWindow(
		"trap2 Terminal",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		config->width,
		config->height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	// Set our GL attributes before getting the context
	// TODO: Switch to OpenGL 4 and use shaders instead of fixed pipeline
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_DisplayMode mode;
	if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {
		fprintf(stderr, "Failed to get display mode\n");
		fprintf(stderr, "%s\n", SDL_GetError());

		SDL_DestroyWindow(state->window);
		SDL_Quit();
		return 0;
	}

	SDL_PixelFormat *format = SDL_AllocFormat(mode.format);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, format->BitsPerPixel);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, format->BitsPerPixel);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, format->BitsPerPixel);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	SDL_GL_SetSwapInterval(1);

	if (state->window == NULL) {
		fprintf(stderr, "Failed to create SDL window\n");
		fprintf(stderr, "%s\n", SDL_GetError());

		SDL_Quit();
		return 0;
	}

	if (SDL_GL_CreateContext(state->window) == NULL) {
		fprintf(stderr, "Failed to create OpenGL context\n");
		fprintf(stderr, "%s\n", SDL_GetError());

		SDL_DestroyWindow(state->window);
		SDL_Quit();
		return 0;
	}

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		fprintf(stderr, "%s\n", glewGetErrorString(err));
		return 0;
	}

	state->key_state = SDL_GetKeyboardState(NULL);
	state->ticks = SDL_GetTicks();
	SDL_StartTextInput();

	// Make the OpenGL coordinate system orthagnoal from top-left
	// This is identical to the non-accelerated SDL renderer coordinates
	int draw_width, draw_height, win_width, win_height;
	SDL_GL_GetDrawableSize(state->window, &draw_width, &draw_height);
	SDL_GetWindowSize(state->window, &win_width, &win_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	state->config->dpi = draw_width / win_width;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int font_size = 15 / state->config->dpi;
	if (!init_font(&state->font, state->config->font, font_size)) {
		fprintf(stderr, "Failed to initialize font\n");
		return 0;
	}

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

	// If running in child
	if (state->child_pty == 0) {
		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);
		execv(config->shell, config->argv);
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

	state->dirty = true;
	return 1;
}
