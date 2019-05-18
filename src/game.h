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

#define TILE_WIDTH  14
#define TILE_HEIGHT 21
//Isometric perspective stuff
#define SHIFT_X 2
#define SHIFT_Y 2

/* Size of the tiel array */
#define TILES_X 17
#define TILES_Y 10
#define TILES_Z 8

/* Colors */
#define BACKGROUND_COLOR 1
#define BOTTOM_COLOR 2
#define SIDE_COLOR 3
#define HIGHLIGHT_BOTTOM_COLOR 4
#define HIGHLIGHT_SIDE_COLOR 5
#define HIGHLIGHT_FACE_COLOR 6
#define BLACK 7
#define WHITE 0
#define INFOBAR_COLOR 9
#define CURSOR_COLOR 10

#define CURSOR_SPEED 2

#define INFOBAR_HEIGHT 12
#define STOPWATCH_WIDTH 64

#define MAGNIFIER_X 32
#define MAGNIFIER_Y 32
#define MAGNIFIER_SCALE 3

#define SCORES 8

#define TEXT_HEIGHT 8

#define MM_PLAY 0
#define MM_HOWTO 1
#define MM_CREDITS 2
#define MM_EXIT 3

/* Number of times loading a layout can result in no valid moves before giving up */
#define LAYOUT_TRIES 10

#define check_value get_ans

typedef struct Position {
	int8_t x;
	int8_t y;
	int8_t z;
} pos_t;

typedef struct Undo {
	pos_t pos1;
	pos_t pos2;
	tile_t tile1;
	tile_t tile2;
} undo_t;

/* Giant jumbled mess of function declarations */

void render_tiles(void);
void render_tile(uint8_t x, uint8_t y, uint8_t z, bool highlight);
void draw_cursor(uint24_t x, uint8_t y);
bool is_removable(uint8_t x, uint8_t y, uint8_t z);
bool left_blocked(uint8_t x, uint8_t y, uint8_t z);
bool right_blocked(uint8_t x, uint8_t y, uint8_t z);
bool top_blocked(uint8_t x, uint8_t y, uint8_t z);
bool remove_tiles(uint8_t x1, uint8_t y1,  uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2);
uint8_t get_type(uint8_t x, uint8_t y, uint8_t z);
bool is_offset_up(uint8_t x, uint8_t y, uint8_t z);
bool is_offset_right(uint8_t x, uint8_t y, uint8_t z);
uint24_t tile_base_x(uint8_t x, uint8_t y, uint8_t z);
uint8_t tile_base_y(uint8_t x, uint8_t y, uint8_t z);
pos_t find_highlight(uint24_t x, uint8_t y);
void set_highlight(int8_t x, int8_t y, int8_t z);
uint8_t calc_num_moves(void);
void load_layout(layout_t *l, bool random);
void shuffle(uint8_t * array, uint24_t size);
void render_stopwatch(void);
void draw_magnifier(uint24_t csrX, uint8_t csrY);
void draw_infobar(void);
void rerender(void);
uint8_t num_digits(uint8_t n);
int get_ans(void);
void pause(void);
void play(void);
void init_hs_appvar(void);
void render_raw_tile(int24_t base_x, int24_t base_y, uint8_t type, bool highlight);
void add_hs(uint24_t time, char *name);
uint24_t get_hs(char* name, uint8_t rank);
uint8_t occurrences(char *str, char delim);
uint8_t main_menu(void);
uint8_t layouts_menu(void);
void credits(void);
void how_to(void);
void how_to_page(uint8_t page);
void cleanup(void);
void packs_menu(void);
uint8_t get_packs(char dest[][9], uint8_t max);
uint24_t read_save(void);
void reset_timer(uint24_t timer);
void win_popup(void);
void set_last_level(char *pack, char *level);
uint8_t lose_popup(void);
void save_game(uint24_t timer);
bool undo(void);
bool redo(void);

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