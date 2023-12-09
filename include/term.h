#include <GL/glew.h>

#include "config.h"
#include "font.h"
#include "log.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <util.h>
#include <vterm.h>

// TODO: Conform all types to inttypes.h

typedef struct {
	bool global_active;
	bool should_render;
	bool should_resize;

	pthread_mutex_t global_lock;
	pthread_cond_t global_cond;

	config_t *config;

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

	struct {
		pthread_t draw_thread;
		pthread_t pty_thread;
		bool active;
		bool resize;
	} threads;

	bool bell_active;
	font_t font;
	bool dirty;
	bool unhandled_key;

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
