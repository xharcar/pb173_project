#include "tank.h"

bool Tank::createServer()
{

        int myPortInt = 10000;
        myPortInt += this->color == RED ? 100 : 0; // + 100 for RED tanks - not controlling already opened ports at this time
        myPortInt += this->vector_order; // + order in vector of tanks in world
        myPort = std::to_string(myPortInt);

        memset(&myhints, 0, sizeof(struct addrinfo));
        myhints.ai_socktype = SOCK_STREAM;
        myhints.ai_family = AF_INET;
        myhints.ai_flags = AI_PASSIVE;

        FD_ZERO(&master);
        FD_ZERO (&tmpSet);

        if (getaddrinfo(NULL, myPort.c_str(), &myhints, &myaddr) != 0) {
            printf("Cannot get server IP address\n");
            return false;
        }

        // Create listener socket
        listener = socket(myaddr->ai_family, myaddr->ai_socktype, myaddr->ai_protocol);
        if (listener == -1) {
            printf("Cannot create new socket%s\n", myPort.c_str());
            return false;
        }

        fdMax = listener;
        FD_SET(listener, &master);

        if (bind(listener, myaddr->ai_addr, myaddr->ai_addrlen) != 0) {
                printf("Cannot bind socket to address\n");
                return false;
            }


        int numOfListenError;
        numOfListenError = listen(listener, 15);

        switch(errno)
        {
        case EADDRINUSE:
            std::cerr << "Address is in use" << std::endl;
            return false;
        case EBADF:
            std::cerr << "bad descriptor" << std::endl;
            return false;
        case ENOTSOCK:
            std::cerr << "not a socket" << std::endl;
            return false;
        case EOPNOTSUPP:
            std::cerr << "not a supported type" << std::endl;
            return false;
        }

//        if (listen(listener, 15) != 0) {
//            printf("Cannot listen on socket");
//            return false;
//        }

        freeaddrinfo(myaddr);
        std::cout << "TANK: " << (this->color == RED ? "RED_" : "GREEN_") << vector_order << " Spawned and listening on " << myPort << std::endl;
        fdMax = listener;
        return true;
}

void Tank::serverLoop()
{
    if(!serverCreated)
        serverCreated = this->createServer();
        // Run until the cows come home
        while (1) {
            // work with copy of master set
            tmpSet = master;
            // wait for data
            if (select(fdMax+1, &tmpSet, NULL, NULL, NULL) == -1) {
                perror("select");
                return;
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
                                return;
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
                                        this->command = std::string(buf);
                                        std::cout << "we got command! " << buf << std::endl;
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
            *address = (char*)malloc(INET_ADDRSTRLEN);
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
            *address = (char*)malloc(INET6_ADDRSTRLEN);
            if (!inet_ntop(AF_INET6, &(in6_addr.s6_addr), *address, INET6_ADDRSTRLEN)) {
                printf("Cannot parse IPv6 address\n");
                return;
            }
        }
}

Tank::Tank(int x, int y, Color color, int order)
    : x(x), y(y), color(color), state(TankState::alive), vector_order(order), command("no"), serverCreated(false)
{
    //std::cout << "Spawning " << this << std::endl;
    this->serverThread = new std::thread(&Tank::serverLoop, this);
    /*
    newSock = -1;
    clientConnected = false;
    t_handle = std::thread(&Tank::serverLoop);
    */
}

Tank::Tank(Coord position, Color color, int order)
    : Tank(position.first, position.second, color, order) {}

std::ostream& operator<<(std::ostream& os, const Tank t)
{
    std::string color = t.get_color() == Color::RED ? "Red" : "Green";
    os << "Tank [" << t.get_x() << ", " << t.get_y() << "] [" << color << "] ";
    return os;
}

void Tank::print_destroyed(Tank *t) const
{
    std::cout << this << "destroyed by " << t << std::endl;
}

void Tank::print_crashed(Tank *t) const
{
    std::cout << this << "crashed into " << t << std::endl;
}

void Tank::print_out_of_map() const {
    std::cout << this << " rolled out of battlefield" << std::endl;
}

void Tank::move()
{
    std::cout << this << "moved to [" << new_position.first << ", "
              << new_position.second << "]" << std::endl;
    x = new_position.first;
    y = new_position.second;
}

void Tank::revive(){
    state = TankState::alive;
}

//void Tank::deposit_command_from_client(std::string command)
//{
//    std::unique_lock<std::mutex> lock(com_mut);
//    command_buffer.push(command);
//    lock.unlock();
//    com.notify_one();
//}


//void Tank::mock_read_command()
//{
//    std::uniform_int_distribution<int> r_op(0,1);
//    std::uniform_int_distribution<int> r_dir(0,3);
//    //command[0] = std::string("am")[r_op(rng)];
//    //command[1] = std::string("lurd")[r_dir(rng)];

//    //command = std::string("am"[r_op(rng)]) + std::string("lurd"[r_dir(rng)]);
//    command = std::string() + "fm"[r_op(rng)] + "lurd"[r_dir(rng)];
//    //std::cout << "DEBUG: " << command << std::endl;
//    //std::cout << "DEBUG: " << command[0] << command[1] << std::endl;
//}

