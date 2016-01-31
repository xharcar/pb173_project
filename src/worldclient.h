#ifndef WORLDCLIENT_H
#define WORLDCLIENT_H

#include <cassert>
#include <ncurses.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

struct ClientOptions {
    char* pipe;
};

enum Color {
    //RED = 1,
    //GREEN = 2,
    RED = 'r',
    GREEN = 'g',
};

void parse_args(int argc, char* argv[], ClientOptions& opts);

void print_help(char* progname);

class WorldClient {
protected:
    pid_t world_pid;
    char* mPipe;
    int pipe_stream;
    int height;
    int width;

public:
    WorldClient(char* pipe)
    {
        mPipe = pipe;
        get_world_pid("./world.pid");
        open_pipe(pipe);
    }

    /**
     * @brief Read pid of the world process from the file /var/run/world.pid
     */
    void get_world_pid(std::string filepath);

    /**
     * @brief Opens pipe for trasfering map from world process
     */
    void open_pipe(char* pipe);

    void parse_dimensions();
};

class NCursesClient : public WorldClient {
    WINDOW* nc_world;
    // WINDOW * nc_stats;

public:
    /*
     * Initializes ncurses
     */
    NCursesClient(char* pipe);

    ~NCursesClient()
    {
//        delwin(nc_world);
//        // delwin(nc_stats);
//        endwin();
    }

    /**
     * @brief Prints the whole map to the screen
     */
    void print_tanks();

    /**
     * @brief handle input commands from the keyboards
     */
    void keys();

    /**
     * @brief draw tank on the screen
     */
    void draw_tank(int x, int y, Color color);

    /**
     * @brief remove tank ftom the screen
     */
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

#endif // WORLDCLIENT_H
