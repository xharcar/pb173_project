#include "config.h"
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>

#define COMPILE_TIME_WIDTH 80

enum TANK { EMPTY, RED, GREEN };

//T foo( size_t cols, int (*arr)[cols], size_t rows)

typedef struct {
    int height;
    int width;
    int (* zone)[COMPILE_TIME_WIDTH];
//    int zone[100][100];
    WINDOW * win;
} World;

void worldloop(int height, int width);

/*
 * Initializes world including the spawn of ncurses window
 */
World * init_world(int height, int width);

/*
 * add_tank calls ncurses wrefresh() upon its invocation
 */
bool add_tank(World * world, int x, int y, int tank_color);

void spawn_tank_process();
