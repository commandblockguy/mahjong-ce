#include <stdbool.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <string.h>


#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include <debug.h>

#include "game.h"
#include "gfx.h"
#include "logic.h"
#include "storage.h"
#include "ui.h"
#include "util.h"

/* One pixel padding, plus one pixel thick line */
#define BUTTON_HEIGHT (TEXT_HEIGHT + 4)

/* Number of tiles in the columns on the side of the screen */
#define MM_TILES_Y ((LCD_HEIGHT / TILE_HEIGHT) + 2)
/* Distance between the edges of the screen and the outside edges of the columns */
#define MM_SIDE_WIDTH 25

#define PACKS_MENU_LINES 30

const char getkey_letters[] = "\0\0\0\0\0\0\0\0\0\0\"WRMH\0\0?\0VQLG\0\0:ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
const char getkey_numeric[] = "\0\0\0\0\0\0\0\0\0\0+-*/^\0\0-369)\0\0\0.258(\0\0\0000147,\0\0\0\0>\0\0\0\0\0\0\0\0\0\0\0\0\0";

uint8_t main_menu(void) {
	int selection = 0, i, start, offset;
	bool key_pressed = true;

	tile_t left[MM_TILES_Y];
	tile_t right[MM_TILES_Y];

    /* Fill random tiles */
	for(i = 0; i < MM_TILES_Y; i++) {
		left[i] = randInt(1, TILE_TYPES);
		right[i] = randInt(1, TILE_TYPES);
	}

	/* Change colors */
	if(check_value() == 0x2B1D) gfx_palette[1] = 0x7953;

	while(true) {

		/* Stuff for tiles on the sides */
		for(start = 0; start < MM_TILES_Y; start++) {
			/* Rotating queue thing */
			/* Shift the offset for smooth transitions */
			for(offset = 0; offset < TILE_HEIGHT; offset++) {

				const uint8_t num_options = 5;

				const char* mm_strings[] = {
					"Resume",
					"New Game",
					"How to Play",
					"Credits",
					"Exit"
				};

				/* Do menu logic */

				kb_Scan();
					
				if(!key_pressed) {
					if(kb_Data[6] & kb_Clear) return MM_EXIT;
					if(kb_Data[1] & kb_2nd) return selection;
					
					if(kb_Data[7] & kb_Up) selection--;
					if(kb_Data[7] & kb_Down) selection++;
					
					if(selection < 0) selection = MM_EXIT;
					if(selection > MM_EXIT) selection = 0;
				}

				/* If a key is pressed, wait for it to release */
				key_pressed = kb_Data[1] || kb_Data[6] || kb_Data[7];

				gfx_FillScreen(BACKGROUND_COLOR);
				gfx_SetTextScale(1, 1);

				/* Draw options text */
				for(i = 0; i < num_options; i++) {
					uint8_t width = gfx_GetStringWidth(mm_strings[i]);
					uint8_t y = (LCD_HEIGHT + 3 * TEXT_HEIGHT) / 2 + 2 * i * TEXT_HEIGHT;
					if(selection == i) {
						#ifdef SELECTION_TILES
						const uint8_t spacing = 10;
						uint8_t tile_y = y + TEXT_HEIGHT / 2 - TILE_HEIGHT / 2;

						/* Draw the tiles to the sides of the text */
						render_raw_tile((LCD_WIDTH - width) / 2 - spacing - TILE_WIDTH, tile_y, select_tile, false);
						render_raw_tile((LCD_WIDTH + width) / 2 + spacing, tile_y, select_tile, false);
						#endif

						gfx_SetTextFGColor(HIGHLIGHT_SIDE_COLOR);
					} else {
						gfx_SetTextFGColor(WHITE);
					}

					gfx_PrintStringXY(mm_strings[i], (LCD_WIDTH - width) / 2, y);
				}

				gfx_SetTextScale(2, 2);
				gfx_SetTextFGColor(BLACK);

				/* Draw the tiles up top */
				for(i = 6; i >= 0; i--) {
					char *str = "Mahjong";
					uint8_t j;
					const uint8_t edge = 15;
					const uint24_t use_area = LCD_WIDTH - 2 * (MM_SIDE_WIDTH + TILE_WIDTH) - 2 * edge;
					uint24_t base_x = MM_SIDE_WIDTH + TILE_WIDTH + edge + 4 + i * ((use_area - 7 * 2 * TILE_WIDTH) / 6 + 2 * TILE_WIDTH);
					/* Makes a nice sine wave */
					uint24_t base_y = (float)LCD_HEIGHT / 6 + sin(2.0 * M_PI * (offset + ((float)(6 - i) * TILE_HEIGHT / 7)) / TILE_HEIGHT) * 8;

					/* Fill in the tile background */
					gfx_SetColor(WHITE);
					gfx_FillRectangle(base_x, base_y, 2 * TILE_WIDTH, 2 * TILE_HEIGHT);
					
					/* Draw letter */
					gfx_SetTextXY(base_x + (TILE_WIDTH - gfx_GetCharWidth(str[i]) / 2), base_y + TILE_HEIGHT - TEXT_HEIGHT);
					gfx_PrintChar(str[i]);

					/* Draw the tile border. */
					gfx_SetColor(BOTTOM_COLOR);
					gfx_HorizLine(base_x, base_y, 2 * TILE_WIDTH);
					gfx_VertLine(base_x + 2 * TILE_WIDTH - 1, base_y, 2 * TILE_HEIGHT - 1);

					gfx_SetColor(SIDE_COLOR);

					/* Display the left side of the tile */
					for(j = 0; j < 5; j++)
						gfx_VertLine(base_x - j, base_y + j, 2 * TILE_HEIGHT - 1);

					/* Display the bottom side of the tile */
					gfx_SetColor(BOTTOM_COLOR);
					for(j = 0; j < 5; j++)
						gfx_HorizLine(base_x - j, base_y + 2 * TILE_HEIGHT - 1 + j, 2 * TILE_WIDTH);
				}

				/* Draw the tiles on the sides */
				for(i = 0; i < MM_TILES_Y; i++) {
					render_raw_tile(MM_SIDE_WIDTH, i * TILE_HEIGHT - offset, left[(start + i) % MM_TILES_Y], false);
				}
				for(i = MM_TILES_Y; i >= 0; i--) {
					render_raw_tile(LCD_WIDTH - TILE_WIDTH - MM_SIDE_WIDTH, LCD_HEIGHT - i * TILE_HEIGHT + offset, right[(start + i - 1) % MM_TILES_Y], false);
				}

				gfx_BlitBuffer();
			}
			/* Change the tile which is offscreen */
			left[start] = randInt(1, TILE_TYPES);
			right[start] = randInt(1, TILE_TYPES);
		}
	}
}

