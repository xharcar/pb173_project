#include <string>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h> // mkfifo
#include <signal.h>
#include <pthread.h>

enum Color {EMPTY = 0, RED = 'r', GREEN = 'g'};
