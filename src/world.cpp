#include "world.h"


// tank vars
int tank_exit = 0;
int tank_send = 0;

// METHOD IMPLEMENTATIONS

// UTILS

Utils::Utils(int argc, char* argv[])
    : mDaemonize(false)
    , mGreenPath("")
    , mRedPath("")
    , mExit(false)
    , red_kills(0)
    , green_kills(0)
    , rounds_played(0)
{
    struct option longopts[] =
    {
        { "daemonize",   no_argument,       NULL, 'd' },
        { "green-tanks", required_argument, NULL, 'c' },
        { "red-tanks",   required_argument, NULL, 'v' },
        { "green-tank",  required_argument, NULL, 'g' },
        { "red-tank",    required_argument, NULL, 'r' },
        { "round-time",  required_argument, NULL, 't' },
        { "area-size",   required_argument, NULL, 'a' },
        { "pipe",        required_argument, NULL, 'p' },
        { "help",        no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "d:c:v:g:r:t:a:p:h", longopts, NULL)) != -1)
    {
        switch (c)
        {
        case 'a':
            this->mMapWidth = atoi(argv[optind-1]);
            this->mMapHeight = atoi(argv[optind]);
            break;
        case 'c':
            this->mGreenTanks = atoi(optarg);
            break;
        case 'v':
            this->mRedTanks = atoi(optarg);
            break;
        case 'd' :
            this->mDaemonize = true;
            break;
        case 'r':
            this->mRedPath.append(optarg);
            break;
        case 'g' :
            this->mGreenPath.append(optarg);
            break;
        case 't':
            this->mRoundTime = atoi(optarg);
            break;
        case 'p':
            this->fifoPath.append(optarg);
            break;
        case 'h':
            this->printHelp();
            exit(0);
        default:
            this->printError();
            exit(-1);
        }
    }
    if(mMapHeight<=0 || mMapWidth<=0 || mRoundTime <=0)
    {
        this->printHelp();
        exit(-1);
    }
}

void Utils::printHelp()
{
    std::cout <<"=====================================================" << std::endl
              <<"|         PB173 Internet Of Tanks presents:  WORLD  |" << std::endl
              <<"-----------------------------------------------------" << std::endl
              <<"                    USAGE                            " << std::endl
              <<"  -h | --help           Show this help               " << std::endl
              <<"  --daemonize           run as daemon                " << std::endl
              <<"  --green-tanks [n]     create n green tanks         " << std::endl
              <<"  --red-tanks [n]       create n red tanks           " << std::endl
              <<"  --green-tank [s]      path to green tank bin       " << std::endl
              <<"  --red-tank [s]        path to red tank bin         " << std::endl
              <<"  --area-size [n] [m]   size of area NxM             " << std::endl
              <<"  --round-time [n]      time of one round in ms      " << std::endl
              <<"  -p | --pipe [file]    path to a named FIFO to output map to in daemon mode" << std::endl
              <<"=====================================================" << std::endl;
}

void Utils::printError()
{
    std::cerr << "Wrong arguments or something" << std::endl;
}

// END OF UTILS

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
    std::vector<std::string > commands {"fu","fd","fr","fl","mu","md","mr","ml"};
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

// WORLD
void World::add_tank(Tank t, Utils u)
{
    if(t.getColor()==RED)
    {
        std::cout << "Adding red tank" << std::endl;
        red_tanks.push_back(t);
        spawn_thread(t,u.getRedPath());
    }
    else
    {
        std::cout << "Adding green tank" << std::endl;
        green_tanks.push_back(t);
        spawn_thread(t,u.getGreenPath());
    }
}

bool World::is_free(int x, int y)
{
    if(this->zone[x][y] == EMPTY)
    {
        return true;
    }
    return false;
}

Coord World::free_coord()
{
    Coord rv;
    std::srand(std::time(0));
    int x = std::rand() % this->width;
    int y = std::rand() % this->height;
    while(this->is_free(x,y))
    {
        // only loops if first try failed,ends as soon as
        // a free field is found
        x = std::rand() % this->width;
        y = std::rand() % this->height;
    }
    rv.first = x;
    rv.second = y;
    return rv;
}

void World::req_com()
{
    std::cout << "Requesting tank orders" << std::endl;
    for(int i=0;i<red_tanks.size();++i)
    {
        pthread_kill(red_tanks[i].getTID(),SIGUSR2);
    }
    for(int i=0;i<green_tanks.size();++i)
    {
        pthread_kill(green_tanks[i].getTID(),SIGUSR2);
    }
}

