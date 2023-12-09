#include "term.h"

void *draw_thread(void *argp) {
	term_t *state = (term_t *)argp;

	if (!init_gl_context(state)) {
		log_error("Failed to initialize OpenGL context");
		return 0;
	}

	if (get_gl_error()) {
		log_error("Failed to initialize OpenGL context 2");
		return 0;
	}

	int font_size = state->config->font_size / state->config->dpi;
	if (!init_font(&state->font, state->config->font, font_size)) {
		log_error("Failed to initialize font");
		return 0;
	}

	while (state->threads.active) {
		pthread_mutex_lock(&state->global_lock);

		while (!state->should_render) {
			pthread_cond_wait(&state->global_cond, &state->global_lock);
		}

		pthread_mutex_unlock(&state->global_lock);

		if (state->window_active) {
			state->should_render = false;
			CGLContextObj ctx = CGLGetCurrentContext();
			CGLLockContext(ctx);

			render_term(state);
			glfwSwapBuffers(state->glfw_window);
			CGLUnlockContext(ctx);
		}
	}

	destroy_gl_context(state);
	pthread_exit(NULL);
}

void render_term(term_t *state) {
	glClearColor(0, 0, 0, state->config->opacity);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (state->should_resize) {
		int draw_width, draw_height;
		glfwGetFramebufferSize(state->glfw_window, &draw_width, &draw_height);
		resize_term(state, draw_width, draw_height);
		state->should_resize = false;
	}

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
