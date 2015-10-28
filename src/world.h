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

pid_t add_tank(World * world, int x, int y, int tank_color);

/**
 * @brief spawn_tank_process calls fork() and synchroniously spawns tank process
 * fixme: report statistics
 */
pid_t spawn_tank_process();

void draw_tank(World * world, int x, int y, int tank_color);

void destroy_tank(World * world, int x, int y);

void print_stats(int height, int width);


/*
 * Tank process representative;
 * pid = PID to be saved here when new tank is created, used to send kill signal
 * when tank is destroyed
 * pipe_in = pipe for incoming communication(tank sending info)
 * pos_x = position of tank relative to X axis
 * pos_y = position of tank relative to Y axis
 */
typedef struct{
    pid_t pid;
    int pipe_in[2];
    short pos_x;
    short pos_y;
}Tankprocess;
