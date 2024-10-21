#define FALSE 0
#define TRUE 1

#define WINDOW_SCALE 3

#define WINDOW_WIDTH (320 * WINDOW_SCALE)
#define WINDOW_HEIGHT (200 * WINDOW_SCALE)

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define GRID_SPACING 64

typedef struct rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb;

#define C_BLACK (rgb) {0, 0, 0}
#define C_WHITE (rgb) {255, 255, 255}
#define C_YELLOW (rgb) {255, 255, 0}
#define C_RED (rgb) {255, 0, 0}
#define C_PURPLE (rgb) {255, 0, 255}
#define C_BLUE (rgb) {0, 0, 255}