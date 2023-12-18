#include "term.h"

void *pty_read_thread(void *argp) {
	term_t *state = (term_t *)argp;

	while (state->states.active) {
		// Update the terminal state machine for vterm
		// Allows us to keep our state machine in sync
		fd_set readfds;
		struct timeval timeout = {0};

		FD_ZERO(&readfds);
		FD_SET(state->child_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 1000;

		if (select(state->child_fd + 1, &readfds, NULL, NULL, &timeout) > 0) {
			// Page size is the most efficient buffer size in this case
			// It doesn't make sense to read more than a page at a time
			size_t page_size = sysconf(_SC_PAGESIZE);
			char line[page_size];

			size_t len = read(state->child_fd, line, sizeof(line));
			if (len < 0) {
				log_error("Failed to read from pty: %s", strerror(errno));
				break;
			}

			if (len == 0) {
				break;
			}

			vterm_input_write(state->vterm, line, len);
			pthread_mutex_lock(&state->states.lock);

			state->states.redraw = true;
			pthread_cond_signal(&state->states.cond);
			pthread_mutex_unlock(&state->states.lock);
		}
	}

	pthread_exit(NULL);
}

// These only fire on keydown because they are important for escape sequences
void handle_key(term_t *state, int key, int mods) {
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
