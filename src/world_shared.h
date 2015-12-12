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
// conditional variable to control writing messages
static pthread_cond_t worldcvariable;
static std::vector<std::string> tank_messages;
#endif // WORLD_SHARED_H
