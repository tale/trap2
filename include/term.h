#ifndef TERM_H
#define TERM_H
#define _POSIX_C_SOURCE 200112L

#include <GL/glew.h>

#include "config.h"
#include "font.h"
#include "log.h"
#include "parser.h"
#include "shader.h"
#include "text.h"
#include <GLFW/glfw3.h>
#include <errno.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __APPLE__
#include <util.h> // forkpty
#else
#include <pty.h>
#endif

typedef struct {
	// This essentially handles all state
	// It's very important for synchronization
	struct {
		bool focused;
		bool active;
		bool reprop;
	} states;

	config_t *config;

	char *title;
	GLFWwindow *glfw_window;

	struct {
		pthread_t draw_thread;
		pthread_t pty_thread;
	} threads;

	font_t font;

	struct {
		GLuint program;
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		text_atlas_t *atlas;
	} gl_state;

	cache_ht_t *glyph_cache;
	parser_t *parser;
} term_t;

#define MAX_BATCH 65536
#define MAX_ATLAS 1024

typedef struct {
	uint16_t col;
	uint16_t row;

	// Glyph offset
	int16_t left;
	int16_t top;

	// Glyph size
	int16_t width;
	int16_t height;

	GLfloat uv_left;
	GLfloat uv_bottom;

	GLfloat uv_width;
	GLfloat uv_height;

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a; // UNUSED

	uint8_t bg_r;
	uint8_t bg_g;
	uint8_t bg_b;
	uint8_t bg_a; // UNUSED
} cell_t;

typedef struct {
	cell_t cells[MAX_BATCH];
	GLuint count;
} batch_t;

GLFWwindow *window;

int init_term(term_t *state, config_t *config);
void destroy_term(term_t *state);
void render_term(term_t *state);
void resize_term(term_t *state, int width, int height);
void *pty_read_thread(void *argp);
void handle_key(term_t *state, int key, int mods);

// Rendering functions
int get_gl_error(void);
int init_gl_context(term_t *state);
void destroy_gl_context(term_t *state);
void update_projection(term_t *state);
void render_cell(term_t *state, int x, int y, int *offset);

#endif
