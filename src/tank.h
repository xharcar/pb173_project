#ifndef TANK_H
#define TANK_H

#include "world_shared.h"

/**
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    std::thread tid;
    int pfd [2];
    bool hit;
    uint x;
    uint y;
    Color color;
    std::string action;
    Tank& attacker;
public:

    /**
     * @brief Tank constructor, sets TID to 0(to indicate not yet initialized properly)
     *  and hit flag to false(when a tank rolls up onto a battlefield, it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color) : hit(false), x(x), y(y), color(color) {}

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
    void spawn_thread();

    /**
     * @brief handles newly created tank thread
     * @param pipeptr pointer pipe from which world reads tank commands
     */
    void* handle_thread(void* pipeptr);

    /**
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void request_command();

    /**
     * @brief communicates with spawned tank
     */
    void read_command();

    /**
     * @brief set tank to be hit if fired upon by foe
     * @param c color of the shooting tank
     */
    void Tank::hit_tank(Tank& attacker)

    /**
     * @brief change tank's coordinates
     */
    void moveleft() { this->x--; }
    void moveright() { this->x++; }
    void moveup() { this->y++; }
    void movedown() { this->y--; }

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
