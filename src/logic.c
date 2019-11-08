#include <stdbool.h>
#include <stdint.h>
#include <tice.h>



#include "game.h"
#include "gfx.h"
#include "logic.h"
#include "tile.h"
#include "util.h"

/* Check if the tile at (x,y,z) can be removed */
bool is_removable(uint8_t x, uint8_t y, uint8_t z) {

	/* Fail if the tile has already been removed */
	if(!get_type(x, y, z)) return false;

	if(top_blocked(x, y, z)) return false;

	if(left_blocked(x, y, z) && right_blocked(x, y, z)) return false;

	return true;
}

/* Check if the right side of a tile is blocked */
bool right_blocked(uint8_t x, uint8_t y, uint8_t z) {
	/* Handle differently depending on whether or not this tile is offset */
	switch(game.tiles[x][y][z] & (bmRightOffset | bmTopOffset)) {
		default: /* Neither right or up offset */
			if(get_type(x + 1, y, z) && !is_offset_right(x + 1, y, z)) return true;
			if(get_type(x + 1, y + 1, z) && is_offset_up(x + 1, y + 1, z) && !is_offset_right(x + 1, y + 1, z)) return true;
			break;
		case bmRightOffset:
			if(get_type(x + 1, y, z)) return true;
			if(get_type(x + 1, y + 1, z) && is_offset_up(x + 1, y + 1, z)) return true;
			break;
		case bmTopOffset:
			if(get_type(x + 1, y - 1, z) && !is_offset_up(x + 1, y - 1, z) && !is_offset_right(x + 1, y - 1, z)) return true;
			if(get_type(x + 1, y, z) && !is_offset_right(x + 1, y, z)) return true;
			break;
		case bmRightOffset | bmTopOffset:
			if(get_type(x + 1, y - 1, z) && !is_offset_up(x + 1, y - 1, z)) return true;
			if(get_type(x + 1, y, z)) return true;
			break;
	}
	return false;
}

/* Check if the left side of a tile is blocked */
bool left_blocked(uint8_t x, uint8_t y, uint8_t z) {
	/* Handle differently depending on whether or not this tile is offset */
	switch(game.tiles[x][y][z] & (bmRightOffset | bmTopOffset)) {
		default: /* Neither right or up offset */
			if(get_type(x - 1, y, z)) return true;
			if(get_type(x - 1, y + 1, z) && is_offset_up(x - 1, y + 1, z)) return true;
			break;
		case bmRightOffset:
			if(get_type(x - 1, y, z) && is_offset_right(x - 1, y, z)) return true;
			if(get_type(x - 1, y + 1, z) && is_offset_up(x - 1, y + 1, z) && is_offset_right(x - 1, y, z)) return true;
			break;
		case bmTopOffset:
			if(get_type(x - 1, y, z)) return true;
			if(get_type(x - 1, y - 1, z) && !is_offset_up(x - 1, y - 1, z)) return true;
			break;
		case bmRightOffset | bmTopOffset:
			if(get_type(x - 1, y, z) && is_offset_right(x - 1, y, z)) return true;
			if(get_type(x - 1, y - 1, z) && is_offset_right(x - 1, y - 1, z) && !is_offset_up(x - 1, y - 1, z)) return true;
			break;
	}
	return false;
}

/* Check if a tile is blocked from above */
bool top_blocked(uint8_t x, uint8_t y, uint8_t z) {

	if(get_type(x, y, z + 1)) return true;
	
	/* Handle differently depending on whether or not this tile is offset */
	switch(game.tiles[x][y][z] & (bmRightOffset | bmTopOffset)) {
		default: /* Neither right or up offset */
			if(get_type(x - 1, y, z + 1) && is_offset_right(x - 1, y, z + 1)) return true;
			if(get_type(x, y + 1, z + 1) && is_offset_up(x, y + 1, z + 1)) return true;
			if(get_type(x - 1, y + 1, z + 1) && is_offset_right(x - 1, y + 1, z + 1) && is_offset_up(x - 1, y + 1, z + 1)) return true;
			break;
		case bmRightOffset:
			if(get_type(x + 1, y, z + 1) && !is_offset_right(x + 1, y, z + 1)) return true;
			if(get_type(x, y + 1, z + 1) && is_offset_up(x, y + 1, z + 1)) return true;
			if(get_type(x + 1, y + 1, z + 1) && is_offset_up(x + 1, y + 1, z + 1) && !is_offset_right(x + 1, y + 1, z + 1)) return true;
			break;
		case bmTopOffset:
			if(get_type(x - 1, y, z + 1) && is_offset_right(x - 1, y, z + 1)) return true;
			if(get_type(x - 1, y - 1, z + 1) && is_offset_right(x - 1, y - 1, z + 1) && !is_offset_up(x - 1, y - 1, z + 1)) return true;
			if(get_type(x, y - 1, z + 1) && !is_offset_up(x, y - 1, z + 1)) return true;
			break;
		case bmRightOffset | bmTopOffset:
			if(get_type(x, y - 1, z + 1) && !is_offset_up(x, y - 1, z + 1)) return true;
			if(get_type(x + 1, y, z + 1) && !is_offset_right(x + 1, y, z + 1)) return true;
			if(get_type(x + 1, y - 1, z + 1) && !is_offset_up(x + 1, y - 1, z + 1) && !is_offset_right(x + 1, y - 1, z + 1)) return true;
			break;
	}
	return false;
}