/* Returns a status code */
/* 0 if successful, 1 if user hit clear */
uint8_t layouts_menu(void) {

	if(strcmp(game.pack_name, "") == 0) {
		/* No pack selected in save file */
		packs_menu();
	}

	while(true) {
		ti_var_t var;
		int selection = 0;
		pack_meta_t meta;
		layout_t *layouts;
		bool key_pressed = true;
		uint8_t key_time = 0;

		/* Clear was pressed, or something. */
		if(strcmp(game.pack_name, "") == 0) return 1;

		ti_CloseAll();

		var = ti_Open(game.pack_name, "r");

		/* Return 1 if it fails */
		if(!var) return 1;

		/* Get the number of layouts */
		ti_Read(&meta, sizeof(pack_meta_t), 1, var);

		/* If no layouts found */
		if(meta.version > 1) {
			const char *str_version = "Unsupported pack version.";

			gfx_FillScreen(BACKGROUND_COLOR);
			gfx_SetTextFGColor(WHITE);
			gfx_PrintStringXY(str_version, (LCD_WIDTH - gfx_GetStringWidth(str_version)) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2);
			gfx_BlitBuffer();

			while(kb_Data[1]) kb_Scan();
			while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
			packs_menu();
			continue;
		}

		/* If no layouts found */
		if(!meta.num_layouts) {
			const char *str_no_layouts = "No layouts in this pack.";

			gfx_FillScreen(BACKGROUND_COLOR);
			gfx_SetTextFGColor(WHITE);
			gfx_PrintStringXY(str_no_layouts, (LCD_WIDTH - gfx_GetStringWidth(str_no_layouts)) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2);
			gfx_BlitBuffer();

			while(kb_Data[1]) kb_Scan();
			while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
			packs_menu();
			continue;
		}

		/* Get a pointer to the array of things */
		layouts = ti_GetDataPtr(var);

		/* Try to find the selected level name */
		if(strcmp(game.layout.name, "") != 0) {
			/* Loop through each layout */
			uint24_t i;
			for(i = 0; i < meta.num_layouts; i++) {
				/* When we reach the layout with the same name, set that as our selection */
				if(strcmp(game.layout.name, layouts[i].name) == 0) {
					selection = i;
					break;
				}
			}
		}

		/* Input loop */
		while(true) {
			const char *str_hs = "high scores:";
			uint24_t i;
			uint8_t key;
			uint8_t width;
			/* Handle keypresses, yada yada. */
			kb_Scan();
			if(!key_pressed || key_time >= 10) {
				if(kb_Data[6] == kb_Clear) {
					packs_menu();
					break;
				}
	
				if(kb_Data[1] == kb_2nd) {
					for(i = 0; i < LAYOUT_TRIES; i++) {
						/* Load the selected layout */
						load_layout(&layouts[selection], true);
						if(calc_num_moves()) break;
					}
					if(i == LAYOUT_TRIES) {
						/* Could not add tiles in a way that does not immediately result in a loss */
						const char *str_imm_loss = "Error: Layout results in immediate loss.";

						gfx_FillScreen(BACKGROUND_COLOR);
						gfx_SetTextFGColor(WHITE);
						gfx_PrintStringXY(str_imm_loss, (LCD_WIDTH - gfx_GetStringWidth(str_imm_loss)) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2);
						gfx_BlitBuffer();

						while(kb_Data[1]) kb_Scan();
						while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
						continue;
					}
					return 0;
				}

				if(kb_Data[7] & kb_Up) selection--;
				if(kb_Data[7] & kb_Down) selection++;

				if(selection < 0) selection = meta.num_layouts - 1;
				if(selection >= meta.num_layouts) selection = 0;
			}

			/* If a key is pressed, wait for it to release */
			key_pressed = kb_Data[1] || kb_Data[6] || kb_Data[7];

			if(!key_pressed) key_time = 0;
			else if(key_time < 10) key_time++;

			key = os_GetCSC();
			if(getkey_letters[key]) {
				char str[2] = {0};
				*str = getkey_letters[key];

				dbg_sprintf(dbgout, "key %u, %s\n", key, str);

				/* Scroll to the correct letter */
				/* Loop through each layout */
				for(i = 0; i < meta.num_layouts; i++) {
					if(strcmp(layouts[i].name, str) > 0) {
						selection = i;
						break;
					}
				}
			}

			/* gooey */
			gfx_FillScreen(BACKGROUND_COLOR);

			/* Draw the margin and highscore inset */
			gfx_SetColor(BOTTOM_COLOR);
			gfx_FillRectangle(0, 0, 10, LCD_HEIGHT);
			gfx_FillRectangle(LCD_WIDTH / 2, LCD_HEIGHT / 2, LCD_WIDTH / 2, LCD_HEIGHT / 2);

			/* Draw the border of the layout view */
			gfx_VertLine(LCD_WIDTH / 2, 0, LCD_HEIGHT / 2);

			/* Draw the layout view */
			/* This probably needs to be fast, so no isometric awesomeness. */
			/* Loop through each slot */
			/* Use 6 by 10 tiles, or a 3, 5 offset amount per slot coord */
			gfx_SetColor(WHITE);
			for(i = 0; i < TILE_TYPES * 4; i++) {
				gfx_FillRectangle(LCD_WIDTH * 3 / 4 - TILES_X * 3 + 3 * layouts[selection].slots[i].x,
					LCD_HEIGHT / 4 - TILES_Y * 5 + 5 * layouts[selection].slots[i].y, 6, 10);
			}

			/* Draw the layout names */
			gfx_SetTextScale(1, 1);
			for(i = (uint24_t) (selection >= 10 ? selection - 10 : 0); i < selection + 10 && i < meta.num_layouts; i++) {
				width = gfx_GetStringWidth(layouts[i].name);
				gfx_SetTextFGColor(i == selection ? WHITE : SIDE_COLOR);
				gfx_PrintStringXY(layouts[i].name, (LCD_WIDTH / 2 + 5 - width) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2 - 12 * (selection - i));
			}

			width = gfx_GetStringWidth(layouts[selection].name);

			/* Draw some pretty lines */
			gfx_SetColor(WHITE);
			gfx_HorizLine((LCD_WIDTH / 2 + 5) / 2 - 4 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT / 2 - 2, width + 7);
			gfx_HorizLine((LCD_WIDTH / 2 + 5) / 2 - 4 - width / 2, LCD_HEIGHT / 2 + TEXT_HEIGHT / 2 + 1, width + 7);

			/* High score text */
			gfx_SetTextFGColor(WHITE);
			gfx_PrintStringXY(str_hs, LCD_WIDTH * 3 / 4 - gfx_GetStringWidth(str_hs) / 2, LCD_HEIGHT / 2 + 2);

			/* Actual high scores */
			for(i = 0; i < SCORES; i++) {
				char player_name[PLAYER_NAME_LENGTH];
				uint24_t time;
				uint24_t minutes;
				uint8_t name_width;

				time = get_hs(layouts[selection].name, i, player_name);

				gfx_SetTextXY(LCD_WIDTH / 2 + 8, LCD_HEIGHT / 2 + 16 + 12 * i);

				gfx_PrintUInt(i + 1, 1);
				gfx_PrintString(". ");

				if(time == -1) {
					continue;
				}

				if(time >> 16 == 0xFF) {
					/* Value is a number of tiles remaining */
					gfx_PrintUInt(time & 0xFF, num_digits(time & 0xFF));
					gfx_PrintString(" left");
				} else {

					minutes = time / 60000;

					/* Print the actual values */
					gfx_PrintUInt(minutes, num_digits(minutes));
					gfx_PrintString(":");
					gfx_PrintUInt((time / 1000) % 60, 2);
					gfx_PrintString(".");
					gfx_PrintUInt(time % 1000, 3);
				}

				name_width = gfx_GetStringWidth(player_name);
				gfx_PrintStringXY(player_name, LCD_WIDTH - name_width - 2, gfx_GetTextY());
			}

			gfx_BlitBuffer();
		}
	}
}

