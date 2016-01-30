#ifndef WORLD_H
#define WORLD_H

#include "tank.h"
#include "world_options.h"

#include <fstream>
#include <unordered_set>

/**
 * @brief setup_signal_handling sets up World::set_world_signal_status()
 * as signal handler
 */
void setup_signal_handling();

/**
 * @brief Represents an in-game basic world
 */
class World
{
private:
    std::vector<Tank*> tanks;
    int height;
    int width;
    std::vector< std::vector<Color> > zone; ///< Holds the state of a map >
    unsigned red_tanks = 0;
    unsigned green_tanks = 0;

    static volatile std::atomic<int> world_signal_status;
	// sig_atomic_t may default to int, useful ops implementation-dependent;
    int map_fifo;

    WorldOptions opts;
    unsigned red_kills = 0;
    unsigned green_kills = 0;
    unsigned rounds_played = 0;


public:
    /**
     * @brief World constructor, also gets a pseudorandom seed
     *  and sets the whole world to empty (no tanks on battlefield)
     * @param opts WorldOptions instance used to create world
     * @param pd pipe descriptor
     */
    World(WorldOptions& opts,int pd);

    ~World() { close(); }

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
    void play_round();

    /**
     * @brief respawn_tanks respawns dead tanks
     */
    void respawn_tanks();

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
     * @brief compute score based on shot down tanks
     */
    void sum_score();

    /**
     * @brief prints map info to cout
     */
    void output_map();

    /**
     * @brief restart
     */
    void restart();

    void init_tanks();	

    /**
     * @brief set_world_signal_status handler to pass caught signal into a flag
     * in an atomic way
     * @param sig caught signal
     */
    static void set_world_signal_status(int sig,siginfo_t* info, void* arg);

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
     * @param order order in vector for identification
     */
    void add_tank(Color,int order);

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
    void fire_direction(Tank*);

    /**
     * @brief movetank moves tank if it hasn't been shot and have received a move order
     */
    void movetank(Tank*);

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
