#include <iostream>
#include <getopt.h>

/**
 * @brief Utility class for holding important data
 */
class WorldOptions
{
    bool daemonize;
    unsigned mRoundTime;
    unsigned mMapHeight;
    unsigned mMapWidth;
    std::string fifo_path;
    unsigned mGreenTanks;
    unsigned mRedTanks;

    unsigned red_kills;
    unsigned green_kills;
    unsigned rounds_played;

public:
    WorldOptions();

    void parse_options(int argc, char* argv[]);

    void print_help();

    void print_error();

    bool get_daemonize() const { return this->daemonize; }

    uint getRoundTime() const { return this->mRoundTime; }

    uint get_map_height() const { return this->mMapHeight; }

    uint get_map_width() const { return this->mMapWidth; }

    uint get_green_tanks() const { return this->mGreenTanks; }

    uint get_red_tanks() const { return this->mRedTanks; }

    uint getRedKills() const { return this->red_kills; }

    uint getGreenKills() const { return this->green_kills; }

    uint getRoundsPlayed() const { return this->rounds_played; }

    std::string get_fifo_path() const { return this->fifo_path; }

    void incRedKills() { this->red_kills++; }

    void incGreenKills() { this->green_kills++; }

    void incRoundsPlayed() { this->rounds_played++; }
};
