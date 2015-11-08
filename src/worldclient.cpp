#include "worldclient.h" 

int main(int argc, char *argv[])
{
    Options opts;
    parse_args(argc, argv, &opts);
    NCursesClient nc_client(opts.pipe);
    /* Refreshing map, react on key presses */
    /* Does not end if EOF is reached */
    while (1) {
        nc_client.print_tanks();
    }
    return 0;
}

void parse_args(int argc, char *argv[], Options * opts)
{
    if (argc < 2) {
        std::cout << "Wrong argument" << std::endl;
        print_help(argv[0]);
        exit(-1);
    }
    struct option longopts[] = {
        { "pipe", required_argument, NULL, 'p' },
        { "help", no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };
    int c;
    while ( (c = getopt_long(argc, argv, "p:h", longopts, NULL) ) != -1) {
        switch (c) {
        case 'p':
            opts->pipe = optarg;
            break;
        case 'h':
            print_help(argv[0]);
            exit(0);
            break;
        default:
            std::cout << "Wrong argument" << std::endl;
            print_help(argv[0]);
            exit(-1);
        }
    }
}

void print_help(char * progname)
{
    std::cout << "Usage: " << progname << " [-p <path to named pipe> | -h]" << std::endl;
    std::cout << std::endl;
}

void WorldClient::get_world_pid() {
    std::ifstream pid_file;
    pid_file.open("/var/run/world.pid");
    if (pid_file) {
        std::cerr << "world.pid file does not exist. World process is not running." << std::endl;
        exit(-1);
    } else if ( pid_file >> world_pid ) {
        /* Successfully read the number  */
    } else {
        /* Failed to read the number */
        std::cerr << "Failed to read the pid from the world.pid file." << std::endl;
        exit(-1);
    }
}

void WorldClient::open_pipe(char * pipe) {
    int fd;
    if ( (fd = open(pipe, O_RDONLY) ) < 0 ) {
        std::cerr << strerror(errno) << "Can not open the pipe for streaming data from world process." << std::endl;
        exit(-1);
    }
    pipe_stream = fdopen(fd, "r");
    if (!pipe_stream) {
        std::cerr << strerror(errno) << "Can not open the pipe for streaming data from world process." << std::endl;
        exit(-1);
    }
    clearerr(pipe_stream);
    int dimensions = fscanf(pipe_stream, "%d, %d", &width, &height);
    if ( dimensions == EOF && ferror(pipe_stream) ) {
        std::cerr << strerror(errno) << "Error occured while parsing the pipe stream." << std::endl;
    } else if (dimensions != 2 ) {
        std::cerr << "Error: Worng format of the data in the pipe." << std::endl;
        exit(-1);
    }
}

NCursesClient::NCursesClient(char * pipe) : WorldClient(pipe) {
    initscr();
    start_color();
    /* Create color associations in ncurses */
    init_pair(Color::RED, COLOR_RED, COLOR_RED);
    init_pair(Color::GREEN, COLOR_GREEN, COLOR_GREEN);
    /* Hide the cursor in ncurses */
    curs_set(0);
    /* Disables line buffering */
    cbreak();
    noecho();
    nodelay(nc_world, TRUE);

    /* Add padding for borders */
    nc_world = newwin(height + 2, width + 2, 0, 0);
    // nc_stats = newwin(10, 20, 1, width + 2 + 3);
    box(nc_world, 0, 0);
    wrefresh(nc_world);
    keys();
}

void NCursesClient::print_tanks() {
    char sector;
    int x = 0, y = 0;
    while ( fscanf(pipe_stream, ",%c", &sector) != EOF) {
        switch (sector) {
        case 'r':
            draw_tank(x, y, Color::RED);
            break;
        case 'g':
            draw_tank(x, y, Color::GREEN);
            break;
        case '0':
            undraw_tank(x, y);
            break;
        }
        x++;
        if (x >= width) {
            x = 0;
            y++;
        }
        if (y >= height) {
            wrefresh(nc_world);
            keys();
        }
    }
}

void NCursesClient::keys() {
    char c = (char)getch();
    switch (c) {
        case 'x':
            kill(world_pid, SIGINT);
            break;
        case 'r':
            kill(world_pid, SIGUSR1);
            break;
        case 'q':
            exit(0);
            break;
    }
}

void NCursesClient::draw_tank(int x, int y, Color color) {
    wattrset(nc_world, COLOR_PAIR(color));
    /* Compensate for border padding */
    mvwaddch(nc_world, y + 1, x + 1, ACS_BLOCK);
    /* type cast enum class color */
    wattroff(nc_world, COLOR_PAIR(color));
    wrefresh(nc_world);
}

void NCursesClient::undraw_tank(int x, int y) {
    /* COLOR_PAIR(0) sets the default color */
    wattrset(nc_world, COLOR_PAIR(0));
    /* Compensate for border padding */
    mvwaddch(nc_world, y + 1, x + 1, ' ');
    wattroff(nc_world, COLOR_PAIR(0));
    wrefresh(nc_world);
}

