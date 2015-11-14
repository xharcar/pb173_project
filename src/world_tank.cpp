#include "world_tank.h"


// TANK

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


