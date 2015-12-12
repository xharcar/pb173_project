#include "world.h"

volatile std::sig_atomic_t World::world_signal_status = 0;

// extra var to pass ARGV through for restart
char** argv_extra;

// METHOD IMPLEMENTATIONS

WorldOptions::WorldOptions(int argc, char* argv[])
    // Set default values here
    : mDaemonize(false)
    , mGreenPath("../bin/tank")
    , mRedPath("../bin/tank")
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
    while ((c = getopt_long(argc, argv, "dc:v:g:r:t:a:p:h", longopts, NULL)) != -1)
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
            this->mRedPath.assign(optarg);
            break;
        case 'g' :
            this->mGreenPath.assign(optarg);
            break;
        case 't':
            this->mRoundTime = atoi(optarg);
            break;
        case 'p':
            this->fifoPath.assign(optarg);
            break;
        case 'h':
            this->print_help();
            exit(0);
        default:
            this->print_error();
            exit(-1);
        }
    }
    if(mMapHeight<=0 || mMapWidth<=0 || mRoundTime <=0)
    {
        this->print_help();
        exit(-1);
    }
}

void WorldOptions::print_help()
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

void WorldOptions::print_error()
{
    std::cerr << "Wrong arguments or something" << std::endl;
}

// WORLD
void World::add_tank(Tank t)
{
    if(t.getColor() == Color::RED)
    {
        std::cout << "Adding red tank" << std::endl;
        red_tanks.push_back(t);
        t.spawn_thread();
    }
    else
    {
        std::cout << "Adding green tank" << std::endl;
        green_tanks.push_back(t);
        t.spawn_thread();
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
    for(uint i=0;i<green_tanks.size();++i)
    {
        if (green_tanks[i].get_action().size() != 0 && green_tanks[i].get_action()[0] == 'f')
        {
            fire_direction(green_tanks[i]);
        }
    }
	for(uint i=0;i<red_tanks.size();++i)
    {
        if (red_tanks[i].get_action().size() != 0 && red_tanks[i].get_action()[0] == 'f')
        {
            fire_direction(red_tanks[i]);
        }
    }
}

void World::fire_direction(Tank t){    
    for (uint i=0;i<green_tanks.size();++i)
    {
        switch(t.get_action()[1])
        {
        case 'u':
            if (green_tanks[i].getY() < t.getY() && green_tanks[i].getX() == t.getX())
            {
                green_tanks[i].hit_tank();
            }
            break;
        case 'd':
            if (green_tanks[i].getY() > t.getY() && green_tanks[i].getX() == t.getX())
            {
                green_tanks[i].hit_tank();
            }
            break;
        case 'l':
            if (green_tanks[i].getY() == t.getY() && green_tanks[i].getX() < t.getX())
            {
                green_tanks[i].hit_tank();
            }
            break;
        case 'r':
            if (green_tanks[i].getY() == t.getY() && green_tanks[i].getX() < t.getX())
            {
               green_tanks[i].hit_tank();
            }
            break;
        default:
            assert(false);
        }
    }
  for (uint i=0;i<red_tanks.size();++i)
    {
        switch(t.get_action()[1])
        {
        case 'u':
            if (red_tanks[i].getY() < t.getY() && red_tanks[i].getX() == t.getX())
            {
                red_tanks[i].hit_tank();
            }
            break;
        case 'd':
            if (red_tanks[i].getY() > t.getY() && red_tanks[i].getX() == t.getX())
            {
                red_tanks[i].hit_tank();
            }
            break;
        case 'l':
            if (red_tanks[i].getY() == t.getY() && red_tanks[i].getX() < t.getX())
            {
                red_tanks[i].hit_tank();
            }
            break;
        case 'r':
            if (red_tanks[i].getY() == t.getY() && red_tanks[i].getX() < t.getX())
            {
                red_tanks[i].hit_tank();
            }
            break;
        default:
            assert(false);
        }
    }
}

void World::movetanks()
{
    for(uint i = 0;i<green_tanks.size();++i)
    {
        if (green_tanks[i].get_action().size() != 0
            && green_tanks[i].get_action()[0] == 'm'
            && !green_tanks[i].isHit())
        {
            switch (green_tanks[i].get_action()[1])
            {
            case 'u' :
                green_tanks[i].moveup();
                break;
            case 'd' :
                green_tanks[i].movedown();
                break;
            case 'l' :
                green_tanks[i].moveleft();
                break;
            case 'r' :
                green_tanks[i].moveright();
                break;
            default:
                assert(false);
            }
        }
    }
    for(uint i = 0;i<red_tanks.size();++i)
    {
        if (red_tanks[i].get_action().size() != 0
            && red_tanks[i].get_action()[0] == 'm'
            && !red_tanks[i].isHit())
        {
            switch (red_tanks[i].get_action()[1])
            {
            case 'u' :
                red_tanks[i].moveup();
                break;
            case 'd' :
                red_tanks[i].movedown();
                break;
            case 'l' :
                red_tanks[i].moveleft();
                break;
            case 'r' :
                red_tanks[i].moveright();
                break;
            default:
                assert(false);
            }
        }
    }

}

void World::crash_tanks(std::vector<Tank> tanks1,
                        std::vector<Tank> tanks2)
{
    for(std::size_t i=0;i<tanks1.size();++i){
        for(std::size_t j=0;j<tanks2.size();++j){
            if(tanks1[i].getTID() != tanks2[j].getTID() &&
               // prevents a tank crashing into itself
               tanks1[i].getX() == tanks2[j].getX() &&
               tanks1[i].getY() == tanks2[j].getY() &&
               !tanks1[i].isHit() && !tanks2[j].isHit()){
                    tanks1[i].hit_tank();
                    tanks2[j].hit_tank();
            }
        }
    }
}

void World::add_kills(WorldOptions u)
{
    std::cout << "Adding kills, old counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
    for(std::size_t i=0;i<red_tanks.size();++i){
        if(red_tanks[i].isHit()) u.incGreenKills();
    }
    for(std::size_t i=0;i<green_tanks.size();++i){
        if(green_tanks[i].isHit()) u.incRedKills();
    }
    std::cout << "New kill counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
}

void World::remove_hit_tanks()
{
    std::cout << "Removing hit tanks" << std::endl;
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        if(t->isHit()){
            std::cout << "Red tank with TID " << t->getTID() << " at "
            << t->getX() << "," << t->getY() << " has been hit, removing" << std::endl;
            t->kill_thread();
            red_tanks.erase(t);
        }
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        if(t->isHit()){
            std::cout << "Green tank with TID " << t->getTID() << " at "
            << t->getX() << "," << t->getY() << " has been hit, removing" << std::endl;
            t->kill_thread();
            green_tanks.erase(t);
        }
    }
}

