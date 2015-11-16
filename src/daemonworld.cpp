
void DaemonWorld::add_tank(TankClient t, Utils u)
{
    if(t.getColor()==RED)
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

void DaemonWorld::crash_tanks(std::vector<TankClient> tanks1,
                        std::vector<TankClient> tanks2)
{
    syslog(LOG_INFO,"Checking for tank crashes");
    for(int i=0;i<tanks1.size();++i){
        for(int j=0;j<tanks2.size();++j){
            if(tanks1[i].getTID() != tanks2[j].getTID() &&
               // prevents a tank crashing into itself
               tanks1[i].getX() == tanks2[j].getX() &&
               tanks1[i].getY() == tanks2[j].getY() &&
               !tanks1[i].getHit() && !tanks2[j].getHit()){
                    tanks1[i].setHit(true);
                    tanks2[j].setHit(true);
            }
        }
    }
}

void DaemonWorld::add_kills(Utils u)
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

void DaemonWorld::respawn_tanks(Utils u)
{
    syslog(LOG_INFO,"Respawning tanks\n");
    while(red_tanks.size() < u.getRedTanks()){
        Coord c = World::free_coord();
        TankClient t = TankClient(c.first, c.second, RED);
        add_tank(t,u);
    }
    while(green_tanks.size() < u.getGreenTanks()){
        Coord c = World::free_coord();
        TankClient t = TankClient(c.first, c.second, GREEN);
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

void DaemonWorld::play_round(Utils u)
{
    std::vector<std::string> red_actions;
    std::vector<std::string> green_actions;
    // re-inited at every round start for easier management
    u.incRoundsPlayed();
    syslog(LOG_INFO,"Round %d\n",u.getRoundsPlayed());
    req_com();
    syslog(LOG_INFO,"Reading tank commands\n");
    read_com();
    syslog(LOG_INFO, "FIRE EVERYTHING!\n");
    fire();
    fire();
    syslog(LOG_INFO,"Moving tanks");
    movetanks();
    movetanks();
    crash_tanks(red_tanks,red_tanks);
    crash_tanks(red_tanks,green_tanks);
    crash_tanks(green_tanks,green_tanks);
    add_kills(u);
    remove_hit_tanks();
    respawn_tanks(u);
    refresh_zone();
    output_map();
    usleep((useconds_t)u.getRoundTime()*1000);
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
