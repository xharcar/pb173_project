#include "world.h"

int arg_height, arg_width, tanks_red, tanks_green, respawns, tanks_number;
int green_kills = 0, red_kills = 0;

Tankprocess* red_team;
Tankprocess* green_team;

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
            green_team = (Tankprocess*) malloc(sizeof(Tankprocess)*tanks_green);
            break;
        case 'r':
            tanks_red = atoi(optarg);
            red_team = (Tankprocess*) malloc(sizeof(Tankprocess)*tanks_red);
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

int main(int argc, char** argv){
    parse_args(argc,argv);
    worldloop(arg_height, arg_width);
    
    
    return 0;
}

void worldloop(int height, int width)
{
    init_ncurses();

    World * w = init_world(height, width);

    tanks_number = tanks_red + tanks_green;
            
    /* Adding initial number of green tanks */
    int i;
    for (i = 0; i < tanks_green; i++) {
        // store tank position for easy access
        // assuming map <= 65535x65535
        short xco = rand()%width; 
        short yco = rand()%height;
        
        // initialize tank:
        // 1) check if process creation was successful; if not, retry
        // 2) if successful, store appropriate values in structure
        pid_t tid = add_tank(w, xco, yco, GREEN); // tid = tank id
        if(tid > 1){
            init_tank_process(green_team+i,tid,xco,yco);
        }
        // fixme:closing read end of pipe in tank
    }
    
    /* Adding initial red tanks*/
    for (i = 0; i < tanks_red; i++) {
        short xco = rand()%width; 
        short yco = rand()%height;
        
        pid_t tid = add_tank(w, xco, yco, RED);
        if(tid > 1){
            init_tank_process(red_team+i,tid,xco,yco);
        }
        // fixme:closing read end of pipe in tank
    }

    while (tanks_number > 0) {
        int respawnflag = 0;
        pid_t id =  wait(NULL);
        tanks_number--;
        for(i=0;i<tanks_green;i++){
            if(respawnflag) break;
            if(green_team[i].pid == id){
                respawnflag = 1;
                if (respawns > 0) {
                short xco = rand()%width; 
                short yco = rand()%height;
                pid_t tid = add_tank(w, xco, yco, GREEN);
                    if(tid>1){
                        init_tank_process(green_team+i,tid,xco,yco);
                        respawns--;
                        tanks_number++;
                    }
                }
            }
        }
        for(i=0;i<tanks_red;i++){
            if(respawnflag) break;
            if(red_team[i].pid == id){
                respawnflag = 1;
                if (respawns > 0) {
                short xco = rand()%width; 
                short yco = rand()%height;
                pid_t tid = add_tank(w, xco, yco, RED);
                    if(tid>1){
                        init_tank_process(red_team+i,tid,xco,yco);
                        respawns--;
                        tanks_number++;
                    }
                }
            }
        }
    }
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


void stats_refresh(World * world, int green_kills, int red_kills)
{
    werase(world->win_stats);
    wprintw(world->win_stats, "Green tanks killed %d\n", green_kills);
    wprintw(world->win_stats, "Red tanks killed %d\n", red_kills);
    wrefresh(world->win_stats);
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

/*
 * Stops drawing a tank that has been destroyed
 * Does not end the process of said tank
 * @param x position of tank relative to X-axis
 * @param y position of tank relative to Y-axis 
 */
void destroy_tank(World * world, int x, int y)
{
    /* Compensate for border padding */
    x++; y++;
    wattrset(world->win, COLOR_PAIR(0));
    mvwaddch(world->win, y, x, ' ');
    wattroff(world->win, COLOR_PAIR(0));
    wrefresh(world->win);
}

void print_stats(int height, int width)
{
    printf("Area size: %d %d\n", height, width);
    printf("Red kills: %d\n", red_kills);
    printf("Green kills: %d\n", green_kills);
}


pid_t add_tank(World * world, int x, int y, int tank_color)
{
    pid_t rv; // unified return value for all outcomes
    if (x < 0 || x > world->width || y < 0 || y > world->height) {
        fprintf(stderr,"Coordinates %d %d outside map boundaries",x,y);
        rv = -2;
        return rv;
    }
    if ( (world->zone)[x][y] != EMPTY) {
        fprintf(stderr,"Coordinates %d %d not empty",x,y);
        rv = -3;
        return rv;
    }
    if (tank_color != RED && tank_color != GREEN) {
        fprintf(stderr,"Wrong tank color");
        rv = -4;
        return rv;
    }
    // Fix: checks return nonzero to clearly indicate error
    // (0 being the "all-clear" by convention)
    
    /*
    (world->zone)[x][y] = tank_color;
    draw_tank(world, x, y, tank_color);
     This would draw a tank even if process spawn failed; fixed
     */ 
    rv = spawn_tank_process(world, tank_color);
    if(rv != -1){
        (world->zone)[x][y] = tank_color;
        draw_tank(world, x, y, tank_color);
        
    }
    return rv;
}

pid_t spawn_tank_process(World * w, int tank_color)
{
    pid_t child = fork();
    if (child == -1) {
        perror("Failed to spawn tank process");
        return -1;
        // exit() terminates the calling process (== main?)
    } else if (child == 0) {
        system(TANK_BIN" --sleep-max 5 --sleep-min 1");
    } else {
        return child;
    }
}

void init_tank_process(Tankprocess* tp, pid_t id, short xco, short yco){
    tp->pid = id;
    tp->pos_x = xco;
    tp->pos_y = yco;
    pipe(tp->pipe_in);
    close(tp->pipe_in[1]);
}