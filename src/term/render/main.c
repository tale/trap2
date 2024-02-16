#include "term.h"

// Called inside of the thread as a bootstrap
int init_gl_context(term_t *state) {
	glfwMakeContextCurrent(state->glfw_window);
	glfwSwapInterval(1);

	log_info("OpenGL Version: %s", glGetString(GL_VERSION));
	log_info("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	log_info("Creating OpenGL context");

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		log_error("Failed to initialize GLEW");
		log_error("GLEW_ERRNO - %s", glewGetErrorString(err));
		return 0;
	}

	// TODO: Embed shaders into the binary instead of runtime loading
	GLuint program = attach_shaders("shaders/cell_vert.glsl", "shaders/cell_frag.glsl");
	if (!program) {
		log_error("Failed to attach cell shaders");
		return 0;
	}

	if (get_gl_error()) {
		log_error("Got a GL error on the cell shaders");
		return 0;
	}

	state->gl_state.program = program;
	glUseProgram(state->gl_state.program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR);
	glDepthMask(GL_FALSE);

	if (get_gl_error()) {
		return 0;
	}

	glGenVertexArrays(1, &state->gl_state.vao);
	glGenBuffers(1, &state->gl_state.vbo);
	glGenBuffers(1, &state->gl_state.ebo);

	if (get_gl_error()) {
		return 0;
	}

	// Bind VAO and VBO
	glBindVertexArray(state->gl_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cell_t) * MAX_BATCH, NULL, GL_STREAM_DRAW);

	int indices[6] = {0, 1, 3, 1, 2, 3};

	// Setup the EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->gl_state.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	if (get_gl_error()) {
		return 0;
	}

	size_t size = sizeof(cell_t);

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, size, (void *)offsetof(cell_t, col));
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);

	glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_FALSE, size, (void *)offsetof(cell_t, left));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, size, (void *)offsetof(cell_t, uv_left));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, size, (void *)offsetof(cell_t, r));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, size, (void *)offsetof(cell_t, bg_r));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	// glEnableVertexAttribArray(0);
	// glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cell_vertex_t), 0);

	if (get_gl_error()) {
		return 0;
	}

	// Unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (get_gl_error()) {
		return 0;
	}

	update_projection(state);
	if (get_gl_error()) {
		return 0;
	}

	return 1;
}

void destroy_gl_context(term_t *state) {
	log_info("Destroying OpenGL context");
	glDeleteVertexArrays(1, &state->gl_state.vao);
	glDeleteBuffers(1, &state->gl_state.vbo);
	glDeleteBuffers(1, &state->gl_state.ebo);
	glDeleteProgram(state->gl_state.program);

	glDisable(GL_BLEND);
	get_gl_error();
}
