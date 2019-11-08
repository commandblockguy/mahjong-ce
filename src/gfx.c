#include <stdbool.h>
#include <stdint.h>
#include <tice.h>

#include <stdio.h>

#include <graphx.h>

#include <debug.h>

#include "game.h"
#include "gfx.h"
#include "gfx/tiles_gfx.h"
#include "gfx/gfx_group.h"
#include "tile.h"
#include "util.h"

/* Re-render the entire screen */
void rerender(void) {
	dbg_sprintf(dbgout, "redrawing\n");
	gfx_FillScreen(BACKGROUND_COLOR);
	render_tiles();
	draw_infobar();
}


/* Draw the infobar */
void draw_infobar(void) {

	gfx_SetColor(INFOBAR_COLOR);
	gfx_FillRectangle(0, LCD_HEIGHT - INFOBAR_HEIGHT, LCD_WIDTH, INFOBAR_HEIGHT);

	/* Print the level name */
	gfx_SetTextXY(LCD_WIDTH + 5, LCD_HEIGHT - INFOBAR_HEIGHT / 2 - 4);
	gfx_SetTextFGColor(WHITE);
	gfx_SetTextScale(1, 1);
	gfx_PrintString(game.layout.name);

	gfx_PrintString(" | ");

	/* Number of tiles left */
	gfx_PrintUInt(game.remaining_tiles, num_digits(game.remaining_tiles));
	gfx_PrintString("/144 left | "); /* Not sure how to do this in a less lazy way, using defines */
	/* You can stringify things in the C preprocessor, and multiply things */
	/* However I don't think it's possible to stringify the result of a multiplication */

	/* Number of moves left */
	gfx_PrintUInt(game.possible_moves, num_digits(game.possible_moves));
	gfx_PrintString(" moves");
}

/* Draw the tile grid */
void render_tiles(void) {
	int x, y, z;
	for(z = 0; z < TILES_Z; z++) {
		for(y = 0; y < TILES_Y; y++) {
			for(x = TILES_X - 1; x >= 0; x--) {
				if(game.tiles[x][y][z])
					/* Adding this check here speeds things up drastically since	*
					 * a frame doesn't need to be allocated for the function		*/
					render_tile(x, y, z, game.highlight.x == x && game.highlight.y == y && game.highlight.z == z);
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

/* Render a particular tile type at a particular location in screen space */
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
	gfx_TempSprite(temp_sprite, cursor_width, cursor_height);

	gfx_GetSprite(temp_sprite, x, y);

	/* Draw the cursor */
	gfx_TransparentSprite(cursor, x, y);

	/* Copy the graphics buffer to the screen */

	gfx_BlitBuffer();

	/* In the buffer, remove the cursor again by drawing the sprite over it */

	gfx_Sprite(temp_sprite, x, y);

}

/* Draw the stopwatch in the bottom of the screen */
void render_stopwatch(void) {
	/* Draw the stopwatch */
	uint24_t ms = atomic_load_increasing_32(&timer_1_Counter) / 33;
	uint8_t minutes, seconds;

	/* Fill the screen region to give a blank slate */
	gfx_SetColor(INFOBAR_COLOR);
	gfx_FillRectangle(LCD_WIDTH - STOPWATCH_WIDTH, LCD_HEIGHT - INFOBAR_HEIGHT, STOPWATCH_WIDTH, INFOBAR_HEIGHT);

	gfx_SetTextFGColor(WHITE);
	gfx_SetTextXY(LCD_WIDTH - STOPWATCH_WIDTH, LCD_HEIGHT - INFOBAR_HEIGHT / 2 - 4);

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

/* Draw the magnifier in the bottom right of the screen */
void draw_magnifier(uint24_t csr_x, uint8_t csr_y) {
	uint24_t x = LCD_WIDTH  - MAGNIFIER_X * MAGNIFIER_SCALE;
	uint24_t y = LCD_HEIGHT - MAGNIFIER_Y * MAGNIFIER_SCALE - INFOBAR_HEIGHT;

	gfx_TempSprite(mag, MAGNIFIER_X, MAGNIFIER_Y);

	/* Clip the copied sprite */

	int24_t left  = csr_x - MAGNIFIER_X / 2;
	int24_t right = csr_x + MAGNIFIER_X / 2;
	int24_t up    = csr_y - MAGNIFIER_Y / 2;
	int24_t down  = csr_y + MAGNIFIER_Y / 2;

	if(left < 0) left = 0;
	if(right > LCD_WIDTH) right = LCD_WIDTH;
	if(up < 0) up = 0;
	if(down > LCD_HEIGHT) down = LCD_HEIGHT;

	/* Fill the background so there are no issues */
	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(x, y, MAGNIFIER_X * MAGNIFIER_SCALE, MAGNIFIER_Y * MAGNIFIER_SCALE);

	/* Copy the area around the cursor */
	mag->width = right - left;
	mag->height = down - up;
	gfx_GetSprite(mag, left, up);

	/* Draw the lines bordering the magnifier */
	gfx_SetColor(WHITE);
	gfx_HorizLine_NoClip(x - 1, y - 1, MAGNIFIER_X * MAGNIFIER_SCALE + 1);
	gfx_VertLine_NoClip (x - 1, y - 1, MAGNIFIER_Y * MAGNIFIER_SCALE + 1);

	gfx_ScaledSprite_NoClip(mag,
		x + MAGNIFIER_SCALE * (left - csr_x + MAGNIFIER_X / 2),
		y + MAGNIFIER_SCALE * (up   - csr_y + MAGNIFIER_Y / 2),
		MAGNIFIER_SCALE, MAGNIFIER_SCALE);
}

/* Display the number of tiles with the same type as teh one under the cursor */
void render_tile_info(tile_t tile) {
	/* Erase the previous stuff */
	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle(2, 2, TILE_WIDTH + 4 + gfx_GetStringWidth(" x 0"), TILE_HEIGHT + 4);

	if(tile) {
		render_raw_tile(4, 2, tile, false);
		gfx_SetTextFGColor(WHITE);
		gfx_SetTextScale(1, 1);
		gfx_PrintStringXY(" x ", TILE_WIDTH + 4, 2 + (TILE_HEIGHT - TEXT_HEIGHT) / 2);
		gfx_PrintUInt(game.tile_count[tile - 1], 1);
	}
}
