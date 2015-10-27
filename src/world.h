#include "config.h"
#include <ncurses.h>

#define COMPILE_TIME_WIDTH 80


enum TANK { EMPTY, RED, GREEN };

typedef struct {
    int height;
    int width;
    int (* zone)[COMPILE_TIME_WIDTH];
    WINDOW * win;
    WINDOW * win_stats;
} World;

void worldloop(int height, int width);

void init_ncurses();

/**
 * @brief init_world Initializes world including the spawn of ncurses window
 * @param height
 * @param width
 * @return
 */
World * init_world(int height, int width);

void stats_refresh(World * world, int green_kills, int red_kills);

bool add_tank(World * world, int x, int y, int tank_color);

/**
 * @brief spawn_tank_process calls fork() and synchroniously spawns tank process
 * fixme: report statistics
 */
void spawn_tank_process();

void draw_tank(World * world, int x, int y, int tank_color);

void destroy_tank(World * world, int x, int y);

void print_stats(int height, int width);
