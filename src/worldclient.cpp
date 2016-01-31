#include "worldclient.h"

int main(int argc, char* argv[])
{
    ClientOptions opts;
    parse_args(argc, argv, opts);
    std::cout << "Pipe filename = " << opts.pipe << std::endl;
    NCursesClient nc_client(opts.pipe);
    /* Refreshing map, react on key presses */
    /* Does not end if EOF is reached */
    while (1) {
        nc_client.print_tanks();
    }
    return 0;
}

void parse_args(int argc, char* argv[], ClientOptions& opts)
{
    if (argc < 2) {
        std::cout << "Wrong number of arguments" << std::endl;
        print_help(argv[0]);
        exit(-1);
    }
    struct option longopts[] = {
        { "pipe", required_argument, NULL, 'p' },
        { "help", no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };
    int c;
    while ((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
        switch (c) {
        case 'p':
            opts.pipe = optarg;
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

void print_help(char* progname)
{
    std::cout << "Usage: " << progname << " [-p <path to named pipe> | -h]"
              << std::endl
              << std::endl;
}

void WorldClient::get_world_pid(std::string filepath)
{
    std::ifstream pid_file(filepath,std::ifstream::in);

    if (pid_file.fail()) {
        std::cerr
            << "world.pid file does not exist. World process is not running."
            << std::endl;
        exit(-1);
    }
    else if (pid_file >> world_pid) {
    std::cout << "Reading PID OK. World PID = " << world_pid << std::endl;
    }
    else {
        std::cerr << "Failed to read the pid from the world.pid file."
                  << std::endl;
        exit(-1);
    }
}

void WorldClient::open_pipe(char* pipe)
{
    int fd;
    if ((fd = open(pipe, O_RDONLY)) == -1) {
        std::cerr
            << strerror(errno)
            << "Can not open the pipe for streaming data from world process."
            << std::endl;
        exit(-1);
    }
    pipe_stream = fd;
    std::cout << "Pipe successfully opened" << std::endl;
}

NCursesClient::NCursesClient(char* pipe) : WorldClient(pipe)
{
//    std::cout << "NC client ctor" << std::endl;
//    initscr();
//    start_color();
//    std::cout << "initscr and start_color OK" << std::endl;

//    /* Create color associations in ncurses */
//    init_pair(Color::RED, COLOR_RED, COLOR_RED);
//    init_pair(Color::GREEN, COLOR_GREEN, COLOR_GREEN);
//    std::cout << "color pairs initialized" << std::endl;

//    /* Hide the cursor in ncurses */
//    curs_set(0);
//    std::cout << "cursor hiding set" << std::endl;

//    /* Disables line buffering */
//    cbreak();
//    noecho();
//    std::cout << "cbreak,noecho OK" << std::endl;

//    nc_world = newwin(height + 2, width + 2, 0, 0);
//    nodelay(nc_world, TRUE);
//    std::cout << "window created, nodelay set" << std::endl;

//    // nc_stats = newwin(10, 20, 1, width + 2 + 3);
//    box(nc_world, 0, 0);
//    wrefresh(nc_world);
//    keys();
//    std::cout << "NC client ctor done" << std::endl;
}

void WorldClient::parse_dimensions()
{
    //std::cerr << "Parsing dimensions" << std::endl;

    width = 0;
    height = 0;
    char sizeChar = ' ';
    //naive implementation
    while(1)
    {
        read(pipe_stream, &sizeChar, 1);
        if(sizeChar == ',')
            break;
        width *= 10;
        width += (sizeChar - '0');
    }

    while(1)
    {
        read(pipe_stream, &sizeChar, 1);
        if(sizeChar == ',')
            break;
        height *= 10;
        height += (sizeChar - '0');
    }
    //std::cout << std::endl << "DIM:" << width << "x" << height;
    //clearerr(this->pipe_stream);
//    int dimensions = fscanf(pipe_stream, "%d,%d", &width, &height);
//    std::cout << width << "x" << height << ",";
//    if (dimensions == EOF && ferror(pipe_stream)) {
//        std::cerr << strerror(errno)
//                  << "Error occured while parsing the pipe stream."
//                  << std::endl;
//    }
//    else if (dimensions != 2) {
//        std::cerr << "Error: Wrong format of the data in the pipe." << dimensions
//                  << std::endl;
//        width = 0;
//        height = 0;
//        //exit(-1);
//    }
}

void NCursesClient::print_tanks()
{
    char sector;
    int x = 0, y = 0;
    parse_dimensions();
    char buffer[4];

    while (y < height) {
//        if (fscanf(pipe_stream, ",%c", &sector) == EOF) {
//            std::cerr << strerror(errno)
//                      << "Error occured while parsing the pipe stream."
//                      << std::endl;
//            break;
//        }

        read(pipe_stream, buffer, 4);
        sector = buffer[1];
        //std::cout << "[" << x << "," << y << "]_" << sector << " ";

        std::cout << (sector == '0' ? ' ' : sector) << ' ';
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
        default:
            std::cerr << "Wrong map format in the pipe, char:" << sector << std::endl;
            //assert(false);
        }
        x++;
        if (x >= this->width) {
            x = 0;
            y++;
            std::cout << std::endl;
        }

    }
    //wrefresh(nc_world);
    //keys();
    close(pipe_stream);
    pipe_stream = open(mPipe, O_RDONLY); //we need to reopen the pipe for block
}

void NCursesClient::keys()
{
//    char c = (char)getch();
//    switch (c) {
//    case 'x':
//        kill(world_pid, SIGINT);
//        break;
//    case 'r':
//        kill(world_pid, SIGUSR1);
//        break;
//    case 'q':
//        exit(0);
//        break;
//    }
}

void NCursesClient::draw_tank(int x, int y, Color color)
{
//    wattrset(nc_world, COLOR_PAIR(color));
//    /* Compensate for border padding */
//    mvwaddch(nc_world, y + 1, x + 1, ACS_BLOCK);
//    /* type cast enum class color */
//    wattroff(nc_world, COLOR_PAIR(color));
//    wrefresh(nc_world);
}

void NCursesClient::undraw_tank(int x, int y)
{
//    /* COLOR_PAIR(0) sets the default color */
//    wattrset(nc_world, COLOR_PAIR(0));
//    /* Compensate for border padding */
//    mvwaddch(nc_world, y + 1, x + 1, ' ');
//    wattroff(nc_world, COLOR_PAIR(0));
//    wrefresh(nc_world);
}
