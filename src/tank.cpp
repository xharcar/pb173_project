#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;

void Tank::request_command()
{
    pthread_kill(t_handle.native_handle(), SIGUSR2);
}

void Tank::read_command()
{
    char buf[4] = "\0";
    read(this->getPipe(), buf, 3);
    this->action = std::string(buf);
}

void Tank::hit_tank(Tank& attacker)
{
    if (attacker.get_color() != this->color)
    {
        this->hit = true;
        this->attacker.color = attacker.get_color();
        this->attacker.x = attacker.get_x();
        this->attacker.y = attacker.get_y();
    }
}

void Tank::kill_thread()
{
    // pthread_kill(t_handle.native_handle(), SIGTERM);
}

void Tank::quit()
{
    t_handle.join();
}

bool Tank::createServer()
{
    // = (pid_t)this->serverThread->get_id().; //TODO
    srand((int)this->tid);
    myPort = rand() % 50000 + 1025;

    memset(&myhints, 0, sizeof(struct addrinfo));
    myhints.ai_socktype = SOCK_DGRAM;
    myhints.ai_family = AF_INET;
    myhints.ai_flags = AI_PASSIVE;

    FD_ZERO(&master);
    FD_ZERO (&tmpSet);

    if (getaddrinfo(NULL, myPort.c_str(), &myhints, &myaddr) != 0) {
        printf("Cannot get server IP address\n");
        return false;
    }

    // Create listener socket
    listener = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
    if (listener == -1) {
        printf("Cannot create new socket%s\n", port);
        return false;
    }

    FD_SET(listener, &master);


    if (listen(listener, 15) != 0) {
        printf("Cannot listen on socket");
        return false;
    }
    freeaddrinfo(myaddr);
    std::cout << "Tank TID " << this->tid << " listening on port " << myPort << std::endl;
    fdMax = listener;
    return true;

}

void Tank::serverLoop()
{
    this->createServer();
    // Run until the cows come home
    while (1) {
        // work with copy of master set
        tmpSet = master;
        // wait for data
        if (select(fdMax+1, &tmpSet, NULL, NULL, NULL) == -1) {
            perror("select");
            return 1;
        }
        // run through the existing connections looking for data to read
        int i;
        for(i = 0; i <= fdMax; i++) {
            // socket i has data to read
            if (FD_ISSET(i, &tmpSet)) {
                if (i == listener) {
                    if(!clientConnected)
                    {
                        // handle new connections
                        struct sockaddr_in sockaddr;
                        socklen_t sockaddrLength = sizeof(sockaddr);
                        newSock = accept(listener, (struct sockaddr*) &sockaddr, &sockaddrLength);
                        if (newSock == -1) {
                            printf("Cannot accept connection\n");
                            return 1;
                        }
                        // add to master set
                        FD_SET(newSock, &master);
                        // watch out for new maximum
                        if (newSock > fdMax) {
                            fdMax = newSock;
                        }
                        char *remoteAddr;
                        getAddress((struct sockaddr*) &sockaddr, &remoteAddr);
                        printf("Connection accepted from IP %s\n", remoteAddr);
                        free(remoteAddr);
                    }
                    else
                    {
                        std::cerr << "Different client tried to connect";
                    }
                } else {
                    // handle data from a client
                    int readBytes;
                    char buf[2];
                    if ((readBytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (readBytes == 0) {
                        // connection closed
                        printf("Socket %d is closed\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                                    if (send(i, buf, readBytes, 0) == -1) {
                                        perror("send");
                                    }
                                    this->action = std::string(buf);
                                }
                            }
                        }
                    }
                }
            }


void Tank::getAddress(sockaddr *ai_addr, char **address)
{
    // IPv4 address
    if (ai_addr->sa_family == AF_INET) {
        struct sockaddr_in *sockaddr_in;
        sockaddr_in = (struct sockaddr_in*) ai_addr;
        struct in_addr in_addr;
        in_addr = sockaddr_in->sin_addr;
        *address = malloc(INET_ADDRSTRLEN);
        if (!inet_ntop(AF_INET, &(in_addr.s_addr), *address, INET_ADDRSTRLEN)) {
            printf("Cannot parse IPv4 address\n");
            return;
        }
    }
    // IPv6 address
    if (ai_addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *sockaddr_in6;
        sockaddr_in6 = (struct sockaddr_in6*) ai_addr;
        struct in6_addr in6_addr;
        in6_addr = sockaddr_in6->sin6_addr;
        *address = malloc(INET6_ADDRSTRLEN);
        if (!inet_ntop(AF_INET6, &(in6_addr.s6_addr), *address, INET6_ADDRSTRLEN)) {
            printf("Cannot parse IPv6 address\n");
            return;
        }
    }
}

void tank_sig_handler(int sig){
    switch (sig) {
    case SIGUSR2:
        tank_send = 1;
        break;
    case SIGTERM:
        tank_exit = 1;
        break;
    }
}

/**
 * @brief runs a tank
 * @param tankpipe pipe to send orders to world through
 */
int run_tank(int socket){
    // fixme: move srand initialization to class/global scope
    std::srand(std::time(0));
    /*
    int x = 0;
    while(tank_exit==0){
        x = std::rand() % 8;
        if(tank_send){
            write(tankpipe[1],commands[x].c_str(),3);
        }
    }
    return 0;
    */
}

void Tank::spawn_thread()
{
    this->t_handle = std::thread(run_tank, NULL);
}

void Tank::print_destroy() {
    std::cout << "Tank destroyed: ";
    if (color == Color::RED) {
        std::cout << "Red";
    } else {
        std::cout << "Green";
    }
    // fixme: possibly return the PID of a running Tankclient
    std::cout << ", " << t_handle.get_id() << ", [" << get_x() << ", " << get_y()
              << "]" << std::endl;
    // fixme:: possibly overload stream operator for Tank to print out tank info
    std::cout << "Attacker: " << attacker.color << ", [" << attacker.x << ", "
              << attacker.y << "]" << std::endl;
}