/* Returns true if tiles were removed successfully */
bool remove_tiles(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2) {
	undo_t *undo;
	/* Check if tiles are the same */
	if(get_type(x1, y1, z1) != get_type(x2, y2, z2)) return false;

	/* Check if both locations are the same */
	if(x1 == x2 && y1 == y2 && z1 == z2) return false;

	/* Check if both tiles can be removed */
	if(!is_removable(x1, y1, z1)) return false;
	if(!is_removable(x2, y2, z2)) return false;

	/* Make the move undo-able */
	undo = &game.undo_stack[game.undos];

	undo->pos1.x = x1;
	undo->pos1.y = y1;
	undo->pos1.z = z1;

	undo->pos2.x = x2;
	undo->pos2.y = y2;
	undo->pos2.z = z2;

	undo->tile1 = game.tiles[x1][y1][z1];
	undo->tile2 = game.tiles[x2][y2][z2];

	game.undos++;
	game.redos = 0;

	/* Remove the tiles */
	game.tiles[x1][y1][z1] = NO_TILE;
	game.tiles[x2][y2][z2] = NO_TILE;

	game.remaining_tiles -= 2;

	/* Recalculate the number of moves */
	calc_num_moves();

	if(!game.remaining_tiles) game.status = WIN;

	return true;
}

/* Calculate the number of possible moves */
uint8_t calc_num_moves(void) {
	uint8_t num_removable[TILE_TYPES] = {0};
	uint8_t x, y, z, i, moves = 0;

	memset(game.tile_count, 0, sizeof(game.tile_count));

	/* Count how many removable tiles of each type there are */
	for(z = 0; z < TILES_Z; z++) {
		for(y = 0; y < TILES_Y; y++) {
			for(x = 0; x < TILES_X; x++) {
				tile_t type = get_type(x, y, z);
				/* Don't count non tiles */
				/* Fun fact, the absence of this line caused a crash for almost a year */
				if(!type) continue;
				game.tile_count[type - 1]++;
				if(is_removable(x, y, z))
					num_removable[type - 1]++;
			}
		}
	}

	/* Add the number of possible moves within the set of removable tiles of one type to the total */
	for(i = 0; i < TILE_TYPES; i++) {
		switch(num_removable[i]) {
			case 2:
				moves +=1;
				break;
			case 3:
				moves += 3;
				break;
			case 4:
				moves += 6;
				break;
		    default: ;
		}
	}

	//If there are no more moves, ya lose!
	if(!moves) game.status = LOSE;

	return game.possible_moves = moves;
}

/* Get the position of the tile under the cursor, if any */
pos_t find_highlight(uint24_t cursor_x, uint8_t cursor_y) {
	pos_t ret = {-1, -1, -1};
	int z_level;
	for(z_level = TILES_Z - 1; z_level >= 0; z_level--) {
		/* Start from the top and work our way down */

		uint8_t x_tile, y_tile;

		int x_i, y_i;

		/* Figure out which tile the pointer would be over */
		x_tile = (cursor_x - (game.layout.start_x + (uint24_t)z_level * SHIFT_X)) / TILE_WIDTH;
		y_tile = (cursor_y - (game.layout.start_y - (uint24_t)z_level * SHIFT_Y)) / TILE_HEIGHT;

		if(x_tile > TILES_X || y_tile > TILES_Y) continue;

		/* Loop through the four tiles which could possibly be at the cursor's location */

		for(x_i = x_tile - 1; x_i <= x_tile; x_i++) {
			for(y_i = y_tile; y_i <= y_tile + 1; y_i++) {
				uint24_t base_x;
				uint8_t  base_y;

				/* Check if tile exists */

				if(!get_type(x_i, y_i, z_level)) continue;

				base_x = tile_base_x(x_i, y_i, z_level);
				base_y = tile_base_y(x_i, y_i, z_level);

				/* Check if the cursor overlaps this tile */

				if(cursor_x < base_x) continue;
				if(cursor_x > base_x + TILE_WIDTH) continue;
				if(cursor_y < base_y) continue;
				if(cursor_y > base_y + TILE_HEIGHT) continue;

				ret.x = x_i;
				ret.y = y_i;
				ret.z = z_level;

				return ret;
			}
		}
	}

	/* Reset the highlight */
	return ret;
}

