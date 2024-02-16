#include "term.h"

void render_term(term_t *state) {
	glClearColor(0, 0, 0, state->config->opacity);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// if (state->states.resize) {
	// 	int draw_width, draw_height;
	// 	glfwGetFramebufferSize(state->glfw_window, &draw_width, &draw_height);
	// 	resize_term(state, draw_width, draw_height);
	// 	state->states.resize = false;
	// }

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

	glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	glBindVertexArray(state->gl_state.vao);

	coord_t cursor = {
		.x = state->cursor.x * font_width,
		.y = state->cursor.y * font_height,
		.width = font_width,
		.height = font_height,
	};

	cell_vertex_t cursor_vertices[4] = {
		{cursor.x, cursor.y, 0.0f, 1.0f},
		{cursor.x, cursor.y + font_height, 0.0f, 0.0f},
		{cursor.x + font_width, cursor.y, 1.0f, 1.0f},
		{cursor.x + font_width, cursor.y + font_height, 1.0f, 0.0f},
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(cursor_vertices), cursor_vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
