#include "world.h"

volatile std::sig_atomic_t World::world_signal_status = 0;

// extra var to pass ARGV through for restart
char** argv_extra;

// World::World(uint height, uint width, std::string pipe)
//     : height(height), width(width), pipe(pipe)
World::World(WorldOptions& opts)
    : height(opts.get_map_height()), width(opts.get_map_width()), pipe(opts.get_fifo_path())
{
    std::vector<std::vector<Color>> zone(height, std::vector<Color>(width, EMPTY));
    pipefd = mkfifo(pipe.c_str(), 0444);
    red_tanks.reserve(opts.get_red_tanks());
    green_tanks.reserve(opts.get_green_tanks());
}

void World::add_tank(Color color)
{
    Coord pos = free_coord();

    this->zone[pos.first][pos.second] = color;

    if(color == Color::RED)
    {
        std::cout << "Adding red tank" << std::endl;
        red_tanks.emplace_back(new Tank(pos, color));
        //red_tanks.push_back(std::unique_ptr<Tank>(new Tank(c.first, c.second, color, bin_path)));
    }
    else
    {
        std::cout << "Adding green tank" << std::endl;
        green_tanks.emplace_back(new Tank(pos, color));
        //green_tanks.push_back(std::unique_ptr<Tank>(new Tank(c.first, c.second, color, bin_path)));
    }
}

bool World::is_free(int x, int y)
{
    return zone[x][y] == EMPTY;
}

Coord World::free_coord()
{
    int x;
    int y;
    std::uniform_int_distribution<int> width_rand(0, width);
    std::uniform_int_distribution<int> height_rand(0, height);
    // only loops if first try failed,ends as soon as a free field is found
    do {
        x = rng.uniform(0u, width);
        y = rng.uniform(0u, height);
        //x = width_rand(rng);
        //y = height_rand(rng);
    } while (is_free(x, y));
    return Coord(x, y);
}

void World::play_round(WorldOptions u)
{
    // re-inited at every round start for easier management
    u.incRoundsPlayed();

    /* Acquire commands from tankclients */
    read_commands();
    /* Create appropriate callbacks for each tank based on it's command */
    process_commands();
    /* Execute given all tanks' callbacks -> change tanks' state */
    take_actions();

    add_kills(u);
    std::cout << "Score:" << std::endl
              << "Red: " << u.getRedKills() << std::endl
              << "Green: " << u.getGreenKills() << std::endl;
    respawn_tanks(u);
    std::cout << "Round " << u.getRoundsPlayed() << std::endl;
    output_map();
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}

void World::process_commands()
{
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        if (t.get_command()[0] == 'f') {
            fire_direction(t);
        }
        else if (t.get_command()[0] == 'm') {
            movetank(t);
        }
    }
}

void World::take_actions()
{
    /*
    for(auto& t : boost::join(green_tanks, red_tanks)) {
        t.take_action();
    }
    */
    for (auto t = red_tanks.begin(); t != red_tanks.end(); t++) {
        //if (t->take_action())
        //if (t->check_bounds(height, width)) {
        //}
    }
    for (auto t = green_tanks.begin(); t != green_tanks.end(); t++) {
        //if (t->take_action())
    }
}

//void World::fire_direction(std::unique_ptr<Tank> t)
void World::fire_direction(Tank& t)
{
    //auto& foe_tanks = t.get_color() == Color::GREEN ? red_tanks : green_tanks;
    //for (auto& box_target : foe_tanks) {
    for (auto& box_target : boost::join(green_tanks, red_tanks)) {
        Tank& target = *box_target.get();
        std::function<bool(int, int)> x_op;
        std::function<bool(int, int)> y_op;
        switch (t.get_command()[1]) {
        case 'u':
            //x_op = [](int a, int b) { return a == b; };
            //y_op = [](int a, int b) { return a < b; };
            x_op = std::equal_to<int>();
            y_op = std::less<int>();
            break;
        case 'd':
            //x_op = [](int a, int b) { return a == b; };
            //y_op = [](int a, int b) { return a > b; };
            x_op = std::equal_to<int>();
            y_op =  std::greater<int>();
            break;
        case 'l':
            //x_op = [](int a, int b) { return a < b; };
            //y_op = [](int a, int b) { return a == b; };
            x_op = std::less<int>();
            y_op = std::equal_to<int>();
            break;
        case 'r':
            //x_op = [](int a, int b) { return a > b; };
            //y_op = [](int a, int b) { return a == b; };
            x_op =  std::greater<int>();
            y_op = std::equal_to<int>();
            break;
        default:
            /* Invalid command */
            assert(false);
        }

        if (y_op(target.get_y(), t.get_y()) &&
            x_op(target.get_x(), t.get_x()) && !t.is_dead())
        {
            t.print_destroyed(target);
            t.make_dead();
        }
    }
}

