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

rast_glyph_t *rast_glyph_ft(FT_Face face, FT_UInt char_code);

typedef struct {
	uint32_t char_code;
	rast_glyph_t *glyph;
} cache_entry_t;

typedef struct {
	cache_entry_t **table;
	int32_t size;
	int32_t count;
} cache_ht_t;

#define CACHE_HT_SIZE 65536

cache_ht_t *cache_ht_create(void);
cache_entry_t *cache_entry_create(rast_glyph_t *glyph);

void cache_ht_destroy(cache_ht_t *ht);
void cache_entry_destroy(cache_entry_t *entry);

int cache_ht_insert(cache_ht_t *ht, rast_glyph_t *glyph);
rast_glyph_t *cache_ht_lookup(cache_ht_t *ht, uint32_t char_code);
rast_glyph_t *load_glyph(cache_ht_t *ht, FT_Face face, uint32_t char_code);

// TODO: Create a struct with a bunch of pointers to the correct functions
// On start, set the functions based on the rasterizer (CoreText, FreeType, etc)