/* Returns the name of the selected pack */
void packs_menu(void) {
	char names[PACKS_MENU_LINES][9];
	uint8_t num_names;
	int selection = 0;
	bool key_pressed = true;

	/* Get the list of packs */
	num_names = get_packs(names, PACKS_MENU_LINES);

	if(!num_names) {
		const char *str_no_packs = "No layout packs found.";
		/* There are no packs installed */
		gfx_FillScreen(BACKGROUND_COLOR);
		gfx_SetTextFGColor(WHITE);
		gfx_PrintStringXY(str_no_packs, (LCD_WIDTH - gfx_GetStringWidth(str_no_packs)) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2);
		gfx_BlitBuffer();

		strcpy(game.pack_name, "");
		while(kb_Data[1]) kb_Scan();
		while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
		return;
	}

	/* Main loop */
	while(true) {
		uint8_t width;
		uint24_t i;
		/* Handle keypresses */
		kb_Scan();

		if(!key_pressed) {
			if(kb_Data[6] & kb_Clear) {
				strcpy(game.pack_name, "");
				return;
			}
		
			if(kb_Data[1] & kb_2nd) {
				strcpy(game.pack_name, names[selection]);
				return;
			}
		
			if(kb_Data[7] & kb_Up) selection--;
			if(kb_Data[7] & kb_Down) selection++;
		
			if(selection < 0) selection = num_names - 1;
			if(selection >= num_names) selection = 0;
		}

		/* If a key is pressed, wait for it to release */
		key_pressed = kb_Data[1] || kb_Data[6] || kb_Data[7];

		gfx_FillScreen(BACKGROUND_COLOR);

		/* Draw the margins */
		gfx_SetColor(BOTTOM_COLOR);
		gfx_FillRectangle(0, 0, LCD_WIDTH / 6, LCD_HEIGHT);
		gfx_FillRectangle(2 * LCD_WIDTH / 3, 0, LCD_WIDTH / 3, LCD_HEIGHT);

		/* Draw the title in the right margin */
		/* (the left margin is the wrong margin) */
		gfx_SetTextFGColor(WHITE);
		gfx_SetTextScale(2, 2);
		gfx_PrintStringXY("pack", LCD_WIDTH * 5 / 6 - gfx_GetStringWidth("pack") / 2, LCD_HEIGHT / 2 - 3 * TEXT_HEIGHT);
		gfx_PrintStringXY("select", LCD_WIDTH * 5 / 6 - gfx_GetStringWidth("select") / 2, LCD_HEIGHT / 2 + TEXT_HEIGHT);
		gfx_SetTextScale(1, 1);

		/* Draw pack names */
		for(i = 0; i < selection; i++) {
			gfx_PrintStringXY(names[i], LCD_WIDTH * 5 / 12 - gfx_GetStringWidth(names[i]) / 2, LCD_HEIGHT / 2 - 12 - 12 * (selection - i));
		}
		gfx_SetTextScale(2, 2);
		width = gfx_GetStringWidth(names[selection]);
		gfx_PrintStringXY(names[selection], LCD_WIDTH * 5 / 12 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT);
		gfx_SetTextScale(1, 1);
		for(i = (uint24_t) (selection + 1); i < num_names; i++) {
			gfx_PrintStringXY(names[i], LCD_WIDTH * 5 / 12 - gfx_GetStringWidth(names[i]) / 2, LCD_HEIGHT / 2 + 4 + 12 * (i - selection));
		}
		/* CSS ain't got nothin' on this. */

		/* Draw some pretty lines */
		gfx_SetColor(WHITE);
		gfx_HorizLine(LCD_WIDTH * 5 / 12 - 8 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT - 4, width + 14);
		gfx_HorizLine(LCD_WIDTH * 5 / 12 - 8 - width / 2, LCD_HEIGHT / 2 + TEXT_HEIGHT + 2, width + 14);

		gfx_BlitBuffer();
	}
}

