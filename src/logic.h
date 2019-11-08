#ifndef H_LOGIC
#define H_LOGIC

#include "tile.h"

typedef struct Position {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} pos_t;

typedef struct Undo {
	pos_t pos1;
	pos_t pos2;
	tile_t tile1;
	tile_t tile2;
} undo_t;

/* Number of times loading a layout can result in no valid moves before giving up */
#define LAYOUT_TRIES 10

bool is_removable(uint8_t x, uint8_t y, uint8_t z);
bool right_blocked(uint8_t x, uint8_t y, uint8_t z);
bool left_blocked(uint8_t x, uint8_t y, uint8_t z);
bool top_blocked(uint8_t x, uint8_t y, uint8_t z);
bool remove_tiles(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2);
uint8_t calc_num_moves(void);
pos_t find_highlight(uint24_t x, uint8_t y);
void load_layout(layout_t *l, bool random);
void set_highlight(uint8_t x, uint8_t y, uint8_t z);
void reset_timer(uint24_t timer);
bool undo(void);
bool redo(void);

#endif