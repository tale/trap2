#include "config.h"
#include "log.h"
#include <GLFW/glfw3.h>
#include <signal.h>
#include <unistd.h>
#include <vterm.h>

// For forkpty()
#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif

typedef struct {
	VTerm *vt;
	VTermScreen *vt_screen;
	VTermState *vt_state;
	VTermScreenCallbacks *vt_callbacks;

	int cursor_x;
	int cursor_y;
	int cursor_a;

	int child_fd;
	pid_t child_pid;
} parser_t;

typedef enum {
	CHILD_STATUS_RUNNING,
	CHILD_STATUS_EXITED,
	CHILD_STATUS_ERROR
} child_status_t;

// The sigaction can only work with globals
static child_status_t child_status;
GLFWwindow *window;

parser_t *parser_create(config_t *config);
void parser_destroy(parser_t *parser);
int vt_create(parser_t *parser, int rows, int cols);
int child_create(parser_t *parser, char *shell, int argc, char **argv);
void sig_action(int sig_type);

int vt_damage(VTermRect area, void *user);
int vt_movecursor(VTermPos dest, VTermPos src, int visible, void *user);
int vt_setprop(VTermProp prop, VTermValue *val, void *user);
int vt_bell(void *user);
void vt_output(const char *bytes, size_t len, void *user);
