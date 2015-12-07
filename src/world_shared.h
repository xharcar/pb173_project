#ifndef WORLD_SHARED_H
#define WORLD_SHARED_H

#include <cassert>
#include <ctime> // time(0)
#include <cstdlib>
#include <string>
#include <vector>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};
// Utility type definitions
typedef std::pair<int, int> Coord;
typedef unsigned int uint;
// messages coming from tanks, to be processed
static std::vector<std::string> tank_messages;
// mutex for writing commands
static pthread_mutex_t worldmtxlock;
// conditional variable to control writing messages
static pthread_cond_t worldcvariable;
#endif // WORLD_SHARED_H
