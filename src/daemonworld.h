#ifndef DAEMONWORLD_H
#define DAEMONWORLD_H

#include "world.h"

/**
 * @brief Represents a daemonized world
 */
class DaemonWorld : public World
{
private:
    std::string pipe;
    int pipefd;
public:
    /**
     * @brief Constructor
     * @param height world height (Y dimension)
     * @param width world width (X dimension)
     * @param pipe pipe to write events to
     */
    DaemonWorld(int height, int width, std::string pipe) : World(height, width), pipe(pipe)
    {
         openlog("Internet of Tanks: World", LOG_PID, LOG_USER);
         //pipefd = mkfifo(pipe.c_str(),0444);
    }

    ~DaemonWorld()
    {
        closelog();
    }
    /**
     * @brief Spawns a tank at given coordinates, which must be empty
     * @param t info about tank to spawn
     * @param u Utils instance with tank binary path
     * @override World::add_tank
     */
    void add_tank(Tank t, WorldOptions u);

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
     * @brief refreshes battlefield status at end of round for
     *          correct output
     */
    void refresh_zone();

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
     * @brief cleans up world
     * @param signal signal which is reacted upon
     * @override World::quit_safe
     */
    void quit_safe(int signal);

    /**
     * @brief prints map info to cout
     * @override World::output_map
     */
    void output_map();

};

#endif // DAEMONWORLD_H
