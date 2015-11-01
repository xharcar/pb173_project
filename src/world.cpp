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

    int green_tanks = 15;
    int red_tanks = 15;
    int tanks_number = green_tanks + red_tanks;
    NCursesWorld w(15, 30);
    for (int i = 0; i < green_tanks; i++) {
        Tank t = Tank(w.free_coord(), Color::GREEN);
        w.add_tank(t);
    }
    for (int i = 0; i < red_tanks; i++) {
        Tank t = Tank(w.free_coord(), Color::RED);
        w.add_tank(t);
    }

    while (tanks_number > 0) {
        pid_t stopped_tank = wait(NULL);
        tan
    }

}
