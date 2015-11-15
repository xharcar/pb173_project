#include "tankclient.h"

// tank vars
int tank_exit = 0;
int tank_send = 0;


pthread_t TankClient::getTID()
{
    return this->tid;
}

bool TankClient::getHit()
{
    return this->hit;
}

void TankClient::setHit(bool shot)
{
    this->hit = shot;
}

uint TankClient::getX()
{
    return this->x;
}

uint TankClient::getY()
{
    return this->y;
}

void TankClient::setX(int newx)
{
    this->x = newx;
}

void TankClient::setY(int newy)
{
    this->y = newy;
}

Color TankClient::getColor()
{
    return this->color;
}

void TankClient::setTID(pthread_t x){
    this->tid = x;
}

void TankClient::request_command()
{
    pthread_kill(this->getTID(), SIGUSR2);
}

void TankClient::read_command()
{
    char buf[4] = "\0";
    read(this->getPipe(), buf, 3);
    this->action = std::string(buf);
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

void spawn_thread(TankClient t, std::string tankpath)
{
    tankpath.clear();
    pipe(t.getpfd());
    pthread_t x = t.getTID();
    pthread_create(&x,NULL,&handle_thread,(void*)t.getpfd());
    t.setTID(x);
}

