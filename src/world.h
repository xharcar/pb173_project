#include "config.h"
#include <ncurses.h>

enum TANK { EMPTY, RED, GREEN };

typedef struct {
    int ** zone;
    WINDOW * win;
    int height;
    int width;
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

