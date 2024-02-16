#include "text.h"

rast_glyph_t *rast_glyph_ft(FT_Face face, FT_UInt char_code) {
	FT_Error error;

	FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

	if (error) {
		log_error("FT_Load_Glyph Error: %s", FT_Error_String(error));
		exit(1);
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LCD);
	if (error) {
		log_error("FT_Render_Glyph Error: %s", FT_Error_String(error));
		exit(1);
	}

	FT_GlyphSlot slot = face->glyph;
	FT_Bitmap bitmap = slot->bitmap;

	rast_glyph_t *glyph = malloc(sizeof(rast_glyph_t));
	glyph->char_code = char_code;
	glyph->width = bitmap.width;
	glyph->height = bitmap.rows;
	glyph->top = slot->bitmap_top;
	glyph->left = slot->bitmap_left;
	glyph->advance = slot->advance.x >> 6;
	glyph->bitmap = malloc(bitmap.width * bitmap.rows * 4);

	// Normalize the bitmap to RGBA
	int pitch = bitmap.pitch;
	for (int i = 0; i < bitmap.rows; i++) {
		int start = i * pitch;
		int stop = start + bitmap.width;
		for (int j = start; j < stop; j++) {
			glyph->bitmap[j * 4] = bitmap.buffer[j];
			glyph->bitmap[j * 4 + 1] = bitmap.buffer[j];
			glyph->bitmap[j * 4 + 2] = bitmap.buffer[j];
		}
	}

	return glyph;
}
