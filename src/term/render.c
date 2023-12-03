#include "term.h"

void render_cell(term_t *state, int x, int y, int *offset) {
	VTermScreenCell cell;
	VTermPos pos = {.row = y, .col = x};

	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;
	int linespace = (ascent - descent);

	vterm_screen_get_cell(state->vterm_screen, pos, &cell);
	uint32_t chars = cell.chars[0];

	coord_t coord = {
		.x = *offset,
		.y = y * linespace,
	};

	// It's important to increase offset BEFORE skipping empty cells
	*offset += (state->font.face->size->metrics.max_advance >> 6);
	if (chars == 0) {
		return;
	}

	vterm_state_convert_color_to_rgb(state->vterm_state, &cell.fg);
	vterm_state_convert_color_to_rgb(state->vterm_state, &cell.bg);

	uint8_t fg_red = cell.fg.rgb.red;
	uint8_t fg_green = cell.fg.rgb.green;
	uint8_t fg_blue = cell.fg.rgb.blue;

	uint8_t bg_red = cell.bg.rgb.red;
	uint8_t bg_green = cell.bg.rgb.green;
	uint8_t bg_blue = cell.bg.rgb.blue;

	if (cell.attrs.reverse) {
		fg_red = ~fg_red;
		fg_green = ~fg_green;
		fg_blue = ~fg_blue;

		bg_red = ~bg_red;
		bg_green = ~bg_green;
		bg_blue = ~bg_blue;
	}

	if (cell.attrs.bold) {
		// TODO: Bold
	}

	// TODO: Other attributes
	FT_UInt32 char_code = (FT_UInt32)chars;

	color_t color = {
		.fg = {
			.r = fg_red,
			.g = fg_green,
			.b = fg_blue,
		},

		.bg = {
			.r = bg_red,
			.g = bg_green,
			.b = bg_blue,
		},
	};

	render_glyph(&state->font, char_code, &coord, &color, state->config->opacity);
}

void *draw_thread(void *argp) {
	term_t *state = (term_t *)argp;

	glfwMakeContextCurrent(state->glfw_window);
	glfwSwapInterval(1);

	log_info("OpenGL Version: %s", glGetString(GL_VERSION));
	log_info("OpenGL Renderer: %s", glGetString(GL_RENDERER));

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		fprintf(stderr, "%s\n", glewGetErrorString(err));
		return 0;
	}

	// Make the OpenGL coordinate system orthagonal from top-left
	int draw_width, draw_height, win_width, win_height;
	glfwGetFramebufferSize(state->glfw_window, &draw_width, &draw_height);
	glfwGetWindowSize(state->glfw_window, &win_width, &win_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	state->config->dpi = draw_width / win_width;

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int font_size = state->config->font_size / state->config->dpi;
	if (!init_font(&state->font, state->config->font, font_size)) {
		fprintf(stderr, "Failed to initialize font\n");
		return 0;
	}

	while (state->draw_thread.active) {
		CGLContextObj ctx = CGLGetCurrentContext();
		CGLLockContext(ctx);

		if (state->draw_thread.resize) {
			pthread_mutex_lock(&state->draw_thread.mutex);
			resize_term_thread(state);
			state->draw_thread.resize = false;
			pthread_mutex_unlock(&state->draw_thread.mutex);
		}

		if (state->window_active) {
			glfwMakeContextCurrent(state->glfw_window);
			render_term(state);
			glfwSwapBuffers(state->glfw_window);
		}

		CGLUnlockContext(ctx);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	return NULL;
}

void render_term(term_t *state) {
	glClearColor(0, 0, 0, state->config->opacity);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (state->dirty) {
		// glNewList(display_list, GL_COMPILE);
		// GLuint display_list = glGenLists(1);

		int x_offset = 0;
		for (int y = 0; y < state->config->rows; y++) {
			for (int x = 0; x < state->config->cols; x++) {
				render_cell(state, x, y, &x_offset);
			}

			x_offset = 0;
		}

		// Render cursor block with pure white bar skulley
		int font_width = state->font.face->size->metrics.max_advance >> 6;
		int ascent = state->font.face->size->metrics.ascender >> 6;
		int descent = state->font.face->size->metrics.descender >> 6;
		int font_height = ascent - descent;

		coord_t cursor = {
			.x = state->cursor.x * font_width,
			.y = state->cursor.y * font_height,
			.width = font_width,
			.height = font_height};

		glBegin(GL_QUADS);
		glColor3ub(255, 255, 255);

		glTexCoord2f(0, 0);
		glVertex2f(cursor.x, cursor.y);

		glTexCoord2f(1, 0);
		glVertex2f(cursor.x + cursor.width, cursor.y);

		glTexCoord2f(1, 1);
		glVertex2f(cursor.x + cursor.width, cursor.y + cursor.height);

		glTexCoord2f(0, 1);
		glVertex2f(cursor.x, cursor.y + cursor.height);

		glEnd();
		// glEndList();
		// glCallList(display_list);
	}
}
