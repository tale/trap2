#include "text.h"

// In order to prevent wasted rasterization, glyphs are cached for reuse.
// This is accomplished using a very basic hash table for quick lookups.

cache_ht_t *cache_ht_create(void) {
	cache_ht_t *ht = malloc(sizeof(cache_ht_t));
	if (!ht) {
		return NULL;
	}

	ht->size = CACHE_HT_SIZE;
	ht->count = 0;

	ht->table = calloc(ht->size, sizeof(cache_entry_t *));
	if (!ht->table) {
		free(ht);
		return NULL;
	}

	for (int i = 0; i < ht->size; i++) {
		ht->table[i] = NULL;
	}

	return ht;
}

cache_entry_t *cache_entry_create(rast_glyph_t *glyph) {
	cache_entry_t *entry = malloc(sizeof(cache_entry_t));
	if (!entry) {
		return NULL;
	}

	entry->char_code = glyph->char_code;
	entry->glyph = glyph;

	return entry;
}

void cache_ht_destroy(cache_ht_t *ht) {
	for (int i = 0; i < ht->size; i++) {
		cache_entry_t *entry = ht->table[i];
		if (entry != NULL) {
			free(entry->glyph);
			free(entry);
		}
	}

	free(ht->table);
	free(ht);
}

int cache_ht_insert(cache_ht_t *ht, rast_glyph_t *glyph) {
	cache_entry_t *entry = cache_entry_create(glyph);
	if (entry == NULL) {
		return 1;
	}

	int index = glyph->char_code % ht->size;
	while (ht->table[index] != NULL) {
		index = (index + 1) % ht->size;
	}

	ht->table[index] = entry;
	ht->count++;

	return 0;
}

rast_glyph_t *cache_ht_lookup(cache_ht_t *ht, uint32_t char_code) {
	int index = char_code % ht->size;
	while (ht->table[index] != NULL) {

		if (ht->table[index]->char_code == char_code) {
			return ht->table[index]->glyph;
		}

		index = (index + 1) % ht->size;
	}

	return NULL;
}

rast_glyph_t *load_glyph(cache_ht_t *ht, FT_Face face, uint32_t char_code) {
	rast_glyph_t *glyph = cache_ht_lookup(ht, char_code);
	if (glyph != NULL) {
		return glyph;
	}

	glyph = rast_glyph_ft(face, char_code);
	if (glyph == NULL) {
		return NULL;
	}

	cache_ht_insert(ht, glyph);
	return glyph;
}
