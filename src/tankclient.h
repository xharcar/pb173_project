#ifndef TANKCLIENT_H
#define TANKCLIENT_H

#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <csignal>
#include <iostream>

struct TankOptions {
    int mMapHeight;
    int mMapWidth;
    bool mExit = false;

public:
    void parse(int argc, char *argv[]);
    void print_help();
    void print_error();

    int get_map_height() { return this->mMapHeight; }
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
        FIRE_RIGHT = 7
    };

    TankClient(TankOptions& utils);
    void waitForSignal();
    void nextMove();
    bool sendCommand(Command command);

    const char * commandToSend;
    const char moveUp[2] = {'m', 'u'};
    const char moveDown[2] = {'m', 'd'};
    const char moveLeft[2] = {'m', 'l'};
    const char moveRight[2] = {'m', 'r'};
    const char fireUp[2] = {'f', 'u'};
    const char fireDown[2] = {'f', 'd'};
    const char fireLeft[2] = {'f', 'l'};
    const char fireRight[2] = {'f', 'r'};

private:
    TankOptions mUtils;
    Command lastCommand;
    bool wasLastMove;
    bool lastCommandSuccess;

    int mWidthPos;
    int mHeightPos;
};

#endif // TANKCLIENT_H
