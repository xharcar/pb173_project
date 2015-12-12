#ifndef TANK_H
#define TANK_H

#include  "world_shared.h"
#include <netdb.h>
#include <iostream>
#include <sys/socket.h>
#include <sstream>

static pthread_mutex_t worldmtxlock;
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
    std::string action;
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
    bool isHit();

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

    int* getpfd(){
        return this->pfd;
    }

    std::string get_action() const {
        return this->action;
    }

    /**
     * @brief spawns a new tank thread, initialized TID of tank
     * @param tankpath path to tank binary to be executed
     */
    void spawn_thread();

    /**
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void request_command();

    /**
     * @brief set tank to be hit if fired upon by foe
     * @param c color of the shooting tank
     */
    void hit_tank();

    /**
     * @brief change tank coordinates
     */
    void moveleft();
    void moveright();
    void moveup();
    void movedown();

    /**
     * @brief sends SIFTERM to the thread handle of tank
     */
    void kill_thread();

    /**
     * @brief waits for tank thread to end
     */
    void quit();
};

void tank_sig_handler(int sig);


#endif // TANK_H
