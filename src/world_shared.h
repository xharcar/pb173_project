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
#include <syslog.h>
#include <streambuf>
#include <cstring>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};
// Utility type definitions
typedef std::pair<int, int> Coord;
typedef unsigned int uint;
// conditional variable to control writing messages
static pthread_cond_t worldcvariable;
static std::vector<std::string> tank_messages;

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
