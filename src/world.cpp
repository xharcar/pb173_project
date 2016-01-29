#include "world.h"
#include "world_shared.h"

volatile std::sig_atomic_t World::world_signal_status = 0;

World::World(WorldOptions& opts)
    : height(opts.get_map_height()),
      width(opts.get_map_width()),
      zone(height, std::vector<Color>(width, Color::EMPTY)),
      map_fifo(opts.get_fifo_path()),
      opts(opts)
{
    tanks.reserve(opts.get_red_tanks() + opts.get_green_tanks());

    if (opts.get_daemonize()) {
        /* Replace std::cout with custom syslog stream */
        std::cout.rdbuf(new Log("Internet of Tanks", LOG_USER, LOG_INFO));
    }
}

void World::play_round()
{
    while(true) {
        // re-inited at every round start for easier management
        rounds_played++;

        respawn_tanks();
        /* Acquire commands from tankclients */
        read_commands();
        process_shots();
        process_moves();

        sum_score();
        std::cout << "Score:" << std::endl
                  << "Red: " << red_kills << std::endl
                  << "Green: " << green_kills << std::endl
                  << "Round " << rounds_played << std::endl;

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
    for(auto& box_t : tanks) {
        Tank& t = *box_t.get();
        t.read_command();
    }
}

void World::process_shots()
{
    for(auto& box_t : tanks) {
        Tank& t = *box_t.get();
        if (t.get_command()[0] == 'f') {
            fire_direction(t);
        }
    }
}

void World::process_moves()
{
    for(auto& box_t : tanks) {
        Tank& t = *box_t.get();
        if (t.get_command()[0] == 'm') {
            movetank(t);
        }
    }
}

void World::fire_direction(Tank& t)
{
    for (auto& box_target : tanks) {
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
    for (auto& box_obstacle : tanks) {
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
    /*
    for (auto it_box_t = tanks.begin(); it_box_t != tanks.end(); it_box_t++) {
        Tank& t = *it_box_t->get();
        if (!t.is_alive()) {
            t.quit();
            tanks.erase(it_box_t);
        } else {
            t.move();
        }
    }
    */
    for(auto& box_t : tanks) {
        Tank& t = *box_t.get();
        if (!t.is_alive()) {
            if (t.get_color() == Color::RED) {
                red_tanks--;
            } else {
                green_tanks--;
            }
            t.quit();
            tanks.erase(box_t);
        } else {
            t.move();
        }
    }
}

void World::add_tank(Color color)
{
    Coord pos = free_coord();
    this->zone[pos.first][pos.second] = color;

    if (color == Color::RED) {
        red_tanks++;
    } else {
        green_tanks++;
    }
    tanks.emplace(new Tank(pos, color));
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


void World::sum_score()
{
    for (auto& box_t : tanks) {
        Tank& t = *box_t.get();
        if (t.is_shot()) {
            if (t.get_color() == Color::RED) {
                green_kills++;
            } else {
                red_kills++;
            }
        }
    }
}

void World::respawn_tanks()
{
    while (red_tanks < opts.get_red_tanks()) {
        add_tank(Color::RED);
    }
    while (green_tanks < opts.get_green_tanks()) {
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

void World::restart() {
    for(auto& box_t : tanks) {
        Tank& t = *box_t.get();
        t.quit();
        tanks.erase(box_t);
    }
    green_kills = 0;
    red_kills = 0;
    rounds_played = 0;
    respawn_tanks();
}

void World::close()
{
    for (auto& box_t : tanks) {
        Tank& t = *box_t.get();
        t.quit();
    }
}

void World::refresh_zone()
{
    for (int i = 0; i < height; i++) {
        std::fill(zone[i].begin(), zone[i].end(), Color::EMPTY);
    }
    for (auto& box_t : tanks) {
        Tank& t = *box_t.get();
        zone[t.get_x()][t.get_y()] = Color::RED;
    }
    for (auto& box_t : tanks) {
        Tank& t = *box_t.get();
        zone[t.get_x()][t.get_y()] = Color::GREEN;
    }
}

void World::set_world_signal_status(int sig, siginfo_t* info, void* context)
{
    // fixme: atomic store
    World::world_signal_status = sig;
}

bool World::handle_signals()
{
    bool exit = false;
    // fixme: atomic load
    switch (world_signal_status) {
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        /* World will go out of scope */
        /* No need to call close explicitly */
        exit = true;
        break;
    case SIGUSR1:
        /* restart game */
        restart();
    default:
        exit = false;
        break;
    }
    return exit;
}

void setup_signal_handling()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    // sa.sa_sigaction  = World::set_world_signal_status;
    sa.sa_sigaction = World::set_world_signal_status;
    /* SA_SIGINFO flag must be set to use sa_sigaction handler */
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
}

int main(int argc, char *argv[])
{
    setup_signal_handling();
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

    w.play_round();

    return EXIT_SUCCESS;
}