// new algo:
// pick value based on system time
// fill all locations with blank tiles
// call recursive function with depth = 0
// recursive(depth):
//  if depth = 2*TILE_TYPES, return
//  count # of removable tiles
//  if 0, return false
//  for attempt in 0..removable*(removable-1):
//   select two random removable tiles, such that no pair is repeated from a previous iteration
//   add the locations to the result
//   recursive(depth + 1)
//   if false, replace the two tiles and remove from result
//   if true, return true
//  return false

// would require all tiles to be scanned at least 2*TILE_TYPES times

/* Load a layout and randomize tiles */
void load_layout(layout_t *l, bool random) {
	tile_t i;

	/* Array to be filled with random tiles */
	tile_t picks[TILE_TYPES * 4];

	/* Copy the layout metadata */
	memcpy(&game.layout, l, sizeof(layout_t));

	if(random) {
		for(i = 0; i < TILE_TYPES; i++) {
			int j;
			for(j = 0; j < 4; j++) {
				picks[4 * i + j] = i + 1;
			}
		}

		shuffle((uint8_t*)picks, sizeof(picks));

	}

	/* Fill the tile array with zeros */
	memset(game.tiles, 0, sizeof(game.tiles));

	for(i = 0; i < TILE_TYPES * 4; i++) {
		slot_t slot = l->slots[i];

		uint8_t x = slot.x / 2;

		/* Division rounds down, but the offset is in the negative direction */
		/* Again, what was I thinking when I decided to use offsets? */

		uint8_t y = slot.y / 2 + slot.y % 2;

		/* Z value does not need to be divided */

		uint8_t z = slot.z;

		tile_t tile = TEMP_TILE;

		if(random) {

			tile = picks[i];

		} else {
			/* Generate a solution that is guaranteed to be possible */

			/* (Not yet implemented) */
		}

		/* Set the offsets if the coordinate value is odd */
		tile |= (slot.x % 2) << bRightOffset | (slot.y % 2) << bTopOffset;

		game.tiles[x][y][z] = tile;
	}

	/* Initialize some stuff. Why is this here? No clue. */
	game.remaining_tiles = TILE_TYPES * 4;
	game.status = IN_PROGRESS;
	game.undos = 0;
	game.redos = 0;
	game.magnifier_shown = false;

	memcpy(game.initial_tiles, game.tiles, sizeof(game.tiles));
}

/* I guess this can go here. */
/* Why is this even a function? */
void set_highlight(uint8_t x, uint8_t y, uint8_t z) {
	game.highlight.x = x;
	game.highlight.y = y;
	game.highlight.z = z;
}

void reset_timer(uint24_t ms) {
	/* Reset the timer */
	timer_Control = TIMER1_DISABLE;
	timer_1_Counter = ms * 33;
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
}

/* Undo a move, returning true if successful */
bool undo(void) {
	undo_t *data;

	if(!game.undos) return false;

	data = &game.undo_stack[game.undos - 1];

	/* Add the tiles back */
	game.tiles[data->pos1.x][data->pos1.y][data->pos1.z] = data->tile1;
	game.tiles[data->pos2.x][data->pos2.y][data->pos2.z] = data->tile2;

	game.remaining_tiles += 2;

	calc_num_moves();

	game.redos++;
	game.undos--;

	return true;
}

/* Redo a move, returning true if successful */
bool redo(void) {
	undo_t *data;

	if(!game.redos) return false;

	data = &game.undo_stack[game.undos];

	game.tiles[data->pos1.x][data->pos1.y][data->pos1.z] = NO_TILE;
	game.tiles[data->pos2.x][data->pos2.y][data->pos2.z] = NO_TILE;

	game.remaining_tiles -= 2;

	calc_num_moves();

	game.undos++;
	game.redos--;

	return true;
}
