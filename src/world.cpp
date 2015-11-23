#include "world.h"
#include "daemonworld.h"


// extra var to pass ARGV through for restart
char** argv_extra;
// world to be constructed after program start;
// can be used in main_sig_handler for correct exit
World *w;


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

Utils::~Utils(){}
// END OF UTILS

// WORLD
void World::add_tank(TankClient t, Utils u)
{
    if(t.getColor() == Color::RED)
    {
        std::cout << "Adding red tank" << std::endl;
        red_tanks.push_back(t);
        spawn_thread(t, u.getRedPath());
    }
    else
    {
        std::cout << "Adding green tank" << std::endl;
        green_tanks.push_back(t);
        spawn_thread(t, u.getGreenPath());
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
    while (this->is_free(x, y))
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


void World::fire()
{
    for(TankClient& t : boost::join(green_tanks, red_tanks))
    {
        if (t.get_action().size() != 0 && t.get_action()[0] == 'f')
        {
            fire_direction(t);
        }
    }
}

void World::fire_direction(TankClient& t) {
    auto& foe_tanks = t.getColor() == Color::GREEN ? red_tanks : green_tanks;
    for (TankClient& target : foe_tanks)
    {
        switch(t.get_action()[1])
        {
        case 'u':
            if (target.getY() < t.getY() && target.getX() == t.getX())
            {
                target.hit_tank(t.getColor());
            }
            break;
        case 'd':
            if (target.getY() > t.getY() && target.getX() == t.getX())
            {
                target.hit_tank(t.getColor());
            }
            break;
        case 'l':
            if (target.getY() == t.getY() && target.getX() < t.getX())
            {
                target.hit_tank(t.getColor());
            }
            break;
        case 'r':
            if (target.getY() == t.getY() && target.getX() < t.getX())
            {
                target.hit_tank(t.getColor());
            }
            break;
        default:
            assert(false);
        }
    }
}

void World::movetanks()
{
    for(TankClient& t : boost::join(green_tanks, red_tanks))
    {
        if (t.get_action().size() != 0
            && t.get_action()[0] == 'm'
            && !t.getHit())
        {
            switch (t.get_action()[1])
            {
            case 'u' :
                t.moveup();
                break;
            case 'd' :
                t.movedown();
                break;
            case 'l' :
                t.moveleft();
                break;
            case 'r' :
                t.moveright();
                break;
            default:
                assert(false);
            }
        }
    }
}

void World::crash_tanks(std::vector<TankClient> tanks1,
                        std::vector<TankClient> tanks2)
{
    for(std::size_t i=0;i<tanks1.size();++i){
        for(std::size_t j=0;j<tanks2.size();++j){
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
    for(std::size_t i=0;i<red_tanks.size();++i){
        if(red_tanks[i].getHit()) u.incGreenKills();
    }
    for(std::size_t i=0;i<green_tanks.size();++i){
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
            t->kill_thread();
            red_tanks.erase(t);
        }
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        if(t->getHit()){
            std::cout << "Green tank with TID " << t->getTID() << " at "
            << t->getX() << "," << t->getY() << " has been hit, removing" << std::endl;
            t->kill_thread();
            green_tanks.erase(t);
        }
    }
}

void World::respawn_tanks(Utils u)
{
    std::cout << "Respawning tanks" << std::endl;
    while(red_tanks.size() < u.getRedTanks()){
        Coord c = World::free_coord();
        TankClient t = TankClient(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.getGreenTanks()){
        Coord c = World::free_coord();
        TankClient t = TankClient(c.first, c.second, GREEN);
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

void World::process_commands(Utils u,std::vector<std::string> ra, std::vector<std::string> ga){
    for(auto m=tank_messages.begin();m!=tank_messages.end();++m){
        char* pch = strtok((char*)m->c_str()," ");
        pthread_t a = (pthread_t)atoi(pch);
        pch = strtok(NULL," ");
        for(int i=0;i<u.getRedTanks();i++){
            if(red_tanks[i].getTID()==a){
                ra[i].assign(pch);
                break;
            }
        }
        for(int i=0;i<u.getGreenTanks();i++){
            if(green_tanks[i].getTID()==a){
                ga[i].assign(pch);
                break;
            }
        }
    }

}

void World::play_round(Utils u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    red_actions.resize(u.getRedTanks());
    green_actions.resize(u.getGreenTanks());
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    pthread_cond_signal(&cvar);
    usleep((useconds_t)u.getRoundTime()*1000);
    process_commands(u,red_actions,green_actions);
    std::cout << "FIRE EVERYTHING!" << std::endl;
    fire();
    std::cout << "Moving tanks" << std::endl;
    movetanks();
    std::cout << "Checking for tank crashes" << std::endl;
    crash_tanks(red_tanks,red_tanks);
    crash_tanks(red_tanks,green_tanks);
    crash_tanks(green_tanks,green_tanks);
    add_kills(u);
    remove_hit_tanks();
    respawn_tanks(u);
    refresh_zone();
    std::cout << "Round " << u.getRoundsPlayed() << std::endl;
    output_map();
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
    std::cout << "Quitting safely" << std::endl;
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        t->kill_thread();
        t->quit();
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        t->kill_thread();
        t->quit();
    }
    red_tanks.clear();
    green_tanks.clear();
    zone.clear();
}
// END OF WORLD

// MAIN SIGNAL HANDLER
void main_sig_handler(int sig){
    switch(sig){
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:{
            w->quit_safe(sig);
            delete (w);
        }break;
        case SIGUSR1:{
            execl("../world","../world,",argv_extra,(char*)NULL);
        }
    return;
    }
}


// /===========================================================/
// MAIN

RunningInstance::RunningInstance( std::string pid_filepath )
    : pid_filepath( pid_filepath )
{
}

RunningInstance::~RunningInstance()
{
    close( pid_fd );
}

void RunningInstance::world_running( char* pid_filepath )
{
    int pid_fd = open( pid_filepath, O_CREAT | O_RDWR, 0666 );
    if ( flock( pid_fd, LOCK_EX | LOCK_NB ) ) {
        switch ( errno ) {
        // Another instance is running
        case EWOULDBLOCK:
            std::cerr << "Another instance of world is already running." << std::endl;
            std::cerr << "Waiting for its end." << std::endl;
            watch_pid( pid_filepath );
            break;
        default:
            assert(false);
        }
    }
}

void RunningInstance::watch_pid( char* pid_filepath )
{
    int inotify_instance = inotify_init();
    if ( inotify_instance == -1 ) {
        std::cerr < "Failed to create inotify instance" < std::endl;
        assert(false);
    }
    int watch_fd = inotify_add_watch( inotify_instance, pid_filepath, IN_CLOSE);
    // struct inotify_event event;
    // read(inotify_instance, &event, sizeof(struct inotify_event) + NAME_MAX + 1);
    // pselect(inotify_instance, NULL, NULL, NULL, NULL, sigmask?);

    // Blocking call waiting for the end of a different world
    select(inotify_instance, NULL, NULL, NULL, NULL);
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&mtx,NULL);
    pthread_cond_init(&cvar,NULL);
    argv_extra = argv;
    struct sigaction action;
    action.sa_flags=0;
    action.sa_handler = main_sig_handler;
    sigaction(SIGINT,&action,NULL);
    sigaction(SIGQUIT,&action,NULL);
    sigaction(SIGTERM,&action,NULL);
    sigaction(SIGUSR1,&action,NULL);
    Utils mUtils(argc, argv);

    int pid_fd = world_running("/var/run/world.pid");

    if((mUtils.getMapHeight()*mUtils.getMapWidth()) < (mUtils.getGreenTanks()+ mUtils.getRedTanks())){
        std::cerr << "Not enough space on map for tanks, exiting" << std::endl;
        return 2;
    }
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
        TankClient t = TankClient(c.first, c.second, GREEN);
        w->add_tank(t,mUtils);
    }
    for (uint i = 0; i < mUtils.getRedTanks(); i++)
    {
        Coord c = w->free_coord();
        TankClient t = TankClient(c.first, c.second, RED);
        w->add_tank(t,mUtils);
    }

    while(true)
    {
        w->play_round(mUtils);
    }

    return 0;
}
