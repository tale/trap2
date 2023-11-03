#include "term.h"

void destroy_term(term_t *state) {
	kill(state->child_pty, SIGTERM);
	vterm_free(state->vterm);

	FT_Done_Face(state->font.face);
	FT_Done_FreeType(state->font.library);

	SDL_DestroyWindow(state->window);
	SDL_Quit();
}
