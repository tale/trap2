// This header will define the font structure and font table.
// THe goal is we have a few functions that we can call to get the entire character glyph rendered and ready to call with glDrawPixels.
#include <ft2build.h>
#include FT_FREETYPE_H
#include <OpenGL/gl.h>

typedef struct {
	FT_Library library;
	FT_Face face;
} font_t;

typedef struct {
	int x;
	int y;
} point_t;

typedef struct {
	int x;
	int y;
	int width;
	int height;
} coord_t;

typedef struct {
	struct {
		GLfloat r;
		GLfloat g;
		GLfloat b;
	} fg;

	struct {
		GLfloat r;
		GLfloat g;
		GLfloat b;
	} bg;
} color_t;

FT_UInt32 get_char_code_point(int values[]);
int init_font(font_t *font, char *font_file, float size);

int get_bitmap(FT_UInt32 char_code, FT_Bitmap *bitmap, font_t *font);
int render_glyph(font_t *font, FT_Bitmap *bitmap, coord_t *coord, color_t *color);
