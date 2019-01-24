/*
 *--------------------------------------
 * Program Name: Mahjong
 * Author: commandblockguy
 * License:
 * Description: Is mahjong solitaire.
 *--------------------------------------
*/

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

#include <debug.h>

#include "game.h"
#include "gfx/tiles_gfx.h"
#include "layouts.h"

game_t game;

void main(void) {
	int option;
	uint24_t timer;

	/* Set the random seed */
	srand(rtc_Time());

	/* Set up graphics, etc. */
	gfx_Begin();
	gfx_SetPalette(tiles_gfx_pal, sizeof_tiles_gfx_pal, 0);
	gfx_SetDraw(gfx_buffer);

	/* Set up the highscore appvar */
	init_hs_appvar();

	/* Do menu stuff */
	/* Everything should exit back to the main menu */
	while(true) {
		/* Get which menu option was selected */
		option = main_menu();
		switch(option) {
			case MM_PLAY:
				/* If there is no recorded time, go to the level select for a new game */
				if(!(timer = read_save()))
					/* If the user doesn't select a layout, go back to main menu */
					if(layouts_menu()) continue;
				/* Reset timer to 0, or the value from the saved game */
				reset_timer(timer);
				play();
				break;
			case MM_EXIT:
				cleanup();
				return;
			case MM_CREDITS:
				credits();
				break;
			case MM_HOWTO:
				how_to();
				break;
		}
	}
}

void cleanup() {
	/* Cleanup */
	gfx_End();
}

/* temp export func
	ti_var_t var;
	pack_meta_t meta = {"MJ", 1, 1};

	ti_CloseAll();

	var = ti_Open("MJPack", "w+");

	ti_Write(&meta, sizeof(pack_meta_t), 1, var);
	ti_Write(&easy, sizeof(layout_t), 1, var);

	ti_CloseAll();
*/

void play() {
	/* Position for the on-screen pointer */
	uint24_t cursor_x = LCD_WIDTH / 2;
	uint8_t cursor_y = LCD_HEIGHT / 2;

	/* Initialise things */
	game.remaining_tiles = TILE_TYPES * 4;
	game.status = IN_PROGRESS;
	game.hl_x = game.hl_y = game.hl_z = -1;

	/* Draw the tiles and infobar */
	rerender();

	/* Main loop */
	while(game.status == IN_PROGRESS) {
		/* Handle keypresses */
		kb_Scan();

		/* Exit if clear is pressed */
		if(kb_Data[6] & kb_Clear) game.status = EXIT;

		/* Exit without saving if del is pressed */
		if(kb_Data[1] & kb_Del) game.status = EXIT_NO_SAVE;

		/* Pause if mode is pressed */
		if(kb_Data[1] & kb_Mode) pause();

		//This will probably get replaced with a fancier method later that causes the speed to gradually increase with time
		/* Control cursor movement */
		if(kb_Data[7] & kb_Up)    cursor_y -= CURSOR_SPEED;
		if(kb_Data[7] & kb_Down)  cursor_y += CURSOR_SPEED;
		if(kb_Data[7] & kb_Left)  cursor_x -= CURSOR_SPEED;
		if(kb_Data[7] & kb_Right) cursor_x += CURSOR_SPEED;

		/* Check if the cursor has gone off-screen, and if so move it to the other side */
		if(cursor_x + 5 < 5) cursor_x = LCD_WIDTH; /* Hacky underflow detection */
		if(cursor_y > LCD_HEIGHT + CURSOR_SPEED) cursor_y = LCD_HEIGHT; /* Hacky underflow detection */
		if(cursor_x > LCD_WIDTH)  cursor_x = 0;
		if(cursor_y > LCD_HEIGHT) cursor_y = 0;

		if(kb_Data[1] & kb_2nd) {

			/* Save the highlighted tile, if any */
			int8_t bk_hl_x = game.hl_x;
			int8_t bk_hl_y = game.hl_y;
			int8_t bk_hl_z = game.hl_z;

			/* Get which tile is under the cursor and highlight it */
			update_highlight(cursor_x, cursor_y);

			/* Try to remove both tiles */
			remove_tiles(game.hl_x, game.hl_y, game.hl_z, bk_hl_x, bk_hl_y, bk_hl_z);

			/* Reset the highlight if the tile is not removable */

			if(!is_removable(game.hl_x, game.hl_y, game.hl_z)) {
				set_highlight(-1, -1, -1);
			}

			/* Redraw everything */

			rerender();
		}

		/* Render things */
		
		render_stopwatch();
		draw_cursor(cursor_x, cursor_y);
	}

	/* If a win, add the time to the high scores, and set the last played level */
	if(game.status == WIN) {
		/* Disable the timer, so we can reference it later */
		timer_Control = TIMER1_DISABLE;
		add_hs(timer_1_Counter / 33, game.layout.name);
		set_last_level(game.pack_name, game.layout.name);
		win_popup();
	}

	if(game.status == LOSE) {
		set_last_level(game.pack_name, game.layout.name);
		if(lose_popup() == 0) {
			/* Restart level */

			/* Copy the backup of tiles into game.tiles */
			memcpy(game.tiles, game.initial_tiles, TILES_X * TILES_Y * TILES_Z * sizeof(tile_t));

			/* Start the timer over */
			reset_timer(0);

			/* Tail-recurse */
			play();
		}
	}

	if(game.status == EXIT_NO_SAVE) {
		set_last_level(game.pack_name, game.layout.name);
	}

	if(game.status == EXIT) {
		game.status = IN_PROGRESS;
		save_game(timer_1_Counter / 33);
	}
}
