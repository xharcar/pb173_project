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
<<<<<<< HEAD
#include <cerrno>
#include <iostream>

=======
>>>>>>> origin/C++impl

using std::vector;
using std::unordered_map;
using std::pair;
using std::signal;
using std::string;
using std::cerr;

using Coord = pair<int, int>;

enum Color { RED = 1, GREEN = 2 };

<<<<<<< HEAD
string rtankpath;
string gtankpath;
=======
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
>>>>>>> origin/C++impl

class Tank {
    pid_t pid;
public:
    const int x;
    const int y;
    const Color color;
public:
    Tank(int x, int y, Color color) : x(x), y(y), color(color), pid(0) {
        // error handling for colors unnecessary
	// (calling with wrong color would happen how exactly?)
    }

    Tank(Coord coord, Color color) : Tank(coord.first, coord.second, color) { }

    /**
     * @brief PID getter (for sending signals,...)
     * @return PID of tank process
     */
    pid_t getPID(){
	    return this.pid;
    }

    void spawn_process(string tankpath) {
        FILE * read_pipe = popen(tankpath, "r");
        if ( read_pipe == NULL) {

        }
        /*--------------------------------------------------*/
        if (pid != 0) {
            /* Process for this tank has already been spawned successfully */
            return;
        }
        id = fork();
        if (id == -1) {
            this.pid = -1;

        } else if (id == 0) {
            execl(tankpath, tankpath,(char*)NULL);
            /* Should not be reached, log failure -> exit/handle failiure, insert assert */
        } else{
		this.pid = id;
		// sets PID of spawned tank for handling
	}
    }
};

class World {
    vector<Tank> green_tanks;
    vector<Tank> red_tanks;
protected:
    vector< vector<Color> > zone;
    const int height;
    const int width;
public:
    World(const int height, const int width) : height(height), width(width) {
        srand(std::time(0));
        for(int i=0;i<width;i++){
		zone.push_back(vector<Color,height>);
	}
	for(int i=0;i<width;i++){
		for(int j=0;j<height;j++){
			zone[i][j] = 0;
		}
	}
    }

    /**
     * @brief Spawns a tank at given coordinates, which must be empty
     * @param t info about tank to spawn
     */
    void add_tank(Tank& t) {
        zone[t.x][t.y] = t.color;
        if (t.color == Color::RED) {
            red_tanks.push_back(t);
	    t.spawn_process(rtankpath);
        } else {
            green_tanks.push_back(t);
	    t.spawn_process(gtankpath);
        }
    }

    /**
     * @brief Checks if given map coordinate is free
     * @param x x coordinate
     * @param y y coordinate
     * @return true if coordinate is free, else false
     */
    bool is_free(int x, int y) const {
         return (zone[x][y] == 0);
    }

    // TODO :
    // 1)send signal to tanks
    // 2)read actions
    // 3)FIRE EVERYTHING
    // 4)movement (tanks cannot dodge by moving, so fire > move)
    // 5)respawn
    // 6)goto 1 (can be done in world.c)
    void play_round(){
    	
    
    }
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
        signal(SIGQUIT, quit_safe);
        signal(SIGINT, quit_safe);
        /* send SIGTERM to all tanks, print game stats, close resources (pipes) */
        signal(SIGTERM, quit_safe);
        signal(SIGUSR1, restart);
    }

    ~DaemonWorld() {
        closelog();
    }

    void static quit_safe(int signal) {
        for (Tank& t : green_tanks) {
            if (kill(t.pid, SIGTERM) == -1) {
		cerr << "SIGTERM on tank with PID " << t.pid
		       	<< " failed with errno " << errno << ".";
		if(errno == ESRCH){ // pretty much only option
			cerr << "Waiting." << std::endl;
			waitpid(t.pid,NULL,0);
            	}
            }
   	 }
	for (Tank& t : red_tanks) {
            if (kill(t.pid, SIGTERM) == -1) {
		cerr << "SIGTERM on tank with PID " << t.pid
		       	<< " failed with errno " << errno << ".";
		if(errno == ESRCH){
			cerr << "Waiting." << std::endl;
			waitpid(t.pid,NULL,0);
            	}
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
		cerr << "Opening of pipe for map output failed." << endl;
        }
        stringstream ss;
	ss << width << "," << height;
        for (int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			ss << "," << zone[i][j];
		}
        }
	const char* mtw = (ss.str()).c_str();
        write(fd, (void*)mtw, strlen(mtw)*)

        // close()
        // unlink()
    }


};
/*
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
        // 0 = READ, 1 = WRITE
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);

        //  Executing in shell
        //  fixme: Try to leave out the shell 
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
*/
