#ifndef TANK_H
#define TANK_H

#include "world_shared.h"
#include <queue>
#include <functional>

enum class TankState {
    alive,
    shot,
    crashed,
};

/**
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    int x;
    int y;
    Color color;

    TankState state;
    std::string command; ///< used to hold a copy of a last command
    Coord new_position;

    std::thread t_handle;
    std::queue<std::string> command_buffer;
    std::condition_variable com; ///< used to wait for socket communication thread if command_buffer is empty
    std::mutex com_mut; ///< synchronizing acces to command_buffer

    //volatile std::sig_atomic_t signal_status = 0;
    int signal_status;

public:

    /**
     * @brief Tank constructor and hit flag to false (when a tank rolls up onto
     * a battlefield, it is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(int x, int y, Color color);

    Tank(Coord position, Color color);

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

    bool check_bounds(int height, int width)
    {
        return (x < 0 || x > width || y < 0 || y > height);
    }

    void get_shot() { state = TankState::shot; }

    void get_crashed() { state = TankState::crashed; }

    bool is_shot() const { return state == TankState::shot; }

    bool is_alive() const { return state == TankState::alive; }

    void set_new_position(Coord new_position) { this->new_position = new_position; }

    void move()
    {
        std::cout << *this << "moved to [" << new_position.first << ", "
                  << new_position.second << "]" << std::endl;
        x = new_position.first;
        y = new_position.second;
    }

    /**
     * @brief spawns a new tank thread, initialized TID of tank
     * @param tankpath path to tank binary to be executed
     */
    // fixme: add argument for socket passing
    void spawn_thread()
    {
        /* Spawn a thread to communicate with tankclient asynchronously */
        this->t_handle = std::thread([&]() {});
    }

    /**
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void request_command()
    {
        /* fixme: rewrite using conditional_variable or make specific thread
         * handles */
        pthread_kill(t_handle.native_handle(), SIGUSR2);
    }

    /**
     * @brief set tank to be hit if fired upon by foe and remember the attacker
     * @param attacker the shooting tank
     */
    //void hit_tank(TankShell attacker);

    /**
     * @brief sends SIFTERM to the thread handle of tank
     */
    void kill_thread()
    {
        pthread_kill(t_handle.native_handle(), SIGTERM);
    }

    /**
     * @brief waits for tank thread to end
     */
    void quit() { t_handle.join(); }

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
    void print_crashed(const Tank& t) const;

    /**
     * @brief print_destroy print tank's info after destruction and attackers info
     */
    void print_destroyed(const Tank& attacker) const;

    void print_out_of_map() const;
};

#endif // TANK_H
