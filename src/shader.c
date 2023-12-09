#include "shader.h"
#include "log.h"

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

int get_shader_status(GLuint shader_id) {
	GLint result = GL_FALSE;
	int info_log_length;

	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

	if (info_log_length > 0) {
		char *shader_error_message = malloc(info_log_length + 1);
		glGetShaderInfoLog(shader_id, info_log_length, NULL, shader_error_message);
		fprintf(stderr, "%s\n", shader_error_message);
		free(shader_error_message);
	}

	return result;
}

void attach_shaders(GLuint program, const char *vert, const char *frag) {
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLchar const *vertex_shader_code = read_file(vert);

	glShaderSource(vertex_shader_id, 1, &vertex_shader_code, NULL);
	glCompileShader(vertex_shader_id);

	if (!get_shader_status(vertex_shader_id)) {
		log_error("Failed to compile vertex shader");
		return;
	}

	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar const *fragment_shader_code = read_file(frag);

	glShaderSource(fragment_shader_id, 1, &fragment_shader_code, NULL);
	glCompileShader(fragment_shader_id);

	if (!get_shader_status(fragment_shader_id)) {
		log_error("Failed to compile fragment shader");
		return;
	}

	glAttachShader(program, vertex_shader_id);
	glAttachShader(program, fragment_shader_id);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		log_error("Failed to attach shaders: Code %d", err);
		return;
	}

	glLinkProgram(program);
	GLint result = GL_FALSE;
	int info_log_length;

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

	if (info_log_length > 0) {
		char *program_error_message = malloc(info_log_length + 1);
		glGetProgramInfoLog(program, info_log_length, NULL, program_error_message);
		log_error("%s\n", program_error_message);
		free(program_error_message);
	}

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
}
