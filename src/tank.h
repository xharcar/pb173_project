#ifndef TANK_H
#define TANK_H

#include "world_shared.h"

#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>

#include <pthread.h>

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
    int x, y;
    Color color;
    TankState state;
    int vector_order;
    std::string command; ///< used to hold a copy of a last command
    Coord new_position;  ///< Position the tank takes after movements of all
                         ///other tanks have been processed

    //std::thread t_handle;
    //std::queue<std::string> command_buffer; //deprecated, we just need one variable for this
    volatile std::condition_variable com; ///< used to wait for socket communication
                                 ///thread if command_buffer is empty
    std::mutex com_mut;          ///< synchronizing acces to command_buffer

    std::condition_variable request;
    std::mutex request_mut;
    //volatile std::sig_atomic_t signal_status = 0;
    int request_status;

    //server thingies
    struct addrinfo *myaddr, myhints;
    int listener, newSock, fdMax;
    std::string myPort;
    fd_set master, tmpSet;
    bool clientConnected;

    std::thread *serverThread;
    bool threadControl;
    bool serverCreated;

    bool createServer();
    void serverLoop();
    void getAddress(struct sockaddr *ai_addr, char **address);


public:
    /**
     * @brief Set up tank state to alive (when a tank rolls up onto a
     * battlefield, it is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(int x, int y, Color color, int order);

    Tank(Coord position, Color color, int order);

    int get_x() const { return x; }

    int get_y() const { return y; }

    Coord get_position() const { return Coord(x, y); }

    Color get_color() const { return color; }

    std::string get_command() const { return command; }

    void get_shot() { state = TankState::shot; }

    void get_crashed() { state = TankState::crashed; }

    bool is_shot() const { return state == TankState::shot; }

    bool is_alive() const { return state == TankState::alive; }
    
    void quit(){
	send_request(SIGTERM);
    }

    int get_order(){
        return vector_order;
    }
 
    void revive();

    void set_new_position(Coord new_position) { this->new_position = new_position; x = this->new_position.first; y = this->new_position.second;}

    /**
     * @brief move tank to new_coordinates and print it out
     */
    void move();

    /**
     * @brief request a command through a SIGUSR2 signal to tank
     */
    void request_command() { send_request(SIGUSR2); }

    void send_request(int signum)
    {
        /* fixme: rewrite using conditional_variable or make specific thread
         * handles */
        //pthread_kill(t_handle.native_handle(), SIGUSR2);
        std::unique_lock<std::mutex> lock(request_mut);
        request_status = signum;
        lock.unlock();
        request.notify_one();
    }


    /**
     * @brief deposit_command_from_client is called from a socket communication's thread
     * @param command to be deposited
     */
    void deposit_command_from_client(std::string command);

    /**
     * @brief Tank::read_command is called from a main thread to obtain a command
     * command is then copied to the this->command
     * @return last command
     */
    void read_command() { } //this does nothing anymore

    //friend std::ostream& operator<<(std::ostream&, const Tank);

    /**
     * @brief print_crashed prints collided tank's infos
     * @param t tank that has been crashed into
     */
    void print_crashed(Tank *t) const;

    std::string to_str() const;

    /**
     * @brief print_destroy print tank's info after destruction and attackers info
     */
    void print_destroyed(Tank *attacker) const;

    void print_out_of_map() const;
};

#endif // TANK_H
