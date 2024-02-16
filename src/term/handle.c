#include "term.h"

// These only fire on keydown because they are important for escape sequences
void handle_key(term_t *state, int key, int mods) {
	switch (key) {
	case GLFW_KEY_ENTER:
	case GLFW_KEY_KP_ENTER:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_ENTER, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_BACKSPACE:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_ESCAPE:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_TAB:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_TAB, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_UP:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_UP, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_DOWN:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_DOWN, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_LEFT:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_LEFT, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_RIGHT:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_RIGHT, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_PAGE_UP:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_PAGE_DOWN:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_HOME:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_HOME, VTERM_MOD_NONE);
		break;
	case GLFW_KEY_END:
		vterm_keyboard_key(state->parser->vt, VTERM_KEY_END, VTERM_MOD_NONE);
		break;
	default:
		if (mods & GLFW_MOD_CONTROL && key < 127) {
			// GLFW does uppercase letters here by default
			if (key >= 'A' && key <= 'Z') {
				key += 32;
			}

			vterm_keyboard_unichar(state->parser->vt, key, VTERM_MOD_CTRL);
		}

		break;
	}
}
