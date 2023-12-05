#include "config.h"
#include "log.h"

int init_config(config_t *config) {
	config->width = 800;
	config->height = 600;
	config->font_size = 15;
	config->cols = 80;
	config->rows = 24;
	config->dpi = 1;
	config->opacity = 1.0;

	config->font = strdup("");
	if (config->font == NULL) {
		log_error("Could not allocate config->font");
		return 0;
	}

	config->shell = strdup(DEFAULT_SHELL);
	if (config->shell == NULL) {
		log_error("Could not allocate config->shell");
		return 0;
	}

	config->argv = malloc(sizeof(char *) * 1);
	if (config->argv == NULL) {
		log_error("Could not allocate config->argv");
		return 0;
	}

	config->argv[0] = NULL;
	config->argc = 0;
	return 1;
}

void trim_whitespace(char *string) {
	if (string == NULL) {
		return;
	}

	// Left side trimming
	while (isspace(*string)) {
		string++;
	}

	// All spaces?
	if (*string == 0) {
		return;
	}

	// Right side trimming
	char *end = string + strlen(string) - 1;
	while (end > string && isspace(*end)) {
		end--;
	}

	// Write new null terminator
	*(end + 1) = 0;
}

void expand_tilde(char *string, char *home) {
	if (string == NULL || home == NULL) {
		return;
	}

	const char *tilde = strchr(string, '~');
	if (tilde == NULL) {
		return;
	}

	char *new_string = malloc(strlen(string) + strlen(home) + 1);
	if (new_string == NULL) {
		log_error("Could not allocate new_string");
		return;
	}

	strcpy(new_string, home);
	strcat(new_string, tilde + 1);
	strcpy(string, new_string);
	free(new_string);
}

int parse_config(config_t *config, FILE *file, char *home) {
	char line[512];

	while (fgets(line, sizeof(line), file) != NULL) {
		trim_whitespace(line);

		if (line[0] == '#') {
			continue;
		}

		// Trim trailing comments
		char *comment = strchr(line, '#');
		if (comment != NULL) {
			*comment = 0;
		}

		char *key = strtok(line, " ");
		char *value = strtok(NULL, " ");

		if (key == NULL || value == NULL) {
			log_warn("Invalid config line: %s", line);
			continue;
		}

		if (value[strlen(value) - 1] == '\n') {
			value[strlen(value) - 1] = 0;
		}

		// Remove the quotes from a value
		if (value[0] == '"' || value[0] == '\'') {
			if (value[strlen(value) - 1] != value[0]) {
				log_warn("Invalid config line: %s", line);
				continue;
			}

			value++;
			value[strlen(value) - 1] = 0;
		}

		if (strncmp(key, "ShellArgs", 9) == 0) {
			if (config->argv != NULL) {
				for (int i = 0; i < config->argc; i++) {
					if (config->argv[i] != NULL) {
						free(config->argv[i]);
					}
				}

				free(config->argv);
			}

			char *argv[64]; // 64 should be enough for anyone
			char *arg = strtok(value, " ");

			config->argc = 0;
			while (arg != NULL) {
				argv[config->argc] = strdup(arg);
				if (argv[config->argc] == NULL) {
					log_error("Could not allocate argv[%d]", config->argc);
					return 0;
				}

				config->argc++;
				arg = strtok(NULL, " ");
			}

			config->argv = malloc(sizeof(char *) * (config->argc + 1));
			for (int i = 0; i < config->argc; i++) {
				config->argv[i] = argv[i];
			}

			config->argv[config->argc] = NULL;
			continue;
		}

		if (strncmp(key, "Shell", 5) == 0) {
			expand_tilde(value, home);
			if (config->shell != NULL) {
				free(config->shell);
			}

			config->shell = strdup(value);
			if (config->shell == NULL) {
				log_error("Could not allocate config->shell");
				return 0;
			}

			continue;
		}

		if (strncmp(key, "FontSize", 8) == 0) {
			config->font_size = atoi(value);
			continue;
		}

		if (strncmp(key, "Font", 4) == 0) {
			expand_tilde(value, home);
			if (config->font != NULL) {
				free(config->font);
			}

			config->font = strdup(value);
			if (config->font == NULL) {
				log_error("Could not allocate config->font");
				return 0;
			}

			continue;
		}

		if (strncmp(key, "Width", 5) == 0) {
			config->width = atoi(value);
			continue;
		}

		if (strncmp(key, "Height", 6) == 0) {
			config->height = atoi(value);
			continue;
		}

		if (strncmp(key, "Opacity", 7) == 0) {
			config->opacity = atof(value);
			continue;
		}
	}

	return 1;
}

int load_config(config_t *config) {
	char *home = getenv("HOME");
	if (home == NULL) {
		log_warn("Could not get $HOME");
		return 0;
	}

	char *config_path = malloc(strlen(home) + strlen(CONFIG_PATH) + 1);
	if (config_path == NULL) {
		log_error("Could not allocate config_path");
		return 0;
	}

	strcpy(config_path, home);
	strcat(config_path, CONFIG_PATH);

	FILE *file = fopen(config_path, "r");
	if (file == NULL) {
		log_warn("Could not open config file");
		return 0;
	}

	if (!parse_config(config, file, home)) {
		log_warn("Could not parse config file");
		return 0;
	}

	return 1;
}

void destroy_config(config_t *config) {
	if (config->font != NULL) {
		free(config->font);
	}

	if (config->shell != NULL) {
		free(config->shell);
	}

	if (config->argv != NULL) {
		for (int i = 0; i < config->argc; i++) {
			if (config->argv[i] != NULL) {
				free(config->argv[i]);
			}
		}

		free(config->argv);
	}
}
