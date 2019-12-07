/*
 *--------------------------------------
 * Program Name: Mahjong
 * Author: commandblockguy
 * License:
 * Description: Is mahjong solitaire.
 *--------------------------------------
*/

#include <stdbool.h>
#include <stdint.h>
#include <tice.h>

#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <debug.h>

#include "game.h"
#include "gfx.h"
#include "gfx/tiles_gfx.h"
#include "logic.h"
#include "storage.h"
#include "tile.h"
#include "ui.h"

game_t game;

void play(void);
void cleanup(void);

void main(void) {
	int option;
	uint24_t timer;

	dbg_sprintf(dbgout, "started\n");

	/* Set the random seed */
	srand(rtc_Time());

	/* Set up graphics, etc. */
	gfx_Begin();
	gfx_SetPalette(tiles_gfx_pal, sizeof_tiles_gfx_pal, 0);
	gfx_FillScreen(BACKGROUND_COLOR);
	gfx_SetDraw(gfx_buffer);

	/* Set up the highscore appvar */
	init_hs_appvar();

	/* Do menu stuff */
	/* Everything should exit back to the main menu */
	while(true) {
		/* Get which menu option was selected */
		option = main_menu();
		switch(option) {
			case MM_RESUME:
				/* If there is no recorded time, go to the level select for a new game */
				if(!(timer = read_save()))
					/* If the user doesn't select a layout, go back to main menu */
					if(layouts_menu()) continue;
				/* Reset timer to 0, or the value from the saved game */
				reset_timer(timer);
				play();
				break;
			case MM_NEW_GAME:
				/* If the user doesn't select a layout, go back to main menu */
				if(layouts_menu()) continue;
				/* Reset timer to 0 */
				reset_timer(0);
				play();
				break;
			case MM_HOWTO:
				how_to();
				break;
			case MM_CREDITS:
				credits();
				break;
			case MM_EXIT:
			default:
				cleanup();
				return;
		}
	}
}

void cleanup(void) {
	/* Cleanup */
	gfx_End();
}

void play(void) {
	/* Position for the on-screen pointer */
	uint24_t cursor_x = LCD_WIDTH / 2;
	uint8_t cursor_y = LCD_HEIGHT / 2;

	/* Initialise things */
	set_highlight(-1, -1, -1);
	calc_num_moves();

	/* Draw the tiles and infobar */
	rerender();

	/* Main loop */
	while(game.status == IN_PROGRESS) {
		pos_t hover;

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
			bool removed;

			/* Save the highlighted tile, if any */
			pos_t old;
			memcpy(&old, &game.highlight, sizeof(pos_t));

			/* Get which tile is under the cursor and highlight it */
			game.highlight = find_highlight(cursor_x, cursor_y);

			/* Try to remove both tiles */
			removed = remove_tiles(game.highlight.x, game.highlight.y, game.highlight.z, old.x, old.y, old.z);

			/* Reset the highlight if the tile is not removable */

			if(!is_removable(game.highlight.x, game.highlight.y, game.highlight.z)) {
				set_highlight(-1, -1, -1);
			}

			/* Redraw everything if necessary */
			if(removed || game.highlight.x != old.x || game.highlight.y != old.y || game.highlight.z != old.z) {
				rerender();
			}
		}

		if(kb_Data[2] & kb_Alpha) {
			if(undo()) rerender();
			while(kb_Data[2]) kb_Scan();
		}

		if(kb_Data[3] & kb_GraphVar) {
			if(redo()) rerender();
			while(kb_Data[3]) kb_Scan();
		}

		if(kb_Data[4] & kb_Stat) {
			if(game.magnifier_shown) {
				game.magnifier_shown = false;
				rerender();
				// TODO: shift the level over a bit
			} else {
				game.magnifier_shown = true;
			}
			while(kb_Data[4]) kb_Scan();
		}

		/* Render things */

		hover = find_highlight(cursor_x, cursor_y);
		render_tile_info(get_type(hover.x, hover.y, hover.z));
		
		render_stopwatch();
		if(game.magnifier_shown)
			draw_magnifier(cursor_x, cursor_y);
		draw_cursor(cursor_x, cursor_y);

		if(game.status == LOSE) {
			char name_buf[PLAYER_NAME_LENGTH];
			set_last_level(game.pack_name, game.layout.name);
			switch(lose_popup()) {
				case 0:
					/* Restart level */

					/* Copy the backup of tiles into game.tiles */
					memcpy(game.tiles, game.initial_tiles, sizeof(game.tiles));

					game.remaining_tiles = TILE_TYPES * 4;
					game.status = IN_PROGRESS;
					game.undos = 0;
					game.redos = 0;

					/* Start the timer over */
					reset_timer(0);

					/* Tail-recurse */
					play();
					return;
				case 1:
					/* Undo */
					if(undo()) {
						game.status = IN_PROGRESS;
						rerender();
					}
					break;
				case 2:
				default:
					/* Exit */
					enter_name(name_buf);
					add_hs((uint24_t)0xFF0000 | game.remaining_tiles, game.layout.name, name_buf);
					break;
			}
		}
	}

	/* If a win, add the time to the high scores, and set the last played level */
	if(game.status == WIN) {
		char name_buf[PLAYER_NAME_LENGTH];
		/* Disable the timer, so we can reference it later */
		timer_Control = TIMER1_DISABLE;
		enter_name(name_buf);
		add_hs(timer_1_Counter / 33, game.layout.name, name_buf);
		set_last_level(game.pack_name, game.layout.name);
		win_popup();
	}

	if(game.status == EXIT_NO_SAVE) {
		set_last_level(game.pack_name, game.layout.name);
	}

	if(game.status == EXIT) {
		game.status = IN_PROGRESS;
		save_game(timer_1_Counter / 33);
	}
}
