#include "parser.h"

void parser_destroy(parser_t *parser) {
	kill(parser->child_pid, SIGTERM);
	parser->child_alive = 0;

	vterm_free(parser->vt);
	free(parser->vt_callbacks);
	free(parser);
}
