#include "world.h"

Utils::Utils(int argc, char *argv[]) :
    mExit(false),
    mDaemonize(false),
    mGreenPath(""),
    mRedPath("")

{
    struct option longopts[] = {
            { "daemonize",     no_argument, NULL, 'd' },
            { "green-tanks",   required_argument, NULL, 'g' },
            { "red-tanks",     required_argument, NULL, 'r' },
            { "green-tank", required_argument, NULL, 'p' },
            { "red-tank", required_argument, NULL, 'q' },
            { "round-time", required_argument, NULL, 't' },
            { "area-size",     required_argument, NULL, 'a' },
            { "help",          no_argument,       NULL, 'h' },
            { 0, 0, 0, 0 }
        };
        int option_index = 0;
        int c;
        while ((c = getopt_long(argc, argv, "dg:r:p:q:t:a:h", longopts, NULL)) != -1) {
            switch (c) {
            case 'a':
                this->mMapWidth = atoi(argv[optind-1]);
                this->mMapHeight = atoi(argv[optind]);
                break;
            case 'g':
                this->mGreenTanks = atoi(optarg);
                //green_team = (Tankprocess*) malloc(sizeof(Tankprocess)*tanks_green);
                //TODO: rewrite to C++
                break;
            case 'r':
                this->mRedTanks = atoi(optarg);
                //red_team = (Tankprocess*) malloc(sizeof(Tankprocess)*tanks_red);
                //TODO: -||-
                break;
            case 'd' :
                this->mDaemonize = true;
                break;
            case 'q':
                this->mRedPath.append(optarg);
                break;
            case 'p' :
                this->mGreenPath.append(optarg);
                break;
            case 't':
                this->mRoundTime = atoi(optarg);
                break;
            case 'h':
                this->printHelp();
                exit(0);
            default:
                this->printError();
                exit(-1);
            }
        }
        // printf("X: %i, Y: %i\n", areaX, areaY);
        /* we need these opts */
        if(mMapHeight<=0 || mMapWidth<=0 || mRoundTime <=0) {
           this->printHelp();
            exit(-1);
        }
}

void Utils::printHelp()
{
       printf("=====================================================\n");
       printf("|         PB173 Internet Of Tanks presents:  WORLD  |\n");;
       printf("-----------------------------------------------------\n");
       printf("                    USAGE                            \n");
       printf("  -h | --help           Show this help               \n");
       printf("  --daemonize           run as daemon                \n");
       printf("  --green-tanks [n]     create n green tanks         \n");
       printf("  --red-tanks [n]       create n red tanks           \n");
       printf("  --green-tank [s]      path to green tank bin       \n");
       printf("  --red-tank [s]        path to red tank bin         \n");
       printf("  --area-size [n] [m]   size of area NxM             \n");
       printf("  --round-time [n]      time of one round in ms      \n");
       printf("=====================================================\n");
}

void Utils::printError()
{
    fprintf(stderr, "Wrong arguments or something\n");
}



int main(int argc, char *argv[])
{

    Utils *mUtils = new Utils(argc, argv);
    /*
     * Use of UTILS:
     * rewrite all classes to have (in constructor) the pointer to mUtils.
     * then just use i.e. mUtils->getRoundTime() for getting the time of one round
     * EXAMPLE:
    */

    std::cout << "green tanks: " << mUtils->getGreenTanks() << " red tanks: "
              << mUtils->getRedTanks() << std::endl
              << "green path: " << mUtils->getGreenPath() << std::endl
              << "red path:" << mUtils->getRedPath() << std::endl
              << "area size: " << mUtils->getMapWidth() << "x" << mUtils->getMapHeight()
              << std::endl << "round time: " << mUtils->getRoundTime() << std::endl;

    int pid_file = open("/var/run/world.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        /* Another instance is running */
        if(EWOULDBLOCK == errno);
    }
    else {
        /* this is the first instance */
    }

    int green_tanks = 15;
    int red_tanks = 15;
    int tanks_number = green_tanks + red_tanks;
    NCursesWorld w(15, 30);
    for (int i = 0; i < green_tanks; i++) {
        Tank t = Tank(w.free_coord(), Color::GREEN);
        w.add_tank(t);
    }
    for (int i = 0; i < red_tanks; i++) {
        Tank t = Tank(w.free_coord(), Color::RED);
        w.add_tank(t);
    }

    while (tanks_number > 0) {
        pid_t stopped_tank = wait(NULL);
        tan
    }

}
