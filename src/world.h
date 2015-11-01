#pragma once

#include "config.h"
#include <ncurses.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <csignal>
#include <sys/file.h>
#include <ctime>
#include <boost/range/join.hpp>

using std::vector;
using std::unordered_map;
using std::pair;
using std::signal;
using std::string;

using Coord = pair<int, int>;

enum Color { RED = 1, GREEN = 2 };

class Utils {
    bool mDaemonize;
    int mRoundTime;
    int mMapHeight;
    int mMapWidth;
    std::string mGreenPath;
    std::string mRedPath;
    int mGreenTanks;
    int mRedTanks;
    bool mExit;

public:
    Utils(int argc, char *argv[]);
    ~Utils();
    void printHelp();
    void printError();

    bool getDaemonize() {return this->mDaemonize; }
    int getRoundTime() {return this->mRoundTime; }
    int getMapHeight() {return this->mMapHeight; }
    int getMapWidth() {return this->mMapWidth; }
    std::string getGreenPath() {return this->mGreenPath; }
    std::string getRedPath() {return this->mRedPath; }
    int getGreenTanks() {return this->mGreenTanks; }
    int getRedTanks() {return this->mRedTanks; }
    bool getExit() {return this->mExit; }



};

class Tank {
    pid_t pid;
public:
    const int x;
    const int y;
    const Color color;
public:
    Tank(int x, int y, Color color) : pid(0), x(x), y(y), color(color) {
        if (color != Color::RED && color != Color::GREEN) {
            /* Worng input color, Log -> handle failiure */
        }
    }

    Tank(Coord coord, Color color) : Tank(coord.first, coord.second, color) { }

    void spawn_process() {
        FILE * read_pipe = popen(TANK_BIN"--sleep-max 5 --sleep-min 1", "r");
        if ( read_pipe == NULL) {

        }
        /*--------------------------------------------------*/
        if (pid != 0) {
            /* Process for this tank has already been spawned */
            return;
        }
        pid = fork();
        if (pid == -1) {
            /* Log unsuccessful fork() and exit? */
        } else if (pid == 0) {
            execl(TANK_BIN, TANK_BIN, "--sleep-max 5", "--sleep-min 1");
            /* Should not be reached, log failure -> exit/handle failiure, insert assert */
        }
    }
};

class World {
    /* fixme: Missing tank collection */
    // vector<Tank> tanks;
    vector<Tank> green_tanks;
    vector<Tank> red_tanks;
protected:
    vector< vector<Color> > zone;
    const int height;
    const int width;
public:
    World(const int height, const int width) : height(height), width(width) {
        srand(std::time(0));
        /* fixme: Initialize zone to given width and height  and fill it with zeroes*/
    }

    /**
     * @brief add_tank on given coordinates, coordinates must be empty
     * @param t
     */
    void add_tank(Tank& t) {
        zone[t.x][t.y] = t.color;
        if (t.color == Color::RED) {
            red_tanks.push_back(t);
        } else {
            green_tanks.push_back(t);
        }
        t.spawn_process();
    }

    Coord free_coord() const {
        int x, y;
        do {
            x = rand()%width;
            y = rand()%height;
        } while (zone[x][y] != 0);

        return Coord(x, y);
    }

    // bool is_free(int x, int y) const {
    //     return not zone[x][y];
    // }
};

class NCursesWorld  : public World {
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
        wattrset(nc_world, COLOR_PAIR(t.color));
        /* Compensate for border padding */
        mvwaddch(nc_world, t.y + 1, t.x + 1, ACS_BLOCK);
        /* type cast enum class color */
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

    void static quit(int signal) {

    }

    void static quit_safe(int signal) {
        for (Tank& t : green_tanks) {
            if (kill(t.pid, SIGTERM) == -1) {
                perror("");
                /* fixme: What should we do if signal fails to be sent */
            }
        }
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
        stringstream
        string map = width + ',' + height;
        for (tank : tanks) {
            map << ',' <<
        }
        write(fd, (void*))

        // write()
        // close()
        // unlink()
    }


};

#define READ 0
#define WRITE 1

pid_t popen2(const char *command, int *infp, int *outfp)
{
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0) {
        return -1;
    }

    pid = fork();

    if (pid < 0) {
        return pid;
    } else if (pid == 0) {
        /* 0 = READ, 1 = WRITE*/
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);

        /* Executing in shell */
        /* fixme: Try to leave out the shell */
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[1]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];

    return pid;
}
