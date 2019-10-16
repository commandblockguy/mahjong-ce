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
	IN_PROGRESS,	/* Game is in progress */
	WIN,			/* No tiles left */
	LOSE,			/* No moves left */
	EXIT,			/* Game should save and return to the main menu */
	EXIT_NO_SAVE	/* Game should return to the main menu without saving */
};

typedef struct {
	/* Uses the enum game_status but is only 1 byte */
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
	/* Number of tiles left */
	uint8_t remaining_tiles;
	/* The last calculated value of the number of possible moves */
	uint8_t possible_moves;
	bool magnifier_shown;
	/* The name of the current pack */
	char pack_name[9];
} game_t;

/* Game global defined in main.c */
extern game_t game;

#endif