#ifndef TANKCLIENT_H
#define TANKCLIENT_H

#include <unistd.h>
#include <getopt.h>
#include <ctime>
#include <csignal>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>


#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct TankOptions {
    int mMapHeight;
    int mMapWidth;
    bool mExit;
    char* mAddress;
    char* mPort;

public:
    TankOptions();
    void parse(int argc, char *argv[]);
    void print_help();
    void print_error();

    int get_map_height() { return this->mMapHeight; }
    char *get_address() { return this->mAddress; }
    char *get_port() { return this->mPort; }
    int get_map_width() { return this->mMapWidth; }
    bool get_exit() { return this->mExit; }
};

class TankClient {
public:
    enum Command {
        MOVE_UP = 0,
        MOVE_DOWN = 1,
        MOVE_LEFT = 2,
        MOVE_RIGHT = 3,
        FIRE_UP = 4,
        FIRE_DOWN = 5,
        FIRE_LEFT = 6,
        FIRE_RIGHT = 7,
        NO_COMMAND = 8,
        REQUEST = 9,
        WRONG_COMMAND = 10
    };

    TankClient(TankOptions& utils);
    ~TankClient();
    void waitForSignal();
    void readKey();

    void nextMove();
    bool sendCommand(Command command);

    bool connectTo();


    const char * commandToSend;
    const char moveUp[2] = {'m', 'u'};
    const char moveDown[2] = {'m', 'd'};
    const char moveLeft[2] = {'m', 'l'};
    const char moveRight[2] = {'m', 'r'};
    const char fireUp[2] = {'f', 'u'};
    const char fireDown[2] = {'f', 'd'};
    const char fireLeft[2] = {'f', 'l'};
    const char fireRight[2] = {'f', 'r'};
    const char noCommand[2] = {'n', 'o'};
    const char requestFromServer[2] = {'r', 'e'};
private:
    std::thread *keyThread;
    //socks stuff
    bool checkConnected(); //TODO
    void getAddress(struct sockaddr *ai_addr, char **address); //TODO
    Command chrToCommand(char *chr);
    struct addrinfo *remaddr, remhint;
    int sock;
    fd_set master, tmpSet;
    char *address;
    char buffer[2];
    std::mutex commandMutex;

private:
    TankOptions mUtils;
    Command lastCommand;
    bool wasLastMove;
    bool lastCommandSuccess;

    bool threadControl;
    bool commandWanted;


    int mWidthPos;
    int mHeightPos;
};

#endif // TANKCLIENT_H
