#pragma once

#include "config.h"
#include <ncurses.h>
#include <vector>
#include <unordered_map>
#include <csignal>
#include <sys/file.h>

using std::vector;
using std::unordered_map;
using std::pair;
using std::signal;
using std::string;

using Coord = pair<int, int>;

enum Color { RED = 1, GREEN = 2 };

class Tank {
    pid_t pid;
public:
    const Coord coord;
    // const int x;
    // const int y;
    const Color color;
public:
    // Tank(int x, int y, Color color) : x(x), y(y), color(color) {
    Tank(Coord coord, Color color) : coord(coord), color(color) {
        pid = fork();
        if (pid == -1) {
            /* Log unsuccessful fork() and exit? */
        } else if (pid == 0) {
            execl(TANK_BIN, "--sleep-max 5", "--sleep-min 1");
            /* Should not be reached, log failure -> exit/handle failiure, insert assert */
        }
    }
};

class World {
    /* fixme: Missing tank collection */
    // vector<Tank> tanks;
    vector< vector<Color> > zone;
    vector<Tank> tanks;
protected:
    const int height;
    const int width;
public:
    World(const int height, const int width) : height(height), width(width) {
        /* fixme: Initialize zone to given width and height  and fill it with zeroes*/
    }

    void add_tank(Tank& t) {
        if (zone[t.x][t.y] != 0) {
        }
        zone[t.x][t.y] = t.color;
    }

    Coord free_coord() const {
        // return not zone[x][y];
        int x, y;
        do {
            x = rand()%width;
            y = rand()%height;
        } while (zone[x][y] != 0);

        return pair<int, int>(x, y);
    }
};

class NCursesWorld  : World {
    WINDOW * nc_world;
    WINDOW * nc_stats;
public:
    NCursesWorld(int height, int width) : World(height, width) {
        initscr();
        start_color();
        /* Create color associations in ncurses */
        init_pair(Color::RED, COLOR_RED, COLOR_RED);
        init_pair(Color::GREEN, COLOR_GREEN, COLOR_GREEN);
        /* Hide the cursor in ncurses */
        curs_set(0);

        /* Add padding for borders */
        nc_world = newwin(height + 2, width + 2, 0, 0);
        nc_stats = newwin(10, 20, 1, width + 2 + 3);
        box(nc_world, 0, 0);
        wrefresh(nc_world);
    }

    ~NCursesWorld() {
        delwin(nc_world);
        delwin(nc_stats);
        endwin();
    }

    void draw_tank(Tank& t) {
        int x = t.coord.first;
        int y = t.coord.second;
        wattrset(nc_world, COLOR_PAIR(t.color));
        /* Compensate for border padding */
        mvwaddch(nc_world, t.y + 1, t.x + 1, ACS_BLOCK);
        wattroff(nc_world, COLOR_PAIR(t.color));
        wrefresh(nc_world);
    }

    void undraw_tank(Tank& t) {
        /* COLOR_PAIR(0) sets the default color */
        wattrset(nc_world, COLOR_PAIR(0));
        /* Compensate for border padding */
        mvwaddch(nc_world, t.y + 1, t.x + 1, ' ');
        wattroff(nc_world, COLOR_PAIR(0));
        wrefresh(nc_world);
    }

    void refresh_stats(int green_kills, int red_kills) {
        werase(nc_stats);
        wprintw(nc_stats, "Green tanks killed %d\n", green_kills);
        wprintw(nc_stats, "Red tanks killed %d\n", red_kills);
        wrefresh(nc_stats);
    }
};

class DaemonWorld : World {
    string pipe;
public:
    DaemonWorld(int height, int width, string pipe) : World(height, width), pipe(pipe) {
        openlog("Internet of Tanks: World", LOG_PID, LOG_USER);
        signal(SIGQUIT, quit);
        signal(SIGINT, quit);
        /* send SIGTERM to all tanks, print game stats, close resources (pipes) */
        signal(SIGTERM, quit_safe);
        signal(SIGUSR1, restart);
    }

    ~DaemonWorld() {
        closelog();
    }

    void outputMap() {
        if (mkfifo(pipe.c_str(), 0666)  == -1) {
            /* Failed to open the pipe */
            /* Log -> exit/retry */
            // perror("Failed to open the pipe");
        }
        int fd = open(pipe.c_str(), 0666);
        if (fd == -1) {

        }
        string map = width + ',' + height;
        for (tank : tanks) {
            map << ',' <<
        }
        write(fd, (void*))

        // write()
        // close()
        // unlink()
    }

    void quit() {

    }
};
