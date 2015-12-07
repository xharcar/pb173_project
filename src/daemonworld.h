#ifndef DAEMONWORLD_H
#define DAEMONWORLD_H

#include "world.h"

/**
 * @brief Represents a daemonized world
 */
class DaemonWorld : public World
{
public:
    /**
     * @brief Constructor
     * @param height world height (Y dimension)
     * @param width world width (X dimension)
     * @param pipe pipe to write events to
     */
    DaemonWorld(int height, int width, std::string pipe) : World(height, width, pipe)
    {
         openlog("Internet of Tanks: World", LOG_PID, LOG_USER);
    }

    ~DaemonWorld()
    {
        close();
    }

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
     * @brief removes hit tanks from the board
     */
    void remove_hit_tanks();

    void close();

};

#endif // DAEMONWORLD_H
