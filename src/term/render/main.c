#include "term.h"

// Called inside of the thread as a bootstrap
int init_gl_context(term_t *state) {
	glfwMakeContextCurrent(state->glfw_window);
	glfwSwapInterval(0);

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (get_gl_error()) {
		return 0;
	}

	glGenVertexArrays(1, &state->gl_state.vao);
	glGenBuffers(1, &state->gl_state.vbo);

	if (get_gl_error()) {
		return 0;
	}

	// Bind VAO and VBO
	glBindVertexArray(state->gl_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->gl_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cell_vertex_t) * 4, NULL, GL_DYNAMIC_DRAW);

	if (get_gl_error()) {
		return 0;
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cell_vertex_t), 0);

	if (get_gl_error()) {
		return 0;
	}

	// Unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	glDeleteProgram(state->gl_state.program);

	glDisable(GL_BLEND);
	get_gl_error();
}
