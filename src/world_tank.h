#include <pthread.h>
#include <sys/types.h>

/**
 * @brief Represents a tank in-game
 */
class Tank
{
private:
    pthread_t tid;
    int pfd [2];
    bool hit;
    uint x;
    uint y;
    Color color;
public:

    /**
     * @brief Tank constructor, sets TID to 0(to indicate not yet initialized properly)
     *  and hit flag to false(when a tank rolls up onto a battlefield, it usually is in fighting condition)
     * @param x x coordinate of tank
     * @param y y coordinate of tank
     */
    Tank(uint x, uint y, Color color) : tid(0), hit(false), x(x), y(y), color(color){}

    /**
     * @brief TID getter (for sending signals,...)
     * @return TID of tank thread
     */
    pthread_t getTID();

    /**
     * @brief TID setter
     * @param x TID to be set
     */
    void setTID(pthread_t x);

    /**
     * @brief hit flag getter
     * @return true if tank has been hit, else false
     */
    bool getHit();

    /**
     * @brief hit flag setter (used only when tank has been hit)
     * @param shot indicates whether tank has been hit(~true)
     */
    void setHit(bool shot);

    /**
     * @brief X coordinate getter
     * @return tank x coordinate
     */
    uint getX();

    /**
     * @brief Y coordinate getter
     * @return tank y coordinate
     */
    uint getY();

    /**
     * @brief X coordinate setter
     */
    void setX(int newx);

    /**
     * @brief Y coordinate setter
     */
    void setY(int newy);

    /**
     * @brief color getter
     */
    Color getColor();

    /**
     * @brief pipe read end getter for commands
     */
    int getPipe(){
        return this->pfd[0];
    }

    int* getpfd(){
        return this->pfd;
    }

    /**
     * @brief spawns a new tank thread, initialized TID of tank
     * @param tankpath path to tank binary to be executed
     */
    void spawn_thread(std::string tankpath);

    /**
     * @brief handles newly created tank thread
     * @param pipeptr pointer pipe from which world reads tank commands
     */
    void* handle_thread(void* pipeptr);

    /**
     * @brief communicates with spawned tank
     */
    void read_action();
};

void tank_sig_handler(int sig){
    switch(sig){
        case SIGUSR2 : tank_send=1;
        break;
        case SIGTERM : tank_exit=1;
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

// END OF TANK
