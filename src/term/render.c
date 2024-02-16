#include "term.h"

void render_term(term_t *state) {
	glClearColor(0, 0, 0, state->config->opacity);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	VTermRect rect = state->gl_state.scissor;
	int x_advance = state->font.face->size->metrics.max_advance >> 6;
	int ascent = state->font.face->size->metrics.ascender >> 6;
	int descent = state->font.face->size->metrics.descender >> 6;
	int linespace = (ascent - descent);

	int x = rect.start_col * x_advance;
	int y = state->config->height - (rect.start_row * linespace);

	int width = (rect.end_col) * x_advance;
	int height = (rect.end_row) * linespace;

	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y - height, width, height);
	glDisable(GL_SCISSOR_TEST);

	// This is the 2nd part of the scissor optimization
	// On each render pass, we reset scissor back to the damage size
	state->gl_state.scissor = state->gl_state.damage;

	int x_offset = 0;

	for (int y = 0; y < state->config->rows; y++) {
		for (int x = 0; x < state->config->cols; x++) {
			render_cell(state, x, y, &x_offset);
		}

		x_offset = 0;
	}

	glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	glBindVertexArray(state->gl_state.vao);

	coord_t cursor = {
		.x = state->cursor.x * x_advance,
		.y = state->cursor.y * linespace,
		.width = x_advance,
		.height = linespace,
	};

	cell_vertex_t cursor_vertices[4] = {
		{cursor.x, cursor.y, 0.0f, 1.0f},
		{cursor.x, cursor.y + linespace, 0.0f, 0.0f},
		{cursor.x + x_advance, cursor.y, 1.0f, 1.0f},
		{cursor.x + x_advance, cursor.y + linespace, 1.0f, 0.0f},
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(cursor_vertices), cursor_vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
