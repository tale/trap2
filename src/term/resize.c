#include "term.h"

void resize_term(term_t *state, int width, int height) {
	// We need to recalculate our orthagonal coordinate mapping system
	// Additionally we need to take display DPI scaling into calculation

	int draw_width, draw_height, win_width, win_height;
	SDL_GL_GetDrawableSize(state->window, &draw_width, &draw_height);
	SDL_GetWindowSize(state->window, &win_width, &win_height);

	int dpi = draw_width / win_width;

	// Reset our coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, draw_width, draw_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	state->config->width = width;
	state->config->height = height;

	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;

	int mono_width = state->font.face->glyph->advance.x >> 6;
	int mono_height = ascent - descent;

	int rows = (width / mono_width) * dpi;
	int cols = (height / mono_height);

	if (cols != state->config->rows || rows != state->config->cols) {

		state->config->rows = cols;
		state->config->cols = rows;
		vterm_set_size(state->vterm, rows, cols);
		// vterm_screen_flush_damage(state->vterm_screen);

		struct winsize ws = {0};
		ws.ws_row = rows;
		ws.ws_col = cols;
		ioctl(state->child_fd, TIOCSWINSZ, &ws);
	}
}