void World::respawn_tanks(WorldOptions u)
{
    std::cout << "Respawning tanks" << std::endl;
    while(red_tanks.size() < u.get_red_tanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, RED);
        add_tank(t);
    }
    while(green_tanks.size() < u.get_green_tanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        add_tank(t);
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


void World::process_commands( WorldOptions u, std::vector< std::string > ra, std::vector< std::string > ga )
{
    for ( uint i = 0;i<tank_messages.size();i++ ) {
        char* pch = strtok((char*)((tank_messages[i]).c_str())," ");
        pthread_t a = (pthread_t)atoi(pch);
        pch = strtok(NULL," ");
        for(uint i=0;i<u.get_red_tanks();i++){
            if(red_tanks[i].getTID()==a){
                ra[i].assign(pch);
                break;
            }
        }
        for(uint i=0;i<u.get_green_tanks();i++){
            if(green_tanks[i].getTID()==a){
                ga[i].assign(pch);
                break;
            }
        }
    }
    tank_messages.clear();
}

void World::play_round(WorldOptions u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    red_actions.resize(u.get_red_tanks());
    green_actions.resize(u.get_green_tanks());
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    pthread_cond_signal(&worldcvariable);
    usleep((useconds_t)u.getRoundTime()*1000);
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
    //process_commands(u,red_actions,green_actions);
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

void World::safe_quit()
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

void World::set_world_signal_status(int sig) {
    World::world_signal_status = sig;
}

void World::handle_signal(int sig)
{
    switch(sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
        safe_quit();
        break;
    case SIGUSR1:
        safe_quit();
        execl("../world","../world,",argv_extra,(char*)NULL);
        break;
    }
}

int world_running( std::string pid_filepath )
{
    // Possibly handle other errors when calling open()
    int pid_fd = open( pid_filepath.c_str(), O_CREAT | O_RDWR, 0666 );
    while ( flock( pid_fd, LOCK_EX | LOCK_NB ) ) {
        switch ( errno ) {
        // Another instance is running
        case EWOULDBLOCK:
            std::cerr << "Another instance of world is already running." << std::endl;
            std::cerr << "Waiting for its end." << std::endl;
            watch_pid( pid_filepath );
            continue;
        default:
            assert( false );
        }
    }
    return pid_fd;
}

void watch_pid( std::string pid_filepath )
{
    int inotify_instance = inotify_init();
    if ( inotify_instance == -1 ) {
        std::cerr << "Failed to create inotify instance" << std::endl;
        assert(false);
    }
    if (inotify_add_watch( inotify_instance, pid_filepath.c_str(), IN_CLOSE)) {
        std::cerr << "Failed to add file in to inotify instance" << std::endl;
        std::abort();
    }

    // Blocking call waiting for the end of a different world
    select(inotify_instance, NULL, NULL, NULL, NULL);
}

void set_up_signal_handling()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = World::set_world_signal_status;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

}
