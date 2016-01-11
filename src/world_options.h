#include <iostream>
#include <getopt.h>

/**
 * @brief Utility class for holding important data
 */
class WorldOptions
{
    bool daemonize;
    unsigned round_time;
    unsigned mMapHeight;
    unsigned mMapWidth;
    std::string fifo_path;
    unsigned mGreenTanks;
    unsigned mRedTanks;

public:
    WorldOptions();

    int parse_options(int argc, char* argv[]);

    int check_valid() const;

    void print_help() const;

    void print_error() const;

    bool get_daemonize() const { return this->daemonize; }

    uint getRoundTime() const { return this->round_time; }

    uint get_map_height() const { return this->mMapHeight; }

    uint get_map_width() const { return this->mMapWidth; }

    uint get_green_tanks() const { return this->mGreenTanks; }

    uint get_red_tanks() const { return this->mRedTanks; }

    std::string get_fifo_path() const { return this->fifo_path; }
};
