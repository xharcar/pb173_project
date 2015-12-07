#ifndef TANK_H
#define TANK_H

#include "world_shared.h"
#include <queue>

/**
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    std::thread t_handle;
    int x;
    int y;
    Color color;
    std::string tankclient_path; ///< Process to be spawned
    std::string action; ///< used to hold a copy of a last command

    // fixmme: attacker attribute could possibly be avoided by erasing tank
    // at the time of tank being hit or moved
    TankShell attacker;
    bool hit;

    std::queue<std::string> command_buffer;
    /* indirection required for moveing tanks to vectors */
    std::unique_ptr<std::condition_variable> com; ///< used to wait for socket communication thread if command_buffer is empty
    std::unique_ptr<std::mutex> com_mut; ///< synchronizing acces to command_buffer

    //volatile std::sig_atomic_t signal_status = 0;
    int signal_status;

public:
    /**
     * @brief Tank constructor, sets TID to 0(to indicate not yet initialized
     * properly) and hit flag to false(when a tank rolls up onto a battlefield,
     * it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color, std::string bin_path);

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
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void request_command();

    /**
     * @brief set tank to be hit if fired upon by foe and remember the attacker
     * @param attacker the shooting tank
     */
    void hit_tank(TankShell attacker);

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

    /**
     * @brief deposit_command_from_client is called from a socket communication's thread
     * @param command to be deposited
     */
    void deposit_command_from_client(std::string command);

    /**
     * @brief Tank::read_command is called from a main thread to obtain a command
     * command is then copied to the this->action
     * @return last command
     */
    //std::string read_command();
    void read_command();
};

struct TankShell
{
    int x;
    int y;
    Color color;
    TankShell(int x, int y, Color c) : x(x), y(y), color(c) {}
    TankShell() = default;
};

void tank_sig_handler(int sig);

#endif // TANK_H
