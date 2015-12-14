#include "world_options.h"

WorldOptions::WorldOptions()
    // Set default values here
    : daemonize(false)
    , green_tankclient_path("../bin/tankclient")
    , red_tankclient_path("../bin/tankclient")
    , red_kills(0)
    , green_kills(0)
    , rounds_played(0)
{}

void WorldOptions::parse_options(int argc, char* argv[])
{
    struct option longopts[] =
    {
        { "daemonize",   no_argument,       NULL, 'd' },
        { "green-tanks", required_argument, NULL, 'c' },
        { "red-tanks",   required_argument, NULL, 'v' },
        { "green-tank",  required_argument, NULL, 'g' },
        { "red-tank",    required_argument, NULL, 'r' },
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
            this->mMapWidth = atoi(argv[optind - 1]);
            this->mMapHeight = atoi(argv[optind]);
            break;
        case 'c':
            this->mGreenTanks = atoi(optarg);
            break;
        case 'v':
            this->mRedTanks = atoi(optarg);
            break;
        case 'd':
            this->daemonize = true;
            break;
        case 'r':
            this->red_tankclient_path.assign(optarg);
            break;
        case 'g':
            this->green_tankclient_path.assign(optarg);
            break;
        case 't':
            this->mRoundTime = atoi(optarg);
            break;
        case 'p':
            this->fifo_path.assign(optarg);
            break;
        case 'h':
            this->print_help();
            exit(0);
        default:
            this->print_error();
            exit(-1);
        }
    }
    if(mMapHeight<=0 || mMapWidth<=0 || mRoundTime <=0)
    {
        this->print_help();
        exit(-1);
    }
}

void WorldOptions::print_help()
{
    std::cout <<"=====================================================" << std::endl
              <<"|         PB173 Internet Of Tanks presents:  WORLD  |" << std::endl
              <<"-----------------------------------------------------" << std::endl
              <<"                    USAGE                            " << std::endl
              <<"  -h | --help           Show this help               " << std::endl
              <<"  --daemonize           run as daemon                " << std::endl
              <<"  --green-tanks [n]     create n green tanks         " << std::endl
              <<"  --red-tanks [n]       create n red tanks           " << std::endl
              <<"  --green-tank [s]      path to green tank bin       " << std::endl
              <<"  --red-tank [s]        path to red tank bin         " << std::endl
              <<"  --area-size [n] [m]   size of area NxM             " << std::endl
              <<"  --round-time [n]      time of one round in ms      " << std::endl
              <<"  -p | --pipe [file]    path to a named FIFO to output map to in daemon mode" << std::endl
              <<"=====================================================" << std::endl;
}

void WorldOptions::print_error()
{
    std::cerr << "Wrong arguments or something" << std::endl;
}
