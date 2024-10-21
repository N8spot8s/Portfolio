#include <SDL2/SDL.h>
#include <math.h>
#include <string.h>
#include "./constants.h"
#include "./grid.h"



void setup(void);
void process_input(void);
void update(void);
void render(void);
void free_memory(void);

float perc(int percent);
void draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b);
void draw_rect_rgb(int x, int y, int length, int width, rgb color);
void draw_point(int x, int y, int radius, unsigned char r, unsigned char g, unsigned char b);
void draw_point_rgb(int x, int y, int radius, rgb color);
void print_debug(void);
void calc_grid_cam_center(void);
void calc_grid_cam_zoom_p(void);
void reset_grid_cam(void);
void reset_player(void);
void zoom_grid_cam(int zoom);
void zoom_grid_cam_center(int zoom);
void g_draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b);
void g_draw_rect_rgb(int x, int y, int length, int width, rgb color);
void g_draw_point(int x, int y, int radius, unsigned char r, unsigned char g, unsigned char b);
void g_draw_point_rgb(int x, int y, int radius, rgb color);
int menu_selection(char* title, char options[11][26]);
void debug_menu(void);
void draw_rect_bordered(int x, int y, int length, int width, unsigned char fill_r, unsigned char fill_g, unsigned char fill_b,
    unsigned char border_r, unsigned char border_g, unsigned char border_b);
void draw_rect_bordered_rgb(int x, int y, int length, int width, rgb fill, rgb border);
void rotate_player(float angle);
void push_player_forward(int force);
void push_player_right(int force);
void add_fill_dgp(int x, int y, rgb color);
int rad_deg(float radians);
void add_temp_dgp(int x, int y, rgb color);
int max(int val1, int val2);
int min(int val1, int val2);
int bounds(int minVal, int val, int maxVal);
void print_rgb(rgb color);



int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time = 0;

int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return FALSE;
    }
    window = SDL_CreateWindow(
        "Sick Raycasting", //Title
        SDL_WINDOWPOS_CENTERED, //X
        SDL_WINDOWPOS_CENTERED, //Y
        WINDOW_WIDTH, //Width
        WINDOW_HEIGHT, //Height
        0 //Flags
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0); //Window, display driver, flags
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return FALSE;
    }

    return TRUE;
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/* Map Variables */

/* Physical Grid */
int grid_length;
int grid_height;
bool_cont* grid_enc;
/* Grid Visual */
rgb grid_bg = {255, 0, 255};
rgb grid_fill_nonsolid = {160, 195, 115};
rgb grid_fill_solid = {100, 110, 100};
int grid_line_width = 1;
rgb grid_line_fill = C_BLACK;
int show_grid_lines = FALSE;
/* Grid Visual Camera */
int grid_cam_x;
int grid_cam_y;
int grid_cam_zoom;
float grid_cam_zoom_p;
int grid_cam_zoom_min;
int grid_cam_zoom_max;
int grid_cam_center_x;
int grid_cam_center_y;
/* Player */
float player_x;
int i_player_x;
float player_y;
int i_player_y;
int player_radius = 10;
float player_x_velocity;
float player_y_velocity;
float player_velocity_angle;
int player_max_velocity = 300;
float player_angle;
const float FOV = M_PI / 3;
int player_movement_accel = 20;
int player_movement_decel = 20;
float player_angle_increment = M_PI / 18;
int player_rotation_speed = 10;

void assign_i_player_pos(void) {
    i_player_x = round(player_x);
    i_player_y = round(player_y);
}
/* Player Grid Visual */
int grid_player_pointer_dist = 15;
int grid_player_pointer_radius_offset = 50;
rgb grid_player_fill = {255, 50, 50};
int grid_follow_player = TRUE;
int show_player_vision = FALSE;
int show_player_trail = FALSE;

int render_in_first_person = FALSE;

/* First Person Rendering */
rgb fp_bg_top = {255, 0, 255};
rgb fp_bg_bottom = {160, 195, 115};
rgb wall_color = {150, 150, 150};
int fp_brightness = 100;
float fp_scale = 0.02f;
unsigned int fp_render_distance = 1000;
unsigned int fp_render_distance_scr = (WINDOW_HEIGHT / 2) + 150;
int fp_show_walls = TRUE;

