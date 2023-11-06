#include <GL/glew.h>
#include <SDL.h>

#include "font.h"
#include <inttypes.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <util.h>
#include <vterm.h>

// TODO: Conform all types to inttypes.h
typedef struct {
	char *shell;
	char **argv;
	char *font;

	int width;
	int height;

	int rows;
	int cols;
	int dpi;
} config_t;

typedef struct {
	VTerm *vterm;
	VTermScreen *vterm_screen;
	VTermState *vterm_state;
	SDL_Window *window;

	struct {
		int x;
		int y;
		bool active;
	} cursor;

	uint32_t ticks;
	const uint8_t *key_state;

	bool bell_active;
	config_t *config;
	font_t font;
	bool dirty;

	pid_t child_pty;
	int child_fd;
} term_t;

// Global to work with signal handler
static int child_state;

int init_term(term_t *state, config_t *config);
void destroy_term(term_t *state);
void render_term(term_t *state);
void resize_term(term_t *state, int width, int height);
int handle_term(term_t *state);
