#include "world.h"

int main()
{
    /* parse arguments */
    /* example arguments */
    int arg_height = 15;
    int arg_width = 40;

    worldloop(arg_height, arg_width);

    return 0;
}

void worldloop(int height, int width)
{
    initscr();
    start_color();
    init_pair(RED, COLOR_RED, COLOR_RED);
    init_pair(GREEN, COLOR_GREEN, COLOR_GREEN);

    World * w = init_world(height, width);

    /* Add initial number of tanks */
    /* add_tank() */

    add_tank(w, 4, 1, RED);

    int status;
    while (status) {
        wrefresh(w->win);
        /* Add respawned tanks */
    }
    while (1);
    delwin(w->win);
    endwin();

    /* Print statistics */

    /* Free resources */
    if (w->zone != NULL) {
        free(w->zone);
    }
}

World * init_world(int height, int width)
{
    World * local_world = malloc(sizeof(World));
    if (local_world == NULL) {
        perror("Failed to allocate memory for world");
        exit(-1);
    }

    size_t world_size = sizeof(int) * height * width;
    int (* zone)[width] = malloc(sizeof(*zone) * height);
    if (zone == NULL) {
        perror("Failed to allocate memory for world zone");
        exit(-1);
    }
//    memset(zone, EMPTY, world_size);

    local_world->height = height;
    local_world->width = width;
    local_world->win = newwin(height, width, 0, 0);
    local_world->zone = zone;

    return local_world;
}

bool add_tank(World * world, int x, int y, int tank_color)
{
    printf("SPADNU?\n");
    world->zone[x][y] = tank_color;
    printf("SPADNU?\n");
    if (x < 0 || x > world->width || y < 0 || y > world->height) {
        return false;
    }
    if (world->zone[x][y] != EMPTY) {
        return false;
    }
    if (tank_color != RED || tank_color != GREEN) {
        return false;
    }
    world->zone[x][y] = tank_color;

    /* fixme: call execv to run TANK_BIN */

    wmove(world->win, y, x);
    /* fixme: Vykreslit tank barevne */
    attrset(COLOR_PAIR(tank_color));
    wprintw(world->win, "T");
    /* fixme: Nechat zmizet kurzot */
    wrefresh(world->win);

    return true;
}

