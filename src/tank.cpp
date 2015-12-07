#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;

void Tank::request_command()
{
    //fixme rewrite to cond_var or make specific thread handles
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
    /* Spawn the process of a tankclient which generates the commands for the tank */
    switch (fork()) {
    /* child process */
    case 0:
        execl(command.c_str(), command.c_str(), (char*)NULL);
        assert(false);
    case -1:
        /* fork failiure */
        assert(false);
    }

    /* Spawn a thread to communicate with tankclient asynchronously */
    this->t_handle = std::thread([&](){
        std::unique_lock<std::mutex> lock(com_mut);
        communicate.wait(com_mut);
    });
}

void Tank::deposit_command_from_client(std::string command)
{
    std::unique_lock<std::mutex> lock(com_mut);
    command_buffer.push(command);
    lock.unlock();
    com.notify_one();
}

std::string Tank::read_command()
{
    std::unique_lock<std::mutex> lock(com_mut);
    com.wait(lock, [this] { return !command_buffer.empty(); });
    std::string command;
    std::swap(command, command_buffer.front());
    command_buffer.pop();
    return command;
}
