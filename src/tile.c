#include <stdbool.h>
#include <stdint.h>



#include "game.h"
#include "tile.h"

/* Get the type of a tile (zeroing the offset bits) */
uint8_t get_type(uint8_t x, uint8_t y, uint8_t z) {
	/* Check if position is valid */
	if(x >= TILES_X) return NO_TILE;
	if(y >= TILES_Y) return NO_TILE;
	if(z >= TILES_Z) return NO_TILE;

	return game.tiles[x][y][z] & bmTileType;
}

/* Check if a tile is shifted up */
bool is_offset_up(uint8_t x, uint8_t y, uint8_t z) {
	/* Check if position is valid */
	if(x > TILES_X) return false;
	if(y > TILES_Y) return false;
	if(z > TILES_Z) return false;

	if(!get_type(x, y, z)) return false;

	return game.tiles[x][y][z] & bmTopOffset ? true : false;
}

/* Check if a tile is shifted right */
bool is_offset_right(uint8_t x, uint8_t y, uint8_t z) {
	/* Check if position is valid */
	if(x > TILES_X) return false;
	if(y > TILES_Y) return false;
	if(z > TILES_Z) return false;

	if(!get_type(x, y, z)) return false;

	return game.tiles[x][y][z] & bmRightOffset ? true : false;
}
