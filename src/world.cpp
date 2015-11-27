#include "world.h"
#include "daemonworld.h"

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
void World::add_tank(Tank t, WorldOptions u)
{
    if(t.getColor() == Color::RED)
    {
        std::cout << "Adding red tank" << std::endl;
        red_tanks.push_back(t);
        // spawn_thread(t, u.getRedPath());
    }
    else
    {
        std::cout << "Adding green tank" << std::endl;
        green_tanks.push_back(t);
        // spawn_thread(t, u.getGreenPath());
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
    for(Tank& t : boost::join(green_tanks, red_tanks))
    {
        if (t.get_action().size() != 0 && t.get_action()[0] == 'f')
        {
            fire_direction(t);
        }
    }
}

void World::fire_direction(Tank& t) {
    auto& foe_tanks = t.getColor() == Color::GREEN ? red_tanks : green_tanks;
    for (Tank& target : foe_tanks)
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
    for(Tank& t : boost::join(green_tanks, red_tanks))
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

void World::crash_tanks(std::vector<Tank> tanks1,
                        std::vector<Tank> tanks2)
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

void World::add_kills(WorldOptions u)
{
    for (Tank& t : red_tanks) {
        if (t.getHit()) {
            u.incGreenKills();
        }
    }
    for (Tank& t : green_tanks) {
        if (t.getHit()) {
            u.incRedKills();
        }
    }
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

void World::respawn_tanks(WorldOptions u)
{
    while(red_tanks.size() < u.get_red_tanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.get_green_tanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        add_tank(t,u);
    }
}

void World::refresh_zone()
{
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

/*
void World::process_commands( WorldOptions u, std::vector< std::string > ra, std::vector< std::string > ga )
{
    for ( auto m = tank_messages.begin(); m != tank_messages.end(); ++m ) {
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
*/

void World::play_round(WorldOptions u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    red_actions.resize(u.get_red_tanks());
    green_actions.resize(u.get_green_tanks());
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
    std::cout << "Adding kills, old counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
    add_kills(u);
    std::cout << "New kill counts: " << std::endl
    << "Red: " << u.getRedKills() << std::endl
    << "Green: " << u.getGreenKills() << std::endl;
    remove_hit_tanks();
    std::cout << "Respawning tanks" << std::endl;
    respawn_tanks(u);
    std::cout << "Refreshing map" << std::endl;
    refresh_zone();
    std::cout << "Round " << u.getRoundsPlayed() << std::endl;
    output_map();
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}

void World::output_map()
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

void World::close()
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
        close();
        break;
    case SIGUSR1:
        close();
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

int main(int argc, char *argv[])
{
    set_up_signal_handling();
    int pid_fd = world_running("/var/run/world.pid");

    // pthread_mutex_init(&mtx,NULL);
    // pthread_cond_init(&cvar,NULL);
    // argv_extra = argv;

    WorldOptions opts(argc, argv);

    // Checking if map space is sufficient
    int map_space = opts.get_map_height()*opts.get_map_width();
    int tank_count = opts.get_green_tanks()+ opts.get_red_tanks();
    if(map_space < tank_count) {
        std::cerr << "Not enough space on map for tanks, exiting" << std::endl;
        return 2;
    }

    World w = World(opts.get_map_height(), opts.get_map_width());
    if (opts.get_daemonize()) {
        /* fixme: supply fifo path from parsed arguments */
        std::string open_pipe;
        w = DaemonWorld(opts.get_map_height(), opts.get_map_width(), open_pipe);
    }

    for (uint i = 0; i < opts.get_green_tanks(); i++)
    {
        Coord c = w.free_coord();
        Tank t = Tank(c.first, c.second, Color::GREEN);
        w.add_tank(t,opts);
    }
    for (uint i = 0; i < opts.get_red_tanks(); i++)
    {
        Coord c = w.free_coord();
        Tank t = Tank(c.first, c.second, Color::RED);
        w.add_tank(t,opts);
    }

    while(true)
    {
        w.play_round(opts);
    }

    close(pid_fd);
    return 0;
}
