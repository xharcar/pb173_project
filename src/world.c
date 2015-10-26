#include "world.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>


int arg_height,arg_width,redTanks,greenTanks,respawns;


int printHelp()
{
    printf("=====================================================\n");
    printf("|         PB173 Internet Of Tanks presents:  WORLD  |\n");;
    printf("-----------------------------------------------------\n");
    printf("                    USAGE                            \n");
    printf("  -h | --help           Show this help                   \n");
    printf("  --green-tanks [n]     create n green tanks             \n");
    printf("  --red-tanks [n]       create n red tanks               \n");
    printf("  --total-respawn [n]   number of tanks to be    \n");
    printf("                        respawned in both teams \n");
    printf("  --area-size [n] [m]   size of area NxM             \n");
    printf("=====================================================\n");

    return 1;
}

int printError()
{
    fprintf(stderr, "Wrong arguments\n");
    return 1;
}

int parseArgs(int argc, char *argv[])
{
    struct option longopts[] = {
       { "green-tanks",     required_argument,       NULL,  'g'   },
       { "red-tanks",    required_argument, NULL,   'r' },
       { "total-respawn",    required_argument, NULL,   'w' },
       { "area-size",    required_argument, NULL,   'a' },
       { "help",    no_argument,       NULL,    'h' },
      { 0, 0, 0, 0 }
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "g:r:w:a:h", longopts, NULL)) != -1) {
        switch (c) {
        case 'a':
            arg_width = atoi(argv[optind-1]);
            arg_height = atoi(argv[optind]);
            break;
        case 'g':   greenTanks = atoi(optarg);
            break;
        case 'r':   redTanks = atoi(optarg);
            break;
        case 'w' :  respawns = atoi(optarg);
            break;
        case 'h':  return printHelp();
            break;
        default:  return printError();
        }

    }
   // printf("X: %i, Y: %i\n", areaX, areaY);

    if(arg_height<=0 || arg_width<=0)   //we need both opts, redtanks, respawn and greentanks can be zero
        return printError();
    return 0;
}

int main(int argc, char *argv[])
{
    if(parseArgs(argc, argv) == 1)  //parse arguments
        return 1;
    /* example arguments */
//    arg_height = 15;
//    arg_width = 40;

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