void credits(void) {
	uint24_t i = 0;
	const char *lines[] = {
		"Made by commandblockguy",
		"",
		"Available on Cemetech.net",
		"https://discord.gg/DZbmraw",
		"",
		"Thanks to MateoC for creating the C toolchain",
		"and putting up with my stupidity.",
		"",
		"Tileset inspired by GNOME Mahjongg",
		"",
		"Source: https://git.io/fjBGS",
		"",
		"",
		"Thank you for playing."
	};

	gfx_FillScreen(BACKGROUND_COLOR);

	gfx_SetTextFGColor(WHITE);
	gfx_SetTextScale(3, 3);

	gfx_PrintStringXY("Credits", (LCD_WIDTH - gfx_GetStringWidth("Credits")) / 2, 4);

	gfx_SetTextScale(1, 1);

	/* Iterate through all lines */
	for(i = 0; i < (uint24_t)(sizeof(lines) / sizeof(lines[0])); i++) {
		gfx_PrintStringXY(lines[i], 6, 32 + TEXT_HEIGHT + 12 * i);
	}

	/* the most important part, let's not leave it out */
	gfx_BlitBuffer();

	while(kb_Data[1]) kb_Scan();
	while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
}

void how_to(void) {
	uint24_t i = 0;
	const char *lines[] = {
		"Controls:",
		"Use the arrow keys to control the cursor",
		"Press 2nd to select a tile",
		"Press alpha / XT0n to undo / redo",
		"Press stat to open / close the magnifier",
		"Press clear to save and exit the game",
		"Press del to exit without saving",
		"Press mode to pause the game",
		"",
		"Rules:",
		"The goal is to remove all tiles by selecting",
		"pairs of matching tiles.",
		"You cannot remove a tile if there is a tile",
		"on top of it, or if both long sides of the",
		"tile are blocked.",
		"When you can no longer remove any pairs,",
		"the game is over."
	};

	gfx_FillScreen(BACKGROUND_COLOR);

	gfx_SetTextFGColor(WHITE);
	gfx_SetTextScale(1, 1);

	gfx_PrintStringXY("How-To", (LCD_WIDTH - gfx_GetStringWidth("How-To")) / 2, 4);

	/* Iterate through all lines */
	for(i = 0; i < (uint24_t)(sizeof(lines) / sizeof(lines[0])); i++) {
		gfx_PrintStringXY(lines[i], 6, 16 + TEXT_HEIGHT + 12 * i);
	}

	/* the most important part, let's not leave it out */
	gfx_BlitBuffer();

	while(kb_Data[1]) kb_Scan();
	while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
}

