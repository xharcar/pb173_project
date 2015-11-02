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
#include <cerrno>
#include <iostream>
#include <string>


using std::vector;
using std::unordered_map;
using std::pair;
using std::signal;
using std::string;
using std::cerr;


using Coord = pair<int, int>;

enum Color { RED = 1, GREEN = 2 };

int red_kills;
int green_kills;
string rtankpath;
string gtankpath;
Utils globals;

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
    FILE* read_pipe;
    bool hit;
public:
    const int x;
    const int y;
    const Color color;
public:
    Tank(int x, int y, Color color) : x(x), y(y), color(color), pid(0),hit(false) {
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
    /**
     * @brief read pipe getter
     * @return pointer to read pipe
     */
    FILE* getRPipe(){
	    return this.read_pipe;
    }
    bool getHit(){
    	return this.hit;
    }
    void setHit(bool shot){
    	this.hit = shot;
    }
    void spawn_process(string tankpath) {
        read_pipe = popen(tankpath, "r");
        if ( read_pipe == NULL) {

        }
        /*--------------------------------------------------*/
        if (pid != 0) {
            /* Process for this tank has already been spawned successfully */
            return;
        }
        pid_t id = fork();
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
    void add_tank(Tank t) {
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
    // 1)send signal to tanks - done
    // 2)read actions - done
    // 3)FIRE EVERYTHING
    // 4)movement (tanks cannot dodge by moving, so fire > move)
    // 5)respawn
    // 6)goto 1 (can be done in world.c)
    void play_round() {
    	vector<string>red_actions;
	vector<string>green_actions;
	char buf[4] = "\0";
	for(Tank t : red_tanks){
		kill(t.getPID(),SIGUSR2);
	}
	for(Tank t : green_tanks){
		kill(t.getPID(),SIGUSR2);
	}
	for(Tank t : red_tanks){
		read(t.getRPipe(),buf,3);
		red_actions.push_back(string(buf));
		memset(buf,0,4);
	}
	for(Tank t : green_tanks){
		read(t.getRPipe(),buf,3);
		red_actions.push_back(string(buf));
		memset(buf,0,4);
	}
	// Probably should be a separate function
	for(int i = 0;i<red_tanks.size();i++){
		switch(red_actions[i][0]){
			case 'm' : continue;
				   break;
			case 'f' : {
				switch(red_actions[i][1]){
					case 'u':{
						for(Tank t: green_tanks){ // regular fire
							if(t.y>red_tanks[i].y) setHit(true);
						}
						for(Tank t: red_tanks){ // friendly fire
							if(t.y>red_tanks[i].y) setHit(true);
						}
					}break;
					case 'd':{
						for(Tank t: green_tanks){
							if(t.y<red_tanks[i].y) setHit(true);
						}
						for(Tank t: red_tanks){
							if(t.y<red_tanks[i].y) setHit(true);
						}
					}break;
					case 'l':{
						for(Tank t: green_tanks){ 
							if(t.x<red_tanks[i].x) setHit(true);
						}
						for(Tank t: red_tanks){ 
							if(t.x<red_tanks[i].x) setHit(true);
						}
					}break;
					case 'r':{
						for(Tank t: green_tanks){ 
							if(t.x>red_tanks[i].x) setHit(true);
						}
						for(Tank t: red_tanks){ 
							if(t.x>red_tanks[i].x) setHit(true);
						}
					}break;
				}
			}
		}
	}
	for(int i = 0;i<green_tanks.size();i++){
		switch(green_actions[i][0]){
			case 'm' : continue;
				   break;
			case 'f' : {
				switch(green_actions[i][1]){
					case 'u':{
						for(Tank t: green_tanks){ //regular fire
							if(t.y>red_tanks[i].y) setHit(true);
						}
						for(Tank t: red_tanks){ //friendly fire
							if(t.y>red_tanks[i].y) setHit(true);
						}
					}break;
					case 'd':{
						for(Tank t: green_tanks){
							if(t.y<red_tanks[i].y) setHit(true);
						}
						for(Tank t: red_tanks){
							if(t.y<red_tanks[i].y) setHit(true);
						}
					}break;
					case 'l':{
						for(Tank t: green_tanks){ 
							if(t.x<red_tanks[i].x) setHit(true);
						}
						for(Tank t: red_tanks){ 
							if(t.x<red_tanks[i].x) setHit(true);
						}
					}break;
					case 'r':{
						for(Tank t: green_tanks){ 
							if(t.x>red_tanks[i].x) setHit(true);
						}
						for(Tank t: red_tanks){ 
							if(t.x>red_tanks[i].x) setHit(true);
						}
					}break;
				}
			}
		}
	}
	for(int i=0;i<red_tanks.size();++i){
		if(red_tanks[i].getHit) {
			green_kills++;
			continue;
		}
		if(red_actions[i][0] != 'm') continue;
		// hit tanks cannot move (~= are on fire but haven't exploded yet)
		else{

			switch(red_actions[i][1]){
				// TODO: map border checking
				case 'u' : red_tanks[i].y++;
					   break;
				case 'd' : red_tanks[i].y--;
					   break;
				case 'l' : red_tanks[i].x--;
					   break;
				case 'r' : red_tanks[i].x++;
					   break;
			}
		}
	}
	for(int i=0;i<green_tanks.size();++i){
		if(green_tanks[i].getHit) {
			red_kills++;
			continue;
		}
		if(green_actions[i][0] != 'm' || green_tanks[i].getHit()) continue;
		else{

			switch(green_actions[i][1]){
				// TODO: map border checking
				case 'u' : green_tanks[i].y++;
					   break;
				case 'd' : green_tanks[i].y--;
					   break;
				case 'l' : green_tanks[i].x--;
					   break;
				case 'r' : green_tanks[i].x++;
					   break;
			}
		}
	}
	for(Tank t : green_tanks){
		if(t.getHit()){
			kill(t.getPID(),SIGTERM);
			green_tanks.erase(t);
		}
	}
	for(Tank t : red_tanks){
		if(t.getHit()){
			kill(t.getPID(),SIGTERM);
			red_tanks.erase(t);
		}
	}
	while(green_tanks.size()<globals.getGreenTanks()){
		int x = std::rand()%this.width;
		int y = std::rand()%this.height;
		if(is_free(x,y)){
			Tank t = new Tank(x,y,RED);
			add_tank(t);
		}
	}
	while(red_tanks.size()<globals.getRedTanks()){
		int x = std::rand()%this.width;
		int y = std::rand()%this.height;
		if(is_free(x,y)){
			Tank t = new Tank(x,y,RED);
			add_tank(t);
		}
	}
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
            if (kill(t.getPID(), SIGTERM) == -1) {
		cerr << "SIGTERM on tank with PID " << t.getPID()
		       	<< " failed with errno " << errno << ".";
		if(errno == ESRCH){ // pretty much only option
			cerr << "Waiting." << std::endl;
			waitpid(t.getPID(),NULL,0);
            	}
            }
   	 }
	for (Tank& t : red_tanks) {
            if (kill(t.getPID(), SIGTERM) == -1) {
		cerr << "SIGTERM on tank with PID " << t.getPID()
		       	<< " failed with errno " << errno << ".";
		if(errno == ESRCH){
			cerr << "Waiting." << std::endl;
			waitpid(t.getPID(),NULL,0);
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
        write(fd, (void*)mtw, strlen(mtw));

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
