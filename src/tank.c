#include "tank.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>


int maxFlag, minFlag, sleepMax, sleepMin;


int printError()
{
    fprintf(stderr, "Wrong arguments, use -h for help\n");
    return 1;
}

int printHelp()
{
    printf("=====================================================\n");
    printf("|         PB173 Internet Of Tanks presents:         |\n");
    printf("|            _________    _   ____ __               |\n");
    printf("|           /_  __/   |  / | / / //_/               |\n");
    printf("|            / / / /| | /  |/ / ,<                  |\n");
    printf("|           / / / ___ |/ /|  / /| |                 |\n");
    printf("|          /_/ /_/  |_/_/ |_/_/ |_|                 |\n");
    printf("-----------------------------------------------------\n");
    printf("                    USAGE                            \n");
    printf("  -h | --help       Show this help                   \n");
    printf("  --leep-min [n]    set min sleep time in secs       \n");
    printf("  --leep-max [n]    set max sleep time in secs       \n");
    printf("=====================================================\n");

    return 1;
}


int parseArgs(int argc, char *argv[])
{
    struct option longopts[] = {
       { "sleep-max",     required_argument,       NULL,  'v'   },
       { "sleep-min",    required_argument, NULL,   'm' },
       { "help",    no_argument,       NULL,    'h' },
      { 0, 0, 0, 0 }
    };
    int c;
    while ((c = getopt_long(argc, argv, "v:m:h", longopts, NULL)) != -1) {
        switch (c) {
        case 'v':   sleepMax = atoi(optarg);
            maxFlag = 1;
            break;
        case 'm':   sleepMin = atoi(optarg);
            minFlag = 1;
            break;
        case 'h':  return printHelp();
            break;
        default:  return printError();
        }

    }

    if(!(maxFlag && minFlag))   //we need both opts
        return printError();
    return 0;
}

int main(int argc, char *argv[])
{
    printf("tank.c: ");
    printf(test_msg);
    printf("Binary name: "TANK_BIN"\n");

    if(parseArgs(argc, argv) == 1)  //parse arguments
        return 1;

    srand(time(NULL));
    int sleepSecs;
    sleepSecs = rand() % (sleepMax-sleepMin) + sleepMin;
    printf("sleep for %i seconds", sleepSecs);
    printf("\n");

    usleep(sleepSecs*1000000);



    return 0;
}

