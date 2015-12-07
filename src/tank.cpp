#include "tank.h"
#include <sys/socket.h>
#include <netdb.h>

// tank vars
int tank_exit = 0;


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

void Tank::hit_tank()
{
    this->hit = true;
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
        case SIGTERM:
            tank_exit = 1;
            break;
    }
}

/**
 * @brief runs a tank
 */
int run_tank(){
    int echk;
    int port = 7020;
    char* portstr = (char*)calloc(6,sizeof(char));
    itoa(port,portstr,10);
    struct addrinfo** res;
    struct addrinfo* hints = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
    hints->ai_family = AF_INET6;
    hints->ai_protocol = 17;
    do{
        echk = getaddrinfo(NULL,portstr,&hints,&res;)
        if(echk!=0){
            freeaddrinfo(res);
            port++;
            free(portstr);
            portstr = (char*)calloc(6,sizeof(char));
            itoa(port,portstr,10);
        }
        if(port > 65536){
            std::cerr << "Could not find suitable network address for tank with TID" << pthread_self() << std::endl;
            pthread_exit(&port);
        }
    }while(echk!=0);
    free(hints);
    free(portstr);
    int sfd;
    struct sigaction action;
    action.sa_flags=0;
    action.sa_handler = tank_sig_handler;
    sigaction(SIGTERM,&action,NULL);
    sfd = socket(AF_INET6,SOCK_DGRAM,17);
    if(sfd == -1){
        std::cerr << "Could not make socket for tank with TID " << pthread_self() << std::endl;
        freeaddrinfo(res);
        pthread_exit(*sfd);
    }
    // Any connection using datagrams and UDP(Aisa's /etc/protocols specifies UDP as 17)
    if(echk = bind(sfd,res[0]->ai_addr,res[0]->ai_addrlen) != 0){
        std::cerr << "Could not bind port to tank with TID " << pthread_self() << std::endl;
        freeaddrinfo(res);
        pthread_exit(*echk);
    }
    // Exits if bind fails
    while(tank_exit==0){
        pthread_cond_wait(&worldcvariable,&worldmtxlock);
        pthread_mutex_lock(&worldmtxlock);
        portstr = calloc(4,sizeof(char));
        recvfrom(sfd,portstr,3,0,res[0]->ai_addr,&res[0]->ai_addrlen);
        std::stringstream ss;
        ss << pthread_self << " " << std::string(portstr);
        tank_messages.push_back(ss.str());
        pthread_cond_signal(&worldcvariable);
        pthread_mutex_unlock(&worldmtxlock);
    }
    freeaddrinfo(res);
    return 0;
}

void Tank::spawn_thread()
{
    pthread_create(&tid, NULL, run_tank, (void*) NULL);
}

