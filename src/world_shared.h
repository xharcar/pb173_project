#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <streambuf>
#include <syslog.h>
#include <cstring>
#include <random>

#include <cassert>
#include <ctime> // time(0)
#include <cstring>
#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <pthread.h>

#include "randutils.hpp"

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};

using Coord = std::pair<int, int>;

/**
 * @brief rng easy to use random number generator
 */
//thread_local static randutils::mt19937_rng rng;
thread_local static std::mt19937 rng;

/* Simplify by defining the signal handler function type, assume SA_SIGINFO */
typedef void (*signal_handler_t)(int, siginfo_t *, void *);

/* Process-wide signal handler dispatches signal to thread signal handlers */
//static void signal_handler(int signum, siginfo_t* info, void* context);

void set_up_thread_hadler(signal_handler_t new_thread_handler);

void process_signal_handling();

void spawn_process(std::string command);


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

#endif // WORLD_SHARED_H
