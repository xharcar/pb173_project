#pragma once

// C++ includes
#include <utility> // pair
#include <cstring> // strtok
#include <iostream> // i/o
#include <sstream> // stringstream
#include <boost/range/join.hpp>

// Legacy C/Linux includes
#include <errno.h> // errno
#include <syslog.h> // logging
#include <getopt.h> // options
#include <sys/file.h> // FIFO, flock
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // flock
#include <sys/inotify.h>

#include "tankclient.h"


// Utility type definitions
typedef std::pair<int, int> Coord;
typedef unsigned int uint;
// mutex for writing commands
pthread_mutex_t mtx;
// conditional variable to control writing messages
pthread_cond_t cvar;
// messages coming from tanks, to be processed
std::vector<std::string> tank_messages;

/**
 * @brief checks whether an instance of world is already running
 * @param pid_filepath location of pid file to check
 */
void world_running( char* pid_filepath );

/**
 * @brief blocking call which waits for the end of other world instances
 * @param pid_filepath location of pid file to check
 */
void watch_pid( char* pid_filepath );

/**
 * @brief Utility class for holding important data
 */
class Utils
{
    bool mDaemonize;
    uint mRoundTime;
    uint mMapHeight;
    uint mMapWidth;
    std::string mGreenPath;
    std::string mRedPath;
    std::string fifoPath;
    uint mGreenTanks;
    uint mRedTanks;
    bool mExit;
    uint red_kills;
    uint green_kills;
    uint rounds_played;

public:
    Utils(int argc, char *argv[]);
    void printHelp();
    void printError();

    bool getDaemonize()
    {
        return this->mDaemonize;
    }
    uint getRoundTime()
    {
        return this->mRoundTime;
    }
    uint getMapHeight()
    {
        return this->mMapHeight;
    }
    uint getMapWidth()
    {
        return this->mMapWidth;
    }
    std::string getGreenPath()
    {
        return this->mGreenPath;
    }
    std::string getRedPath()
    {
        return this->mRedPath;
    }
    uint getGreenTanks()
    {
        return this->mGreenTanks;
    }
    uint getRedTanks()
    {
        return this->mRedTanks;
    }
    bool getExit()
    {
        return this->mExit;
    }
    void incRedKills(){
        this->red_kills++;
    }
    void incGreenKills(){
        this->green_kills++;
    }
    uint getRedKills(){
        return this->red_kills;
    }
    uint getGreenKills(){
        return this->green_kills;
    }
    uint getRoundsPlayed(){
        return this->rounds_played;
    }
    void incRoundsPlayed(){
        this->rounds_played++;
    }
    std::string getFifoPath(){
        return this->fifoPath;
    }
};

/**
 * @brief Represents an in-game basic world
 */
class World
{
protected:
    std::vector<TankClient> green_tanks;
    std::vector<TankClient> red_tanks;
    std::vector< std::vector<Color> > zone;
    uint height;
    uint width;
    pthread_cond_t tank_cond_com;
    pthread_mutex_t tank_mutex_com;
    std::vector<int> tank_messages;
public:
    /**
     * @brief World constructor, also gets a pseudorandom seed
     *  and sets the whole world to empty (no tanks on battlefield)
     * @param height height of the world (Y-axis)
     * @param width width of the world (X-axis)
     */
    World(uint height, uint width) : height(height), width(width)
    {
        std::vector<std::vector<Color> > zone(height,std::vector<Color>(width,EMPTY));
        pthread_mutex_init(&this->tank_mutex_com, NULL);
    }

    /**
     * @brief Spawns a tank at given coordinates, which must be empty
     * @param t info about tank to spawn
     * @param u Utils instance with tank binary path
     */
    void add_tank(TankClient t, Utils u);

    /**
     * @brief Checks if given map coordinate is free
     * @param x x coordinate
     * @param y y coordinate
     * @return true if coordinate is free, else false
     */
    bool is_free(int x, int y);

    /**
     * @brief gives a random set of free coordinates for a tank to spawn at
     * @return coordinates to spawn new tank at
     */
    Coord free_coord();

    /**
     * @brief represents a round of gameplay;
     *  first, all actions sent are read;
     *  second, firing is processed: every fire command is executed;
     *  next, depending on tank positioning, hit flags are set
     *  (this emulates all guns firing at the same time);
     *  then, whatever hasn't got hit and has been ordered to move, moves;
     *  finally, all hits are accounted for(tanks deleted off the map)
     *  and respawns occur
     * @param u Utils class instance holding necessary data
     */
    void play_round(Utils u);

    /**
     * @brief refreshes battlefield status at end of round for
     *          correct output
     */
    void refresh_zone();

    /**
     * @brief sends all tanks SIGUSR2 as a request for commands
     */
    void req_com();

    /**
     * @brief reads commands from tanks
     */
    void read_com();

    /**
     * @brief fires the main guns of all give tanks
     */
    void fire();

    /**
     * @brief tank t fires in a specifis direction based on his action attribute
     */
    void fire_direction(TankClient& t);

    /**
     * @brief moves tanks if they weren't hit and have received a move order
     * @param tanks tanks possibly ordered to move
     * @param actions orders to said tanks, fire orders now ignored
     */
    void movetanks();

    /**
     * @brief checks for tanks running into each other,
     *          healthy tanks running into hit ones do not crash
     * @param tanks1 set of tanks possibly crashing into others
     * @param tanks2 set of tanks tanks from tanks1 can run into
     * note: allied tanks can crash into each other
     */
    void crash_tanks(std::vector<TankClient> tanks1,
                     std::vector<TankClient> tanks2);

    /**
     * @brief adds kills according to tanks hit; crashes count
     * since as long as a tank is destroyed, the other side benefits from it,
     * the reason why the tank is out of action is irrelevant
     * @param u Utils class instance into which kill counts are written
     */
    void add_kills(Utils u);

    /**
     * @brief removes hit tanks from the board
     */
    void remove_hit_tanks();

    /**
     * @brief respawns tanks at end of round
     * @param u Utils class instance holding necessary info
     *  (how many tanks each side fields)
     */
    void respawn_tanks(Utils u);

    /**
     * @brief cleans up world
     * @param signal signal which is reacted upon
     */
    void quit_safe(int signal);

    /**
     * @brief prints map info to cout
     */
    void output_map();
};

