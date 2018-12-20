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
#include "layouts.h"

#include <debug.h>

const char *hs_appvar = "MJHigh";
const char *save_appvar = "MJSave";

typedef struct {
	char name[NAME_LENGTH];
	uint24_t times[SCORES];
} score_entry_t;

void init_hs_appvar(void) {
	ti_var_t appvar;

	/* Reasons??? */
	ti_CloseAll();

	/* Try to open */
	appvar = ti_Open(hs_appvar, "r");

	if(appvar) {
		/* File exists. No action must be taken. */
		ti_CloseAll();

		return;
	}

	/* Create a new highscore appvar */
	appvar = ti_Open(hs_appvar, "w");

	/* Write a 0 (the number of high scores) */
	ti_PutC(0, appvar);
}

/* Time in timer units, name of layout */
void add_hs(uint24_t time, char* name) {
	ti_var_t appvar;
	uint8_t num_layouts, i;
	score_entry_t scores;

	ti_CloseAll();

	/* Open the highscore appvar */
	appvar = ti_Open(hs_appvar, "r+");

	/* Get the number of layouts with highscores */
	num_layouts = ti_GetC(appvar);

	/* Loop through all the high scores */
	for(i = 0; i < num_layouts; i++) {
		ti_Read(&scores, sizeof(score_entry_t), 1, appvar);
		if(strcmp(scores.name, name) == 0) {
			int rank;
			/* Strings are the same */

			/* Rewind the appvar */
			ti_Seek(-sizeof(score_entry_t), SEEK_CUR, appvar);

			/* Sort the time into the times */
			for(rank = SCORES - 1; (rank >= 0  && scores.times[rank] > time); rank--) {
				if(rank < SCORES) scores.times[rank + 1] = scores.times[rank]; 
			}

			if(rank + 1 < SCORES)
				scores.times[rank + 1] = time;

			ti_Write(&scores, sizeof(score_entry_t), 1, appvar);

			ti_SetArchiveStatus(true, appvar);

			ti_CloseAll();

			return;
		}
	}

	/* No high scores were found with that name, make a new entry */

	/* Add one to the number of entries */
	ti_Rewind(appvar);
	i = ti_GetC(appvar);
	i++;
	ti_Rewind(appvar);
	ti_PutC(i, appvar);

	/* Initialize usused times to -1 ms */
	strcpy(scores.name, name);
	scores.times[0] = time;
	for(i = 1; i < SCORES; i++)
		scores.times[i] = -1;

	/* Add the scores the end of the appvar */
	ti_Seek(0, SEEK_END, appvar);
	ti_Write(&scores, sizeof(score_entry_t), 1, appvar);

}

/* Rank is number of scores with a lower time */
/* eg. best time is 0, worst is 7 */
uint24_t get_hs(char* name, uint8_t rank) {
	ti_var_t appvar;
	uint8_t num_layouts, i;
	score_entry_t scores;

	ti_CloseAll();

	/* Open the highscore appvar */
	appvar = ti_Open(hs_appvar, "r");

	/* Get the number of layouts with highscores */
	num_layouts = ti_GetC(appvar);

	for(i = 0; i < num_layouts; i++) {
		ti_Read(&scores, sizeof(score_entry_t), 1, appvar);
		if(strcmp(scores.name, name) == 0) {
			/* Strings are the same */
			ti_CloseAll();
			return scores.times[rank];
		}
	}

	/* No score has been found */
	return -1;
}

/* Stores the first max pack names in dest, then returns the number of packs found */
uint8_t get_packs(char dest[][9], uint8_t max) {
	uint8_t found;
	uint8_t *search_pos = NULL;
	char *var_name;

	ti_CloseAll();

	for(found = 0; found < max; found++) {
		var_name = ti_Detect(&search_pos, "MJ");
		if(var_name == NULL) return found;
		strcpy(dest[found], var_name);
	}
	return found;
}

/* Store the level that was last played */
void set_last_level(char *pack, char *level) {
	ti_var_t var;

	ti_CloseAll();
	var = ti_Open(save_appvar, "w+");
	/* Write the magic number 255 to the save */
	ti_PutC(255, var);
	/* Write pack and level name */
	ti_Write(pack, sizeof(char), 9, var);
	ti_Write(level, sizeof(char), NAME_LENGTH, var);

	ti_SetArchiveStatus(true, var);

	ti_CloseAll();
}

/* Saves the game so it can be restored later */
/* Timer is game time in ms */
void save_game(uint24_t timer) {
	ti_var_t var;

	/* Open the appvar */
	ti_CloseAll();
	var = ti_Open(save_appvar, "w+");

	/* Write the game global into it */
	ti_Write(&game, sizeof(game_t), 1, var);
	/* Write the time in ms */
	ti_Write(&timer, sizeof(uint24_t), 1, var);

	ti_SetArchiveStatus(true, var);

	ti_CloseAll();
}

/* Returns time in ms if the save appvar has a game in progress */
uint24_t read_save(void) {
	ti_var_t var;

	/* Try to open the save appvar */
	ti_CloseAll();
	var = ti_Open(save_appvar, "r");

	/* If it fails, set pack and level to "" and return false */
	if(!var) {
		strcpy(game.pack_name, "");
		strcpy(game.layout.name, "");

		ti_CloseAll();
		return 0;
	}

	/* Check if the first byte is 255 */
	if(ti_GetC(var) == 255) {
		/* If so, read the level and pack names */
		ti_Read(game.pack_name, sizeof(char), 9, var);
		ti_Read(game.layout.name, sizeof(char), NAME_LENGTH, var);
		return 0;
	} else {
		uint24_t timer;
		/* Otherwise, load the entire appvar into the game global */
		ti_Rewind(var);
		ti_Read(&game, sizeof(game_t), 1, var);
		/* Return the timer value */
		ti_Read(&timer, sizeof(uint24_t), 1, var);
		return timer;
	}
}
