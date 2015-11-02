#pragma once

#include "config.h"
#include <ncurses.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

using std::signal;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

typedef struct {
    char * pipe;
} Options;

enum Color { RED = 1, GREEN = 2 };

void parse_args(int argc, char *argv[], Options * opts);

void print_help(char * progname);

class  WorldClient {
    pid_t world_pid;
protected:
    FILE * pipe_stream;
    int height;
    int width;
public:
    WorldClient(char * pipe) {
        get_pid();
        open_pipe(pipe);
    }

    void get_pid() {
        ifstream pid_file;
        pid_file.open("/var/run/world.pid");
        if (pid_file) {
            cerr << "world.pid file does not exist. World process is not running." << endl;
            exit(-1);
        } else if ( pid_file >> world_pid ) {
            /* Successfully read the number  */
        } else {
            /* Failed to read the number */
            cerr << "Failed to read the pid from the world.pid file." << endl;
            exit(-1);
        }
    }

    void open_pipe(char * pipe) {
        int fd;
        if ( (fd = open(pipe, O_RDONLY) ) < 0 ) {
            cerr << strerror(errno) << "Can not open the pipe for streaming data from world process." << endl;
            exit(-1);
        }
        pipe_stream = fdopen(fd, "r");
        if (!pipe_stream) {
            cerr << strerror(errno) << "Can not open the pipe for streaming data from world process." << endl;
            exit(-1);
        }
        clearerr(pipe_stream);
        int dimensions = fscanf(pipe_stream, "%d, %d", &width, &height);
        if ( dimensions == EOF && ferror(pipe_stream) ) {
            cerr << strerror(errno) << "Error occured while parsing the pipe stream." << endl;
        } else if (dimensions != 2 ) {
            cerr << "Error: Worng format of the data in the pipe." << endl;
            exit(-1);
        }
    }
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

    void print_tanks() {
        char sector;
        int x = 0, y = 0;
        while ( fscanf(pipe_stream, ",%c", &sector) != EOF) {
            switch (sector) {
            case 'r':
                draw_tank(x, y, Color::RED);
                break;
            case 'g':
                draw_tank(x, y, Color::GREEN);
                break;
            case '0':
                undraw_tank(x, y);
                break;
            }
            x++;
            if (x >= width) {
                x = 0;
                y++;
            }
        }
        wrefresh(nc_world);
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

    /*
    void refresh_stats(int green_kills, int red_kills) {
        werase(nc_stats);
        wprintw(nc_stats, "Green tanks killed %d\n", green_kills);
        wprintw(nc_stats, "Red tanks killed %d\n", red_kills);
        wrefresh(nc_stats);
    }
    */
};
