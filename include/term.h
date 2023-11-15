#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "font.h"
#include "log.h"
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
	float font_size;

	int width;
	int height;

	int rows;
	int cols;
	int dpi;
	int opacity;
} config_t;

typedef struct {
	VTerm *vterm;
	VTermScreen *vterm_screen;
	VTermState *vterm_state;
	GLFWwindow *glfw_window;
	bool window_active;

	struct {
		int x;
		int y;
		bool active;
	} cursor;

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
void handle_key(term_t *state, int key, int mods);