void World::read_com(std::vector<std::string> red,
                     std::vector<std::string> green)
{
    std::cout << "Reading tank commands" << std::endl;
    for(int i=0;i<red_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(red_tanks[i].getPipe(),buf,3);
        red.push_back(std::string(buf));
    }
    for(int i=0;i<green_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(green_tanks[i].getPipe(),buf,3);
        green.push_back(std::string(buf));
    }
}

void World::fire(std::vector<Tank> tanks,
                 std::vector<std::string> actions)
{
    std::cout << "FIRE EVERYTHING!" << std::endl;
    for(unsigned i=0; i<actions.size(); ++i)
    {
        if(actions[i][0] == 'f')
        {
            switch(actions[i][1])
            {
            case 'u':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getY() < tanks[i].getY()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getY() < tanks[i].getY()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'd':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getY() > tanks[i].getY()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getY() > tanks[i].getY()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'l':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getX() < tanks[i].getX()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getX() < tanks[i].getX()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'r':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getX() > tanks[i].getX()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getX() > tanks[i].getX()) red_tanks[j].setHit(true);
                }
            }
            break;
            }
        }
    }
}

void World::movetanks(std::vector<Tank> tanks,
                      std::vector<std::string> actions)
{
    std::cout << "Moving tanks" << std::endl;
    for(unsigned i=0; i<actions.size(); ++i)
    {
        if(actions[i][0]=='m' && !tanks[i].getHit())
        {
            switch (actions[i][1])
            {
            case 'u' :
                tanks[i].setY(tanks[i].getY()-1);
                break;
            case 'd' :
                tanks[i].setY(tanks[i].getY()+1);
                break;
            case 'l' :
                tanks[i].setX(tanks[i].getX()-1);
                break;
            case 'r' :
                tanks[i].setX(tanks[i].getX()+1);
                break;
            }
        }
    }
}

void World::crash_tanks(std::vector<Tank> tanks1,
                        std::vector<Tank> tanks2)
{
    std::cout << "Checking for tank crashes" << std::endl;
    for(int i=0;i<tanks1.size();++i){
        for(int j=0;j<tanks2.size();++j){
            if(tanks1[i].getTID() != tanks2[j].getTID() &&
               // prevents a tank crashing into itself
               tanks1[i].getX() == tanks2[j].getX() &&
               tanks1[i].getY() == tanks2[j].getY() &&
               !tanks1[i].getHit() && !tanks2[j].getHit()){
                    tanks1[i].setHit(true);
                    tanks2[j].setHit(true);
            }
        }
    }
}

void World::add_kills(Utils u)
{
    std::cout << "Adding kills, old counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
    for(int i=0;i<red_tanks.size();++i){
        if(red_tanks[i].getHit()) u.incGreenKills();
    }
    for(int i=0;i<green_tanks.size();++i){
        if(green_tanks[i].getHit()) u.incRedKills();
    }
    std::cout << "New kill counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
}

void World::remove_hit_tanks()
{
    std::cout << "Removing hit tanks" << std::endl;
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        if(t->getHit()){
            std::cout << "Red tank with TID " << t->getTID() << " at "
            << t->getX() << "," << t->getY() << " has been hit, removing" << std::endl;
            pthread_kill(t->getTID(),SIGTERM);
            red_tanks.erase(t);
        }
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        if(t->getHit()){
            std::cout << "Green tank with TID " << t->getTID() << " at "
            << t->getX() << "," << t->getY() << " has been hit, removing" << std::endl;
            pthread_kill(t->getTID(),SIGTERM);
            green_tanks.erase(t);
        }
    }
}

void World::respawn_tanks(Utils u)
{
    std::cout << "Respawning tanks" << std::endl;
    while(red_tanks.size() < u.getRedTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.getGreenTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        add_tank(t,u);
    }
}

void World::refresh_zone()
{
    std::cout << "Refreshing map" << std::endl;
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;i++){
            zone[i][j] = EMPTY;
        }
    }
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        zone[t->getX()][t->getY()] = RED;
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        zone[t->getX()][t->getY()] = GREEN;
    }
}

void World::play_round(Utils u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    req_com();
    read_com(red_actions,green_actions);
    fire(red_tanks,red_actions);
    fire(green_tanks,green_actions);
    movetanks(red_tanks,red_actions);
    movetanks(green_tanks,green_actions);
    crash_tanks(red_tanks,red_tanks);
    crash_tanks(red_tanks,green_tanks);
    crash_tanks(green_tanks,green_tanks);
    add_kills(u);
    remove_hit_tanks();
    respawn_tanks(u);
    refresh_zone();
    std::cout << "Round " << u.getRoundsPlayed() << std::endl;
    output_map();
    usleep((useconds_t)u.getRoundTime()*1000);
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}

