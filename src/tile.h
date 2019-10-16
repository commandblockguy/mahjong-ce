#ifndef H_TILE
#define H_TILE

//Bit    7: Is offset to the right by a half-tile?
//Bit    6: Is offset to the top by a half-file?
//Bits 0-5: Tile type
//Tile type 0 = no tile
typedef uint8_t tile_t;

#define bRightOffset 7
#define bmRightOffset (1 << bRightOffset)
#define bTopOffset 6
#define bmTopOffset (1 << bTopOffset)
#define bmTileType 63 /* 0b00111111 */

#define TILE_TYPES 36
#define NO_TILE 0
/* Temporary tile used while generating maps */
#define TEMP_TILE TILE_TYPES

/* Size of the tile array */
#define TILES_X 17
#define TILES_Y 10
#define TILES_Z 8

uint8_t get_type(uint8_t x, uint8_t y, uint8_t z);
bool is_offset_up(uint8_t x, uint8_t y, uint8_t z);
bool is_offset_right(uint8_t x, uint8_t y, uint8_t z);

#endif