void pause(void) {
	const char *text = "game paused";

	/* Wait for the user to stop pressing mode */
	do kb_Scan(); while(kb_Data[1] & kb_Mode);

	/* Stop the timer */
	timer_Control = TIMER1_DISABLE;

	/* Hide the screen so players don't cheat */
	gfx_SetDrawScreen();
	gfx_FillScreen(BACKGROUND_COLOR);

	/* I guess the infobar isn't cheating. */
	draw_infobar();
	render_stopwatch();

	/* Print "game paused" in a OCD-satisfying location */
	gfx_SetTextFGColor(WHITE);
	gfx_PrintStringXY(text, (LCD_WIDTH - gfx_GetStringWidth(text))/2, LCD_HEIGHT / 3);

	/* Wait for the user to press mode or clear */
	do {
		kb_Scan();
	} while(!(kb_Data[6] & kb_Clear) && !(kb_Data[1] & kb_Mode));

	if(kb_Data[6] & kb_Clear) game.status = EXIT;

	/* Wait for the user to stop pressing mode */
	do kb_Scan(); while(kb_Data[1] & kb_Mode);

	/* Resume the timer */
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

	gfx_SetDrawBuffer();
}

void win_popup(void) {
	uint8_t i;
	
	/* Draw a nice little box */
	gfx_SetColor(INFOBAR_COLOR);
	gfx_FillRectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);
	gfx_SetColor(WHITE);
	gfx_Rectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);

	/* Add some text */
	gfx_SetTextFGColor(WHITE);
	gfx_SetTextScale(3, 3);
	gfx_PrintStringXY("win", (LCD_WIDTH - gfx_GetStringWidth("win")) / 2, LCD_HEIGHT / 4 + 4);
	gfx_SetTextScale(1, 1);
	gfx_HorizLine(LCD_WIDTH / 3, LCD_HEIGHT / 4 + 4 + 3 * TEXT_HEIGHT, LCD_WIDTH / 3);

	/* High score display */
	for(i = 0; i < SCORES; i++) {
		char player_name[PLAYER_NAME_LENGTH];
		uint24_t time = get_hs(game.layout.name, i, player_name);
		uint24_t minutes;

		gfx_SetTextXY(LCD_WIDTH / 3 + 4, LCD_HEIGHT / 4 + 8 + 3 * TEXT_HEIGHT + 10 * i);
		gfx_SetTextFGColor(timer_1_Counter / 33 == time ? HIGHLIGHT_SIDE_COLOR : WHITE);

		gfx_PrintUInt(i + 1, 1);
		gfx_PrintString(". ");

		if(time == -1) {
			continue;
		}

		/* Value is a time */
		minutes = time / 60000;

		/* Print the actual values */
		gfx_PrintUInt(minutes, num_digits(minutes));
		gfx_PrintString(":");
		gfx_PrintUInt((time / 1000) % 60, 2);
		gfx_PrintString(".");
		gfx_PrintUInt(time % 1000, 3);
	}

	gfx_BlitBuffer();

	/* Wait for a new keypress */
	while(kb_Data[1] || kb_Data[6] || kb_Data[7]) kb_Scan();
	while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
}

