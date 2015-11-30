#include "tankclient.h"


TankOptions::TankOptions() :
    mExit(false)
{}

void TankOptions::parse(int argc, char *argv[])
{
    struct option longopts[] = {
            { "area-size",     required_argument, NULL, 'a' },
            { "help",          no_argument,       NULL, 'h' },
            { 0, 0, 0, 0 }
        };
        int option_index = 0;
        int c;
        while ((c = getopt_long(argc, argv, "a:h", longopts, NULL)) != -1) {
            switch (c) {
            case 'a':
                this->mMapWidth = atoi(argv[optind-1]);
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
        if(mMapHeight<=0 || mMapWidth<=0) {
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
    printf("  --address   address to connect to   \n");
    printf("  --port      port. This implementation   \n");
    printf("              Every tank has it's own port in this'         \n");
    printf("              implementation  \n");
    printf("=====================================================\n");

    ;
}

TankClient::TankClient(TankOptions& utils)
    : mUtils(utils),
      lastCommand(NO_COMMAND),
      lastCommandSuccess(true),
      wasLastMove(false),
      threadControl(true),
      commandWanted(false)
{
    srand(time(NULL));
    keyThread = new std::thread(&TankClient::readKey, this);
}

TankClient::~TankClient()
{
    this->threadControl = false;
    if(keyThread->joinable())
        keyThread->join();
    delete keyThread;
}

//running in separate thread
//we have the latest command every time
void TankClient::readKey()
{
    char c;
    std::string str;
    std::cout << "reading commans.." << std::endl;
    while(threadControl)
    {
        std::cin >> str;
        c = str.at(0);
        commandMutex.lock();
        switch(c)
        {
        case 'a' :
            this->lastCommand = MOVE_LEFT;
            break;
        case 'w' :
            this->lastCommand = MOVE_UP;
            break;
        case 's' :
            this->lastCommand = MOVE_DOWN;
            break;
        case 'd' :
            this->lastCommand = MOVE_RIGHT;
            break;
        case 'i' :
            this->lastCommand = FIRE_UP;
            break;
        case 'j' :
            this->lastCommand = FIRE_LEFT;
            break;
        case 'k' :
            this->lastCommand = FIRE_DOWN;
            break;
        case 'l' :
            this->lastCommand = FIRE_RIGHT;
            break;
        case ' ' :
            this->lastCommand = NO_COMMAND;
        }
        commandMutex.unlock();

    }
}



void TankClient::waitForSignal()
{
//    pause();    //waiting
//    if(wasSigUsr2 == 1)
//    {
//        wasSigUsr2 = 0;
//        this->nextMove();
//    }
//    else if(wasExitSig == 1)
//    {
//        delete this;
//        exit(0);
//    }


    tmpSet = master;
    if (select(sock+1, &tmpSet, NULL, NULL, NULL) == -1) {
        perror("select");
        return;
    }
    int readBytes;
    if (FD_ISSET(sock, &tmpSet)) {
        if ((readBytes = recv(sock, buffer, sizeof(buffer), 0)) <= 0) {
            // Got error or connection closed by server
            if (readBytes == 0) {
                // Connection closed
                printf("Connection closed by server\n");
                return;
            } else {
                perror("recv");
                return;
            }
        } else {
            if (readBytes != 2)
                std::cerr << "wrong size of data from server" << std::endl;
            // We got some data from the server
            if(!commandWanted)
            {
                if(strcmp(buffer, requestFromServer) == 0)
                    commandWanted = true;
                else if(chrToCommand(buffer) != lastCommand)
                    std::cerr << "Server acknowledged wrong command" << std::endl;
            }

            else
            {
                if(!this->sendCommand(this->lastCommand))
                    std::cerr << "error sending command" << std::endl;
            }
        }
    }
}

//deprecated
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
    case Command::NO_COMMAND :
        this->commandToSend = this->noCommand;
        break;
    default :
        std::cerr << "wrong command, wtf" << std::endl;

    }
    commandMutex.unlock();

//    if (write(STDOUT_FILENO, this->commandToSend, 2) < 2)
//    {
//        std::cerr << "something went wrong while writing to pipe (too lazy to syslog)" << std::endl;
//        return false;
//    }

    if (send(sock, this->commandToSend, 2, 0) == -1) {
        return false;
    }

    return true;
}

bool TankClient::connectTo()
{
    memset (&remhint, 0, sizeof(remhint));
    remhint.ai_family = AF_INET;
    remhint.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(mUtils.get_address(), mUtils.get_port(), &remhint, &remaddr ) != 0)
    {
        std::cerr << "Cannot get server info" << std::endl;
        return false;
    }

    sock = socket(remaddr->ai_family, remaddr->ai_socktype, remaddr->ai_protocol);
    if (sock == -1)
    {
        std::cerr << "cannot create sockets" << std::endl;
        return false;
    }

    if(connect(sock, remaddr->ai_addr, remaddr->ai_addrlen) == -1)
    {
        std::cerr << "cannot cnnect to server " << strerror(errno) << std::endl;
        return false;
    }
    freeaddrinfo(remaddr);

    FD_ZERO(&master);
    FD_SET(sock, &master);
    FD_SET(0, &master);
    std::cout << "successfully connected to " << mUtils.get_address() << ":" << mUtils.get_port() << std::endl;

    return true;
}

TankClient::Command TankClient::chrToCommand(char *chr)
{
    if((strcmp(chr, noCommand) == 0))
        return NO_COMMAND;
    else if((strcmp(chr, moveUp) == 0))
        return MOVE_UP;
    else if((strcmp(chr, moveDown) == 0))
        return MOVE_DOWN;
    else if((strcmp(chr, moveLeft) == 0))
        return MOVE_LEFT;
    else if((strcmp(chr, moveRight) == 0))
        return MOVE_RIGHT;
    else if((strcmp(chr, fireUp) == 0))
        return FIRE_UP;
    else if((strcmp(chr, fireDown) == 0))
        return FIRE_DOWN;
    else if((strcmp(chr, fireLeft) == 0))
        return FIRE_LEFT;
    else if((strcmp(chr, fireRight) == 0))
        return FIRE_RIGHT;
    else if((strcmp(chr, requestFromServer) == 0))
        return REQUEST;
    else
        return WRONG_COMMAND;
}







int main(int argc, char* argv[])
{
    TankOptions opts;
    opts.parse(argc, argv);

    TankClient tank(opts);

    bool connected = false;
    while(true)
    {
        connected = tank.connectTo();
        if(connected)
            break;
        std::cout << "Trying to reconnect" << std::endl;
        sleep(5);
    }

    /*
    while(true)
    {
        tank.waitForSignal(); //everything is here
    }
    */
}

