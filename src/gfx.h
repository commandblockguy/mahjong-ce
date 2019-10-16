#ifndef H_GFX
#define H_GFX

/* Dimensions of the face of a tile in pixels */
#define TILE_WIDTH  14
#define TILE_HEIGHT 21
/* Width in pixels of the left edge of a tile when applying isometric effects */
#define SHIFT_X 2
/* Height in pixels of the bottom edge of a tile when applying isometric effects */
#define SHIFT_Y 2

/* Colors */
#define BACKGROUND_COLOR 1
#define BOTTOM_COLOR 2
#define SIDE_COLOR 3
#define HIGHLIGHT_BOTTOM_COLOR 4
#define HIGHLIGHT_SIDE_COLOR 5
#define HIGHLIGHT_FACE_COLOR 6
#define BLACK 7
#define WHITE 0
#define INFOBAR_COLOR 9
#define CURSOR_COLOR 10

#define INFOBAR_HEIGHT 12
#define STOPWATCH_WIDTH 64

/* Speed of the cursor in pixels/loop */
#define CURSOR_SPEED 2

/* Size of the region around the cursor used by the magnifier, prior to magnification */
#define MAGNIFIER_X 32
#define MAGNIFIER_Y 32
/* Scaling factor the magnifier should use */
#define MAGNIFIER_SCALE 3

#define TEXT_HEIGHT 8

void rerender(void);
void draw_infobar(void);
void render_tiles(void);
void render_tile(uint8_t x, uint8_t y, uint8_t z, bool highlight);
void render_raw_tile(int24_t base_x, int24_t base_y, uint8_t type, bool highlight);
void draw_cursor(uint24_t x, uint8_t y);
void render_stopwatch(void);

uint24_t tile_base_x(uint8_t x, uint8_t y, uint8_t z);
uint8_t tile_base_y(uint8_t x, uint8_t y, uint8_t z);

void draw_magnifier(uint24_t csrX, uint8_t csrY);
void render_tile_info(tile_t tile);

#endif