/* Debug Vaiable Labels */
struct int_varlabel {
    char* label;
    int* value;
};

struct flt_varlabel {
    char* label;
    float* value;
};

char int_vls_menu[11][26];
char flt_vls_menu[11][26];

#define INT_VLS_LEN 9
struct int_varlabel int_vls[INT_VLS_LEN];

#define FLT_VLS_LEN 3
struct flt_varlabel flt_vls[FLT_VLS_LEN];

/* Debug Menu */
char TOP_MENU[11][26] = {
    "Set int variables",
    "Set float variables",
    "See all variables",
    "Advance one frame",
    "Switch viewpoints",
    "Reset player",
    "Close program",
    "end"
};

int open_debug_menu = FALSE;
int debug_menu_was_open = FALSE;
int reopen_debug_menu = FALSE;
int show_grid_crosshairs = FALSE;

struct debug_grid_point {
    int x;
    int y;
    rgb color;
    struct debug_grid_point* next;
};

int dgp_radius = 5;
int num_fill_dgps = 0;
int max_fill_dgps = FPS / 2;
struct debug_grid_point* fill_dgp_head;
struct debug_grid_point* fill_dgp_tail;
struct debug_grid_point* temp_dgp_head;
struct debug_grid_point* temp_dgp_tail;

/* User Input Variables */
int shift = FALSE;
int left_mouse_down = FALSE;
int prev_mouse_x = -1;
int prev_mouse_y = -1;
char horizontal_input;
char vertical_input;
char rotation_input = 0;


/* Color */
rgb brighten(rgb color, int offset) {
    int new_r = color.r;
    int new_g = color.g;
    int new_b = color.b;
    new_r += offset; new_g += offset; new_b += offset;
    new_r = bounds(0, new_r, 255);
    new_g = bounds(0, new_g, 255);
    new_b = bounds(0, new_b, 255);
    return (rgb) {new_r, new_g, new_b};
}

/* Graphics */
void draw_rect_a(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    SDL_Rect rect = {x, y, length, width};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_rect_a_rgb(int x, int y, int length, int width, rgb color, int a) {
    draw_rect_a(x, y, length, width, color.r, color.g, color.b, a);
}

void draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b) {
    draw_rect_a(x, y, length, width, r, g, b, 255);
}

void set_draw_color_rgb(rgb color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}

