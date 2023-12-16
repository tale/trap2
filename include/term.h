#ifndef TERM_H
#define TERM_H

#include <GL/glew.h>

#include "config.h"
#include "font.h"
#include "log.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vterm.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h> // Needed to lock CG context
#include <util.h>		   // forkpty
#else
#include <pty.h>
#endif

typedef struct {
	// This essentially handles all state
	// It's very important for synchronization
	struct {
		bool focused;
		bool redraw;
		bool resize;
		bool active;
		bool reprop;
		pthread_mutex_t lock;
		pthread_cond_t cond;
	} states;

	config_t *config;

	VTerm *vterm;
	char *title;
	VTermScreen *vterm_screen;
	VTermState *vterm_state;
	GLFWwindow *glfw_window;

	struct {
		int x;
		int y;
		bool active;
	} cursor;

	struct {
		pthread_t draw_thread;
		pthread_t pty_thread;
	} threads;

	font_t font;

	pid_t child_pty;
	int child_fd;

	struct {
		GLuint program;
		GLuint vao;
		GLuint vbo;
	} gl_state;
} term_t;

// Global to work with signal handler
static int child_state;

int init_term(term_t *state, config_t *config);
void destroy_term(term_t *state);
void render_term(term_t *state);
void resize_term(term_t *state, int width, int height);
void *pty_read_thread(void *argp);
void *draw_thread(void *argp);
void handle_key(term_t *state, int key, int mods);

// Rendering functions
int get_gl_error(void);
int init_gl_context(term_t *state);
void destroy_gl_context(term_t *state);
void update_projection(term_t *state);
void render_cell(term_t *state, int x, int y, int *offset);

#endif
