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

    std::vector< std::vector<Color> > zone; ///< Holds the state of a map >
    int height;
    int width;

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

    /**
     * @brief represents a round of gameplay;
     * 1) Tanks are spawned on the map
     * 2) All recieved actions are read
     * 3) Firing is processed, flag is set on shot tanks
     *    (this emulates all guns firing at the same time)
     * 4) Remaining tanks move:
     *      4.1) flag is set for crashed tanks and tanks out of map boundary
     *      4.2) remaining tanks obtain new coordinates
     * 5) Destroyed tanks are removed and remaining tanks are moved to new coordinates
     */
    void play_round(WorldOptions);

    /**
     * @brief respawn_tanks respawns dead tanks
     */
    void respawn_tanks(WorldOptions);

    /**
     * @brief read_commands fetch commands for all tanks
     */
    void read_commands();

    /**
     * @brief process_shots process fire commands
     */
    void process_shots();

    /**
     * @brief process_moves process move commands
     */
    void process_moves();

    /**
     * @brief take_actions move tanks to new positions and remove dead tanks
     */
    void take_actions();

    /**
     * @brief adds kills according to tanks hit; crashes count
     * since as long as a tank is destroyed, the other side benefits from it,
     * the reason why the tank is out of action is irrelevant
     * @param u Utils class instance into which kill counts are written
     */
    void add_kills(WorldOptions u);

    /**
     * @brief prints map info to cout
     */
    void output_map();

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

    /**
     * @brief add_tank spawns tank on free coordinates
     * @param color of the new tank
     */
    void add_tank(Color);

    /**
     * @brief take_action_tank auxiliary method for take_actions()
     */
    void take_action_tank(std::vector<std::unique_ptr<Tank>>::iterator&);

    /**
     * @brief out_of_bounds check wether giver coordinates are outside of map
     * @return true iff outside of map
     */
    bool out_of_bounds(Coord);

    /**
     * @brief tank fires in a specified direction based on his command
     */
    void fire_direction(Tank&);

    /**
     * @brief movetank moves tank if it hasn't been shot and have received a move order
     */
    void movetank(Tank&);

    /**
     * @brief set_world_signal_status handler to pass caught signal into a flag
     * in an atomic way
     * @param sig caught signal
     */
    static void set_world_signal_status(int sig, siginfo_t *info, void *uctx);

    /**
     * @brief handle_signal is used to check flag World::world_signal_status
     * for any caught signals and act upon them
     */
    bool handle_signals();

    /**
     * @brief refreshes battlefield status at end of round for correct output
     */
    void refresh_zone();

    /**
     * @brief cleans up world's resources
     */
    void close();

};

#endif // WORLD_H
