#include "world.h"
#include "world_shared.h"

volatile std::sig_atomic_t World::world_signal_status = 0;

World::World(WorldOptions& opts)
    : height(opts.get_map_height()),
      width(opts.get_map_width()),
      zone(height, std::vector<Color>(width, Color::EMPTY)),
      map_fifo(opts.get_fifo_path())
{
    red_tanks.reserve(opts.get_red_tanks());
    green_tanks.reserve(opts.get_green_tanks());

    if (opts.get_daemonize()) {
        /* Replace std::cout with custom syslog stream */
        std::cout.rdbuf(new Log("Internet of Tanks", LOG_USER, LOG_INFO));
    }
}

void World::play_round(WorldOptions opts)
{
    while(true) {
        // re-inited at every round start for easier management
        opts.incRoundsPlayed();

        respawn_tanks(opts);
        /* Acquire commands from tankclients */
        read_commands();
        process_shots();
        process_moves();

        sum_score(opts);
        std::cout << "Score:" << std::endl
                  << "Red: " << opts.getRedKills() << std::endl
                  << "Green: " << opts.getGreenKills() << std::endl
                  << "Round " << opts.get_rounds_played() << std::endl;

        take_actions();
        refresh_zone();
        output_map();
        if (handle_signals()) {
            break;
        }
        // waits for round time to pass
        std::this_thread::sleep_for(std::chrono::seconds(opts.getRoundTime()));
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
    //std::cout << "DEBUG P1 reach" << std::endl;
    for(auto& box_t : boost::join(green_tanks, red_tanks)) {
        Tank& t = *box_t.get();
        //std::cout << "DEBUG P2 reach" << std::endl;
        //std::cout << "DEBUG command[0]: " << t.get_command()[0] << std::endl;
        //std::cout << "DEBUG command[1]: " << t.get_command()[1] << std::endl;
        //std::cout << "DEBUG command: " << t.get_command() << std::endl;
        if (t.get_command()[0] == 'm') {
            //std::cout << "DEBUG P2 reach" << std::endl;
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
            /* malformed command format */
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
        t.quit();
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
    return zone[x][y] == Color::EMPTY;
}

Coord World::free_coord()
{
    int x;
    int y;
    std::uniform_int_distribution<int> width_rand(0, width - 1);
    std::uniform_int_distribution<int> height_rand(0, height - 1);
    // only loops if first try failed,ends as soon as a free field is found
    do {
        x = width_rand(rng);
        y = height_rand(rng);
    } while (!is_free(x, y));
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
    map_fifo << width << ',' << height;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            map_fifo << ',' << zone[i][j];
        }
    }
}

void World::close()
{
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        (*t)->quit();
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        (*t)->quit();
    }
}

void World::refresh_zone()
{
    for (int i = 0; i < height; i++) {
        std::fill(zone[i].begin(), zone[i].end(), Color::EMPTY);
        /*
        for (int j = 0; j < width; i++) {
            zone[i][j] = Color::EMPTY;
        }
        */
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

int main(int argc, char *argv[])
{
    //process_signal_handling();
    //set_up_thread_hadler(World::set_world_signal_status);

    RunningInstance instance("world.pid");
    if (instance.acquire() <= 0) {
        return EXIT_FAILURE;
    }

    WorldOptions opts;
    if (opts.parse_options(argc, argv)) {
        return EXIT_FAILURE;
    }
    if (opts.check_valid()) {
        return EXIT_FAILURE;
    }

    NamedFifo map_fifo(opts.get_fifo_path());
    if (map_fifo.open() != 0) {
        std::cout << "Failed to open " << opts.get_fifo_path() << ": "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    World w(opts);

    w.play_round(opts);

    return EXIT_SUCCESS;
}
