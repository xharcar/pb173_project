#ifndef TANK_H
#define TANK_H

#include "world_shared.h"

struct TankShell
{
    Color color;
    int x;
    int y;
};

/**
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    std::thread t_handle;
    int pfd [2];
    bool hit;
    int x;
    int y;
    Color color;
    std::string action;
    // fixmme: attacker attribute could possibly be avoided by erasing tank
    // at the time of tank being hit or moved
    TankShell attacker;
    volatile std::sig_atomic_t tank_signal_status = 0;

public:
    /**
     * @brief Tank constructor, sets TID to 0(to indicate not yet initialized
     * properly) and hit flag to false(when a tank rolls up onto a battlefield,
     * it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color) : hit(false), x(x), y(y), color(color) {}

    /**
     * @brief hit flag getter
     * @return true if tank has been hit, else false
     */
    bool get_hit() const { return this->hit; }

    /**
     * @brief X coordinate getter
     * @return tank x coordinate
     */
    uint get_x() const { return this->x; }

    /**
     * @brief Y coordinate getter
     * @return tank y coordinate
     */
    uint get_y() const { return this->y; }

    /**
     * @brief getPosition
     * @return tank position
     */
    Coord get_position() const { return Coord(x, y); }

    /**
     * @brief color getter
     */
    Color get_color() const { return this->color; }

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
     * @brief change tank's coordinates
     */
    void moveleft() { this->x--; }
    void moveright() { this->x++; }
    void moveup() { this->y++; }
    void movedown() { this->y--; }

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
     * @brief set tank to be hit if fired upon by foe and remember the attacker
     * @param attacker the shooting tank
     */
    void hit_tank(Tank& attacker);

    /**
     * @brief sends SIFTERM to the thread handle of tank
     */
    void kill_thread();

    /**
     * @brief waits for tank thread to end
     */
    void quit();

    /**
     * @brief print_destroy print tank's info after destruction and attackers info
     */
    void print_destroy();
};

void tank_sig_handler(int sig);

#endif // TANK_H