//void World::movetank(std::unique_ptr<Tank> t)
void World::movetank(Tank& t)
{
    for (auto& box_obstacle : boost::join(green_tanks, red_tanks)) {
        Tank& obstacle = *box_obstacle.get();
        int x_direction = 0;
        int y_direction = 0;
        switch (t.get_command()[1]) {
        case 'u':
            //action = std::bind(&Tank::moveup, t);
            y_direction++;
            break;
        case 'd':
            //action = std::bind(&Tank::movedown, t);
            y_direction--;
            break;
        case 'l':
            //action = std::bind(&Tank::moveleft, t);
            x_direction--;
            break;
        case 'r':
            //action = std::bind(&Tank::moveright, t);
            x_direction++;
            break;
        default:
            assert(false);
        }
        Coord new_pos = Coord(t.get_x() + x_direction, t.get_y() + y_direction);
        if (&t != &obstacle && new_pos == obstacle.get_position()) {
            /* crash tanks */
            t.subscribe_action([&] {
                std::cout << "Crash tanks: " << t.get_color() << " " << obstacle.get_color() << std::endl;
                return true;
            });
        } else {
            /* move tank to new position */
            t.subscribe_action(std::bind(&Tank::move, &t, height, width, new_pos));
        }
    }
}

/*
bool World::check_bounds(int x, int y)
{
    return (x < 0 || x > width || y < 0 || y > height);
}

bool World::check_bounds(Coord c)
{
    return check_bounds(c.first, c.second);
}
*/


/*
void World::crash_tanks()
{
    for (auto& t : boost::join(green_tanks, red_tanks)) {
        for (auto& u : boost::join(green_tanks, red_tanks)) {
            if (&t == &u || t->get_hit() || u->get_hit()) {
                continue;
            }
            else if (t->get_position() == u->get_position()) {
                t->hit_tank(TankShell(u->get_x(), u->get_y(), u->get_color()));
                u->hit_tank(TankShell(t->get_x(), t->get_y(), t->get_color()));
            }
        }
    }
}
*/

void World::add_kills(WorldOptions u)
{
    for (auto& t : red_tanks) {
        if (t->is_dead()) {
            u.incGreenKills();
        }
    }
    for (auto& t : green_tanks) {
        if (t->is_dead()) {
            u.incRedKills();
        }
    }
}

void World::remove_dead_tanks()
{
    for (auto t = red_tanks.begin(); t != red_tanks.end(); t++) {
        if ((*t)->is_dead()) {
            //this->zone[(*t)->get_x()][(*t)->get_x()] = Color::EMPTY;
            //(*t)->kill_thread();
            remove_tank(**t);
            red_tanks.erase(t);
        }
    }
    for (auto t = green_tanks.begin(); t != green_tanks.end(); t++) {
        if ((*t)->is_dead()) {
            remove_tank(**t);
            green_tanks.erase(t);
        }
    }
}

void World::remove_tank(Tank& t)
{
    this->zone[t.get_x()][t.get_x()] = Color::EMPTY;
    t.kill_thread();
}

void World::respawn_tanks(WorldOptions opts)
{
    while(red_tanks.size() < opts.get_red_tanks()){
        add_tank(Color::RED);
    }
    while(green_tanks.size() < opts.get_green_tanks()){
        add_tank(Color::GREEN);
    }
}

void World::read_commands()
{
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        t.read_command();
    }
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
        (*t)->kill_thread();
        (*t)->quit();
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        (*t)->kill_thread();
        (*t)->quit();
    }
    red_tanks.clear();
    green_tanks.clear();
    zone.clear();
    ::close(pipefd);
}

void World::set_world_signal_status(int sig, siginfo_t* info, void* context) {
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

void World::refresh_zone()
{
    std::cout << "Refreshing map" << std::endl;
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;i++){
            zone[i][j] = EMPTY;
        }
    }
    for (auto& box_t : red_tanks) {
        Tank& t = *box_t.get();
        zone[t.get_x()][t.get_y()] = Color::RED;
    }
    for (auto& box_t : green_tanks) {
        Tank& t = *box_t.get();
        zone[t.get_x()][t.get_y()] = Color::GREEN;
    }
}

int world_running(std::string pid_filepath)
{
    // Possibly handle other errors when calling open()
    int pid_fd = open(pid_filepath.c_str(), O_CREAT | O_RDWR, 0666);
    while (flock(pid_fd, LOCK_EX | LOCK_NB)) {
        switch (errno) {
        // Another instance is running
        case EWOULDBLOCK:
            std::cerr << "Another instance of world is already running."
                      << std::endl;
            std::cerr << "Waiting for its end." << std::endl;
            watch_pid(pid_filepath);
            continue;
        default:
            assert(false);
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

    /* Blocking call waiting for the end of a different world */
    select(inotify_instance, NULL, NULL, NULL, NULL);
}

int main(int argc, char *argv[])
{
    //process_signal_handling();
    //set_up_thread_hadler(World::set_world_signal_status);

    int pid_fd = world_running("/var/run/world.pid");

    // argv_extra = argv;

    WorldOptions opts;
    opts.parse_options(argc, argv);

    // Checking if map space is sufficient
    int map_space = opts.get_map_height() * opts.get_map_width();
    int tank_count = opts.get_green_tanks() + opts.get_red_tanks();
    if(map_space < tank_count) {
        std::cerr << "Not enough space on map for tanks, exiting" << std::endl;
        return 2;
    }

    //std::unique_ptr<World> w(new World(opts.get_map_height(), opts.get_map_width(), opts.get_fifo_path()));
    std::unique_ptr<World> w(new World(opts));

    for (uint i = 0; i < opts.get_green_tanks(); i++)
    {
        w->add_tank(Color::GREEN);
    }
    for (uint i = 0; i < opts.get_red_tanks(); i++)
    {
        w->add_tank(Color::RED);
    }

    while(true)
    {
        w->play_round(opts);
    }

    close(pid_fd);
    return 0;
}
