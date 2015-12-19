#include "world_options.h"

WorldOptions::WorldOptions()
    /* Set default values here */
    : daemonize(false)
    , red_kills(0)
    , green_kills(0)
    , rounds_played(0)
{}

int WorldOptions::parse_options(int argc, char* argv[])
{
    struct option longopts[] =
    {
        { "daemonize",   no_argument,       NULL, 'd' },
        { "green-tanks", required_argument, NULL, 'c' },
        { "red-tanks",   required_argument, NULL, 'v' },
        { "round-time",  required_argument, NULL, 't' },
        { "area-size",   required_argument, NULL, 'a' },
        { "pipe",        required_argument, NULL, 'p' },
        { "help",        no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "d:c:v:g:r:t:a:p:h", longopts, NULL)) != -1)
    {
        switch (c) {
        case 'a':
            mMapWidth = atoi(argv[optind - 1]);
            mMapHeight = atoi(argv[optind]);
            break;
        case 'c':
            mGreenTanks = atoi(optarg);
            break;
        case 'v':
            mRedTanks = atoi(optarg);
            break;
        case 'd':
            daemonize = true;
            break;
        case 't':
            round_time = atoi(optarg);
            break;
        case 'p':
            fifo_path.assign(optarg);
            break;
        case 'h':
            print_help();
            return EXIT_FAILURE;
        default:
            print_error();
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int WorldOptions::check_valid() const
{
    if (mMapHeight <= 0 || mMapWidth <= 0 || round_time <= 0) {
        std::cout << "Map height, width and round time options can not be below zero"
                  << std::endl;
        return EXIT_FAILURE;
    }
    /* Checking if map space is sufficient */
    int map_space = get_map_height() * get_map_width();
    int tank_count = get_green_tanks() + get_red_tanks();
    if (map_space < tank_count) {
        std::cout << "Not enough space on map for tanks, exiting" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void WorldOptions::print_help() const
{
    std::cout <<"=====================================================" << std::endl
              <<"|         PB173 Internet Of Tanks presents:  WORLD  |" << std::endl
              <<"-----------------------------------------------------" << std::endl
              <<"                    USAGE                            " << std::endl
              <<"  -h | --help           Show this help               " << std::endl
              <<"  --daemonize           run as daemon                " << std::endl
              <<"  --green-tanks [n]     create n green tanks         " << std::endl
              <<"  --red-tanks [n]       create n red tanks           " << std::endl
              <<"  --area-size [n] [m]   size of area NxM             " << std::endl
              <<"  --round-time [n]      time of one round in ms      " << std::endl
              <<"  -p | --pipe [file]    path to a named FIFO to output map to in daemon mode" << std::endl
              <<"=====================================================" << std::endl;
}

void WorldOptions::print_error() const
{
    std::cerr << "Wrong arguments" << std::endl;
}
