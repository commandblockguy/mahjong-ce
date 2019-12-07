#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <fileioc.h>

#include <debug.h>

#include "game.h"
#include "storage.h"
#include "layouts.h"


const char *hs_appvar = "MJScores";
const char *hs_legacy_appvar = "MJHigh";
const char *save_appvar = "MJSave";

/* Set up the high scores appvar if it does not exist */
void init_hs_appvar(void) {
	ti_var_t appvar, legacy_var;
	score_header_t header;

	/* Reasons??? */
	ti_CloseAll();

	/* Try to open */
	appvar = ti_Open(hs_appvar, "r");

	if(appvar) {
		/* If appvar exists, we are good to go */
		ti_Close(appvar);

		return;
	}

	/* Try to open legacy score appvar */
	legacy_var = ti_Open(hs_legacy_appvar, "r");
	if(legacy_var) {
		uint24_t i;
		uint8_t num_layouts;
		score_entry_t scores;
		appvar = ti_Open(hs_appvar, "w");

		/* Get the number of layouts */
		num_layouts = ti_GetC(legacy_var);

		dbg_sprintf(dbgout, "found a legacy appvar with %u layouts\n", num_layouts);

		/* Write the header for the new appvar */
		header.num_layouts = num_layouts;
		header.version = CURRENT_HS_VERSION;

		ti_Write(&header, sizeof(header), 1, appvar);

		memset(&scores.players, 0, sizeof(scores.players));

		/* Copy each layout's scores over */
		for(i = 0; i < num_layouts; i++) {
			/* Copy the old scores into the new score struct */
			ti_Read(&scores, sizeof(legacy_score_entry_t), 1, legacy_var);

			/* Write the new scores */
			ti_Write(&scores, sizeof(score_entry_t), 1, appvar);
		}

		/* Delete the old var */
		ti_Close(legacy_var);
		ti_Delete(hs_legacy_appvar);
		ti_Close(appvar);

		return;
	}

	/* Create a new highscore appvar */
	appvar = ti_Open(hs_appvar, "w");

	/* Write a 0 (the number of high scores) */
	header.num_layouts = 0;
	header.version = CURRENT_HS_VERSION;
	ti_Write(&header, sizeof(header), 1, appvar);
	ti_Close(appvar);
}

/* Add a high score to the list */
/* Time in timer units, name of layout */
void add_hs(uint24_t time, char* name, char* player) {
	ti_var_t appvar;
	uint24_t i;
	score_entry_t scores;
	score_header_t header;

	ti_CloseAll();

	/* Open the highscore appvar */
	appvar = ti_Open(hs_appvar, "r+");

	/* Get the header */
	ti_Read(&header, sizeof(header), 1, appvar);

	/* Return if an unsupported version */
	if(header.version != CURRENT_HS_VERSION) {
		return;
	}

	/* Loop through all the high scores */
	for(i = 0; i < header.num_layouts; i++) {
		ti_Read(&scores, sizeof(score_entry_t), 1, appvar);
		if(strcmp(scores.name, name) == 0) {
			uint8_t rank;
			/* Strings are the same */

			/* Rewind the appvar */
			ti_Seek((int)-sizeof(score_entry_t), SEEK_CUR, appvar);

			/* Sort the time into the times */
			for(rank = SCORES - 1; rank >= 0  && scores.times[rank] > time; rank--) {
				if(rank + 1 < SCORES) {
					scores.times[rank + 1] = scores.times[rank];
					memcpy(scores.players[rank + 1], scores.players[rank], sizeof(scores.players[0]));
				}
			}
			rank++;

			if(rank < SCORES) {
				scores.times[rank] = time;
				memcpy(scores.players[rank], player, sizeof(scores.players[0]));
			}

			ti_Write(&scores, sizeof(score_entry_t), 1, appvar);

			ti_SetArchiveStatus(true, appvar);

			ti_CloseAll();

			return;
		}
	}

	/* No high scores were found with that name, make a new entry */

	dbg_sprintf(dbgout, "Creating new entry\n");

	/* Add one to the number of entries */
	ti_Rewind(appvar);
	header.num_layouts++;
	ti_Write(&header, sizeof(header), 1, appvar);

	strcpy(scores.name, name);
	scores.times[0] = time;
	strcpy(scores.players[0], player);

	/* Initialize unused times to -1 ms */
	for(i = 1; i < SCORES; i++) {
		scores.times[i] = (uint24_t) -1;
	}

	/* Add the scores the end of the appvar */
	ti_Seek(0, SEEK_END, appvar);
	ti_Write(&scores, sizeof(score_entry_t), 1, appvar);

	ti_SetArchiveStatus(appvar, true);

	ti_Close(appvar);

}

/* Rank is number of scores with a lower time */
/* eg. best time is 0, worst is 7 */
/* Player name is copied into player */
uint24_t get_hs(char* name, uint8_t rank, char *player) {
	ti_var_t appvar;
	uint24_t i;
	score_entry_t scores;
	score_header_t header;

	ti_CloseAll();

	/* Open the highscore appvar */
	appvar = ti_Open(hs_appvar, "r");

	/* Get the header */
	ti_Read(&header, sizeof(header), 1, appvar);

	/* Return if an unsupported version */
	if(header.version != CURRENT_HS_VERSION) {
		*player = 0;
		return (uint24_t) -1;
	}

	for(i = 0; i < header.num_layouts; i++) {
		ti_Read(&scores, sizeof(score_entry_t), 1, appvar);
		if(strcmp(scores.name, name) == 0) {
			/* Strings are the same */
			ti_CloseAll();

			/* Copy the player name */
			if(player) {
				memcpy(player, scores.players[rank], sizeof(scores.players[rank]));
			}
			return scores.times[rank];
		}
	}

	/* No score has been found */
	return (uint24_t) -1;
}

/* Stores the first max pack names in dest, then returns the number of packs found */
uint8_t get_packs(char dest[][9], uint8_t max) {
	uint8_t found;
	uint8_t *search_pos = NULL;
	char *var_name;

	ti_CloseAll();

	for(found = 0; found < max; found++) {
		var_name = ti_Detect((void **) &search_pos, "MJ");
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
	ti_PutC((char)255, var);
	/* Write pack and level name */
	ti_Write(pack, sizeof(char), 9, var);
	ti_Write(level, sizeof(char), LEVEL_NAME_LENGTH, var);

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

/* Reads and loads a save */
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
		ti_Read(game.layout.name, sizeof(char), LEVEL_NAME_LENGTH, var);
		ti_Close(var);
		return 0;
	} else {
		uint24_t timer;
		/* Otherwise, load the entire appvar into the game global */
		ti_Rewind(var);
		ti_Read(&game, sizeof(game_t), 1, var);
		/* Return the timer value */
		ti_Read(&timer, sizeof(uint24_t), 1, var);
		ti_Close(var);
		return timer;
	}
}