void draw_rect_rgb(int x, int y, int length, int width, rgb color) {
    draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void draw_rect_bordered(int x, int y, int length, int width, unsigned char fill_r, unsigned char fill_g, unsigned char fill_b,
    unsigned char border_r, unsigned char border_g, unsigned char border_b) {
    SDL_Rect rect = {x, y, length, width};
    SDL_SetRenderDrawColor(renderer, fill_r, fill_g, fill_b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, border_r, border_g, border_b, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void draw_rect_bordered_rgb(int x, int y, int length, int width, rgb fill, rgb border) {
    draw_rect_bordered(x, y, length, width, fill.r, fill.g, fill.b, border.r, border.g, border.b);
}

void draw_point(int x, int y, int radius, unsigned char r, unsigned char g, unsigned char b) {
    draw_rect_bordered(x - radius, y - radius, radius * 2, radius * 2, r, g, b, 0, 0, 0);
}

void draw_point_rgb(int x, int y, int radius, rgb color) {
    draw_point(x, y, radius, color.r, color.g, color.b);
}

void vertical_gradient(int x, int y, int length, int width, rgb top_color, rgb bottom_color) {
    float c_r = (float) (top_color.r - bottom_color.r) / -width;
    float c_g = (float) (top_color.g - bottom_color.g) / -width;
    float c_b = (float) (top_color.b - bottom_color.b) / -width;

    float grad_r = top_color.r;
    float grad_g = top_color.g;
    float grad_b = top_color.b;

    for (int i = 0; i < width; i++) {
        draw_rect(x, y + i, length, 1, round(grad_r), round(grad_g), round(grad_b));
        grad_r += c_r;
        grad_g += c_g;
        grad_b += c_b;
    }
}

/* Grid Graphics */
void g_draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b) {
    int real_length = ceil(length * grid_cam_zoom_p);
    int real_width = ceil(width * grid_cam_zoom_p);
    draw_rect(
        round( (x - grid_cam_x) * grid_cam_zoom_p ),
        round( (y - grid_cam_y) * grid_cam_zoom_p ),
        real_length,
        real_width,
        r, g, b
    );
}

void g_draw_rect_rgb(int x, int y, int length, int width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(int x, int y, int radius, unsigned char r, unsigned char g, unsigned char b) {
    draw_point(
        round((x - grid_cam_x) * grid_cam_zoom_p),
        round((y - grid_cam_y) * grid_cam_zoom_p),
        round(radius * grid_cam_zoom_p),
        r, g, b
    );
}

void g_draw_point_rgb(int x, int y, int radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

/* Grid */
int get_grid_bool(int x, int y) {
    return bit_bool(
        grid_enc[((grid_length * y) + x) / (SIZE_BOOL_CONT * 8)],
        ((grid_length * y) + x) % (SIZE_BOOL_CONT * 8)
    );
}

int get_grid_bool_coords(int x, int y) {
    return get_grid_bool(x / GRID_SPACING, y / GRID_SPACING);
}

/* Raycasting */
typedef struct xy {
    int x;
    int y;
} xy;

xy raycast(int x, int y, float angle) {
    int c_hx, c_hy;
    int d_hx, d_hy;
    int c_vx, c_vy;
    int d_vx, d_vy;
    float alpha;
    if (angle < M_PI / 2) {
        alpha = angle;
        c_hy = ((ceil((float) y / GRID_SPACING)) * GRID_SPACING);// - 1;
        c_hx = ((c_hy - y) / tan(alpha)) + x;
        d_hx = GRID_SPACING / tan(alpha);
        d_hy = GRID_SPACING;
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = (tan(alpha) * (c_vx - x)) + y;
        d_vx = GRID_SPACING;
        d_vy = tan(alpha) * (d_vx);
    } else if (angle < M_PI) {
        alpha = angle - (M_PI / 2);
        c_hy = (ceil((float) y / GRID_SPACING) * GRID_SPACING);// - 1;
        c_hx = tan(alpha) * (y - c_hy) + x;
        d_hy = GRID_SPACING;
        d_hx = -GRID_SPACING * tan(alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y + ((x - c_vx)/ tan(alpha));
        d_vx = -GRID_SPACING;
        d_vy = GRID_SPACING / tan(alpha);
    } else if (angle < M_PI + (M_PI / 2)) {
        alpha = angle - M_PI;
        c_hy = (floor((float) y / GRID_SPACING) * GRID_SPACING);
        c_hx = x + (c_hy - y) / tan(alpha);
        d_hy = -GRID_SPACING;
        d_hx = -GRID_SPACING * tan((M_PI / 2) - alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y - (tan(alpha) * (x - c_vx));
        d_vx = -GRID_SPACING;
        d_vy = -GRID_SPACING * tan(alpha);
    } else {
        alpha = angle - (M_PI + (M_PI / 2));
        c_hy = ((floor((float) y / GRID_SPACING) - 1) * GRID_SPACING) + GRID_SPACING;
        c_hx = x - tan(alpha) * (c_hy - y);
        d_hy = -GRID_SPACING;
        d_hx = GRID_SPACING * tan(alpha);
        c_vx = (floor((float) x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = y - (c_vx - x) / tan(alpha);
        d_vx = GRID_SPACING;
        d_vy = -GRID_SPACING / tan(alpha);
    }
    
    while (!(
        (
            get_grid_bool(c_hx / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ||
            get_grid_bool(c_hx / GRID_SPACING, c_hy / GRID_SPACING)
        ) || !(
            ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < grid_length ) &&
            ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < grid_height )
        ))
    ) {
        if (show_player_vision) add_temp_dgp(c_hx, c_hy, C_RED);
        c_hx += d_hx; c_hy += d_hy;
    }

    while (!(
        (
            get_grid_bool((c_vx / GRID_SPACING) - 1, c_vy / GRID_SPACING) ||
            get_grid_bool(c_vx / GRID_SPACING, c_vy / GRID_SPACING)
        ) || !(
            ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < grid_length ) &&
            ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < grid_height )
        )
    )) {
        if (show_player_vision) add_temp_dgp(c_vx, c_vy, C_RED);
        c_vx += d_vx; c_vy += d_vy;
    }

    if ( sqrt(pow(c_hx - x, 2) + pow(c_hy - y, 2)) < sqrt(pow(c_vx - x, 2) + pow(c_vy - y, 2)) ) {
        return (xy) {c_hx, c_hy};
    } else {
        return (xy) {c_vx, c_vy};
    }
}

/* Utilities */
float perc(int percent) {
    return (percent / 100.0f);
}

int rad_deg(float radians) {
    return radians * (180 / M_PI);
}

int max(int val1, int val2) {
    if (val1 > val2) return val1; else return val2;
}

int min(int val1, int val2) {
    if (val1 < val2) return val1; else return val2;
}

int bounds(int minVal, int val, int maxVal) {
    return max( min(val, maxVal), minVal );
}

/* Grid Variable Calculations */
void calc_grid_cam_zoom_p(void) {
    grid_cam_zoom_p = perc(grid_cam_zoom);
}

void calc_grid_cam_center(void) {
    grid_cam_center_x = grid_cam_x + round( (WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p) );
    grid_cam_center_y = grid_cam_y + round( (WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p) );
}

/* Grid Resets */
void reset_grid_cam(void) {
    grid_cam_x = 0;
    grid_cam_y = 0;
    grid_cam_zoom = 100;
    calc_grid_cam_zoom_p();
    grid_cam_zoom_min = 40;
    grid_cam_zoom_max = 200;
}

void reset_player(void) {
    player_x = GRID_SPACING * 4;
    player_y = GRID_SPACING * 4;
    assign_i_player_pos();
    player_x_velocity = 0;
    player_y_velocity = 0;
    player_angle = 0;
}

/* Grid Zoom */
void zoom_grid_cam(int zoom) {
    grid_cam_zoom += zoom;
    if (grid_cam_zoom < grid_cam_zoom_min) grid_cam_zoom = grid_cam_zoom_min;
    else if (grid_cam_zoom > grid_cam_zoom_max) grid_cam_zoom = grid_cam_zoom_max;
    calc_grid_cam_zoom_p();
}

void zoom_grid_cam_center(int zoom) {
    calc_grid_cam_center();
    int old_grid_cam_center_x = grid_cam_center_x;
    int old_grid_cam_center_y = grid_cam_center_y;
    zoom_grid_cam(zoom);
    grid_cam_x = old_grid_cam_center_x - round( (WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p) );
    grid_cam_y = old_grid_cam_center_y - round( (WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p) );
}

/* Player Control */
void rotate_player(float angle) {
    player_angle += angle;
    while (player_angle < 0) player_angle += M_PI * 2;
    while (player_angle >= M_PI * 2) player_angle -= (M_PI * 2);
}

void push_player_forward(int force) {
    player_x_velocity += cos(player_angle) * force;
    player_y_velocity += sin(player_angle) * force;
}

void push_player_right(int force) {
    player_angle += M_PI / 2;
    push_player_forward(force);
    player_angle -= M_PI / 2;
}

/* Debugging */
void print_debug(void) {
    
}

void print_rgb(rgb color) {
    printf("(%d, %d, %d)", color.r, color.g, color.b);
}

int menu_selection(char* title, char options[11][26]) {
    while (TRUE) {
        int op_i;
        printf("\n%s\n", title);
        for (op_i = 0; strcmp(options[op_i], "end") != 0; op_i++) {
            printf("%s %d. %s\n", (
                op_i == 9 ? "" : " " /* Spacing for the tenth option so the numbers line up */
            ), op_i + 1, options[op_i]);
        };

        int choice = -1;
        printf("(Enter '-1' to return): "); scanf("%d", &choice);
        if (choice == -1) return -1;
        if (1 <= choice && choice <= op_i) return choice - 1;
        else continue;
    }
}

void debug_menu(void) {
    open_debug_menu = FALSE;
    debug_menu_was_open = TRUE;
    reopen_debug_menu = FALSE;
    int loop = TRUE;
    while (loop) {
        switch (menu_selection("Debug Menu", TOP_MENU)) {
            case 0: { /* Set int variables */
                int var_choice = menu_selection("Select varible", int_vls_menu);
                if (var_choice == -1) break;
                struct int_varlabel* vl_choice = &int_vls[var_choice];
                printf("(%s) = %d\n", vl_choice->label, *vl_choice->value);
                printf("Set variable (%s) to ", vl_choice->label);
                scanf("%d", vl_choice->value);
                break;
            }
            case 1: { /* Set float variables */
                int var_choice = menu_selection("Select variable", flt_vls_menu);
                if (var_choice == -1) break;
                struct flt_varlabel* vl_choice = &flt_vls[var_choice];
                printf("(%s) = %f\n", vl_choice->label, *vl_choice->value);
                printf("Set variable (%s) to ", vl_choice->label);
                scanf("%f", vl_choice->value);
                break;
            }
            case 2: /* See variables */
                for (int i = 0; i < INT_VLS_LEN; i++) printf("(%s): %d\n", int_vls[i].label, *int_vls[i].value);
                for (int i = 0; i < FLT_VLS_LEN; i++) printf("(%s): %f\n", flt_vls[i].label, *flt_vls[i].value);
                print_debug();
                break;
            case 3: /* Advance one frame */
                open_debug_menu = TRUE;
                reopen_debug_menu = TRUE;
                loop = FALSE;
                break;
            case 4: /* Switch viewpoints */
                render_in_first_person = !render_in_first_person;
                break;
            case 5: /* Reset player */
                reset_grid_cam();
                reset_player();
                break;
            case 6: /* Close Program */
                game_is_running = FALSE;
                loop = FALSE;
                break;
            default:
                loop = FALSE;
        }
    }
}

void new_dgp(int x, int y, rgb color, struct debug_grid_point** head, struct debug_grid_point** tail) {
    struct debug_grid_point* p = malloc(sizeof(*p));
    p->x = x;
    p->y = y;
    p->color = color;
    p->next = NULL;
    if (*head) {
        (*tail)->next = p;
        *tail = p;
    } else {
        *head = p;
        *tail = p;
    }
}

void add_fill_dgp(int x, int y, rgb color) {
    new_dgp(x, y, color, &fill_dgp_head, &fill_dgp_tail);
    num_fill_dgps++;
    if (num_fill_dgps > max_fill_dgps) {
        struct debug_grid_point* old_head = fill_dgp_head;
        fill_dgp_head = fill_dgp_head->next;
        free(old_head);
    }
}

void add_temp_dgp(int x, int y, rgb color) {
    new_dgp(x, y, color, &temp_dgp_head, &temp_dgp_tail);
}


const Uint8 *state;
/* Engine Functions */
void setup(void) {
    bit_rep_init();

    state = SDL_GetKeyboardState(NULL);

    grid_length = 16;
    grid_height = grid_length;

    grid_enc = (bool_cont *) malloc(ceil( (float) ((grid_length * grid_height) / SIZE_BOOL_CONT) ));

    int new_grid[16][16] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1},
        {1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,1,1,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    for (int row = 0; row < grid_height; row++) {
        for (int col = 0; col < grid_length; col++) {
            bit_assign(
                &grid_enc[((grid_length * row) + col) / (SIZE_BOOL_CONT * 8)],
                ((grid_length * row) + col) % (SIZE_BOOL_CONT * 8),
                new_grid[row][col]
            );
        }
    }

    reset_player();
    reset_grid_cam();
    grid_cam_zoom_p = perc(grid_cam_zoom);
    calc_grid_cam_center();

    /* Set up variable label structs */
    struct int_varlabel new_int_vls[INT_VLS_LEN] = {
        {"grid camera x", &grid_cam_x},
        {"grid camera y", &grid_cam_y},
        {"grid camera zoom", &grid_cam_zoom},
        {"grid show crosshairs", &show_grid_crosshairs},
        {"grid cam follows player", &grid_follow_player},
        {"grid show player vision", &show_player_vision},
        {"show player trail", &show_player_trail},
        {"grid show grid", &show_grid_lines},
        {"render walls", &fp_show_walls}
    };
    for (int i = 0; i < INT_VLS_LEN; i++) int_vls[i] = new_int_vls[i];

    struct flt_varlabel new_flt_vls[FLT_VLS_LEN] = {
        {"player x", &player_x},
        {"player y", &player_y},
        {"player angle", &player_angle}
    };
    for (int i = 0; i < FLT_VLS_LEN; i++) flt_vls[i] = new_flt_vls[i];

    /* Set up debug menus for varlabels */
    for (int i = 0; i < INT_VLS_LEN; i++) strcpy(int_vls_menu[i], int_vls[i].label);
    strcpy(int_vls_menu[INT_VLS_LEN], "end");

    for (int i = 0; i < FLT_VLS_LEN; i++) strcpy(flt_vls_menu[i], flt_vls[i].label);
    strcpy(flt_vls_menu[FLT_VLS_LEN], "end");

}

Uint8 prev_state[SDL_NUM_SCANCODES];
int key_just_pressed(int scancode) {
    return state[scancode] && !prev_state[scancode];
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = FALSE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = TRUE;
            break;
        case SDL_MOUSEWHEEL:
            zoom_grid_cam_center(-event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
            if (prev_mouse_x != -1 && prev_mouse_y != -1 && left_mouse_down) {
                grid_cam_x -= round( (event.motion.x - prev_mouse_x) * (1.0f / grid_cam_zoom_p) );
                grid_cam_y -= round( (event.motion.y - prev_mouse_y) * (1.0f / grid_cam_zoom_p) );
            }
            prev_mouse_x = event.motion.x;
            prev_mouse_y = event.motion.y;
    }

    shift = state[SDL_SCANCODE_LSHIFT];

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;
    if (state[SDL_SCANCODE_RETURN] && !debug_menu_was_open) open_debug_menu = TRUE;

    if (state[SDL_SCANCODE_RIGHT]) rotation_input++;
    if (state[SDL_SCANCODE_LEFT]) rotation_input--;

    if (state[SDL_SCANCODE_R]) {
        reset_grid_cam();
        reset_player();
    }
    if (key_just_pressed(SDL_SCANCODE_P)) print_debug();

    if (state[SDL_SCANCODE_W]) vertical_input++;
    if (state[SDL_SCANCODE_S]) vertical_input--;
    if (state[SDL_SCANCODE_A]) horizontal_input--;
    if (state[SDL_SCANCODE_D]) horizontal_input++;

    if (key_just_pressed(SDL_SCANCODE_M)) render_in_first_person = !render_in_first_person;

    memcpy(prev_state, state, sizeof(prev_state));
}

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    // Only delay if we are too fast to update this frame
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    // Get a delta time factor converted to seconds to be used to update my objects
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    if (open_debug_menu && !(debug_menu_was_open && !reopen_debug_menu)) debug_menu();
    else debug_menu_was_open = FALSE;

    static float prev_player_x;
    static float prev_player_y;

    push_player_forward(player_movement_accel * vertical_input);
    push_player_right(player_movement_accel * horizontal_input);
    rotate_player(rotation_input * player_angle_increment * player_rotation_speed * delta_time);

    player_velocity_angle = atan(player_y_velocity / player_x_velocity);
    if (player_x_velocity < 0) player_velocity_angle += M_PI;

    if (player_x_velocity != 0 || player_y_velocity != 0) {
        if (player_x_velocity == 0) {
            if (player_y_velocity > player_max_velocity) player_y_velocity = player_max_velocity;
            else if (player_y_velocity < -player_max_velocity) player_y_velocity = -player_max_velocity;

        } else if (player_y_velocity == 0) {
            if (player_x_velocity > player_max_velocity) player_x_velocity = player_max_velocity;
            else if (player_x_velocity < -player_max_velocity) player_x_velocity = -player_max_velocity;

        } else {
            if (sqrt( pow(player_x_velocity, 2) + pow(player_y_velocity, 2) ) > player_max_velocity) {
                player_x_velocity = cos(player_velocity_angle) * player_max_velocity;
                player_y_velocity = sin(player_velocity_angle) * player_max_velocity;
            }
        }
    }

    if (!horizontal_input && !vertical_input) {
        if (player_x_velocity != 0) {
            int vel_is_pos = player_x_velocity > 0;
            player_x_velocity -= cos(player_velocity_angle) * player_movement_decel;
            if (vel_is_pos && player_x_velocity < 0) player_x_velocity = 0;
            else if (!vel_is_pos && player_x_velocity > 0) player_x_velocity = 0;
        }
        if (player_y_velocity != 0) {
            int vel_is_pos = player_y_velocity > 0;
            player_y_velocity -= sin(player_velocity_angle) * player_movement_decel;
            if (vel_is_pos && player_y_velocity < 0) player_y_velocity = 0;
            else if (!vel_is_pos && player_y_velocity > 0) player_y_velocity = 0;
        }
    }

    prev_player_x = player_x;
    prev_player_y = player_y;

    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;
    assign_i_player_pos();

    // Wall collisions
    int northeast_collision = get_grid_bool_coords(player_x + player_radius, player_y - player_radius);
    if (northeast_collision) puts("northeast coll");
    int southeast_collision = get_grid_bool_coords(player_x + player_radius, player_y + player_radius);
    if (southeast_collision) puts("southeast coll");
    int southwest_collision = get_grid_bool_coords(player_x - player_radius, player_y + player_radius);
    if (southwest_collision) puts("southwest coll");
    int northwest_collision = get_grid_bool_coords(player_x - player_radius, player_y - player_radius);
    if (northwest_collision) puts("northwest coll");
/* 
    if (northeast_collision || southeast_collision || southwest_collision || northwest_collision)
    add_temp_dgp(i_player_x, i_player_y, C_PURPLE);
 */
    // North
    if (
        (
            northeast_collision && (
                !(southeast_collision && !northwest_collision) || (
                    player_x_velocity != 0 &&
                    (( ((int) prev_player_x + player_radius) / GRID_SPACING ) == ( ((int) player_x + player_radius) / GRID_SPACING ))
                )
            )
        ) || (
            northwest_collision && (
                !(southwest_collision && !northeast_collision) || (
                    player_x_velocity != 0 &&
                    (( ((int) prev_player_x - player_radius) / GRID_SPACING ) == ( ((int) player_x - player_radius) / GRID_SPACING ))
                )
            )
        )
    ) {
        player_y = (((int) player_y / GRID_SPACING) * GRID_SPACING) + player_radius;
        player_y_velocity = 0;
        printf(
            "north collision: east now_x (%d), east prev_x (%d)\n",
            ((int) player_x + player_radius) / GRID_SPACING,
            ((int) prev_player_x + player_radius) / GRID_SPACING
        );
        // open_debug_menu = TRUE;
    }

    // South
    if (
        (southeast_collision && !(northeast_collision && !southwest_collision)) ||
        (southwest_collision && !(northwest_collision && !southeast_collision))
    ) {
        player_y = ((((int) player_y / GRID_SPACING) + 1) * GRID_SPACING) - player_radius;
        player_y_velocity = 0;
        puts("south collision");
    }

    // East
    if (
        (northeast_collision && !(northwest_collision && !southeast_collision)) ||
        (southeast_collision && !(southwest_collision && !northeast_collision))
    ) {
        player_x = ((((int) player_x / GRID_SPACING) + 1) * GRID_SPACING) - player_radius;
        player_x_velocity = 0;
        puts("east collision");
    }

    // West
    if (
        (northwest_collision && !(northeast_collision && !southwest_collision)) ||
        (southwest_collision && !(southeast_collision && !northwest_collision))
    ) {
        player_x = (((int) player_x / GRID_SPACING) * GRID_SPACING) + player_radius;
        player_x_velocity = 0;
        puts("west collision");
    }

    if (show_player_trail) add_fill_dgp(player_x, player_y, C_YELLOW);

    if (grid_follow_player) {
        grid_cam_x = round( player_x - ((WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p)) );
        grid_cam_y = round( player_y - ((WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p)) );
    }

    printf("\nplayer pos: (%f, %f) facing %f (%d deg)\n", player_x, player_y, player_angle, rad_deg(player_angle));
    printf("player velocity: (%f, %f)\n", player_x_velocity, player_y_velocity);

    vertical_input = 0;
    horizontal_input = 0;
    rotation_input = 0;

    last_frame_time = SDL_GetTicks();
}

void render(void) {
    set_draw_color_rgb(render_in_first_person ? fp_bg_top : grid_bg);
    SDL_RenderClear(renderer);

    if (render_in_first_person) {
        draw_rect_rgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2, C_BLACK);
        vertical_gradient(0, (WINDOW_HEIGHT / 2) + ((WINDOW_HEIGHT / 2) - fp_render_distance_scr), WINDOW_WIDTH, fp_render_distance_scr, C_BLACK, fp_bg_bottom);
        draw_rect_rgb(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2, fp_bg_top);
    }

    if (render_in_first_person || show_player_vision) {
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            float ray_angle = (player_angle - (FOV / 2)) + ((FOV / WINDOW_WIDTH) * ray_i);
            if (ray_angle < 0) ray_angle += M_PI * 2;
            else if (ray_angle >= M_PI * 2) ray_angle -= M_PI * 2;
            xy hit = raycast(round(player_x), round(player_y), ray_angle);

            if (render_in_first_person && fp_show_walls) {
                float dist = cos(ray_angle - player_angle) * sqrt( pow(hit.x - player_x, 2) + pow(hit.y - player_y, 2) );
                int height = (1.0f / (dist * fp_scale)) * WINDOW_HEIGHT;
                rgb new_wall_color = brighten(wall_color, (float) -(dist / fp_render_distance) * fp_brightness);
                draw_rect_rgb(ray_i, (WINDOW_HEIGHT / 2) - (height / 2), 1, height, new_wall_color);

            } else if (show_player_vision) add_temp_dgp(hit.x, hit.y, C_WHITE);
        }
    }

    if (!render_in_first_person) { /* Map View */
        /* Grid */
        
        /* Fill */
        for (int row = 0; row < grid_height; row++) {
            for (int col = 0; col < grid_length; col++) {
                g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                get_grid_bool(col, row) ? grid_fill_solid : grid_fill_nonsolid);
            }
        }

        if (show_grid_lines) {
            /* Vertical lines */
            for (int i = 0; i < grid_length + 1; i++) {
                g_draw_rect_rgb(
                    (i * GRID_SPACING) - ceil(grid_line_width / 2), 
                    0,
                    grid_line_width,
                    grid_height * GRID_SPACING,
                    grid_line_fill
                );
            }
            /* Horizontal lines */
            for (int i = 0; i < grid_length + 1; i++) {
                g_draw_rect_rgb(
                    0,
                    (i * GRID_SPACING) - ceil(grid_line_width / 2),
                    grid_length * GRID_SPACING,
                    grid_line_width,
                    grid_line_fill
                );
            }
        }

        /* Fill DGPs */
        for (struct debug_grid_point* p_i = fill_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, p_i->color);
        }

        /* Player */
        g_draw_point_rgb( // Player pointer
            player_x + round(cos(player_angle) * grid_player_pointer_dist),
            player_y + round(sin(player_angle) * grid_player_pointer_dist),
            player_radius * perc(grid_player_pointer_radius_offset),
            grid_player_fill
        );
        g_draw_point_rgb(player_x, player_y, player_radius, grid_player_fill); // Player

        /* Temp DGPs */
        for (struct debug_grid_point* p_i = temp_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, p_i->color);
        }

        if (show_grid_crosshairs) {
            draw_rect_rgb(WINDOW_WIDTH / 2, 0, 1, WINDOW_HEIGHT, C_WHITE);
            draw_rect_rgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, 1, C_WHITE);
        }
    }

    struct debug_grid_point* p_i = temp_dgp_head;
    while (p_i) {
        struct debug_grid_point* next = p_i->next;
        free(p_i);
        p_i = next;
    }

    temp_dgp_head = NULL;
    temp_dgp_tail = NULL;

    if (shift) draw_rect(25, 25, 50, 50, 200, 55, 55);

    SDL_RenderPresent(renderer);
}

void free_memory(void) {
    free(grid_enc);

    struct debug_grid_point* p_i = fill_dgp_head;
    while (p_i) {
        struct debug_grid_point* next = p_i->next;
        free(p_i);
        p_i = next;
    }
}

int main() {
    printf("Start\n");

    game_is_running = initialize_window();

    setup();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    free_memory();
    destroy_window();

    return 0;
}