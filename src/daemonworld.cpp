#include "daemonworld.h"

void DaemonWorld::add_tank(Tank t, WorldOptions u)
{
    if(t.getColor()==Color::RED)
    {
        syslog(LOG_INFO,"Adding red tank\n");
        red_tanks.push_back(t);
        spawn_thread(t,u.getRedPath());
    }
    else
    {
        syslog(LOG_INFO,"Adding green tank\n");
        green_tanks.push_back(t);
        spawn_thread(t,u.getGreenPath());
    }
}

void DaemonWorld::add_kills(WorldOptions u)
{
    syslog(LOG_INFO,"Adding kills, old counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
    for(int i=0;i<red_tanks.size();++i){
        if(red_tanks[i].getHit()) u.incGreenKills();
    }
    for(int i=0;i<green_tanks.size();++i){
        if(green_tanks[i].getHit()) u.incRedKills();
    }
    syslog(LOG_INFO,"New kill counts: \nRed: %d\nGreen: %d\n",u.getRedKills(),u.getGreenKills());
}

void DaemonWorld::remove_hit_tanks()
{
    syslog(LOG_INFO,"Removing hit tanks\n");
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        if(t->getHit()){
            pthread_kill(t->getTID(),SIGTERM);
            red_tanks.erase(t);
        }
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        if(t->getHit()){
            pthread_kill(t->getTID(),SIGTERM);
            green_tanks.erase(t);
        }
    }
}

void DaemonWorld::respawn_tanks(WorldOptions u)
{
    syslog(LOG_INFO,"Respawning tanks\n");
    while(red_tanks.size() < u.getRedTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.getGreenTanks()){
        Coord c = World::free_coord();
        Tank t = Tank(c.first, c.second, GREEN);
        add_tank(t,u);
    }
}

void DaemonWorld::refresh_zone()
{
    syslog(LOG_INFO,"Refreshing map\n");
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;i++){
            zone[i][j] = EMPTY;
        }
    }
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        zone[t->getX()][t->getY()] = RED;
    }
    for(auto t=green_tanks.begin();t!=green_tanks.end();++t){
        zone[t->getX()][t->getY()] = GREEN;
    }
}

void DaemonWorld::play_round(WorldOptions u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    red_actions.resize(u.getRedTanks());
    green_actions.resize(u.getGreenTanks());
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    syslog(LOG_INFO,"Round %d\n",u.getRoundsPlayed());
    pthread_cond_signal(&cvar);
    usleep((useconds_t)u.getRoundTime()*1000);
    process_commands(u,red_actions,green_actions);
    syslog(LOG_INFO, "FIRE EVERYTHING!\n");
    fire();
    syslog(LOG_INFO,"Moving tanks");
    movetanks();
    syslog(LOG_INFO,"Checking for tank crashes");
    crash_tanks(red_tanks,red_tanks);
    crash_tanks(red_tanks,green_tanks);
    crash_tanks(green_tanks,green_tanks);
    add_kills(u);
    remove_hit_tanks();
    respawn_tanks(u);
    refresh_zone();
    output_map();
    // waits for round time to pass : round time given in ms,
    // sleep time in us, hence *1000
}


void DaemonWorld::quit_safe(int sig)
{
    syslog(LOG_INFO,"Quitting safely\n");
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    for(auto t=red_tanks.begin();t!=red_tanks.end();++t){
        pthread_kill(t->getTID(),SIGTERM);
        pthread_join(t->getTID(),NULL);
    }
    red_tanks.clear();
    green_tanks.clear();
    zone.clear();
    close(pipefd);
}

void DaemonWorld::output_map()
{
    std::stringstream ss;
    ss << width << ',' << height;
    for(uint i=0;i<height;i++){
        for(uint j=0;j<width;j++){
            ss << ',' << zone[i][j];
        }
    }
    write(pipefd,ss.str().c_str(),(height*width*2)+4);
}

// END OF DAEMONWORLD OVERRIDES
