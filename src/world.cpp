#include "world.h"

volatile std::sig_atomic_t World::world_signal_status = 0;

World::World(WorldOptions& opts)
    : height(opts.get_map_height()),
      width(opts.get_map_width()),
      pipe(opts.get_fifo_path())
{
    std::vector<std::vector<Color>> zone(height,
                                         std::vector<Color>(width, EMPTY));
    /* fixme!: Handle error when given pipe name already exists*/
    pipefd = mkfifo(pipe.c_str(), 0444);
    if (pipefd) {
        std::cout << "DEBUG: " << pipefd << std::endl;
        std::cout << "DEBUG: " << strerror(errno) << std::endl;
    }
    red_tanks.reserve(opts.get_red_tanks());
    green_tanks.reserve(opts.get_green_tanks());
    std::cout.rdbuf(new Log("Internet of Tanks", LOG_USER, LOG_INFO));
}

void World::play_round(WorldOptions u)
{
    while(true) {
        // re-inited at every round start for easier management
        u.incRoundsPlayed();

        respawn_tanks(u);
        /* Acquire commands from tankclients */
        read_commands();
        process_shots();
        process_moves();

        sum_score(u);
        std::cout << "Score:" << std::endl
                  << "Red: " << u.getRedKills() << std::endl
                  << "Green: " << u.getGreenKills() << std::endl;
        std::cout << "Round " << u.getRoundsPlayed() << std::endl;

        take_actions();
        refresh_zone();
        output_map();
        if (handle_signals()) {
            break;
        }
        // waits for round time to pass : round time given in ms,
    }
}

void World::read_commands()
{
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        t.read_command();
    }
}

void World::process_shots()
{
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        if (t.get_command()[0] == 'f') {
            fire_direction(t);
        }
    }
}

void World::process_moves()
{
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        if (t.get_command()[0] == 'm') {
            movetank(t);
        }
    }
}

void World::fire_direction(Tank& t)
{
    for (auto& box_target : boost::join(green_tanks, red_tanks)) {
        Tank& target = *box_target.get();
        /* Pick the right operation for finding targets relative to this tank's
         * position */
        std::function<bool(int, int)> x_op = std::equal_to<int>();
        std::function<bool(int, int)> y_op = std::equal_to<int>();
        switch (t.get_command()[1]) {
        case 'u':
            y_op = std::less<int>();
            break;
        case 'd':
            y_op = std::greater<int>();
            break;
        case 'l':
            x_op = std::less<int>();
            break;
        case 'r':
            x_op =  std::greater<int>();
            break;
        default:
            /* Invalid command */
            assert(false);
        }

        if (y_op(target.get_y(), t.get_y()) &&
            x_op(target.get_x(), t.get_x()))
        {
            t.get_shot();
            t.print_destroyed(target);
        }
    }
}

void World::movetank(Tank& t)
{
    for (auto& box_obstacle : boost::join(green_tanks, red_tanks)) {
        Tank& obstacle = *box_obstacle.get();
        int x_shift = 0;
        int y_shift = 0;

        switch (t.get_command()[1]) {
        case 'u':
            y_shift++;
            break;
        case 'd':
            y_shift--;
            break;
        case 'l':
            x_shift--;
            break;
        case 'r':
            x_shift++;
            break;
        default:
            assert(false);
        }

        /* Set new coordinates only for tanks that haven't been shot or crashed */
        Coord new_pos = Coord(t.get_x() + x_shift, t.get_y() + y_shift);
        if (&t != &obstacle &&
            new_pos == obstacle.get_position() &&
            !obstacle.is_shot() &&
            !t.is_shot())
        {
            /* crash tanks */
            t.get_crashed();
            obstacle.get_crashed();
            t.print_crashed(obstacle);
        } else if (out_of_bounds(new_pos)) {
            t.get_crashed();
            t.print_out_of_map();
        } else {
            /* Set new cooradinates for tank */
            t.set_new_position(new_pos);
        }
    }
}

bool World::out_of_bounds(Coord pos)
{
    return (pos.first < 0 || pos.first > width || pos.second < 0 ||
            pos.second > height);
}

void World::take_actions()
{
    for (auto it_box_t = red_tanks.begin(); it_box_t != red_tanks.end(); it_box_t++) {
        take_action_tank(it_box_t);
    }
    for (auto it_box_t = green_tanks.begin(); it_box_t != green_tanks.end(); it_box_t++) {
        take_action_tank(it_box_t);
    }
}

void World::take_action_tank(std::vector<std::unique_ptr<Tank>>::iterator& it_box_t)
{
    Tank& t = *it_box_t->get();
    auto& tanks = t.get_color() == Color::RED ? red_tanks : green_tanks;
    if (!t.is_alive()) {
        //this->zone[t.get_x()][t.get_x()] = Color::EMPTY;
        t.kill_thread();
        tanks.erase(it_box_t);
    } else {
        t.move();
    }
}

void World::add_tank(Color color)
{
    Coord pos = free_coord();

    this->zone[pos.first][pos.second] = color;

    if (color == Color::RED) {
        red_tanks.emplace_back(new Tank(pos, color));
    }
    else {
        green_tanks.emplace_back(new Tank(pos, color));
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
        x = width_rand(rng);
        y = height_rand(rng);
    } while (is_free(x, y));
    return Coord(x, y);
}


void World::sum_score(WorldOptions u)
{
    for (auto& box_t : red_tanks) {
        Tank& t = *box_t.get();
        if (t.is_shot()) {
            u.incGreenKills();
        }
    }
    for (auto& box_t : green_tanks) {
        Tank& t = *box_t.get();
        if (t.is_shot()) {
            u.incRedKills();
        }
    }
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

void World::output_map()
{
    std::stringstream ss;
    ss << width << ',' << height;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ss << ',' << zone[i][j];
        }
    }
    write(pipefd, ss.str().c_str(), (height * width * 2) + 4);
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

bool World::handle_signals() {
    bool ret = true;
    //fixme: Should acces to world_signal)status be atomic?
    switch (world_signal_status) {
    case SIGUSR1:
        /* restart game */
        break;
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        /* World will go out of scope */
        /* No need to call close explicitly */
        // close();
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

void World::refresh_zone()
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; i++) {
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
    int locked;
    if (!pid_fd) {
        std::cout << "Failed to open pid file: " << strerror(errno) << std::endl;
        exit(-1);
    }

    while ((locked = flock(pid_fd, LOCK_EX | LOCK_NB))) {
        switch (errno) {
        // Another instance is running
        case EWOULDBLOCK:
            std::cerr << "Another instance of world is already running."
                      << std::endl;
            std::cerr << "Waiting for its end." << std::endl;
            watch_pid(pid_filepath);
            break;
        //default:
            //assert(false);
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

    int pid_fd = world_running("world.pid");

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

    w->play_round(opts);

    close(pid_fd);
    return 0;
}
