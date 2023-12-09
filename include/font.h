// This header will define the font structure and font table.
// THe goal is we have a few functions that we can call to get the entire character glyph rendered and ready to call with glDrawPixels.
#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>

#define MAX_CODE_POINT 0x10FFFF

// The index for a glyph is its character code point.
// This is a hashmap like structure implementation
typedef struct {
	GLuint texture;
	FT_Glyph_Metrics metrics;
	FT_Vector advance;
} glyph_t;

typedef struct {
	FT_Library library;
	FT_Face face;
	glyph_t *cache;
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

typedef struct {
	GLfloat x, y; // Position
	GLfloat s, t; // TexCoord
} cell_vertex_t;

int get_bitmap(FT_UInt32 char_code, FT_Bitmap *bitmap, font_t *font);
int render_glyph(font_t *font, FT_UInt32 char_code, coord_t *coord, color_t *color, float opacity, GLuint vbo);
