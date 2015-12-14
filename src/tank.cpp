#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;

Tank::Tank(uint x, uint y, Color color)
    : x(x), y(y), color(color), dead(false)
{
    std::cout << "Spawning " << *this << std::endl;
}

Tank::Tank(Coord position, Color color)
    : Tank(position.first, position.second, color) {}

std::ostream& operator<<(std::ostream& os, const Tank& t)
{
    std::string color = t.get_color() == Color::RED ? "Red" : "Green";
    os << "Tank [" << t.get_x() << ", " << t.get_y() << "] [" << color << "] ";
    return os;
}

void Tank::print_destroyed(const Tank& t)
{
    std::cout << *this << "destroyed by " << t << std::endl;
}

void Tank::print_crashed(const Tank& t)
{
    std::cout << *this << "crashed into " << t << std::endl;
}

bool Tank::move(int height, int width, Coord new_pos)
{
    bool ret = false;
    if (!check_bounds(height, width)) {
        ret = true;
        std::cout << "Tank out of map: " << get_color() << " " << std::endl;
    }
    else {
        x = new_pos.first;
        y = new_pos.second;
        std::cout << *this << "moved to [" << new_pos.first << ", "
                  << new_pos.second << "]" << std::endl;
    }
    return ret;
}

void Tank::request_command()
{
    /* fixme: rewrite using conditional_variable or make specific thread handles */
    pthread_kill(t_handle.native_handle(), SIGUSR2);
}

/*
void Tank::hit_tank(TankShell attacker)
{
    if (attacker.color != this->color)
    {
        this->dead = true;
        this->attacker = attacker;
    }
}
*/

void Tank::kill_thread()
{
    pthread_kill(t_handle.native_handle(), SIGTERM);
}

void Tank::quit()
{
    t_handle.join();
}

void tank_sig_handler(int sig){
    switch (sig) {
    case SIGUSR2:
        tank_send = 1;
        break;
    case SIGTERM:
        tank_exit = 1;
        break;
    }
}

// fixme: add argument for socket passing
void Tank::spawn_thread()
{
    /* Spawn a thread to communicate with tankclient asynchronously */
    this->t_handle = std::thread([&](){
    });
}

void Tank::deposit_command_from_client(std::string command)
{
    std::unique_lock<std::mutex> lock(com_mut);
    command_buffer.push(command);
    lock.unlock();
    com.notify_one();
}

//std::string Tank::read_command()
void Tank::read_command()
{
    std::unique_lock<std::mutex> lock(com_mut);
    com.wait(lock, [this] { return !command_buffer.empty(); });
    std::string command;
    //std::swap(command, command_buffer.front());
    std::swap(command, command_buffer.front());
    command_buffer.pop();
    //return command;
}

bool Tank::take_action()
{
    bool destroy = false;
    if (action) {
        if ( action() )
            destroy = true;
    }
    action = nullptr;
    return destroy;
}

void Tank::subscribe_action(std::function<bool ()> action)
{
    /*
    if (!this->action) {
        this->action = action;
    }
    */
    this->action = action;
}
