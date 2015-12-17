#include "tank.h"

void tank_sig_handlerr(int signum, siginfo_t* info, void* context)
{
    switch (signum) {
    case SIGUSR2:
        break;
    case SIGTERM:
        break;
    }
}

Tank::Tank(int x, int y, Color color)
    : x(x), y(y), color(color), state(TankState::alive)
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

void Tank::print_destroyed(const Tank& t) const
{
    std::cout << *this << "destroyed by " << t << std::endl;
}

void Tank::print_crashed(const Tank& t) const
{
    std::cout << *this << "crashed into " << t << std::endl;
}

void Tank::print_out_of_map() const {
        std::cout << *this << " rolled out of battlefield" << std::endl;
}

/*
void Tank::move(int height, int width, Coord new_pos)
{
    if (!check_bounds(height, width)) {
        std::cout << "Tank out of map: " << get_color() << " " << std::endl;
    }
    else {
        x = new_pos.first;
        y = new_pos.second;
        std::cout << *this << "moved to [" << new_pos.first << ", "
                  << new_pos.second << "]" << std::endl;
    }
}
*/

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
