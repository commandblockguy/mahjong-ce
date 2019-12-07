#ifndef H_STORAGE
#define H_STORAGE

#define SCORES 8
#define PLAYER_NAME_LENGTH 10

#define CURRENT_HS_VERSION 1

typedef struct {
	char name[LEVEL_NAME_LENGTH];
	uint24_t times[SCORES];
} legacy_score_entry_t;

typedef struct {
	char name[LEVEL_NAME_LENGTH];
	uint24_t times[SCORES];
	char players[SCORES][PLAYER_NAME_LENGTH];
} score_entry_t;

typedef struct {
	uint24_t num_layouts;
	uint8_t version;
} score_header_t;

void init_hs_appvar(void);
void add_hs(uint24_t time, char *name, char* player);
uint24_t get_hs(char* name, uint8_t rank, char* player);
uint8_t get_packs(char dest[][9], uint8_t max);
void set_last_level(char *pack, char *level);
void save_game(uint24_t timer);
uint24_t read_save(void);

#endif