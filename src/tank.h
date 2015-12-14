#ifndef TANK_H
#define TANK_H

#include "world_shared.h"
#include <queue>
#include <functional>

void tank_sig_handler(int sig);

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
    bool dead;
    std::string tankclient_path; ///< Process to be spawned
    std::string command; ///< used to hold a copy of a last command

    // fixmme: attacker attribute could possibly be avoided by erasing tank
    // at the time of tank being hit or moved
    //TankShell attacker;

    std::queue<std::string> command_buffer;
    std::condition_variable com; ///< used to wait for socket communication thread if command_buffer is empty
    std::mutex com_mut; ///< synchronizing acces to command_buffer

    //volatile std::sig_atomic_t signal_status = 0;
    int signal_status;
    std::function<bool ()> action;
    //std::function<void (const Tank&)> destroyed;
    //std::function<void (const Tank&)> crashed;

public:

    /**
     * @brief Tank constructor and hit flag to false (when a tank rolls up onto
     * a battlefield, it is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color);

    Tank(Coord position, Color color);

    //Tank(const Tank&) = delete;
    //Tank& operator=(const Tank&) = delete;
    //Tank(Tank&&) noexcept = default;
    //Tank& operator=(Tank&&) noexcept = default;

    /**
     * @brief X coordinate getter
     * @return tank x coordinate
     */
    int get_x() const { return this->x; }

    /**
     * @brief Y coordinate getter
     * @return tank y coordinate
     */
    int get_y() const { return this->y; }

    /**
     * @brief getPosition
     * @return tank position
     */
    Coord get_position() const { return Coord(x, y); }

    /**
     * @brief color getter
     */
    Color get_color() const { return this->color; }

    std::string get_command() const { return this->command; }

    /**
     * @brief change tank's coordinates
     */
    bool move(int height, int width, Coord c);

    /*
    void moveleft() { this->x--; }
    void moveright() { this->x++; }
    void moveup() { this->y++; }
    void movedown() { this->y--; }
    */

    bool check_bounds(int height, int width)
    {
        return (x < 0 || x > width || y < 0 || y > height);
    }

    void make_dead() { dead = true; }

    bool is_dead() { return dead; }

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
    //void hit_tank(TankShell attacker);

    /**
     * @brief sends SIFTERM to the thread handle of tank
     */
    void kill_thread();

    /**
     * @brief waits for tank thread to end
     */
    void quit();


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

    bool take_action();

    /**
     * @brief subscribe_action saves one action that is to be performed after
     * all tank's commands are processed
     * @param action to be saved
     */
    void subscribe_action(std::function<bool ()> action);
    //void set_crash(std::function<void ()> action);

    friend std::ostream& operator<<(std::ostream&, const Tank&);

    /**
     * @brief print_crashed prints collided tank's infos
     * @param t tank that has been crashed into
     */
    void print_crashed(const Tank& t);

    /**
     * @brief print_destroy print tank's info after destruction and attackers info
     */
    void print_destroyed(const Tank& attacker);
};

#endif // TANK_H
