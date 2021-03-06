#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <streambuf>
#include <utility>
#include <vector>
#include <atomic>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};

using Coord = std::pair<int, int>;

/**
 * @brief rng holds the state of a random number generator
 */
thread_local static std::mt19937 rng;

/* Simplify by defining the signal handler function type, assume SA_SIGINFO */
typedef void (*signal_handler_t)(int, siginfo_t *, void *);

/* Process-wide signal handler dispatches signal to thread signal handlers */
//static void signal_handler(int signum, siginfo_t* info, void* context);

void set_up_thread_hadler(signal_handler_t new_thread_handler);

void process_signal_handling();

class Log : public std::basic_streambuf<char, std::char_traits<char> > {
public:
    explicit Log(std::string ident_, int facility_, int priority_);
    ~Log();

protected:
    int sync();
    int overflow(int c);

private:
    std::string buffer;
    int facility;
    int priority;
    char ident[50];
};

class NamedFifo {
    std::string filepath;
    int success;

public:
    NamedFifo(std::string filepath) : filepath(filepath) {}

    ~NamedFifo() {
            std::remove(filepath.c_str());
    }

    /**
     * @brief open
     * @return success
     */
    int open_nf() {
        if((success = mkfifo(filepath.c_str(), 0777)) < 0){
		return success;
    	}
	int s = S_IRWXU|S_IRWXG|S_IRWXO;
	success = open(filepath.c_str(),O_WRONLY,s);
        return success;
    }	

};

/**
 * @brief checks whether an instance of world is already running
 */
class RunningInstance {
    int pid_fd;
    std::string pid_filepath;

public:
    RunningInstance(std::string pid_filepath) : pid_filepath(pid_filepath) {}

    ~RunningInstance()
    {
        if (pid_fd > 0) {
            close(pid_fd);
        }
    }

    int acquire()
    {
        pid_fd = ::open(pid_filepath.c_str(), O_CREAT | O_RDWR, 0666);
        if (pid_fd <= 0) {
            std::cout << "Failed to open " << pid_filepath << ": "
                      << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }

        int locked;
        while ((locked = flock(pid_fd, LOCK_EX | LOCK_NB))) {
            switch (errno) {
            // Another instance is running
            case EWOULDBLOCK:
                std::cout << "Another instance of world is already running."
                          << std::endl;
                std::cout << "Waiting for its end." << std::endl;
                watch_pid(pid_filepath);
                break;
            }
        }
        return pid_fd;
    }

    ssize_t write_pid(){
	int pid = (int) getpid();
	std::cout << "Writing PID : " << pid << " to file." << std::endl;
	std::string pidstr = std::to_string(pid);
	std::cout << "Written string: " << pidstr << std::endl;
	ssize_t rv =  write(pid_fd,pidstr.c_str(),pidstr.size()+1);
	if(rv >= 0){
		std::cout << "Write successful. " << rv << " bytes written." << std::endl;
	}
	return rv;
    }

private:
    /**
     * @brief blocking call which waits for the end of other world instances
     * @param pid_filepath location of pid file to check
     */
    void watch_pid( std::string pid_filepath )
    {
        int inotify_instance = inotify_init();
        if ( inotify_instance == -1 ) {
            std::cerr << "Failed to create inotify instance" << std::endl;
            assert(false);
        }
        if (inotify_add_watch( inotify_instance, pid_filepath.c_str(), IN_CLOSE)) {
            std::cerr << "Failed to add file in to inotify instance" << std::endl;
            assert(false);
        }

        /* Blocking call waiting for the end of a different world */
        select(inotify_instance, NULL, NULL, NULL, NULL);
    }

};

#endif // WORLD_SHARED_H
