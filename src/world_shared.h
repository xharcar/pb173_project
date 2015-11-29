#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <random>
#include <iostream>

#include <cassert>
#include <ctime> // time(0)
#include <cstring>
#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};

using Coord = std::pair<int, int>;

/* Simplify by defining the signal handler function type, assume SA_SIGINFO */
typedef void (*signal_handler_t)(int, siginfo_t *, void *);

/* Per-thread variable pointing to the signal handler function */
static thread_local signal_handler_t thread_handler = nullptr;

/* Process-wide signal handler dispatches signal to thread signal handlers */
static void signal_handler(int signum, siginfo_t* info, void* context)
{
    signal_handler_t func;

    // fixme: use std::atomic
    func = __sync_fetch_and_or(&thread_handler, (signal_handler_t)0);

    if (func) {
        func(signum, info, context);
    }
}

void set_up_thread_hadler(signal_handler_t new_thread_handler) {
    signal_handler_t  func;

    do {
        func = thread_handler;
    } while (!__sync_bool_compare_and_swap(&thread_handler, func, new_thread_handler));
}

void process_signal_handling()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    // sa.sa_sigaction  = World::set_world_signal_status;
    sa.sa_sigaction  = signal_handler;
    /* SA_SIGINFO flag must be set to use sa_sigaction handler */
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
}


/*
struct RandomGen {
    std::minstd_rand seed;
    auto t = std::chrono::high_resolution_clock::now()::time_since_epoch().count();
    int rand() {
        return seed.
    }

};
*/

#endif // WORLD_SHARED_H
