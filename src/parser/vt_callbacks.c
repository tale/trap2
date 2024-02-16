#include "parser.h"

int vt_damage(VTermRect area, void *user) {
	glfwPostEmptyEvent();
	return 1;
}

int vt_movecursor(VTermPos dest, VTermPos src, int visible, void *user) {
	parser_t *parser = (parser_t *)user;
	parser->cursor_x = dest.col;
	parser->cursor_y = dest.row;
	parser->cursor_a = visible;
	return 1;
}

int vt_setprop(VTermProp prop, VTermValue *val, void *user) {
	// TODO: Implement
	switch (prop) {
	case VTERM_PROP_ALTSCREEN:
		break;

	case VTERM_PROP_CURSORVISIBLE:
		break;

	case VTERM_PROP_TITLE:
		break;

	case VTERM_PROP_ICONNAME:
		break;

	case VTERM_PROP_REVERSE:
		break;

	case VTERM_PROP_CURSORSHAPE:
		break;

	case VTERM_PROP_MOUSE:
		break;

	case VTERM_PROP_CURSORBLINK:
		break;

	default:
		return 0;
	}

	return 1;
}

int vt_bell(void *user) {
	glfwRequestWindowAttention(window);
	return 1;
}

void vt_output(const char *bytes, size_t len, void *user) {
	int fd = *(int *)user;
	write(fd, bytes, len);
}
