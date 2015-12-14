#ifndef WORLD_H
#define WORLD_H

#include <utility>
#include <sstream>
#include <boost/range/join.hpp>
#include <memory>

#include <errno.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>

#include "world_options.h"
#include "tank.h"

/**
 * @brief process_signal_handling uses sigaction function
 * to set up World::set_world_signal_status(int sig) as signal handler
 */
void process_signal_handling();

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
 * @brief Represents an in-game basic world
 */
class World
{
private:
    std::vector<std::unique_ptr<Tank>> green_tanks;
    std::vector<std::unique_ptr<Tank>> red_tanks;
    //std::vector<Tank> green_tanks;
    //std::vector<Tank> red_tanks;

    std::vector< std::vector<Color> > zone; ///< Holds the state of a map >
    unsigned height;
    unsigned width;
    static volatile sig_atomic_t world_signal_status;
    std::string pipe;
    int pipefd;

public:
    /**
     * @brief World constructor, also gets a pseudorandom seed
     *  and sets the whole world to empty (no tanks on battlefield)
     * @param height height of the world (Y-axis)
     * @param width width of the world (X-axis)
     * @param pipe pipe to write events to
     */
    //World(uint height, uint width, std::string pipe);
    World(WorldOptions& opts);

    /**
     * @brief World remove the copy constructor
     */
    World(const World&) = delete;

    ~World() {
        close();
    }

private:
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

public:
    /**
     * @brief add_tank spawns tank on free coordinates
     * @param color of the new tank
     */
    void add_tank(Color color);

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
     * @brief fires the main guns of all give tanks
     */
    void fire();

    /**
     * @brief tank t fires in a specifis direction based on his action attribute
     */
    void fire_direction(Tank&);
    //void fire_direction(std::unique_ptr<Tank> t);

    /**
     * @brief moves tanks if they weren't hit and have received a move order
     * @param tanks tanks possibly ordered to move
     */
    void movetank(Tank&);
    //void movetank(std::unique_ptr<Tank> t);

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
    void remove_dead_tanks();

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

    /**
     * @brief read_commands fetch commands for all tanks
     */
    void read_commands();

    /**
     * @brief set_world_signal_status handler to pass caught signal into a flag
     * in an atomic way
     * @param sig caught signal
     */
    static void set_world_signal_status(int sig, siginfo_t *info, void *uctx);

    /**
     * @brief handle_signal is used to check flag World::world_signal_status
     * for any caught signals and act upon them
     * @param sig causing the interuption
     */
    void handle_signal(int sig);

    /**
     * @brief refreshes battlefield status at end of round for
     *          correct output
     */
    void refresh_zone();
    void take_actions();
    void process_commands();
    bool check_bounds(int, int);
    bool check_bounds(Coord);
    void remove_tank(Tank& t);
};

#endif // WORLD_H
