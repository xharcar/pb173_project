#pragma once

#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <iostream>

class Utils {
    int mMapHeight;
    int mMapWidth;
    bool mExit;

public:
    Utils(int argc, char *argv[]);
    ~Utils();
    void printHelp();
    void printError();

    int getMapHeight() {return this->mMapHeight; }
    int getMapWidth() {return this->mMapWidth; }
    bool getExit() {return this->mExit; }
};

class Tank {
public:
    enum Command {
        MOVE_UP = 0, MOVE_DOWN = 1, MOVE_LEFT = 2, MOVE_RIGHT = 3, FIRE_UP = 4, FIRE_DOWN = 5, FIRE_LEFT = 6, FIRE_RIGHT = 7
    } ;

    Tank(Utils *utils);
    ~Tank();
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
    Utils *mUtils;
    Command lastCommand;
    bool wasLastMove;
    bool lastCommandSuccess;

    int mWidthPos;
    int mHeightPos;
};