/* 0: restart level */
/* 1: undo last move */
/* 2: exit to main menu */
uint8_t lose_popup(void) {
	const char *strs[3] = {"Restart", "Undo", "Quit"};
	uint8_t i, selection = 0;
	bool key_pressed = true;
	
	while(true) {
		kb_Scan();

		/* Draw a nice little box */
		gfx_SetColor(INFOBAR_COLOR);
		gfx_FillRectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);
		gfx_SetColor(WHITE);
		gfx_Rectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);
	
		/* Add some text */
		gfx_SetTextFGColor(WHITE);
		gfx_SetTextScale(3, 3);
		gfx_PrintStringXY("lose", (LCD_WIDTH - gfx_GetStringWidth("lose")) / 2, LCD_HEIGHT / 4 + 4);
		gfx_SetTextScale(1, 1);
		gfx_HorizLine(LCD_WIDTH / 3, LCD_HEIGHT / 4 + 4 + 3 * TEXT_HEIGHT, LCD_WIDTH / 3);
	
		/* Literally the same thing as every other thing. */
		if(!key_pressed) {
			if(kb_Data[6] & kb_Clear) return 2;
			if(kb_Data[1] & kb_2nd) return selection;

			if(kb_Data[7] & kb_Up) selection--;
			if(kb_Data[7] & kb_Down) selection++;

			if(selection >= 3) selection = 0;
			if(selection == -1) selection = 2;
		}
	
		/* Go read my other comments, I'm tired of writing these. */
		key_pressed = kb_Data[1] || kb_Data[6] || kb_Data[7];
	
		for(i = 0; i < 3; i++) {
			gfx_SetTextFGColor(i == selection ? HIGHLIGHT_SIDE_COLOR : WHITE);
			gfx_PrintStringXY(strs[i], LCD_WIDTH / 3 + 4, LCD_HEIGHT / 4 + 16 + 3 * TEXT_HEIGHT + 20 * i);
		}
	
		gfx_BlitBuffer();
	}
}

