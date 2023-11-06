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

	FT_Bitmap bitmap;
	FT_UInt32 char_code = (FT_UInt32)chars;

	if (get_bitmap(char_code, &bitmap, &state->font)) {
		fprintf(stderr, "Could not get character bitmap\n");
		return;
	}

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

	*offset += (state->font.face->glyph->advance.x >> 6);
	render_glyph(&state->font, &bitmap, &coord, &color);
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
