#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <string>
#include <vector>
#include <thread>
#include <random>

#include <ctime> // time(0)
#include <cassert>
#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};

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
