#include "tank.h"

// tank vars
int tank_exit = 0;


pthread_t Tank::getTID()
{
    return this->tid;
}

bool Tank::isHit()
{
    return this->hit;
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
void* run_tank(void* unused){
    unused = unused;
    int echk;
    int port = 7020 + (int) pthread_self();
    char *buf = (char*) calloc(6, sizeof(char));
    struct addrinfo** res = NULL;
    struct addrinfo* hints = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
    hints->ai_family = AF_INET6;
    hints->ai_protocol = 17;
    echk = getaddrinfo(NULL,buf,hints,res);
    if(echk!=0){
        freeaddrinfo(*res);
        free(buf);
	std::cout << "Could not find suitable network address for tank "
	 << pthread_self() << std::endl;
	pthread_exit(&port);
    }
    free(hints);
    int sfd;
    struct sigaction action;
    action.sa_flags=0;
    action.sa_handler = tank_sig_handler;
    sigaction(SIGTERM,&action,NULL);
    sfd = socket(AF_INET6,SOCK_DGRAM,17);
    if(sfd == -1){
        std::cerr << "Could not make socket for tank with TID " << pthread_self() << std::endl;
        freeaddrinfo(*res);
        pthread_exit(&sfd);
    }
    // Any connection using datagrams and UDP(Aisa's /etc/protocols specifies UDP as 17)
    if((echk = bind(sfd,res[0]->ai_addr,res[0]->ai_addrlen)) != 0){
        std::cerr << "Could not bind port to tank with TID " << pthread_self() << std::endl;
        freeaddrinfo(*res);
        pthread_exit(&echk);
    }
    // Exits if bind fails
    while(tank_exit==0){
        pthread_cond_wait(&worldcvariable,&worldmtxlock);
        pthread_mutex_lock(&worldmtxlock);
        buf = (char*) calloc(4,sizeof(char));
        recvfrom(sfd,buf,3,0,res[0]->ai_addr,&res[0]->ai_addrlen);
        std::stringstream ss;
        ss << pthread_self() << " " << std::string(buf);
        tank_messages.push_back(ss.str());
        pthread_cond_signal(&worldcvariable);
        pthread_mutex_unlock(&worldmtxlock);
	free(buf);
    }
    freeaddrinfo(*res);
    return NULL;
}

void Tank::spawn_thread()
{
    pthread_create(&tid, NULL, run_tank, NULL);
}
