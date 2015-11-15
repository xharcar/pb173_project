#include "world_tank.h"

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

void Tank::setX(int newx)
{
    this->x = newx;
}

void Tank::setY(int newy)
{
    this->y = newy;
}

Color Tank::getColor()
{
    return this->color;
}

void Tank::setTID(pthread_t x){
    this->tid = x;
}

void Tank::read_com()
{
    std::cout << "Reading tank commands" << std::endl;
    for(std::size_t i=0;i<red_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(red_tanks[i].getPipe(),buf,3);
        red.push_back(std::string(buf));
    }
    for(std::size_t i=0;i<green_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(green_tanks[i].getPipe(),buf,3);
        green.push_back(std::string(buf));
    }
}

void Tank::request_command()
{
    pthread_kill(this->getTID(), SIGUSR2);
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

void* handle_thread(void* tankpipe){
    run_tank((int*)tankpipe);
    return NULL;
}

void spawn_thread(Tank t, std::string tankpath)
{
    tankpath.clear();
    pipe(t.getpfd());
    pthread_t x = t.getTID();
    pthread_create(&x,NULL,&handle_thread,(void*)t.getpfd());
    t.setTID(x);
}

