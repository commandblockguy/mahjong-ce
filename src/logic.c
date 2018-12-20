#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include "game.h"

bool is_removable(uint8_t x, uint8_t y, uint8_t z) {

	/* Fail if the tile has already been removed */
	if(!get_type(x, y, z)) return false;

	if(top_blocked(x, y, z)) return false;

	if(left_blocked(x, y, z) && right_blocked(x, y, z)) return false;

	return true;
}

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

/* Returns true if successful */
bool remove_tiles(uint8_t x1, uint8_t y1,  uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2) {
	/* Check if tiles are the same */
	if(get_type(x1, y1, z1) != get_type(x2, y2, z2)) return false;

	/* Check if both locations are the same */
	if(x1 == x2 && y1 == y2 && z1 == z2) return false;

	/* Check if both tiles can be removed */
	if(!is_removable(x1, y1, z1)) return false;
	if(!is_removable(x2, y2, z2)) return false;

	/* Remove the tiles */
	game.tiles[x1][y1][z1] = NO_TILE;
	game.tiles[x2][y2][z2] = NO_TILE;

	game.remaining_tiles -= 2;

	if(!game.remaining_tiles) game.status = WIN;

	/* Recalculate the number of moves */
	calc_num_moves();

	return true;
}

/* Calcuate the number of possible moves */
uint8_t calc_num_moves() {
	uint8_t num_removable[TILE_TYPES * 4] = {0};
	uint8_t x, y, z, i, moves = 0;

	/* Count how many removable tiles of each type there are */
	for(z = 0; z < TILES_Z; z++) {
		for(y = 0; y < TILES_Y; y++) {
			for(x = 0; x < TILES_X; x++) {
				if(is_removable(x, y, z))
					num_removable[get_type(x, y, z) - 1]++;
			}
		}
	}

	/* Add the number of possible moves within the set of removable tiles of one type to the total */
	for(i = 0; i < TILE_TYPES * 4; i++) {
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
		}
	}

	//If there are no more moves, ya lose!
	if(!moves) game.status = LOSE;

	return game.possible_moves = moves;
}

void update_highlight(uint24_t cursor_x, uint8_t cursor_y) {
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
				uint24_t base_x = tile_base_x(x_i, y_i, z_level);
				uint8_t  base_y = tile_base_y(x_i, y_i, z_level);

				/* Check if tile exists */

				if(!get_type(x_i, y_i, z_level)) continue;

				/* Check if the cursor overlaps this tile */

				if(cursor_x < base_x) continue;
				if(cursor_x > base_x + TILE_WIDTH) continue;
				if(cursor_y < base_y) continue;
				if(cursor_y > base_y + TILE_HEIGHT) continue;

				set_highlight(x_i, y_i, z_level);
				return;
			}
		}
	}

	/* Reset the highlight */
	set_highlight(-1, -1, -1);
}

void load_layout(layout_t *l, bool random) {
	int i;

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

		shuffle(picks, TILE_TYPES * 4);

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

	memcpy(game.initial_tiles, game.tiles, sizeof(game.tiles));
}

/* I guess this can go here. */
void set_highlight(int8_t x, int8_t y, int8_t z) {
	game.hl_x = x;
	game.hl_y = y;
	game.hl_z = z;
}

void reset_timer(uint24_t ms) {
	/* Reset the timer */
	timer_Control = TIMER1_DISABLE;
	timer_1_Counter = ms * 33;
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
}
