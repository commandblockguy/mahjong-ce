#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include <debug.h>

#include "game.h"
#include "gfx/tiles_gfx.h"

/* One pixel padding, plus one pixel thick line */
#define BUTTON_HEIGHT TEXT_HEIGHT + 4

#define MM_TILES_Y ((LCD_HEIGHT / TILE_HEIGHT) + 2)
#define MM_SIDE_WIDTH 25

#define PACKS_MENU_LINES 30

uint8_t main_menu() {
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
			/* Roatating queue thing */
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
					if(selection == i) {
						gfx_SetTextFGColor(HIGHLIGHT_SIDE_COLOR);
					} else {
						gfx_SetTextFGColor(WHITE);
					}

					gfx_PrintStringXY(mm_strings[i],
						(LCD_WIDTH - gfx_GetStringWidth(mm_strings[i])) / 2,
						(LCD_HEIGHT + 3 * TEXT_HEIGHT) / 2 + 2 * i * TEXT_HEIGHT);

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
					uint24_t base_y = LCD_HEIGHT / 6;

					/* Fill in the tile background */
					gfx_SetColor(WHITE);
					gfx_FillRectangle(base_x, base_y, 2 * TILE_WIDTH, 2 * TILE_HEIGHT);
					
					/* Draw letter */
					gfx_SetTextXY(base_x + (TILE_WIDTH - gfx_GetCharWidth(str[i]) / 2), base_y + TILE_HEIGHT - TEXT_HEIGHT);
					gfx_PrintChar(str[i]);

					/* Draw the tile border. */
					gfx_SetColor(SIDE_COLOR);
					gfx_HorizLine(base_x, base_y, 2 * TILE_WIDTH);
					gfx_VertLine(base_x + 2 * TILE_WIDTH - 1, base_y, 2 * TILE_HEIGHT - 1);
					gfx_VertLine(base_x, base_y, 2 * TILE_HEIGHT - 1);

					/* Display the left side of the tile */
					for(j = 1; j <= 4; j++)
						gfx_VertLine(base_x - j, base_y + j, 2 * TILE_HEIGHT - 1);

					/* Display the bottom side of the tile */
					gfx_SetColor(BOTTOM_COLOR);
					for(j = 0; j < 6; j++)
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
uint8_t layouts_menu() {

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
		if(!meta.version > 1) {
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
			const char *chars = "\0\0\0\0\0\0\0\0\0\0\0WRMH\0\0\0\0VQLG\0\0\0ZUPKFC\0\0YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
			int i;
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
			if(chars[key]) {
				char str[2] = {0};
				*str = chars[key];

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

			/* Draw the margin and highschore inset */
			gfx_SetColor(BOTTOM_COLOR);
			gfx_FillRectangle(0, 0, 10, LCD_HEIGHT);
			gfx_FillRectangle(LCD_WIDTH / 2, LCD_HEIGHT / 2, LCD_WIDTH / 2, LCD_HEIGHT / 2);

			/* Draw the border of the layout view */
			gfx_VertLine(LCD_WIDTH / 2, 0, LCD_HEIGHT / 2);

			/* Draw the layout view */
			/* This probably needs to be fast, so no isometric awesomeness. */
			/* Loop through each slot */
			/* Use 6 by 10 tiles, or a 3, 5 offset ammount per slot coord */
			gfx_SetColor(WHITE);
			for(i = 0; i < TILE_TYPES * 4; i++) {
				gfx_FillRectangle(LCD_WIDTH * 3 / 4 - TILES_X * 3 + 3 * layouts[selection].slots[i].x,
					LCD_HEIGHT / 4 - TILES_Y * 5 + 5 * layouts[selection].slots[i].y, 6, 10);
			}

			/* Draw the layout names */
			gfx_SetTextScale(1, 1);
			for(i = selection >= 10 ? selection - 10 : 0; i < selection + 10 && i < meta.num_layouts; i++) {
				width = gfx_GetStringWidth(&layouts[i].name);
				gfx_SetTextFGColor(i == selection ? WHITE : SIDE_COLOR);
				gfx_PrintStringXY(&layouts[i].name, (LCD_WIDTH / 2 + 5 - width) / 2, (LCD_HEIGHT - TEXT_HEIGHT) / 2 - 12 * (selection - i));
			}

			width = gfx_GetStringWidth(&layouts[selection].name);

			/* Draw some pretty lines */
			gfx_SetColor(WHITE);
			gfx_HorizLine((LCD_WIDTH / 2 + 5) / 2 - 4 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT / 2 - 2, width + 7);
			gfx_HorizLine((LCD_WIDTH / 2 + 5) / 2 - 4 - width / 2, LCD_HEIGHT / 2 + TEXT_HEIGHT / 2 + 1, width + 7);

			/* High score text */
			gfx_SetTextFGColor(WHITE);
			gfx_PrintStringXY(str_hs, LCD_WIDTH * 3 / 4 - gfx_GetStringWidth(str_hs) / 2, LCD_HEIGHT / 2 + 2);

			/* Actual high scores */
			for(i = 0; i < SCORES; i++) {
				uint24_t time = get_hs(&layouts[selection].name, i);
				uint24_t minutes;

				gfx_SetTextXY(LCD_WIDTH / 2 + 8, LCD_HEIGHT / 2 + 16 + 12 * i);

				gfx_PrintUInt(i + 1, 1);
				gfx_PrintString(". ");

				if(time == -1) {
					continue;
				}

				minutes = time / 60000;

				/* Print the actual values */
				gfx_PrintUInt(minutes, num_digits(minutes));
				gfx_PrintString(":");
				gfx_PrintUInt((time / 1000) % 60, 2);
				gfx_PrintString(".");
				gfx_PrintUInt(time % 1000, 3);
			}

			gfx_BlitBuffer();
		}
	}
}

/* Returns the name of the selected pack */
void packs_menu() {
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
		uint8_t i, width;
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
			gfx_PrintStringXY(&names[i], LCD_WIDTH * 5 / 12 - gfx_GetStringWidth(&names[i]) / 2, LCD_HEIGHT / 2 - 12 - 12 * (selection - i));
		}
		gfx_SetTextScale(2, 2);
		width = gfx_GetStringWidth(&names[selection]);
		gfx_PrintStringXY(&names[selection], LCD_WIDTH * 5 / 12 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT);
		gfx_SetTextScale(1, 1);
		for(i = selection + 1; i < num_names; i++) {
			gfx_PrintStringXY(&names[i], LCD_WIDTH * 5 / 12 - gfx_GetStringWidth(&names[i]) / 2, LCD_HEIGHT / 2 + 4 + 12 * (i - selection));
		}
		/* CSS ain't got nothin' on this. */

		/* Draw some pretty lines */
		gfx_SetColor(WHITE);
		gfx_HorizLine(LCD_WIDTH * 5 / 12 - 8 - width / 2, LCD_HEIGHT / 2 - TEXT_HEIGHT - 4, width + 14);
		gfx_HorizLine(LCD_WIDTH * 5 / 12 - 8 - width / 2, LCD_HEIGHT / 2 + TEXT_HEIGHT + 2, width + 14);

		gfx_BlitBuffer();
	}
}

