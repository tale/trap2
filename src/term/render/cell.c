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

	GLuint color_uniform = glGetUniformLocation(state->gl_state.program, "text_color");
	GLuint bg_color_uniform = glGetUniformLocation(state->gl_state.program, "bg_color");

	glUniform4f(
		color_uniform,
		color.fg.r / 255.0f,
		color.fg.g / 255.0f,
		color.fg.b / 255.0f,
		state->config->opacity);

	glUniform4f(
		bg_color_uniform,
		color.bg.r / 255.0f,
		color.bg.g / 255.0f,
		color.bg.b / 255.0f,
		1.0f);

	glActiveTexture(GL_TEXTURE0);

	if (get_gl_error()) {
		return;
	}

	render_glyph(&state->font, char_code, &coord, &color, state->config->opacity, state->gl_state.vbo);

	glBindVertexArray(state->gl_state.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
