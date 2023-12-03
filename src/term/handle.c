#include "term.h"

void *pty_read_thread(void *argp) {
	term_t *state = (term_t *)argp;

	while (state->threads.active) {
		// Update the terminal state machine for vterm
		// Allows us to keep our state machine in sync
		fd_set readfds;
		struct timeval timeout = {0};

		FD_ZERO(&readfds);
		FD_SET(state->child_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;

		if (select(state->child_fd + 1, &readfds, NULL, NULL, &timeout) > 0) {
			// Page size is the most efficient buffer size in this case
			// It doesn't make sense to read more than a page at a time
			size_t page_size = getpagesize();
			char line[page_size];
			int n;

			if ((n = read(state->child_fd, line, sizeof(line))) > 0) {
				vterm_input_write(state->vterm, line, n);
				state->dirty = true;
			}
		}
	}

	return NULL;
}

// These only fire on keydown because they are important for escape sequences
void handle_key(term_t *state, int key, int mods) {
	int vterm_modifier = VTERM_MOD_NONE;

	if (mods & GLFW_MOD_CONTROL) {
		vterm_modifier |= VTERM_MOD_CTRL;
	}

	// Alt is equivalent to Meta on macOS
	if (mods & GLFW_MOD_ALT) {
		vterm_modifier |= VTERM_MOD_ALT;
	}

	if (mods & GLFW_MOD_SHIFT) {
		vterm_modifier |= VTERM_MOD_SHIFT;
	}

	if (mods & GLFW_MOD_SUPER) {
		// TODO: VTerm doesn't support super key
		// Makes sense since XTerm doesn't either
	}

	switch (key) {
	case GLFW_KEY_ENTER:
	case GLFW_KEY_KP_ENTER:
		vterm_keyboard_key(state->vterm, VTERM_KEY_ENTER, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_BACKSPACE:
		vterm_keyboard_key(state->vterm, VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_ESCAPE:
		vterm_keyboard_key(state->vterm, VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_TAB:
		vterm_keyboard_key(state->vterm, VTERM_KEY_TAB, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_UP:
		vterm_keyboard_key(state->vterm, VTERM_KEY_UP, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_DOWN:
		vterm_keyboard_key(state->vterm, VTERM_KEY_DOWN, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_LEFT:
		vterm_keyboard_key(state->vterm, VTERM_KEY_LEFT, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_RIGHT:
		vterm_keyboard_key(state->vterm, VTERM_KEY_RIGHT, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_PAGE_UP:
		vterm_keyboard_key(state->vterm, VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_PAGE_DOWN:
		vterm_keyboard_key(state->vterm, VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_HOME:
		vterm_keyboard_key(state->vterm, VTERM_KEY_HOME, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_END:
		vterm_keyboard_key(state->vterm, VTERM_KEY_END, VTERM_MOD_NONE);
		break;
	default:
		if (mods & GLFW_MOD_CONTROL && key < 127) {
			// GLFW does uppercase letters here by default
			if (key >= 'A' && key <= 'Z') {
				key += 32;
			}

			vterm_keyboard_unichar(state->vterm, key, VTERM_MOD_CTRL);
		}

		break;
	}
}
