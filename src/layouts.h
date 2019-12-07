#ifndef LAYOUTS_H
#define LAYOUTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TILE_TYPES 36
#define LEVEL_NAME_LENGTH 16

typedef struct {
	uint8_t x; /* For X and Y, divide by 2 to get the tile x value */
	uint8_t y; /* If odd, the tile will be offset */
	uint8_t z; /* Z requires no dividing */
} slot_t;

typedef struct {
	uint8_t version; //Latest: 1
	uint24_t start_x;
	uint8_t start_y;
	char name[LEVEL_NAME_LENGTH];
	slot_t slots[TILE_TYPES * 4];
} layout_t;

typedef struct {
	char ident[3]; //"MJ"
	uint8_t version; //Latest: 1
	uint24_t num_layouts;
} pack_meta_t;

/* A level pack is a pack_meta_t structure followed by num_layouts layouts */

#endif