void World::output_map()
{
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;j++){
            std::cout << "|" << zone[i][j];
        }
        std::cout << "|" << std::endl;
    }
}

void World::quit_safe(int sig)
{
    sig = sig;
    std::cout << "Quitting safely" << std::endl;
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    red_tanks.clear();
    green_tanks.clear();
    zone.clear();
}
// END OF WORLD

// DAEMONWORLD OVERRIDES
void DaemonWorld::add_tank(Tank t, Utils u)
{
    if(t.getColor()==RED)
    {
        syslog(LOG_INFO,"Adding red tank\n");
        red_tanks.push_back(t);
        spawn_thread(t,u.getRedPath());
    }
    else
    {
        syslog(LOG_INFO,"Adding green tank\n");
        green_tanks.push_back(t);
        spawn_thread(t,u.getGreenPath());
    }
}

bool DaemonWorld::is_free(int x, int y)
{
    if(this->zone[x][y] == EMPTY)
    {
        return true;
    }
    return false;
}

Coord DaemonWorld::free_coord()
{
    Coord rv;
    std::srand(std::time(0));
    int x = std::rand() % this->width;
    int y = std::rand() % this->height;
    while(this->is_free(x,y))
    {
        // only loops if first try failed,ends as soon as
        // a free field is found
        x = std::rand() % this->width;
        y = std::rand() % this->height;
    }
    rv.first = x;
    rv.second = y;
    return rv;
}

void DaemonWorld::req_com()
{
    syslog(LOG_INFO,"Requesting tank orders\n");
    for(int i=0;i<red_tanks.size();++i)
    {
        pthread_kill(red_tanks[i].getTID(),SIGUSR2);
    }
    for(int i=0;i<green_tanks.size();++i)
    {
        pthread_kill(green_tanks[i].getTID(),SIGUSR2);
    }
}

void DaemonWorld::read_com(std::vector<std::string> red,
                     std::vector<std::string> green)
{
    syslog(LOG_INFO,"Reading tank commands\n");
    for(int i=0;i<red_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(red_tanks[i].getPipe(),buf,3);
        red.push_back(std::string(buf));
    }
    for(int i=0;i<green_tanks.size();++i)
    {
        char buf[4] = "\0";
        read(green_tanks[i].getPipe(),buf,3);
        green.push_back(std::string(buf));
    }
}

void DaemonWorld::fire(std::vector<Tank> tanks,
                 std::vector<std::string> actions)
{
   syslog(LOG_INFO, "FIRE EVERYTHING!\n");
    for(unsigned i=0; i<actions.size(); ++i)
    {
        if(actions[i][0] == 'f')
        {
            switch(actions[i][1])
            {
            case 'u':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getY() < tanks[i].getY()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getY() < tanks[i].getY()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'd':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getY() > tanks[i].getY()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getY() > tanks[i].getY()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'l':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getX() < tanks[i].getX()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getX() < tanks[i].getX()) red_tanks[j].setHit(true);
                }
            }
            break;
            case 'r':
            {
                for(int j=0;j<green_tanks.size();++j)
                {
                    if(green_tanks[j].getX() > tanks[i].getX()) green_tanks[j].setHit(true);
                }
                for(int j=0;j<red_tanks.size();++j)
                {
                    if(red_tanks[j].getX() > tanks[i].getX()) red_tanks[j].setHit(true);
                }
            }
            break;
            }
        }
    }
}

void DaemonWorld::movetanks(std::vector<Tank> tanks,
                      std::vector<std::string> actions)
{
    syslog(LOG_INFO,"Moving tanks");
    for(unsigned i=0; i<actions.size(); ++i)
    {
        if(actions[i][0]=='m' && !tanks[i].getHit())
        {
            switch (actions[i][1])
            {
            case 'u' :
                tanks[i].setY(tanks[i].getY()-1);
                break;
            case 'd' :
                tanks[i].setY(tanks[i].getY()+1);
                break;
            case 'l' :
                tanks[i].setX(tanks[i].getX()-1);
                break;
            case 'r' :
                tanks[i].setX(tanks[i].getX()+1);
                break;
            }
        }
    }
}

