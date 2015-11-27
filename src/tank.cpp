#include "tank.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;


pthread_t Tank::getTID()
{
    return this->tid;
}

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
    pthread_kill(this->getTID(), SIGUSR2);
}

void Tank::read_command()
{
    char buf[4] = "\0";
    read(this->getPipe(), buf, 3);
    this->action = std::string(buf);
}

void Tank::hit_tank(Color c)
{
    if (c != this->color)
    {
        this->hit = true;
    }
}

void Tank::moveleft()
{
    this->x--;
}

void Tank::moveright()
{
    this->x++;
}

void Tank::moveup()
{
    this->y++;
}

void Tank::movedown()
{
    this->y--;
}

void Tank::kill_thread()
{
    pthread_kill(getTID(), SIGTERM);
}

void Tank::quit()
{
    pthread_join(getTID(), NULL);
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
int run_tank(int* tankpipe){
    // fixme: move srand initialization to class/global scope
    std::srand(std::time(0));
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
}

void Tank::spawn_thread()
{
    /*
    tankpath.clear();
    pipe(t.getpfd());
    pthread_t x = t.getTID();
    t.setTID(x);
    */
    // TankOptions opts;
    // pthread_create(&tid, NULL, run_tank, (void*) opts);
}

