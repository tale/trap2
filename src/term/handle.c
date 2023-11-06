#include "term.h"

int handle_term(term_t *state) {
	SDL_Event event;

	// Update the terminal state machine for vterm
	// Allows us to keep our state machine in sync
	fd_set readfds;
	struct timeval timeout = {0};

	FD_ZERO(&readfds);
	FD_SET(state->child_fd, &readfds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;

	if (select(state->child_fd + 1, &readfds, NULL, NULL, &timeout) > 0) {
		size_t page_size = getpagesize();
		char line[page_size];
		int n;

		if ((n = read(state->child_fd, line, sizeof(line))) > 0) {
			vterm_input_write(state->vterm, line, n);
			state->dirty = true;
		}
	}

	// Wait until the next keypress
	while (SDL_PollEvent(&event)) {

		switch (event.type) {
		case SDL_QUIT:
			fprintf(stderr, "SDL_QUIT\n");
			return 1;
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				resize_term(state, event.window.data1, event.window.data2);
				break;
			}

			break;

		case SDL_TEXTINPUT:
			state->key_state = SDL_GetKeyboardState(NULL);

			int vterm_modifier = VTERM_MOD_NONE;

			// Handle Left and Right Ctrl
			if (state->key_state[SDL_SCANCODE_LCTRL] || state->key_state[SDL_SCANCODE_RCTRL]) {
				vterm_modifier |= VTERM_MOD_CTRL;
			}

			// Handle Left and Right Alt (Counts as Meta key on macOS)
			if (state->key_state[SDL_SCANCODE_LALT] || state->key_state[SDL_SCANCODE_RALT]) {
				vterm_modifier |= VTERM_MOD_ALT;
			}

			// Handle Left and Right Shift
			if (state->key_state[SDL_SCANCODE_LSHIFT] || state->key_state[SDL_SCANCODE_RSHIFT]) {
				vterm_modifier |= VTERM_MOD_SHIFT;
			}

			for (int i = 0; i < strlen(event.text.text); i++) {
				vterm_keyboard_unichar(state->vterm, event.text.text[i], vterm_modifier);
			}

			break;

		case SDL_KEYDOWN:
			// This is an important list of escape sequence codes for Xterm
			// That's why they are handled separately along with firing differently
			switch (event.key.keysym.sym) {
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				vterm_keyboard_key(state->vterm, VTERM_KEY_ENTER, VTERM_MOD_NONE);
				break;
			case SDLK_BACKSPACE:
				vterm_keyboard_key(state->vterm, VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
				break;
			case SDLK_ESCAPE:
				vterm_keyboard_key(state->vterm, VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
				break;
			case SDLK_TAB:
				vterm_keyboard_key(state->vterm, VTERM_KEY_TAB, VTERM_MOD_NONE);
				break;
			case SDLK_UP:
				vterm_keyboard_key(state->vterm, VTERM_KEY_UP, VTERM_MOD_NONE);
				break;
			case SDLK_DOWN:
				vterm_keyboard_key(state->vterm, VTERM_KEY_DOWN, VTERM_MOD_NONE);
				break;
			case SDLK_LEFT:
				vterm_keyboard_key(state->vterm, VTERM_KEY_LEFT, VTERM_MOD_NONE);
				break;
			case SDLK_RIGHT:
				vterm_keyboard_key(state->vterm, VTERM_KEY_RIGHT, VTERM_MOD_NONE);
				break;
			case SDLK_PAGEUP:
				vterm_keyboard_key(state->vterm, VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
				break;
			case SDLK_PAGEDOWN:
				vterm_keyboard_key(state->vterm, VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
				break;
			case SDLK_HOME:
				vterm_keyboard_key(state->vterm, VTERM_KEY_HOME, VTERM_MOD_NONE);
				break;
			case SDLK_END:
				vterm_keyboard_key(state->vterm, VTERM_KEY_END, VTERM_MOD_NONE);
				break;
			default:
				// Control is a very weird case because we use multiple modifiers
				if (event.key.keysym.mod & KMOD_CTRL && event.key.keysym.sym < 127) {
					vterm_keyboard_unichar(state->vterm, event.key.keysym.sym, VTERM_MOD_CTRL);
				}

				break;
			}

			break;
		}
	}

	return 0;
}
