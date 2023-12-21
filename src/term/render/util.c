#include "term.h"

int get_gl_error(void) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		log_error("OpenGL Error Code: %d", err);

		void *callstack[128];
		int backtrace_data = backtrace(callstack, 128);
		backtrace_symbols_fd(callstack, backtrace_data, STDERR_FILENO);
		return 1;
	}

	return 0;
}

void update_projection(term_t *state) {
	// Make the OpenGL coordinate system orthagonal from top-left
	int draw_width, draw_height, win_width, win_height;
	glfwGetFramebufferSize(state->glfw_window, &draw_width, &draw_height);
	glfwGetWindowSize(state->glfw_window, &win_width, &win_height);

	float left = 0.0f;
	float right = draw_width;

	float top = 0.0f;
	float bottom = draw_height;

	// These are the calculations for handling the scale for the 2D space
	// We use 2.0f, because the coordinates are from -1.0 to 1.0
	// That's a total of 2 space to work with that we have to scale
	// We divide by the width and height to get the scale factor
	// This lets us use coordinates from (0, 0) to (width, height)

	float *matrix = malloc(sizeof(float) * 16);
	matrix[0] = 2.0f / (right - left);
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 2.0f / (top - bottom);
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	// These are meant for 3D scaling, we will keep as 1x
	// We are rendering only 2D graphics anyways so we can ignore it
	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 1.0f;
	matrix[11] = 0.0f;

	// These calculations are used to map the actual positions
	// This is relative to the center, moving (0, 0) to the top left
	matrix[12] = -(right + left) / (right - left);
	matrix[13] = -(top + bottom) / (top - bottom);
	matrix[14] = 0.0f; // We are in a 2D space anyways
	matrix[15] = 1.0f; // Required for homogenous coordinates

	GLuint projection_uniform = glGetUniformLocation(state->gl_state.program, "projection");
	glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, matrix);

	state->config->dpi = draw_width / win_width;
	if (get_gl_error()) {
		log_error("Failed to update projection");
		return;
	}
}
