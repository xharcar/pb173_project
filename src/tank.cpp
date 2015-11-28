#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;


bool Tank::getHit()
{
    return this->hit;
}

void Tank::setHit(bool shot)
{
    this->hit = shot;
}

uint Tank::getX()
{
    return this->x;
}

uint Tank::getY()
{
    return this->y;
}

Color Tank::getColor()
{
    return this->color;
}

void Tank::request_command()
{
    pthread_kill(tid.native_handle(), SIGUSR2);
}

void Tank::read_command()
{
    char buf[4] = "\0";
    read(this->getPipe(), buf, 3);
    this->action = std::string(buf);
}

void Tank::hit_tank(Tank& attacker)
{
    if (attacker.getColor() != this->color)
    {
        this->hit = true;
        this->attacker = attacker;
    }
}

void Tank::kill_thread()
{
    pthread_kill(tid.native_handle(), SIGTERM);
}

void Tank::quit()
{
    tid.join();
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
    struct sigaction action;
    action.sa_flags=0;
    action.sa_handler = tank_sig_handler;
    sigaction(SIGTERM,&action,NULL);
    // not gonna do AI in 20min
    std::vector<std::string> commands {"fu","fd","fr","fl","mu","md","mr","ml"};
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
    this->tid = std::thread(run_tank, NULL);
}

void Tank::print_destroy(Tank& attacker) {
    std::cout << "Tank destroyed: ";
    if (color == Color::RED) {
        std::cout << "Red";
    } else {
        std::cout << "Green";
    }
    // fixme: possibly return the PID of a running Tankclient
    std::cout << ", " << tid.get_id() << ", [" << getX() << ", " << getY()
              << "]" << std::endl;
    // fixme:: possibly overload stream operator for Tank to print out tank info
    std::cout << "Attacker: " << tid.get_id() << ", [" << getX() << ", "
              << getY() << "]" << std::endl;
}


