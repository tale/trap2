#include <text.h>

// In order to make atlases easy to work with, they're modeled after GL.
// We start packing from the bottom left and work our way to width and height.
//
// |----------------------------|
// |                            |
// |                 ^          |
// |-----|-----|     |          |
// |  1  |  2  |     |          |
// |     |     |   ----->       |
// |-----|-----|----------------|
//

text_atlas_t *atlas_create(int32_t size) {
	text_atlas_t *atlas = malloc(sizeof(text_atlas_t));
	if (!atlas) {
		return NULL;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		size,
		size,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	atlas->id = texture;
	atlas->width = size;
	atlas->height = size;
	atlas->row_cursor = 0;
	atlas->row_baseline = 0;
	atlas->row_height = 0;
	return atlas;
}

int check_pack_fit(text_atlas_t *atlas, rast_glyph_t *glyph) {
	int w_ok = (atlas->row_cursor + glyph->width) <= atlas->width;
	int h_ok = glyph->height < (atlas->height - atlas->row_baseline);
	return w_ok && h_ok;
}

// TODO: Deduplicate the atlas insert for a character????
atlas_status_t atlas_add_glyph(text_atlas_t *atlas, rast_glyph_t *glyph, uv_t *uv) {
	if (glyph->width > atlas->width || glyph->height > atlas->height) {
		return ATLAS_GLYPH_TOO_LARGE;
	}

	if (!check_pack_fit(atlas, glyph)) {
		int advance = atlas->row_height + atlas->row_baseline;
		if (atlas->height - advance <= 0) {
			return ATLAS_FULL;
		}

		atlas->row_cursor = 0;
		atlas->row_height = 0;
		atlas->row_baseline = advance;
	}

	if (!check_pack_fit(atlas, glyph)) {
		return ATLAS_FULL;
	}

	int offset_x = atlas->row_cursor;
	int offset_y = atlas->row_baseline;
	int height = glyph->height;
	int width = glyph->width;

	glBindTexture(GL_TEXTURE_2D, atlas->id);
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		offset_x,
		offset_y,
		width,
		height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		glyph->bitmap);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (glGetError() != GL_NO_ERROR) {
		return ATLAS_ERROR;
	}

	if (uv == NULL) {
		return ATLAS_ERROR;
	}

	atlas->row_cursor = offset_x + width;
	if (height > atlas->row_height) {
		atlas->row_height = height;
	}

	// Texture atlas coordinate mapping to the glyph
	uv->bottom = (float)offset_y / (float)atlas->height;
	uv->left = (float)offset_x / (float)atlas->width;
	uv->height = (float)height / (float)atlas->height;
	uv->width = (float)width / (float)atlas->width;

	return ATLAS_OK;
}

void atlas_destroy(text_atlas_t *atlas) {
	glDeleteTextures(1, &atlas->id);
	free(atlas);
}
