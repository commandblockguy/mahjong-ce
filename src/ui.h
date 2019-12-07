#ifndef H_UI
#define H_UI

#include "storage.h"

enum MM_Options {
	MM_RESUME,
	MM_NEW_GAME,
	MM_HOWTO,
	MM_CREDITS,
	MM_EXIT
};

uint8_t main_menu(void);
uint8_t layouts_menu(void);
void packs_menu(void);
void credits(void);
void how_to(void);
void pause(void);
void win_popup(void);
uint8_t lose_popup(void);
void enter_name(char str[PLAYER_NAME_LENGTH]);

#endif