void credits() {
	int i = 0;
	const char *lines[] = {
		"Made by commandblockguy",
		"",
		"Avaliable on Cemetech.net",
		"",
		"Thanks to MateoC for creating the C toolchain",
		"and putting up with my stupidity.",
		"",
		"Tileset inspired by GNOME Mahjongg",
		"",
		"Source: github.com/commandblockguy/mahjong-ce",
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
	for(i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
		gfx_PrintStringXY(lines[i], 6, 32 + TEXT_HEIGHT + 12 * i);
	}

	/* the most important part, let's not leave it out */
	gfx_BlitBuffer();

	while(kb_Data[1]) kb_Scan();
	while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
}

void how_to() {
	int i = 0;
	const char *lines[] = {
		"Controls:",
		"Use the arrow keys to control the cursor",
		"Press 2nd to select a tile",
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
	for(i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
		gfx_PrintStringXY(lines[i], 6, 16 + TEXT_HEIGHT + 12 * i);
	}

	/* the most important part, let's not leave it out */
	gfx_BlitBuffer();

	while(kb_Data[1]) kb_Scan();
	while(!(kb_Data[1] || kb_Data[6] || kb_Data[7])) kb_Scan();
}

void how_to_page(uint8_t page) {

}

void pause() {
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
		uint24_t time = get_hs(game.layout.name, i);
		uint24_t minutes;

		gfx_SetTextXY(LCD_WIDTH / 3 + 4, LCD_HEIGHT / 4 + 8 + 3 * TEXT_HEIGHT + 10 * i);
		gfx_SetTextFGColor(timer_1_Counter / 33 == time ? HIGHLIGHT_SIDE_COLOR : WHITE);

		gfx_PrintUInt(i + 1, 1);
		gfx_PrintString(". ");

		if(time == -1) {
			continue;
		}

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
			uint24_t time = get_hs(game.layout.name, i);
			uint24_t minutes;
	
			gfx_SetTextFGColor(i == selection ? HIGHLIGHT_SIDE_COLOR : WHITE);
			gfx_PrintStringXY(strs[i], LCD_WIDTH / 3 + 4, LCD_HEIGHT / 4 + 16 + 3 * TEXT_HEIGHT + 20 * i);
		}
	
		gfx_BlitBuffer();
	}
}
