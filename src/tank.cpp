#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;

void Tank::request_command()
{
    pthread_kill(t_handle.native_handle(), SIGUSR2);
}

void Tank::read_command()
{
    char buf[4] = "\0";
    read(this->getPipe(), buf, 3);
    this->action = std::string(buf);
}

void Tank::hit_tank(Tank& attacker)
{
    if (attacker.get_color() != this->color)
    {
        this->hit = true;
        this->attacker.color = attacker.get_color();
        this->attacker.x = attacker.get_x();
        this->attacker.y = attacker.get_y();
    }
}

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

/**
 * @brief runs a tank
 * @param tankpipe pipe to send orders to world through
 */
int run_tank(int socket){
    // fixme: move srand initialization to class/global scope
    std::srand(std::time(0));
    /*
    int x = 0;
    while(tank_exit==0){
        x = std::rand() % 8;
        if(tank_send){
            write(tankpipe[1],commands[x].c_str(),3);
        }
    }
    return 0;
    */
}

void Tank::spawn_thread()
{
    this->t_handle = std::thread(run_tank, NULL);
}

void Tank::print_destroy() {
    std::cout << "Tank destroyed: ";
    if (color == Color::RED) {
        std::cout << "Red";
    } else {
        std::cout << "Green";
    }
    // fixme: possibly return the PID of a running Tankclient
    std::cout << ", " << t_handle.get_id() << ", [" << get_x() << ", " << get_y()
              << "]" << std::endl;
    // fixme:: possibly overload stream operator for Tank to print out tank info
    std::cout << "Attacker: " << attacker.color << ", [" << attacker.x << ", "
              << attacker.y << "]" << std::endl;
}
