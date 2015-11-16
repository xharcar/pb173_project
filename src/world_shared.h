#pragma once

#include <cassert>
#include <ctime> // time(0)
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <signal.h>
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};
