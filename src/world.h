#ifndef WORLD_H
#define WORLD_H

#include <utility>
#include <cstring>
#include <sstream>
#include <boost/range/join.hpp>

#include <errno.h>
#include <syslog.h>
#include <getopt.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>

#include "tank.h"


// Utility type definitions
typedef unsigned int uint;
// mutex for writing commands
pthread_mutex_t mtx;
// conditional variable to control writing messages
pthread_cond_t cvar;
// messages coming from tanks, to be processed
std::vector<std::string> tank_messages;

/**
 * @brief set_up_signal_handling uses sigaction function
 * to set up World::set_world_signal_status(int sig) as signal handler
 */

void set_up_signal_handling();

/**
 * @brief checks whether an instance of world is already running
 * @param pid_filepath location of pid file to check
 */
int world_running( std::string pid_filepath );

/**
 * @brief blocking call which waits for the end of other world instances
 * @param pid_filepath location of pid file to check
 */
void watch_pid( std::string pid_filepath );

/**
 * @brief Utility class for holding important data
 */
class WorldOptions
{
    bool daemonize;
    uint mRoundTime;
    uint mMapHeight;
    uint mMapWidth;
    std::string green_tank_path;
    std::string red_tank_path;
    std::string fifo_path;
    uint mGreenTanks;
    uint mRedTanks;
    uint red_kills;
    uint green_kills;
    uint rounds_played;

public:
    WorldOptions();

    void parse_options(int argc, char* argv[]);

    void print_help();

    void print_error();

    bool get_daemonize() { return this->daemonize; }

    uint getRoundTime() { return this->mRoundTime; }

    uint get_map_height() { return this->mMapHeight; }

    uint get_map_width() { return this->mMapWidth; }

    std::string getGreenPath() { return this->green_tank_path; }

    std::string getRedPath() { return this->red_tank_path; }

    uint get_green_tanks() { return this->mGreenTanks; }

    uint get_red_tanks() { return this->mRedTanks; }

    uint getRedKills() { return this->red_kills; }

    uint getGreenKills() { return this->green_kills; }

    uint getRoundsPlayed() { return this->rounds_played; }

    std::string get_fifo_path() { return this->fifo_path; }

    void incRedKills()
    {
        this->red_kills++;
    }

    void incGreenKills()
    {
        this->green_kills++;
    }

    void incRoundsPlayed()
    {
        this->rounds_played++;
    }

};

/**
 * @brief Represents an in-game basic world
 */
class World
{
protected:
    std::vector<Tank> green_tanks;
    std::vector<Tank> red_tanks;
    std::vector< std::vector<Color> > zone; ///< Holds the state of a map >
    uint height;
    uint width;
    pthread_cond_t tank_cond_com;
    pthread_mutex_t tank_mutex_com;
    std::vector<int> tank_messages;
    static volatile sig_atomic_t world_signal_status;

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

    ~World() {
        close();
    }

    /**
     * @brief Spawns a tank at given coordinates, which must be empty
     * @param t info about tank to spawn
     * @param u Utils instance with tank binary path
     */
    void add_tank(Tank& t, WorldOptions u);

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
    void play_round(WorldOptions u);

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
    void fire_direction(Tank& t);

    /**
     * @brief moves tanks if they weren't hit and have received a move order
     * @param tanks tanks possibly ordered to move
     * @param actions orders to said tanks, fire orders now ignored
     */
    void movetanks();

    /**
     * @brief checks for tanks running into each other,
     *          healthy tanks running into hit ones do not crash
     * note: allied tanks can crash into each other
     */
    void crash_tanks();

    /**
     * @brief adds kills according to tanks hit; crashes count
     * since as long as a tank is destroyed, the other side benefits from it,
     * the reason why the tank is out of action is irrelevant
     * @param u Utils class instance into which kill counts are written
     */
    void add_kills(WorldOptions u);

    /**
     * @brief removes hit tanks from the board
     */
    void remove_hit_tanks();

    /**
     * @brief respawns tanks at end of round
     * @param u Utils class instance holding necessary info
     *  (how many tanks each side fields)
     */
    void respawn_tanks(WorldOptions u);

    /**
     * @brief cleans up world's resources
     */
    void close();

    /**
     * @brief prints map info to cout
     */
    void output_map();

    void process_commands(WorldOptions u, std::vector<std::string> ra,
                          std::vector<std::string> ga);

    /**
     * @brief set_world_signal_status handler to pass caught signal into a flag
     * in an atomic way
     * @param sig caught signal
     */
    static void set_world_signal_status(int sig);

    /**
     * @brief handle_signal is used to check flag World::world_signal_status
     * for any caught signals and act upon them
     * @param sig causing the interuption
     */
    void handle_signal(int sig);
};

#endif // WORLD_H
