#include "font.h"

FT_UInt32 get_char_code_point(int values[]) {
	return (FT_UInt32)*values;
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

	// Get the display DPI scale

	// Load unnaturally large font in order to get the largest possible glyph
	error = FT_Set_Char_Size(font->face, 0, (int)(size * 64), 300, 300);
	if (error) {
		fprintf(stderr, "FT_Set_Char_Size Error: %s\n", FT_Error_String(error));
		return 0;
	}

	return 1;
}

int get_bitmap(FT_UInt32 char_code, FT_Bitmap *bitmap, font_t *font) {
	FT_Error error;

	FT_UInt glyph_index = FT_Get_Char_Index(font->face, char_code);

	error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
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

int render_glyph(font_t *font, FT_Bitmap *bitmap, coord_t *coord, color_t *color) {
	// All the operations reference a diagram on FreeType's website
	// https://freetype.org/freetype2/docs/glyphs/glyph-metrics-3.svg
	int glyph_height = font->face->glyph->metrics.height >> 6;
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
	int horizontal_bearing = font->face->glyph->metrics.horiBearingX >> 6;
	top_left.x += horizontal_bearing;
	bottom_left.x += horizontal_bearing;

	// Step 2. Align the character to the baseline
	// The amount of height above the baseline is the Y bearing
	int vertical_bearing = font->face->glyph->metrics.horiBearingY >> 6;
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

	GLuint texture_handle;

	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		bitmap->width,
		bitmap->rows,
		0,
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE,
		bitmap->buffer);

	glEnable(GL_TEXTURE_2D);
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
	glDisable(GL_TEXTURE_2D);

	// TODO: Support background colors
	return 0;
}