enum entry_mode {
	UPPERCASE,
	LOWERCASE,
	NUMERIC
};

void enter_name(char str[PLAYER_NAME_LENGTH]) {
	uint8_t pos = 0; // the position new characters will be inserted into
	uint8_t end = 0; // the index of the null byte
	uint8_t mode = 0;
	bool key_pressed = true;
	uint8_t key_value;
	uint8_t blink_value = 0;
	uint8_t i;

	memset(str, 0, PLAYER_NAME_LENGTH);

	while(true) {
		kb_Scan();

		key_value = os_GetCSC();

		if(!key_pressed) {
			if(kb_IsDown(kb_KeyClear)) {
				memset(str, 0, PLAYER_NAME_LENGTH);
				return;
			}
			if(kb_IsDown(kb_KeyEnter)) {
				return;
			}

			if(kb_IsDown(kb_KeyRight) && pos < end) pos++;
			if(kb_IsDown(kb_KeyLeft) && pos > 0) pos--;

			if(kb_IsDown(kb_KeyDel) && pos != end) {
				memmove(&str[pos], &str[pos+1], end - pos);
				end--;
			}

			if(kb_IsDown(kb_KeyAlpha)) {
				mode++;
				if(mode > 2) mode = 0;
			}
		}

		key_pressed = false;
		for(i = 1; i <= 7; i++) {
			if(kb_Data[i]) key_pressed = true;
		}

		if(key_value && end < PLAYER_NAME_LENGTH - 1) {
			char ch;
			if(mode == NUMERIC) {
				ch = getkey_numeric[key_value];
			} else {
				ch = getkey_letters[key_value];
				if(mode == LOWERCASE && ch >= 'A' && ch <= 'Z') {
					// convert to lowercase
					ch -= ('A' - 'a');
				}
			}

			if(ch) {
				memmove(&str[pos + 1], &str[pos], end - pos + 1);
				str[pos] = ch;
				pos++;
				end++;
			}
		}

		gfx_SetColor(INFOBAR_COLOR);
		gfx_FillRectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);
		gfx_SetColor(WHITE);
		gfx_Rectangle(LCD_WIDTH / 3, LCD_HEIGHT / 4, LCD_WIDTH / 3, LCD_HEIGHT / 2);

		gfx_SetTextFGColor(WHITE);
		gfx_SetTextScale(3, 3);
		gfx_PrintStringXY("name", (LCD_WIDTH - gfx_GetStringWidth("name")) / 2, LCD_HEIGHT / 4 + 4);
		gfx_SetTextScale(1, 1);
		gfx_HorizLine(LCD_WIDTH / 3, LCD_HEIGHT / 4 + 4 + 3 * TEXT_HEIGHT, LCD_WIDTH / 3);

		for(i = 0; i < PLAYER_NAME_LENGTH - 1; i++) {
#define SLOT_WIDTH 8
#define SLOT_GAP 3
#define BASE_X ((LCD_WIDTH - (SLOT_WIDTH * (PLAYER_NAME_LENGTH - 1) + SLOT_GAP * (PLAYER_NAME_LENGTH - 2))) / 2)
#define TEXT_Y ((LCD_HEIGHT - TEXT_HEIGHT) / 2)
#define LINE_Y (TEXT_Y + TEXT_HEIGHT)
			gfx_SetColor(SIDE_COLOR);
			if(i < end) {
				gfx_SetTextXY(BASE_X + i * (SLOT_WIDTH + SLOT_GAP), TEXT_Y);
				gfx_PrintChar(str[i]);
			}
			if(i == pos) {
				gfx_SetTextXY(BASE_X + i * (SLOT_WIDTH + SLOT_GAP), TEXT_Y + TEXT_HEIGHT + 3);
				switch(mode) {
					default:
					case UPPERCASE:
						gfx_PrintChar('A');
						break;
					case LOWERCASE:
						gfx_PrintChar('a');
						break;
					case NUMERIC:
						gfx_PrintChar('1');
						break;
				}
				if (blink_value & 32) {
					gfx_SetColor(WHITE);
				}
			}
			gfx_HorizLine(BASE_X + i * (SLOT_WIDTH + SLOT_GAP), LINE_Y, SLOT_WIDTH);
		}

		blink_value++;
		gfx_BlitBuffer();
	}
}
