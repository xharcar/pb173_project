#pragma once

#include "config.h"
#include <ncurses.h>
#include <csignal>
#include <iostream>

using std::signal;
using std::cout;
using std::endl;

typedef struct {
    char * pipe;
} Options;

enum Color { RED = 1, GREEN = 2 };

void parse_args(int argc, char *argv[], Options * opts);

void print_help(char * progname);

class  WorldClient {
    pid_t world_pid;
protected:
    int height;
    int width;
public:
    WorldClient(char * pipe) { }
};

class NCursesClient  : public WorldClient {
    WINDOW * nc_world;
    WINDOW * nc_stats;
public:
    NCursesClient(char * pipe) : WorldClient(pipe) {
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

    ~NCursesClient() {
        delwin(nc_world);
        delwin(nc_stats);
        endwin();
    }

    void draw_tank(int x, int y, Color color) {
        wattrset(nc_world, COLOR_PAIR(color));
        /* Compensate for border padding */
        mvwaddch(nc_world, y + 1, x + 1, ACS_BLOCK);
        /* type cast enum class color */
        wattroff(nc_world, COLOR_PAIR(color));
        wrefresh(nc_world);
    }

    void undraw_tank(int x, int y) {
        /* COLOR_PAIR(0) sets the default color */
        wattrset(nc_world, COLOR_PAIR(0));
        /* Compensate for border padding */
        mvwaddch(nc_world, y + 1, x + 1, ' ');
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
