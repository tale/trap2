#include "log.h"
#include <GL/glew.h>
#include <ft2build.h>
#include <stdlib.h>
#include FT_FREETYPE_H

// Standardized raster type allows us to create abstractions.
// The different OSes will all rasterize to this standard struct
typedef struct {
	uint32_t char_code;
	int32_t width;
	int32_t height;
	int32_t top;
	int32_t left;
	int32_t advance;
	uint8_t *bitmap;
} rast_glyph_t;

typedef struct {
	GLuint id;
	int32_t width;		  // Width of the texture
	int32_t height;		  // Height of the texture
	int32_t row_cursor;	  // Next pixel on the current row
	int32_t row_baseline; // Baseline of the current row
	int32_t row_height;	  // Height of the current row
} text_atlas_t;

typedef enum {
	ATLAS_OK,
	ATLAS_FULL,
	ATLAS_ERROR,
	ATLAS_GLYPH_TOO_LARGE
} atlas_status_t;

typedef struct {
	GLfloat bottom;
	GLfloat left;
	GLfloat height;
	GLfloat width;
} uv_t;

text_atlas_t *atlas_create(int32_t size);
void atlas_destroy(text_atlas_t *atlas);
atlas_status_t atlas_add_glyph(text_atlas_t *atlas, rast_glyph_t *glyph, uv_t *uv);

rast_glyph_t rast_glyph_ft(FT_Face face, FT_UInt char_code);
