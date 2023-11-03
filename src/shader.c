#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

GLchar const *read_file(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open %s\n", filename);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buffer = malloc(size + 1);
	fread(buffer, size, 1, fp);
	fclose(fp);

	buffer[size] = '\0';
	return buffer;
}

GLuint load_shaders(const char *vert, const char *frag) {
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	GLchar const *vertex_shader_code = read_file(vert);
	GLchar const *fragment_shader_code = read_file(frag);

	GLint result = GL_FALSE;
	int info_log_length;

	fprintf(stdout, "Compiling shader: %s\n", vert);
	glShaderSource(vertex_shader_id, 1, &vertex_shader_code, NULL);
	glCompileShader(vertex_shader_id);

	fprintf(stdout, "Checking shader: %s\n", vert);
	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		char *vertex_shader_error_message = malloc(info_log_length + 1);
		glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, vertex_shader_error_message);
		fprintf(stderr, "%s\n", vertex_shader_error_message);
		free(vertex_shader_error_message);
	}

	fprintf(stdout, "Compiling shader: %s\n", frag);
	glShaderSource(fragment_shader_id, 1, &fragment_shader_code, NULL);
	glCompileShader(fragment_shader_id);

	fprintf(stdout, "Checking shader: %s\n", frag);
	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		char *fragment_shader_error_message = malloc(info_log_length + 1);
		glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, fragment_shader_error_message);
		fprintf(stderr, "%s\n", fragment_shader_error_message);
		free(fragment_shader_error_message);
	}

	fprintf(stdout, "Linking shaders\n");
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	// Check for linker errors when attaching the shaders
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		char *program_error_message = malloc(info_log_length + 1);
		glGetProgramInfoLog(program_id, info_log_length, NULL, program_error_message);
		fprintf(stderr, "%s\n", program_error_message);
		free(program_error_message);
	}

	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
	return program_id;
}
