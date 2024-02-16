#include "term.h"

void render_term(term_t *state) {
	glUseProgram(state->gl_state.program);
	glBindVertexArray(state->gl_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->gl_state.ebo);
	glActiveTexture(GL_TEXTURE0);

	glClearColor(0, 0, 0, state->config->opacity);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// VTermRect rect = state->gl_state.scissor;
	int x_advance = state->font.face->size->metrics.max_advance >> 6;
	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;
	int linespace = (ascent - descent);

	// int x = rect.start_col * x_advance;
	// int y = state->config->height - (rect.start_row * linespace);
	//
	// int width = (rect.end_col) * x_advance;
	// int height = (rect.end_row) * linespace;

	// glEnable(GL_SCISSOR_TEST);
	// glScissor(x, y - height, width, height);
	// glDisable(GL_SCISSOR_TEST);

	// This is the 2nd part of the scissor optimization
	// On each render pass, we reset scissor back to the damage size
	// state->gl_state.scissor = state->gl_state.damage;

	// Worst case scenario is a different thing in the entire screen
	// HOWEVER there are lots of blank characters so this array is not filled up
	cell_t *cells = malloc(sizeof(cell_t) * state->config->rows * state->config->cols);
	int idx = 0;

	state->gl_state.atlas = atlas_create(MAX_ATLAS);
	if (state->gl_state.atlas == NULL) {
		log_error("Failed to create atlas");
		return;
	}

	// Set the cell_size vec2 uniform
	GLuint cell_size_uniform = glGetUniformLocation(state->gl_state.program, "cell_size");
	glUniform2f(cell_size_uniform, x_advance, linespace);

	for (int y = 0; y < state->config->rows; y++) {
		for (int x = 0; x < state->config->cols; x++) {
			// render_cell(state, x, y, &x_offset);

			uv_t uv;

			VTermScreenCell vt_cell;
			VTermPos pos = {.row = y, .col = x};

			vterm_screen_get_cell(state->parser->vt_screen, pos, &vt_cell);
			uint32_t char_code = vt_cell.chars[0];

			if (char_code == 0) {
				continue;
			}

			rast_glyph_t *rast = load_glyph(
				state->glyph_cache,
				state->font.face,
				char_code);

			atlas_add_glyph(state->gl_state.atlas, rast, &uv);

			vterm_state_convert_color_to_rgb(state->parser->vt_state, &vt_cell.fg);
			vterm_state_convert_color_to_rgb(state->parser->vt_state, &vt_cell.bg);

			uint8_t fg_red = vt_cell.fg.rgb.red;
			uint8_t fg_green = vt_cell.fg.rgb.green;
			uint8_t fg_blue = vt_cell.fg.rgb.blue;

			uint8_t bg_red = vt_cell.bg.rgb.red;
			uint8_t bg_green = vt_cell.bg.rgb.green;
			uint8_t bg_blue = vt_cell.bg.rgb.blue;

			if (vt_cell.attrs.reverse) {
				fg_red = ~fg_red;
				fg_green = ~fg_green;
				fg_blue = ~fg_blue;

				bg_red = ~bg_red;
				bg_green = ~bg_green;
				bg_blue = ~bg_blue;
			}

			if (vt_cell.attrs.bold) {
				// TODO: Bold + Other attributes
			}

			cell_t cell = {
				.col = x,
				.row = y,

				.top = rast->top,
				.left = rast->left,

				.width = rast->width,
				.height = rast->height,

				.uv_bottom = uv.bottom,
				.uv_left = uv.left,

				.uv_width = uv.width,
				.uv_height = uv.height,

				.r = fg_red,
				.g = fg_green,
				.b = fg_blue,
				.a = 255,

				.bg_r = bg_red,
				.bg_g = bg_green,
				.bg_b = bg_blue,
				.bg_a = char_code != ' ' ? 255 : 0,
			};

			cells[idx++] = cell;
		}
	}

	// glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	// glBindVertexArray(state->gl_state.vao);

	// coord_t cursor = {
	// 	.x = state->cursor.x * x_advance,
	// 	.y = state->cursor.y * linespace,
	// 	.width = x_advance,
	// 	.height = linespace,
	// };
	//
	// cell_vertex_t cursor_vertices[4] = {
	// 	{cursor.x, cursor.y, 0.0f, 1.0f},
	// 	{cursor.x, cursor.y + linespace, 0.0f, 0.0f},
	// 	{cursor.x + x_advance, cursor.y, 1.0f, 1.0f},
	// 	{cursor.x + x_advance, cursor.y + linespace, 1.0f, 0.0f},
	// };

	// glBufferData(GL_ARRAY_BUFFER, sizeof(cursor_vertices), cursor_vertices, GL_DYNAMIC_DRAW);
	// glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, state->gl_state.atlas->id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cell_t) * state->config->rows * state->config->cols, cells);

	glUniform1i(glGetUniformLocation(state->gl_state.program, "pass"), 1);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, state->config->rows * state->config->cols);

	glUniform1i(glGetUniformLocation(state->gl_state.program, "pass"), 2);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, state->config->rows * state->config->cols);

	atlas_destroy(state->gl_state.atlas);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	free(cells);
}
