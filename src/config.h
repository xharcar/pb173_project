#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>

#define BIN_DIR "../src"
#define WORLD_BIN      BIN_DIR "/" "world"
#define TANK_BIN       BIN_DIR "/" "tank"
#define TANKCLIENT_BIN BIN_DIR "/" "tankclent"

#define DEBUG_MSG(level,format,args...) \
    if(debug_level>=level)\
    {syslog(LOG_WARNING, "%s:%d : "format"\n",__FILE__,__LINE__,##args);}

char * test_msg = "Testing message\n";
