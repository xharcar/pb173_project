#include "world.h"

int main()
{
    int pid_file = open("/var/run/world.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        /* Another instance is running */
        if(EWOULDBLOCK == errno);
    }
    else {
        /* this is the first instance */
    }

    NCursesWorld w(15, 30);
}
