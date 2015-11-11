#pragma once

// C++ includes
#include <utility> // pair
#include <string> // strings
#include <vector> // map is a 2D vector
#include <cstdlib> // rand(),malloc()
#include <iostream> // i/o
#include <sstream> // stringstream
#include <ctime> // time(0)

// Legacy C/Linux includes
#include <errno.h> // errno
#include <unistd.h> // usleep
#include <pthread.h> // threads
#include <syslog.h> // logging
#include <getopt.h> // options
#include <sys/file.h> // FIFO, flock
#include <sys/types.h> // mkfifo
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // flock
#include <signal.h>


// Utility type definitions
typedef std::pair<int, int> Coord;
typedef unsigned int uint;
enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};
typedef struct{
    int pfd[2];
    char binpath[256];
}tankutils;


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
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    pthread_t tid;
    int pfd [2];
    bool hit;
    uint x;
    uint y;
    Color color;
public:

    /**
     * @brief Tank constructor, sets TID to 0(to indicate not yet initialized properly)
     *  and hit flag to false(when a tank rolls up onto a battlefield, it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color) : tid(0), hit(false), x(x), y(y), color(color){}

    /**
     * @brief TID getter (for sending signals,...)
     * @return TID of tank thread
     */
    pthread_t getTID();

    /**
     * @brief TID setter
     * @param x TID to be set
     */
    void setTID(pthread_t x);

    /**
     * @brief hit flag getter
     * @return true if tank has been hit, else false
     */
    bool getHit();

    /**
     * @brief hit flag setter (used only when tank has been hit)
     * @param shot indicates whether tank has been hit(~true)
     */
    void setHit(bool shot);

    /**
     * @brief X coordinate getter
     * @return tank x coordinate
     */
    uint getX();

    /**
     * @brief Y coordinate getter
     * @return tank y coordinate
     */
    uint getY();

    /**
     * @brief X coordinate setter
     */
    void setX(int newx);

    /**
     * @brief Y coordinate setter
     */
    void setY(int newy);

    /**
     * @brief color getter
     */
    Color getColor();

    /**
     * @brief pipe read end getter for commands
     */
    int getPipe(){
        return this->pfd[0];
    }

    int* getpfd(){
        return this->pfd;
    }

    /**
     * @brief spawns a new tank thread, initialized TID of tank
     * @param tankpath path to tank binary to be executed
     */
    void spawn_thread(std::string tankpath);

    /**
     * @brief handles newly created tank thread
     * @param pipeptr pointer pipe from which world reads tank commands
     */
    void* handle_thread(void* pipeptr);

};

/**
 * @brief Represents an in-game basic world
 */
class World
{
protected:
    std::vector<Tank> green_tanks;
    std::vector<Tank> red_tanks;
    std::vector< std::vector<Color> > zone;
    uint height;
    uint width;
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
    }

    /**
     * @brief Spawns a tank at given coordinates, which must be empty
     * @param t info about tank to spawn
     * @param u Utils instance with tank binary path
     */
    void add_tank(Tank t, Utils u);

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
     * @param red red tank commands vector
     * @param green tank commands vector
     */
    void read_com(std::vector<std::string> red,
                  std::vector<std::string> green);

    /**
     * @brief fires the main guns of all give tanks
     * @param tanks tanks possibly ordered to fire
     * @param actions orders to tanks, non-fire (=move) orders ignored for now
     */
    void fire(std::vector<Tank> tanks,
              std::vector<std::string> actions);

    /**
     * @brief moves tanks if they weren't hit and have received a move order
     * @param tanks tanks possibly ordered to move
     * @param actions orders to said tanks, fire orders now ignored
     */
    void movetanks(std::vector<Tank> tanks,
                   std::vector<std::string> actions);

    /**
     * @brief checks for tanks running into each other,
     *          healthy tanks running into hit ones do not crash
     * @param tanks1 set of tanks possibly crashing into others
     * @param tanks2 set of tanks tanks from tanks1 can run into
     * note: allied tanks can crash into each other
     */
    void crash_tanks(std::vector<Tank> tanks1,
                     std::vector<Tank> tanks2);

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
    void add_tank(Tank t, Utils u);

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
     * @param red red tank commands vector
     * @param green tank commands vector
     */
    void read_com(std::vector<std::string> red,
                  std::vector<std::string> green);

    /**
     * @brief fires the main guns of all give tanks
     * @param tanks tanks possibly ordered to fire
     * @param actions orders to tanks, non-fire (=move) orders ignored for now
     */
    void fire(std::vector<Tank> tanks,
              std::vector<std::string> actions);

    /**
     * @brief moves tanks if they weren't hit and have received a move order
     * @param tanks tanks possibly ordered to move
     * @param actions orders to said tanks, fire orders now ignored
     */
    void movetanks(std::vector<Tank> tanks,
                   std::vector<std::string> actions);

    /**
     * @brief checks for tanks running into each other,
     *          healthy tanks running into hit ones do not crash
     * @param tanks1 set of tanks possibly crashing into others
     * @param tanks2 set of tanks tanks from tanks1 can run into
     * note: allied tanks can crash into each other
     */
    void crash_tanks(std::vector<Tank> tanks1,
                     std::vector<Tank> tanks2);

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
     * @override World::quit_safe
     */
    void quit_safe(int signal);

    /**
     * @brief prints map info to cout
     * @override World::output_map
     */
    void output_map();

};

