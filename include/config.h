#ifndef CONFIG_H
#define CONFIG_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SHELL "/bin/bash"
#define CONFIG_PATH "/.config/trap2/trap2.conf"

typedef struct {
	char *shell;
	char **argv;
	int argc;

	char *font;
	float font_size;

	int width;
	int height;

	int rows;
	int cols;
	int dpi;
	float opacity;
} config_t;

int init_config(config_t *config);
int load_config(config_t *config);
void destroy_config(config_t *config);

#endif
