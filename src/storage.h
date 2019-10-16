#ifndef H_STORAGE
#define H_STORAGE

#define SCORES 8

typedef struct {
	char name[NAME_LENGTH];
	uint24_t times[SCORES];
} score_entry_t;

void init_hs_appvar(void);
void add_hs(uint24_t time, char *name);
uint24_t get_hs(char* name, uint8_t rank);
uint8_t get_packs(char dest[][9], uint8_t max);
void set_last_level(char *pack, char *level);
void save_game(uint24_t timer);
uint24_t read_save(void);

#endif