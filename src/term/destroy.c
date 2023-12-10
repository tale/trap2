#include "term.h"

void destroy_term(term_t *state) {
	state->states.active = false;
	kill(state->child_pty, SIGTERM);
	vterm_free(state->vterm);

	FT_Done_Face(state->font.face);
	FT_Done_FreeType(state->font.library);

	// pthread_join(state->threads.pty_thread, NULL);
	// pthread_join(state->threads.draw_thread, NULL);

	glfwDestroyWindow(state->glfw_window);
	glfwTerminate();
}