void DaemonWorld::crash_tanks(std::vector<Tank> tanks1,
                        std::vector<Tank> tanks2)
{
    syslog(LOG_INFO,"Checking for tank crashes");
    for(int i=0;i<tanks1.size();++i){
        for(int j=0;j<tanks2.size();++j){
            if(tanks1[i].getTID() != tanks2[j].getTID() &&
               // prevents a tank crashing into itself
               tanks1[i].getX() == tanks2[j].getX() &&
               tanks1[i].getY() == tanks2[j].getY() &&
               !tanks1[i].getHit() && !tanks2[j].getHit()){
                    tanks1[i].setHit(true);
                    tanks2[j].setHit(true);
            }
        }
    }
}

void DaemonWorld::add_kills(Utils u)
{
    syslog(LOG_INFO,"Adding kills, old counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
    for(int i=0;i<red_tanks.size();++i){
        if(red_tanks[i].getHit()) u.incGreenKills();
    }
    for(int i=0;i<green_tanks.size();++i){
        if(green_tanks[i].getHit()) u.incRedKills();
    }
    syslog(LOG_INFO,"New kill counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
}

void DaemonWorld::remove_hit_tanks()
{
    syslog(LOG_INFO,"Removing hit tanks\n");
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        if(t->getHit()){
            pthread_kill(t->getTID(),SIGTERM);
            red_tanks.erase(t);
        }
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        if(t->getHit()){
            pthread_kill(t->getTID(),SIGTERM);
            green_tanks.erase(t);
        }
    }
}

void DaemonWorld::respawn_tanks(Utils u)
{
    syslog(LOG_INFO,"Respawning tanks\n");
    while(red_tanks.size() < u.getRedTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.getGreenTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        add_tank(t,u);
    }
}

void DaemonWorld::refresh_zone()
{
    syslog(LOG_INFO,"Refreshing map\n");
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;i++){
            zone[i][j] = EMPTY;
        }
    }
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        zone[t->getX()][t->getY()] = RED;
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        zone[t->getX()][t->getY()] = GREEN;
    }
}

void DaemonWorld::play_round(Utils u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    syslog(LOG_INFO,"Round %d\n",u.getRoundsPlayed());
    req_com();
    read_com(red_actions,green_actions);
    fire(red_tanks,red_actions);
    fire(green_tanks,green_actions);
    movetanks(red_tanks,red_actions);
    movetanks(green_tanks,green_actions);
    crash_tanks(red_tanks,red_tanks);
    crash_tanks(red_tanks,green_tanks);
    crash_tanks(green_tanks,green_tanks);
    add_kills(u);
    remove_hit_tanks();
    respawn_tanks(u);
    refresh_zone();
    output_map();
    usleep((useconds_t)u.getRoundTime()*1000);
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}


void DaemonWorld::quit_safe(int sig)
{
    sig = sig;
    syslog(LOG_INFO,"Quitting safely\n");
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    red_tanks.clear();
    green_tanks.clear();
    zone.clear();
    close(pipefd);
}

void DaemonWorld::output_map()
{
    std::stringstream ss;
    ss << width << ',' << height;
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;j++){
            ss << ',' << zone[i][j];
        }
    }
    write(pipefd,ss.str().c_str(),(height*width*2)+4);
}

// END OF DAEMONWORLD OVERRIDES
// /===========================================================/
// MAIN

int main(int argc, char *argv[])
{
    // TODO : sigactions
    Utils mUtils(argc, argv);
    World *w;
    /*
        int pid_file = open("/var/run/world.pid", O_CREAT | O_RDWR, 0666);
        if(flock(pid_file, LOCK_EX | LOCK_NB)) {
            // Another instance is running, end
            if(errno == EWOULDBLOCK){
                std::cerr << "World already running" << std::endl;
                delete(mUtils);
                return 1;
            }
        }
    */
    if(mUtils.getDaemonize())
    {
        static_cast<DaemonWorld*> (w);
        DaemonWorld* w = new DaemonWorld(mUtils.getMapHeight(),mUtils.getMapWidth(),mUtils.getFifoPath());
    }else
    {
        w = new World(mUtils.getMapHeight(),mUtils.getMapWidth());
    }
    for (uint i = 0; i < mUtils.getGreenTanks(); i++)
    {
        Coord c = w->free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        w->add_tank(t,mUtils);
    }
    for (uint i = 0; i < mUtils.getRedTanks(); i++)
    {
        Coord c = w->free_coord();
        Tank t = Tank(c.first, c.second, RED);
        w->add_tank(t,mUtils);
    }

    while(true)
    {
        w->play_round(mUtils);
    }
    return 0;
}
