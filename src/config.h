#include <stdio.h>

#define BIN_DIR "../src"
#define WORLD      BIN_DIR "/" "world"
#define TANK       BIN_DIR "/" "tank"
#define TANKCLIENT BIN_DIR "/" "tankclent"

#define DEBUG_MSG(level,format,args...) \
    if(debug_level>=level)\
    {syslog(LOG_WARNING, "%s:%d : "format"\n",__FILE__,__LINE__,##args);}

char * test_msg = "Testing message\n";
