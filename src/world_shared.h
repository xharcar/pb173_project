#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <cassert>
#include <ctime> // time(0)
#include <cstdlib>
#include <string>
#include <vector>
#include <csignal>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};

struct RandomGen {
    std::minstd_rand seed;
    seed.(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    int rand() {
        return
    }

}

#endif // WORLD_SHARED_H
