#include "term.h"

void destroy_term(term_t *state) {
	kill(state->child_pty, SIGTERM);
	vterm_free(state->vterm);

	FT_Done_Face(state->font.face);
	FT_Done_FreeType(state->font.library);

	state->threads.active = false;
	pthread_join(state->threads.pty_thread, NULL);
	pthread_join(state->threads.draw_thread, NULL);
	pthread_mutex_destroy(&state->threads.mutex);

	glfwDestroyWindow(state->glfw_window);
	glfwTerminate();
}
