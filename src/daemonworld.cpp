#include "daemonworld.h"

void DaemonWorld::remove_hit_tanks()
{
    syslog(LOG_INFO,"Removing hit tanks\n");
    for (auto t = red_tanks.begin(); t != red_tanks.end(); t++) {
        if (t->get_hit()) {
            this->zone[t->get_x()][t->get_x()] = Color::EMPTY;
            t->kill_thread();
            red_tanks.erase(t);
        }
    }
    for (auto t = green_tanks.begin(); t != green_tanks.end(); t++) {
        if (t->get_hit()) {
            this->zone[t->get_x()][t->get_x()] = Color::EMPTY;
            t->kill_thread();
            green_tanks.erase(t);
        }
    }
}

void DaemonWorld::play_round(WorldOptions u)
{
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    syslog(LOG_INFO,"Round %d\n",u.getRoundsPlayed());

    read_commands();
    syslog(LOG_INFO, "FIRE EVERYTHING!\n");
    fire();
    syslog(LOG_INFO,"Moving tanks");
    movetanks();
    syslog(LOG_INFO,"Checking for tank crashes");
    crash_tanks();
    syslog(LOG_INFO,"Adding kills, old counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
    World::add_kills(u);
    syslog(LOG_INFO,"New kill counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
    remove_hit_tanks();
    syslog(LOG_INFO,"Respawning tanks\n");
    respawn_tanks(u);
    syslog(LOG_INFO,"Refreshing map\n");
    World::output_map();
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}


void DaemonWorld::close()
{
    syslog(LOG_INFO,"Quitting safely\n");
    closelog();
    World::close();
}

// END OF DAEMONWORLD OVERRIDES
