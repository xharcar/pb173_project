#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;

Tank::Tank(uint x, uint y, Color color, std::string bin_path)
    : x(x), y(y), color(color), tankclient_path(bin_path), hit(false) {}

void Tank::request_command()
{
    //fixme rewrite to cond_var or make specific thread handles
    pthread_kill(t_handle.native_handle(), SIGUSR2);
}

void Tank::hit_tank(TankShell attacker)
{
    if (attacker.color != this->color)
    {
        this->hit = true;
        this->attacker = attacker;
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

/**
 * @brief runs a tank
 * @param tankpipe pipe to send orders to world through
 */
//int run_tank(int socket){
    // fixme: move srand initialization to class/global scope
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
//}

// fixme: add argument for socket passing
void Tank::spawn_thread()
{
    spawn_process(tankclient_path);

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
    std::swap(action, command_buffer.front());
    command_buffer.pop();
    //return command;
}

