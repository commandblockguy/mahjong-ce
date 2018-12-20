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
#include "gfx/tiles_gfx.h"
#include "layouts.h"


void rerender() {
	gfx_FillScreen(BACKGROUND_COLOR);
	render_tiles();
	draw_infobar();
}


/* Draw the infobar */
void draw_infobar() {

	gfx_SetColor(INFOBAR_COLOR);
	gfx_FillRectangle(0, LCD_HEIGHT - INFOBAR_HEIGHT, LCD_WIDTH, INFOBAR_HEIGHT);

	/* Print the level name */
	gfx_SetTextXY(LCD_WIDTH + 5, LCD_HEIGHT - INFOBAR_HEIGHT / 2 - 4);
	gfx_SetTextFGColor(WHITE);
	gfx_PrintString(game.layout.name);

	gfx_PrintString(" | ");

	/* Number of tiles left */
	gfx_PrintUInt(game.remaining_tiles, num_digits(game.remaining_tiles));
	gfx_PrintString("/144 left | "); /* Not sure how to do this in a less lazy way, using defines */

	/* Number of moves left */
	gfx_PrintUInt(game.possible_moves, num_digits(game.possible_moves));
	gfx_PrintString(" moves");
}

/* Draw the tile grid */
void render_tiles() {
	int x, y, z;
	for(z = 0; z < TILES_Z; z++) {
		for(y = 0; y < TILES_Y; y++) {
			for(x = TILES_X - 1; x >= 0; x--) {
				if(game.tiles[x][y][z])
					/* Adding this check here speeds things up drastically since	*
					 * a frame doesn't need to be allocated for the function		*/
					render_tile(x, y, z, game.hl_x == x && game.hl_y == y && game.hl_z == z);
			}
		}
	}
}

/* Isometric-ish rendering */
void render_tile(uint8_t x, uint8_t y, uint8_t z, bool highlight) {
	uint24_t base_x;
	uint8_t base_y;

	/* Do nothing if no tile */
	if(!get_type(x, y, z)) return;

	/* Calculate the base position based on coords */
	base_x = tile_base_x(x, y, z);
	base_y = tile_base_y(x, y, z);

	render_raw_tile(base_x, base_y, get_type(x, y, z), highlight);

	/* Draw a darker border on the right side if there is no adjacent tile */
	if(!get_type(x + 1, y, z) || is_offset_right(x, y, z) != is_offset_right(x + 1, y, z)) {
		gfx_SetColor(BLACK);
		gfx_VertLine(base_x + TILE_WIDTH - 1, base_y, TILE_HEIGHT - 1);
	}

	/* Draw a darker border on the top side if there is no adjacent tile */
	if(!get_type(x, y - 1, z) || is_offset_up(x, y, z) != is_offset_up(x, y - 1, z)) {
		gfx_SetColor(BLACK);
		gfx_HorizLine(base_x, base_y, TILE_WIDTH);
	}

}

void render_raw_tile(int24_t base_x, int24_t base_y, uint8_t type, bool highlight) {
	/* Display the face sprite */
	if(highlight) {
		gfx_SetColor(HIGHLIGHT_FACE_COLOR);
		gfx_FillRectangle(base_x + 1, base_y + 1, TILE_WIDTH - 2, TILE_HEIGHT - 2);
		gfx_TransparentSprite(tilemap_tiles[type - 1], base_x + 1, base_y + 1);
	} else {
		/* Sprites are indexed from zero, but type 0 is no tile, so we subtract 1 */
		gfx_Sprite(tilemap_tiles[type - 1], base_x + 1, base_y + 1);
	}

	/* Draw the tile border. */
	gfx_SetColor(highlight ? HIGHLIGHT_SIDE_COLOR : SIDE_COLOR);
	gfx_HorizLine(base_x, base_y, TILE_WIDTH);
	gfx_VertLine(base_x + TILE_WIDTH - 1, base_y, TILE_HEIGHT - 1);
	gfx_VertLine(base_x, base_y, TILE_HEIGHT - 1);

	/* Display the left side of the tile */
	gfx_VertLine(base_x - 1, base_y + 1, TILE_HEIGHT - 1);
	gfx_VertLine(base_x - 2, base_y + 2, TILE_HEIGHT - 1);

	/* Display the bottom side of the tile */

	gfx_SetColor(highlight ? HIGHLIGHT_BOTTOM_COLOR : BOTTOM_COLOR);
	gfx_HorizLine(base_x, base_y + TILE_HEIGHT - 1, TILE_WIDTH);

	gfx_HorizLine(base_x - 1, base_y + TILE_HEIGHT, TILE_WIDTH);
	gfx_HorizLine(base_x - 2, base_y + TILE_HEIGHT + 1, TILE_WIDTH);
}

/* Draw the cursor without redrawing everything else */
void draw_cursor(uint24_t x, uint8_t y) {
	/* Temporary sprite to hold the section of the screen the cursor draws over */
	gfx_TempSprite(temp_sprite, 3, 3);

	gfx_GetSprite(temp_sprite, x - 1, y - 1);

	/* Draw the cursor */
	gfx_SetColor(CURSOR_COLOR);
	gfx_FillRectangle(x - 1, y - 1, 3, 3);

	/* Copy the graphics buffer to the screen */

	gfx_BlitBuffer();

	/* In the buffer, remove the cursor again by drawing the sprite over it */

	gfx_Sprite(temp_sprite, x - 1, y - 1);

}

void render_stopwatch(void) {
	/* Draw the stopwatch */
	uint24_t ms = atomic_load_increasing_32(&timer_1_Counter) / 33;
	uint8_t minutes, seconds;

	/* Fill the screen region to give a blank slate */
	gfx_SetColor(INFOBAR_COLOR);
	gfx_FillRectangle(LCD_WIDTH - STOPWATCH_WIDTH, LCD_HEIGHT - INFOBAR_HEIGHT, STOPWATCH_WIDTH, INFOBAR_HEIGHT);

	gfx_SetTextXY(LCD_WIDTH - STOPWATCH_WIDTH, LCD_HEIGHT - INFOBAR_HEIGHT / 2 - 4);
	gfx_SetTextFGColor(WHITE);

	minutes = ms / 60000;
	seconds = (ms / 1000) % 60;

	/* Print the actual values */
	gfx_PrintUInt(minutes, num_digits(minutes));
	gfx_PrintString(":");
	gfx_PrintUInt(seconds, 2);
	gfx_PrintString(".");
	gfx_PrintUInt(ms % 1000, 3);

}

/* Get the base of a tile in screen space */
uint24_t tile_base_x(uint8_t x, uint8_t y, uint8_t z) {
	return game.layout.start_x + (x * TILE_WIDTH) + (SHIFT_X * z) + (is_offset_right(x, y, z) ? TILE_WIDTH  / 2 : 0);
}
uint8_t tile_base_y(uint8_t x, uint8_t y, uint8_t z) {
	return game.layout.start_y + (y * TILE_HEIGHT) - SHIFT_Y * z - (is_offset_up(x, y, z) ? TILE_HEIGHT / 2 : 0);
}
