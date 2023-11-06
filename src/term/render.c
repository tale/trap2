#include "term.h"

void render_cell(term_t *state, int x, int y, int *offset) {
	VTermScreenCell cell;
	VTermPos pos = {.row = y, .col = x};

	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;
	int linespace = (ascent - descent);

	vterm_screen_get_cell(state->vterm_screen, pos, &cell);
	uint32_t chars = cell.chars[0];
	if (chars == 0) {
		return;
	}

	coord_t coord = {
		.x = *offset,
		.y = y * linespace,
		.width = state->font.face->glyph->advance.x >> 6,
		.height = state->font.face->glyph->advance.y >> 6,
	};

	vterm_state_convert_color_to_rgb(state->vterm_state, &cell.fg);
	vterm_state_convert_color_to_rgb(state->vterm_state, &cell.bg);

	// fprintf(stdout, "%s\n", cell.chars);

	// TODO: Color support

	if (cell.attrs.reverse) {
		// SDL_Rect rect = {cursor.x, cursor.y + 4, state->font.max_width, state->font.max_height};
		// SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, color.a);
		// color.r = ~color.r;
		// color.g = ~color.g;
		// color.b = ~color.b;
		//
		// SDL_RenderFillRect(state->renderer, &rect);
	}

	if (cell.attrs.bold) {
		// TODO: Bold
		// color.r = color.r * 1.5;
		// color.g = color.g * 1.5;
		// color.b = color.b * 1.5;
	}

	// TODO: Other attributes

	FT_Bitmap bitmap;
	FT_UInt32 char_code = (FT_UInt32)chars;

	if (get_bitmap(char_code, &bitmap, &state->font)) {
		fprintf(stderr, "Could not get character bitmap\n");
		return;
	}

	*offset += (state->font.face->glyph->advance.x >> 6);
	render_glyph(&state->font, &bitmap, &coord);
}

void render_term(term_t *state) {
	if (state->dirty) {
		GLuint display_list = glGenLists(1);
		glNewList(display_list, GL_COMPILE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		// print the cursor.x and y
		// printf("%d %d\n", state->cursor.x, state->cursor.y);

		GLubyte color[4] = {255, 255, 255, 255};
		GLuint texture_handle;

		glGenTextures(1, &texture_handle);
		glBindTexture(GL_TEXTURE_2D, texture_handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(cursor.x, cursor.y);
		glTexCoord2f(1, 0);
		glVertex2f(cursor.x + cursor.width, cursor.y);
		glTexCoord2f(1, 1);
		glVertex2f(cursor.x + cursor.width, cursor.y + cursor.height);
		glTexCoord2f(0, 1);
		glVertex2f(cursor.x, cursor.y + cursor.height);
		glEnd();

		glEndList();
		glCallList(display_list);
	}
}
