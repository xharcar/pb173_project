#include "world.h"

int arg_height, arg_width, tanks_red, tanks_green, respawns, tanks_number;
int green_kills = 0, red_kills = 0;

void print_help()
{
    printf("=====================================================\n");
    printf("|         PB173 Internet Of Tanks presents:  WORLD  |\n");;
    printf("-----------------------------------------------------\n");
    printf("                    USAGE                            \n");
    printf("  -h | --help           Show this help               \n");
    printf("  --green-tanks [n]     create n green tanks         \n");
    printf("  --red-tanks [n]       create n red tanks           \n");
    printf("  --total-respawn [n]   number of tanks to be        \n");
    printf("                        respawned in both teams      \n");
    printf("  --area-size [n] [m]   size of area NxM             \n");
    printf("=====================================================\n");
}

void print_error()
{
    fprintf(stderr, "Wrong arguments\n");
}

void parse_args(int argc, char *argv[])
{
    struct option longopts[] = {
        { "green-tanks",   required_argument, NULL, 'g' },
        { "red-tanks",     required_argument, NULL, 'r' },
        { "total-respawn", required_argument, NULL, 'w' },
        { "area-size",     required_argument, NULL, 'a' },
        { "help",          no_argument,       NULL, 'h' },
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
        case 'g':
            tanks_green = atoi(optarg);
            break;
        case 'r':
            tanks_red = atoi(optarg);
            break;
        case 'w' :
            respawns = atoi(optarg);
            break;
        case 'h':
            print_help();
            exit(0);
        default:
            print_error();
            exit(-1);
        }
    }
    // printf("X: %i, Y: %i\n", areaX, areaY);
    /* we need both opts, redtanks, respawn and greentanks can be zero */
    if(arg_height<=0 || arg_width<=0) {
        print_error();
        exit(-1);
    }
}

void testdraw_worldloop(int height, int width)
{
    tanks_red = 2, tanks_green = 2;
    tanks_number = tanks_red + tanks_green;
    respawns = 0;

    init_ncurses();

    World * w = init_world(height, width);

    stats_refresh(w, 0, 0);
    draw_tank(w, 0, 0, GREEN);
    draw_tank(w, 2, 2, GREEN);
    draw_tank(w, 4, 4, RED);
    for(int i = 0; i < 1000000000; i++);
    destroy_tank(w, 4, 4);
    stats_refresh(w, 3, 9);

    while (1);

    delwin(w->win);
    endwin();

    /* Free resources */
    if (w->zone != NULL) {
        free(w->zone);
    }
    if (w != NULL) {
        free(w);
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    worldloop(arg_height, arg_width);
    //testdraw_worldloop(8, 15);

    return 0;
}

void worldloop(int height, int width)
{
    init_ncurses();

    World * w = init_world(height, width);

    tanks_number = tanks_red + tanks_green;

    /* Adding initial number of tanks */
    for (int i = 0; i < tanks_green; i++) {
        add_tank(w, rand()%width, rand()%height, GREEN);
    }
    for (int i = 0; i < tanks_red; i++) {
        add_tank(w, rand()%width, rand()%height, RED);
    }

    while (tanks_number > 0);

    delwin(w->win);
    endwin();

    /* Print statistics */
    print_stats(height, width);

    /* Free resources */
    if (w->zone != NULL) {
        free(w->zone);
    }
    if (w != NULL) {
        free(w);
    }
}

void init_ncurses()
{
    initscr();
    start_color();
    init_pair(RED, COLOR_RED, COLOR_RED);
    init_pair(GREEN, COLOR_GREEN, COLOR_GREEN);
    curs_set(0);
}

void stats_refresh(World * world, int green_kills, int red_kills)
{
    werase(world->win_stats);
    wprintw(world->win_stats, "Green tanks killed %d\n", green_kills);
    wprintw(world->win_stats, "Red tanks killed %d\n", red_kills);
    wrefresh(world->win_stats);
}

World * init_world(int height, int width) {
    World * local_world = malloc(sizeof(World));
    if (local_world == NULL) {
        perror("Failed to allocate memory for world");
        exit(-1);
    }

    size_t world_size = sizeof(int) * height * width;
    int (* zone)[COMPILE_TIME_WIDTH] = malloc(sizeof(*zone) * height);
    if (zone == NULL) {
        perror("Failed to allocate memory for world zone");
        exit(-1);
    }
    memset(zone, EMPTY, world_size);

    local_world->height = height;
    local_world->width = width;
    local_world->zone = zone;
    /* Add padding for borders */
    local_world->win = newwin(height + 2, width + 2, 0, 0);
    local_world->win_stats = newwin(10, 20, 1, width + 2 + 3);

    box(local_world->win, 0, 0);
    wrefresh(local_world->win);

    return local_world;
}

bool add_tank(World * world, int x, int y, int tank_color)
{
    if (x < 0 || x > world->width || y < 0 || y > world->height) {
        return false;
    }
    if ( (world->zone)[x][y] != EMPTY) {
        return false;
    }
    if (tank_color != RED && tank_color != GREEN) {
        return false;
    }
    (world->zone)[x][y] = tank_color;

    draw_tank(world, x, y, tank_color);
    spawn_tank_process(world, tank_color);

    return true;
}

void draw_tank(World * world, int x, int y, int tank_color)
{
    /* Compensate for border padding */
    x++, y++;
    wattrset(world->win, COLOR_PAIR(tank_color));
    mvwaddch(world->win, y, x, ACS_BLOCK);
    wattroff(world->win, COLOR_PAIR(tank_color));
    wrefresh(world->win);
}

void destroy_tank(World * world, int x, int y)
{
    /* Compensate for border padding */
    x++; y++;
    wattrset(world->win, COLOR_PAIR(0));
    mvwaddch(world->win, y, x, ' ');
    wattroff(world->win, COLOR_PAIR(0));
    wrefresh(world->win);
}

void spawn_tank_process(World * w, int tank_color)
{
    tanks_number--;
    if (tank_color == RED) {
        green_kills++;
    } else {
        red_kills++;
    }
    pid_t child = fork();
    if (child == -1) {
        perror("Failed to spawn tank process");
        exit(-1);
    } else if (child == 0) {
        //execl(TANK_BIN, TANK_BIN, "--sleep-max=5", "sleep-min=1", (char *)NULL);
        system(TANK_BIN" --sleep-max 5 --sleep-min 1");
    } else {
        if (respawns > 0) {
            respawns--;
            tanks_number++;
            add_tank(w, rand()%w->width, rand()%w->height, tank_color);
        }
    }
}

void print_stats(int height, int width)
{
    printf("Area size: %d %d\n", height, width);
    printf("Red kills: %d\n", red_kills);
    printf("Green kills: %d\n", green_kills);
}
