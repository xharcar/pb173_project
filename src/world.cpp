#include "world.h"
#include "world_shared.h"

std::string pf = "./world.pid";
std::string pfdel = "rm " + pf;
const char* pfdel_c = pfdel.c_str();

volatile std::atomic<int> World::world_signal_status;

World::World(WorldOptions& opts, int pd)
    : height(opts.get_map_height()),
      width(opts.get_map_width()),
      zone(height, std::vector<Color>(width, Color::EMPTY)),
      map_fifo(pd),
      opts(opts)
{
    tanks.reserve(opts.get_red_tanks() + opts.get_green_tanks());
    
    if (opts.get_daemonize()) {
        /* Replace std::cout with custom syslog stream */
        std::cout.rdbuf(new Log("Internet of Tanks", LOG_USER, LOG_INFO));
    }
    world_signal_status.store(0);
	// atomic status inited here for simplicity, doesn't matter anyway
}

void World::play_round()
{
    while(true) {
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

        refresh_zone();
        output_map();
        if (handle_signals()) {
	    close();
            break;
        }
        // waits for round time to pass
        std::this_thread::sleep_for(std::chrono::seconds(opts.getRoundTime()));
    }
}

void World::read_commands()
{
    for(uint i=0;i<tanks.size();i++) {
        tanks[i]->read_command();
    }
}

void World::process_shots()
{
    for(uint i=0;i<tanks.size();i++) {
        if (tanks[i]->get_command()[0] == 'f') {
            fire_direction(tanks[i]);
        }
    }
}

void World::process_moves()
{
    for(uint i=0;i<tanks.size();i++) {
        if (tanks[i]->get_command()[0] == 'm') {
            movetank(tanks[i]);
        }
    }
}

void World::fire_direction(Tank *t)
{
    for (uint i=0;i<tanks.size();i++) {
        /* Pick the right operation for finding targets relative to this tank's
         * position */
        std::function<bool(int, int)> x_op = std::equal_to<int>();
        std::function<bool(int, int)> y_op = std::equal_to<int>();
        switch (t->get_command()[1]) {
        case 'u':
            x_op = std::less<int>();
            break;
        case 'd':
            x_op = std::greater<int>();
            break;
        case 'l':
            y_op = std::less<int>();
            break;
        case 'r':
            y_op =  std::greater<int>();
            break;
        default:
            /* Invalid command */
            assert(false);
        }

        if (y_op(tanks[i]->get_y(), t->get_y()) &&
            x_op(tanks[i]->get_x(), t->get_x()))
        {
            t->get_shot();
            t->print_destroyed(tanks[i]);
        }
    }
}

void World::movetank(Tank *t)
{
        int x_shift = 0;
        int y_shift = 0;

        switch (t->get_command()[1]) {
        case 'u':
            x_shift--;
            break;
        case 'd':
            x_shift++;
            break;
        case 'l':
            y_shift--;
            break;
        case 'r':
            y_shift++;
            break;
        default:
            /* malformed command format */
            assert(false);
        }

        /* Set new coordinates only for tanks that haven't been shot or crashed */
        Coord new_pos = Coord(t->get_x() + x_shift, t->get_y() + y_shift);
	
	if (out_of_bounds(new_pos)) {
            t->get_crashed();
            t->print_out_of_map();
            new_pos = Coord(t->get_x(), t->get_y());
	    return;
        }

        for (uint i=0;i<tanks.size();i++)
        {
            if (t->get_order() != tanks[i]->get_order() &&
                new_pos == tanks[i]->get_position() &&
                !tanks[i]->is_shot() &&
                !t->is_shot())
            {
                /* crash tanks */
                t->get_crashed();
                tanks[i]->get_crashed();
                t->print_crashed(tanks[i]);
		return;
            }
        }

        t->set_new_position(new_pos);
}

bool World::out_of_bounds(Coord pos)
{
    return (pos.first < 0 || pos.first >= height || pos.second < 0 ||
            pos.second >= width);
}

