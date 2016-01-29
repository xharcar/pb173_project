#include "tankclient.h"


volatile std::sig_atomic_t wasSigUsr2 = 0;
volatile std::sig_atomic_t wasExitSig = 0;

void TankOptions::parse(int argc, char *argv[])
{
    struct option longopts[] = {
        {"area-size", required_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "a:h", longopts, NULL)) != -1) {
        switch (c) {
        case 'a':
            this->mMapWidth = atoi(argv[optind - 1]);
            this->mMapHeight = atoi(argv[optind]);
            break;
        case 'h':
            this->print_help();
            exit(0);
        default:
            this->print_error();
            exit(-1);
        }
    }
    // printf("X: %i, Y: %i\n", areaX, areaY);
    /* we need these opts */
    if (mMapHeight <= 0 || mMapWidth <= 0) {
        this->print_help();
        exit(-1);
    }
}

void TankOptions::print_error()
{
    fprintf(stderr, "Wrong arguments, use -h for help\n");
    exit(-1);
}

void TankOptions::print_help()
{
    printf("=====================================================\n");
    printf("|         PB173 Internet Of Tanks presents:         |\n");
    printf("|            _________    _   ____ __               |\n");
    printf("|           /_  __/   |  / | / / //_/               |\n");
    printf("|            / / / /| | /  |/ / ,<                  |\n");
    printf("|           / / / ___ |/ /|  / /| |                 |\n");
    printf("|          /_/ /_/  |_/_/ |_/_/ |_|                 |\n");
    printf("-----------------------------------------------------\n");
    printf("                        USAGE                            \n");
    printf("  -h | --help           Show this help                   \n");
    printf("  --area-size [n]x[m]   size of area   \n");
    printf("=====================================================\n");
}

TankClient::TankClient(TankOptions& utils)
    : mUtils(utils),
      wasLastMove(false), // temporary
      lastCommandSuccess(true)
{
    srand(time(NULL));
}

void TankClient::waitForSignal()
{
    pause();    //waiting
    if(wasSigUsr2 == 1)
    {
        wasSigUsr2 = 0;
        this->nextMove();
    }
    else if(wasExitSig == 1)
    {
        delete this;
        exit(0);
    }

}

void TankClient::nextMove()
{
    //as completly different process i don't know my initial position
    //so i am going to fire everywhere BADAAAAAAAAA
    //and move randomly to test ncurses
    switch (this->lastCommand)
    {
    case FIRE_UP:
        this->lastCommand = FIRE_RIGHT;
        break;
    case FIRE_RIGHT:
        this->lastCommand = FIRE_DOWN;
        break;
    case FIRE_DOWN :
        this->lastCommand = FIRE_LEFT;
        break;
    default :
        this->lastCommand = FIRE_UP;
    }

    if (this->wasLastMove)
    {
        this->wasLastMove = false;
    }
    else
    {
        this->wasLastMove = true;
        this->lastCommand = (Command)(rand() % 4);
    }

    this->sendCommand(this->lastCommand);


}

bool TankClient::sendCommand(Command command)
{
    switch (command)
    {
    case Command::MOVE_UP :
        this->commandToSend = this->moveUp;
        break;
    case Command::MOVE_DOWN :
        this->commandToSend = this->moveDown;
        break;
    case Command::MOVE_LEFT :
        this->commandToSend = this->moveLeft;
        break;
    case Command::MOVE_RIGHT :
        this->commandToSend = this->moveRight;
        break;
    case Command::FIRE_UP :
        this->commandToSend = this->fireUp;
        break;
    case Command::FIRE_DOWN :
        this->commandToSend = this->fireDown;
        break;
    case Command::FIRE_LEFT :
        this->commandToSend = this->fireLeft;
        break;
    case Command::FIRE_RIGHT :
        this->commandToSend = this->fireRight;
        break;
    }

    if (write(STDOUT_FILENO, this->commandToSend, 2) < 2)
    {
        std::cerr << "something went wrong while writing to pipe (too lazy to syslog)" << std::endl;
        return false;
    }
    return true;
}
void signal_handler(int sig)
{
    switch(sig)
    {
    case SIGUSR2:
        wasSigUsr2 = 1;
        break;
    case SIGINT :
    case SIGTERM:
    case SIGQUIT:
        wasExitSig = 1;
        break;
    default:
        break;
    }
}

int main(int argc, char* argv[])
{
    TankOptions opts;
    opts.parse(argc, argv);

    TankClient tank(opts);

    while(true)
    {
        tank.waitForSignal();
    }
}

