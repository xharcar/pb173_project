#pragma once

#include "world_shared.h"

/**
 * @brief Represents a tank in-game
 */
class TankClient
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
     * @brief TankClient constructor, sets TID to 0(to indicate not yet initialized properly)
     *  and hit flag to false(when a tank rolls up onto a battlefield, it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    TankClient(uint x, uint y, Color color) : tid(0), hit(false), x(x), y(y), color(color){}

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

    std::string get_action() const {
        return this->action;
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

    /**
     * @brief communicates with spawned tank
     */
    void read_action();

    /**
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void req_com();

    void request_command();

    /**
     * @brief read command from a pipe
     */
    void read_command();

    /**
     * @brief set tank to be hit if fired upon by foe
     * @param c color of the shooting tank
     */
    void hit_tank(Color c);

    void moveleft();
    void moveright();
    void moveup();
    void movedown();
};

void tank_sig_handler(int sig);

/**
 * @brief runs a tank
 * @param tankpipe pipe to send orders to world through
 */
int run_tank(int* tankpipe);

void* handle_thread(void* tankpipe);

void spawn_thread(TankClient t, std::string tankpath);
