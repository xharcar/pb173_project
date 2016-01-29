#include <iostream>
#include <getopt.h>

/**
 * @brief Utility class for holding important data
 */
class WorldOptions
{
    bool daemonize = false;
    unsigned round_time;
    unsigned map_height;
    unsigned map_width;
    std::string fifo_path;
    unsigned green_tanks_count;
    unsigned red_tanks_count;

public:
    int parse_options(int argc, char* argv[]);

    int check_valid() const;

    void print_help() const;

    void print_error() const;

    bool get_daemonize() const { return this->daemonize; }

    uint getRoundTime() const { return this->round_time; }

    uint get_map_height() const { return this->map_height; }

    uint get_map_width() const { return this->map_width; }

    uint get_green_tanks() const { return this->green_tanks_count; }

    uint get_red_tanks() const { return this->red_tanks_count; }

    std::string get_fifo_path() const { return this->fifo_path; }
};