void World::add_tank(Color color,int order)
{
    Coord pos = free_coord();
    this->zone[pos.first][pos.second] = color;

    if (color == Color::RED) {
        red_tanks++;
    } else {
        green_tanks++;
    }
    tanks.push_back(new Tank(pos,color,order));
}

void World::init_tanks(){
	int count=0;
	while(red_tanks < opts.get_red_tanks()){
		add_tank(Color::RED,count);
		count++;
	}
	while(green_tanks < opts.get_green_tanks()){
		add_tank(Color::GREEN,count);
		count++;
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
        x = height_rand(rng);
        y = width_rand(rng);
    } while (!is_free(x, y));
    return Coord(x, y);
}


void World::sum_score()
{
    for (uint i=0;i<tanks.size();i++) {
        if (tanks[i]->is_shot()) {
            if (tanks[i]->get_color() == Color::RED) {
                green_kills++;
            } else {
                red_kills++;
            }
        }
    }
}

void World::respawn_tanks()
{
    for(uint i=0;i<tanks.size();i++ ){	
	tanks[i]->revive();
    }
}

void World::output_map()
{
    std::stringstream ss;
    ss << width << ',' << height << ',';
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ss << '(';
            switch(zone[i][j])
            {
            case EMPTY:
                ss << '0';
                break;
            case RED:
                ss << 'r';
                break;
            case GREEN:
                ss << 'g';
                break;
            }
            ss << "),";
        }
    }
    std::string outstr = ss.str();
    if(write(map_fifo,outstr.c_str(),outstr.size()) < 0){
	std::cerr << "Failed output to FIFO: " << strerror(errno) << std::endl;	
    }

}

void World::restart() {
    green_kills = 0;
    red_kills = 0;
    rounds_played = 0;
    respawn_tanks();
}

void World::close()
{
    for (uint i=0;i<tanks.size();i++) {
        tanks[i]->quit();
	delete tanks[i];
    }
    tanks.clear();
}

void World::refresh_zone()
{
    for (int i = 0; i < height; i++) {
        std::fill(zone[i].begin(), zone[i].end(), Color::EMPTY);
    }
    for (uint i=0;i<tanks.size();i++) {
        zone[tanks[i]->get_x()][tanks[i]->get_y()] = tanks[i]->get_color();
    }
}

void World::set_world_signal_status(int sig, siginfo_t* info, void* arg)
{
   World::world_signal_status.store(sig,std::memory_order_seq_cst);
   info=info;// ignoring other arguments
   arg=arg;
}


bool World::handle_signals()
{
    bool exit = false;
    switch (this->world_signal_status.load(std::memory_order_seq_cst)) {
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        /* World will go out of scope */
        /* No need to call close explicitly */
        exit = true;
        break;
    case SIGUSR1:
        /* restart game */
	world_signal_status.store(0);
        restart();
	// need to reset this so world doesn't keep restarting
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
    RunningInstance instance(pf);
    if (instance.acquire() <= 0) {
        return EXIT_FAILURE;
    }
    if (instance.write_pid() < 0){
	std::cout << "Writing PID to PID file failed. Exiting." << std::endl;
	return EXIT_FAILURE;
    }

    WorldOptions opts;
    if (opts.parse_options(argc, argv)) {
	std::cerr << "World options parse failed" <<std::endl;
        return EXIT_FAILURE;
    }
    if (opts.check_valid()) {
	std::cerr << "World options invalid" << std::endl;
        return EXIT_FAILURE;
    }
    int pd;
    NamedFifo map_fifo(opts.get_fifo_path());
    if ((pd = map_fifo.open_nf()) < 0) {
        std::cout << "Failed to open " << opts.get_fifo_path() << ": "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    World w(opts,pd);

    w.init_tanks();
    w.play_round();
    system(pfdel_c);
    std::string frm = "rm " + opts.get_fifo_path();
    system(frm.c_str());
    return EXIT_SUCCESS;
}
