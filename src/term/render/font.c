#include "mac.h"
#include "term.h"

FT_UInt32 get_char_code_point(int values[]) {
	return (FT_UInt32)*values;
}

void load_cache(font_t *font, FT_UInt32 char_code) {
	if (char_code > MAX_CODE_POINT) {
		log_error("Charcode: %d is out of range", char_code);
		return;
	}

	// Avoid loading the same glyph twice
	if (font->cache[char_code].texture != 0) {
		return;
	}

	FT_Bitmap bitmap;
	int status = get_bitmap(char_code, &bitmap, font);
	if (status) {
		log_error("Charcode: %d failed to load bitmap", char_code);
		return;
	}

	// Generate a texture for the glyph
	GLuint texture_handle;
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Load the bitmap into the texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		font->face->glyph->bitmap.width,
		font->face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		font->face->glyph->bitmap.buffer);

	glGenerateMipmap(GL_TEXTURE_2D);

	if (get_gl_error()) {
		log_error("Charcode: %d failed to load glTexImage2D", char_code);
		return;
	}

	// Add the texture handle to the font cache
	glyph_t glyph = {
		.texture = texture_handle,
		.metrics = font->face->glyph->metrics,
		.advance = font->face->glyph->advance,
	};

	font->cache[char_code] = glyph;
}

int bust_glyph_cache(font_t *font) {
	for (int i = 0; i < MAX_CODE_POINT; i++) {
		if (font->cache[i].texture != 0) {
			glDeleteTextures(1, &font->cache[i].texture);
			font->cache[i].texture = 0;
		}
	}

	return 0;
}

// Returns 0 on failure so that we can do if (!init_font()) {}
int init_font(font_t *font, char *font_file, float size) {
	log_info("Loading font: %s", font_file);
	FT_Error error;

	error = FT_Init_FreeType(&font->library);
	if (error) {
		fprintf(stderr, "FT_Init_FreeType Error: %s\n", FT_Error_String(error));
		return 0;
	}

	error = FT_New_Face(font->library, font_file, 0, &font->face);
	if (error) {
		fprintf(stderr, "FT_New_Face Error: %s\n", FT_Error_String(error));
		return 0;
	}

	// Pixel to 26.6 fixed point is just size * 64
	int size_26_6 = size * 64;

	// Assume 96 DPI if we can't get the actual DPI
	float hor_dpi = 96, ver_dpi = 96;

#ifdef __APPLE__
	getScreenDPI(&hor_dpi, &ver_dpi);
#endif

	error = FT_Set_Char_Size(font->face, size_26_6, size_26_6, hor_dpi, ver_dpi);
	if (error) {
		fprintf(stderr, "FT_Set_Char_Size Error: %s\n", FT_Error_String(error));
		return 0;
	}

	// Allocate font->glyphs by the number of glyphs in the font
	int num_glyphs = font->face->num_glyphs;
	font->cache = malloc(sizeof(glyph_t) * num_glyphs);
	if (font->cache == NULL) {
		fprintf(stderr, "Could not allocate font->glyphs\n");
		return 0;
	}

	// Load the first 128 glyphs (ASCII)
	for (int i = 0; i < 128; i++) {
		load_cache(font, i);
	}

	return 1;
}

int get_bitmap(FT_UInt32 char_code, FT_Bitmap *bitmap, font_t *font) {
	FT_Error error;

	FT_UInt glyph_index = FT_Get_Char_Index(font->face, char_code);

	error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_RENDER);
	if (error) {
		fprintf(stderr, "FT_Load_Glyph Error: %s\n", FT_Error_String(error));
		return 1;
	}

	error = FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
		fprintf(stderr, "FT_Render_Glyph Error: %s\n", FT_Error_String(error));
		return 1;
	}

	*bitmap = font->face->glyph->bitmap;
	return 0;
}

int render_glyph(font_t *font, FT_UInt32 char_code, coord_t *coord, color_t *color, float opacity, GLuint vbo) {
	// This has a harness to skip if already loaded
	load_cache(font, char_code);
	if (font->cache[char_code].texture == 0) {
		fprintf(stderr, "Could not load glyph for char_code: %d\n", char_code);
		return 1;
	}

	if (get_gl_error()) {
		log_error("OpenGL Cache Load Error: %d\n", char_code);
		return 1;
	}

	GLuint texture_handle = font->cache[char_code].texture;
	FT_Glyph_Metrics metrics = font->cache[char_code].metrics;
	FT_Vector advance = font->cache[char_code].advance;

	coord->width = advance.x >> 6;
	coord->height = advance.y >> 6;

	// All the operations reference a diagram on FreeType's website
	// https://freetype.org/freetype2/docs/glyphs/glyph-metrics-3.svg
	int glyph_height = metrics.height >> 6;
	int ascent = font->face->size->metrics.ascender >> 6;	// Distance from baseline to top
	int descent = font->face->size->metrics.descender >> 6; // Distance from baseline to bottom (negative)
	int max_height = ascent - descent;

	// These are just starting values for points
	// The locations are what logically comes to mind if you imagined a cell
	// Now we are going to take the FreeType hints to get the actual locations
	// Note: coord->width is the advance value
	point_t top_left = {coord->x, coord->y};
	point_t bottom_left = {coord->x, coord->y + max_height};

	point_t top_right = {coord->x + coord->width, coord->y};
	point_t bottom_right = {coord->x + coord->width, coord->y + max_height};

	// Step 1. Float the start over by the value of the X bearing
	int horizontal_bearing = metrics.horiBearingX >> 6;
	top_left.x += horizontal_bearing;
	bottom_left.x += horizontal_bearing;

	// Step 2. Align the character to the baseline
	// The amount of height above the baseline is the Y bearing
	int vertical_bearing = metrics.horiBearingY >> 6;
	int offset_from_top = ascent - vertical_bearing;

	top_left.y += offset_from_top;
	top_right.y += offset_from_top;

	// Step 3. Float up the characters without a descender
	// The amount of height below the baseline is height - Y bearing
	int below_baseline = glyph_height - vertical_bearing;

	if (below_baseline == 0) {
		bottom_left.y += descent;
		bottom_right.y += descent;
	} else {
		bottom_left.y += descent;
		bottom_left.y += below_baseline;

		bottom_right.y += descent;
		bottom_right.y += below_baseline;
	}

	// Step 4. Calculate the right side bearing in order to re-offset characters back
	// The right side bearing is the advance value - the width - the left side bearing
	int right_side_bearing = (metrics.horiAdvance >> 6) - (metrics.width >> 6) - horizontal_bearing;
	top_right.x -= right_side_bearing;
	bottom_right.x -= right_side_bearing;

	cell_vertex_t vertices[4] = {
		{bottom_left.x, bottom_left.y, 0.0f, 1.0f},
		{top_left.x, top_left.y, 0.0f, 0.0f},
		{bottom_right.x, bottom_right.y, 1.0f, 1.0f},
		{top_right.x, top_right.y, 1.0f, 0.0f},
	};

	if (get_gl_error()) {
		log_error("OpenGL Vertex Error: %d\n", char_code);
		return 1;
	}

	if (vbo == 0) {
		log_error("OpenGL VBO Error: %d\n", char_code);
		return 1;
	}

	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cell_vertex_t) * 4, vertices);

	if (get_gl_error()) {
		log_error("OpenGL Buffer Sub Data Error: %d\n", char_code);
		return 1;
	}

	return 0;
}
