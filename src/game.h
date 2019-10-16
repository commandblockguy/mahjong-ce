#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "layouts.h"
#include "logic.h"
#include "tile.h"

enum game_status {
	IN_PROGRESS,
	WIN,
	LOSE,
	EXIT,
	EXIT_NO_SAVE
};

typedef struct {
	uint8_t status;
	/* x, y, z(layers) */
	tile_t tiles[TILES_X][TILES_Y][TILES_Z];
	/* Backup of the starting conditions */
	tile_t initial_tiles[TILES_X][TILES_Y][TILES_Z];
	/* Count of each tile type */
	tile_t tile_count[TILE_TYPES];
	/* Stack containing info about how to undo/redo */
	undo_t undo_stack[TILE_TYPES * 2];
	uint8_t undos;
	uint8_t redos;
	/* Tile which is highlighted */
	pos_t highlight;
	/* Pointer to the currently loaded layout */
	/* Is set in the load_layout function */
	layout_t layout;
	uint8_t remaining_tiles;
	uint8_t possible_moves;
	bool magnifier_shown;
	char pack_name[9];
} game_t;

extern game_t game;

#endif