#ifndef TANKCLIENT_H
#define TANKCLIENT_H

#include <unistd.h>
#include <getopt.h>
#include <ctime>
#include <csignal>
#include <iostream>

struct TankOptions {
    int mMapHeight;
    int mMapWidth;
    bool mExit;

public:
    TankOptions(int argc, char *argv[]);
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

    TankClient(TankOptions *utils);
    ~TankClient();
    void waitForSignal();
    void nextMove();
    bool sendCommand(Command command);

    const char * commandToSend;
    static const char moveUp[3];
    static const char moveDown[3];
    static const char moveLeft[3];
    static const char moveRight[3];
    static const char fireUp[3];
    static const char fireDown[3];
    static const char fireLeft[3];
    static const char fireRight[3];

private:
    TankOptions *mUtils;
    Command lastCommand;
    bool wasLastMove;
    bool lastCommandSuccess;

    int mWidthPos;
    int mHeightPos;
};

#endif // TANKCLIENT_H
