#include "term.h"

void resize_term(term_t *state, int width, int height) {
	state->config->width = width;
	state->config->height = height;
	update_projection(state);

	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;

	int mono_width = state->font.face->glyph->advance.x >> 6;
	int mono_height = ascent - descent;

	int cols = (state->config->width / mono_width);
	int rows = (state->config->height / mono_height);

	log_info("Resizing to: %dx%d", cols, rows);

	// TODO: Is there a way to stop infinite calls to resize and reduce it?
	if (cols != state->config->cols || rows != state->config->rows) {

		state->config->rows = rows;
		state->config->cols = cols;
		vterm_set_size(state->vterm, rows, cols);
		// vterm_screen_flush_damage(state->vterm_screen);

		struct winsize ws = {0};
		ws.ws_row = rows;
		ws.ws_col = cols;
		ioctl(state->child_fd, TIOCSWINSZ, &ws);
	}
}
