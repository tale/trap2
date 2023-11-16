#include "font.h"

FT_UInt32 get_char_code_point(int values[]) {
	return (FT_UInt32)*values;
}

void load_cache(font_t *font, FT_UInt32 char_code) {
	if (char_code > MAX_CODE_POINT) {
		fprintf(stderr, "Invalid char_code: %d\n", char_code);
		return;
	}

	// Avoid loading the same glyph twice
	if (font->cache[char_code].texture != 0) {
		return;
	}

	FT_Bitmap bitmap;
	int status = get_bitmap(char_code, &bitmap, font);
	if (status) {
		fprintf(stderr, "Could not get bitmap for char_code: %d\n", char_code);
		return;
	}

	// Generate a texture for the glyph
	GLuint texture_handle;
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Set the texture environment
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Load the bitmap into the texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_ALPHA,
		font->face->glyph->bitmap.width,
		font->face->glyph->bitmap.rows,
		0,
		GL_ALPHA,
		GL_UNSIGNED_BYTE,
		font->face->glyph->bitmap.buffer);

	// Add the texture handle to the font cache
	glyph_t glyph = {
		.texture = texture_handle,
		.metrics = font->face->glyph->metrics,
		.advance = font->face->glyph->advance,
	};

	font->cache[char_code] = glyph;
}

// Returns 0 on failure so that we can do if (!init_font()) {}
int init_font(font_t *font, char *font_file, float size) {
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

	// Load unnaturally large font in order to get the largest possible glyph
	error = FT_Set_Char_Size(font->face, 0, (int)(size * 64), 300, 300);
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

int render_glyph(font_t *font, FT_UInt32 char_code, coord_t *coord, color_t *color, float opacity) {
	// This has a harness to skip if already loaded
	load_cache(font, char_code);
	if (font->cache[char_code].texture == 0) {
		fprintf(stderr, "Could not load glyph for char_code: %d\n", char_code);
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

	// Draw the background
	glColor3ub(color->bg.r, color->bg.g, color->bg.b);
	glBegin(GL_QUADS);

	glVertex2i(coord->x, coord->y);
	glVertex2i(coord->x + coord->width, coord->y);
	glVertex2i(coord->x + coord->width, coord->y + max_height);
	glVertex2i(coord->x, coord->y + max_height);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glColor3ub(color->fg.r, color->fg.g, color->fg.b);

	glBegin(GL_QUADS);

	glTexCoord2d(0, 0); // Top-left of the texture
	glVertex2i(top_left.x, top_left.y);

	glTexCoord2d(1, 0); // Top-right of the texture
	glVertex2i(top_right.x, top_right.y);

	glTexCoord2d(1, 1); // Bottom-right of the texture
	glVertex2i(bottom_right.x, bottom_right.y);

	glTexCoord2d(0, 1); // Bottom-left of the texture
	glVertex2i(bottom_left.x, bottom_left.y);

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(0, 0, 0);
	return 0;
}
