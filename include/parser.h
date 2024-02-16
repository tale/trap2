#ifndef PARSER_H
#define PARSER_H
#define _POSIX_C_SOURCE 200112L

#include "config.h"
#include "log.h"
#include <GLFW/glfw3.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
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
	int child_alive;
	pid_t child_pid;
} parser_t;

// Needed for signal handling
GLFWwindow *window;

parser_t *parser_create(config_t *config);
void parser_destroy(parser_t *parser);
int vt_create(parser_t *parser, int rows, int cols);
int child_create(parser_t *parser, char *shell, int argc, char **argv);
void *child_read(void *user);

int vt_damage(VTermRect area, void *user);
int vt_movecursor(VTermPos dest, VTermPos src, int visible, void *user);
int vt_setprop(VTermProp prop, VTermValue *val, void *user);
int vt_bell(void *user);
void vt_output(const char *bytes, size_t len, void *user);
void sig_action(int sig_type);

#endif // PARSER_H
