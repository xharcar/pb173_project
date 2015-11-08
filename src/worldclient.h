#pragma once

#include <ncurses.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>

typedef struct {
    char * pipe;
} Options;

enum Color {
    RED = 1,
    GREEN = 2,
};

void parse_args(int argc, char *argv[], Options * opts);

void print_help(char * progname);

class  WorldClient {
protected:
    pid_t world_pid;
    FILE * pipe_stream;
    int height;
    int width;
public:
    WorldClient(char * pipe) {
        get_world_pid();
        open_pipe(pipe);
    }

    /*
     * Read pid of the world process from the file /var/run/world.pid
     */
    void get_world_pid();
    /*
     * Opens pipe for trasfering map from world process
     */
    void open_pipe(char * pipe);
};

class NCursesClient  : public WorldClient {
    WINDOW * nc_world;
    // WINDOW * nc_stats;
public:
    /*
     * Initializes ncurses
     */
    NCursesClient(char * pipe);

    ~NCursesClient() {
        delwin(nc_world);
        // delwin(nc_stats);
        endwin();
    }

    /*
     * Prints the whole map to the screen
     */
    void print_tanks();

    void keys();

    void draw_tank(int x, int y, Color color);
    
    void undraw_tank(int x, int y);

    /*
    void refresh_stats(int green_kills, int red_kills) {
        werase(nc_stats);
        wprintw(nc_stats, "Green tanks killed %d\n", green_kills);
        wprintw(nc_stats, "Red tanks killed %d\n", red_kills);
        wrefresh(nc_stats);
    }
    */
};
